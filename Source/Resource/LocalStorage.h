//-----------------------------------------------------------------------------
//	LocalStorage.h: A local resource storage
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	A local storage, which compile, cache and reload resources
	 *	from the disk ( working copy )
	 */
	class LocalStorage: public IStorage
	{
	public:
		using UPtr = UniquePtr<LocalStorage>;

		LocalStorage( String packagesPath, String cachePath, Bool useCache );
		~LocalStorage();

		void registerCompiler( EResourceType type, IResourceCompiler* compiler );

		void setListener( IListener* listener ) override;

		CompiledResource requestCompiled( EResourceType type, String resourceName ) override;
		CompiledResource requestCompiled( ResourceId resourceId ) override;

		String resolveResourceId( ResourceId resourceId ) override;

		Array<ResourceId> trackChanges();
		void update( ResourceSystemList& systemList ) override;

		EResourceType getFileResourceType( String fileName ) const;

		String getPackagesPath() const
		{
			return m_packagesPath;
		}

		template<typename T, typename ...Args> typename Bool construct( String resourceName, 
			IListener& listener, Args... args );

	private:
		static constexpr const Char CACHE_EXTENSION[] = TXT( ".fcache" );

		String m_packagesPath;
		String m_cachePath;
		Bool m_useCache;

		FilesTracker::UPtr m_filesTracker;
		NamesResolver m_namesResolver;

		StaticArray<IResourceCompiler::UPtr, Resource::NUM_TYPES> m_compilers;

		IListener* m_listener;

		LocalStorage() = delete;

		CompiledResource requestCompiledImpl( EResourceType type, String resourceName, Bool allowCached );
		CompiledResource requestCompiledImpl( ResourceId resourceId, Bool allowCached );

		CompiledResource loadFromCache( ResourceId resourceId, String resourceName );
		void saveToCache( ResourceId resourceId, String resourceName, const CompilationOutput& output );

		CompilationOutput compile( ResourceId resourceId, String resourceName );

		String findResourceFile( String resourceName, EResourceType* outType ) const;

		String prepareResourceFolder( String resourceName );

		IResourceCompiler* getCompiler( EResourceType resType ) const
		{
			return m_compilers[static_cast<SizeT>( resType )].get();
		}
	};

	template<typename T, typename ...Args> typename Bool LocalStorage::construct( String resourceName, 
		IListener& listener, Args... args )
	{
		// make sure destination directory exists
		String resourcePath = prepareResourceFolder( resourceName, listener );
		if( !resourcePath )
		{
			return false;
		}

		// parse resource file name
		String resourceFileName = cstr::findRevChar( *resourceName, TXT('.') ) + 1;

		class ConstructionEnvironment: public IConstructionEnvironment
		{
		public:
			ConstructionEnvironment( String basePath )
				:	m_basePath( basePath )
			{
			}

			Bool writeTextFile( String relativeFileName, Text::Ptr text ) const override
			{
				assert( text.hasObject() );
				return fm::writeTextFile( *( m_basePath + relativeFileName ), text );
			}

			IOutputStream::Ptr writeBinaryFile( String relativeFileName ) const override
			{
				return fm::writeBinaryFile( *( m_basePath + relativeFileName ) ).get();
			}

		private:
			String m_basePath;
		};

		ConstructionEnvironment environment( m_packagesPath + resourcePath );

		// construct resource
		typename T::CompilerType* compiler = dynamic_cast<typename T::CompilerType*>( getCompiler( T::RESOURCE_TYPE ) );
		assert( compiler );

		String errorMsg;
		if( compiler->construct( resourceFileName, environment, errorMsg, args... ) )
		{
			return true;
		}
		else
		{
			listener.onError( resourceName, errorMsg );
			return false;
		}
	}
}
}