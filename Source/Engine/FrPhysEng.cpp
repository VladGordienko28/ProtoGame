/*=============================================================================
    FrPhysEng.cpp: Flu physics engine.
    Copyright Sep.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"
#include <search.h>

/*-----------------------------------------------------------------------------
	CPhysics implementation.
-----------------------------------------------------------------------------*/

//
// Static variables.
//
math::Vector			CPhysics::HitNormal;
math::Vector			CPhysics::HitSlope;
Float					CPhysics::HitTime;
EHitSide				CPhysics::HitSide;
FLevel*					CPhysics::Level;
EHitSolution			CPhysics::Solution;
Bool					CPhysics::bBrake;
Float					CPhysics::BodyInvMass;
Float					CPhysics::BodyInvIner;
Float					CPhysics::OtherInvMass;
Float					CPhysics::OtherInvIner;
FBaseComponent*			CPhysics::Other;
FBaseComponent*			CPhysics::Others[MAX_COLL_LIST_OBJS];
Int32					CPhysics::NumOthers;
math::Vector			CPhysics::AVerts[16];
math::Vector			CPhysics::ANorms[16];
math::Vector			CPhysics::BVerts[16];
math::Vector			CPhysics::BNorms[16];
Int32					CPhysics::ANum;
Int32					CPhysics::BNum;
math::Vector			CPhysics::Contacts[2];
Int32					CPhysics::NumConts;


/*-----------------------------------------------------------------------------
	Material table.
-----------------------------------------------------------------------------*/

//
// A properties of the physics material.
//
struct TPhysMaterial
{
public:
	Float		Density;
	Float		Elasticity;
	Float		SFriction;
	Float		DFriction;
};


//
// Materials table. Not real values, just
// my imagination.
//
static const TPhysMaterial GMaterials[PM_MAX]	=
{
//-----------------------------------------------------------------------------
//		Density		|	Elasticity		|	SFriction		|	DFriction	
//		-------			----------			---------			---------
	{	1.00f,			0.05f,				0.30f,				0.20f },		// PM_Custom		
	{	0.65f,			0.20f,				0.25f,				0.20f },		// PM_Wood			
	{	2.60f,			0.10f,				0.20f,				0.20f },		// PM_Rock			
	{	7.00f,			0.05f,				0.45f,				0.40f },		// PM_Metal			
	{	0.90f,			0.09f,				0.09f,				0.05f },		// PM_Ice			
	{	2.60f,			0.07f,				0.75f,				0.40f },		// PM_Glass			
	{	0.30f,			0.80f,				0.75f,				0.35f },		// PM_BouncyBall
//-----------------------------------------------------------------------------
};


/*-----------------------------------------------------------------------------
    Physics magic numbers.
-----------------------------------------------------------------------------*/

//
// Magic constants :3
//
#define PHYS_PENET_PERCENT		0.65f		// Penetration percentage.
#define PHYS_PENET_ALLOW		0.005f		// Penetration allowance.	
#define INFINITE_MASS			999999.9f	
#define SLEEP_THRESHOLD			0.01f


/*-----------------------------------------------------------------------------
    Physics utility.
-----------------------------------------------------------------------------*/

//
// Retrieve the hit side.
//
inline EHitSide NormalToSide( const math::Vector& N )
{
	// About 45 degs.
	if( ( N.x < +0.7f ) && ( N.x > -0.7f ) )
	{
		// Bottom or Top.
		return N.y > 0.f ? HSIDE_Top : HSIDE_Bottom;
	}
	else
	{
		// Left or Right.
		return N.x > 0.f ? HSIDE_Right : HSIDE_Left;
	}
}


inline EHitSide OppositeSide( EHitSide Side )
{
	static const EHitSide OpSides[HSIDE_MAX] =
	{
		HSIDE_Bottom,
		HSIDE_Top,
		HSIDE_Right,
		HSIDE_Left
	};
	return OpSides[Side];
}


//
// Return body's invert mass.
//
inline Float GetInvMass( FPhysicComponent* Body )
{
	return Body->Mass > 0.f ? 1.f/Body->Mass : 0.f;
}


//
// Return body's invert moment of inertia.
//
inline Float GetInvInertia( FPhysicComponent* Body )
{
	return (Body->Inertia > 0.f && !Body->bFixedAngle) ? 1.f/Body->Inertia : 0.f;
}


//
// Retrieve body's polygon vertices.
//
void BodyToPoly( FBaseComponent* Body, math::Vector* OutVerts, math::Vector* OutNorms, Int32& OutNum )
{
	if( Body->IsA(FBrushComponent::MetaClass) )
	{
		FBrushComponent* Brush = (FBrushComponent*)Body;

		// Transform vertices from local to world.
		// Brushes don't support rotation, so
		// simplify transformation.
		OutNum	= Brush->NumVerts;
		for( Int32 i=0; i<OutNum; i++ )
			OutVerts[i] = Brush->Vertices[i] + Brush->Location;

		// Compute normals.
		math::Vector P1=Brush->Vertices[OutNum-1], P2;
		for( Int32 i=0, j=OutNum-1; i<OutNum; j=i, i++ )
		{
			P2			= Brush->Vertices[i];
			math::Vector N	= P2 - P1;
			N.normalize();
			OutNorms[j]	= N.cross();
			P1			= P2;
		}
	}
	else 
	{
		// Rectangle.
		if( Body->bFixedAngle || !Body->Rotation )
		{
			// AABB body.
			math::Rect AABB( Body->Location, Body->Size.x, Body->Size.y );
			OutNum	= 4;

			OutVerts[0]	= math::Vector( AABB.min.x, AABB.min.y );
			OutVerts[1]	= math::Vector( AABB.min.x, AABB.max.y );
			OutVerts[2]	= math::Vector( AABB.max.x, AABB.max.y );
			OutVerts[3]	= math::Vector( AABB.max.x, AABB.min.y );

			OutNorms[0]	= math::Vector( -1.f, +0.f );
			OutNorms[1]	= math::Vector( +0.f, +1.f );
			OutNorms[2]	= math::Vector( +1.f, +0.f );
			OutNorms[3]	= math::Vector( +0.f, -1.f );
		}
		else
		{
			// OOB Body.
			math::Vector Size2	= Body->Size * 0.5f;
			math::Coords ToLocal = Body->ToLocal();

			math::Vector XAxis = ToLocal.xAxis * Size2.x,
					YAxis = ToLocal.yAxis * Size2.y;

			OutNum		= 4;
			OutVerts[0]	= Body->Location - YAxis - XAxis;
			OutVerts[1]	= Body->Location + YAxis - XAxis;
			OutVerts[2]	= Body->Location + YAxis + XAxis;
			OutVerts[3]	= Body->Location - YAxis + XAxis;

			OutNorms[0]	= -ToLocal.xAxis;
			OutNorms[1]	= +ToLocal.yAxis;
			OutNorms[2]	= +ToLocal.xAxis;
			OutNorms[3]	= -ToLocal.yAxis;
		}
	}
}


