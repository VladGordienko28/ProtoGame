/*=============================================================================
    FrComTyp.h: Various components.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    FKeyframeComponent.
-----------------------------------------------------------------------------*/

//
// Keyframe glide type.
//
enum EKeyGlide
{
	GLIDE_None,
	GLIDE_Forward,
	GLIDE_Target
};


//
// An keyframe interpolation point.
//
struct TKeyframePoint
{
public:
	math::Vector	Location;
	math::Angle		Rotation;
	Bool			bCCW;
};


//
// Keyframe.
//
class FKeyframeComponent: public FExtraComponent
{
REGISTER_CLASS_H(FKeyframeComponent);
public:
	friend					CPhysics;

	// Variables.
	Array<TKeyframePoint>	Points;

	// FKeyframeComponent interface.
	FKeyframeComponent();

	// FComponent interface.
	void BeginPlay();
	void Tick( Float Delta );

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

private:
	// Keyframe internal variables.
	EKeyGlide			GlideType;
	UInt8				iTarget;
	Bool				bLooped;
	Float				Speed;
	Float				Progress;
	math::Vector		StartLocation;

	// Native.
	void nativeStart( CFrame& Frame );
	void nativeStop( CFrame& Frame );
	void nativeMoveTo( CFrame& Frame );
};


/*-----------------------------------------------------------------------------
    FLogicComponent.
-----------------------------------------------------------------------------*/

//
// A logic elements connector.
//
struct TLogicConnector
{
public:
	// Variables.
	FLogicComponent*	Target;
	Int32				iJack;

	// TLogicConnector interface.
	friend void Serialize( CSerializer& S, TLogicConnector& V );
	Bool operator==( const TLogicConnector& LC ) const;
};


//
// A logic element.
//
class FLogicComponent: public FExtraComponent
{
REGISTER_CLASS_H( FLogicComponent );
public:
	// Constants.
	enum { MAX_LOGIC_PLUGS	= 8 };
	enum { MAX_LOGIC_JACKS	= 8 };

	// Variables.
	Bool		bEnabled;
	String		PlugsName[MAX_LOGIC_PLUGS];
	String		JacksName[MAX_LOGIC_JACKS];
	Int32		NumPlugs;
	Int32		NumJacks;

	// Internal.
	FLogicComponent*	NextLogicElement;

	// List of logic connections.
	Array<TLogicConnector>	Plugs[MAX_LOGIC_PLUGS];
	
	// FLogicComponent interface.
	FLogicComponent();
	~FLogicComponent();
	math::Vector GetPlugPos( Int32 iPlug );
	math::Vector GetJackPos( Int32 iJack );
	void AddConnector( FLogicComponent* InTarget, Int32 iPlug, Int32 iJack );
	void RemoveConnectors( Int32 iPlug );
	void CleanBadConnectors();

	// FComponent interface.
	void InitForEntity( FEntity* InEntity );
	void Render( CCanvas* Canvas );
	
	// FObject interface.
	void EditChange();
	void SerializeThis( CSerializer& S );
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

private:
	// Logic natives.
	void nativeInduceSignal( CFrame& Frame );
};


/*-----------------------------------------------------------------------------
    FAnimatedSpriteComponent.
-----------------------------------------------------------------------------*/

//
// A sprite with the animation.
//
class FAnimatedSpriteComponent: public FExtraComponent
{
REGISTER_CLASS_H(FAnimatedSpriteComponent);
public:
	// Variables.
	Bool			bHidden;
	Bool			bUnlit;
	Bool			bFlipH;
	Bool			bFlipV;
	FAnimation*		Animation;
	math::Color		Color;
	math::Vector	Offset;
	math::Vector	Scale;
	math::Angle		Rotation;

	// FAnimatedSpriteComponent interface.
	FAnimatedSpriteComponent();

	// FComponent interface.
	void Tick( Float Delta );
	void Render( CCanvas* Canvas );

	// FObject interface.
	void SerializeThis( CSerializer& S );

private:
	// Played animation type.
	enum EAnimType
	{
		ANIM_Once,
		ANIM_Loop,
		ANIM_PingPong
	};

	// Animation internal.
	EAnimType			AnimType;
	UInt8				iSequence;
	Bool				bBackward;
	Float				Rate;
	Float				Frame;

