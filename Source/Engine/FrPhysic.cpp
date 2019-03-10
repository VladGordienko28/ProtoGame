/*=============================================================================
    FrPhysic.cpp: Physics relative classes.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FJointComponent implementation.
-----------------------------------------------------------------------------*/

//
// Joint constructor.
//
FJointComponent::FJointComponent()
	:	FRectComponent(),
		Body1( nullptr ),
		Body2( nullptr ),
		Hook1( 0.f, 0.f ),
		Hook2( 0.f, 0.f )
{
	bTickable	= true;
}


//
// Initialize joint for level.
//
void FJointComponent::InitForEntity( FEntity* InEntity )
{
	FRectComponent::InitForEntity( InEntity );
}


//
// Serialize joint.
//
void FJointComponent::SerializeThis( CSerializer& S )
{
	FRectComponent::SerializeThis( S );
	Serialize( S, Body1 );
	Serialize( S, Body2 );
	Serialize( S, Hook1 );
	Serialize( S, Hook2 );
}


//
// Render joint.
//
void FJointComponent::Render( CCanvas* Canvas )
{
	FRectComponent::Render( Canvas );

	// Draw line from Body1 to Body2.
	if( Level->RndFlags & RND_Other )
		if( Body1 && Body2 )
		{
			math::Vector Point1 = math::transformPointBy( Hook1, Body1->Base->ToWorld() );
			math::Vector Point2 = math::transformPointBy( Hook2, Body2->Base->ToWorld() );

			Canvas->DrawLine
						(
							Point1,
							Point2,
							math::colors::LIME_GREEN,
							false
						);
		}
}


/*-----------------------------------------------------------------------------
    FSpringComponent implementation.
-----------------------------------------------------------------------------*/

//
// Spring constructor.
//
FSpringComponent::FSpringComponent()
	:	FJointComponent(),
		Damping( 5.f ),			
		Spring( 5.f ),
		Length( 8.f ),
		NumSegs( 3 ),
		Segment( nullptr ),
		Width( 1.f )
{
}


//
// Spring serialization.
//
void FSpringComponent::SerializeThis( CSerializer& S )
{
	FJointComponent::SerializeThis( S );
	Serialize( S, Damping );
	Serialize( S, Spring );
	Serialize( S, Length );
	Serialize( S, NumSegs );
	Serialize( S, Segment );
	Serialize( S, Width );
}


//
// Spring rendering.
//
void FSpringComponent::Render( CCanvas* Canvas )
{
	FJointComponent::Render( Canvas );

	// Render a spring.
	if( Body1 && Body2 && Segment && NumSegs >= 0 )
	{
		// Get hooks location.
		math::Vector Point1 = math::transformPointBy( Hook1, Body1->Base->ToWorld() );
		math::Vector Point2 = math::transformPointBy( Hook2, Body2->Base->ToWorld() );

		// Setup spring rectangle.
		TRenderRect Rect;
		Rect.Texture			= Segment;
		Rect.Color				= bSelected ? math::Color( 0x80, 0xe6, 0x80, 0xff ) : math::colors::WHITE;
		Rect.Flags				= POLY_Unlit;
		Rect.Rotation			= math::vectorToAngle((Point1-Point2).cross());
		Rect.TexCoords			= math::Rect( math::Vector( 0.5f, 0.f ), 1.f, NumSegs );
		Rect.Bounds				= math::Rect
									( 
										(Point1+Point2) * 0.5, 
										Width, (Point1-Point2).size()
									);

		// Draw spring.
		Canvas->DrawRect( Rect );
	}
}


