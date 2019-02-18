/*=============================================================================
    FrBitmap.cpp: FBitmap class.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    TPalette implementation.
-----------------------------------------------------------------------------*/

//
// Palette default constructor.
//
TPalette::TPalette()
{}


//
// Palette copy constructor.
//
TPalette::TPalette( const TPalette& Other )
{
	assert( Other.Colors.size() > 0 && 
		    Other.Colors.size() <= 256 );

	Colors.setSize( Other.Colors.size() );
	for( Int32 i=0; i<Other.Colors.size(); i++ )
		Colors[i]	= Other.Colors[i];
}


//
// Palette allocator.
//
void TPalette::Allocate( UInt32 NumCols )
{
	assert( NumCols > 0 && NumCols <= 256 );
	Colors.setSize( NumCols );
}


//
// Release the palette.
// 
void TPalette::Release()
{
	Colors.empty();
}


//
// Find the most close color from palette.
// Don't compare alpha channel.
//
UInt8 TPalette::FindMatched( TColor InColor )
{
#define R_FACTOR	30
#define G_FACTOR	59
#define B_FACTOR	11

	UInt8 Best = 0;
	Int32 BestCost = 0x7fffffff, Cost;

	for( Int32 i=0; i < Colors.size(); i++ )
	{
		Cost	= R_FACTOR * sqr( (Int32)InColor.R - (Int32)Colors[i].R ) +
			      G_FACTOR * sqr( (Int32)InColor.G - (Int32)Colors[i].G ) +
				  B_FACTOR * sqr( (Int32)InColor.B - (Int32)Colors[i].B );

		if( Cost < BestCost )
		{
			Best		= i;
			BestCost	= Cost;
		}
	}

	return Best;

#undef R_FACTOR
#undef G_FACTOR
#undef B_FACTOR
}


//
// TPalette serialization.
//
void Serialize( CSerializer& S, TPalette& V )
{
	Serialize( S, V.Colors );
}


/*-----------------------------------------------------------------------------
    FBitmap implementation.
-----------------------------------------------------------------------------*/

//
// Bitmap constructor.
//
FBitmap::FBitmap()
	:	Format( BF_Palette8 ),
		Filter( BFILTER_Nearest ),
		BlendMode( BLEND_Regular )
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
	PanUSpeed	= Clamp( PanUSpeed,		-10.f,	+10.f );
	PanVSpeed	= Clamp( PanVSpeed,		-10.f,	+10.f );
	Saturation	= Clamp( Saturation,	-10.f,	+10.f );
	AnimSpeed	= Clamp( AnimSpeed,		0.01f,	60.f );
}


//
// Erase temporal bitmap effects.
//
void FBitmap::Erase()
{
}


//
// Initialize bitmap, used to setup bitmap
// use it only in editor.
//
void FBitmap::Init( Int32 InU, Int32 InV )
{
	// Resolution should be power of two.
    assert(((InU)&(InU-1)) == 0);
    assert(((InV)&(InV-1)) == 0);

	// Editor only.
	assert(GIsEditor);

	// Setup fields.
	USize				= InU;
	VSize				= InV;
	UBits				= IntLog2(USize);
	VBits				= IntLog2(VSize);
	Format				= BF_Palette8;
	Filter				= BFILTER_Nearest;
	BlendMode			= BLEND_Regular;
	RenderInfo			= -1;
	PanUSpeed			= 0.f;
	PanVSpeed			= 0.f;
	Saturation			= 1.f;
	AnimSpeed			= 30.f;
	bDynamic			= false;
	bRedrawn			= false;
	LastRenderTime		= 0.0;

    //!! Data, Palette must be initialized in
    // importer or dynamic bitmaps subclasses.
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
	USize			= 1 << UBits;
	VSize			= 1 << VBits;
	bDynamic		= false;
	bRedrawn		= false;
	LastRenderTime	= 0.0;

	// Force OpenGL/DirectX upload bitmap.
	RenderInfo	= -1;
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

	Serialize( S, iBlock );
	SerializeEnum( S, Format );
	Serialize( S, UBits );
	Serialize( S, VBits );
	Serialize( S, Palette );
	SerializeEnum( S, Filter );
	SerializeEnum( S, BlendMode );
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

	IMPORT_BYTE( UBits );
	IMPORT_BYTE( VBits );

	// Import the palette.
	if( Format == BF_Palette8 && this->IsA(FDemoBitmap::MetaClass) )
	{
		Palette.Allocate( Im.ImportInteger( L"NumColors" ) );

		for( Int32 iColor=0; iColor<Palette.Colors.size(); iColor++ )
			Palette.Colors[iColor]	= Im.ImportColor( *String::Format( L"Colors[%d]", iColor ) );
	}
}


//
// Export bitmap.
//
void FBitmap::Export( CExporterBase& Ex )
{
	FTexture::Export( Ex );

	EXPORT_BYTE( UBits );
	EXPORT_BYTE( VBits );

	// Export the palette if has, and
	// it's a procedural bitmap, because simple
	// bitmaps will be imported.
	if( Format == BF_Palette8 && this->IsA(FDemoBitmap::MetaClass) )
	{
		Ex.ExportInteger( L"NumColors", Palette.Colors.size() );
		for( Int32 iColor=0; iColor<Palette.Colors.size(); iColor++ )
			Ex.ExportColor( *String::Format( L"Colors[%d]", iColor ), Palette.Colors[iColor] );
	}
}


