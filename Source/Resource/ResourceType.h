//-----------------------------------------------------------------------------
//	ResourceType.h: All supported resource types
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	A resource type
	 */
	enum class EResourceType: UInt32
	{
		Effect,
		Image,
		Font,
		Sound,
		MAX
	};
	ENUM_FOR_STREAM( EResourceType );

	/**
	 *	A binary representation of the resource
	 */
	struct CompiledResource
	{
	public:
		Array<UInt8> data;

		CompiledResource() = default;
		~CompiledResource() = default;

		Bool isValid() const
		{
			return data.size() > 0;
		}

		UInt32 getChecksum() const
		{
			return data.size() > 0 ? hashing::murmur32( &data[0], data.size() ) : 0;
		}

		friend IOutputStream& operator<<( IOutputStream& stream, const CompiledResource& x )
		{
			stream << x.data;
			return stream;
		}

		friend IInputStream& operator>>( IInputStream& stream, CompiledResource& x )
		{
			stream >> x.data;
			return stream;
		}
	};

	/**
	 *	An abstract resource
	 */
	class Resource: public ReferenceCount
	{
	public:
		static const SizeT NUM_TYPES = static_cast< SizeT >( EResourceType::MAX );

		Resource() = default;
		virtual ~Resource() = default;
	};

	/**
	 *	Resource declaration macro
	 */
	#define	DECLARE_RESOURCE( resName, systemName, compilerName )\
			using Ptr = SharedPtr<resName>;\
			typedef class systemName SystemType;\
			typedef class compilerName CompilerType;\
			static const res::EResourceType RESOURCE_TYPE = res::EResourceType::##resName;
}
}