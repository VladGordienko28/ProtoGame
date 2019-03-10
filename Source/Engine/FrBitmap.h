/*=============================================================================
    FrBitmap.h: An image class.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
	FTexture.
-----------------------------------------------------------------------------*/

//
// An abstract image instance that imposed to objects.
//
class FTexture: public FResource
{
REGISTER_CLASS_H(FTexture);
public:
	// Variables.
	UInt8				UBits;		
	UInt8				VBits;
	Int32				USize;
	Int32				VSize;

	// FTexture interface.
	FTexture();
	~FTexture();
};


/*-----------------------------------------------------------------------------
    TPalette.
-----------------------------------------------------------------------------*/

//
// A normal 256 colors palette.
//
struct TPalette
{
public:
	// Variable.
	Array<math::Color>		Colors;

	// Functions.
	TPalette();
	TPalette( const TPalette& Other );
	void Allocate( UInt32 NumCols );
	void Release();
	UInt8 FindMatched( math::Color InColor );
	friend void Serialize( CSerializer& S, TPalette& V );
};


/*-----------------------------------------------------------------------------
    FBitmap.
-----------------------------------------------------------------------------*/

// Masking.
static const math::Color MASK_COLOR = { 0xff, 0x00, 0xff, 0xff };


// Bitmap formats.
enum EBitmapFormat
{
	BF_Palette8,
	BF_RGBA
};


// Bitmap filter.
enum EBitmapFilter
{
	BFILTER_Nearest,
	BFILTER_Bilinear,
	BFILTER_Dither
};


// Bitmap blend.
enum EBitmapBlend
{
	BLEND_Regular,
	BLEND_Masked,
	BLEND_Translucent,
	BLEND_Modulated,
	BLEND_Alpha,
	BLEND_Darken,
	BLEND_Brighten,
	BLEND_FastOpaque,
	BLEND_MAX
};


//
// A Bitmap.
// 
class FBitmap: public FTexture, public CResourceBlock
{
REGISTER_CLASS_H(FBitmap)
public:
	// General info.
	EBitmapFormat		Format;

	// Bitmap data.
	TPalette			Palette;

	// Render styles.
	EBitmapFilter		Filter;
	EBitmapBlend		BlendMode;
	Int32				RenderInfo;

	// Simple effects.
	Float				PanUSpeed;
	Float				PanVSpeed;
	Float				Saturation;
	Float				AnimSpeed;

	// Internal.
	Bool				bDynamic;
	Bool				bRedrawn;
	Double				LastRenderTime;

	// Static.
	static FBitmap* NullBitmap();

	// FBitmap interface.
	FBitmap();
	~FBitmap();
	void Tick();
	virtual void Init( Int32 InU, Int32 InV );
	virtual void Erase();
	virtual void MouseClick( Int32 Button, Int32 X, Int32 Y );
	virtual void MouseMove( Int32 Button, Int32 X, Int32 Y );
	virtual void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

	math::Color getAverageColor()
	{
		Int32 r = 0, g = 0, b = 0, a = 0;
		Int32 n = USize * VSize;

		for( Int32 i = 0; i < n; i++ )
		{
			math::Color c;

			if( Format == EBitmapFormat::BF_Palette8 )
			{
				c = Palette.Colors[ ((UInt8*)GetData())[i] ];
			}
			else
			{
				c = ((math::Color*)GetData())[i];
			}

			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;
		}

		return math::Color( r / n, g / n, b / n, a / n );
	}
};


/*-----------------------------------------------------------------------------
	TStaticBitmap.
-----------------------------------------------------------------------------*/

//
// Non project bitmap.
//
class TStaticBitmap: public FBitmap
{
public:
	// Variables.
	Array<UInt8>		Data;

	// TStaticBitmap interface.
	TStaticBitmap();
	~TStaticBitmap();

	// FLargeResource interface.
	Bool AllocateBlock( SizeT NumBytes, UInt32 ExtraFlags = BLOCK_None );
	Bool ReleaseBlock();
	Bool IsValidBlock() const;
	SizeT GetBlockSize();
	void* GetData();
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/