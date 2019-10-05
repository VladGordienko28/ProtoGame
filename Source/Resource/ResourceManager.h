//-----------------------------------------------------------------------------
//	ResourceManager.h: A Resource Manager
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	Describe what doing after bad loading
	 */
	enum class EFailPolicy
	{
		RETURN_NULL,
		FATAL
	};

	/**
	 *	A global resource manager
	 */
	class ResourceManager final: NonCopyable
	{
	public:
		static Bool create( String packagesPath, String cachePath, Bool useCache );
		static void destroy();

		static void registerResourceType( EResourceType type, IResourceSystem* system,
			IResourceCompiler* compiler );

		// todo: move to background thread
		static void update();

		static Bool generatePackages(); // todo: add output add output directory + flags
		static Bool loadAllPackages(); // todo: rethink

		template<typename T> static typename T::Ptr get( String resourceName, EFailPolicy failPolicy = EFailPolicy::RETURN_NULL );
		template<typename T> static typename T::Ptr get( ResourceId resourceId, EFailPolicy failPolicy = EFailPolicy::RETURN_NULL );

		static void addListener( IListener* listener );
		static void removeListener( IListener* listener );

	private:
		String m_packagesPath;
		Bool m_isInitialized;
		ListenerList m_listener;

		ResourceSystemList m_systems;

		LocalStorage::UPtr m_localStorage;
		PackageStorage::UPtr m_packageStorage;

		static const Int32 MAX_REQUESTS_DEPTH = 8;
		FixedStack<ResourceId> m_requestsStack;

		class ScopedRequest: public NonCopyable
		{
		public:
			ScopedRequest( FixedStack<ResourceId>& stack, ResourceId resourceId )
				:	m_stack( stack ),
					m_resourceId( resourceId )
			{
				if( m_stack.isInStack( m_resourceId ) )
				{
					fatal( TEXT( "Circular reference detected in \"%s\"" ), *m_resourceId.toString() );
				}

				m_stack.push( m_resourceId );
			}

			~ScopedRequest()
			{
				verify( m_stack.pop() == m_resourceId );
			}

		private:
			FixedStack<ResourceId>& m_stack;
			ResourceId m_resourceId;

			ScopedRequest() = delete;
		};

		ResourceManager();
		~ResourceManager();

		Bool createImpl( String packagesPath, String cachePath, Bool useCache );
		void destroyImpl();

		void registerResourceTypeImpl( EResourceType type, IResourceSystem* system,
			IResourceCompiler* compiler );

		void updateImpl();

		CompiledResource requestCompiled( EResourceType type, String resourceName );
		CompiledResource requestCompiled( ResourceId resourceId );

		String resolveResourceId( ResourceId resourceId ) const;

		IResourceSystem* getSystem( EResourceType resType ) const
		{
			return m_systems[static_cast<SizeT>( resType )].get();
		}

		static ResourceManager& instance();
	};

	template<typename T> static typename T::Ptr ResourceManager::get( String resourceName, EFailPolicy failPolicy )
	{
		ResourceManager& manager = instance();
		assert( manager.m_isInitialized );

		const EResourceType resType = T::RESOURCE_TYPE;
		IResourceSystem* system = manager.getSystem( resType );
		assert( system );

		ResourceId resourceId( resType, resourceName );
		ScopedRequest sr( manager.m_requestsStack, resourceId );

		if( system->hasResource( resourceId ) )
		{
			return dynamic_cast< T* >( system->getResource( resourceId ) );
		}
		else
		{
			CompiledResource compiledResource = manager.requestCompiled( resType, resourceName );

			if( compiledResource.isValid() )
			{
				return dynamic_cast< T* >( system->createResource( resourceName, resourceId, compiledResource ) );			
			}
			else
			{
				if( failPolicy == EFailPolicy::FATAL )
				{
					fatal( TEXT( "Resource \"%s\" is not found" ), *resourceName );
				}

				manager.m_listener.onError( resourceName, TEXT( "is not found" ) );
				return nullptr;
			}
		}
	}

	template<typename T> static typename T::Ptr ResourceManager::get( ResourceId resourceId, EFailPolicy failPolicy )
	{
		ResourceManager& manager = instance();
		assert( manager.m_isInitialized );
		assert( resourceId != ResourceId::NONE() );

		const EResourceType resType = T::RESOURCE_TYPE;
		IResourceSystem* system = manager.getSystem( resType );
		assert( system );
	
		ScopedRequest sr( manager.m_requestsStack, resourceId );

		if( system->hasResource( resourceId ) )
		{
			return dynamic_cast< T* >( system->getResource( resourceId ) );
		}
		else
		{
			CompiledResource compiledResource = manager.requestCompiled( resourceId );

			if( compiledResource.isValid() )
			{
				String resolvedName = manager.resolveResourceId( resourceId );
				assert( resolvedName );
				return dynamic_cast< T* >( system->createResource( resolvedName, resourceId, compiledResource ) );		
			}
			else
			{
				if( failPolicy == EFailPolicy::FATAL )
				{
					fatal( TEXT( "Resource \"%s\" is not found" ), *resourceId.toString() );
				}

				manager.m_listener.onError( resourceId.toString(), TEXT( "is not found" ) );
				return nullptr;
			}
		}
	}
}
}