//
// Setup spring physics.
//
void FSpringComponent::PreTick( Float Delta )
{
	if( !Body1 || !Body2 )
		return;
	
	FPhysicComponent* Phys1	= As<FPhysicComponent>( Body1->Base );
	FPhysicComponent* Phys2	= As<FPhysicComponent>( Body2->Base );

	math::Coords ToWorld1 = Body1->Base->ToWorld(),
			ToWorld2 = Body2->Base->ToWorld();

	math::Vector	Point1	= math::transformPointBy( Hook1, ToWorld1 ),
			Point2	= math::transformPointBy( Hook2, ToWorld2 );

	math::Vector	Rad1	= math::transformVectorBy( Hook1, ToWorld1 ),
			Rad2	= math::transformVectorBy( Hook2, ToWorld2 );

	math::Vector Vel1	= Phys1 ? Phys1->Velocity : math::Vector( 0.f, 0.f );
	math::Vector Vel2	= Phys2 ? Phys2->Velocity : math::Vector( 0.f, 0.f );

	math::Vector RelVel, RelLoc;
	RelVel	= Vel2 - Vel1;
	RelLoc	= Point2 - Point1;

	Float DL, S;
	math::Vector F;

	DL	= RelLoc.size() - Length;
	S	= Spring * DL;
	RelLoc.normalize();

	F	= (RelLoc * S) + RelLoc*(Damping*(RelVel*RelLoc));

	// Test for sleeping.
	FRigidBodyComponent*	Rigid1	= As<FRigidBodyComponent>(Phys1);
	FRigidBodyComponent*	Rigid2	= As<FRigidBodyComponent>(Phys2);

	// Apply impulses.
	if( Phys1 && !(Rigid1 && Rigid1->bSleeping) )
	{
		Phys1->Forces	+= F;
		Phys1->Torque	+= Rad1 / F;
	}
	if( Phys2 && !(Rigid2 && Rigid2->bSleeping) )
	{
		Phys2->Forces	-= F;
		Phys2->Torque	-= Rad2 / F;
	}
}


/*-----------------------------------------------------------------------------
    FHingeComponent implementation.
-----------------------------------------------------------------------------*/

//
// Hinge constructor.
//
FHingeComponent::FHingeComponent()
	:	FJointComponent()
{
}


//
// Hinge serialization.
//
void FHingeComponent::SerializeThis( CSerializer& S )
{
	FJointComponent::SerializeThis(S);
}


//
// Hinge import.
//
void FHingeComponent::Import( CImporterBase& Im )
{
	FJointComponent::Import(Im);
}


//
// Hinge export.
//
void FHingeComponent::Export( CExporterBase& Ex )
{
	FJointComponent::Export(Ex);
}


//
// Hinge joint rendering.
//
void FHingeComponent::Render( CCanvas* Canvas )
{
	FJointComponent::Render( Canvas );

	if( Level->RndFlags & RND_Other )		
	{
		// Render Body1.
		if( Body1 )
		{
			math::Vector Pin = math::transformPointBy( Hook1, Body1->Base->ToWorld() );
			Canvas->DrawLineStar( Pin, Body1->Base->Rotation, 2.f, math::colors::LIGHT_CORAL, false );
			Canvas->DrawPoint( Pin, 5.f, math::colors::LIGHT_CORAL );
		}

		// Render Body2.
		if( Body2 )
		{
			math::Vector Pin = math::transformPointBy( Hook2, Body2->Base->ToWorld() );
			Canvas->DrawLineStar( Pin, Body2->Base->Rotation, 2.f, math::colors::LIGHT_BLUE, false );
			Canvas->DrawPoint( Pin, 5.f, math::colors::LIGHT_BLUE );
		}
	}
}


//
// Setup hinge physics.
//
void FHingeComponent::PreTick( Float Delta )
{
	// Check bodies.
	if( !Body1 || !Body2 )
		return;

	FPhysicComponent* Phys2 = As<FPhysicComponent>(Body2->Base);
	if( !Phys2 )
		return;

	// Unhash body to perform movement.
	if( Phys2->bHashable ) 
		Level->CollHash->RemoveFromHash(Phys2);
	{
		math::Vector	Pin1	= math::transformPointBy( Hook1, Body1->Base->ToWorld() );
		math::Vector	Pin2	= math::transformPointBy( Hook2, Body2->Base->ToWorld() );
		math::Vector	Fix		= Pin1 - Pin2;

		// Move body to pin, and eliminate movement
		// forces.
		Phys2->Location	+= Fix;
		Phys2->Velocity	= math::Vector( 0.f, 0.f );
		Phys2->Forces	= math::Vector( 0.f, 0.f );
	}
	if( Phys2->bHashable )
		Level->CollHash->AddToHash(Phys2);
}


