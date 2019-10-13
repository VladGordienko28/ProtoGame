//-----------------------------------------------------------------------------
//	Stack.h: A stack templates
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	A very simple fixed-sized stack
	 */
	template<typename T, SizeT SIZE> class FixedStack
	{
	public:
		FixedStack()
			:	m_currentSize( 0 )
		{
		}

		~FixedStack()
		{
			empty();
		}

		void empty()
		{
			m_currentSize = 0;
		}

		T pop()
		{
			assert( m_currentSize > 0 && "Stack underflowed" );
			return m_data[--m_currentSize];
		}

		void push( const T& item )
		{
			assert( m_currentSize < SIZE && "Stack overflowed" );
			m_data[m_currentSize++] = item; 
		}

		const T& peek() const
		{
			assert( m_currentSize > 0 && "Cannot peek empty stack" );
			return m_data[m_currentSize - 1];
		}

		Bool isEmpty() const
		{
			return m_currentSize == 0;
		}

		Bool isInStack( const T& item ) const
		{
			for( Int32 i = 0; i < m_currentSize; ++i )
			{
				if( m_data[i] == item )
				{
					return true;
				}
			}

			return false;
		}

	private:
		T m_data[SIZE];
		Int32 m_currentSize;

		FixedStack( FixedStack& ) = delete;
		FixedStack<T, SIZE> operator=( FixedStack& ) = delete;

		static_assert( SIZE > 0, "FixedArray size should be greater than 0" );
	};

} // namespace flu