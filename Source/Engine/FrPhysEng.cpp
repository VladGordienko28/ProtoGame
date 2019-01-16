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
TVector					CPhysics::HitNormal;
TVector					CPhysics::HitSlope;
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
TVector					CPhysics::AVerts[16];
TVector					CPhysics::ANorms[16];
TVector					CPhysics::BVerts[16];
TVector					CPhysics::BNorms[16];
Int32					CPhysics::ANum;
Int32					CPhysics::BNum;
TVector					CPhysics::Contacts[2];
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
inline EHitSide NormalToSide( const TVector& N )
{
	// About 45 degs.
	if( ( N.X < +0.7f ) && ( N.X > -0.7f ) )
	{
		// Bottom or Top.
		return N.Y > 0.f ? HSIDE_Top : HSIDE_Bottom;
	}
	else
	{
		// Left or Right.
		return N.X > 0.f ? HSIDE_Right : HSIDE_Left;
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
void BodyToPoly( FBaseComponent* Body, TVector* OutVerts, TVector* OutNorms, Int32& OutNum )
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
		TVector P1=Brush->Vertices[OutNum-1], P2;
		for( Int32 i=0, j=OutNum-1; i<OutNum; j=i, i++ )
		{
			P2			= Brush->Vertices[i];
			TVector N	= P2 - P1;
			N.Normalize();
			OutNorms[j]	= N.Cross();
			P1			= P2;
		}
	}
	else 
	{
		// Rectangle.
		if( Body->bFixedAngle || Body->Rotation.Angle == 0 )
		{
			// AABB body.
			TRect AABB( Body->Location, Body->Size );
			OutNum	= 4;

			OutVerts[0]	= TVector( AABB.Min.X, AABB.Min.Y );
			OutVerts[1]	= TVector( AABB.Min.X, AABB.Max.Y );
			OutVerts[2]	= TVector( AABB.Max.X, AABB.Max.Y );
			OutVerts[3]	= TVector( AABB.Max.X, AABB.Min.Y );

			OutNorms[0]	= TVector( -1.f, +0.f );
			OutNorms[1]	= TVector( +0.f, +1.f );
			OutNorms[2]	= TVector( +1.f, +0.f );
			OutNorms[3]	= TVector( +0.f, -1.f );
		}
		else
		{
			// OOB Body.
			TVector Size2	= Body->Size * 0.5f;
			TCoords ToLocal = Body->ToLocal();

			TVector XAxis = ToLocal.XAxis * Size2.X,
					YAxis = ToLocal.YAxis * Size2.Y;

			OutNum		= 4;
			OutVerts[0]	= Body->Location - YAxis - XAxis;
			OutVerts[1]	= Body->Location + YAxis - XAxis;
			OutVerts[2]	= Body->Location + YAxis + XAxis;
			OutVerts[3]	= Body->Location - YAxis + XAxis;

			OutNorms[0]	= -ToLocal.XAxis;
			OutNorms[1]	= +ToLocal.YAxis;
			OutNorms[2]	= +ToLocal.XAxis;
			OutNorms[3]	= -ToLocal.YAxis;
		}
	}
}


//
// Mix object's friction.
//
inline Float MixFriction( Float AFric, Float BFric )
{
	return Sqrt( AFric*AFric + BFric*BFric );
}


//
// Calculate body's mass.
//
Float CalculateMass( TVector* Verts, Int32 NumVers, Float Density )
{
	Float Mass	= 0.f;

	for( Int32 j=NumVers-1, i=0; i<NumVers; j=i, i++ )
	{
		TVector	P0	= Verts[j];
		TVector	P1	= Verts[i];
		Mass += Abs(P0 / P1);
	}

	return Mass * Density * 0.5f;
}