//
// Mix object's friction.
//
inline Float MixFriction( Float AFric, Float BFric )
{
	return math::sqrt( AFric*AFric + BFric*BFric );
}


//
// Calculate body's mass.
//
Float CalculateMass( math::Vector* Verts, Int32 NumVers, Float Density )
{
	Float Mass	= 0.f;

	for( Int32 j=NumVers-1, i=0; i<NumVers; j=i, i++ )
	{
		math::Vector	P0	= Verts[j];
		math::Vector	P1	= Verts[i];
		Mass += abs(P0 / P1);
	}

	return Mass * Density * 0.5f;
}


//
// Calculate body's inertia.
//
Float CalculateInertia( math::Vector* Verts, Int32 NumVers, Float Mass )
{
	Float	Denom	= 0.f,
			Numer	= 0.f;

	for( Int32 j=NumVers-1, i=0; i<NumVers; j=i, i++ )
	{
		math::Vector	P0	= Verts[j];
		math::Vector	P1	= Verts[i];

		Float	A	= abs(P0 / P1);
		Float	B	= P1*P1 + P1*P0 + P0*P0;

		Denom	+= A * B;
		Numer	+= A;
	}

	return (Mass / 6.f) * (Denom/Numer);
}


//
// Object's mass comparison.
//
int MassCompare( const void* Arg1, const void* Arg2 )
{
	FBaseComponent*		Obj1	= *(FBaseComponent**)Arg1;
	FBaseComponent*		Obj2	= *(FBaseComponent**)Arg2;
	FPhysicComponent*	P1		= As<FPhysicComponent>(Obj1);
	FPhysicComponent*	P2		= As<FPhysicComponent>(Obj2);

	Float Mass1	= P1 ? P1->Mass : INFINITE_MASS;
	Float Mass2 = P2 ? P2->Mass : INFINITE_MASS;

	if( abs(Mass1-Mass2) < 0.5f )
	{
		return Obj1->Location.y < Obj2->Location.y;
	}
	else
		return Mass1 > Mass2;
}


/*-----------------------------------------------------------------------------
    Polygon overlap detection.
-----------------------------------------------------------------------------*/

//
// Project poly onto axis. Return bounds.
//
void ProjectPoly( const math::Vector& Axis, math::Vector* Verts, Int32 Num, Float& OutMin, Float& OutMax )
{
	OutMin	= OutMax	= Verts[0] * Axis;
	for( Int32 i=1; i<Num; i++ )
	{
		Float Test = Verts[i] * Axis;
		OutMin	= min( Test, OutMin );
		OutMax	= max( Test, OutMax );
	}
}


//
// Test an axis for SAT.
// Return true if poly's projections are overlap.
//
Bool TestAxis( const math::Vector& Axis, math::Vector* AVerts, Int32 ANum, math::Vector* BVerts, Int32 BNum )
{
	Float MinA, MinB, MaxA, MaxB;
	ProjectPoly( Axis, AVerts, ANum, MinA, MaxA );
	ProjectPoly( Axis, BVerts, BNum, MinB, MaxB );

	return (MinA < MaxB) && (MinB < MaxA);

}


//
// Figure out is two polys overlap.
// Don't compute hit info, just return
// logical value.
//
Bool PolysIsOverlap( math::Vector* AVerts, Int32 ANum, math::Vector* BVerts, Int32 BNum )
{
	math::Vector P1, P2, Axis;

	// Test edges of A poly.
	P1	= AVerts[ANum-1];
	for( Int32 i=0; i<ANum; i++ )
	{
		P2		= AVerts[i];
		Axis	= (P2 - P1).cross();

		if( !TestAxis( Axis, AVerts, ANum, BVerts, BNum ) )
			return false;

		P1	= P2;
	}

	// Test edges of B poly.
	P1	= BVerts[BNum-1];
	for( Int32 i=0; i<BNum; i++ )
	{
		P2		= BVerts[i];
		Axis	= (P2 - P1).cross();

		if( !TestAxis( Axis, AVerts, ANum, BVerts, BNum ) )
			return false;

		P1	= P2;
	}

	// They are overlap.
	return true;
}


/*-----------------------------------------------------------------------------
    Polygon collision detection.
-----------------------------------------------------------------------------*/

//
// Get the farthest point along dir vecctor.
//
math::Vector GetSupport( math::Vector Dir, math::Vector* Verts, Int32 NumVerts )
{
	math::Vector	Result		= Verts[0];
	Float	BestDist	= Verts[0] * Dir;

	for( Int32 i=1; i<NumVerts; i++ )
	{
		Float	TestDist	= Verts[i] * Dir;

		if( TestDist > BestDist )
		{
			Result		= Verts[i];
			BestDist	= TestDist;
		}
	}

	return Result;
}


//
// Figure out face with a least penetration (time).
//
Float FindAxisLeastTime
(
	math::Vector* AVerts, math::Vector* ANorms, Int32 ANum,
	math::Vector* BVerts, math::Vector* BNorms, Int32 BNum,
	Int32& Index
)
{
	Float Result = -999999.0;

	for( Int32 i=0; i<ANum; i++ )
	{
		// Normal for projection.
		math::Vector Normal	= ANorms[i];

		// Get support point being normal.
		math::Vector S		= GetSupport( -Normal, BVerts, BNum );
		Float	Time	= Normal * (S - AVerts[i]);

		if( Time > Result )
		{
			Result	= Time;
			Index	= i;
		}
	}

	return Result;
}


void FindIncidentFace
(
	math::Vector* V,
	math::Vector* RefVerts, math::Vector* RefNorms, Int32 NumRef,
	math::Vector* IncVerts, math::Vector* IncNorms, Int32 NumInc,
	Int32 RefIndex
)
{
	math::Vector	RefNormal	= RefNorms[RefIndex];
	Int32	IncFace		= 0;
	Float	MinDot		= 999999.0;

	for( Int32 i=0; i<NumInc; i++ )
	{
		Float	Dot	= RefNormal * IncNorms[i];

		if( Dot < MinDot )
		{
			MinDot	= Dot;
			IncFace	= i;
		}
	}

	// Set vertices.
	V[0]	= IncVerts[IncFace];
	V[1]	= IncVerts[(IncFace+1) % NumInc];
}


Int32 Clip( math::Vector N, Float C, math::Vector* Face )
{
	Int32	Result		= 0;
	math::Vector	OutFace[2]	= { Face[0], Face[1] };

	Float	D1	= N * Face[0] - C;
	Float	D2	= N * Face[1] - C;

	if( D1 <= 0.f )
	{
		OutFace[Result]	= Face[0];
		Result++;
	}
	if( D2 <= 0.f )
	{
		OutFace[Result]	= Face[1];
		Result++;
	}

	if( D1*D2 < 0.f )
	{
		Float	Alpha	= D1 / (D1-D2);
		OutFace[Result]	= Face[0] + (Face[1]-Face[0])*Alpha;
		Result++;
	}

	Face[0]	= OutFace[0];
	Face[1]	= OutFace[1];

	return Result;
}


