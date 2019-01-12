/*=============================================================================
	FrMaterial.h: Cool materials.
	Created by Vlad Gordienko, Feb. 2018.
=============================================================================*/

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
	TArray<FMaterialLayer*>	Layers;
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
		TAngle	Direction = 0;
		Float	Speed = 0.f;
	};
	struct TScalerParam
	{
		TVector	Scale = TVector(1.f, 1.f);
	};
	struct TRotatorParam
	{
		Float	Speed = 0.f;
		TVector	Origin = TVector(0.f, 0.f);
	};
	struct TOscillatorParam
	{
		TVector	Amplitude = TVector(0.f, 0.f);
		TVector Frequency = TVector(0.f, 0.f);
		TVector	Phase = TVector(0.f, 0.f);
	};
	struct TParallaxParam
	{
		TVector Coefficient = TVector(0.f, 0.f);
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
	TColor				OverlayColor;

	// Params.
	TPannerParam		Panner;
	TScalerParam		Scaler;
	TRotatorParam		Rotator;
	TOscillatorParam	Oscillator;
	TParallaxParam		Parallax;

	// FDiffuseLayer interface.
	FDiffuseLayer();
	~FDiffuseLayer();
	void ApplyTransform( const TViewInfo& View, const TVector* InCoords, TVector* OutCoords, Int32 NumVerts );

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/