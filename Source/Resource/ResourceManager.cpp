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
		:	m_packagesPath( TEXT( "" ) ),
			m_isInitialized( false )
	{
	}

	ResourceManager::~ResourceManager()
	{
		if( m_isInitialized )
		{
			fatal( TEXT( "ResourceManager wasn't deinitialized properly" ) );
		}
	}

	Bool ResourceManager::create( String packagesPath, String cachePath, Bool useCache )
	{
		return instance().createImpl( packagesPath, cachePath, useCache );
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
		return PackageStorage::generatePackages( manager.m_localStorage.get(), manager.m_packagesPath ) > 0;
	}

	Bool ResourceManager::loadAllPackages()
	{
		// todo: cleanup this mess
		ResourceManager& manager = instance();
		if( manager.m_packageStorage.hasObject() )
		{
			return manager.m_packageStorage->loadAllPackages( manager.m_packagesPath );
		}
		else
		{
			return false;
		}
	}

	Bool ResourceManager::createImpl( String packagesPath, String cachePath, Bool useCache )
	{
		assert( !m_isInitialized );

		m_packagesPath = fm::resolveFileName( *packagesPath, fm::EPathBase::Exe );

		m_localStorage = new LocalStorage( packagesPath, cachePath, useCache );

		m_packageStorage = new PackageStorage();

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

		info( L"Registered a new resource type %d", type );
	}

	void ResourceManager::updateImpl()
	{
		assert( m_isInitialized );	

		if( m_localStorage.hasObject() )
		{
			m_localStorage->reloadChanged( m_systems, m_listener );
		}
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
		CompiledResource compiledResource;
/*
		// attempt to load from compiled package
		if( m_packageStorage.hasObject() )
		{
			compiledResource = m_packageStorage->requestCompiled( type, resourceName, m_listener );
			
			if( compiledResource.isValid() )
			{
				return compiledResource;
			}
		}*/

		// attempt to compile or load from the cache
		if( m_localStorage.hasObject() )
		{
			compiledResource = m_localStorage->requestCompiled( type, resourceName, m_listener, true );
		}

		return compiledResource;
	}

	CompiledResource ResourceManager::requestCompiled( ResourceId resourceId )
	{
		CompiledResource compiledResource;
/*
		// attempt to load from compiled package
		if( m_packageStorage.hasObject() )
		{
			compiledResource = m_packageStorage->requestCompiled( type, resourceName, m_listener );
			
			if( compiledResource.isValid() )
			{
				return compiledResource;
			}
		}*/

		// attempt to compile or load from the cache
		if( m_localStorage.hasObject() )
		{
			compiledResource = m_localStorage->requestCompiled( resourceId, m_listener, true );
		}

		return compiledResource;
	}

	String ResourceManager::resolveResourceId( ResourceId resourceId ) const
	{
		if( m_localStorage.hasObject() )
		{
			return m_localStorage->resolveResourceId( resourceId );
		}

		return TEXT( "" );
	}
}
}