/*-----------------------------------------------------------------------------
    Collision detection functions.
-----------------------------------------------------------------------------*/

//
// Compute arcade collision info for body. Return false
// if bodies are not collided, otherwise return true, and set up
// the collision info.
//
Bool CPhysics::DetectArcadeCollision( EAxis Axis, FPhysicComponent* Body, FBaseComponent* Other )
{	
	if( Other->IsA(FBrushComponent::MetaClass) )
	{
		// Detect aabb collision with brush.		
		math::Vector				P1, P2, TestNormal, TestSlope;
		FBrushComponent*	Brush		= (FBrushComponent*)Other;		
		Bool				Result		= false;
		Float				TestTime	= 1000.f;
		math::Rect			BodyRect	= Body->GetAABB();

		HitTime							= 500.f;
		HitNormal						= math::Vector( 0.f, 0.f );

		P1	= Brush->Vertices[Brush->NumVerts-1] + Brush->Location;
		for( Int32 i=0; i<Brush->NumVerts; i++ )
		{
			P2				= Brush->Vertices[i] + Brush->Location;
			math::Vector	Normal	= ( P2 - P1 ).cross();
			Float ExtraTime = 0.f;
			TestTime		= 1000.f;

			// Edge bounds.
			math::Rect Edge;
			Edge.min.x	= min( P1.x, P2.x );
			Edge.min.y	= min( P1.y, P2.y );
			Edge.max.x	= max( P1.x, P2.x );
			Edge.max.y	= max( P1.y, P2.y );
			
			if( Axis == AXIS_X && (Edge.max.x-Edge.min.x)<math::EPSILON )
			{
				// Flat vertical surface.
				if	( 
						BodyRect.min.x	< Edge.min.x &&
						BodyRect.max.x	> Edge.max.x &&
						BodyRect.max.y	> Edge.min.y &&
						BodyRect.min.y	< Edge.max.y
					)
				{
					if( Normal.x > 0.f )
					{
						TestNormal		= math::Vector( +1.f, 0.f );
						TestSlope		= TestNormal;
						TestTime		= Edge.max.x - BodyRect.min.x;
					}
					else
					{
						TestNormal		= math::Vector( -1.f, 0.f );
						TestSlope		= TestNormal;
						TestTime		= BodyRect.max.x - Edge.max.x;
					} 
				}
			}
			else if( Axis == AXIS_Y && (Edge.max.y-Edge.min.y)<math::EPSILON )
			{
				// Flat horizontal surface.
				if	(
						BodyRect.min.y	< Edge.min.y &&
						BodyRect.max.y	> Edge.max.y &&
						BodyRect.max.x	> Edge.min.x &&
						BodyRect.min.x	< Edge.max.x
					)
				{
					if( Normal.y > 0.f )
					{
						TestNormal		= math::Vector( 0.f, +1.f );
						TestSlope		= TestNormal;
						TestTime		= Edge.max.y - BodyRect.min.y;
					}
					else
					{
						TestNormal		= math::Vector( 0.f, -1.f );
						TestSlope		= TestNormal;
						TestTime		= BodyRect.max.y - Edge.max.y;
					}
				}
			}
			else if	( 
						Body->Velocity.y*Normal.y <= 0.f && 
						Edge.isOverlap( BodyRect ) && 
						math::pointLineDistance( Body->Location, P1, Normal ) >= 0.f 
					)
			{			
				// Slope surface.
				Normal.normalize();

				if( Normal.y > 0.f )
				{
					// I or II quadrant.
					if( Normal.x > 0.f )
					{
						// I'st quadrant.
						Float X			= clamp( BodyRect.min.x, Edge.min.x, Edge.max.x );		
						Float YSlope	= Edge.max.y + ((Edge.max.y-Edge.min.y)*(Edge.min.x-X)) / (Edge.max.x-Edge.min.x);	
							
						if( YSlope > BodyRect.min.y )
						{
							TestNormal		= math::Vector( 0.f, +1.f );
							TestSlope		= Normal;
							TestTime		= YSlope - BodyRect.min.y;
						}
						if( X != BodyRect.min.x )	
							ExtraTime	= 5.f;
					}
					else
					{
						// II'nd quadrant.
						Float X			= clamp( BodyRect.max.x, Edge.min.x, Edge.max.x );		
						Float YSlope	= Edge.min.y + ((X-Edge.min.x)*(Edge.max.y-Edge.min.y)) / (Edge.max.x-Edge.min.x);	
						
						if( YSlope > BodyRect.min.y )
						{
							TestNormal		= math::Vector( 0.f, +1.f );
							TestSlope		= Normal;
							TestTime		= YSlope - BodyRect.min.y;
						}
						if( X != BodyRect.max.x )	
							ExtraTime	= 5.f;
					}
				}
				else
				{
					// III or IV quadrant.
					if( Normal.x > 0.f )
					{
						// IV'th quadrant.
						Float X			= clamp( BodyRect.min.x, Edge.min.x, Edge.max.x );	
						Float YSlope	= Edge.min.y + ((X-Edge.min.x)*(Edge.max.y-Edge.min.y)) / (Edge.max.x-Edge.min.x);	

						if( YSlope < BodyRect.max.y )
						{
							TestNormal		= math::Vector( 0.f, -1.f );
							TestSlope		= Normal;
							TestTime		= BodyRect.max.y - YSlope;
						}
						if( X != BodyRect.min.x )	
							ExtraTime	= 5.f;
					}
					else
					{
						// III'rd quadrant.
						Float X			= clamp( BodyRect.max.x, Edge.min.x, Edge.max.x );		
						Float YSlope	= Edge.max.y + ((Edge.max.y-Edge.min.y)*(Edge.min.x-X)) / (Edge.max.x-Edge.min.x);

						if( YSlope < BodyRect.max.y )
						{
							TestNormal		= math::Vector( 0.f, -1.f );
							TestSlope		= Normal;
							TestTime		= BodyRect.max.y - YSlope;
						}
						if( X != BodyRect.max.x )	
							ExtraTime	= 5.f;
					}
				}
			}

			// Select least time.
			if( TestTime+ExtraTime < HitTime )
			{
				HitNormal	= TestNormal;
				HitTime		= TestTime;
				HitSlope	= TestSlope;
				Result		= true;				
			}

			P1		= P2;
		}
		
		return Result;
	}
	else if( !Other->IsA(FRigidBodyComponent::MetaClass) )
	{
		// Detect aabb collision.
		math::Rect BodyRect		= Body->GetAABB();
		math::Rect OtherRect	= Other->GetAABB();

		// Direction from Other to Body.
		Float	BodyEx, OtherEx, OverlapX, OverlapY;
		math::Vector Dir			= Body->Location - Other->Location;

		BodyEx		= 0.5f * (BodyRect.max.x - BodyRect.min.x);
		OtherEx		= 0.5f * (OtherRect.max.x - OtherRect.min.x);

		OverlapX	= BodyEx + OtherEx - abs(Dir.x);

		if( OverlapX > 0.f )
		{
			BodyEx		= 0.5f * (BodyRect.max.y - BodyRect.min.y);
			OtherEx		= 0.5f * (OtherRect.max.y - OtherRect.min.y);

			OverlapY	= BodyEx + OtherEx - abs(Dir.y);

			if( OverlapY > 0.f )
			{
				// Figure out the axis with least penetration.
				if( OverlapX < OverlapY )
				{
					HitNormal	= math::Vector( Dir.x<0.f ? -1.f : +1.f, 0.f );
					HitTime		= OverlapX;
					HitSlope	= HitNormal;
					return Axis == AXIS_X || Axis == AXIS_None;
				}
				else
				{
					HitNormal	= math::Vector( 0.f, Dir.y<0.f ? -1.f : +1.f );
					HitTime		= OverlapY;
					HitSlope	= HitNormal;
					return Axis == AXIS_Y || Axis == AXIS_None;
				}
			}
				return false;
		}
			return false;		
	}
	else
	{
		// Detect AABB collision with OOB(complex physics object).
		// Let's complex physics do this job, arcade physics
		// can't handle it properly.
		//
		// But we should let Rigid Body do work. Awake rigid if any.
		FRigidBodyComponent* Rigid	= (FRigidBodyComponent*)Other;
		Rigid->bSleeping			= false;
		return false;
	}
}