/*-----------------------------------------------------------------------------
    FKeyframeComponent implementation.
-----------------------------------------------------------------------------*/

//
// Keyframe constructor.
//
FKeyframeComponent::FKeyframeComponent()
	:	Points(),
		GlideType( GLIDE_None ),
		iTarget( -1 ),
		bLooped( false ),
		Speed( 0.f ),
		Progress( 0.f ),
		StartLocation( 0.f, 0.f )
{
	bTickable = true;
}


//
// Game just started.
//
void FKeyframeComponent::BeginPlay()
{
	FExtraComponent::BeginPlay();
}


//
// Tick keyframe.
//
void FKeyframeComponent::Tick( Float Delta )
{
	if( GlideType != GLIDE_None )
		CPhysics::PhysicKeyframe( this, Delta );
}


//
// Keypoint serialization.
//
void Serialize( CSerializer& S, TKeyframePoint& V )
{
	Serialize( S, V.Location );
	Serialize( S, V.Rotation );
	Serialize( S, V.bCCW );
}


//
// Serialize keyframe.
//
void FKeyframeComponent::SerializeThis( CSerializer& S )
{
	FExtraComponent::SerializeThis( S );
	Serialize( S, Points );

#if 0
	// Is really should serialize all value, cause
	// them used during play only.
	SerializeEnum( S, GlideType );
	Serialize( S, iTarget );
	Serialize( S, bLooped );
	Serialize( S, Speed );
	Serialize( S, Progress );
	Serialize( S, StartLocation );
#endif
}


//
// Import the keyframe.
//
void FKeyframeComponent::Import( CImporterBase& Im )
{
	FExtraComponent::Import( Im );

	Points.setSize( Im.ImportInteger(L"NumPoints") );
	for( Int32 i=0; i<Points.size(); i++ )
	{
		Points[i].Location	= Im.ImportVector( *String::format( L"Points[%d].Location", i ) );
		Points[i].Rotation	= Im.ImportAngle( *String::format( L"Points[%d].Rotation", i ) );
		Points[i].bCCW		= Im.ImportBool( *String::format( L"Points[%d].bCCW", i ) );
	}
}


//
// Export the keyframe.
//
void FKeyframeComponent::Export( CExporterBase& Ex )
{
	FExtraComponent::Export( Ex );

	Ex.ExportInteger( L"NumPoints", Points.size() );
	for( Int32 i=0; i<Points.size(); i++ )
	{
		Ex.ExportVector( *String::format( L"Points[%d].Location", i ), Points[i].Location );
		Ex.ExportAngle( *String::format( L"Points[%d].Rotation", i ), Points[i].Rotation );
		Ex.ExportBool( *String::format( L"Points[%d].bCCW", i ), Points[i].bCCW );
	}
}

//
// Start moving.
//
void FKeyframeComponent::nativeStart( CFrame& Frame )
{
	Speed		= POP_FLOAT;
	bLooped		= POP_BOOL;
	GlideType	= GLIDE_Forward;
}


//
// Stop moving.
//
void FKeyframeComponent::nativeStop( CFrame& Frame )
{
	GlideType	= GLIDE_None;
	Speed		= 0.f;
}


//
// Move to the key from current location.
//
void FKeyframeComponent::nativeMoveTo( CFrame& Frame )
{
	Speed			= POP_FLOAT;
	iTarget			= clamp( POP_INTEGER, 0, Points.size()-1 );
	StartLocation	= Base->Location;
	GlideType		= GLIDE_Target;
	Progress		= 0.f;
}


/*-----------------------------------------------------------------------------
    FRigidBodyComponent implementation.
-----------------------------------------------------------------------------*/

