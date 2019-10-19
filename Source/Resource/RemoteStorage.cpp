//-----------------------------------------------------------------------------
//	RemoteStorage.cpp: A remote machine storage implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Resource.h"

namespace flu
{
namespace res
{
	RemoteStorage::RemoteStorage( String serverAddress )
	{
		net::Address address = net::Address::fromString( serverAddress );
		if( !address.isValid() )
		{
			fatal( L"Bad ResourceServer address \"%s\"", *serverAddress );
		}

		m_client = new ResourceClient( ConfigManager::getApplicationName(), address );

		m_listener = nullptr;
	}

	RemoteStorage::~RemoteStorage()
	{
		m_client = nullptr;
	}

	void RemoteStorage::setListener( IListener* listener )
	{
		m_listener = listener;
	}

	CompiledResource RemoteStorage::requestCompiled( EResourceType type, String resourceName )
	{
		// store for future usage
		ResourceId resourceId( type, resourceName );
		m_namesResolver.addName( resourceId, resourceName );

		return m_client->requestCompiled( type, resourceName );
	}

	CompiledResource RemoteStorage::requestCompiled( ResourceId resourceId )
	{
		return m_client->requestCompiled( resourceId );
	}

	String RemoteStorage::resolveResourceId( ResourceId resourceId )
	{
		// find in cache
		String resourceName = m_namesResolver.getName( resourceId );

		if( !resourceName )
		{
			// request from the server
			resourceName = m_client->resolveResourceId( resourceId );
			m_namesResolver.addName( resourceId, resourceName );
		}

		return resourceName;
	}

	void RemoteStorage::update( ResourceSystemList& systemList )
	{
		Array<ResourceId> changedResources = m_client->trackChanges();

		for( auto& it : changedResources )
		{
			IResourceSystem* system = systemList[static_cast<SizeT>( it.getType() )].get();
			assert( system );

			if( system->allowHotReloading() )
			{
				String prettyName = m_namesResolver.getPrettyName( it );

				if( m_listener )
				{
					m_listener->onInfo( prettyName, TXT( "changed from the outside, reloading..." ) );
				}

				CompiledResource recompiled = m_client->requestCompiled( it );

				if( recompiled.isValid() )
				{
					system->reloadResource( it, recompiled );
				}
				else
				{
					if( m_listener )
					{
						m_listener->onError( prettyName, TXT( "unable to reload changed resource" ) );
					}
				}
			}
		}
	}
}
}