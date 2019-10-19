//-----------------------------------------------------------------------------
//	Heap.cpp: Basic memory functions
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Core.h"

#include <memory>

#if FLU_ENABLE_MEM_TRACKING
#pragma pack( push, 8 )
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment( lib, "dbghelp.lib" )
#pragma pack( pop ) 
#endif

namespace flu
{
namespace mem
{
#if FLU_ENABLE_MEM_TRACKING
	/**
	 *	A memory tracker, which tracks all allocations and
	 *	their callstack
	 */
	class MemoryTracker
	{
	public:
		MemoryTracker()
			:	m_allocationList( nullptr ),
				m_knownLeaksZoneCounter( 0 )
		{
			m_kernel32Module = LoadLibrary( TXT("kernel32.dll") );
			assert( m_kernel32Module != NULL && "Failed to load kernel32 module" );

			m_captureStackFunc = reinterpret_cast<CaptureStackBackTraceType>( 
				GetProcAddress(m_kernel32Module, "RtlCaptureStackBackTrace" ) );

			assert( m_captureStackFunc && "Function \"RtlCaptureStackBackTrace\" is not found" );
		}

		~MemoryTracker()
		{
			dumpAllocations();

			while( m_allocationList )
			{
				Allocation* next = m_allocationList->next;

				::free( m_allocationList );
				m_allocationList = next;
			}

			FreeLibrary( m_kernel32Module );
		}

		void trackAllocation( void* address, SizeT size )
		{
			assert( address && size );

			Allocation* newAllocation = reinterpret_cast<Allocation*>( ::malloc( sizeof( Allocation ) ) );
			assert( newAllocation );

			UInt16 numFrames = m_captureStackFunc( NUM_FRAMES_TO_SKIP, MAX_CALLSTACK_DEPTH, 
				newAllocation->callstack, NULL );

			newAllocation->numFrames = numFrames;
			newAllocation->address = address;
			newAllocation->size = size;
			newAllocation->isKnownLeak = m_knownLeaksZoneCounter > 0;
			newAllocation->next = m_allocationList;

			m_allocationList = newAllocation;
		}

		void untrackAllocation( void* address )
		{
			Allocation** allocationPtr = &m_allocationList;

			while( *allocationPtr )
			{
				if( (*allocationPtr)->address == address )
				{
					Allocation* temp = *allocationPtr;
					*allocationPtr = (*allocationPtr)->next;	

					::free( temp );
					return;
				}
				else
				{
					allocationPtr = &( (*allocationPtr)->next );
				}
			}

			assert( false && "Attempt to untrack unknwon allocation" );
		}

		void dumpAllocations( const Char* fileName = DEFAULT_DUMP_FILE_NAME, Bool ignoreKnown = true )
		{
			FILE* file;
			auto errorCode = _wfopen_s( &file, fileName, L"w" );

			if( file )
			{
				static const SizeT MAX_SYMBOL_LENGTH = 256;

				HANDLE hProcess = GetCurrentProcess();
				SymInitialize( hProcess, NULL, TRUE );

				UInt8 symbolBuffer[sizeof( SYMBOL_INFO ) + MAX_SYMBOL_LENGTH] = {};

				SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>( symbolBuffer );
				symbol->SizeOfStruct = sizeof( SYMBOL_INFO );
				symbol->MaxNameLen = MAX_SYMBOL_LENGTH - 1;

				SizeT numAllocations = 0;
				SizeT allocationsSize = 0;

				for( Allocation* it = m_allocationList; it; it = it->next )
				{
					if( it->isKnownLeak && ignoreKnown )
					{
						continue;
					}

					fwprintf( file, L"=====\n" );
					fwprintf( file, L"0x%p : %d bytes\n", it->address, it->size );

					for( UInt32 i = 0; i < it->numFrames; ++i )
					{
						SymFromAddr( hProcess, reinterpret_cast<DWORD64>( it->callstack[i] ), 0, symbol );

						fwprintf( file, L"    0x%p: %hs ( 0x%p )\n", it->callstack[i], symbol->Name, (void*)symbol->Address );
					}

					fwprintf( file, L"\n" );

					++numAllocations;
					allocationsSize += it->size;
				}

				fwprintf( file, L"\n==========\n" );
				fwprintf( file, L"Total Allocations: %d\n", numAllocations );
				fwprintf( file, L"Total Size: %d bytes\n", allocationsSize );

				fclose( file );
			}
			else
			{
				fatal( L"Unable to write memory dump file \"%s\" with error %d", fileName, errorCode );
			}
		}

