//-----------------------------------------------------------------------------
//	Hash.h: Hashing functions implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Core.h"

namespace flu
{
namespace hashing
{
	UInt32 murmur32( const void* data, SizeT size )
	{
		static_assert( sizeof( UInt32 ) == 4, "size of UInt32 should be 4" );
		const UInt8* ptr = reinterpret_cast<const UInt8*>( data );

		UInt32 m = 0x5bd1e995;
		UInt32 r = 24;
		UInt32 h = 0 ^ size;

		while( size >= 4 )
		{
			UInt32 k = *(UInt32*)ptr;

			k *= m;
			k ^= k >> r;
			k *= m;

			h *= m;
			h ^= k;

			ptr += 4;
			size -= 4;
		}

		switch( size )
		{
		case 3:	h ^= ptr[2] << 16;
		case 2:	h ^= ptr[1] << 8;
		case 1:
				h ^= ptr[0];
				h *= m;
		}

		h ^= h >> 13;
		h *= m;
		h ^= h >> 15;

		return h;
	}

	UInt64 murmur64( const void* data, SizeT size )
	{
		assert( false && "not implemented" );
		return 0;
	}
}
}