//
// Compute complex collision info for body's polygons.
// Return false if bodies doesn't collided, otherwise return true,
// and set up the collision info.
//
Bool CPhysics::DetectComplexCollision()
{
	NumConts	= 0;
	HitTime		= 0.f;

	Int32 FaceA, FaceB;

	// Check for SAP with A planes.
	Float TimeA	= FindAxisLeastTime
	(
		AVerts, ANorms, ANum,
		BVerts, BNorms, BNum,
		FaceA
	);

	if( TimeA >= 0.f )
		return false;

	// Check for SAP with B planes.
	Float TimeB	= FindAxisLeastTime
	(
		BVerts, BNorms, BNum,
		AVerts, ANorms, ANum,
		FaceB
	);

	if( TimeB >= 0.f )
		return false;

	Bool	bFlip;
	Int32	RefInd;
	if( TimeA > TimeB )
	{
		RefInd	= FaceA;
		bFlip	= false;
	}
	else
	{
		RefInd	= FaceB;
		bFlip	= true;
	}

	// Get incident face.
	math::Vector IncFace[2];
	if( !bFlip )
		FindIncidentFace
		(
			IncFace,
			AVerts, ANorms, ANum,
			BVerts, BNorms, BNum,
			RefInd
		);
	else
		FindIncidentFace
		(
			IncFace,
			BVerts, BNorms, BNum,
			AVerts, ANorms, ANum,
			RefInd
		);

	// Set refernce face.
	math::Vector V1, V2;
	if( !bFlip )
	{
		V1	= AVerts[RefInd];
		V2	= AVerts[(RefInd+1) % ANum];
	}
	else
	{
		V1	= BVerts[RefInd];
		V2	= BVerts[(RefInd+1) % BNum];
	}

	math::Vector PlaneNormal = V2 - V1;
	PlaneNormal.normalize();
	math::Vector	RefNormal	= PlaneNormal.cross();

	Float	RefC	= RefNormal * V1;
	Float	NegSide	= -(PlaneNormal * V1);
	Float	PosSide	= +(PlaneNormal * V2);

	// Clip incident face to reference face.
	if( Clip( -PlaneNormal, NegSide, IncFace ) < 2 )
		return false;

	if( Clip( PlaneNormal, PosSide, IncFace ) < 2 )
		return false;

	// Flip if any.
	HitNormal	= bFlip ? -RefNormal : +RefNormal;

	Int32 Cp	= 0;
	Float	Sep	= (RefNormal * IncFace[0]) - RefC;
	if( Sep <= 0.f )
	{
		Contacts[Cp++]	= IncFace[0];
		HitTime	= -Sep;
	}
	else
		HitTime	= 0.f;

	Sep	= (RefNormal * IncFace[1]) - RefC;
	if( Sep <= 0.f )
	{
		Contacts[Cp++]	= IncFace[1];
		HitTime	-=	Sep;
		HitTime	/= Cp;
	}

	NumConts	= Cp;
	return Cp > 0;
}


/*-----------------------------------------------------------------------------
    Top physics functions.
-----------------------------------------------------------------------------*/

