//-----------------------------------------------------------------------------
//	ResourceManager.cpp: A Resource Manager implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Resource.h"

namespace flu
{
namespace res
{
	ResourceManager::ResourceManager()
		:	m_isInitialized( false )
	{
	}

	ResourceManager::~ResourceManager()
	{
		if( m_isInitialized )
		{
			fatal( TXT( "ResourceManager wasn't deinitialized properly" ) );
		}
	}

	Bool ResourceManager::create()
	{
		return instance().createImpl();
	}

	void ResourceManager::destroy()
	{
		instance().destroyImpl();
	}

	void ResourceManager::registerResourceType( EResourceType type, IResourceSystem* system,
		IResourceCompiler* compiler )
	{
		instance().registerResourceTypeImpl( type, system, compiler );
	}

	void ResourceManager::update()
	{
		instance().updateImpl();
	}

	Bool ResourceManager::generatePackages()
	{
		// todo: cleanup this mess
		ResourceManager& manager = instance();

		assert( manager.m_localStorage );
		return PackageStorage::generatePackages( manager.m_localStorage.get(), manager.m_localStorage->getPackagesPath() ) > 0;
	}

	Bool ResourceManager::loadAllPackages()
	{
		// todo: cleanup this mess
		ResourceManager& manager = instance();
		if( manager.m_packageStorage.hasObject() )
		{
			return manager.m_packageStorage->loadAllPackages( manager.m_localStorage->getPackagesPath() );
		}
		else
		{
			return false;
		}
	}

	Bool ResourceManager::createImpl()
	{
		assert( !m_isInitialized );

		String storageType = ConfigManager::readString( EConfigFile::Application, TXT("ResourceManager"), TXT("Storage") );

		if( storageType == TXT("LOCAL") )
		{
			String packagesPath = ConfigManager::readString( EConfigFile::Application, TXT("ResourceManager"), TXT("PackagesPath") );
			String cachePath = ConfigManager::readString( EConfigFile::Application, TXT("ResourceManager"), TXT("CachePath") );
			Bool useCache = ConfigManager::readString( EConfigFile::Application, TXT("ResourceManager"), TXT("UseCache") );

			m_localStorage = new LocalStorage( packagesPath, cachePath, useCache );
			m_storage = m_localStorage.get();
		}
		else if( storageType == TXT("PACKAGE") )
		{
			m_packageStorage = new PackageStorage();
			m_storage = m_packageStorage.get();
		}
		else if( storageType == TXT("REMOTE") )
		{
			String serverAddress = ConfigManager::readString( EConfigFile::Application, TXT("ResourceManager"), TXT("ResourceServer") );
			
			m_remoteStorage = new RemoteStorage( serverAddress );
			m_storage = m_remoteStorage.get();
		}
		else
		{
			fatal( L"Unknown resource storage type \"%s\"", *storageType );
			return false;
		}

		m_storage->setListener( &m_listener );

		m_isInitialized = true;
		info( L"ResourceManager is initialized" );

		return true;
	}

	void ResourceManager::destroyImpl()
	{
		assert( m_isInitialized );
		assert( m_requestsStack.isEmpty() );

		// shutdown all systems
		for( auto& it : m_systems )
		{
			it = nullptr;
		}

		// shutdown one of the storages
		m_localStorage = nullptr;
		m_packageStorage = nullptr;
		m_remoteStorage = nullptr;

		m_isInitialized = false;
		info( L"ResourceManager is deinitialized" );
	}

	void ResourceManager::registerResourceTypeImpl( EResourceType type, IResourceSystem* system,
		IResourceCompiler* compiler )
	{
		assert( m_isInitialized );

		SizeT resTypeId = static_cast< SizeT >( type );

		assert( system );
		assert( !m_systems[resTypeId].hasObject() );

		m_systems[resTypeId] = system;

		if( m_localStorage.hasObject() && compiler )
		{
			m_localStorage->registerCompiler( type, compiler );
		}

		if( !m_localStorage.hasObject() && compiler )
		{
			// current storage is not allow compilation
			delete compiler;
		}

		info( L"Registered a new resource type %d", type );
	}

	void ResourceManager::updateImpl()
	{
		assert( m_isInitialized );	
		assert( m_storage );

		m_storage->update( m_systems );
	}

	void ResourceManager::addListener( IListener* listener )
	{
		assert( instance().m_isInitialized );
		instance().m_listener.addListener( listener );
	}

	void ResourceManager::removeListener( IListener* listener )
	{
		assert( instance().m_isInitialized );
		instance().m_listener.removeListener( listener );
	}

	ResourceManager& ResourceManager::instance()
	{
		static ResourceManager manager;
		return manager;
	}

	CompiledResource ResourceManager::requestCompiled( EResourceType type, String resourceName )
	{
		assert( m_storage );

		return m_storage->requestCompiled( type, resourceName );
	}

	CompiledResource ResourceManager::requestCompiled( ResourceId resourceId )
	{
		assert( m_storage );

		return m_storage->requestCompiled( resourceId );
	}

	String ResourceManager::resolveResourceId( ResourceId resourceId ) const
	{
		assert( m_storage );

		return m_storage->resolveResourceId( resourceId );
	}
}
}