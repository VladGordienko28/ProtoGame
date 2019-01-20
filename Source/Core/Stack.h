//-----------------------------------------------------------------------------
//	Stack.h: A stack templates
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	A very simple fixed-sized stack
	 */
	template<typename T> class FixedStack
	{
	public:
		FixedStack( Int32 maxSize )
			:	m_data( nullptr ),
				m_maxSize( maxSize ),
				m_currentSize( 0 )
		{
			assert( maxSize > 0 );
			m_data = new T[maxSize]();
		}

		~FixedStack()
		{
			empty();
			delete[] m_data;
		}

		void empty()
		{
			for( Int32 i = 0; i < m_maxSize; ++i )
			{
				(&m_data[i])->~T();
			}
		}

		T pop()
		{
			assert( m_currentSize > 0 && "Stack underflowed" );
			return m_data[--m_currentSize];
		}

		void push( T& item )
		{
			assert( m_currentSize < m_maxSize && "Stack overflowed" );
			m_data[m_currentSize++] = item; 
		}

		Bool isEmpty() const
		{
			return m_currentSize == 0;
		}

	private:
		T* m_data;
		Int32 m_currentSize;
		Int32 m_maxSize;

		FixedStack() = delete;
		FixedStack( FixedStack& ) = delete;
		FixedStack<T> operator=( FixedStack& ) = delete;
	};

} // namespace flu