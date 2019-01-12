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
	TColor			Color;
	DWord			Flags;
	Integer			iPosCtrl;
	Integer			iRotCtrl;
	Float			Scale;		

	// Control type specific.
	union
	{
		struct{ Bool	bLookAt; };							// SC_Bone.
		struct{ Bool	bFlipIK;	Integer iEndJoint; };	// SC_IKSolver.
	};

	// Constructors.
	TBoneInfo();
	TBoneInfo( ESkelCntrl InType, String InName, TColor InColor );

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
	TVector	Location;			// World space position of bone.
	TAngle	Rotation;			// World space rotation of bone.
	TCoords	Coords;				// Local coords system in parent space.

	// TBonePose interface.
	TBonePose( const TVector& InLocation, TAngle InRotation )
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
	TArray<TBonePose>	BonesPose;

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
	Integer					iBone;
	TInterpCurve<TVector>	PosKeys;
	TInterpCurve<TAngle>	RotKeys;

	// TBoneTrack interface.
	TBoneTrack()
	{}
	TBoneTrack( Integer iInBone )
		:	iBone( iInBone )
	{}

	// Friends.
	friend void Serialize( CSerializer& S, TBoneTrack& V )
	{
		Serialize( S, V.iBone );
		Serialize( S, V.PosKeys );
		Serialize( S, V.RotKeys );
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
	TArray<TBoneTrack>	BoneTracks;

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
	TArray<TBoneInfo>	Bones;
	TSkelPose			RefPose;

	// Bones remap tables.
	TArray<Integer>		TransformTable;		// Computed order for transformations.
	TArray<Integer>		RenderTable;		// Based on Z-value....

	// Actions.
	TArray<TSkeletonAction>	Actions;		// List of all animations.

	// Placeholder -------------------------------
	Integer		Placeholder;


	// FSkeleton interface.
	FSkeleton();
	~FSkeleton();
	TBoneInfo* FindBone( String InName );
	Integer FindAction( String InName );

	// Skeleton rendering.
	void Render
	( 
		CCanvas* Canvas, 
		const TVector& Origin, 
		const TVector& Scale, 
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
	Bool LinkTo( Integer iBone, Integer iParent );
	Bool LinkRotationTo( Integer iBone, Integer iParent );
	Bool LinkLookAtTo( Integer iBone, Integer iParent );
	Bool LinkIKSolverTo( Integer iIKSolver, Integer iEndJoint );
	Bool BreakPosLink( Integer iBone );
	Bool BreakRotLink( Integer iBone );
	Bool BreakLinks( Integer iBone );

	// Circular dependencies detector.
	Bool TryLink( Integer iBone, Integer iParent );
	Bool TryIKSolver( Integer iIK, Integer iEndJoint );		// ADD NAMES OF CONTROLLERS, USE CONST FLAG AND ADDRESS OF CHILDER NAME OR STATIC NONE STRING
	Bool TryLookAt( Integer iBone, Integer iTarget );
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/