// Physics per frame iterations count.		
enum{ NUM_PHYS_ITERS = 3 };


//
// Initialize rigid body.
//
FRigidBodyComponent::FRigidBodyComponent()
	:	FPhysicComponent(),
		bSleeping( false ),
		bCanSleep( true )
{
}


//
// Pre-tick physics.
//
void FRigidBodyComponent::PreTick( Float Delta )
{
	if( !bCanSleep || !bSleeping )
	{
		// Let's script setup forces.
		Entity->OnPreTick(Delta);

		// Let's physics engine prepare.
		CPhysics::SetupPhysics( this, Delta );
	}
}


//
// Tick rigid body.
//
void FRigidBodyComponent::Tick( Float Delta )
{
	if( !bCanSleep || !bSleeping )
	{
		// Process physics.
		for( Int32 i=0; i<NUM_PHYS_ITERS; i++ )
			CPhysics::PhysicComplex( this, Delta*(1.f/NUM_PHYS_ITERS) );

		// Notify script.
		Entity->OnTick( Delta );
	}
}


//
// Serialize body.
//
void FRigidBodyComponent::SerializeThis( CSerializer& S )
{
	FPhysicComponent::SerializeThis( S );
	SerializeEnum( S, Material );
	Serialize( S, bCanSleep );
	Serialize( S, bSleeping );
	Serialize( S, Inertia );
	Serialize( S, Forces );
	Serialize( S, Torque );
}


//
// Import body properties.
//
void FRigidBodyComponent::Import( CImporterBase& Im )
{
	FPhysicComponent::Import( Im );
	IMPORT_FLOAT( Inertia );
}


//
// Export body properties.
//
void FRigidBodyComponent::Export( CExporterBase& Ex )
{
	FPhysicComponent::Export( Ex );
	EXPORT_FLOAT( Inertia );
}


//
// When some rigid body property changed via
// Object inspector.
//
void FRigidBodyComponent::EditChange()
{
	FPhysicComponent::EditChange();

	if( Material != PM_Custom )
		CPhysics::ComputeRigidMaterial( this );
}


/*-----------------------------------------------------------------------------
    FArcadeBodyComponent implementation.
-----------------------------------------------------------------------------*/

//
// Initialize arcade body.
//
FArcadeBodyComponent::FArcadeBodyComponent()
	:	FPhysicComponent()
{
}


//
// Pre-tick arcade physics.
//
void FArcadeBodyComponent::PreTick( Float Delta )
{
	// Let's script setup forces.
	Entity->OnPreTick( Delta );

	// Let's physics engine prepare.
	CPhysics::SetupPhysics( this, Delta );
}


//
// Tick arcade body.
//
void FArcadeBodyComponent::Tick( Float Delta )
{
	// Process physics.
	CPhysics::PhysicArcade( this, Delta );

	// Notify script.
	Entity->OnTick( Delta );
}


/*-----------------------------------------------------------------------------
    FPhysicComponent implementation.
-----------------------------------------------------------------------------*/

//
// Physic body constructor.
//
FPhysicComponent::FPhysicComponent()
	:	FRectComponent(),
		Material( PM_Custom ),
		Velocity( 0.f, 0.f ),
		Mass( 1.f ),
		Inertia( 0.f ),
		Forces( 0.f, 0.f ),
		AngVelocity( 0.f ),
		Torque( 0.f ),
		Floor( nullptr ),
		Zone( nullptr )
{
	mem::zero( Touched, sizeof(Touched) );
	bHashable	= true;

	bTickable	= true;
}


//
// Initialize body for entity.
//
void FPhysicComponent::InitForEntity( FEntity* InEntity )
{
	FRectComponent::InitForEntity( InEntity );
}


//
// Serialize physics fields.
//
void FPhysicComponent::SerializeThis( CSerializer& S )
{
	FRectComponent::SerializeThis( S );
	
	// It's just serialize common physics
	// properties.
	Serialize( S, Velocity );
	Serialize( S, AngVelocity );
	Serialize( S, Mass );
	Serialize( S, Floor );
	Serialize( S, Zone );

	for( Int32 i=0; i<arraySize(Touched); i++ )
		Serialize( S, Touched[i] );
}


