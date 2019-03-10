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
		OverlayColor(math::colors::WHITE),
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
void FDiffuseLayer::ApplyTransform( const TViewInfo& View, const math::Vector* InCoords, math::Vector* OutCoords, Int32 NumVerts )
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
	if( Scaler.Scale.x != 1.f || Scaler.Scale.y != 1.f )
	{
		for( Int32 i=0; i<NumVerts; i++ )
		{
			OutCoords[i].x *= Scaler.Scale.x;
			OutCoords[i].y *= Scaler.Scale.y;
		}
	}

	//
	// Apply rotation.
	//
	if( Rotator.Speed != 0.f )
	{
		math::Angle Angle = math::Angle::fromRads(Rotator.Speed*Time);
		math::Coords Coords = math::Coords( Rotator.Origin, Angle );
		for( Int32 i=0; i<NumVerts; i++ )
			OutCoords[i] = math::transformPointBy(OutCoords[i], Coords);
	}

	// 
	// Apply panner.
	//
	if( Panner.Speed != 0.f )
	{
		math::Vector PanVector = math::angleToVector(Panner.Direction);
		for( Int32 i=0; i<NumVerts; i++ )
			OutCoords[i] += PanVector * Time * Panner.Speed;
	}
	if( Oscillator.Amplitude.x != 0.f || Oscillator.Amplitude.y != 0.f )
	{
		for( Int32 i=0; i<NumVerts; i++ )
		{
			OutCoords[i].x += math::sin(2.f*math::PI * Time*Oscillator.Frequency.x + 2.f*math::PI * Oscillator.Phase.x) * Oscillator.Amplitude.x;
			OutCoords[i].y += math::sin(2.f*math::PI * Time*Oscillator.Frequency.y + 2.f*math::PI * Oscillator.Phase.y) * Oscillator.Amplitude.y;
		}
	}

	//
	// Apply flip and turn.
	//
	if( bFlipH )
	{
		for( Int32 i=0; i<NumVerts; i++ )
			OutCoords[i].x = -OutCoords[i].x;
	}
	if( bFlipV )
	{
		for( Int32 i=0; i<NumVerts; i++ )
			OutCoords[i].y = -OutCoords[i].y;
	}
	if( bTurn90 )
	{
		for( Int32 i=0; i<NumVerts; i++ )
			exchange(OutCoords[i].x, OutCoords[i].y);
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