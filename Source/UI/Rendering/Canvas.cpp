//-----------------------------------------------------------------------------
//	Canvas.cpp: An UI layer canvas implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "UI/UI.h"

namespace flu
{
namespace ui
{
namespace rendering
{
	Canvas::Canvas()
		:	m_origin( 0.f, 0.f )
	{
		m_flatShadeOps[FSO_Solid].alphaEnabled = false;

		m_flatShadeOps[FSO_Alpha].alphaEnabled = true;
	}

	Canvas::~Canvas()
	{
	}

	void Canvas::drawRect( const Position& pos, const Size& size, math::Color color )
	{

		// experimental!!
		FlatShadeOp& op = m_flatShadeOps[ color.a == 255 ? FSO_Solid : FSO_Alpha];

		UInt32 firstVtxIndex = op.vertices.size();

		auto vertices = op.vertices.obtainRaw( 4 );
		auto indices = op.indices.obtainRaw( 6 );

		vertices[0].pos = { pos.x, pos.y };
		vertices[0].color = color;

		vertices[1].pos = { pos.x, pos.y + size.height };
		vertices[1].color = color;

		vertices[2].pos = { pos.x + size.width, pos.y + size.height };
		vertices[2].color = color;

		vertices[3].pos = { pos.x + size.width, pos.y };
		vertices[3].color = color;

		indices[0] = firstVtxIndex + 0;
		indices[1] = firstVtxIndex + 1;
		indices[2] = firstVtxIndex + 2;

		indices[3] = firstVtxIndex + 0;
		indices[4] = firstVtxIndex + 3;
		indices[5] = firstVtxIndex + 2;
	}

	void Canvas::drawPatternRect( const Position& pos, const Size& size, 
		math::Color color, EPattern pattern )
	{
	}

	void Canvas::drawBorderRect( const Position& pos, const Size& size, 
		Float borderThickness, math::Color color, math::Color borderColor )
	{
		// experimental :)
		drawRect( pos, size, borderColor );
		drawRect( { pos.x + borderThickness, pos.y + borderThickness }, { size.width - borderThickness*2, size.height - borderThickness * 2 }, color );

	}

	void Canvas::drawPatternBorderRect( const Position& pos, const Size& size, 
		Float borderThickness, math::Color color, math::Color borderColor, EPattern pattern )
	{
	}

	void Canvas::drawImageRect( const Position& pos, const Size& size,
		const math::Vector& tc0, const math::Vector& tc1, img::Image::Ptr image )
	{
	}

	void Canvas::drawImageRect( const Position& pos, const Size& size,
		const math::Vector& tc0, const math::Vector& tc1, rend::Texture2DHandle texture )
	{
	}

	void Canvas::drawImageRect( const Position& pos, const Size& size,
		const math::Vector& tc0, const math::Vector& tc1, rend::ShaderResourceView srv )
	{
	}

	void Canvas::drawTextLine( const Position& pos, const Char* text, Int32 len, math::Color color,
		fnt::Font::Ptr font, Float xScale, Float yScale )
	{
		TextOp& op = findOrCreateTextOp( font->getImage()->getSRV() );

		UInt32 firstVtxIndex = op.vertices.size();

		auto vertices = op.vertices.obtainRaw( fnt::getBatchVertexCount( len ) );
		auto indices = op.indices.obtainRaw( fnt::getBatchIndexCount( len ) );

		fnt::batchLine( text, len, font, color, vertices, firstVtxIndex, indices, {pos.x, pos.y}, xScale, yScale );
	}

	void Canvas::drawLine( const Position& from, const Position& to, math::Color color )
	{
	}

	void Canvas::pushClipArea( const Position& pos, const Size& size )
	{
	}

	void Canvas::popClipArea()
	{
	}

	void Canvas::clearOps()
	{
		for( auto& it : m_flatShadeOps )
		{
			it.indices.empty();
			it.vertices.empty();
		}

		m_textOps.empty();
	}

	FlatShadeOp* Canvas::getFlatShadeOps( UInt32& count )
	{
		count = FSO_MAX;
		return &m_flatShadeOps[0];
	}

	TextOp* Canvas::getTextOps( UInt32& count )
	{
		count = m_textOps.size();
		return count > 0 ? &m_textOps[0] : nullptr;
	}

	TextOp& Canvas::findOrCreateTextOp( rend::ShaderResourceView srv )
	{
		// O(n) is ok, still we have only a couple fonts per layout
		for( Int32 i = 0; i < m_textOps.size(); ++i )
		{
			if( m_textOps[i].srv == srv )
			{
				return m_textOps[i];
			}
		}

		TextOp newOp;
		newOp.srv = srv;
		m_textOps.push( newOp );

		return m_textOps.last();
	}
}
}
}