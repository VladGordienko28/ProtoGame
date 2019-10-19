//-----------------------------------------------------------------------------
//	FontType.cpp: A font type
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Font.h"

namespace flu
{
namespace fnt
{
	Font::Font( String name )
		:	m_name( name ),
			m_image(),
			m_glyphs(),
			m_remap()
	{
	}

	Font::~Font()
	{
	}

	Bool Font::create( const res::CompiledResource& compiledResource )
	{
		assert( compiledResource.isValid() );
		assert( m_image == nullptr );

		IInputStream::Ptr reader = new BufferReader( compiledResource.data );

		res::ResourceId imageResourceId;
		*reader >> imageResourceId;
		m_image = res::ResourceManager::get<img::Image>( imageResourceId, res::EFailPolicy::FATAL ); // todo: add handler!

		*reader >> m_glyphs;
		*reader >> m_remap;

		// rescale glyphs
		for( auto& it : m_glyphs )
		{
			it.x /= m_image->getUSize();
			it.y /= m_image->getVSize();
			it.width /= m_image->getUSize();
			it.height /= m_image->getVSize();
		}

		return !reader->hasError();
	}

	void Font::destroy()
	{
		m_image = nullptr;
		m_glyphs.empty();
		m_remap.empty();
	}

	Float Font::maxHeight() const
	{
		// todo: assumed all glyphs are equal
		assert( m_image.hasObject() );
		return getGlyph( 'A' ).height * m_image->getVSize();
	}

	Float Font::textWidth( const Char* text ) const
	{
		assert( text );
		assert( m_image.hasObject() );

		Float width = 0.f;
		for( const Char* symbol = text; *symbol; ++symbol )
		{
			width += getGlyph( *symbol ).width;
		}

		return width * m_image->getUSize();
	}
}
}