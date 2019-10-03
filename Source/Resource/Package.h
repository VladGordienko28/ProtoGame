//-----------------------------------------------------------------------------
//	Package.h: A resource package
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	A package header
	 */
	struct PackageHeader
	{
	public:
		static const UInt32 MAGIC = 'FPKG';
		static const UInt32 VERSION = '0.1';

		UInt32 magic;
		UInt32 version;
		String name;
		UInt32 size;

		friend IOutputStream& operator<<( IOutputStream& stream, const PackageHeader& x )
		{
			stream << x.magic << x.version << x.name << x.size;
			return stream;
		}

		friend IInputStream& operator>>( IInputStream& stream, PackageHeader& x )
		{
			stream >> x.magic >> x.version >> x.name >> x.size;
			return stream;
		}
	};

	/**
	 *	A loaded package
	 */
	class Package final: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<Package>;
		static constexpr const Char* EXTENSION = TEXT( ".fpkg" );

		Package();
		~Package();

		Bool load( String fileName );
		Array<ResourceId> getResourceList() const;

		CompiledResource getResource( ResourceId resourceId );

		Bool fillNamesResolver( NamesResolver& resolver ) const;

		Bool isLoaded() const
		{
			return m_loader.hasObject();
		}

		String getName() const
		{
			return m_name;
		}

	private:
		struct ResourceEntry
		{
		public:
			String resourceName;
			UInt32 checksum;
			UInt32 dataOffset;
		};

		String m_name;
		Map<ResourceId, ResourceEntry> m_entries;

		IInputStream::Ptr m_loader;
	};
}
}