		void enterKnownMemLeaksZone()
		{
			++m_knownLeaksZoneCounter;
		}

		void leaveKnownMemLeaksZone()
		{
			--m_knownLeaksZoneCounter;
			assert( m_knownLeaksZoneCounter >= 0 );
		}

	private:
		static const constexpr Char DEFAULT_DUMP_FILE_NAME[] = TXT("MemoryDump.txt");
		static const SizeT MAX_CALLSTACK_DEPTH = 8;
		static const UInt32 NUM_FRAMES_TO_SKIP = 1;

		typedef USHORT ( WINAPI *CaptureStackBackTraceType )( __in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG );

		struct Allocation
		{
		public:
			Allocation* next;
			void* callstack[MAX_CALLSTACK_DEPTH];
			SizeT numFrames;
			void* address;
			SizeT size;
			Bool isKnownLeak;
		};

		Allocation* m_allocationList;
		Int32 m_knownLeaksZoneCounter;

		CaptureStackBackTraceType m_captureStackFunc;
		HMODULE m_kernel32Module;
	};

	static MemoryTracker g_memoryTracker;
#endif

	static Stats g_stats;

	void* alloc( SizeT numBytes )
	{
#if FLU_PROFILE_MEMORY
		g_stats.totalAllocatedBytes += numBytes;
		g_stats.totalAllocations++;
		g_stats.peakAllocatedBytes = max( g_stats.peakAllocatedBytes, g_stats.totalAllocatedBytes );
#endif

		void* address = ::calloc( 1, numBytes );

#if FLU_ENABLE_MEM_TRACKING
		g_memoryTracker.trackAllocation( address, numBytes );
#endif

		return address; 
	}

	void* malloc( SizeT numBytes )
	{
#if FLU_PROFILE_MEMORY
		g_stats.totalAllocatedBytes += numBytes;
		g_stats.totalAllocations++;
		g_stats.peakAllocatedBytes = max( g_stats.peakAllocatedBytes, g_stats.totalAllocatedBytes );
#endif

		void* address = ::malloc( numBytes );

#if FLU_ENABLE_MEM_TRACKING
		g_memoryTracker.trackAllocation( address, numBytes );
#endif

		return address;
	}

	void* realloc( void* data, SizeT newNumBytes )
	{
#if FLU_PROFILE_MEMORY
		g_stats.totalAllocatedBytes -= size( data );
		g_stats.totalAllocatedBytes += newNumBytes;
		g_stats.totalAllocations++;
		g_stats.peakAllocatedBytes = max( g_stats.peakAllocatedBytes, g_stats.totalAllocatedBytes );
#endif

#if FLU_ENABLE_MEM_TRACKING
		g_memoryTracker.untrackAllocation( data );
#endif

		void* newAddress = ::realloc( data, newNumBytes );

#if FLU_ENABLE_MEM_TRACKING
		g_memoryTracker.trackAllocation( newAddress, newNumBytes );
#endif

		return newAddress;
	}

	void free( void* data )
	{
#if FLU_PROFILE_MEMORY
		g_stats.totalAllocatedBytes -= size( data );
#endif

#if FLU_ENABLE_MEM_TRACKING
		g_memoryTracker.untrackAllocation( data );
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

	void enterKnownMemLeaksZone()
	{
#if FLU_ENABLE_MEM_TRACKING
		g_memoryTracker.enterKnownMemLeaksZone();
#endif
	}

	void leaveKnownMemLeaksZone()
	{
#if FLU_ENABLE_MEM_TRACKING
		g_memoryTracker.leaveKnownMemLeaksZone();
#endif
	}

	void dumpAllocations( const Char* fileName, Bool ignoreKnown )
	{
#if FLU_ENABLE_MEM_TRACKING
		g_memoryTracker.dumpAllocations( fileName, ignoreKnown );
#endif
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