//
// Solve solid collision hit.
//
void FPhysicComponent::nativeSolveSolid( CFrame& Frame )
{
	CPhysics::bBrake	= POP_BOOL || CPhysics::bBrake;
	CPhysics::Solution	= max( CPhysics::Solution, HSOL_Solid );
}


//
// Solve oneway collision hit. Very useful
// for platformer stuff.
//
void FPhysicComponent::nativeSolveOneway( CFrame& Frame )
{
	CPhysics::bBrake	= POP_BOOL || CPhysics::bBrake;
	CPhysics::Solution	= max( CPhysics::Solution, HSOL_Oneway );
}


//
// Return true if object touching other.
//
void FPhysicComponent::nativeIsTouching( CFrame& Frame )
{
	FEntity*	Other	= POP_ENTITY;
	Bool		Result	= false;

	if( Other )
		for( Int32 i=0; i<arraySize(Touched); i++ )
			if( Touched[i] == Other )
			{
				Result	= true;
				break;
			}

	*POPA_BOOL	= Result;
}


/*-----------------------------------------------------------------------------
    FMovingPlatformComponent implementation.
-----------------------------------------------------------------------------*/

//
// Mover constructor.
//
FMoverComponent::FMoverComponent()
	:	FRectComponent(),
		NumRds( 0 ),
		OldLocation( 0.f, 0.f )
{
	mem::zero( Riders, sizeof(Riders) );
	bHashable	= true;

	bTickable	= true;
}


//
// Initialize mover for game.
//
void FMoverComponent::BeginPlay()
{
	FRectComponent::BeginPlay();
	
	OldLocation		= Location;
	Reset();
}


//
// Reset the mover.
//
void FMoverComponent::Reset()
{
	NumRds	= 0;
}


//
// Add another passenger to this mover.
//
Bool FMoverComponent::AddRider( FPhysicComponent* InRider )
{
	if( NumRds < MAX_RIDERS )
	{
		Riders[NumRds++]	= InRider;
		return true;
	}
	else
	{
		warn( L"Phys: Mover '%s' exceeded %d riders", *GetFullName(), MAX_RIDERS );
		return false;
	}
}


//
// Serialize a mover.
//
void FMoverComponent::SerializeThis( CSerializer& S )
{
	FRectComponent::SerializeThis( S );

	// Don't save or load properties.
	if( S.GetMode() == SM_Undefined )
	{
		Serialize( S, OldLocation );
		Serialize( S, NumRds );
		for( Int32 iRd=0; iRd<NumRds; iRd++ )
			Serialize( S, Riders[iRd] );
	}
}


//
// Step moving platform in current frame.
//
void FMoverComponent::PreTick( Float Delta )
{
	Level->CollHash->RemoveFromHash( this );
	{
		// Handle the manual script moving.
		// Let's script modify location.
		Entity->OnStep( Delta );

		// Compute delta.
		math::Vector Shift	= Location - OldLocation;
		OldLocation		= Location;					

		// Move each rider.
		{
#if 1
			// Drown riders a little, this is a little hack 
			// but works well.
			Shift	-= math::Vector( 0.f, 4.f ) * Delta;
#endif
			for( Int32 iRd=0; iRd<NumRds; iRd++ )
			{
				Level->CollHash->RemoveFromHash( Riders[iRd] );
				{
					Riders[iRd]->Location	+= Shift;
				}
				Level->CollHash->AddToHash( Riders[iRd] );
			}
		}
	}
	Level->CollHash->AddToHash( this );

	// Reset mover until new time pass.
	Reset();
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FKeyframeComponent, FExtraComponent, CLASS_SingleComp )
{
	ADD_PROPERTY( iTarget, PROP_None );
	ADD_PROPERTY( bLooped, PROP_None );
	ADD_PROPERTY( Speed, PROP_None );
	ADD_PROPERTY( Progress, PROP_None );

	DECLARE_METHOD( Start, TYPE_None, ARG(speed, TYPE_Float, ARG(looped, TYPE_Bool, END)) );
	DECLARE_METHOD( Stop, TYPE_None, END );
	DECLARE_METHOD( MoveTo, TYPE_None, ARG(speed, TYPE_Float, ARG(iTarget, TYPE_Integer, END)) );
}
    