	// Natives.
	void nativePlayAnim( CFrame& Frame );
	void nativePauseAnim( CFrame& Frame );
	void nativeGetAnimName( CFrame& Frame );
	void nativeIsPlaying( CFrame& Frame );
};


/*-----------------------------------------------------------------------------
    FBrushComponent.
-----------------------------------------------------------------------------*/

//
// Brush collision type.
//
enum EBrushType
{
	BRUSH_NotSolid,
	BRUSH_SemiSolid,
	BRUSH_Solid,
	BRUSH_MAX
};


//
// A level geometry block.
//
class FBrushComponent: public FBaseComponent
{
REGISTER_CLASS_H(FBrushComponent);
public:
	// How much allow vertex per brush.
	enum { MAX_BRUSH_VERTS	= 8 };

	// Variables.
	Bool				bUnlit;
	Bool				bFlipH;
	Bool				bFlipV;
	math::Color			Color;	
	FTexture*			Texture;
	EBrushType			Type;
	math::Vector		Vertices[MAX_BRUSH_VERTS];
	Int32				NumVerts;
	math::Coords		TexCoords;
	math::Vector		Scroll;

	// FBrushComponent interface.
	FBrushComponent();

	// FBaseComponent interface.
	math::Rect GetAABB();

	// FComponent interface.
	void Render( CCanvas* Canvas );

	// FObject interface.
	void SerializeThis( CSerializer& S );
};


/*-----------------------------------------------------------------------------
    FPortalComponent.
-----------------------------------------------------------------------------*/

//
// An abstract portal.
//
class FPortalComponent: public FBaseComponent
{
REGISTER_CLASS_H(FPortalComponent);
public:
	// Variables.
	Float			Width;

	// Internal.
	FPortalComponent*	NextPortal;

	// FPortalComponent interface.
	FPortalComponent();
	~FPortalComponent();
	virtual math::Vector TransferPoint( math::Vector P );
	virtual math::Vector TransferVector( math::Vector V );
	virtual Bool ComputeViewInfo( const TViewInfo& Parent, TViewInfo& Result );

	// FComponent interface.
	void InitForEntity( FEntity* InEntity );

	// FObject interface.
	void EditChange();
	void SerializeThis( CSerializer& S );
};


/*-----------------------------------------------------------------------------
    FMirrorComponent.
-----------------------------------------------------------------------------*/

//
// Mirror portal.
//
class FMirrorComponent: public FPortalComponent
{
REGISTER_CLASS_H(FMirrorComponent);
public:
	// FPortalComponent interface.
	FMirrorComponent();
	math::Vector TransferPoint( math::Vector P );
	math::Vector TransferVector( math::Vector V );
	Bool ComputeViewInfo( const TViewInfo& Parent, TViewInfo& Result );

	// CRenderAddon interface.
	void Render( CCanvas* Canvas );

private:
	// Natives.
	void nativeMirrorPoint( CFrame& Frame );
	void nativeMirrorVector( CFrame& Frame );
};


/*-----------------------------------------------------------------------------
    FWarpComponent.
-----------------------------------------------------------------------------*/

//
// Warp portal.
//
class FWarpComponent: public FPortalComponent
{
REGISTER_CLASS_H(FWarpComponent);
public:
	// Variables.
	FEntity*		Other;

	// FWarpComponent interface.
	FWarpComponent();

	// FPortalComponent interface.
	math::Vector TransferPoint( math::Vector P );
	math::Vector TransferVector( math::Vector V );
	Bool ComputeViewInfo( const TViewInfo& Parent, TViewInfo& Result );

	// FObject interface.
	void EditChange();
	void SerializeThis( CSerializer& S );

	// CRenderAddon interface.
	void Render( CCanvas* Canvas );

private:
	// Natives.
	void nativeWarpPoint( CFrame& Frame );
	void nativeWarpVector( CFrame& Frame );
};


/*-----------------------------------------------------------------------------
    FModelComponent.
-----------------------------------------------------------------------------*/

//
// Tile graphics model.
//
class FModelComponent: public FBaseComponent
{
REGISTER_CLASS_H(FModelComponent);
public:
	// Total allow tiles per side.
	enum { MAX_TILES_SIDE = 1024 };

	// Variables.
	Bool				bUnlit;
	math::Color			Color;	
	FTexture*			Texture;

