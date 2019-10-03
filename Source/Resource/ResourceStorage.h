//-----------------------------------------------------------------------------
//	ResourceStorage.h: A resource storages
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

		IResourceCompiler* getCompiler( EResourceType resType ) const
		{
			return m_compilers[static_cast<SizeT>( resType )].get();
		}
	};

	/**
	 *	A package storage, which load compiled resources from the
	 *	packages
	 */
	class PackageStorage final: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<PackageStorage>;

		PackageStorage();
		~PackageStorage();

		CompiledResource requestCompiled( EResourceType type, String resourceName, IListener& listener );
		CompiledResource requestCompiled( ResourceId resourceId, IListener& listener );

		String resolveResourceId( ResourceId resourceId ) const;

		Bool loadAllPackages( String directory );
		Bool loadPackage( String fileName );

		Array<String> getPackageNames() const;

		static UInt32 generatePackages( LocalStorage* localStorage, String outputPath );

	private:
		Array<Package*> m_packages;
		Map<ResourceId, Package*> m_resourceId2Package;

		NamesResolver m_namesResolver;
	};
}
}