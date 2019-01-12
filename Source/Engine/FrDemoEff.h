/*=============================================================================
    FrDemoEff.h: Demo scene effects.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    FDemoBitmap.
-----------------------------------------------------------------------------*/

//
// An abstract demo-scene effect.
//
class FDemoBitmap: public FBitmap
{
REGISTER_CLASS_H( FDemoBitmap );
public:
	// Variables.
	Byte*			EffectPtr;
	DWord			UMask;
	DWord			VMask;
	DWord			Phase;
	FBitmap*		PaletteRef;

	// FDemoBitmap interface.
	FDemoBitmap();
	~FDemoBitmap();

	// FBitmap interface.
	void Init( Integer InU, Integer InV );
	void* GetData();
	SizeT GetBlockSize();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
};


/*-----------------------------------------------------------------------------
    FPlasmaBitmap.
-----------------------------------------------------------------------------*/

//
// An oldschool plasma effect.
//
class FPlasmaBitmap: public FDemoBitmap
{
REGISTER_CLASS_H( FPlasmaBitmap );
public:
	// Variables.
	Float		PlasmaA;
	Float		PlasmaB;
	Float		PlasmaC;
	Float		PlasmaD;

	// FPlasmaBitmap interface.
	FPlasmaBitmap();
	~FPlasmaBitmap();

	// FBitmap interface.
	void Init( Integer InU, Integer InV );
	void Erase();
	void MouseClick( Integer Button, Integer X, Integer Y );
	void MouseMove( Integer Button, Integer X, Integer Y );
	void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();

private:
	// Plasma internal.
	Integer		PaletteShift;
	Byte		PlasmaTable[4][256];
	Byte*		HeatBuffer;

	void SetPlasmaTable();
	void CalculatePlasma();
	void RenderPlasma();
};


/*-----------------------------------------------------------------------------
    FFireBitmap.
-----------------------------------------------------------------------------*/

// Maximum sparks per fire bitmap.
#define MAX_FIRE_SPARKS		1024


//
// A cool fire effects.
//
class FFireBitmap: public FDemoBitmap
{
REGISTER_CLASS_H( FFireBitmap );
public:
	// Spark types.
	enum ESparkType
	{
		SPARK_Point,
		SPARK_RandomPoint,
		SPARK_Phase,
		SPARK_Jitter,
		SPARK_Twister,
		SPARK_Fireball,
		SPARK_JetUpward,
		SPARK_JetLeftward,
		SPARK_JetRightward,
		SPARK_JetDownward,
		SPARK_Spermatozoa,
		SPARK_Whirligig,
		SPARK_Cloud,
		SPARK_LineLighting,
		SPARK_RampLighting,
		SPARK_RandomLighting,
		SPARK_BallLighting,
		_SPARK_Particle,
		_SPARK_Orbit
	};

	// Spark type.
	struct TSpark
	{
	public:
		Byte		Type;
		Byte		X;
		Byte		Y;
		Byte		Heat;
		Byte		ParamA;
		Byte		ParamB;
		Byte		ParamC;
		Byte		ParamD;
	};

	// Fire-draw params.
	struct TFireDrawParams
	{
	public:
		ESparkType	DrawType;
		Byte		Heat;
		Byte		Life;
		Byte		Frequency;
		Byte		Size;
		Byte		Area;
		Byte		Direction;
		Byte		Speed;
	};

	// Variables.
	Bool			bRising;
	Byte			FireHeat;
	TFireDrawParams	DrawParams;

	// FFireBitmap interface.
	FFireBitmap();

	// FBitmap interface.
	void Init( Integer InU, Integer InV );
	void Erase();
	void MouseClick( Integer Button, Integer X, Integer Y );
	void MouseMove( Integer Button, Integer X, Integer Y );
	void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

private:
	// Fire internal.
	Byte		FireTable[512];
	Integer		NumSparks;
	TSpark		Sparks[MAX_FIRE_SPARKS];

	// FFireBitmap interface.
	void AddSpark( Integer X, Integer Y );
	void DeleteSparks( Integer X, Integer Y, Integer Size );
	void RedrawSparks();
	void RenderStillFire();		
	void RenderRisingFire();
	void SetFireTable();
	void DrawLighting( Integer X1, Integer Y1, Integer X2, Integer Y2, Byte Heat1, Byte Heat2 );
};


/*-----------------------------------------------------------------------------
    FWaterBitmap.
-----------------------------------------------------------------------------*/

// Maximum drops per water bitmap.
#define MAX_WATER_DROPS		128


//
// A cool water effects.
//
class FWaterBitmap: public FDemoBitmap
{
REGISTER_CLASS_H( FWaterBitmap );
public:
	// Drop types.
	enum EDropType
	{
		DROP_Point,
		DROP_RandomPoint,
		DROP_Tap,
		DROP_Surfer,
		DROP_RainDrops,
		DROP_Oscillator,
		DROP_VertLine,
		DROP_HorizLine
	};

	// Drop type.
	struct TDrop
	{
	public:
		Byte		Type;
		Byte		X;
		Byte		Y;
		Byte		Depth;
		Byte		ParamA;
		Byte		ParamB;
		Byte		ParamC;
		Byte		ParamD;
	};