	// Tilemap.
	Array<UInt16>		Map;
	Int32				MapXSize;
	Int32				MapYSize;
	math::Vector		TileSize;

	// Tileset information.
	UInt8				TilesPerU;
	UInt8				TilesPerV;

	// Precomputed tables.
	math::Rect			AtlasTable[256];

	// Model edit tool.
	Int32				PenIndex;
	Array<UInt8>		Selected;

	// FModelComponent interface.
	FModelComponent();
	void ReallocMap();
	void SetAtlasTable();
	Int32 WorldToMapIndex( Float vX, Float vY );

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );
	void EditChange();
	void PostLoad();

	// FBaseComponent interface.
	void InitForEntity( FEntity* InEntity );
	math::Rect GetAABB();

	// CBitmapRenderAddon interface.
	void Render( CCanvas* Canvas );

private:
	// Natives.
	void nativeGetTile( CFrame& Frame );
	void nativeSetTile( CFrame& Frame );
	void nativeWorldToMap( CFrame& Frame );
};


/*-----------------------------------------------------------------------------
    FDecoComponent.
-----------------------------------------------------------------------------*/

//
// Deco animation type.
//
enum EDecoType
{
	DECO_None,
	DECO_Trunc,
	DECO_Shrub,
	DECO_Liana
};


//
// A decoration component.
//						
class FDecoComponent: public FExtraComponent
{
REGISTER_CLASS_H(FDecoComponent);
public:
	// Variables.
	Bool			bUnlit;
	Bool			bFlipH;
	Bool			bFlipV;
	math::Color		Color;	
	FTexture*		Texture;
	EDecoType		DecoType;
	Float			Frequency;
	Float			Amplitude;
	math::Rect		TexCoords;

	// FDecoComponent interface.
	FDecoComponent();

	// FObject interface.
	void EditChange();
	void SerializeThis( CSerializer& S );

	// CRenderAddon interface.
	void Render( CCanvas* Canvas );
};


/*-----------------------------------------------------------------------------
    FSpriteComponent.
-----------------------------------------------------------------------------*/

//
// A simple sprite.
// 
class FSpriteComponent: public FExtraComponent
{
REGISTER_CLASS_H(FSpriteComponent)
public:
	// Variables.
	Bool				bHidden;
	Bool				bUnlit;
	Bool				bFlipH;
	Bool				bFlipV;
	math::Color			Color;	
	FTexture*			Texture;
	math::Vector		Offset;
	math::Vector		Scale;
	math::Angle			Rotation;
	math::Rect			TexCoords;

	// FSpriteComponent interface.
	FSpriteComponent();

	// FComponent interface.
	void Render( CCanvas* Canvas );

	// FObject interface.
	void SerializeThis( CSerializer& S );
};


/*-----------------------------------------------------------------------------
    FLabelComponent.
-----------------------------------------------------------------------------*/

//
// A text label.
//
class FLabelComponent: public FExtraComponent
{
REGISTER_CLASS_H(FLabelComponent);
public:
	// Variables.
	math::Color		Color;	
	String			Text;
	FFont*			Font;
	Float			Scale;

	// FLabelComponent interface.
	FLabelComponent();

	// FComponent interface.
	void Render( CCanvas* Canvas );

	// FObject interface.
	void SerializeThis( CSerializer& S );
};


/*-----------------------------------------------------------------------------
    FRectComponent.
-----------------------------------------------------------------------------*/

//
// Rectangular scene object.
//
class FRectComponent: public FBaseComponent
{
REGISTER_CLASS_H(FRectComponent);
public:
	// FRectComponent interface.
	FRectComponent();

	// FComponent interface.
	void BeginPlay();
	math::Rect GetAABB();

	// CRenderable interface.
	void Render( CCanvas* Canvas );

private:
	// Rect internal.
	Bool		bShown;

	// Natives.
	void nativeIsShown( CFrame& Frame );
};


/*-----------------------------------------------------------------------------
    FZoneComponent.
-----------------------------------------------------------------------------*/

//
// An AABB area in the level.
//
class FZoneComponent: public FRectComponent
{
REGISTER_CLASS_H(FZoneComponent);
public:
	// FZoneComponent interface.
	FZoneComponent();

	// CRenderable interface.
	void Render( CCanvas* Canvas );
};


