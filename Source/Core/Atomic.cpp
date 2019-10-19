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
	/**
	 *	A WinApi based atomic integer value
	 */
	class WinAtomic: public Atomic
	{
	public:
		WinAtomic()
			:	m_value( 0 )
		{
		}

		WinAtomic( Int32 value )
			:	m_value( value )
		{
		}

		Int32 increment()
		{
			return (Int32)InterlockedIncrement( (LPLONG)&m_value );
		}

		Int32 decrement()
		{
			return (Int32)InterlockedDecrement( (LPLONG)&m_value );
		}

		Int32 add( Int32 amount )
		{
			return (Int32)InterlockedAdd( (LPLONG)&m_value, amount );
		}

		Int32 subtract( Int32 amount )
		{
			return (Int32)InterlockedAdd( (LPLONG)&m_value, -amount );
		}

		Int32 setValue( Int32 newValue )
		{
			return (Int32)InterlockedExchange( (LPLONG)&m_value, newValue );
		}

		Int32 getValue() const
		{
			return m_value;
		}

	private:
		volatile Int32 m_value;
	};

	Atomic* Atomic::create()
	{
		return new WinAtomic();
	}

	Atomic* Atomic::create( Int32 value )
	{
		return new WinAtomic( value );
	}
#else
#error Atomic is not implemented for current platform
#endif
} // namespace concurrency
} // namespace flu