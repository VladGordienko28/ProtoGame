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
	class LocalStorage final: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<LocalStorage>;

		LocalStorage( String packagesPath, String cachePath, Bool useCache );
		~LocalStorage();

		void registerCompiler( EResourceType type, IResourceCompiler* compiler );

		CompiledResource requestCompiled( EResourceType type, String resourceName, 
			IListener& listener, Bool allowCached );

		CompiledResource requestCompiled( ResourceId resourceId,
			IListener& listener, Bool allowCached );

		String resolveResourceId( ResourceId resourceId ) const;

		void reloadChanged( ResourceSystemList& systems, IListener& listener );

		EResourceType getFileResourceType( String fileName ) const;

		String getPackagesPath() const
		{
			return m_packagesPath;
		}

		template<typename T, typename ...Args> typename Bool construct( String resourceName, 
			IListener& listener, Args... args );

	private:
		static constexpr const Char CACHE_EXTENSION[] = TEXT( ".fcache" );

		String m_packagesPath;
		String m_cachePath;
		Bool m_useCache;

		FilesTracker::UPtr m_filesTracker;
		NamesResolver m_namesResolver;

		StaticArray<IResourceCompiler::UPtr, Resource::NUM_TYPES> m_compilers;

		LocalStorage() = delete;

		CompiledResource loadFromCache( ResourceId resourceId, String resourceName, IListener& listener );
		void saveToCache( ResourceId resourceId, String resourceName, const CompilationOutput& output, IListener& listener );

		CompilationOutput compile( ResourceId resourceId, String resourceName, IListener& listener );

		String findResourceFile( String resourceName, EResourceType* outType ) const;

		String prepareResourceFolder( String resourceName, IListener& listener );

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
		String resourceFileName = cstr::findRevChar( *resourceName, TEXT('.') ) + 1;

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