/*-----------------------------------------------------------------------------
    FJointComponent.
-----------------------------------------------------------------------------*/

//
// An abstract physics joint.
//
class FJointComponent: public FRectComponent
{
REGISTER_CLASS_H(FJointComponent);
public:
	// Variables.
	FEntity*		Body1;
	FEntity*		Body2;
	math::Vector	Hook1;
	math::Vector	Hook2;

	// FJointComponent interface.
	FJointComponent();

	// FComponent interface.
	void InitForEntity( FEntity* InEntity );

	// FObject interface.
	void SerializeThis( CSerializer& S );

	// CRenderAddon interface.
	void Render( CCanvas* Canvas );
};


/*-----------------------------------------------------------------------------
    FSpringComponent.
-----------------------------------------------------------------------------*/

//
// A physics spring.
//
class FSpringComponent: public FJointComponent
{
REGISTER_CLASS_H(FSpringComponent);
public:
	// Variables.
	Float			Damping;
	Float			Spring;
	Float			Length;
	Int32			NumSegs;
	FTexture*		Segment;
	Float			Width;

	// FSpringComponent interface.
	FSpringComponent();

	// CRenderAddon interface.
	void Render( CCanvas* Canvas );

	// FComponent interface.
	void PreTick( Float Delta );

	// FObject interface.
	void SerializeThis( CSerializer& S );
};


/*-----------------------------------------------------------------------------
    FSkyComponent.
-----------------------------------------------------------------------------*/

//
// Rectangular-Cylindrical sky projector.
//
class FSkyComponent: public FZoneComponent
{
REGISTER_CLASS_H(FSkyComponent);
public:
	// Variables.
	math::Vector	Parallax;
	Float			Extent;
	math::Vector	Offset;
	Float			RollSpeed;

	// FSkyComponent interface.
	FSkyComponent();

	// CRenderable interface.
	void Render( CCanvas* Canvas );

	// FComponent interface.
	void InitForEntity( FEntity* InEntity );

	// FObject interface.
	void EditChange();
	void SerializeThis( CSerializer& S );
};


/*-----------------------------------------------------------------------------
    FLightComponent.
-----------------------------------------------------------------------------*/

//
// Light maximum radius.
//
#define MAX_LIGHT_RADIUS		64.f


//
// A light source effect.
//
enum ELightType
{
	LIGHT_Steady,
	LIGHT_Flicker,
	LIGHT_Pulse,
	LIGHT_SoftPulse,
	LIGHT_SlowWave,
	LIGHT_FastWave,
	LIGHT_SpotLight,
	LIGHT_Searchlight,
	LIGHT_Fan,
	LIGHT_Disco,
	LIGHT_Flower,
	LIGHT_Hypnosis,
	LIGHT_Whirligig,
	LIGHT_MAX
};


//
// Lighting model.
//
enum ELightFunc
{
	LF_Additive,
	LF_Multiplicative
};


//
// A Light source. 
//
class FLightComponent: public FExtraComponent
{
REGISTER_CLASS_H(FLightComponent);
public:
	// Variables.
	math::Color		Color;
	Bool			bEnabled;
	ELightType		LightType;
	ELightFunc		LightFunc;
	Float			Radius;
	Float			Brightness;

	// Internal.
	FLightComponent*	NextLight;

	// FLightComponent interface.
	FLightComponent();
	~FLightComponent();
	math::Rect GetLightRect();

	// FComponent interface.
	void InitForEntity( FEntity* InEntity );
	void Render( CCanvas* Canvas );

	// FObject interface.
	void EditChange();
	void SerializeThis( CSerializer& S );
};


/*-----------------------------------------------------------------------------
    FParallaxLayerComponent.
-----------------------------------------------------------------------------*/

//
// A scrolling parallax layer.
//
class FParallaxLayerComponent: public FExtraComponent
{
REGISTER_CLASS_H(FParallaxLayerComponent);
public:
	// Variables.
	Bool			bUnlit;
	Bool			bFlipH;
	Bool			bFlipV;
	math::Color		Color;	
	FTexture*		Texture;
	math::Vector	Scale;
	math::Vector	Parallax;
	math::Vector	Gap;
	math::Vector	Offset;
	math::Rect		TexCoords;

	// FParallaxLayerComponent interface.
	FParallaxLayerComponent();

