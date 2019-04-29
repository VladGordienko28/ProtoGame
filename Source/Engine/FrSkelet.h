/*=============================================================================
	FrSkelet.h: Skeletal animation classes.
	Created by Vlad Gordienko, Dec. 2017.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Bone structures.
-----------------------------------------------------------------------------*/

//
// Bone flags.
//
#define	BONE_None			0x0000
#define	BONE_Selected		0x0001
#define	BONE_Reached		0x0002
#define BONE_PosProcessed	0x0004
#define BONE_RotProcessed	0x0008
#define BONE_Processed		(BONE_PosProcessed | BONE_RotProcessed) 


//
// A skeleton controllers.
//
enum ESkelCntrl
{
	SC_Bone,		// Simple bone controller.
	SC_Master,		// Bone master controller.
	SC_IKSolver,	// IK solver controller.
	SC_MAX
};


//
// An information about bone in the skeleton
// hierarchy. Doesn't contains bone transformations.
//
struct TBoneInfo
{
public:
	// General variables.
	ESkelCntrl		Type;
	String			Name;
	math::Color		Color;
	UInt32			Flags;
	Int32			iPosCtrl;
	Int32			iRotCtrl;
	Float			Scale;		

	// Control type specific.
	union
	{
		struct{ Bool	bLookAt; };							// SC_Bone.
		struct{ Bool	bFlipIK;	Int32 iEndJoint; };	// SC_IKSolver.
	};

	// Constructors.
	TBoneInfo();
	TBoneInfo( ESkelCntrl InType, String InName, math::Color InColor );

	// Friends.
	friend void Serialize( CSerializer& S, TBoneInfo& V );
};


/*-----------------------------------------------------------------------------
	Pose structures.
-----------------------------------------------------------------------------*/

//
// A single bone transformation at particular time.
//
struct TBonePose
{
public:
	// Variables.
	math::Vector	Location;			// World space position of bone.
	math::Angle		Rotation;			// World space rotation of bone.
	math::Coords	Coords;				// Local coords system in parent space.

	// TBonePose interface.
	TBonePose( const math::Vector& InLocation, math::Angle InRotation )
		:	Location(InLocation), Rotation(InRotation), 
			Coords( InLocation, InRotation )
	{}

	// Friends.
	friend void Serialize( CSerializer& S, TBonePose& V )
	{
		Serialize( S, V.Coords );
	}
};


//
// A skeleton pose at particular time.
//
struct TSkelPose
{
public:
	// Variables.
	Array<TBonePose>	BonesPose;

	// TSkelPose interface.
	TSkelPose();
	void ComputeRefTransform( FSkeleton* Skel );
	void CumputeAnimFrame( FSkeleton* Skel, class TSkeletonAction& Action, Float Time );

	// Friends.
	friend void Serialize( CSerializer& S, TSkelPose& V )
	{
		Serialize( S, V.BonesPose );
	}

private:
	// Internal.
	void SolveSkeleton( FSkeleton* Skel );
};


/*-----------------------------------------------------------------------------
	Skeleton animation structures.
-----------------------------------------------------------------------------*/

//
// Animation track of single bone in action.
//
class TBoneTrack
{
public:
	// Variables.
	Int32							iBone;
	math::InterpCurve<math::Vector>	PosKeys;
	math::InterpCurve<math::Angle>	RotKeys;

	// TBoneTrack interface.
	TBoneTrack()
	{}
	TBoneTrack( Int32 iInBone )
		:	iBone( iInBone )
	{}

	// Friends.
	friend void Serialize( CSerializer& S, TBoneTrack& V )
	{
		Serialize( S, V.iBone );
		// todo: implement serialization!
		//Serialize( S, V.PosKeys );
		//Serialize( S, V.RotKeys );
	}
};


//
// A skeleton animation sequence.
//
class TSkeletonAction
{
public:
	// Variables.
	String				Name;
	Array<TBoneTrack>	BoneTracks;

	// TSkeletonAction interface.
	TSkeletonAction();
	TSkeletonAction( String InName );

	// Friends.
	friend void Serialize( CSerializer& S, TSkeletonAction& V )
	{
		Serialize( S, V.Name );
		Serialize( S, V.BoneTracks );
	}
};


/*-----------------------------------------------------------------------------
	FSkeleton.
-----------------------------------------------------------------------------*/

//
// A skeleton.
//
class FSkeleton: public FResource
{
REGISTER_CLASS_H(FSkeleton)
public:
	// Variables.
	Array<TBoneInfo>	Bones;
	TSkelPose			RefPose;

	// Bones remap tables.
	Array<Int32>		TransformTable;		// Computed order for transformations.
	Array<Int32>		RenderTable;		// Based on Z-value....

	// Actions.
	Array<TSkeletonAction>	Actions;		// List of all animations.

	// Placeholder -------------------------------
	Int32		Placeholder;


	// FSkeleton interface.
	FSkeleton();
	~FSkeleton();
	TBoneInfo* FindBone( String InName );
	Int32 FindAction( String InName );

	// Skeleton rendering.
	void Render
	( 
		CCanvas* Canvas, 
		const math::Vector& Origin, 
		const math::Vector& Scale, 
		const TSkelPose& Pose 
	);

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void PostLoad();						// validation here?
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

	// Precompute skeleton remap tables.
	void BuildTransformationTable();


	// Skeleton hierarchy management.
	Bool LinkTo( Int32 iBone, Int32 iParent );
	Bool LinkRotationTo( Int32 iBone, Int32 iParent );
	Bool LinkLookAtTo( Int32 iBone, Int32 iParent );
	Bool LinkIKSolverTo( Int32 iIKSolver, Int32 iEndJoint );
	Bool BreakPosLink( Int32 iBone );
	Bool BreakRotLink( Int32 iBone );
	Bool BreakLinks( Int32 iBone );

	// Circular dependencies detector.
	Bool TryLink( Int32 iBone, Int32 iParent );
	Bool TryIKSolver( Int32 iIK, Int32 iEndJoint );		// ADD NAMES OF CONTROLLERS, USE CONST FLAG AND ADDRESS OF CHILDER NAME OR STATIC NONE STRING
	Bool TryLookAt( Int32 iBone, Int32 iTarget );
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/