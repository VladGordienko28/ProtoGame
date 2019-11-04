//-----------------------------------------------------------------------------
//	UIRender.cpp: An UI render
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "UI.h"

namespace flu
{
namespace ui
{
	static const Char FLAT_SHADE_EFFECT_NAME[] = TXT("System.Shaders.UI.FlatShade");

	Render::Render( rend::Device* device )
		:	m_device( device ),
			m_effect( nullptr ),
			m_vb( INVALID_HANDLE<rend::VertexBufferHandle>() ),
			m_vbSize( 0 ),
			m_cpuBuffer()
	{
		assert( m_device );
		m_effect = res::ResourceManager::get<ffx::Effect>( FLAT_SHADE_EFFECT_NAME, res::EFailPolicy::FATAL );

		m_blendStateId = device->getBlendState( { rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::InvSrcAlpha, 
			rend::EBlendOp::Add, rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add } );
	}

	Render::~Render()
	{
		m_effect = nullptr;
		m_cpuBuffer.empty();

		if( m_vb != INVALID_HANDLE<rend::VertexBufferHandle>() )
		{
			m_device->destroyVertexBuffer( m_vb );
		}
	}

	void Render::prepareBatches( Container* treeRoot )
	{
		assert( treeRoot );
		assert( m_cpuBuffer.size() == 0 );
		
		profile_zone( UI, CPU_PrepareBatches );

		for( auto& it : treeRoot->m_children )
		{
			Float itX, itY, itWidth, itHeight;
			
			itX = it->m_position.x;
			itY = it->m_position.y;
			itWidth = it->m_size.width;
			itHeight = it->m_size.height;
			
			//it->getBounds( treeRoot->m_size, treeRoot->m_padding, itX, itY, itWidth, itHeight );

			Vertex v;
			v.color = it->isFocused() ? math::colors::YELLOW : /*it->isHighlighted() ? math::colors::RED :*/ math::colors::BLUE;
			v.color.a = 255 * it->m_opacity;

			v.pos = { itX, itY };
			m_cpuBuffer.push( v );

			v.pos = { itX, itY + itHeight };
			m_cpuBuffer.push( v );

			v.pos = { itX + itWidth, itY };
			m_cpuBuffer.push( v );

			v.pos = { itX, itY + itHeight };
			m_cpuBuffer.push( v );

			v.pos = { itX + itWidth, itY };
			m_cpuBuffer.push( v );

			v.pos = { itX + itWidth, itY + itHeight };
			m_cpuBuffer.push( v );
		}
	}

	void Render::flushBatches()
	{
		//assert( threading::isMainThread() );

		profile_zone( UI, CPU_FlushBatches );

		if( m_cpuBuffer.size() == 0 )
		{
			return;
		}

		if( m_vb == INVALID_HANDLE<rend::VertexBufferHandle>() || m_cpuBuffer.size() > m_vbSize )
		{
			if( m_vb != INVALID_HANDLE<rend::VertexBufferHandle>() )
			{
				m_device->destroyVertexBuffer( m_vb );
			}

			m_vb = m_device->createVertexBuffer( sizeof(Vertex), m_cpuBuffer.size(), rend::EUsage::Dynamic, nullptr, "GUI_VB" );
			m_vbSize = m_cpuBuffer.size();
		}

		m_device->updateVertexBuffer( m_vb, &m_cpuBuffer[0], m_cpuBuffer.size() * sizeof(Vertex) );

		m_effect->apply();
		m_effect->setBlendState( m_blendStateId );

		m_device->setTopology( rend::EPrimitiveTopology::TriangleList );
		m_device->setVertexBuffer( m_vb );
		m_device->draw( m_cpuBuffer.size(), 0 );

		m_cpuBuffer.empty();
	}
}
}