	// FComponent interface.
	void Render( CCanvas* Canvas );
	
	// FObject interface.
	void EditChange();
	void SerializeThis( CSerializer& S );
};


/*-----------------------------------------------------------------------------
    FInputComponent.
-----------------------------------------------------------------------------*/

//
// An input component, allow script to capture and process
// all input events, such as keyboard, mouse and joystick.
//
class FInputComponent: public FExtraComponent
{
REGISTER_CLASS_H(FInputComponent);
public:
	// Internal.
	FInputComponent*	NextInput;

	// FInputComponent interface.
	FInputComponent();
	~FInputComponent();

	// FComponent interface.
	void InitForEntity( FEntity* InEntity );
};


/*-----------------------------------------------------------------------------
    FEmitterComponent.
-----------------------------------------------------------------------------*/

//
// A information about single particle.
//
struct TParticle
{
public:
	math::Vector	Location;
	math::Angle		Rotation;
	math::Vector	Speed;
	Float			SpinRate;
	Float			Size;
	Float			Life;
	Float			MaxLifeInv;
	Float			Phase;
	UInt8			iTile;
};


//
// A particle parameter type.
//
enum EParticleParam
{
	PPT_Random,
	PPT_Linear
};


//
// An abstract emitter.
//
class FEmitterComponent: public FExtraComponent
{
REGISTER_CLASS_H(FEmitterComponent);
public:
	// Constants.
	enum{ MAX_PARTICLES	= 10000 };

	// Variables.
	Int32					MaxParticles;
	Float					LifeRange[2];
	math::Vector			SpawnArea;
	math::Vector			SpawnOffset;
	Int32					EmitPerSec;
	EParticleParam			SizeParam;
	Float					SizeRange[2];
	Bool					bUnlit;
	math::Color				Colors[3];
	Float					SpinRange[2];
	FTexture*				Texture;
	UInt8					NumUTiles;
	UInt8					NumVTiles;

	// FEmitterComponent interface.
	FEmitterComponent();
	~FEmitterComponent();
	virtual math::Rect GetCloudRect();
	virtual void UpdateEmitter( Float Delta );

	// CRenderAddon interface.
	void Render( CCanvas* Canvas );

	// FComponent interface.
	void InitForEntity( FEntity* InEntity );
	void Tick( Float Delta );
	void TickNonPlay( Float Delta );

	// FObject interface.
	void EditChange();
	void SerializeThis( CSerializer& S );
	void PostLoad();

protected:
	// Emitter internal.
	Array<TParticle>		Particles;
	Int32					NumPrts;
	Float					Accumulator;
};


/*-----------------------------------------------------------------------------
    FPhysEmitterComponent.
-----------------------------------------------------------------------------*/

//
// A physics emitter.
//
class FPhysEmitterComponent: public FEmitterComponent
{
REGISTER_CLASS_H(FPhysEmitterComponent);
public:
	// Variables.
	math::Vector			SpeedRange[2];
	math::Vector			Acceleration;

	// FEmitter interface.
	FPhysEmitterComponent();
	void UpdateEmitter( Float Delta );

	// FObject interface.
	void SerializeThis( CSerializer& S );
};


/*-----------------------------------------------------------------------------
    FLissajousEmitterComponent.
-----------------------------------------------------------------------------*/

//
// A Lissajous particles.
//
class FLissajousEmitterComponent: public FEmitterComponent
{
REGISTER_CLASS_H(FLissajousEmitterComponent);
public:
	// Variables.
	Float					Alpha, Beta;
	Float					Delta;
	Float					X, Y;

	// FEmitter interface.
	FLissajousEmitterComponent();
	void UpdateEmitter( Float Delta );

	// FObject interface.
	void SerializeThis( CSerializer& S );
};


/*-----------------------------------------------------------------------------
    FWeatherEmitterComponent.
-----------------------------------------------------------------------------*/

//
// A weather type.
//
enum EWeather
{
	WEATHER_Snow,
	WEATHER_Rain
};


//
// A Weather emitter.
//
class FWeatherEmitterComponent: public FEmitterComponent
{
REGISTER_CLASS_H(FWeatherEmitterComponent);
public:
	// Variables.
	EWeather				WeatherType;
	Float					SpeedRange[2];