//
// Complex physics - handles rotation, friction, restitution,
// portals, touches compute forces and so on. It's pretty
// expensive, so don't use it too often.
//
void CPhysics::PhysicComplex( FPhysicComponent* Body, Float Delta )
{
	// Don't process unmovable body.
	if( Body->Mass <= 0.f )
		return;

	// Setup pointers.
	Level	= Body->Level;

	// Unhash body to perform movement.
	// And return to it back.
	Level->CollHash->RemoveFromHash( Body );
	{
		// Prepare.
		FZoneComponent*	DetectedZone	= nullptr;
		math::Vector	OldLocation		= Body->Location;
		BodyInvMass		= GetInvMass(Body);
		BodyInvIner		= GetInvInertia(Body);

		// Integrate translate forces.
		Body->Velocity	+=	Body->Forces * (BodyInvMass * Delta);
		Body->Forces	=	math::Vector( 0.f, 0.f );

		//
		// Here we clamp delta, to avoid very long distances.
		// It's reduce situations when body fall, or pass
		// through obstacles. I think max delta it's 95%
		// of body's size.
		//
		math::Vector VelDelta	= Body->Velocity * Delta;
		VelDelta.x	= clamp( VelDelta.x, -Body->Size.x*0.95f, +Body->Size.x*0.95f );
		VelDelta.y	= clamp( VelDelta.y, -Body->Size.y*0.95f, +Body->Size.y*0.95f );

		Body->Location	+= VelDelta;

		// Integrate rotation forces.
		Body->AngVelocity	+=	Body->Torque * (BodyInvIner * Delta);
		Body->Rotation		+=	math::Angle( Body->AngVelocity * Delta );
		Body->Torque		=	0.f;

		// Get list of potential collide bodies, using cheap
		// AABB test.
		math::Rect OtherAABB, BodyAABB = Body->GetAABB();
		Level->CollHash->GetOverlapped( BodyAABB, NumOthers, Others );

		// Sort list of objects's for proper processing order.
		qsort( Others, NumOthers, sizeof(FBaseComponent*), MassCompare );

		// Convert Body to polygon.
		math::Vector	PolyOrig	= Body->Location;
		BodyToPoly( Body, AVerts, ANorms, ANum );

		// Test collision with all bodies.
		for( Int32 i=0; i<NumOthers; i++ )
		{
			// Figure out is body still overlaps with
			// Others[i] due position correction.
			Other		=	Others[i];
			BodyAABB	=	Body->GetAABB();
			OtherAABB	=	Other->GetAABB();

			if( !BodyAABB.isOverlap( OtherAABB ) )
				continue;

			// See if body already touch other.
			if( IsTouch( Body, Other ) )
				continue;

			// Polys for collision.
			BodyToPoly( Other, BVerts, BNorms, BNum );
			if( Body->Location != PolyOrig )
				BodyToPoly( Body, AVerts, ANorms, ANum );

			// Compute collsion.
			DetectComplexCollision();
			HitSide	= OppositeSide(NormalToSide(HitNormal));

			// No collision?
			if( NumConts == 0 )
				continue;

			// Handle zones.
			if( Other->IsA(FZoneComponent::MetaClass) )
			{
				DetectedZone	= (FZoneComponent*)Other;
				continue;
			}

			// Compute other's physics properties.
			FPhysicComponent* PhysOther = nullptr;
			if( Other->IsA(FPhysicComponent::MetaClass) )
			{
				PhysOther		= (FPhysicComponent*)Other;
				OtherInvMass	= GetInvMass(PhysOther);
				OtherInvIner	= GetInvInertia(PhysOther);
			}
			else
			{
				OtherInvMass	= 0.f;
				OtherInvIner	= 0.f;
			}
			FRigidBodyComponent* RigidOther	= As<FRigidBodyComponent>(PhysOther);

			// Ask script how to handle hit.
			Solution	= HSOL_None;
			bBrake		= false;

			Body->Entity->OnCollide( Other->Entity, HitSide );
			Other->Entity->OnCollide( Body->Entity, OppositeSide(HitSide) );

			if
				(
					(Solution == HSOL_Solid) ||
					(Solution == HSOL_Oneway && HitSide == HSIDE_Top && Body->Velocity.y < 0.f)
				)
			{
				// Handle solid collision.

				// Process REAL physics interaction.
				Float SFriction	= !PhysOther ? GMaterials[Body->Material].SFriction : MixFriction
				(
					GMaterials[Body->Material].SFriction,
					GMaterials[PhysOther->Material].SFriction
				);
				Float DFriction	= !PhysOther ? GMaterials[Body->Material].DFriction : MixFriction
				(
					GMaterials[Body->Material].DFriction,
					GMaterials[PhysOther->Material].DFriction
				);

				// Awake other if any.
				if( RigidOther )
					RigidOther->bSleeping	= false;

				Float	AV1 = 0.f, 
						AV2 = 0.f;
				math::Vector	V1 = math::Vector( 0.f, 0.f ), 
						V2 = math::Vector (0.f, 0.f );

				for( Int32 k=0; k<NumConts; k++ )
				{
					// Radii vectors.
					math::Vector RadBody	= Contacts[k] - Body->Location;
					math::Vector RadOther	= Contacts[k] - Other->Location;

					// Relative velocity.
					math::Vector RelVel;
					if( PhysOther )
						RelVel	=	PhysOther->Velocity + (RadOther / PhysOther->AngVelocity) -
									Body->Velocity		- (RadBody / Body->AngVelocity);
					else
						RelVel	= -Body->Velocity - (RadBody / Body->AngVelocity);

					// Project velocity along hit normal.
					Float ProjVel	= RelVel * HitNormal;

					// Collide only when velocity along hit normal
					// and satisfied script's solution about this hit.
					if 
						(
							(ProjVel <= 0.f) &&
							(	( Solution == HSOL_Solid )||
								( Solution == HSOL_Oneway && HitSide == HSIDE_Top ) )
						)
					{
						Float	RACrossN	= RadBody / HitNormal;
						Float	RBCrossN	= RadOther / HitNormal;

						Float	InvMassTotal	=	BodyInvMass + OtherInvMass +
													sqr(RACrossN) * BodyInvIner +
													sqr(RBCrossN) * OtherInvIner;

						// Pick elasticity for hit solving.
						Float	e	=	!PhysOther ? GMaterials[Body->Material].Elasticity :
										min( GMaterials[Body->Material].Elasticity, GMaterials[PhysOther->Material].Elasticity );

						// Scalar impulse.
						Float	j	= -(1 + e) * ProjVel / (InvMassTotal*NumConts);
						math::Vector	TotalImpulse	= HitNormal * j;

						// Apply impulse friction.
						math::Vector	Tangent	= RelVel - (HitNormal * (RelVel*HitNormal));
						Tangent.normalize();
						Float jt	= -(RelVel * Tangent) / (InvMassTotal*NumConts);

						// Don't apply too small friction.
						if( ( jt <= -0.001f )||( jt >= +0.001f ) )
						{
							if( abs(jt) < j*SFriction )
							{
								// Static friction.
								TotalImpulse	+=	Tangent * jt;
							}
							else
							{
								// Dynamic friction.
								TotalImpulse	+=	Tangent * (-j*DFriction);
							}
						}

						// Apply sum of impulses to bodies movement.
						V1 += TotalImpulse * BodyInvMass;
						if( PhysOther )
							V2	+= TotalImpulse * OtherInvMass;

						// Apply sum of impulses to bodies rotation.
						AV1 += (RadBody/TotalImpulse)*BodyInvIner;
						if( PhysOther )
							AV2 += (RadOther/TotalImpulse)*OtherInvIner;
					}
				}

				// Apply sum of impulses to bodies movement.
				Body->Velocity -= V1;
				if( PhysOther )
				{
					//if( PhysOther->IsA(FArcadeBodyComponent::MetaClass) )
					//{
					//	PhysOther->Velocity.X += V2.X;
					//}
					//else
						PhysOther->Velocity	+= V2;
				}

				// Apply sum of impulses to bodies rotation.
				Body->AngVelocity -= AV1;
				if( PhysOther )
					PhysOther->AngVelocity += AV2;

				// Apply correction to bodies location.
				math::Vector Correct =	HitNormal * PHYS_PENET_PERCENT * 
									(max( 0.f, HitTime-PHYS_PENET_ALLOW )/(BodyInvMass+OtherInvMass)); 

				Body->Location -= Correct * BodyInvMass;

				if( PhysOther )
				{
					// Don't let sink.
					Level->CollHash->RemoveFromHash(PhysOther);
					PhysOther->Location += Correct * OtherInvMass;
					Level->CollHash->AddToHash(PhysOther);
				}

				// Handle floor.
				if( HitSide == HSIDE_Top )
				{
					// Body get floor slab.
					FMoverComponent* Mover = As<FMoverComponent>(Other);
					if( Mover )
						Mover->AddRider( Body );
					Body->Floor		= Other->Entity;
				}
				else if( HitSide == HSIDE_Bottom )
				{
					// Other get floor slab.
					if( Other->IsA(FPhysicComponent::MetaClass) )
						((FPhysicComponent*)Other)->Floor	= Body->Entity;
				}
			}
			else
			{
				// Bodies don't want to collide, so
				// touch 'em.
				if( Solution != HSOL_Oneway )
				{
					BeginTouch( Body, Other );
					BodyAABB = Body->GetAABB();
				}
			}
		}

		//
		// See if no more touch touched actors.
		//
		{
			BodyToPoly( Body, AVerts, ANorms, ANum );
			for( Int32 i=0; i<arraySize(Body->Touched); i++ )
				if( Body->Touched[i] )
				{
					Other	= Body->Touched[i]->Base;
					BodyToPoly( Other, BVerts, BNorms, BNum );

					if( !PolysIsOverlap( AVerts, ANum, BVerts, BNum ) )
						EndTouch( Body, Other );
				}
		}

		// Process zone.
		SetBodyZone( Body, DetectedZone );

		// Process portal pass.
		HandlePortals( Body, OldLocation );	
		
		// Handle sleeping of rigid.
		// Not each frame, let them heat up for begging.
		if( !((GFrameStamp ^ Body->GetId()) & 63) )
		{
			if( abs(Body->AngVelocity) <= 0.1f )
				Body->AngVelocity	= 0.f;
			if( Body->Velocity.sizeSquared() <= SLEEP_THRESHOLD )
				Body->Velocity	= math::Vector( 0.f, 0.f );
		}

		FRigidBodyComponent* Rigid	= (FRigidBodyComponent*)Body;
		if( Rigid->bCanSleep )
		{
			if	( 
					Body->AngVelocity == 0.f && 
					Body->Velocity == math::Vector( 0.f, 0.f ) &&
					Body->Floor
				)
			{
				// Sweet dreams are made of this!
				Rigid->bSleeping	= true;		
			}
		}
	}
	Level->CollHash->AddToHash( Body );		
}


