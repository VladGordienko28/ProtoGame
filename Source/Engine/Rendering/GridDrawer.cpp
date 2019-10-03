//-----------------------------------------------------------------------------
//	GridDrawer.cpp: Editor grid drawer
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine/Engine.h"

namespace flu
{
namespace gfx
{
	static const Char GRID_EFFECT_NAME[] = TEXT( "System.Shaders.Grid" );

	GridDrawer::GridDrawer()
		:	m_gridSize( 0.f ),
			m_color( 0.f, 0.f, 0.f, 1.f ),
			m_effect( nullptr )
	{
	}

	GridDrawer::~GridDrawer()
	{
	}

	void GridDrawer::create( math::Color color, UInt32 size )
	{
		assert( size > 0 );

		m_gridSize = size;
		m_color = color;
		m_effect = res::ResourceManager::get< ffx::Effect >( GRID_EFFECT_NAME );
	}

	void GridDrawer::destroy()
	{
		m_effect = nullptr;
	}

	void GridDrawer::render( const gfx::ViewInfo& viewInfo )
	{
		assert( m_effect.hasObject() );

		const Int32 cMinX = math::floor( max<Float>( viewInfo.bounds.min.x, -m_gridSize * 0.5f ));
		const Int32 cMinY = math::floor( max<Float>( viewInfo.bounds.min.y, -m_gridSize * 0.5f ));
		const Int32 cMaxX = math::ceil( min<Float>( viewInfo.bounds.max.x, m_gridSize * 0.5f ));
		const Int32 cMaxY = math::ceil( min<Float>( viewInfo.bounds.max.y, m_gridSize * 0.5f ));		

		if( cMinX <= cMaxX && cMinY <= cMaxY )
		{
			const Int32 numXLines = cMaxX - cMinX + 1;
			const Int32 numYLines = cMaxY - cMinY + 1;
			const Int32 numLines = numXLines + numYLines;

			struct GridEffectData
			{
				Int32 numXVerts;
				math::Vector3 color;
				math::Vector4 bounds;
			};

			GridEffectData gridData;
			gridData.numXVerts = numXLines * 2;
			gridData.color = math::Vector3( m_color.r, m_color.g, m_color.b );
			gridData.bounds = math::Vector4( cMinX, cMaxX, cMinY, cMaxY );

			m_effect->setData( &gridData, sizeof( GridEffectData ), 0 );
			m_effect->apply();

			api::setVertexBuffer( INVALID_HANDLE<rend::VertexBufferHandle>() );
			api::setTopology( rend::EPrimitiveTopology::LineList );
			api::draw( numLines * 2, 0 );
		}
	}
}
}