	// FEmitter interface.
	FWeatherEmitterComponent();
	math::Rect GetCloudRect();
	void UpdateEmitter( Float Delta );

	// CRenderAddon interface.
	void Render( CCanvas* Canvas );

	// FObject interface.
	void SerializeThis( CSerializer& S );
};


/*-----------------------------------------------------------------------------
    FHingeComponent.
-----------------------------------------------------------------------------*/

//
// A hinge joint.
//
class FHingeComponent: public FJointComponent
{
REGISTER_CLASS_H(FHingeComponent);
public:
	// FHingeComponent interface.
	FHingeComponent();

	// CRenderAddon interface.
	void Render( CCanvas* Canvas );

	// CTickAddon interface.
	void PreTick( Float Delta );

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );
};


/*-----------------------------------------------------------------------------
    FPhysicComponent.
-----------------------------------------------------------------------------*/

//
// A body material.
//
enum EPhysMaterial
{
	PM_Custom,
	PM_Wood,
	PM_Rock,
	PM_Metal,
	PM_Ice,
	PM_Glass,
	PM_BouncyBall,
	PM_MAX
};


//
// An abstract physic body in level.
//
class FPhysicComponent: public FRectComponent
{
REGISTER_CLASS_H(FPhysicComponent);
public:
	// Variables.
	EPhysMaterial	Material;
	math::Vector	Velocity;
	Float			Mass;
	Float			Inertia;
	math::Vector	Forces;
	Float			AngVelocity;
	Float			Torque;
	FEntity*		Floor;
	FEntity*		Zone;
	FEntity*		Touched[4];

	// FPhysicComponent interface.
	FPhysicComponent();

	// FComponent interface.
	void InitForEntity( FEntity* InEntity );

	// FObject interface.
	void SerializeThis( CSerializer& S );

private:
	// Physic natives.
	void nativeSolveSolid( CFrame& Frame );
	void nativeSolveOneway( CFrame& Frame );
	void nativeIsTouching( CFrame& Frame );
};


/*-----------------------------------------------------------------------------
    FMoverComponent.
-----------------------------------------------------------------------------*/

//
// A moving platform.
//
class FMoverComponent: public FRectComponent
{
REGISTER_CLASS_H(FMoverComponent);
public:
	// Constants.
	enum { MAX_RIDERS = 8 };

	// FMoverComponent interface.
	FMoverComponent();
	Bool AddRider( FPhysicComponent* InRider );
	void Reset();

	// FComponent interface.
	void BeginPlay();
	void PreTick( Float Delta );

	// FObject interface.
	void SerializeThis( CSerializer& S );

private:
	// Mover internal.
	FPhysicComponent*	Riders[MAX_RIDERS];
	Int32				NumRds;
	math::Vector		OldLocation;
};


/*-----------------------------------------------------------------------------
    FArcadeBodyComponent.
-----------------------------------------------------------------------------*/

//
// An arcade physics body.
//
class FArcadeBodyComponent: public FPhysicComponent
{
REGISTER_CLASS_H(FArcadeBodyComponent);
public:
	// FArcadeBodyComponent interface.
	FArcadeBodyComponent();

	// FPhysicComponent interface.
	void PreTick( Float Delta );
	void Tick( Float Delta );
};


/*-----------------------------------------------------------------------------
    FRigidBodyComponent.
-----------------------------------------------------------------------------*/

//
// A rigid body.
//
class FRigidBodyComponent: public FPhysicComponent
{
REGISTER_CLASS_H(FRigidBodyComponent);
public:
	// Variables.
	Bool			bCanSleep;
	Bool			bSleeping;

	// FRigidBodyComponent interface.
	FRigidBodyComponent();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );
	void EditChange();

	// FPhysicComponent interface.
	void PreTick( Float Delta );
	void Tick( Float Delta );
};


/*-----------------------------------------------------------------------------
    FPainterComponent.
-----------------------------------------------------------------------------*/

//
// An component, allows script to render HUD.
// Or just draw.
//
class FPainterComponent: public FExtraComponent
{
REGISTER_CLASS_H(FPainterComponent);
public:
	// Internal.
	FPainterComponent* NextPainter;

	// FPainterComponent interface.
	FPainterComponent();
	~FPainterComponent();
	void RenderHUD( CCanvas* InCanvas );

	// FComponent interface.
	void InitForEntity( FEntity* InEntity );

