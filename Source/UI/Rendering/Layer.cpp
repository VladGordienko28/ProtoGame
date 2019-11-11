//-----------------------------------------------------------------------------
//	Layer.cpp: An UI layer for rendering
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "UI/UI.h"

namespace flu
{
namespace ui
{
namespace rendering
{
	Layer::Layer()
	{
	}

	Layer::~Layer()
	{
	}

	void Layer::create( rend::Device* device )
	{
		// create blend states
		m_blendStateNone = rend::BlendState::INVALID;

		m_blendStateAlpha = device->getBlendState( { rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::InvSrcAlpha, 
			rend::EBlendOp::Add, rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add } );

		// create sampler states
		m_samplerStateLinearWrap = device->getSamplerState( 
			{ rend::ESamplerFilter::Linear, rend::ESamplerAddressMode::Wrap } );
	}

	void Layer::destroy()
	{
	}

	void Layer::clear()
	{
		m_flatShadeBatches.empty();
		m_imageBatches.empty();
		m_textBatches.empty();

		m_canvas.clearOps();
	}

	void Layer::generateFlatShadeBatches( FlatShadeStream& stream )
	{
		UInt32 numOps;
		FlatShadeOp* ops = m_canvas.getFlatShadeOps( numOps );

		for( UInt32 i = 0; i < numOps; ++i )
		{
			FlatShadeOp* op = &ops[i];

			if( op->indices.size() > 0 && op->vertices.size() > 0 )
			{
				UInt32 firstVtxIndex = stream.getVertexCount();	
				UInt32 firstIdxIndex = stream.getIndexCount();

				// copy vertices
				mem::copy( stream.obtainVertices( op->vertices.size() ), 
					&op->vertices[0], op->vertices.size() * sizeof( FlatShadeOp::Vertex ) );

				// copy indices
				UInt16* destIndices = stream.obtainIndices( op->indices.size() );
				for( Int32 i = 0; i < op->indices.size(); ++i )
				{
					destIndices[i] = firstVtxIndex + op->indices[i];
				}

				FlatShadeBatch batch;
				batch.firstIndex = firstIdxIndex;
				batch.numIndices = op->indices.size();
				batch.alphaEnabled = op->alphaEnabled;
				m_flatShadeBatches.push( batch );
			}
		}
	}

	void Layer::generateTextBatches( TextStream& stream )
	{
		UInt32 numOps;
		TextOp* ops = m_canvas.getTextOps( numOps );

		for( UInt32 i = 0; i < numOps; ++i )
		{
			TextOp* op = &ops[i];

			if( op->indices.size() > 0 && op->vertices.size() > 0 )
			{
				UInt32 firstVtxIndex = stream.getVertexCount();	
				UInt32 firstIdxIndex = stream.getIndexCount();

				// copy vertices
				mem::copy( stream.obtainVertices( op->vertices.size() ), 
					&op->vertices[0], op->vertices.size() * sizeof( TextOp::Vertex ) );

				// copy indices
				UInt16* destIndices = stream.obtainIndices( op->indices.size() );
				for( Int32 i = 0; i < op->indices.size(); ++i )
				{
					destIndices[i] = firstVtxIndex + op->indices[i];
				}

				TextBatch batch;
				batch.firstIndex = firstIdxIndex;
				batch.numIndices = op->indices.size();
				batch.srv = op->srv;
				m_textBatches.push( batch );
			}
		}
	}

	void Layer::drawFlatShadeBatches( rend::Device* device, ffx::Effect::Ptr effect )
	{
		assert( device && effect.hasObject() );

		device->setTopology( rend::EPrimitiveTopology::TriangleList );

		for( auto& it : m_flatShadeBatches )
		{
			effect->setBlendState( it.alphaEnabled ? m_blendStateAlpha : m_blendStateNone );
			effect->apply();

			device->drawIndexed( it.numIndices, it.firstIndex, 0 );
		}
	}

	void Layer::drawImageBatches( rend::Device* device, ffx::Effect::Ptr effect )
	{
	}

	void Layer::drawTextBatches( rend::Device* device, ffx::Effect::Ptr effect )
	{
		assert( device && effect.hasObject() );

		device->setTopology( rend::EPrimitiveTopology::TriangleList );
		
		effect->setBlendState( m_blendStateAlpha );
		effect->setSamplerState( 0, m_samplerStateLinearWrap );

		for( auto& it : m_textBatches )
		{
			effect->setSRV( 0, it.srv );
			effect->apply();

			device->drawIndexed( it.numIndices, it.firstIndex, 0 );
		}
	}
}
}
}