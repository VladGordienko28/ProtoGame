//-----------------------------------------------------------------------------
//	Queue.h: A queue templates
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	A simple ring queue. Super fast without
	 *	memory allocations
	 */
	template<typename T, Int32 SIZE> class RingQueue
	{
	public:
		RingQueue()
			:	m_tailIndex( 0 ),
				m_currentSize( 0 )
		{
		}

		~RingQueue()
		{
			empty();
		}

		Int32 currentSize() const
		{
			return m_currentSize;
		}

		Bool isEmpty() const
		{
			return m_currentSize == 0;
		}

		void empty()
		{
			m_tailIndex = 0;
			m_currentSize = 0;
		}

		void enqueue( const T& item )
		{
			++m_currentSize;
			Int32 headIndex = SIZE_MASK & ( m_tailIndex + m_currentSize );

			assert( headIndex != m_tailIndex && "RingQueue overflowed" );
			m_buffer[headIndex] = item;
		}

		T dequeue()
		{
			assert( m_currentSize > 0 && "RingQueue underflowed" );
			T temp = m_buffer[m_tailIndex];

			--m_currentSize;
			m_tailIndex = SIZE_MASK & ( m_tailIndex + 1 );

			return temp;
		}

		Bool operator==( const RingQueue<T, SIZE>& other ) const
		{
			if( m_currentSize == other.m_currentSize )
			{
				for( Int32 i = 0; i < m_currentSize; ++i )
				{
					if( m_buffer[SIZE_MASK & (i + m_tailIndex)] != other.m_buffer[SIZE_MASK & (i + other.m_tailIndex)] )
					{
						return false;
					}
				}

				return true;
			}
			else
			{
				return false;
			}
		}

		Bool operator!=( const RingQueue<T, SIZE>& other ) const
		{
			return !operator==( other );
		}

	private:
		static_assert( isPowerOfTwo( SIZE ), "RingQueue size should be power of two" );
		static const Int32 SIZE_MASK = SIZE - 1;

		T m_buffer[SIZE];
		Int32 m_tailIndex;
		Int32 m_currentSize;

		RingQueue( RingQueue& ) = delete;
		RingQueue<T, SIZE> operator=( RingQueue& ) = delete;
	};
}