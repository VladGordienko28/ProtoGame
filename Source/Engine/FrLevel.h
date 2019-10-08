/*=============================================================================
    FrLevel.h: A game level definition.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Declarations.
-----------------------------------------------------------------------------*/

//
// A level rendering flags.
//
#define RND_None			0x000000
#define RND_Grid			0x000001
#define RND_Backdrop		0x000002
#define RND_Hidden			0x000004
#define RND_Logic			0x000008
#define RND_Particles		0x000010
#define RND_Portals			0x000020
#define RND_Lighting		0x000040
#define RND_Model			0x000080
#define RND_Stats			0x000100
#define RND_Other			0x000200
#define RND_HUD				0x000400
#define RND_Effects			0x000800
#define RND_Paths			0x001000
#define RND_Environment		0x002000

// Combo flags.
#define RND_Editor			RND_Grid | RND_Backdrop | RND_Hidden | RND_Logic | RND_Particles |\
							RND_Portals | RND_Lighting | RND_Model | RND_Stats | RND_Other | RND_Effects | RND_Paths |\
							RND_Environment


#define RND_Game			RND_Backdrop | RND_Particles | RND_Portals | RND_Lighting | RND_Model | RND_HUD |\
							RND_Effects | RND_Environment


/*-----------------------------------------------------------------------------
	TCamera.
-----------------------------------------------------------------------------*/

//
// A 2D camera.
//
struct TCamera
{
public:
	// Variables.
	math::Vector	Location;
	math::Angle		Rotation;
	math::Vector	FOV;
	Float			Zoom;
	math::Rect		ScrollBound;

	// TCamera interface.
	TCamera();
	math::Vector GetFitFOV( Float ScreenX, Float ScreenY ) const;
	friend void Serialize( CSerializer& S, TCamera& V );
};


/*-----------------------------------------------------------------------------
    FLevel.
-----------------------------------------------------------------------------*/

//
// A Level.
//
class FLevel: public FResource
{
REGISTER_CLASS_H(FLevel)
public:
	// Variables.
	FLevel*					Original;
	UInt32					RndFlags;

	// Database.
	Array<FEntity*>			Entities;

	// Fast access tables.
	Array<FComponent*>		RenderObjects;
	Array<FComponent*>		TickObjects;

	// Linked list of often used objects.
	FInputComponent*		FirstInput;
	FPortalComponent*		FirstPortal;
	FPuppetComponent*		FirstPuppet;
	FLightComponent*		FirstLight;
	FLogicComponent*		FirstLogicElement;
	FPainterComponent*		FirstPainter;

	// Level objects
	TCamera					Camera;
	FSkyComponent*			Sky;
	CCollisionHash*			CollHash;
	CGFXManager*			GFXManager;
	navi::Navigator m_navigator;

	// Level variables.
	Bool					bIsPlaying;
	Bool					bIsPause;
	Float					GameSpeed;
	FMusic*					Soundtrack;
	Float					Effect[10];
	math::Color				AmbientLight;
	Float					BlurIntensity;



	// temporary
	// -----------

	Float					AberrationIntensity = 0.f;

	math::InterpCurve<math::Color> m_ambientColors;



	FBitmap* m_midnightBitmap = nullptr;
	FBitmap* m_dawnBitmap = nullptr;
	FBitmap* m_noonBitmap = nullptr;
	FBitmap* m_duskBitmap = nullptr;

	fx::Vignette m_vignette;
	Bool m_enableFXAA = true;  // not here, but in config;


	// -----------

	// -------

	envi::Environment m_environment;
	envi::EnvironmentContext m_environmentContext;

	// FLevel interface.
	FLevel();
	~FLevel();

	// Level functions.
	void BeginPlay();
	void EndPlay();
	void Tick( Float Delta );

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void PostLoad();
	void EditChange() override;

	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

	// Entity functions.
	FEntity* CreateEntity( FScript* InScript, String InName, math::Vector InLocation );
	void DestroyEntity( FEntity* Entity );
	FEntity* FindEntity( String InName );
	Int32 GetEntityIndex( FEntity* Entity );
	void ReleaseEntity( Int32 iEntity );

	// Collisions.
	FBrushComponent* TestPointGeom( const math::Vector& P );
	FBrushComponent* TestLineGeom( const math::Vector& A, const math::Vector& B, Bool bFast, math::Vector* Hit = nullptr, math::Vector* Normal = nullptr );

	// Accessors.
	inline Bool IsTemporal()
	{
		return Original != nullptr;
	}

public:
	gfx::GridDrawer::UPtr m_gridDrawer;

	// Level rendering
	void renderLevel( CCanvas* canvas, Int32 x, Int32 y, Int32 width, Int32 height );


};


/*-----------------------------------------------------------------------------
    TIncomingLevel.
-----------------------------------------------------------------------------*/

//
// An information about next incoming level.
//
struct TIncomingLevel
{
public:
	// Variables.
	Bool		bCopy;			// Whether should duplicate destination level to play on it.
	FLevel*		Destination;	// Incoming level.
	FEntity*	Teleportee;		// Entity to travel between source and destination levels, or null.

	// TIncomingLevel interface.
	TIncomingLevel()
		:	bCopy( false ),
			Destination( nullptr ),
			Teleportee( nullptr )
	{}
	operator Bool() const
	{
		return Destination != nullptr;
	}
};


//
// An information about incoming level.
// When player want to change level, just
// initialize this variables. It's should
// be global, since we can't change level during
// update loop. So don't forget to track this variable
// after the update loop.
//
extern TIncomingLevel	GIncomingLevel;


//
// A static buffer is a little database to store
// trans-level values, such as scores, big lifes,
// and so on. When level had finished old Player saves
// required variables here, and then when new level 
// had launched - new player restore this values
// from here. Enjoy..
//
extern Map<String, String>	GStaticBuffer;


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/