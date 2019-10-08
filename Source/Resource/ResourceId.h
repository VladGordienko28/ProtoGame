//-----------------------------------------------------------------------------
//	ResourceId.h: A resource's short name
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	A resource id
	 */
	struct ResourceId
	{
	public:
		ResourceId()
			:	type( EResourceType::MAX ),
				hash( -1 )
		{
		}

		ResourceId( EResourceType inType, String inName )
			:	type( inType ),
				hash( hashing::murmur32( *inName, inName.len() * sizeof( Char ) ) )
		{
		}

		EResourceType getType() const
		{
			return type;
		}

		UInt32 getHash() const
		{
			return hash;
		}

		Bool operator==( const ResourceId& other ) const
		{
			return data == other.data;
		}

		Bool operator!=( const ResourceId& other ) const
		{
			return data != other.data;
		}

		Bool operator>( const ResourceId& other ) const
		{
			return data > other.data;
		}

		Bool operator>=( const ResourceId& other ) const
		{
			return data >= other.data;
		}

		Bool operator<( const ResourceId& other ) const
		{
			return data < other.data;
		}

		Bool operator<=( const ResourceId& other ) const
		{
			return data <= other.data;
		}

		String toString() const
		{
			return String::format( L"%llu", data ); 
		}

		friend IOutputStream& operator<<( IOutputStream& stream, const ResourceId& x )
		{
			stream << x.data;
			return stream;
		}

		friend IInputStream& operator>>( IInputStream& stream, ResourceId& x )
		{
			stream >> x.data;
			return stream;
		}

		static constexpr ResourceId NONE()
		{
			return ResourceId( -1 );
		}

	private:
		union
		{
			struct
			{
				EResourceType type;
				UInt32 hash;			
			};

			UInt64 data;
		};

		constexpr ResourceId( UInt64 inData )
			:	data( inData )
		{
		}
	};

	static_assert( sizeof( ResourceId ) == sizeof( UInt64 ), 
		"ResourceId size should be equal to size of UInt64" );
}
}