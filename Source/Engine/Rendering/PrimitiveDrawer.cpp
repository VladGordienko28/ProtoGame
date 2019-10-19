//-----------------------------------------------------------------------------
//	PrimitiveDrawer.cpp: A helper class, which draw some editor or debug stuff
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine/Engine.h"

namespace flu
{
namespace gfx
{
	static const Char PRIMITIVE_EFFECT_NAME[] = TXT( "System.Shaders.Primitive" );

	PrimitiveDrawer::PrimitiveDrawer()
		:	m_vertexBuffer( "PrimitiveDrawer_VB" ),
			m_effect( nullptr )
	{
		m_effect = res::ResourceManager::get<ffx::Effect>( PRIMITIVE_EFFECT_NAME, res::EFailPolicy::FATAL );
	}

	PrimitiveDrawer::~PrimitiveDrawer()
	{
		m_effect = nullptr;
	}

	void PrimitiveDrawer::batchPoint( const math::Vector& pos, Float z, Float size, math::Color color )
	{
		// not implemented yet
	}

	void PrimitiveDrawer::batchCoolPoint( const math::Vector& pos, Float z, Float size, math::Color color )
	{
		// not implemented yet
	}

	void PrimitiveDrawer::batchLine( const math::Vector& from, const math::Vector& to, Float z, math::Color color )
	{
		Vertex* data = m_vertexBuffer.reserve( 2 );

		data[0].pos = { from, z };
		data[0].color = color;

		data[1].pos = { to, z };
		data[1].color = color;
	}

	void PrimitiveDrawer::batchLineStrip( const math::Vector* list, UInt32 numVerts, Float z, math::Color color )
	{
		// not implemented yet
	}

	void PrimitiveDrawer::batchSmoothLine( const math::Vector& from, const math::Vector& to, Float z, math::Color color, UInt32 detail )
	{
		// not implemented yet
	}

	void PrimitiveDrawer::batchCircle( const math::Vector& pos, Float radius, Float z, math::Color color, UInt32 detail )
	{
		// not implemented yet
	}

	void PrimitiveDrawer::batchEllipse( const math::Vector& pos, Float xSize, Float ySize, Float z, math::Color color, UInt32 detail )
	{
		// not implemented yet
	}

	void PrimitiveDrawer::batchLineStar( const math::Vector& pos, math::Angle rot, Float size, Float z, math::Color color )
	{
		const math::Coords localCoords( pos, rot );

		const math::Vector xAxis = localCoords.xAxis * ( size * 0.5f );
		const math::Vector yAxis = localCoords.yAxis * ( size * 0.5f );

		Vertex* data = m_vertexBuffer.reserve( 4 );

		data[0] = { { ( pos - xAxis ), z }, color };
		data[1] = { { ( pos + xAxis ), z }, color };
		data[2] = { { ( pos - yAxis ), z }, color };
		data[3] = { { ( pos + yAxis ), z }, color };
	}

	void PrimitiveDrawer::batchLineRect( const math::Vector& pos, Float xSize, Float ySize, math::Angle rot, Float z, math::Color color )
	{
		const math::Coords localCoords( pos, rot );

		const math::Vector xAxis = localCoords.xAxis * ( xSize * 0.5f );
		const math::Vector yAxis = localCoords.yAxis * ( ySize * 0.5f );

		Vertex* data = m_vertexBuffer.reserve( 8 );

		data[7] = data[0] = { { ( pos - yAxis - xAxis ), z }, color };
		data[1] = data[2] = { { ( pos + yAxis - xAxis ), z }, color };
		data[3] = data[4] = { { ( pos + yAxis + xAxis ), z }, color };
		data[5] = data[6] = { { ( pos - yAxis + xAxis ), z }, color };
	}

	void PrimitiveDrawer::flush()
	{
		if( m_vertexBuffer.getSize() > 0 )
		{
			UInt32 numVerts = m_vertexBuffer.flushAndBind();

			m_effect->apply();

			api::setTopology( rend::EPrimitiveTopology::LineList );
			api::draw( numVerts, 0 );
		}
	}
}
}