REGISTER_CLASS_CPP( FRigidBodyComponent, FPhysicComponent, CLASS_None )
{
	BEGIN_ENUM(EPhysMaterial)
		ENUM_ELEM(PM_Manual);
		ENUM_ELEM(PM_Wood);
		ENUM_ELEM(PM_Rock);
		ENUM_ELEM(PM_Metal);
		ENUM_ELEM(PM_Ice);
		ENUM_ELEM(PM_Glass);
		ENUM_ELEM(PM_BouncyBall);
	END_ENUM;

	ADD_PROPERTY( bCanSleep, PROP_Editable );
	ADD_PROPERTY( bSleeping, PROP_Editable );
	ADD_PROPERTY( Material, PROP_Editable );
}


REGISTER_CLASS_CPP( FArcadeBodyComponent, FPhysicComponent, CLASS_None )
{
}


REGISTER_CLASS_CPP( FMoverComponent, FRectComponent, CLASS_None )
{
	ADD_PROPERTY( OldLocation, PROP_None );
}


REGISTER_CLASS_CPP( FPhysicComponent, FRectComponent, CLASS_Abstract )
{
	BEGIN_ENUM(EHitSolution)
		ENUM_ELEM(HSOL_None);
		ENUM_ELEM(HSOL_Oneway);
		ENUM_ELEM(HSOL_Solid);
	END_ENUM;

	BEGIN_ENUM(EHitSide)
		ENUM_ELEM(HSIDE_Top);
		ENUM_ELEM(HSIDE_Bottom);
		ENUM_ELEM(HSIDE_Left);
		ENUM_ELEM(HSIDE_Right);
	END_ENUM;

	ADD_PROPERTY( Velocity, PROP_Editable );
	ADD_PROPERTY( Mass, PROP_Editable );
	ADD_PROPERTY( Inertia, PROP_Editable | PROP_NoImEx );
	ADD_PROPERTY( Forces, PROP_None );
	ADD_PROPERTY( AngVelocity, PROP_Editable );
	ADD_PROPERTY( Torque, PROP_None );
	ADD_PROPERTY( Floor, PROP_None | PROP_NoImEx );
	ADD_PROPERTY( Zone, PROP_None | PROP_NoImEx );
	ADD_PROPERTY( Touched, PROP_None | PROP_NoImEx );

	DECLARE_METHOD( SolveSolid, TYPE_None, ARG(bBrake, TYPE_Bool, END) );
	DECLARE_METHOD( SolveOneway, TYPE_None, ARG(bBrake, TYPE_Bool, END) );
	DECLARE_METHOD( IsTouching, TYPE_Bool, ARG(other, TYPE_Entity, END) );
}


REGISTER_CLASS_CPP( FJointComponent, FRectComponent, CLASS_Abstract )
{
	ADD_PROPERTY( Body1, PROP_Editable );
	ADD_PROPERTY( Body2, PROP_Editable );
	ADD_PROPERTY( Hook1, PROP_Editable );
	ADD_PROPERTY( Hook2, PROP_Editable );
}


REGISTER_CLASS_CPP( FSpringComponent, FJointComponent, CLASS_None )
{
	ADD_PROPERTY( Damping, PROP_Editable );
	ADD_PROPERTY( Spring, PROP_Editable );
	ADD_PROPERTY( Length, PROP_Editable );
	ADD_PROPERTY( NumSegs, PROP_Editable );
	ADD_PROPERTY( Segment, PROP_Editable );
	ADD_PROPERTY( Width, PROP_Editable );
}


REGISTER_CLASS_CPP( FHingeComponent, FJointComponent, CLASS_None )
{
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/