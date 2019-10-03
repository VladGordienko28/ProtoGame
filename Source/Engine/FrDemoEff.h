/*=============================================================================
    FrDemoEff.h: Demo scene effects.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#define DEMO_EFFECTS_ENABLED 0

#if DEMO_EFFECTS_ENABLED

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
	UInt8*			EffectPtr;
	UInt32			UMask;
	UInt32			VMask;
	UInt32			Phase;
	FBitmap*		PaletteRef;

	// FDemoBitmap interface.
	FDemoBitmap();
	~FDemoBitmap();

	// FBitmap interface.
	void Init( Int32 InU, Int32 InV );
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
	void Init( Int32 InU, Int32 InV );
	void Erase();
	void MouseClick( Int32 Button, Int32 X, Int32 Y );
	void MouseMove( Int32 Button, Int32 X, Int32 Y );
	void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();

private:
	// Plasma internal.
	Int32		PaletteShift;
	UInt8		PlasmaTable[4][256];
	UInt8*		HeatBuffer;

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
		UInt8		Type;
		UInt8		X;
		UInt8		Y;
		UInt8		Heat;
		UInt8		ParamA;
		UInt8		ParamB;
		UInt8		ParamC;
		UInt8		ParamD;
	};

	// Fire-draw params.
	struct TFireDrawParams
	{
	public:
		ESparkType	DrawType;
		UInt8		Heat;
		UInt8		Life;
		UInt8		Frequency;
		UInt8		Size;
		UInt8		Area;
		UInt8		Direction;
		UInt8		Speed;
	};

	// Variables.
	Bool			bRising;
	UInt8			FireHeat;
	TFireDrawParams	DrawParams;

	// FFireBitmap interface.
	FFireBitmap();

	// FBitmap interface.
	void Init( Int32 InU, Int32 InV );
	void Erase();
	void MouseClick( Int32 Button, Int32 X, Int32 Y );
	void MouseMove( Int32 Button, Int32 X, Int32 Y );
	void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

private:
	// Fire internal.
	UInt8		FireTable[512];
	Int32		NumSparks;
	TSpark		Sparks[MAX_FIRE_SPARKS];

	// FFireBitmap interface.
	void AddSpark( Int32 X, Int32 Y );
	void DeleteSparks( Int32 X, Int32 Y, Int32 Size );
	void RedrawSparks();
	void RenderStillFire();		
	void RenderRisingFire();
	void SetFireTable();
	void DrawLighting( Int32 X1, Int32 Y1, Int32 X2, Int32 Y2, UInt8 Heat1, UInt8 Heat2 );
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
		UInt8		Type;
		UInt8		X;
		UInt8		Y;
		UInt8		Depth;
		UInt8		ParamA;
		UInt8		ParamB;
		UInt8		ParamC;
		UInt8		ParamD;
	};

	// Water-draw params.
	struct TWaterDrawParams
	{
	public:
		EDropType	DrawType;
		UInt8		Depth;
		UInt8		Amplitude;
		UInt8		Frequency;
		UInt8		Size;
		UInt8		Speed;
	};

	// Variables.
	UInt8				WaterAmpl;
	FBitmap*			Image;
	TWaterDrawParams	DrawParams;

	// FWaterBitmap interface.
	FWaterBitmap();
	~FWaterBitmap();

	// FBitmap interface.
	void Init( Int32 InU, Int32 InV );
	void Erase();
	void MouseClick( Int32 Button, Int32 X, Int32 Y );
	void MouseMove( Int32 Button, Int32 X, Int32 Y );
	void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

private:
	// Water internal.
	UInt8		DistTable[1024];
	Int32		NumDrops;
	TDrop		Drops[MAX_WATER_DROPS];
	UInt8*		ZBuffer;

	// FWaterBitmap interface.
	void AddDrop( Int32 X, Int32 Y );
	void DeleteDrops( Int32 X, Int32 Y, Int32 Size );
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
		UInt8	Type;
		UInt8	X;
		UInt8	Y;
		UInt8	Depth;
		UInt8	ParamA;
		UInt8	ParamB;
		UInt8	ParamC;
		UInt8	ParamD;
	};

	// Tech-draw params.
	struct TTechDrawParams
	{
	public:
		EPanelType	DrawType;
		UInt8		Depth;
		UInt8		Size;
		UInt8		Time;
	};

	// Variables.
	UInt8			BumpMapLight;
	UInt8			BumpMapAngle;
	TTechDrawParams	DrawParams;

	// FTechBitmap interface.
	FTechBitmap();
	~FTechBitmap();

	// FBitmap interface.
	void Init( Int32 InU, Int32 InV );
	void Erase();
	void MouseClick( Int32 Button, Int32 X, Int32 Y );
	void MouseMove( Int32 Button, Int32 X, Int32 Y );
	void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

private:
	// Tech internal.
	UInt8		LightTable[580];
	Int32		NumPanels;
	TPanel		Panels[MAX_TECH_PANELS];
	UInt8*		ZBuffer;

	// FTechBitmap interface.
	void AddPanel( Int32 X, Int32 Y );
	void DeletePanels( Int32 X, Int32 Y, Int32 Area );
	void RedrawPanels();
	void CalculateBumpMap();
	void SetLigthTable();

	void ApplyFilter1( Int32 X, Int32 Y, Int32 Area );
	void ApplyFilter2( Int32 X, Int32 Y, Int32 Area );
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
	void Init( Int32 InU, Int32 InV );
	void Erase();
	void MouseClick( Int32 Button, Int32 X, Int32 Y );
	void MouseMove( Int32 Button, Int32 X, Int32 Y );
	void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

private:
	// Glass internal.
	UInt8			VOffset;
	UInt8			HOffset;

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
	UInt8			WaveAmpl;
	UInt8			WaveFreq;
	FBitmap*		Image;

	// FHarmonicBitmap interface.
	FHarmonicBitmap();
	~FHarmonicBitmap();

	// FBitmap interface.
	void Init( Int32 InU, Int32 InV );
	void Erase();
	void MouseClick( Int32 Button, Int32 X, Int32 Y );
	void MouseMove( Int32 Button, Int32 X, Int32 Y );
	void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

private:
	// Harmonic internal.
	UInt8			VOffset;
	UInt8			HOffset;

	void RenderHarmonicH();
	void RenderHarmonicV();
};

#endif

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/