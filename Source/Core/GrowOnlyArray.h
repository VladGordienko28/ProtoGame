//-----------------------------------------------------------------------------
//	GrowOnlyArray.h: An growing size array
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	A very useful and grow only array
	 *	template
	 */
	template<typename T> class GrowOnlyArray
	{
	public:
		GrowOnlyArray()
			:	m_array(),
				m_currentSize( 0 )
		{
		}

		GrowOnlyArray( Int32 initialSize )
			:	m_array( initialSize ),
				m_currentSize( 0 )
		{
		}

		GrowOnlyArray( const GrowOnlyArray<T>& other )
			:	m_array( other.m_array ),
				m_currentSize( other.m_currentSize )
		{
		}

		~GrowOnlyArray()
		{
			reset();
		}

		/**
		 *	Returns current array size
		 */
		Int32 size() const
		{
			return m_currentSize;
		}

		/**
		 *	Returns current array capacity
		 */
		Int32 capacity() const
		{
			return m_array.size();
		}

		/**
		 *	Remove all items from the array
		 */
		void empty()
		{
			// todo: add support for non POD-types
			m_currentSize = 0;
		}

		/**
		 *	Actualy cleanup array
		 */
		void reset()
		{
			m_array.empty();
			m_currentSize = 0;
		}

		/**
		 *	Obtains some items
		 */
		T* obtainRaw( Int32 count )
		{
			assert( count > 0 );

			if( m_array.size() < m_currentSize + count )
			{
				m_array.setSize( m_currentSize + count * GROW_AMORTIZATION );
			}

			T* result = &m_array[m_currentSize];
			m_currentSize += count;

			return result;
		}

		/**
		 *	Push a new item to the end of the array and
		 *	return index of new item
		 */
		Int32 push( const T& newItem )
		{
			*obtainRaw( 1 ) = newItem;
			return m_currentSize - 1;
		}

		using Iterator = typename Array<T>::Iterator;
		using ConstIterator = typename Array<T>::ConstIterator;

		Iterator begin()
		{
			return m_array.begin();
		}

		ConstIterator begin() const
		{
			return m_array.begin();
		}

		Iterator end()
		{
			return m_array.begin() + m_currentSize;
		}

		ConstIterator end() const
		{
			return m_array.begin() + m_currentSize;
		}

		T& operator[]( Int32 i )
		{
			assert( i >= 0 && i < m_currentSize );
			return m_array[i];
		}

		const T& operator[]( Int32 i ) const
		{
			assert( i >= 0 && i < m_currentSize );
			return m_array[i];
		}

	private:
		static const Int32 GROW_AMORTIZATION = 4;

		Array<T> m_array;
		Int32 m_currentSize;
	};

} // namespace flu