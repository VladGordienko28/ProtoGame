//-----------------------------------------------------------------------------
//	Batching.cpp: A helping text batching functions implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Font.h"

namespace flu
{
namespace fnt
{
	void batchLine( const Char* text, Int32 len, Font::Ptr font, math::Color color, 
		TextVertex* vb, UInt32 firstVtxIndex, UInt16* ib, const math::Vector& from, 
		Float xScale, Float yScale )
	{
		assert( font.hasObject() );
		assert( vb && ib );
		assert( xScale != 0.f && yScale != 0.f );

		// nothing to draw
		if( !text || !*text || len == 0 )
		{
			return;
		}

		img::Image::Ptr atlas = font->getImage();
		const Float atlasUSize = static_cast<Float>( atlas->getUSize() );
		const Float atlasVSize = static_cast<Float>( atlas->getVSize() );

		const UInt16 firstVertId = firstVtxIndex;

		math::Vector walker = from;

		for( Int32 i = 0; i < len; ++i )
		{
			const fnt::Glyph glyph = font->getGlyph( text[i] );
			
			const Float xCharSize = atlasUSize * glyph.width * xScale;
			const Float yCharSize = atlasVSize * glyph.height * yScale;

			const math::Vector tc0 = { glyph.x, glyph.y };
			const math::Vector tc1 = { glyph.x + glyph.width, glyph.y + glyph.height };

			// batch vertices
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

			// batch indices
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

	Float getLineHeight( fnt::Font::Ptr font, Float yScale )
	{
		assert( font.hasObject() );
	
		return font->maxHeight() * yScale;
	}

	Float getLineWidth( const Char* text, Int32 len, Font::Ptr font, Float xScale )
	{
		assert( font.hasObject() );
	
		return font->textWidth( text, len ) * xScale;
	}

	Float getLineWidth( const Char* text, Font::Ptr font, Float xScale )
	{
		assert( font.hasObject() );
	
		return font->textWidth( text ) * xScale;
	}

	UInt32 getBatchVertexCount( Int32 textLength )
	{
		return textLength * 4;
	}

	UInt32 getBatchIndexCount( Int32 textLength )
	{
		return textLength * 6;
	}
}
}