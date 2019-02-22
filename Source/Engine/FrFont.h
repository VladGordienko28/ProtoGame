/*=============================================================================
    FrFont.h: A font resource.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    FFont.
-----------------------------------------------------------------------------*/

// Size of the glyphs atlas.
#define GLYPHS_ATLAS_SIZE	256


//
// A single font glyph.
//
struct TGlyph
{
public:
	// Variables.
	Int32	iBitmap;
	UInt8	X;
	UInt8	Y;
	UInt8	W;
	UInt8	H;
};


//
// A font object.
//
class FFont: public FResource
{
REGISTER_CLASS_H(FFont);
public:
	// Variables.
	Array<FBitmap*>	Bitmaps;
	Int32				Height;
	Array<TGlyph>		Glyphs;
	Array<UInt8>		Remap;

	// FFont interface.
	FFont();
	~FFont();
	Int32 TextWidth( const Char* Text );

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

	// Return the glyph associative with
	// given character.
	inline TGlyph& GetGlyph( Char C )
	{
		Int32 iGlyph = Remap[min<Int32>( C, Remap.size()-1 )];
		return Glyphs[min( iGlyph, Glyphs.size()-1 )];
	}
};


/*-----------------------------------------------------------------------------
    TStaticFont.
-----------------------------------------------------------------------------*/

//
// Non project font.
//
class TStaticFont: public FFont
{
public:
	// TStaticFont interface.
	TStaticFont();
	~TStaticFont();
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/