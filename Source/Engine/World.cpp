//-----------------------------------------------------------------------------
//	World.cpp: An engine world
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine.h"

namespace flu
{
	World::World( rend::Device* inRenderDevice )
		:	m_renderDevice( inRenderDevice ),
			m_drawContext( inRenderDevice, m_sharedConstants ),
			m_sharedConstants( inRenderDevice )
	{
		assert( inRenderDevice );

		// initialize resource manager
		{
			String packagesPath	= ConfigManager::readString( EConfigFile::System, TXT("ResMan"), TXT("PackagesPath") );
			String cachePath	= ConfigManager::readString( EConfigFile::System, TXT("ResMan"), TXT("CachePath") );
			Bool useCache		= ConfigManager::readString( EConfigFile::System, TXT("ResMan"), TXT("UseCache") );


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
		}

		// initialize engine gfx
		gfx::api::initialize( m_renderDevice );
	}

	World::~World()
	{

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
		Float lockTime = math::fMod( GPlat->Now(), 1000.f * 2.f * math::PI );

		gfx::SharedConstants::PerFrameData perFrameData;
		perFrameData.gameTime = lockTime;

		m_sharedConstants.setPerFrameData( perFrameData );
		m_sharedConstants.bindToPipeline();

		// clear swap-chain buffer
		m_renderDevice->beginFrame();
		m_renderDevice->clearRenderTarget( INVALID_HANDLE<rend::RenderTargetHandle>(), math::colors::BLACK );
	}

	void World::onEndUpdate()
	{
		updateMetrics();

		profile_zone( EProfilerGroup::Render, Present );
		m_renderDevice->endFrame();
	}

	void World::onResize( UInt32 newWidth, UInt32 newHeight, Bool fullScreen )
	{
	}



/*/
		// rendering systems
		rend::Device* m_renderDevice;
		gfx::DrawContext m_drawContext;
*/



	void World::updateMetrics()
	{
		// RAM memory stats
		profile_counter( EProfilerGroup::RAMMemory,	Total_Kb,	mem::stats().totalAllocatedBytes / 1024 );
		profile_counter( EProfilerGroup::RAMMemory,	Peak_Kb,	mem::stats().peakAllocatedBytes / 1024 );

		// GPU memory stats
		profile_counter( EProfilerGroup::GPUMemory, Total_Kb,		m_renderDevice->getMemoryStats().totalBytes() / 1024 );
		profile_counter( EProfilerGroup::GPUMemory, Texture_Kb,		m_renderDevice->getMemoryStats().m_texureBytes / 1024 );
		profile_counter( EProfilerGroup::GPUMemory, Vertex_Kb,		m_renderDevice->getMemoryStats().m_vertexBufferBytes / 1024 );
		profile_counter( EProfilerGroup::GPUMemory, Constant_Kb,	m_renderDevice->getMemoryStats().m_constantBufferBytes / 1024 );
		profile_counter( EProfilerGroup::GPUMemory, Index_Kb,		m_renderDevice->getMemoryStats().m_indexBufferBytes / 1024 );

		// Draw calls stats
		profile_counter( EProfilerGroup::DrawCalls, Draw_Calls,		m_renderDevice->getDrawStats().m_drawCalls );
		profile_counter( EProfilerGroup::DrawCalls, BS_Switches,	m_renderDevice->getDrawStats().m_blendStateSwitches );
		profile_counter( EProfilerGroup::DrawCalls, RS_Switches,	m_renderDevice->getDrawStats().m_renderStateSwitches );
	}
}