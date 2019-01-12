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
	Integer	iBitmap;
	Byte	X;
	Byte	Y;
	Byte	W;
	Byte	H;
};


//
// A font object.
//
class FFont: public FResource
{
REGISTER_CLASS_H(FFont);
public:
	// Variables.
	TArray<FBitmap*>	Bitmaps;
	Integer				Height;
	TArray<TGlyph>		Glyphs;
	TArray<Byte>		Remap;

	// FFont interface.
	FFont();
	~FFont();
	Integer TextWidth( const Char* Text );

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

	// Return the glyph associative with
	// given character.
	inline TGlyph& GetGlyph( Char C )
	{
		Integer iGlyph = Remap[Min<Integer>( C, Remap.Num()-1 )];
		return Glyphs[Min( iGlyph, Glyphs.Num()-1 )];
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