//
// Arcade physics computation.
// No friction, no rotation. Perfectly for
// player figures.
//
void CPhysics::PhysicArcade( FPhysicComponent* Body, Float Delta )
{
	// Setup pointers.
	Level	= Body->Level;

	// Unhash body to perform movement.
	// And return to it back.
	Level->CollHash->RemoveFromHash( Body );
	{
		// Prepare.
		FZoneComponent*	DetectedZone	= nullptr;
		math::Vector	OldLocation		= Body->Location;

		//
		// Here we clamp delta, to avoid very long distances.
		// It's reduce situations when body fall, or pass
		// through obstacles. I think max delta it's 95%
		// of body's size.
		//
		math::Vector VelDelta	= Body->Velocity * Delta;
		VelDelta.x	= clamp( VelDelta.x, -Body->Size.x*0.95f, +Body->Size.x*0.95f );
		VelDelta.y	= clamp( VelDelta.y, -Body->Size.y*0.95f, +Body->Size.y*0.95f );

		//
		// Perform X-Movement.
		//
		{
			Body->Location.x	+= VelDelta.x;

			// Get list of potential collide bodies, using cheap
			// AABB test.
			math::Rect BodyAABB = Body->GetAABB();
			Level->CollHash->GetOverlapped( BodyAABB, NumOthers, Others );

			// Test collision with all actors.
			for( Int32 iOther=0; iOther<NumOthers; iOther++ )
			{
				// Figure out is body still overlaps with
				// Others[i] due position correction.
				Other		= Others[iOther];
	
				// See if body already touch other.
				if( IsTouch( Body, Other ) )
					continue;

				// Handle zones.
				if( Other->IsA(FZoneComponent::MetaClass) && BodyAABB.isOverlap( Other->GetAABB() ) )
				{
					DetectedZone	= (FZoneComponent*)Other;
					continue;
				}

				// Figure out collision hit info.
				if( !DetectArcadeCollision( AXIS_X, Body, Other ) )
					continue;

				// Ask script how to handle hit.
				HitSide		= NormalToSide(HitNormal);
				Solution	= HSOL_None;
				bBrake		= false;
				Body->Entity->OnCollide( Other->Entity, HitSide );
				Other->Entity->OnCollide( Body->Entity, OppositeSide(HitSide) );

				// Process collision.
				if( Solution == HSOL_Solid )
				{
					// Handle solid collision.
					// Push up actor's location.
					Body->Location.x	+= HitNormal.x * HitTime;
					BodyAABB			= Body->GetAABB();

					// Brake velocity.
					if( bBrake && HitNormal.x != 0.f )
					{
						// Brake X-vector.
						if( HitSide == HSIDE_Left && Body->Velocity.x > 0.f )
							Body->Velocity.x	= 0.f;

						if( HitSide == HSIDE_Right && Body->Velocity.x < 0.f )
							Body->Velocity.x	= 0.f;
					}
				}
				else
				{
					// Bodies don't want to collide, so
					// touch 'em.
					if( Solution != HSOL_Oneway )
					{
						BeginTouch( Body, Other );
						BodyAABB = Body->GetAABB();
					}
				}
			}
		}

		//
		// Perform Y-Movement or stand, its required for floor and 
		// proper zone detection.
		//
		{
			Bool bNoMove = abs(VelDelta.y) <= math::EPSILON;
			Body->Location.y	+= VelDelta.y;

			// Get list of potential collide bodies, using cheap
			// AABB test.
			math::Rect BodyAABB = Body->GetAABB();
			Level->CollHash->GetOverlapped( BodyAABB, NumOthers, Others );

			// Test collision with all actors.
			for( Int32 iOther=0; iOther<NumOthers; iOther++ )
			{
				// Figure out is body still overlaps with
				// Others[i] due position correction.
				Other		= Others[iOther];

				// See if body already touch other.
				if( IsTouch( Body, Other ) )
					continue;

				// Handle zones.
				if( Other->IsA(FZoneComponent::MetaClass) && BodyAABB.isOverlap( Other->GetAABB() ) )
				{
					DetectedZone	= (FZoneComponent*)Other;
					continue;
				}

				// Figure out collision hit info.
				if( !DetectArcadeCollision( bNoMove ? AXIS_None : AXIS_Y, Body, Other ) )
					continue;

				// Ask script how to handle hit.
				HitSide		= NormalToSide(HitNormal);
				Solution	= HSOL_None;
				bBrake		= false;
				Body->Entity->OnCollide( Other->Entity, HitSide );
				Other->Entity->OnCollide( Body->Entity, OppositeSide(HitSide) );

				// Process collision.
				if	(	
						( Solution == HSOL_Solid ) ||
						( Solution == HSOL_Oneway && HitSide == HSIDE_Top && Body->Velocity.y < 0.f )
					)
				{
					// Handle solid collision.
					// Push up actor's location.
					Body->Location.y	+= HitNormal.y * HitTime;
					BodyAABB			= Body->GetAABB();

					// Brake velocity, but not for slopes surfaces.
					if( bBrake && HitSlope == HitNormal )
					{
						// Brake Y-vector.
						if( HitSide == HSIDE_Bottom && Body->Velocity.y > 0.f )
							Body->Velocity.y	= 0.f;

						if( HitSide == HSIDE_Top && Body->Velocity.y < 0.f )
							Body->Velocity.y	= 0.f;
					}

					// Handle floor.
					if( HitSide == HSIDE_Top )
					{
						// Body get floor slab.
						FMoverComponent* Mover = As<FMoverComponent>(Other);
						if( Mover )
							Mover->AddRider( Body );
						Body->Floor		= Other->Entity;
					}
					else if( HitSide == HSIDE_Bottom )
					{
						// Other get floor slab.
						if( Other->IsA(FPhysicComponent::MetaClass) )
							((FPhysicComponent*)Other)->Floor	= Body->Entity;
					}
				}
				else
				{
					// Bodies don't want to collide, so
					// touch 'em.
					if( Solution != HSOL_Oneway )
					{
						BeginTouch( Body, Other );
						BodyAABB = Body->GetAABB();
					}
				}
			}
		}

		//
		// See if no more touch touched actors.
		//
		{
			math::Rect ThisRect = Body->GetAABB();

			for( Int32 i=0; i<arraySize(Body->Touched); i++ )
				if( Body->Touched[i]  )
				{
					FBaseComponent* Other = Body->Touched[i]->Base;
					if	( 
							!Other->IsA(FRigidBodyComponent::MetaClass) &&
							!Other->IsA(FBrushComponent::MetaClass)
						)
					{
						math::Rect OtherRect = Other->GetAABB();
						if( !ThisRect.isOverlap( OtherRect ) )
							EndTouch( Body, Other );
					}
				}
		}
		
		// Process zone.
		SetBodyZone( Body, DetectedZone );

		// Process portal pass.
		HandlePortals( Body, OldLocation );	
	}
	Level->CollHash->AddToHash( Body );			
}


