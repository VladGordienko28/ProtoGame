/*=============================================================================
    FrBitmap.cpp: FBitmap class.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FBitmap implementation.
-----------------------------------------------------------------------------*/

//
// Bitmap constructor.
//
FBitmap::FBitmap()
{}


//
// Bitmap destructor.
//
FBitmap::~FBitmap()
{
}


//
// When user changed some field.
//
void FBitmap::EditChange()
{
	FTexture::EditChange();

	// Validate input data.
	PanUSpeed	= clamp( PanUSpeed,		-10.f,	+10.f );
	PanVSpeed	= clamp( PanVSpeed,		-10.f,	+10.f );
	Saturation	= clamp( Saturation,	-10.f,	+10.f );
	AnimSpeed	= clamp( AnimSpeed,		0.01f,	60.f );
}


//
// Erase temporal bitmap effects.
//
void FBitmap::Erase()
{
}


//
// Handle mouse click in subclasses. For
// dynamic bitmaps.
//
void FBitmap::MouseClick( Int32 Button, Int32 X, Int32 Y )
{
}


//
// Handle mouse move in subclasses. For
// dynamic bitmaps.
//
void FBitmap::MouseMove( Int32 Button, Int32 X, Int32 Y )
{
}


//
// After loading code, place to restore
// some fields.
//
void FBitmap::PostLoad()
{
	FTexture::PostLoad();

	// Restore.
	bDynamic		= false;
	bRedrawn		= false;
	LastRenderTime	= 0.0;
}


//
// Redraw, used only for dynamic bitmaps.
//
void FBitmap::Redraw()
{
}


//
// Bitmap serialization.
//
void FBitmap::SerializeThis( CSerializer& S )
{
	FTexture::SerializeThis( S );

	Serialize( S, PanUSpeed );
	Serialize( S, PanVSpeed );
	Serialize( S, Saturation );
	Serialize( S, AnimSpeed );
}


//
// Tick texture effects.
//
void FBitmap::Tick()
{
	Double Time = GPlat->Now();
	Double Delta = Time - LastRenderTime;

	if( Delta > 1.0 )
	{
		// If not initialized.
		LastRenderTime = Time;
		return;
	}

	if( Delta > (60.01-AnimSpeed)/1000.0 )
	{
		Redraw();
		LastRenderTime = Time;
	}
}


//
// Import bitmap.
//
void FBitmap::Import( CImporterBase& Im )
{
	FTexture::Import( Im );

	// Import the palette.
#if DEMO_EFFECTS_ENABLED
	if( Format == BF_Palette8 && this->IsA(FDemoBitmap::MetaClass) )
	{
		Palette.Allocate( Im.ImportInteger( L"NumColors" ) );

		for( Int32 iColor=0; iColor<Palette.Colors.size(); iColor++ )
			Palette.Colors[iColor]	= Im.ImportColor( *String::format( L"Colors[%d]", iColor ) );
	}
#endif
}


//
// Export bitmap.
//
void FBitmap::Export( CExporterBase& Ex )
{
	FTexture::Export( Ex );

	// Export the palette if has, and
	// it's a procedural bitmap, because simple
	// bitmaps will be imported.
#if DEMO_EFFECTS_ENABLED
	if( Format == BF_Palette8 && this->IsA(FDemoBitmap::MetaClass) )
	{
		Ex.ExportInteger( L"NumColors", Palette.Colors.size() );
		for( Int32 iColor=0; iColor<Palette.Colors.size(); iColor++ )
			Ex.ExportColor( *String::format( L"Colors[%d]", iColor ), Palette.Colors[iColor] );
	}
#endif
}

//
// Initialize instance.
//
img::Image::Ptr FBitmap::NullBitmap()
{
	return res::ResourceManager::get<img::Image>( L"System.NullTexture", res::EFailPolicy::FATAL );
}

/*-----------------------------------------------------------------------------
	FTexture implementation.
-----------------------------------------------------------------------------*/

//
// Texture constructor.
//
FTexture::FTexture()
	:	FResource()
{
}


//
// Texture destructor.
//
FTexture::~FTexture()
{
}


/*-----------------------------------------------------------------------------
	Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FTexture, FResource, CLASS_Abstract )
{
}

REGISTER_CLASS_CPP( FBitmap, FTexture, CLASS_None )
{
	ADD_PROPERTY( PanUSpeed, PROP_Editable );
	ADD_PROPERTY( PanVSpeed, PROP_Editable );
	ADD_PROPERTY( Saturation, PROP_Editable );
	ADD_PROPERTY( AnimSpeed, PROP_Editable );
	ADD_PROPERTY( FileName, PROP_Editable | PROP_Const | PROP_NoImEx );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/