	// FObject interface.
	void SerializeThis( CSerializer& S );

private:
	// All in-game variables.
	CCanvas*		Canvas;
	Float			Width;
	Float			Height;
	math::Color		Color;
	FFont*			Font;
	FTexture*		Texture;
	Float			Effect[10];
	TViewInfo		ViewInfo;

	// Natives.
	void nativePoint( CFrame& Frame );
	void nativeLine( CFrame& Frame );
	void nativeTile( CFrame& Frame );
	void nativeTextSize( CFrame& Frame );
	void nativeTextOut( CFrame& Frame );
	void nativePopEffect( CFrame& Frame );
	void nativePushEffect( CFrame& Frame );
	void nativeProject( CFrame& Frame );
	void nativeDeproject( CFrame& Frame );
};


/*-----------------------------------------------------------------------------
    FPuppetComponent.
-----------------------------------------------------------------------------*/

//
// A puppet watching side.
//
enum ELookDirection
{
	LOOK_None,		// Puppet has no eyes.
	LOOK_Left,		// Puppet looks to left.
	LOOK_Right,		// Puppet looks to right.
	LOOK_Both		// Puppet looks to both sides.
};


//
// An player or AI controlled puppet.
//
class FPuppetComponent: public FExtraComponent
{
REGISTER_CLASS_H(FPuppetComponent);
public:
	// General.
	Int32				Health;
	Int32				Clan;
	
	// Movement.
	Float				MoveSpeed;
	Float				JumpHeight;	
	Float				GravityScale;

	// Navigation.
	math::Vector		Goal;
	navi::EPathType		GoalReach;
	Float				GoalHint;

	// AI vision.
	enum { MAX_WATCHED	= 8 };
	ELookDirection		LookDirection;
	Float				LookPeriod;
	Float				LookRadius;

	// FPuppetComponent interface.
	FPuppetComponent();
	~FPuppetComponent();

	// FComponent interface.
	void InitForEntity( FEntity* InEntity );
	void BeginPlay();
	void Tick( Float Delta );

	// FObject interface.
	void SerializeThis( CSerializer& S );

	// Friends.
	friend class CNavigator;
	friend class FLevel;

private:
	// Puppet internal.
	FPuppetComponent*		NextPuppet;
	FArcadeBodyComponent*	Body;
	math::Vector			GoalStart;
	Float					LookCounter;
	FPuppetComponent*		LookList[MAX_WATCHED];
	Int32					iHoldenNode;
	Int32					iGoalNode;

	// Internal functions.
	void LookAtPuppets();
	Bool MoveToGoal();

	// Natives.
	void nativeSendOrder( CFrame& Frame );
	void nativeSuggestJumpHeight( CFrame& Frame );
	void nativeSuggestJumpSpeed( CFrame& Frame );
	void nativeMakeNoise( CFrame& Frame );
	void nativeIsVisible( CFrame& Frame );
	void nativeCreatePathTo( CFrame& Frame );
	void nativeCreateRandomPath( CFrame& Frame );
	void nativeMoveToGoal( CFrame& Frame ); // target


/*
	enum EMoveStatus
	{
		None,
		Aborted,
		Complete,
		InProgress
	};

	fn MoveToPoint( vector destination, float radius, float speedScale, float timeout );
	fn MoveToEntity( entity victim, float radius, float speedScale );



	EMoveStatus processMove();

*/

};


/*-----------------------------------------------------------------------------
	FSkeletonComponent.
-----------------------------------------------------------------------------*/

//
// A skeleton animation.
//
class FSkeletonComponent: public FExtraComponent
{
REGISTER_CLASS_H(FSkeletonComponent);
public:
	// Variables.
	Bool			bHidden;
	math::Color		Color;
	FSkeleton*		Skeleton;
	math::Vector	Scale;
	//	+offset and rotation.

	// FSkeletonComponent interface.
	FSkeletonComponent();

	// FComponent interface.
	void InitForEntity( FEntity* InEntity );
	void Tick( Float Delta );
	void Render( CCanvas* Canvas );

	// FObject interface.
	void SerializeThis( CSerializer& S );

private:
	// Internal.
	Float		Rate;
	Float		Frame;
	Int32		iAction;

	// Natives.
	void nativePlayAnim( CFrame& Frame );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/