	// Water-draw params.
	struct TWaterDrawParams
	{
	public:
		EDropType	DrawType;
		Byte		Depth;
		Byte		Amplitude;
		Byte		Frequency;
		Byte		Size;
		Byte		Speed;
	};

	// Variables.
	Byte				WaterAmpl;
	FBitmap*			Image;
	TWaterDrawParams	DrawParams;

	// FWaterBitmap interface.
	FWaterBitmap();
	~FWaterBitmap();

	// FBitmap interface.
	void Init( Integer InU, Integer InV );
	void Erase();
	void MouseClick( Integer Button, Integer X, Integer Y );
	void MouseMove( Integer Button, Integer X, Integer Y );
	void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

private:
	// Water internal.
	Byte		DistTable[1024];
	Integer		NumDrops;
	TDrop		Drops[MAX_WATER_DROPS];
	Byte*		ZBuffer;

	// FWaterBitmap interface.
	void AddDrop( Integer X, Integer Y );
	void DeleteDrops( Integer X, Integer Y, Integer Size );
	void RedrawDrops();
	void CalculateWater();
	void RenderWater();
	void SetDistortionTable();
};


/*-----------------------------------------------------------------------------
	FTechBitmap.
-----------------------------------------------------------------------------*/

// Maximum tech panels per bitmap.
#define MAX_TECH_PANELS		128


//
// A cool tech effects.
//
class FTechBitmap: public FDemoBitmap
{
REGISTER_CLASS_H(FTechBitmap);
public:
	// Panel types.
	enum EPanelType
	{
		TECH_Ivy,
		TECH_Circle,
		TECH_Straight,
		TECH_Segments,
		TECH_Grinder,
		TECH_Noisy,
		TECH_Wave,
		_TECH_Ivy1,
		_TECH_Ivy2,
	};

	// Panel type.
	struct TPanel
	{
		Byte	Type;
		Byte	X;
		Byte	Y;
		Byte	Depth;
		Byte	ParamA;
		Byte	ParamB;
		Byte	ParamC;
		Byte	ParamD;
	};

	// Tech-draw params.
	struct TTechDrawParams
	{
	public:
		EPanelType	DrawType;
		Byte		Depth;
		Byte		Size;
		Byte		Time;
	};

	// Variables.
	Byte			BumpMapLight;
	Byte			BumpMapAngle;
	TTechDrawParams	DrawParams;

	// FTechBitmap interface.
	FTechBitmap();
	~FTechBitmap();

	// FBitmap interface.
	void Init( Integer InU, Integer InV );
	void Erase();
	void MouseClick( Integer Button, Integer X, Integer Y );
	void MouseMove( Integer Button, Integer X, Integer Y );
	void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

private:
	// Tech internal.
	Byte		LightTable[580];
	Integer		NumPanels;
	TPanel		Panels[MAX_TECH_PANELS];
	Byte*		ZBuffer;

	// FTechBitmap interface.
	void AddPanel( Integer X, Integer Y );
	void DeletePanels( Integer X, Integer Y, Integer Area );
	void RedrawPanels();
	void CalculateBumpMap();
	void SetLigthTable();

	void ApplyFilter1( Integer X, Integer Y, Integer Area );
	void ApplyFilter2( Integer X, Integer Y, Integer Area );
};


/*-----------------------------------------------------------------------------
    FGlassBitmap.
-----------------------------------------------------------------------------*/

//
// Glass effect.
//
class FGlassBitmap: public FDemoBitmap
{
REGISTER_CLASS_H( FGlassBitmap );
public:
	// Variables.
	Bool			bMoveGlass;
	FBitmap*		Glass;
	FBitmap*		Image;
	Float			HSpeed;
	Float			VSpeed;

	// FGlassBitmap interface.
	FGlassBitmap();
	~FGlassBitmap();

	// FBitmap interface.
	void Init( Integer InU, Integer InV );
	void Erase();
	void MouseClick( Integer Button, Integer X, Integer Y );
	void MouseMove( Integer Button, Integer X, Integer Y );
	void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

private:
	// Glass internal.
	Byte			VOffset;
	Byte			HOffset;

	void RenderGlassI();
	void RenderGlassII();
};


/*-----------------------------------------------------------------------------
    FHarmonicBitmap.
-----------------------------------------------------------------------------*/

// Wave direction.
enum EWaveDirection
{
	WAVE_Horizontal,
	WAVE_Vertical
};


//
// Harmonic wave effect.
//
class FHarmonicBitmap: public FDemoBitmap
{
REGISTER_CLASS_H( FHarmonicBitmap );
public:
	// Variables.
	EWaveDirection	Direction;
	Byte			WaveAmpl;
	Byte			WaveFreq;
	FBitmap*		Image;

	// FHarmonicBitmap interface.
	FHarmonicBitmap();
	~FHarmonicBitmap();

	// FBitmap interface.
	void Init( Integer InU, Integer InV );
	void Erase();
	void MouseClick( Integer Button, Integer X, Integer Y );
	void MouseMove( Integer Button, Integer X, Integer Y );
	void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

private:
	// Harmonic internal.
	Byte			VOffset;
	Byte			HOffset;

	void RenderHarmonicH();
	void RenderHarmonicV();
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/