//-----------------------------------------------------------------------------
//	Atomic.cpp: A thread-safe counter
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Core.h"

#if FLU_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace flu
{
namespace concurrency
{
#if FLU_PLATFORM_WINDOWS

	Int32 Atomic::increment()
	{
		return (Int32)InterlockedIncrement( (LPLONG)&m_value );
	}

	Int32 Atomic::decrement()
	{
		return (Int32)InterlockedDecrement( (LPLONG)&m_value );
	}

	Int32 Atomic::add( Int32 amount )
	{
		return (Int32)InterlockedAdd( (LPLONG)&m_value, amount );
	}

	Int32 Atomic::subtract( Int32 amount )
	{
		return (Int32)InterlockedAdd( (LPLONG)&m_value, -amount );
	}

	Int32 Atomic::setValue( Int32 newValue )
	{
		return (Int32)InterlockedExchange( (LPLONG)&m_value, newValue );
	}

	Int32 Atomic::getValue() const
	{
		return m_value;
	}

#else
#error Atomic is not implemented for current platform
#endif
} // namespace concurrency
} // namespace flu