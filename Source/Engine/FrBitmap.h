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
	Byte				UBits;		
	Byte				VBits;
	Integer				USize;
	Integer				VSize;

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
	TArray<TColor>		Colors;

	// Functions.
	TPalette();
	TPalette( const TPalette& Other );
	void Allocate( DWord NumCols );
	void Release();
	Byte FindMatched( TColor InColor );
	friend void Serialize( CSerializer& S, TPalette& V );
};


/*-----------------------------------------------------------------------------
    FBitmap.
-----------------------------------------------------------------------------*/

// Masking.
#define MASK_COLOR		TColor( 0xff, 0x00, 0xff, 0xff )


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
	Integer				RenderInfo;

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
	virtual void Init( Integer InU, Integer InV );
	virtual void Erase();
	virtual void MouseClick( Integer Button, Integer X, Integer Y );
	virtual void MouseMove( Integer Button, Integer X, Integer Y );
	virtual void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );
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
	TArray<Byte>		Data;

	// TStaticBitmap interface.
	TStaticBitmap();
	~TStaticBitmap();

	// FLargeResource interface.
	Bool AllocateBlock( SizeT NumBytes, DWord ExtraFlags = BLOCK_None );
	Bool ReleaseBlock();
	Bool IsValidBlock() const;
	SizeT GetBlockSize();
	void* GetData();
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/