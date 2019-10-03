/*=============================================================================
	FrMaterial.h: Cool materials.
	Created by Vlad Gordienko, Feb. 2018.
=============================================================================*/

#define MATERIAL_ENABLED 0

#if MATERIAL_ENABLED

/*-----------------------------------------------------------------------------
	FMaterial.
-----------------------------------------------------------------------------*/

//
// A multi-layer texture.
//
class FMaterial: public FTexture
{
REGISTER_CLASS_H(FMaterial);
public:
	// Variables.
	Array<FMaterialLayer*>	Layers;
	FMaterialLayer*	MainLayer;

	// FMaterial interface.
	FMaterial();
	~FMaterial();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );
};


/*-----------------------------------------------------------------------------
	FMaterialLayer.
-----------------------------------------------------------------------------*/

//
// An abstract material layer.
//
class FMaterialLayer: public FModifier
{
REGISTER_CLASS_H(FMaterialLayer);
public:
	// FMaterialLayer interface.
	FMaterialLayer();
	~FMaterialLayer();
	FMaterial* Material() const;

	// FObject interface.
	void SerializeThis( CSerializer& S );
};


/*-----------------------------------------------------------------------------
	FDiffuseLayer.
-----------------------------------------------------------------------------*/

//
// Simple bitmap-like layer with many editable parameters.
//
class FDiffuseLayer: public FMaterialLayer
{
REGISTER_CLASS_H(FDiffuseLayer);
public:
	// Internal structs.
	struct TPannerParam
	{
		math::Angle	Direction = 0;
		Float	Speed = 0.f;
	};
	struct TScalerParam
	{
		math::Vector	Scale = math::Vector(1.f, 1.f);
	};
	struct TRotatorParam
	{
		Float	Speed = 0.f;
		math::Vector	Origin = math::Vector(0.f, 0.f);
	};
	struct TOscillatorParam
	{
		math::Vector Amplitude = math::Vector(0.f, 0.f);
		math::Vector Frequency = math::Vector(0.f, 0.f);
		math::Vector Phase = math::Vector(0.f, 0.f);
	};
	struct TParallaxParam
	{
		math::Vector Coefficient = math::Vector(0.f, 0.f);
	};


	// General variables.
	FBitmap*			Bitmap;
	EBitmapBlend		BlendMode;
	Bool				bInheritAlpha;
	Bool				bIgnoreMainBias;
	Bool				bUnlit;
	Bool				bFlipH;
	Bool				bFlipV;
	Bool				bTurn90;
	math::Color			OverlayColor;

	// Params.
	TPannerParam		Panner;
	TScalerParam		Scaler;
	TRotatorParam		Rotator;
	TOscillatorParam	Oscillator;
	TParallaxParam		Parallax;

	// FDiffuseLayer interface.
	FDiffuseLayer();
	~FDiffuseLayer();
	void ApplyTransform( const gfx::ViewInfo& View, const math::Vector* InCoords, math::Vector* OutCoords, Int32 NumVerts );

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
};

#endif

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/