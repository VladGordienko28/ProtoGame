//-----------------------------------------------------------------------------
//	Hash.h: Various hashing functions
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace hashing
{
	extern UInt32 murmur32( const void* data, SizeT size );
	extern UInt64 murmur64( const void* data, SizeT size );
}
}