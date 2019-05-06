//-----------------------------------------------------------------------------
//	Atomic.h: A thread-safe counter
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
namespace concurrency
{
	/**
	 *	An atomic integer value
	 */
	class Atomic
	{
#if FLU_PLATFORM_WINDOWS
	public:
		Atomic()
			:	m_value( 0 )
		{
		}

		Atomic( Int32 value )
			:	m_value( value )
		{
		}

		// is not really thread-safe
		Atomic( const Atomic& other )
			:	m_value( other.m_value )
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
#else
#error Atomic is not implemented for current platform
#endif
	};

} // namespace concurrency
} // namespace flu