/*-----------------------------------------------------------------------------
	TStaticBitmap implementation.
-----------------------------------------------------------------------------*/

//
// GUI bitmap constructor.
//
TStaticBitmap::TStaticBitmap()
	:	FBitmap(),
		Data()
{
	Name = L"StaticBitmap";
	Id = -1;
	Class = FBitmap::MetaClass;
}


//
// GUI bitmap destructor.
// 
TStaticBitmap::~TStaticBitmap()
{
	Data.empty();
}


//
// Return GUI bitmap data.
//
void* TStaticBitmap::GetData()
{
	return Data.size() > 0 ? &Data[0] : nullptr;
}


//
// Return GUI bitmap size.
//
SizeT TStaticBitmap::GetBlockSize()
{
	return Data.size();
}


//
// Allocate static bitmap data.
//
Bool TStaticBitmap::AllocateBlock( SizeT NumBytes, UInt32 ExtraFlags )
{
	assert(NumBytes > 0);
	Data.setSize( NumBytes );
	return true;
}


//
// Release static bitmap data.
//
Bool TStaticBitmap::ReleaseBlock()
{
	Data.empty();
	return true;
}


//
// Return true, if datablock is valid.
//
Bool TStaticBitmap::IsValidBlock() const
{
	return true;
}


/*-----------------------------------------------------------------------------
	CDefaultBitmap implementation.
-----------------------------------------------------------------------------*/

//
//  The ÑDefaultBitmap it a temporal object, so do not register it and
//  avoid references to it, there only one instance and used as null bitmap for
//  rendering.
//
class CDefaultBitmap: public FBitmap
{
public:
	// Variables.
	TColor		ChessPattern[16][16];

	// CDefaultBitmap interface.
	CDefaultBitmap()
	{
		// Set fields.
		Format		= BF_RGBA;
		Filter		= BFILTER_Nearest;
		BlendMode	= BLEND_Regular;
		USize		= 16;
		VSize		= 16;
		UBits		= IntLog2( USize );
		VBits		= IntLog2( VSize );
		Saturation	= 1.f;
		RenderInfo	= -1;
		bDynamic	= false;
		Name		= L"HipHop";
		Id			= -1;
		Class		= FBitmap::MetaClass;

		// Plot cells.
		for( Int32 v=0; v<16; v++ )
		for( Int32 u=0; u<16; u++ )
		{
			ChessPattern[v][u].R	= (( u ^ v ) << 3) + 32;
			ChessPattern[v][u].G	= (( u ^ v ) << 3) + 32;
			ChessPattern[v][u].B	= (( u ^ v ) << 3) + 32;
			ChessPattern[v][u].A	= 0xff;
		}
	}
	~CDefaultBitmap()
	{}

	// FBitmap interface.
	void* GetData()
	{
		return ChessPattern;
	}
	Bool AllocateBlock( SizeT NumBytes, UInt32 ExtraFlags )
	{
		return false;
	}
	Bool ReleaseBlock()
	{
		return false;
	}
	Bool IsValidBlock() const
	{
		return true;
	}
	SizeT GetBlockSize()
	{
		return sizeof(ChessPattern);
	}
};


//
// Initialize instance.
//
FBitmap* FBitmap::NullBitmap()
{
	static CDefaultBitmap ChessBitmap;
	return &ChessBitmap;
}



/*-----------------------------------------------------------------------------
	FTexture implementation.
-----------------------------------------------------------------------------*/

//
// Texture constructor.
//
FTexture::FTexture()
	:	FResource(),
		UBits(0), VBits(0),
		USize(0), VSize(0)
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
	BEGIN_ENUM(EBitmapFormat)
		ENUM_ELEM(BF_Palette8);
		ENUM_ELEM(BF_RGBA);
	END_ENUM;

	BEGIN_ENUM(EBitmapFilter)
		ENUM_ELEM(BFILTER_Nearest);
		ENUM_ELEM(BFILTER_Bilinear);
		ENUM_ELEM(BFILTER_Dither);
	END_ENUM;

	BEGIN_ENUM(EBitmapBlend)
		ENUM_ELEM(BLEND_Regular);
		ENUM_ELEM(BLEND_Masked);
		ENUM_ELEM(BLEND_Translucent);
		ENUM_ELEM(BLEND_Modulated);
		ENUM_ELEM(BLEND_Alpha);
		ENUM_ELEM(BLEND_Darken);
		ENUM_ELEM(BLEND_Brighten);
		ENUM_ELEM(BLEND_FastOpaque);
	END_ENUM;

	ADD_PROPERTY( Format, PROP_Editable | PROP_Const );
	ADD_PROPERTY( Filter, PROP_Editable );
	ADD_PROPERTY( BlendMode, PROP_Editable );
	ADD_PROPERTY( USize, PROP_Editable | PROP_Const | PROP_NoImEx );
	ADD_PROPERTY( VSize, PROP_Editable | PROP_Const | PROP_NoImEx );
	ADD_PROPERTY( PanUSpeed, PROP_Editable );
	ADD_PROPERTY( PanVSpeed, PROP_Editable );
	ADD_PROPERTY( Saturation, PROP_Editable );
	ADD_PROPERTY( AnimSpeed, PROP_Editable );
	ADD_PROPERTY( FileName, PROP_Editable | PROP_Const | PROP_NoImEx );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/