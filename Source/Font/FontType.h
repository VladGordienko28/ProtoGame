//-----------------------------------------------------------------------------
//	FontType.h: A font type
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace fnt
{
	/**
	 *	A font
	 */
	class Font: public res::Resource
	{
	public:
		DECLARE_RESOURCE( Font, System, Compiler );

		~Font();

		img::Image::Ptr getImage() const { return m_image; }

		const Glyph& getGlyph( Char c ) const
		{
			const UInt32 glyphId = m_remap[min<Int32>( c, m_remap.size() - 1 )];
			return m_glyphs[glyphId];
		}

		// in pixels
		Float maxHeight() const;
		Float textWidth( const Char* text ) const;
		Float textWidth( const Char* text, Int32 len ) const;

	private:
		img::Image::Ptr m_image;
		Array<Glyph> m_glyphs;
		Array<UInt16> m_remap;

		Font() = delete;
		Font( String name );

		Bool create( const res::CompiledResource& compiledResource );
		void destroy();

		friend class System;
	};
}
}