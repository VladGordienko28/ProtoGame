//-----------------------------------------------------------------------------
//	World.cpp: An engine world
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine.h"

namespace flu
{
	World::World( rend::Device* renderDevice, aud::Device* audioDevice, in::Device* inputDevice )
		:	m_renderDevice( renderDevice ),
			m_audioDevice( audioDevice ),
			m_inputDevice( inputDevice ),
			m_drawContext( renderDevice, m_sharedConstants ),
			m_sharedConstants( renderDevice )
	{
		assert( renderDevice );
		assert( inputDevice );

		// initialize resource manager
		{


			// fooooooooooooo
			net::NetworkManager::create();

			Bool created = res::ResourceManager::create(  );


			if( !created )
			{
				fatal( TXT("Resource Manager initialization failed") );
			}
		}

		// register all resource types
		{
			using namespace res;

			ResourceManager::registerResourceType( EResourceType::Effect, new ffx::System( m_renderDevice ), new ffx::Compiler( m_renderDevice->createCompiler() ) );
			ResourceManager::registerResourceType( EResourceType::Image, new img::System( m_renderDevice ), new img::Converter() );
			ResourceManager::registerResourceType( EResourceType::Font, new fnt::System(), new fnt::Compiler() );
			ResourceManager::registerResourceType( EResourceType::Sound, new aud::SoundSystem( m_audioDevice ), new aud::SoundCompiler() );

			ResourceManager::registerResourceType( EResourceType::Layout, new ui::System(), new ui::Compiler() );
		}

		// initialize engine gfx
		gfx::api::initialize( m_renderDevice );


		m_engineChart = new EngineChart();
		m_inputDevice->addClient( m_engineChart.get() );

		m_uiRoot = new ui::Root( m_renderDevice );
		m_inputDevice->addClient( m_uiRoot.get() );


		ui::UserLayout* elem = new ui::UserLayout( L"TestLayout", m_uiRoot.get() );
		elem->setHorizAlign( ui::EHorizAlign::Stretch );
		elem->setVertAlign( ui::EVertAlign::Stretch );
		elem->load( L"Experimental.TestUI" );
		m_uiRoot->addChild( elem );



		/*
		ui::Button* elem = new ui::Button(L"Button0",m_uiRoot.get());
		elem->setSize( 100, 64 );
		elem->setHorizAlign( ui::EHorizAlign::Center );
		elem->setVertAlign( ui::EVertAlign::Center );

		m_uiRoot->addChild( elem );


		elem = new ui::Button(L"Button1",m_uiRoot.get());
		elem->setSize( 100, 64 );
		elem->setHorizAlign( ui::EHorizAlign::Stretch );
		elem->setVertAlign( ui::EVertAlign::Bottom );

		//elem->m_margin.left = 30;
		//elem->m_margin.bottom = 100;

		m_uiRoot->addChild( elem );
		m_uiRoot->setPaddingBottom( 30.f );
		m_uiRoot->setPaddingLeft( 10.f );
		m_uiRoot->setPaddingRight( 10.f );

		auto lay = res::ResourceManager::get<ui::Layout>( L"Experimental.TestUI" );*/

	}

	World::~World()
	{
		m_inputDevice->removeClient( m_engineChart.get() );
		m_engineChart = nullptr;

		m_inputDevice->removeClient( m_uiRoot.get() );
		m_uiRoot = nullptr;


		gfx::api::finalize();
		res::ResourceManager::destroy();
	}
/*
	void World::onUpdate()
	{
		Float lockTime = math::fMod( GPlat->Now(), 1000.f * 2.f * math::PI );

		gfx::SharedConstants::PerFrameData perFrameData;
		perFrameData.gameTime = lockTime;

		m_drawContext.sharedConstants().setPerFrameData( perFrameData );
		m_drawContext.sharedConstants().bindToPipeline();

	}
*/

	void World::onBeginUpdate()
	{
		Float lockTime = /*math::fMod( GPlat->Now(), 1000.f * 2.f * math::PI )*/0;

		gfx::SharedConstants::PerFrameData perFrameData;
		perFrameData.gameTime = lockTime;

		m_sharedConstants.setPerFrameData( perFrameData );
		m_sharedConstants.bindToPipeline();

		// clear swap-chain buffer
		m_renderDevice->beginFrame();

		if( m_renderDevice->getProfiler() )
		{
			m_renderDevice->getProfiler()->beginFrame();
		}

		m_renderDevice->clearRenderTarget( INVALID_HANDLE<rend::RenderTargetHandle>(), math::colors::BLACK );
	}

	void World::onEndUpdate( CCanvas* canvas ) // fo!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	{
		m_engineChart->render( canvas, m_drawContext );


		updateMetrics();

		////////////////////////////////////////////////////////////////////
		const Float screenW = m_drawContext.backbufferWidth();
		const Float screenH = m_drawContext.backbufferHeight();

		m_drawContext.pushViewInfo( gfx::ViewInfo( 
			0.f, 0.f, screenW, screenH ) );

		m_uiRoot->update( 1.f / 60.f );	// todo: fooooooooo use real delta time, not hardcoded 60 Hz

		m_uiRoot->prepareBatches();
		m_uiRoot->flushBatches( );

		m_drawContext.popViewInfo();
		/////////////////////////////////////////////////////////////


		profile_zone( Render, Present );
		m_renderDevice->endFrame( true );

		if( m_renderDevice->getProfiler() )
		{
			m_renderDevice->getProfiler()->endFrame();
		}
	}

	void World::onResize( UInt32 newWidth, UInt32 newHeight, Bool fullScreen )
	{
		m_uiRoot->resize( newWidth, newHeight );
	}



/*/
		// rendering systems
		rend::Device* m_renderDevice;
		gfx::DrawContext m_drawContext;
*/



	void World::updateMetrics()
	{
		// RAM memory stats
		profile_counter( RAM_Memory, Total_Kb,	mem::stats().totalAllocatedBytes / 1024 );
		profile_counter( RAM_Memory, Peak_Kb,	mem::stats().peakAllocatedBytes / 1024 );

		// GPU memory stats
		profile_counter( GPU_Memory, Total_Kb,		m_renderDevice->getMemoryStats().totalBytes() / 1024 );
		profile_counter( GPU_Memory, Texture_Kb,	m_renderDevice->getMemoryStats().m_texureBytes / 1024 );
		profile_counter( GPU_Memory, Vertex_Kb,		m_renderDevice->getMemoryStats().m_vertexBufferBytes / 1024 );
		profile_counter( GPU_Memory, Constant_Kb,	m_renderDevice->getMemoryStats().m_constantBufferBytes / 1024 );
		profile_counter( GPU_Memory, Index_Kb,		m_renderDevice->getMemoryStats().m_indexBufferBytes / 1024 );

		// Draw calls stats
		profile_counter( Draw_Calls, Draw_Calls,	m_renderDevice->getDrawStats().m_drawCalls );
		profile_counter( Draw_Calls, BS_Switches,	m_renderDevice->getDrawStats().m_blendStateSwitches );
		profile_counter( Draw_Calls, RS_Switches,	m_renderDevice->getDrawStats().m_renderStateSwitches );
	}
}