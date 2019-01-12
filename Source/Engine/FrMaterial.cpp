/*=============================================================================
	FrMaterial.cpp: Material and Layers implementation.
	Created by Vlad Gordienko, Feb. 2018.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
	FMaterial implementation.
-----------------------------------------------------------------------------*/

// A minimal material dimention.
#define MINIMAL_MATERIAL_SIZE	16


//
// Material constructor.
//
FMaterial::FMaterial()
	:	FTexture(),
		Layers(),
		MainLayer(nullptr)
{
	// Set default size.
	UBits = VBits = IntLog2(MINIMAL_MATERIAL_SIZE);
	USize = VSize = 1 << UBits;
}


//
// Material destructor.
//
FMaterial::~FMaterial()
{
}


//
// Material serialization.
//
void FMaterial::SerializeThis( CSerializer& S )
{
	FTexture::SerializeThis(S);
	Serialize( S, Layers );
	Serialize( S, MainLayer );
	CLEANUP_ARR_NULL(Layers);
}


//
// Material's property changed.
//
void FMaterial::EditChange()
{
	FTexture::EditChange();

	// Update general material size.
	FDiffuseLayer* DiffuseMap;
	if( (DiffuseMap = As<FDiffuseLayer>(MainLayer)) && DiffuseMap->Bitmap )
	{
		UBits = DiffuseMap->Bitmap->UBits;
		VBits = DiffuseMap->Bitmap->VBits;
	}
	else
	{
		UBits = VBits = IntLog2(MINIMAL_MATERIAL_SIZE);
	}

	USize = 1 << UBits;
	VSize = 1 << VBits;
}


//
// Material after-loading initializaion.
//
void FMaterial::PostLoad()
{
	FTexture::PostLoad();

	// Update general material size.
	FDiffuseLayer* DiffuseMap;
	if( (DiffuseMap = As<FDiffuseLayer>(MainLayer)) && DiffuseMap->Bitmap )
	{
		UBits = DiffuseMap->Bitmap->UBits;
		VBits = DiffuseMap->Bitmap->VBits;
	}
	else
	{
		UBits = VBits = IntLog2(MINIMAL_MATERIAL_SIZE);
	}

	USize = 1 << UBits;
	VSize = 1 << VBits;
}


//
// Material import.
//
void FMaterial::Import( CImporterBase& Im )
{
	FTexture::Import(Im);
	IMPORT_OBJECT(MainLayer);
}


//
// Material export.
//
void FMaterial::Export( CExporterBase& Ex )
{
	FTexture::Export(Ex);
	EXPORT_OBJECT(MainLayer);
}


/*-----------------------------------------------------------------------------
	FDiffuseLayer implementation.
-----------------------------------------------------------------------------*/

//
// Diffuse layer constructor.
//
FDiffuseLayer::FDiffuseLayer()
	:	FMaterialLayer(),
		Bitmap(nullptr),
		BlendMode(BLEND_Regular),
		bInheritAlpha(false),
		bIgnoreMainBias(false),
		bUnlit(false),
		bFlipH(false),
		bFlipV(false),
		bTurn90(false),
		OverlayColor(COLOR_White),
		Panner(),
		Scaler(),
		Rotator(),
		Oscillator(),
		Parallax()
{
}


//
// Layer destructor.
//
FDiffuseLayer::~FDiffuseLayer()
{
}


//
// Apply layer transforms.
//
void FDiffuseLayer::ApplyTransform( const TViewInfo& View, const TVector* InCoords, TVector* OutCoords, Int32 NumVerts )
{
	Float Time = GPlat->TimeStamp();

	if( bIgnoreMainBias )
	{
		// Untransfrom it.
		for( Int32 i=0; i<NumVerts; i++ )
			OutCoords[i] = InCoords[i] - InCoords[0];
	}
	else
	{
		// Transfrom with master.
		for( Int32 i=0; i<NumVerts; i++ )
			OutCoords[i] = InCoords[i];
	}

	//
	// Apply scale.
	//
	if( Scaler.Scale.X != 1.f || Scaler.Scale.Y != 1.f )
	{
		for( Int32 i=0; i<NumVerts; i++ )
		{
			OutCoords[i].X *= Scaler.Scale.X;
			OutCoords[i].Y *= Scaler.Scale.Y;
		}
	}

	//
	// Apply rotation.
	//
	if( Rotator.Speed != 0.f )
	{
		TAngle Angle = TAngle::FromRads(Rotator.Speed*Time);
		TCoords Coords = TCoords( Rotator.Origin, Angle );
		for( Int32 i=0; i<NumVerts; i++ )
			OutCoords[i] = TransformPointBy(OutCoords[i], Coords);
	}

	// 
	// Apply panner.
	//
	if( Panner.Speed != 0.f )
	{
		TVector PanVector = AngleToVector(Panner.Direction);
		for( Int32 i=0; i<NumVerts; i++ )
			OutCoords[i] += PanVector * Time * Panner.Speed;
	}
	if( Oscillator.Amplitude.X != 0.f || Oscillator.Amplitude.Y != 0.f )
	{
		for( Int32 i=0; i<NumVerts; i++ )
		{
			OutCoords[i].X += FastSinF(2.f*PI * Time*Oscillator.Frequency.X + 2.f*PI * Oscillator.Phase.X) * Oscillator.Amplitude.X;
			OutCoords[i].Y += FastSinF(2.f*PI * Time*Oscillator.Frequency.Y + 2.f*PI * Oscillator.Phase.Y) * Oscillator.Amplitude.Y;
		}
	}

	//
	// Apply flip and turn.
	//
	if( bFlipH )
	{
		for( Int32 i=0; i<NumVerts; i++ )
			OutCoords[i].X = -OutCoords[i].X;
	}
	if( bFlipV )
	{
		for( Int32 i=0; i<NumVerts; i++ )
			OutCoords[i].Y = -OutCoords[i].Y;
	}
	if( bTurn90 )
	{
		for( Int32 i=0; i<NumVerts; i++ )
			Exchange(OutCoords[i].X, OutCoords[i].Y);
	}
}


