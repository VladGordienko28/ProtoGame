//-----------------------------------------------------------------------------
//	PackageStorage.h: A package storage
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	A package storage, which load compiled resources from the
	 *	packages
	 */
	class PackageStorage final: public IStorage
	{
	public:
		using UPtr = UniquePtr<PackageStorage>;

		PackageStorage();
		~PackageStorage();

		void setListener( IListener* listener ) override;

		CompiledResource requestCompiled( EResourceType type, String resourceName ) override;
		CompiledResource requestCompiled( ResourceId resourceId ) override;

		String resolveResourceId( ResourceId resourceId ) override;

		void update( ResourceSystemList& systemList ) override;

		Bool loadAllPackages( String directory );
		Bool loadPackage( String fileName );

		Array<String> getPackageNames() const;

		static UInt32 generatePackages( LocalStorage* localStorage, String outputPath );

	private:
		Array<Package*> m_packages;
		Map<ResourceId, Package*> m_resourceId2Package;

		NamesResolver m_namesResolver;

		IListener* m_listener;
	};
}
}