//
// A keyframe interpolation polynomial.
// Make sure: f(0)=0 and f(1)=1.
//
inline Float Interpolate( Float X )
{
	return X;					
	//return X * X;	
	//return sinf(X)*0.5f + 0.5f;
}


//
// Keyframe gliding movement physics.
// Fake rotation, no friction or other forces
// even no collision detection, zones detection,
// portal passing and touch notification.
//
void CPhysics::PhysicKeyframe( FKeyframeComponent* Object, Float Delta )
{
	// Prepare.
	FLevel*			Level	= Object->Level;
	FBaseComponent*	Base	= Object->Base;

	// Unhash self to perform movement.
	// And return to it back.
	Level->CollHash->RemoveFromHash( Base );
	{
		// Store source info.
		math::Vector OldLocation	= Base->Location;
		math::Angle	OldRotation		= Base->Rotation;

		switch(	Object->GlideType )
		{
			case GLIDE_None:
			{
				// No movement.
				break;
			}
			case GLIDE_Forward:
			{
				// Perform forward movement.
				if( Object->Speed > 0.f )
				{
					// Forward movement.
					Int32 iPrevKey	= math::floor(Object->Progress);
					Int32 iDestKey	= iPrevKey + 1;

					if( iDestKey < Object->Points.size() )
					{
						// Continue gliding.
						Float PathLen = max( math::distance( Object->Points[iPrevKey].Location, Object->Points[iDestKey].Location ), 0.01f );
						Object->Progress += (Object->Speed / PathLen) * Delta;
						Float Alpha	  = Interpolate(Object->Progress-iPrevKey);
						Base->Location	= lerp
											( 
												Object->Points[iPrevKey].Location, 
												Object->Points[iDestKey].Location, 
												Alpha 
											);

						Base->Rotation	= AngleLerp
												( 
													Object->Points[iPrevKey].Rotation, 
													Object->Points[iDestKey].Rotation, 
													Alpha, 
													Object->Points[iDestKey].bCCW 
												);
					}
					else
					{
						// End reached.
						if( Object->bLooped )
						{
							// Reloop.
							Object->Speed		= -Object->Speed;
							Object->Progress	= Object->Points.size() - 1.f;
						}
						else
						{
							// Stop.
							Object->GlideType	= GLIDE_None;
							Object->Speed		= 0.f;
							Object->Progress	= 0.f;
						}
					}
				}
				else
				{
					// Backward movement.
					Int32 iPrevKey	= math::ceil(Object->Progress);
					Int32 iDestKey	= iPrevKey - 1;

					if( iDestKey >= 0 )
					{
						// Continue gliding.
						Float PathLen = max( math::distance( Object->Points[iDestKey].Location, Object->Points[iPrevKey].Location ), 0.01f );
						Object->Progress += (Object->Speed / PathLen) * Delta;
						Float Alpha	  = Interpolate(Object->Progress-iDestKey);
						Base->Location	= lerp
											( 
												Object->Points[iDestKey].Location, 
												Object->Points[iPrevKey].Location, 
												Alpha 
											);

						Base->Rotation	= AngleLerp
												( 
													Object->Points[iDestKey].Rotation, 
													Object->Points[iPrevKey].Rotation, 
													Alpha, 
													Object->Points[iDestKey].bCCW 
												);
					}
					else
					{
						// End reached.
						if( Object->bLooped )
						{
							// Reloop.
							Object->Speed		= -Object->Speed;
							Object->Progress	= 0.f;
						}
						else
						{
							// Stop.
							Object->GlideType	= GLIDE_None;
							Object->Speed		= 0.f;
							Object->Progress	= 0.f;
						}
					}
				}
				break;
			}
			case GLIDE_Target:
			{
				// Glide to the target key.
				Int32 iTarget		= Object->iTarget;
				math::Vector MoveFrom	= Object->StartLocation;
				Float PathLen		= max( math::distance
												( 
													MoveFrom, 
													Object->Points[iTarget].Location 
												), 
													0.01f );

				Object->Progress += (Object->Speed / PathLen) * Delta;

				if( Object->Progress <= 1.f )
				{
					// Continue glide.
					Float Alpha		= Interpolate( Object->Progress );
					Base->Location	= lerp( MoveFrom, Object->Points[iTarget].Location, Alpha );
				}
				else
				{
					// End reached.
					Object->Progress	= 1.f;
					Object->Speed		= 0.f;
					Object->GlideType	= GLIDE_None;	
				}
				break;
			}
		}
	}
	Level->CollHash->AddToHash( Base );
}


//
// Setup actor physics for current frame.
// Call it before major physics.
//
void CPhysics::SetupPhysics( FPhysicComponent* Body, Float Delta )
{
	// Reset a body floor.
	Body->Floor		= nullptr;
}