//
// When property changed.
//
void FDiffuseLayer::EditChange()
{
	FMaterialLayer::EditChange();
}


//
// Layer after loading initialization.
//
void FDiffuseLayer::PostLoad()
{
	FMaterialLayer::PostLoad();
}


//
// Layer serialization.
//
void FDiffuseLayer::SerializeThis( CSerializer& S )
{
	FMaterialLayer::SerializeThis(S);

	Serialize( S, Bitmap );
	SerializeEnum( S, BlendMode );
	Serialize( S, bInheritAlpha );
	Serialize( S, bIgnoreMainBias );
	Serialize( S, bUnlit );
	Serialize( S, bFlipH );
	Serialize( S, bFlipV );
	Serialize( S, bTurn90 );
	Serialize( S, OverlayColor );

	// Assumed all parameters are POD-structs.
	S.SerializeData( &Panner, sizeof(TPannerParam) );
	S.SerializeData( &Scaler, sizeof(TScalerParam) );
	S.SerializeData( &Rotator, sizeof(TRotatorParam) );
	S.SerializeData( &Oscillator, sizeof(TOscillatorParam) );
	S.SerializeData( &Parallax, sizeof(TParallaxParam) );
}


/*-----------------------------------------------------------------------------
	FMaterialLayer implementation.
-----------------------------------------------------------------------------*/

//
// Layer constructor.
//
FMaterialLayer::FMaterialLayer()
	:	FModifier()
{
}


//
// Layer destructor.
//
FMaterialLayer::~FMaterialLayer()
{
}


//
// Return layer's material.
//
FMaterial* FMaterialLayer::Material() const
{
	FMaterial* Material = As<FMaterial>(Owner);
	assert(Material);
	return Material;
}


//
// Serialize layer.
//
void FMaterialLayer::SerializeThis( CSerializer& S )
{
	FModifier::SerializeThis(S);
}


/*-----------------------------------------------------------------------------
	Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FMaterial, FTexture, CLASS_None )
{
}

REGISTER_CLASS_CPP( FMaterialLayer, FModifier, CLASS_Abstract )
{
}

REGISTER_CLASS_CPP( FDiffuseLayer, FMaterialLayer, CLASS_None )
{
	BEGIN_STRUCT(TPannerParam)
		STRUCT_MEMBER(Direction);
		STRUCT_MEMBER(Speed);
	END_STRUCT;

	BEGIN_STRUCT(TScalerParam)
		STRUCT_MEMBER(Scale);
	END_STRUCT;

	BEGIN_STRUCT(TRotatorParam)
		STRUCT_MEMBER(Speed);
		STRUCT_MEMBER(Origin);
	END_STRUCT;

	BEGIN_STRUCT(TOscillatorParam)
		STRUCT_MEMBER(Amplitude);
		STRUCT_MEMBER(Frequency);
		STRUCT_MEMBER(Phase);
	END_STRUCT;

	BEGIN_STRUCT(TParallaxParam)
		STRUCT_MEMBER(Coefficient);
	END_STRUCT;

	ADD_PROPERTY(Bitmap, PROP_Editable);
	ADD_PROPERTY(BlendMode, PROP_Editable);
	ADD_PROPERTY(bInheritAlpha, PROP_Editable);
	ADD_PROPERTY(bIgnoreMainBias, PROP_Editable);
	ADD_PROPERTY(bUnlit, PROP_Editable);
	ADD_PROPERTY(bFlipH, PROP_Editable);
	ADD_PROPERTY(bFlipV, PROP_Editable);
	ADD_PROPERTY(bTurn90, PROP_Editable);
	ADD_PROPERTY(OverlayColor, PROP_Editable);
	ADD_PROPERTY(Panner, PROP_Editable);
	ADD_PROPERTY(Scaler, PROP_Editable);
	ADD_PROPERTY(Rotator, PROP_Editable);
	ADD_PROPERTY(Oscillator, PROP_Editable);
	ADD_PROPERTY(Parallax, PROP_Editable);
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/