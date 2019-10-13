//-----------------------------------------------------------------------------
//	Heap.h: Basic memory functions
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
namespace mem
{
	extern void* alloc( SizeT numBytes );
	extern void* malloc( SizeT numBytes );
	extern void* realloc( void* data, SizeT newNumBytes );
	extern void free( void* data );
	extern SizeT size( void* data );

	extern void zero( void* data, SizeT numBytes );
	extern void set( void* data, SizeT numBytes, UInt8 value );
	extern Bool cmp( const void* data1, const void* data2, SizeT numBytes );
	extern void copy( void* destData, const void* srcData, SizeT numBytes );
	extern void swap( void* data1, void* data2, SizeT numBytes );

	struct Stats
	{
		SizeT totalAllocatedBytes = 0;
		SizeT peakAllocatedBytes = 0;
		SizeT totalAllocations = 0;
	};

	extern const Stats& stats();

	extern void enterKnownMemLeaksZone();
	extern void leaveKnownMemLeaksZone();
	extern void dumpAllocations( const Char* fileName, Bool ignoreKnown = true );
}
}

// override standard operators
extern void* operator new( SizeT numBytes );
extern void* operator new[]( SizeT numBytes );
extern void operator delete( void* data );
extern void operator delete[]( void* data );