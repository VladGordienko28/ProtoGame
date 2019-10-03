//-----------------------------------------------------------------------------
//	Map.h: An associative array
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

// todo: implement proper hashing
namespace flu
{
	/**
	 *	A very useful and simple associative array
	 *	Really really simple, but fits my requirements yet
	 *	Based on binary search and insertion to sorted array
	 */
	template<typename K, typename V> class Map
	{
	public:
		/**
		 *	A pair of key and value
		 */
		struct Pair
		{
			K key;
			V value;

			friend IOutputStream& operator<<( IOutputStream& stream, const Map<K, V>::Pair& pair )
			{
				stream << pair.key << pair.value;
				return stream;
			}

			friend IInputStream& operator>>( IInputStream& stream, Map<K, V>::Pair& pair )
			{
				stream >> pair.key >> pair.value;
				return stream;
			}
		};

		Map()
			:	m_pairs()
		{
		}

		Map( const Map<K, V>& other )
			:	m_pairs( other.m_pairs )
		{
		}

		~Map()
		{
			empty();
		}

		/**
		 *	Returns current map size
		 */
		Int32 size() const
		{
			return m_pairs.size();
		}

		/**
		 *	Remove all items from the map
		 */
		void empty()
		{
			m_pairs.empty();
		}

		/**
		 *	Return true, if map contains specified key
		 */
		Bool hasKey( const K& key ) const
		{
			return findPairIndex( key ) != -1;
		}

		/**
		 *	Return true, if map contains specified value
		 */
		Bool hasValue( const V& value ) const
		{
			for( Int32 i = 0; i < m_pairs.size(); ++i )
			{
				if( m_pairs[i].value == value )
				{
					return true;
				}
			}

			return false;
		}

		V* get( const K& key )
		{
			Int32 index = findPairIndex( key );
			return index != -1 ? &m_pairs[index].value : nullptr;
		}

		const V* get( const K& key ) const
		{
			Int32 index = findPairIndex( key );
			return index != -1 ? &m_pairs[index].value : nullptr;
		}

		V& getRef( const K& key )
		{
			Int32 index = findPairIndex( key );
			assert( index != -1 );
			return m_pairs[index].value;
		}

		const V& getRef( const K& key ) const
		{
			Int32 index = findPairIndex( key );
			assert( index != -1 );
			return m_pairs[index].value;
		}

		Bool isEmpty() const
		{
			return m_pairs.size() == 0;
		}

		/**
		 *	Put a new pair to the dictionary
		 *	Return false if existing pair were overrided and
		 *	return true if this is a new pair
		 */
		Bool put( const K& key, const V& value )
		{
			Int32 i = findPairIndex( key );

			if( i != -1 )
			{
				m_pairs[i].value = value;
				return false;
			}
			else
			{
				insertSorted( key, value );
				return true;
			}
		}

		/**
		 *	Return list of all keys
		 */
		Array<K> keys() const
		{
			Array<K> keys( m_pairs.size() );

			for( Int32 i = 0; i < m_pairs.size(); ++i )
			{
				keys[i] = m_pairs[i].key;
			}

			return static_cast<Array<K>&&>( keys );
		}

		/**
		 *	Return list of all values
		 */
		Array<V> values() const
		{
			Array<V> vals( m_pairs.size() );

			for( Int32 i = 0; i < m_pairs.size(); ++i )
			{
				vals[i] = m_pairs[i].value;
			}

			return static_cast<Array<V>&&>( vals );
		}

		/**
		 *	Remove a pair with specified key. Return false if 
		 *	key is not found
		 */
		Bool remove( const K& key )
		{
			Int32 i = findPairIndex( key );

			if( i != -1 )
			{
				m_pairs.removeShift( i );
				return true;
			}
			else
			{
				return false;
			}
		}

		Bool operator==( const Map<K, V>& other ) const
		{
			return m_pairs == other.m_pairs;
		}

		Bool operator!=( const Map<K, V>& other ) const
		{
			return m_pairs != other.m_pairs;
		}

		Map<K, V>& operator=( const Map<K, V>& other )
		{
			m_pairs = other.m_pairs;
			return *this;
		}

		using Iterator = Pair*;
		using ConstIterator = const Pair*;

		Iterator begin()
		{
			return m_pairs.begin();
		}

		ConstIterator begin() const
		{
			return m_pairs.begin();
		}

		Iterator end()
		{
			return m_pairs.end();
		}

		ConstIterator end() const
		{
			return m_pairs.end();
		}

		friend IOutputStream& operator<<( IOutputStream& stream, const Map<K, V>& map )
		{
			stream << map.m_pairs;
			return stream;
		}

		friend IInputStream& operator>>( IInputStream& stream, Map<K, V>& map )
		{
			stream >> map.m_pairs;
			return stream;
		}

	private:
		Array<Pair> m_pairs;

		/**
		 *	Finds an index of the pair in the pairs array,
		 *	return -1 if key is not found
		 */
		Int32 findPairIndex( const K& key ) const
		{
			Int32 high, low;

			for( low = 0, high = m_pairs.size(); low < high;  )
			{
				Int32 middle = low + ( high - low ) / 2;

				if( key > m_pairs[middle].key )
				{
					low = middle + 1;
				}
				else
				{
					high = middle;
				}
			}

			return high < m_pairs.size() && m_pairs[high].key == key ?
				high : -1;
		}

		/**
		 *	Insert a new pair to the sorted array
		 */
		void insertSorted( const K& key, const V& value )
		{
			m_pairs.setSize( m_pairs.size() + 1 );
			Int32 i = m_pairs.size() - 1;

			while( ( i > 0 ) && ( key < m_pairs[i - 1].key ) )
			{
				m_pairs[i] = m_pairs[i - 1];
				--i;
			}

			m_pairs[i] = { key, value };
		}
	};
}