//
// Calculate body's inertia.
//
Float CalculateInertia( TVector* Verts, Int32 NumVers, Float Mass )
{
	Float	Denom	= 0.f,
			Numer	= 0.f;

	for( Int32 j=NumVers-1, i=0; i<NumVers; j=i, i++ )
	{
		TVector	P0	= Verts[j];
		TVector	P1	= Verts[i];

		Float	A	= Abs(P0 / P1);
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

	if( Abs(Mass1-Mass2) < 0.5f )
	{
		return Obj1->Location.Y < Obj2->Location.Y;
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
void ProjectPoly( const TVector& Axis, TVector* Verts, Int32 Num, Float& OutMin, Float& OutMax )
{
	OutMin	= OutMax	= Verts[0] * Axis;
	for( Int32 i=1; i<Num; i++ )
	{
		Float Test = Verts[i] * Axis;
		OutMin	= Min( Test, OutMin );
		OutMax	= Max( Test, OutMax );
	}
}


//
// Test an axis for SAT.
// Return true if poly's projections are overlap.
//
Bool TestAxis( const TVector& Axis, TVector* AVerts, Int32 ANum, TVector* BVerts, Int32 BNum )
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
Bool PolysIsOverlap( TVector* AVerts, Int32 ANum, TVector* BVerts, Int32 BNum )
{
	TVector P1, P2, Axis;

	// Test edges of A poly.
	P1	= AVerts[ANum-1];
	for( Int32 i=0; i<ANum; i++ )
	{
		P2		= AVerts[i];
		Axis	= (P2 - P1).Cross();

		if( !TestAxis( Axis, AVerts, ANum, BVerts, BNum ) )
			return false;

		P1	= P2;
	}

	// Test edges of B poly.
	P1	= BVerts[BNum-1];
	for( Int32 i=0; i<BNum; i++ )
	{
		P2		= BVerts[i];
		Axis	= (P2 - P1).Cross();

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
TVector GetSupport( TVector Dir, TVector* Verts, Int32 NumVerts )
{
	TVector	Result		= Verts[0];
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
	TVector* AVerts, TVector* ANorms, Int32 ANum,
	TVector* BVerts, TVector* BNorms, Int32 BNum,
	Int32& Index
)
{
	Float Result = -999999.0;

	for( Int32 i=0; i<ANum; i++ )
	{
		// Normal for projection.
		TVector Normal	= ANorms[i];

		// Get support point being normal.
		TVector S		= GetSupport( -Normal, BVerts, BNum );
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
	TVector* V,
	TVector* RefVerts, TVector* RefNorms, Int32 NumRef,
	TVector* IncVerts, TVector* IncNorms, Int32 NumInc,
	Int32 RefIndex
)
{
	TVector	RefNormal	= RefNorms[RefIndex];
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


Int32 Clip( TVector N, Float C, TVector* Face )
{
	Int32	Result		= 0;
	TVector	OutFace[2]	= { Face[0], Face[1] };

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
		TVector				P1, P2, TestNormal, TestSlope;
		FBrushComponent*	Brush		= (FBrushComponent*)Other;		
		Bool				Result		= false;
		Float				TestTime	= 1000.f;
		TRect				BodyRect	= Body->GetAABB();

		HitTime							= 500.f;
		HitNormal						= TVector( 0.f, 0.f );

		P1	= Brush->Vertices[Brush->NumVerts-1] + Brush->Location;
		for( Int32 i=0; i<Brush->NumVerts; i++ )
		{
			P2				= Brush->Vertices[i] + Brush->Location;
			TVector	Normal	= ( P2 - P1 ).Cross();
			Float ExtraTime = 0.f;
			TestTime		= 1000.f;

			// Edge bounds.
			TRect Edge;
			Edge.Min.X	= Min( P1.X, P2.X );
			Edge.Min.Y	= Min( P1.Y, P2.Y );
			Edge.Max.X	= Max( P1.X, P2.X );
			Edge.Max.Y	= Max( P1.Y, P2.Y );

			if( Axis == AXIS_X && (Edge.Max.X-Edge.Min.X)<EPSILON )
			{
				// Flat vertical surface.
				if	( 
						BodyRect.Min.X	< Edge.Min.X &&
						BodyRect.Max.X	> Edge.Max.X &&
						BodyRect.Max.Y	> Edge.Min.Y &&
						BodyRect.Min.Y	< Edge.Max.Y
					)
				{
					if( Normal.X > 0.f )
					{
						TestNormal		= TVector( +1.f, 0.f );
						TestSlope		= TestNormal;
						TestTime		= Edge.Max.X - BodyRect.Min.X;
					}
					else
					{
						TestNormal		= TVector( -1.f, 0.f );
						TestSlope		= TestNormal;
						TestTime		= BodyRect.Max.X - Edge.Max.X;
					} 
				}
			}
			else if( Axis == AXIS_Y && (Edge.Max.Y-Edge.Min.Y)<EPSILON )
			{
				// Flat horizontal surface.
				if	(
						BodyRect.Min.Y	< Edge.Min.Y &&
						BodyRect.Max.Y	> Edge.Max.Y &&
						BodyRect.Max.X	> Edge.Min.X &&
						BodyRect.Min.X	< Edge.Max.X
					)
				{
					if( Normal.Y > 0.f )
					{
						TestNormal		= TVector( 0.f, +1.f );
						TestSlope		= TestNormal;
						TestTime		= Edge.Max.Y - BodyRect.Min.Y;
					}
					else
					{
						TestNormal		= TVector( 0.f, -1.f );
						TestSlope		= TestNormal;
						TestTime		= BodyRect.Max.Y - Edge.Max.Y;
					}
				}
			}
			else if	( 
						Body->Velocity.Y*Normal.Y <= 0.f && 
						Edge.IsOverlap(BodyRect) && 
						PointLineDist( Body->Location, P1, Normal ) >= 0.f 
					)
			{			
				// Slope surface.
				Normal.Normalize();

				if( Normal.Y > 0.f )
				{
					// I or II quadrant.
					if( Normal.X > 0.f )
					{
						// I'st quadrant.
						Float X			= Clamp( BodyRect.Min.X, Edge.Min.X, Edge.Max.X );		
						Float YSlope	= Edge.Max.Y + ((Edge.Max.Y-Edge.Min.Y)*(Edge.Min.X-X)) / (Edge.Max.X-Edge.Min.X);	
							
						if( YSlope > BodyRect.Min.Y )
						{
							TestNormal		= TVector( 0.f, +1.f );
							TestSlope		= Normal;
							TestTime		= YSlope - BodyRect.Min.Y;
						}
						if( X != BodyRect.Min.X )	
							ExtraTime	= 5.f;
					}
					else
					{
						// II'nd quadrant.
						Float X			= Clamp( BodyRect.Max.X, Edge.Min.X, Edge.Max.X );		
						Float YSlope	= Edge.Min.Y + ((X-Edge.Min.X)*(Edge.Max.Y-Edge.Min.Y)) / (Edge.Max.X-Edge.Min.X);	
						
						if( YSlope > BodyRect.Min.Y )
						{
							TestNormal		= TVector( 0.f, +1.f );
							TestSlope		= Normal;
							TestTime		= YSlope - BodyRect.Min.Y;
						}
						if( X != BodyRect.Max.X )	
							ExtraTime	= 5.f;
					}
				}
				else
				{
					// III or IV quadrant.
					if( Normal.X > 0.f )
					{
						// IV'th quadrant.
						Float X			= Clamp( BodyRect.Min.X, Edge.Min.X, Edge.Max.X );	
						Float YSlope	= Edge.Min.Y + ((X-Edge.Min.X)*(Edge.Max.Y-Edge.Min.Y)) / (Edge.Max.X-Edge.Min.X);	

						if( YSlope < BodyRect.Max.Y )
						{
							TestNormal		= TVector( 0.f, -1.f );
							TestSlope		= Normal;
							TestTime		= BodyRect.Max.Y - YSlope;
						}
						if( X != BodyRect.Min.X )	
							ExtraTime	= 5.f;
					}
					else
					{
						// III'rd quadrant.
						Float X			= Clamp( BodyRect.Max.X, Edge.Min.X, Edge.Max.X );		
						Float YSlope	= Edge.Max.Y + ((Edge.Max.Y-Edge.Min.Y)*(Edge.Min.X-X)) / (Edge.Max.X-Edge.Min.X);

						if( YSlope < BodyRect.Max.Y )
						{
							TestNormal		= TVector( 0.f, -1.f );
							TestSlope		= Normal;
							TestTime		= BodyRect.Max.Y - YSlope;
						}
						if( X != BodyRect.Max.X )	
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
		TRect BodyRect		= Body->GetAABB();
		TRect OtherRect		= Other->GetAABB();

		// Direction from Other to Body.
		Float	BodyEx, OtherEx, OverlapX, OverlapY;
		TVector Dir			= Body->Location - Other->Location;

		BodyEx		= 0.5f * (BodyRect.Max.X - BodyRect.Min.X);
		OtherEx		= 0.5f * (OtherRect.Max.X - OtherRect.Min.X);

		OverlapX	= BodyEx + OtherEx - Abs(Dir.X);

		if( OverlapX > 0.f )
		{
			BodyEx		= 0.5f * (BodyRect.Max.Y - BodyRect.Min.Y);
			OtherEx		= 0.5f * (OtherRect.Max.Y - OtherRect.Min.Y);

			OverlapY	= BodyEx + OtherEx - Abs(Dir.Y);

			if( OverlapY > 0.f )
			{
				// Figure out the axis with least penetration.
				if( OverlapX < OverlapY )
				{
					HitNormal	= TVector( Dir.X<0.f ? -1.f : +1.f, 0.f );
					HitTime		= OverlapX;
					HitSlope	= HitNormal;
					return Axis == AXIS_X || Axis == AXIS_None;
				}
				else
				{
					HitNormal	= TVector( 0.f, Dir.Y<0.f ? -1.f : +1.f );
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
	TVector IncFace[2];
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
	TVector V1, V2;
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

	TVector PlaneNormal = V2 - V1;
	PlaneNormal.Normalize();
	TVector	RefNormal	= PlaneNormal.Cross();

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
		TVector			OldLocation		= Body->Location;
		BodyInvMass		= GetInvMass(Body);
		BodyInvIner		= GetInvInertia(Body);

		// Integrate translate forces.
		Body->Velocity	+=	Body->Forces * (BodyInvMass * Delta);
		Body->Forces	=	TVector( 0.f, 0.f );

		//
		// Here we clamp delta, to avoid very long distances.
		// It's reduce situations when body fall, or pass
		// through obstacles. I think max delta it's 95%
		// of body's size.
		//
		TVector VelDelta	= Body->Velocity * Delta;
		VelDelta.X	= Clamp( VelDelta.X, -Body->Size.X*0.95f, +Body->Size.X*0.95f );
		VelDelta.Y	= Clamp( VelDelta.Y, -Body->Size.Y*0.95f, +Body->Size.Y*0.95f );

		Body->Location	+= VelDelta;

		// Integrate rotation forces.
		Body->AngVelocity	+=	Body->Torque * (BodyInvIner * Delta);
		Body->Rotation		+=	TAngle( Body->AngVelocity * Delta );
		Body->Torque		=	0.f;

		// Get list of potential collide bodies, using cheap
		// AABB test.
		TRect OtherAABB, BodyAABB = Body->GetAABB();
		Level->CollHash->GetOverlapped( BodyAABB, NumOthers, Others );

		// Sort list of objects's for proper processing order.
		qsort( Others, NumOthers, sizeof(FBaseComponent*), MassCompare );

		// Convert Body to polygon.
		TVector	PolyOrig	= Body->Location;
		BodyToPoly( Body, AVerts, ANorms, ANum );

		// Test collision with all bodies.
		for( Int32 i=0; i<NumOthers; i++ )
		{
			// Figure out is body still overlaps with
			// Others[i] due position correction.
			Other		=	Others[i];
			BodyAABB	=	Body->GetAABB();
			OtherAABB	=	Other->GetAABB();

			if( !BodyAABB.IsOverlap(OtherAABB) )
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
					(Solution == HSOL_Oneway && HitSide == HSIDE_Top && Body->Velocity.Y < 0.f)
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
				TVector	V1 = TVector( 0.f, 0.f ), 
						V2 = TVector (0.f, 0.f );

				for( Int32 k=0; k<NumConts; k++ )
				{
					// Radii vectors.
					TVector RadBody		= Contacts[k] - Body->Location;
					TVector	RadOther	= Contacts[k] - Other->Location;

					// Relative velocity.
					TVector RelVel;
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
													Sqr(RACrossN) * BodyInvIner +
													Sqr(RBCrossN) * OtherInvIner;

						// Pick elasticity for hit solving.
						Float	e	=	!PhysOther ? GMaterials[Body->Material].Elasticity :
										Min( GMaterials[Body->Material].Elasticity, GMaterials[PhysOther->Material].Elasticity );

						// Scalar impulse.
						Float	j	= -(1 + e) * ProjVel / (InvMassTotal*NumConts);
						TVector	TotalImpulse	= HitNormal * j;

						// Apply impulse friction.
						TVector	Tangent	= RelVel - (HitNormal * (RelVel*HitNormal));
						Tangent.Normalize();
						Float jt	= -(RelVel * Tangent) / (InvMassTotal*NumConts);

						// Don't apply too small friction.
						if( ( jt <= -0.001f )||( jt >= +0.001f ) )
						{
							if( Abs(jt) < j*SFriction )
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
				TVector Correct =	HitNormal * PHYS_PENET_PERCENT * 
									(Max( 0.f, HitTime-PHYS_PENET_ALLOW )/(BodyInvMass+OtherInvMass)); 

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
			for( Int32 i=0; i<arr_len(Body->Touched); i++ )
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
			if( Abs(Body->AngVelocity) <= 0.1f )
				Body->AngVelocity	= 0.f;
			if( Body->Velocity.SizeSquared() <= SLEEP_THRESHOLD )
				Body->Velocity	= TVector( 0.f, 0.f );
		}

		FRigidBodyComponent* Rigid	= (FRigidBodyComponent*)Body;
		if( Rigid->bCanSleep )
		{
			if	( 
					Body->AngVelocity == 0.f && 
					Body->Velocity == TVector( 0.f, 0.f ) &&
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
		TVector			OldLocation		= Body->Location;

		//
		// Here we clamp delta, to avoid very long distances.
		// It's reduce situations when body fall, or pass
		// through obstacles. I think max delta it's 95%
		// of body's size.
		//
		TVector VelDelta	= Body->Velocity * Delta;
		VelDelta.X	= Clamp( VelDelta.X, -Body->Size.X*0.95f, +Body->Size.X*0.95f );
		VelDelta.Y	= Clamp( VelDelta.Y, -Body->Size.Y*0.95f, +Body->Size.Y*0.95f );

		//
		// Perform X-Movement.
		//
		{
			Body->Location.X	+= VelDelta.X;

			// Get list of potential collide bodies, using cheap
			// AABB test.
			TRect BodyAABB = Body->GetAABB();
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
				if( Other->IsA(FZoneComponent::MetaClass) && BodyAABB.IsOverlap(Other->GetAABB()) )
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
					Body->Location.X	+= HitNormal.X * HitTime;
					BodyAABB			= Body->GetAABB();

					// Brake velocity.
					if( bBrake && HitNormal.X != 0.f )
					{
						// Brake X-vector.
						if( HitSide == HSIDE_Left && Body->Velocity.X > 0.f )
							Body->Velocity.X	= 0.f;

						if( HitSide == HSIDE_Right && Body->Velocity.X < 0.f )
							Body->Velocity.X	= 0.f;
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
			Bool bNoMove = Abs(VelDelta.Y) <= EPSILON;
			Body->Location.Y	+= VelDelta.Y;

			// Get list of potential collide bodies, using cheap
			// AABB test.
			TRect BodyAABB = Body->GetAABB();
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
				if( Other->IsA(FZoneComponent::MetaClass) && BodyAABB.IsOverlap(Other->GetAABB()) )
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
						( Solution == HSOL_Oneway && HitSide == HSIDE_Top && Body->Velocity.Y < 0.f )
					)
				{
					// Handle solid collision.
					// Push up actor's location.
					Body->Location.Y	+= HitNormal.Y * HitTime;
					BodyAABB			= Body->GetAABB();

					// Brake velocity, but not for slopes surfaces.
					if( bBrake && HitSlope == HitNormal )
					{
						// Brake Y-vector.
						if( HitSide == HSIDE_Bottom && Body->Velocity.Y > 0.f )
							Body->Velocity.Y	= 0.f;

						if( HitSide == HSIDE_Top && Body->Velocity.Y < 0.f )
							Body->Velocity.Y	= 0.f;
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
			TRect ThisRect = Body->GetAABB();

			for( Int32 i=0; i<arr_len(Body->Touched); i++ )
				if( Body->Touched[i]  )
				{
					FBaseComponent* Other = Body->Touched[i]->Base;
					if	( 
							!Other->IsA(FRigidBodyComponent::MetaClass) &&
							!Other->IsA(FBrushComponent::MetaClass)
						)
					{
						TRect OtherRect = Other->GetAABB();
						if( !ThisRect.IsOverlap(OtherRect) )
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
		TVector OldLocation		= Base->Location;
		TAngle	OldRotation		= Base->Rotation;

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
					Int32 iPrevKey	= Floor(Object->Progress);
					Int32 iDestKey	= iPrevKey + 1;

					if( iDestKey < Object->Points.size() )
					{
						// Continue gliding.
						Float PathLen = Max( Distance( Object->Points[iPrevKey].Location, Object->Points[iDestKey].Location ), 0.01f );
						Object->Progress += (Object->Speed / PathLen) * Delta;
						Float Alpha	  = Interpolate(Object->Progress-iPrevKey);
						Base->Location	= Lerp
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
					Int32 iPrevKey	= Ceil(Object->Progress);
					Int32 iDestKey	= iPrevKey - 1;

					if( iDestKey >= 0 )
					{
						// Continue gliding.
						Float PathLen = Max( Distance( Object->Points[iDestKey].Location, Object->Points[iPrevKey].Location ), 0.01f );
						Object->Progress += (Object->Speed / PathLen) * Delta;
						Float Alpha	  = Interpolate(Object->Progress-iDestKey);
						Base->Location	= Lerp
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
				TVector MoveFrom	= Object->StartLocation;
				Float PathLen		= Max( Distance
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
					Base->Location	= Lerp( MoveFrom, Object->Points[iTarget].Location, Alpha );
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

	Float	W	= Rigid->Size.X,
			H	= Rigid->Size.Y,
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
void CPhysics::HandlePortals( FPhysicComponent* Body, const TVector& OldLocation )
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

			TCoords WarpLocal	= Warp->ToLocal();
			TVector NewLocal	= TransformPointBy( Body->Location,		WarpLocal );
			TVector OldLocal	= TransformPointBy( OldLocation,		WarpLocal );

			// Test for passing.
			if	( 
					( NewLocal.X * OldLocal.X < 0.f ) &&
					( OldLocal.Y >= -Warp->Width*0.5f ) &&
					( OldLocal.Y <= +Warp->Width*0.5f )
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

			Float A	= OldLocation.X			- Mirror->Location.X;
			Float B = Body->Location.X		- Mirror->Location.X;

			// Mirror X-test.
			if( A*B < 0.f )
			{
				Float C = Mirror->Location.Y	- Mirror->Width * 0.5f;
				Float D	= Mirror->Location.Y	+ Mirror->Width * 0.5f;

				// Mirror Y-test.
				if( Body->Location.Y >= C && Body->Location.Y <= D )
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
	for( Int32 i=0; i<arr_len(Body->Touched); i++ )
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
	for( Int32 i=0; i<arr_len(Body->Touched); i++ )
		if( Body->Touched[i] == nullptr )
		{
			iA	= i;
			break;
		}

	// Find avail slot in Other.
	FPhysicComponent* Phys = As<FPhysicComponent>( Other );
	if( Phys )
		for( Int32 i=0; i<arr_len(Phys->Touched); i++ )
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

	for( Int32 i=0; i<arr_len(Body->Touched); i++ )
		if( Body->Touched[i] == Other->Entity )
		{
			iA	= i;
			break;
		}

	FPhysicComponent* Phys = As<FPhysicComponent>( Other );
	if( Phys )
		for( Int32 i=0; i<arr_len(Phys->Touched); i++ )
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