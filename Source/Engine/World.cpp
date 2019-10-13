//-----------------------------------------------------------------------------
//	World.cpp: An engine world
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine.h"

namespace flu
{
	World::World( rend::Device* inRenderDevice )
		:	m_renderDevice( inRenderDevice ),
			m_drawContext( inRenderDevice )
	{
		assert( inRenderDevice );

		// initialize resource manager
		{
			String packagesPath	= ConfigManager::readString( EConfigFile::System, TEXT("ResMan"), TEXT("PackagesPath") );
			String cachePath	= ConfigManager::readString( EConfigFile::System, TEXT("ResMan"), TEXT("CachePath") );
			Bool useCache		= ConfigManager::readString( EConfigFile::System, TEXT("ResMan"), TEXT("UseCache") );

			Bool created = res::ResourceManager::create( packagesPath, cachePath, useCache );

			if( !created )
			{
				fatal( TEXT("Resource Manager initialization failed") );
			}
		}

		// register all resource types
		{
			using namespace res;

			ResourceManager::registerResourceType( EResourceType::Effect, new ffx::System( m_renderDevice ), new ffx::Compiler( m_renderDevice ) );
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

	void World::onUpdate()
	{
		Float lockTime = math::fMod( GPlat->Now(), 1000.f * 2.f * math::PI );

		gfx::SharedConstants::PerFrameData perFrameData;
		perFrameData.gameTime = lockTime;

		m_drawContext.sharedConstants().setPerFrameData( perFrameData );
		m_drawContext.sharedConstants().bindToPipeline();

	}

	void World::onResize( UInt32 newWidth, UInt32 newHeight, Bool fullScreen )
	{
	}



/*/
		// rendering systems
		rend::Device* m_renderDevice;
		gfx::DrawContext m_drawContext;
*/

}