//
// Compute rigid mass and inertia from it material.
//
void CPhysics::ComputeRigidMaterial( FRigidBodyComponent* Rigid )
{
	assert(Rigid->Material != PM_Custom);

	Float	W	= Rigid->Size.x,
			H	= Rigid->Size.y,
			Rho	= GMaterials[Rigid->Material].Density;

	Rigid->Mass		= W * H * Rho;
	Rigid->Inertia	= (Rho * W * H * (W*W + H*H))/12.f;
}


/*-----------------------------------------------------------------------------
    Zones.
-----------------------------------------------------------------------------*/

//
// Set an body zone, return true if zone accepted and
// entity will got a script notification, if NewZone is already
// entity's zone return false.
//
Bool CPhysics::SetBodyZone( FPhysicComponent* Body, FZoneComponent* NewZone )
{
	if	(	
			( NewZone && NewZone->Entity != Body->Zone ) || 
			( !NewZone && Body->Zone != nullptr )
		)
	{
		// Enter new zone.
		Body->Zone	= NewZone ? NewZone->Entity : nullptr;
		Body->Entity->OnZoneChange();
		return true;
	}
	else
	{
		// Same zone.
		return false;
	}
}


/*-----------------------------------------------------------------------------
    Portals.
-----------------------------------------------------------------------------*/

//
// Handle portal features for this entity.
// during physics performing.
//
void CPhysics::HandlePortals( FPhysicComponent* Body, const math::Vector& OldLocation )
{
	assert(Body);
	assert(!Body->IsHashed());

	// Don't process portals, if no movement.
	if( Body->Location == OldLocation )
		return;

	for( FPortalComponent* Portal=Level->FirstPortal; Portal; Portal=Portal->NextPortal )
	{
		if( Portal->IsA(FWarpComponent::MetaClass) )
		{
			// Warping portal.
			FWarpComponent* Warp = (FWarpComponent*)Portal;
			if( !Warp->Other )
				continue;

			math::Coords WarpLocal	= Warp->ToLocal();
			math::Vector NewLocal	= math::transformPointBy( Body->Location,	WarpLocal );
			math::Vector OldLocal	= math::transformPointBy( OldLocation,		WarpLocal );

			// Test for passing.
			if	( 
					( NewLocal.x * OldLocal.x < 0.f ) &&
					( OldLocal.y >= -Warp->Width*0.5f ) &&
					( OldLocal.y <= +Warp->Width*0.5f )
				)
			{
				// Pass through mirror portal.
				Body->Entity->OnWarpPass(Warp->Entity);

				// Transfer object location & velocity.
				Body->Location		= Warp->TransferPoint( Body->Location );	
				Body->Velocity		= Warp->TransferVector( Body->Velocity );	

				break;
			}
		}
		else if( Portal->IsA(FMirrorComponent::MetaClass) )
		{
			// Mirror portal.
			FMirrorComponent* Mirror = (FMirrorComponent*)Portal;

			Float A	= OldLocation.x			- Mirror->Location.x;
			Float B = Body->Location.x		- Mirror->Location.x;

			// Mirror X-test.
			if( A*B < 0.f )
			{
				Float C = Mirror->Location.y	- Mirror->Width * 0.5f;
				Float D	= Mirror->Location.y	+ Mirror->Width * 0.5f;

				// Mirror Y-test.
				if( Body->Location.y >= C && Body->Location.y <= D )
				{
					// Pass through mirror portal.
					Body->Entity->OnMirrorPass(Mirror->Entity);

					// Flip forces.
					Body->Location		= Mirror->TransferPoint( Body->Location );
					Body->Velocity		= Mirror->TransferVector( Body->Velocity );
					Body->AngVelocity	= -Body->AngVelocity;

					break;
				}
			}
		}
		else
		{
			// Bad portal type.
			debug( L"Phys: Unsupported portal '%s'", *Portal->GetFullName() );
		}
	}
}


/*-----------------------------------------------------------------------------
    Touching.
-----------------------------------------------------------------------------*/

//
// Return true, if Body has Other in the list of touched 
// objects.
//
Bool CPhysics::IsTouch( FPhysicComponent* Body, FBaseComponent* Other )
{
	// Iterate through array.
	for( Int32 i=0; i<arraySize(Body->Touched); i++ )
		if( Body->Touched[i] == Other->Entity )
			return true;

	// Not found.
	return false;
}


//
// Makes entity touch. Return false if entity's are
// already touched or no available place found,
// otherwise return true.
//
Bool CPhysics::BeginTouch( FPhysicComponent* Body, FBaseComponent* Other )
{
	Int32 iA = -1, iB = -1;

	// Maybe already touch.
	if( IsTouch( Body, Other ) )
		return false;

	// Find avail slot in Object.
	for( Int32 i=0; i<arraySize(Body->Touched); i++ )
		if( Body->Touched[i] == nullptr )
		{
			iA	= i;
			break;
		}

	// Find avail slot in Other.
	FPhysicComponent* Phys = As<FPhysicComponent>( Other );
	if( Phys )
		for( Int32 i=0; i<arraySize(Phys->Touched); i++ )
			if( Phys->Touched[i] == nullptr )
			{
				iB	= i;
				break;
			}

	// There is avail.
	if( iA != -1 )
	{
		if( Phys )
		{
			if( iB == -1 )
				return false;

			// Touch to B.
			Phys->Touched[iB]	= Body->Entity;
		}

		// Touch to A.
		Body->Touched[iA]	= Other->Entity;
		Body->Entity->OnBeginTouch( Other->Entity );

		// Anyway notify other.
		Other->Entity->OnBeginTouch( Body->Entity );
	}
	else
	{
		// No avail place, skip touch :(
		return false;
	}

	return true;
}


//
// Notify entity's end of touch, and remove it from
// the touched list. Return true if valid entities was touched,
// otherwise return false.
//
Bool CPhysics::EndTouch( FPhysicComponent* Body, FBaseComponent* Other )
{
	Int32 iA = -1, iB = -1;

	for( Int32 i=0; i<arraySize(Body->Touched); i++ )
		if( Body->Touched[i] == Other->Entity )
		{
			iA	= i;
			break;
		}

	FPhysicComponent* Phys = As<FPhysicComponent>( Other );
	if( Phys )
		for( Int32 i=0; i<arraySize(Phys->Touched); i++ )
			if( Phys->Touched[i] == Body->Entity )
			{
				iB	= i;
				break;
			}

	if( iA != -1 )
	{
		// They are touched.
		Body->Touched[iA]	= nullptr;
		Body->Entity->OnEndTouch(Other->Entity);

		if( Phys )
		{
			assert(iB != -1);
			Phys->Touched[iB]	= nullptr;
		}
		// Anyway notify other.
		Other->Entity->OnEndTouch(Body->Entity);

		return true;
	}
	else
	{
		// No touching.
		assert(iB == -1);
		return false;
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/