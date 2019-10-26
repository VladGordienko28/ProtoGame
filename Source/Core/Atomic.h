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
	public:
		Atomic()
			:	m_value( 0 )
		{
		}

		Atomic( Int32 value )
			:	m_value( value )
		{
		}

		Int32 increment();
		Int32 decrement();
		Int32 add( Int32 amount );
		Int32 subtract( Int32 amount );
		Int32 setValue( Int32 newValue );
		Int32 getValue() const;

	private:
		volatile Int32 m_value;
	};

} // namespace concurrency
} // namespace flu