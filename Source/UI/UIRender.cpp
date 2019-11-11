//-----------------------------------------------------------------------------
//	UIRender.cpp: An UI render
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "UI.h"

namespace flu
{
namespace ui
{
	/**
	 *	An UI render implementation
	 */
	class RenderImpl: public Render
	{
	public:
		RenderImpl( rend::Device* device );
		~RenderImpl();

		void prepareBatches( Container* treeRoot ) override;
		void flushBatches() override;

	private:
		static const UInt32 MAX_LAYERS = 16;
		using LayerList = rendering::Layer[MAX_LAYERS];

		LayerList m_layers;
		UInt32 m_numLayers;

		rendering::FlatShadeStream m_flatShadeStream;
		rendering::ImageStream m_imageStream;
		rendering::TextStream m_textStream;

		rend::Device* m_device;
	};

	Render* Render::createRender( rend::Device* device )
	{
		return new RenderImpl( device );
	}

	RenderImpl::RenderImpl( rend::Device* device )
		:	m_device( device ),
			m_flatShadeStream( device ),
			m_imageStream( device ),
			m_textStream( device ),
			m_numLayers( 0 )
	{
		assert( m_device );

		for( auto& layer : m_layers )
		{
			layer.create( m_device );
		}
	}

	RenderImpl::~RenderImpl()
	{
		for( auto& layer : m_layers )
		{
			layer.destroy();
		}
	}

	void RenderImpl::prepareBatches( Container* treeRoot )
	{
		profile_zone( UI, CPU_PrepareBatches );

		assert( treeRoot );
		m_numLayers = 0;

		class DrawVisitor: public IVisitor
		{
		public:
			DrawVisitor( LayerList& layers, UInt32& layerDepth )
				:	m_layers( layers ),
					m_layerDepth( layerDepth ),
					m_stackTop( 0 )
			{
			}

			~DrawVisitor()
			{
			}

			void visit( Element* element ) override
			{
				if( m_stackTop > 0 ) // todo: fix hack for Root element
				{
					element->draw( m_layers[m_stackTop-1].getCanvas() );				
				}
			}

			void enterContainer( Container* container ) override
			{
				++m_stackTop;
				m_layerDepth = max( m_layerDepth, m_stackTop );
				assert( m_stackTop < MAX_LAYERS );
			}

			void leaveContainer( Container* container ) override
			{
				assert( m_stackTop > 0 );
				--m_stackTop;
			}
		
		private:
			LayerList& m_layers;
			UInt32& m_layerDepth;

			UInt32 m_stackTop;
		};

		// travers elements and fill layers with data
		DrawVisitor drawVisitor( m_layers, m_numLayers );
		treeRoot->visit( drawVisitor );

		assert( m_numLayers > 0 );

		// generate batches to draw
		for( UInt32 i = 0; i < m_numLayers; ++i )
		{
			rendering::Layer& layer = m_layers[i];

			layer.generateFlatShadeBatches( m_flatShadeStream );
			layer.generateImageBatches( m_imageStream );
			layer.generateTextBatches( m_textStream );
		}
	}

	void RenderImpl::flushBatches()
	{
		profile_zone( UI, CPU_FlushBatches );
		m_device->enterZone( L"DrawUI" );
		m_device->getProfiler()->enterZone( L"GPU_DrawUI" );

		// submit all streams
		m_flatShadeStream.submitToGPU();
		m_imageStream.submitToGPU();
		m_textStream.submitToGPU();

		// draw all layers
		assert( m_numLayers > 0 );

		for( UInt32 i = 0; i < m_numLayers; ++i ) // todo: flip direction and add pass support
		{
			rendering::Layer& layer = m_layers[i];

			// flat shade ops
			if( layer.hasFlatShadeBatches() )
			{
				m_flatShadeStream.bindBuffers();
				layer.drawFlatShadeBatches( m_device, m_flatShadeStream.getEffect() );
			}

			// image ops
			if( layer.hasImageBatches() )
			{
				m_imageStream.bindBuffers();
				layer.drawImageBatches( m_device, m_imageStream.getEffect() );
			}
		
			// text ops
			if( layer.hasTextBatches() )
			{
				m_textStream.bindBuffers();
				layer.drawTextBatches( m_device, m_textStream.getEffect() );
			}

			layer.clear();
		}
		m_numLayers = 0;

		// clear all CPU buffers
		m_flatShadeStream.clearBuffers();
		m_imageStream.clearBuffers();
		m_textStream.clearBuffers();

		m_device->getProfiler()->leaveZone();
		m_device->leaveZone();
	}
}
}