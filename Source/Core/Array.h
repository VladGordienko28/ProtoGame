//-----------------------------------------------------------------------------
//	Array.h: Array templates
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	A very useful and simple dynamic array
	 *	template
	 */
	template<typename T> class Array
	{
	public:
		Array()
			:	m_data( nullptr ),
				m_size( 0 )
		{
		}

		Array( Int32 initialSize )
			:	m_data( nullptr ),
				m_size( 0 )
		{
			setSize( initialSize );
		}

		Array( const Array<T>& other )
			:	m_data( nullptr ),
				m_size( 0 )
		{
			setSize( other.m_size );

			for( Int32 i = 0; i < other.m_size; ++i )
			{
				m_data[i] = other.m_data[i];
			}
		}

		Array( Array<T>&& other )
			:	m_data( other.m_data ),
				m_size( other.m_size )
		{
			other.m_data = nullptr;
			other.m_size = 0;
		}

		template<Int32 S> Array( const T(&initialData)[S] )
			:	m_data( nullptr ),
				m_size( 0 )
		{
			setSize( S );

			for( Int32 i = 0; i < S; ++i )
				m_data[i] = initialData[i];
		}

		~Array()
		{
			empty();
		}

		/**
		 *	Returns current array size
		 */
		Int32 size() const
		{
			return m_size;
		}

		/**
		 *	Remove all items from the array
		 */
		void empty()
		{
			setSize( 0 );
		}

		/**
		 *	Get an array item using inverted index
		 */
		T& last( Int32 invIndex = 0 )
		{
			assert( invIndex >= 0 && invIndex < m_size );
			return m_data[m_size - invIndex - 1];
		}

		const T& last( Int32 invIndex = 0 ) const
		{
			assert( invIndex >= 0 && invIndex < m_size );
			return m_data[m_size - invIndex - 1];
		}

		/**
		 *	Return the last item from array and remove it
		 */
		T pop()
		{
			assert( m_size > 0 );
			T tmp = m_data[m_size - 1];

			setSize( m_size - 1 );
			return tmp;
		}

		/**
		 *	Push a new item to the end of the array and
		 *	return index of new item
		 */
		Int32 push( const T& newItem )
		{
			setSize( m_size + 1 );
			m_data[m_size - 1] = newItem;
			return m_size - 1;
		}

		/**
		 *	Return the first item from array and remove it
		 */
		inline T shift()
		{
			assert( m_size > 0 );
			T result = m_data[0];

			for( Int32 i = 0; i < m_size - 1; ++i )
			{
				m_data[i] = m_data[i+1];
			}

			setSize( m_size - 1 );
			return result;
		}

		/**
		 *	Unshift a new item to the begging of the array and
		 *	return array new size
		 */
		inline Int32 unshift( const T& item )
		{
			setSize( m_size + 1 );

			for( Int32 i = m_size - 1; i > 0; --i )
			{
				m_data[i] = m_data[i-1];
			}

			m_data[0] = item;
			return m_size;
		}

		/**
		 *	Add an unique item to the array
		 */
		Int32 addUnique( const T& newItem )
		{
			Int32 i = find( newItem );

			if( i == -1 )
			{
				i = push( newItem );
			}

			return i;
		}

		/**
		 *	Find an item in the array, awful O(n)
		 */
		Int32 find( const T& item ) const
		{
			for( Int32 i = 0; i < m_size; ++i )
			{
				if( m_data[i] == item )
					return i;
			}

			return -1;
		}

		/**
		 *	Remove all items which is equal to inItem from
		 *	the array
		 */
		void removeUnique( const T& inItem, Bool keepOrder = false )
		{
			for( Int32 i = 0; i < m_size;  )
			{
				if( inItem == m_data[i] )
				{
					if( keepOrder )
						removeShift( i );
					else
						removeFast( i );
				}
				else
				{
					++i;
				}
			}
		}

		/**
		 *	Remove item from the array fast, but
		 *	shuffle items
		 */
		void removeFast( Int32 idx )
		{
			assert( idx >= 0 && idx < m_size );
			assert( m_size > 0 );

			m_data[idx] = m_data[m_size - 1];
			setSize( m_size - 1 );
		}

		/**
		 *	Remove an item from the array slowly, with shifting
		 *	all followed item
		 */
		void removeShift( Int32 idx )
		{
			assert( idx >= 0 && idx < m_size );
			assert( m_size > 0 );

			for( Int32 i = idx; i < m_size - 1; ++i )
			{
				m_data[i] = m_data[i + 1];
			}

			setSize( m_size - 1 );
		}

		/**
		 *	Swap an array's items using their indexes
		 */
		void swap( Int32 a, Int32 b )
		{
			assert( a >= 0 && a < m_size );
			assert( b >= 0 && b < m_size );

			T tmp = m_data[a];
			m_data[a] = m_data[b];
			m_data[b] = tmp;
		}

		/**
		 *	Insert an item/s to the array starting from index
		 */
		void insert( Int32 index, Int32 count )
		{
			assert( m_size >= 0 );
			assert( count >= 0 );
			assert( index >= 0 && index <= m_size );

			Int32 oldSize = m_size;
			setSize( m_size + count );

			for( Int32 i = m_size-1; i >= index + count; i-- )
				m_data[i] = m_data[i - count];
		}

		/**
		 *	Quick array sort
		 */
		void sort( Bool(*sortFunc)( const T&, const T& ) )
		{
			if( m_size > 1 )
			{
				qSort( 0, m_size - 1, sortFunc );
			}
		}

		using Iterator = T*;
		using ConstIterator = const T*;

		Iterator begin()
		{
			return m_data;
		}

		ConstIterator begin() const
		{
			return m_data;
		}

		Iterator end()
		{
			return m_data + m_size;
		}

		ConstIterator end() const
		{
			return m_data + m_size;
		}

		Int32 iteratorToIndex( Iterator it ) const
		{
			return ( it >= begin() && it < end() ) ? 
				(Int32)( ((UInt8*)it - (UInt8*)m_data) / sizeof(T) ) : -1;
		}

		Iterator indexToIterator( Int32 i ) const
		{
			return ( i >= 0 && i < m_size ) ? &m_data[i] : end();
		}

		T& operator[]( Int32 i )
		{
			assert( i >= 0 && i < m_size );
			return m_data[i];
		}

		const T& operator[]( Int32 i ) const
		{
			assert( i >= 0 && i < m_size );
			return m_data[i];
		}

		Bool operator==( const Array<T>& other ) const
		{
			if( m_size != other.m_size )
			{
				return false;
			}

			for( Int32 i = 0; i < m_size; ++i )
			{
				if( m_data[i] != other.m_data[i] )
					return false;
			}

			return true;
		}

		Bool operator!=( const Array<T>& other ) const
		{
			if( m_size != other.m_size )
			{
				return true;
			}

			for( Int32 i = 0; i < m_size; ++i )
			{
				if( m_data[i] != other.m_data[i] )
					return true;
			}

			return false;
		}

		Array<T>& operator=( const Array<T>& other )
		{
			setSize( other.size() );

			for( Int32 i = 0; i < m_size; ++i )
			{
				m_data[i] = other.m_data[i];
			}

			return *this;
		}

		void setSize( Int32 newSize )
		{
			assert( newSize >= 0 );

			if( newSize != m_size )
			{
				for( Int32 i = newSize; i < m_size; ++i )
				{
					(&m_data[i])->~T();
				}

				array::reallocate( *((void**)&m_data), m_size, newSize, sizeof(T) );
			}
		}

		// Legacy!
		// to be eliminated
		friend void Serialize( class CSerializer& s, Array<T>& v )
		{
			if( s.GetMode() == SM_Load )
			{
				Int32 arrSize;
				Serialize( s, arrSize );
				v.empty();
				v.setSize( arrSize );
			}
			else
			{
				Int32 arrSize = v.size();
				Serialize( s, arrSize );
			}
			for( Int32 i = 0; i < v.size(); i++ )
				Serialize( s, v[i] );
		}

	private:
		T* m_data;
		Int32 m_size;

		void qSort( Int32 iMin, Int32 iMax, Bool(*sortFunc)( const T&, const T& ) )
		{
			Int32 i = iMin, j = iMax;
			T middle = m_data[ iMax - ( iMax - iMin ) / 2 ];
			while( i < j )
			{
				while( sortFunc( m_data[i], middle ) ) i++;
				while( sortFunc( middle, m_data[j] ) ) j--;
				if( i <= j )
				{
					swap( i, j );
					i++;
					j--;
				}
			}
			if( iMin < j ) qSort( iMin, j, sortFunc );
			if( i < iMax ) qSort( i, iMax, sortFunc );
		}

#if 0
		static_assert( sizeof(Array<Int32>::m_data) == sizeof(ArrayPOD::data), "sizeof(Array<Int32>::m_data) == sizeof(ArrayPOD::data)" );
		static_assert( sizeof(Array<Int32>::m_size) == sizeof(ArrayPOD::size), "sizeof(Array<Int32>::m_size) == sizeof(ArrayPOD::size)" );
		static_assert( sizeof(Array<Int32>) == sizeof(ArrayPOD), "sizeof(Array<Int32>) == sizeof(ArrayPOD)" );
		static_assert( offsetof(Array<Int32>, m_data) == offsetof(ArrayPOD, data), "offsetof(Array<Int32>, m_data) == offsetof(ArrayPOD, data)" );
		static_assert( offsetof(Array<Int32>, m_size) == offsetof(ArrayPOD, size), "offsetof(Array<Int32>, m_size) == offsetof(ArrayPOD, size)" );
#endif
	};

	/**
	 *	A POD mirror of real Array<T> template,
	 *	which used by FluScript for reflection
	 */
	struct ArrayPOD
	{
		void* data;
		Int32 size;
	};

	namespace array
	{
		/**
		 *	Complicated reallocation function :)
		 */
		extern void reallocate( void*& data, Int32& oldSize, Int32 newSize, SizeT elementSize );
	}

} // namespace flu