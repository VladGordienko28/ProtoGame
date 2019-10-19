//-----------------------------------------------------------------------------
//	TextDrawer.cpp: TextDrawer implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine/Engine.h"

namespace flu
{
namespace gfx
{
	static const Char TEXT_EFFECT_NAME[] = TXT( "System.Shaders.Text" );

	TextDrawer::TextDrawer()
		:	m_vertexBuffer( "TextDrawer_VB" ),
			m_indexBuffer( "TextDrawer_IB" ),
			m_currentFont( nullptr ),
			m_effect( nullptr )
	{
		m_effect = res::ResourceManager::get<ffx::Effect>( TEXT_EFFECT_NAME, res::EFailPolicy::FATAL );

		m_samplerState = api::getSamplerState( { rend::ESamplerFilter::Point, rend::ESamplerAddressMode::Clamp } );

		m_blendState = api::getBlendState( { rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::InvSrcAlpha, 
			rend::EBlendOp::Add, rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add } );
	}

	TextDrawer::~TextDrawer()
	{
		m_currentFont = nullptr;
		m_effect = nullptr;
	}

	void TextDrawer::batchText( const Char* text, Int32 len, fnt::Font::Ptr font, math::Color color, 
		const math::Vector& from, Float xScale, Float yScale )
	{
		assert( font.hasObject() );
		assert( xScale != 0.f && yScale != 0.f );

		// nothing to draw
		if( !text || !*text || len == 0 )
		{
			return;
		}

		// flush old batches
		if( font != m_currentFont )
		{
			if( m_currentFont.hasObject() )
			{
				flush();
			}

			m_currentFont = font;
		}

		img::Image::Ptr atlas = font->getImage();
		const Float atlasUSize = atlas->getUSize();
		const Float atlasVSize = atlas->getVSize();

		const UInt16 firstVertId = m_vertexBuffer.getSize();

		Vertex* vb = m_vertexBuffer.reserve( len * 4 );
		UInt16* ib = m_indexBuffer.reserve( len * 6 );

		math::Vector walker = from;

		for( Int32 i = 0; i < len; ++i )
		{
			const fnt::Glyph glyph = font->getGlyph( text[i] );
			
			const Float xCharSize = atlasUSize * glyph.width * xScale;
			const Float yCharSize = atlasVSize * glyph.height * yScale;

			const math::Vector tc0 = { glyph.x, glyph.y };
			const math::Vector tc1 = { glyph.x + glyph.width, glyph.y + glyph.height };

			const UInt16 vertId = i * 4;

			vb[vertId + 0].pos = { walker.x, walker.y };
			vb[vertId + 0].tc = { tc0.x, tc0.y };
			vb[vertId + 0].color = color;

			vb[vertId + 1].pos = { walker.x, walker.y + yCharSize };
			vb[vertId + 1].tc = { tc0.x, tc1.y };
			vb[vertId + 1].color = color;

			vb[vertId + 2].pos = { walker.x + xCharSize, walker.y + yCharSize };
			vb[vertId + 2].tc = { tc1.x, tc1.y };
			vb[vertId + 2].color = color;

			vb[vertId + 3].pos = { walker.x + xCharSize, walker.y };
			vb[vertId + 3].tc = { tc1.x, tc0.y };
			vb[vertId + 3].color = color;

			const UInt16 indexId = i * 6;

			ib[indexId + 0] = firstVertId + vertId + 0;
			ib[indexId + 1] = firstVertId + vertId + 1;
			ib[indexId + 2] = firstVertId + vertId + 2;

			ib[indexId + 3] = firstVertId + vertId + 0;
			ib[indexId + 4] = firstVertId + vertId + 3;
			ib[indexId + 5] = firstVertId + vertId + 2;

			walker.x += xCharSize;
		}
	}

	Float TextDrawer::textHeight( fnt::Font::Ptr font, Float yScale ) const
	{
		assert( font.hasObject() );

		return font->maxHeight() * yScale;
	}

	Float TextDrawer::textWidth( const Char* text, Int32 len, fnt::Font::Ptr font, Float xScale ) const
	{
		assert( font.hasObject() );

		return font->textWidth( text ) * xScale; // todo: take len into according
	}

	void TextDrawer::flush()
	{
		if( m_currentFont.hasObject() )
		{
			assert( m_vertexBuffer.getSize() > 0 );

			m_vertexBuffer.flushAndBind();
			UInt32 numIndices = m_indexBuffer.flushAndBind();

			m_effect->setBlendState( m_blendState );
			m_effect->setTexture( 0, m_currentFont->getImage()->getHandle() );
			m_effect->setSamplerState( 0, m_samplerState );
			m_effect->apply();

			api::setTopology( rend::EPrimitiveTopology::TriangleList );
			api::drawIndexed( numIndices, 0, 0 );

			m_currentFont = nullptr;
		}
	}
}
}