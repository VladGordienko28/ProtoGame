//-----------------------------------------------------------------------------
//	ResourceServerApp.cpp: A resource server app implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Shell.h"

namespace flu
{
namespace shell
{
	ResourceServerApp::ResourceServerApp()
	{
	}

	ResourceServerApp::~ResourceServerApp()
	{
	}

	Bool ResourceServerApp::create( String commandLine )
	{
		// initialize network
		if( !net::NetworkManager::create() )
		{
			return false;
		}

		// initialize resource server
		if( !res::ResourceServer::create() )
		{
			return false;
		}

		res::ResourceServer::registerResourceType( res::EResourceType::Effect, new ffx::Compiler( new dx11::ShaderCompiler() ) );
		res::ResourceServer::registerResourceType( res::EResourceType::Image, new img::Converter() );
		res::ResourceServer::registerResourceType( res::EResourceType::Font, new fnt::Compiler() );
		res::ResourceServer::registerResourceType( res::EResourceType::Sound, new aud::SoundCompiler() );
		res::ResourceServer::registerResourceType( res::EResourceType::Layout, new ui::Compiler() );

		info( L"ResourceServerApp successfully initialized" );
		return true;
	}

	Bool ResourceServerApp::destroy()
	{	
		res::ResourceServer::destroy();
		net::NetworkManager::destroy();

		info( L"ResourceServerApp shutdown" );
		return true;
	}

	Int32 ResourceServerApp::run()
	{
		while( true ) // todo: add signal for exit
		{
			const Bool wasProcessed = res::ResourceServer::processRequests();

			if( !wasProcessed )
			{
				// nothing were processed, so we have time to idle
				threading::sleep( SERVER_IDLE_MS );		
			}
		}

		return 0;
	}
}
}