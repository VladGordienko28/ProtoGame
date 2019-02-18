/*=============================================================================
    FrPhysEng.h: Flu physics engine.
    Copyright Sep.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
	CPhysics.
-----------------------------------------------------------------------------*/

//
// A hit solution.
//
enum EHitSolution
{
	HSOL_None,			// No collision solution.
	HSOL_Oneway,		// Oneway platform solution.
	HSOL_Solid			// Solid collision solution.
};


//
// A side of collision hit.
//
enum EHitSide
{
	HSIDE_Top,
	HSIDE_Bottom,
	HSIDE_Left,
	HSIDE_Right,
	HSIDE_MAX
};


//
// An axis.
//
enum EAxis
{
	AXIS_None,
	AXIS_X,
	AXIS_Y,
};


//
// Rigid-body physics simulator.
//
class CPhysics
{
public:
	// Top level physics functions.
	static void SetupPhysics( FPhysicComponent* Body, Float Delta );
	static void PhysicComplex( FPhysicComponent* Body, Float Delta );
	static void PhysicArcade( FPhysicComponent* Body, Float Delta );
	static void PhysicKeyframe( FKeyframeComponent* Object, Float Delta );

private:
	// Detected collision info.
	static math::Vector		HitNormal;
	static math::Vector		HitSlope;
	static Float			HitTime;
	static EHitSide			HitSide;

	// Collision detection functions.
	static Bool DetectArcadeCollision( EAxis Axis, FPhysicComponent* Body, FBaseComponent* Other );
	static Bool DetectComplexCollision();

	// Touching.
	static Bool BeginTouch( FPhysicComponent* Body, FBaseComponent* Other );
	static Bool EndTouch( FPhysicComponent* Body, FBaseComponent* Other );
	static Bool IsTouch( FPhysicComponent* Body, FBaseComponent* Other );

	// Portals.
	static void HandlePortals( FPhysicComponent* Body, const math::Vector& OldLocation );

	// Zones.
	static Bool SetBodyZone( FPhysicComponent* Body, FZoneComponent* NewZone );

	// Other.
	static void ComputeRigidMaterial( FRigidBodyComponent* Rigid );

	// Script communication variables.
	static FLevel*			Level;
	static EHitSolution		Solution;
	static Bool				bBrake;

	// Complex physics.
	static Float			BodyInvMass;
	static Float			BodyInvIner;
	static Float			OtherInvMass;
	static Float			OtherInvIner;

	// List of collide objects.
	static FBaseComponent*	Other;
	static FBaseComponent*	Others[MAX_COLL_LIST_OBJS];
	static Int32			NumOthers;

	// Polys.
	static math::Vector		AVerts[16];
	static math::Vector		ANorms[16];
	static math::Vector		BVerts[16];
	static math::Vector		BNorms[16];
	static Int32			ANum;
	static Int32			BNum;

	// Contact info.
	static math::Vector		Contacts[2];
	static Int32			NumConts;

	// Friends.
	friend FPhysicComponent;
	friend FArcadeBodyComponent;
	friend FRigidBodyComponent;
	friend FLevel;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/