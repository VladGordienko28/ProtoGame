//-----------------------------------------------------------------------------
//	Time.cpp: Time relative functions
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Core.h"

#if FLU_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace flu
{
namespace time
{
#if FLU_PLATFORM_WINDOWS
	struct TimeInitializer
	{
	public:
		Double secPerCycle;
		Double msPerCycle;

		TimeInitializer()
		{
			LARGE_INTEGER largeInt;
			if( !QueryPerformanceFrequency( &largeInt ) )
			{
				fatal( L"'QueryPerformanceFrequency' failed with error %d", 
					GetLastError() );
			}

			secPerCycle = 1.0 / static_cast<Double>( largeInt.QuadPart );
			msPerCycle = 1000.0 / static_cast<Double>( largeInt.QuadPart );
		}
	};

	static TimeInitializer g_time;
#endif


	UInt32 cycles()
	{
#if FLU_PLATFORM_WINDOWS
	LARGE_INTEGER largeInt;
	QueryPerformanceCounter( &largeInt );
	return largeInt.LowPart;
#else
#error time::cycles is not implemented for current platform
#endif
	}

	UInt64 cycles64()
	{
#if FLU_PLATFORM_WINDOWS
	LARGE_INTEGER largeInt;
	QueryPerformanceCounter( &largeInt );
	return largeInt.QuadPart;
#else
#error time::cycles64 is not implemented for current platform
#endif
	}

	Double cyclesToSec( UInt64 c )
	{
#if FLU_PLATFORM_WINDOWS
		return c * g_time.secPerCycle;
#else
#error time::cyclesToSec is not implemented for current platform
#endif
	}

	Double cyclesToMs( UInt64 c )
	{
#if FLU_PLATFORM_WINDOWS
		return c * g_time.msPerCycle;
#else
#error time::cyclesToMs is not implemented for current platform
#endif
	}

	Double elapsedSecFrom( UInt64 c )
	{
#if FLU_PLATFORM_WINDOWS
	LARGE_INTEGER largeInt;
	QueryPerformanceCounter( &largeInt );	
	return g_time.secPerCycle * ( largeInt.QuadPart - c );

#else
#error time::elapsedSecFrom is not implemented for current platform
#endif
	}

	Double elapsedMsFrom( UInt64 c )
	{
#if FLU_PLATFORM_WINDOWS
	LARGE_INTEGER largeInt;
	QueryPerformanceCounter( &largeInt );	
	return g_time.msPerCycle * ( largeInt.QuadPart - c );
#else
#error time::elapsedMsFrom is not implemented for current platform
#endif
	}
}
}