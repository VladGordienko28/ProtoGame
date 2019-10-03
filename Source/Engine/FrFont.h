/*=============================================================================
    FrFont.h: A font resource.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    FFont.
-----------------------------------------------------------------------------*/

//
// A font object.
//
class FFont: public FResource
{
REGISTER_CLASS_H(FFont);
public:
	// Variables.
	fnt::Font::Ptr m_font;

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
	inline const fnt::Glyph& GetGlyph( Char C ) const
	{
		return m_font->getGlyph( C );
	}

	inline Int32 maxHeight() const
	{
		return m_font->maxHeight();
	}
};

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/