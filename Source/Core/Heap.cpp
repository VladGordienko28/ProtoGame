//-----------------------------------------------------------------------------
//	Heap.cpp: Basic memory functions
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Core.h"

#include <memory>

namespace flu
{
namespace mem
{
	static Stats g_stats;

	void* alloc( SizeT numBytes )
	{
#if FLU_PROFILE_MEMORY
		g_stats.totalAllocatedBytes += numBytes;
		g_stats.totalAllocations++;
#endif

		return ::calloc( 1, numBytes ); 
	}

	void* malloc( SizeT numBytes )
	{
#if FLU_PROFILE_MEMORY
		g_stats.totalAllocatedBytes += numBytes;
		g_stats.totalAllocations++;
#endif

		return ::malloc( numBytes );
	}

	void* realloc( void* data, SizeT newNumBytes )
	{
#if FLU_PROFILE_MEMORY
		g_stats.totalAllocatedBytes -= size( data );
		g_stats.totalAllocatedBytes += newNumBytes;
		g_stats.totalAllocations++;
#endif

		return ::realloc( data, newNumBytes );
	}

	void free( void* data )
	{
#if FLU_PROFILE_MEMORY
		g_stats.totalAllocatedBytes -= size( data );
#endif

		::free( data );
	}

	SizeT size( void* data )
	{
		return ::_msize( data );
	}

	void zero( void* data, SizeT numBytes )
	{
		::memset( data, 0, numBytes );
	}
	
	void set( void* data, SizeT numBytes, UInt8 value )
	{
		::memset( data, value, numBytes );
	}
	
	Bool cmp( const void* data1, const void* data2, SizeT numBytes )
	{
		return ::memcmp( data1, data2, numBytes ) == 0;
	}
	
	void copy( void* destData, const void* srcData, SizeT numBytes )
	{
		::memcpy( destData, srcData, numBytes );
	}
	
	void swap( void* data1, void* data2, SizeT numBytes )
	{
		static const SizeT TMP_BUFFER_SIZE = 4096;

		if( numBytes <= TMP_BUFFER_SIZE )
		{
			UInt8 tmpBuffer[TMP_BUFFER_SIZE];
			copy( tmpBuffer, data1, numBytes );
			copy( data1, data2, numBytes );
			copy( data2, tmpBuffer, numBytes );
		}
		else
		{
			void* tmpBuffer = malloc( numBytes );
			copy( tmpBuffer, data1, numBytes );
			copy( data1, data2, numBytes );
			copy( data2, tmpBuffer, numBytes );
			free( tmpBuffer );
		}
	}

	const Stats& stats()
	{
		return g_stats;
	}
}
}

void* operator new( SizeT numBytes )
{
	return flu::mem::malloc( numBytes );
}

void* operator new[]( SizeT numBytes )
{
	return flu::mem::malloc( numBytes );
}

void operator delete( void* data )
{
	flu::mem::free( data );
}

void operator delete[]( void* data )
{
	flu::mem::free( data );
}