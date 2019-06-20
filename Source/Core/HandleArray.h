//-----------------------------------------------------------------------------
//	HandleArray.h: A simple array which is using handles instead of indexes
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	An abstract handle of the item in the HandleArray
	 */
	template<SizeT ID_BITS, SizeT GEN_BITS, UInt32 DESCR> class Handle
	{
	public:
		static const SizeT BITS_PER_ID = ID_BITS;
		static const SizeT BITS_PER_GEN = GEN_BITS;

		Handle()
			:	id( ( 1 << ID_BITS ) - 1 ),
				gen( ( 1 << GEN_BITS ) - 1 )
		{
		}

		Bool operator==( Handle<ID_BITS, GEN_BITS, DESCR> other ) const
		{
			return id == other.id && gen == other.gen;
		}

		Bool operator!=( Handle<ID_BITS, GEN_BITS, DESCR> other ) const
		{
			return id != other.id || gen != other.gen;
		}

	private:
		UInt32 id : ID_BITS;
		UInt32 gen : GEN_BITS;	

		Handle( UInt32 inId, UInt32 inGen )
			:	id( inId ),
				gen( inGen )
		{
		}

		template<typename HANDLE_TYPE, typename T, SizeT MAX_SIZE> friend class HandleArray;
		template<typename HANDLE> friend HANDLE INVALID_HANDLE();

		static_assert( ID_BITS + GEN_BITS == 32, "Total Handle bits should be 32" );
	};

	// Invalid handle constructor
	template<typename HANDLE> inline HANDLE INVALID_HANDLE()
	{
		return HANDLE( ( 1 << HANDLE::BITS_PER_ID ) - 1, ( 1 << HANDLE::BITS_PER_GEN ) - 1 );
	}

	/**
	 *	An abstract non-resizable container, which uses immutable handles for
	 *	elements access
	 */
	template<typename HANDLE_TYPE, typename T, SizeT MAX_SIZE> class HandleArray
	{
	public:
		HandleArray()
		{
			for( UInt32 i = 0; i < MAX_SIZE; ++i )
			{
				m_available[i].id = i + 1;
				m_available[i].gen = 0;
			}

			m_firstAvailable = 0;
			m_available[MAX_SIZE - 1].id = MAX_SIZE;
		}

		~HandleArray()
		{
		}

		HANDLE_TYPE addElement( const T& element )
		{
			assert( m_firstAvailable != MAX_SIZE && "HandleArray overflowed" );

			m_data[m_firstAvailable] = element;

			HANDLE_TYPE handle = HANDLE_TYPE( m_firstAvailable, ++m_available[m_firstAvailable].gen );

			m_firstAvailable = m_available[m_firstAvailable].id;
			m_available[handle.id].id = MAX_SIZE;

			return handle;
		}

		T* emplaceElement( HANDLE_TYPE& outHandle )
		{
			assert( m_firstAvailable != MAX_SIZE && "HandleArray overflowed" );

			T* result = &m_data[m_firstAvailable];

			HANDLE_TYPE handle = HANDLE_TYPE( m_firstAvailable, ++m_available[m_firstAvailable].gen );

			m_firstAvailable = m_available[m_firstAvailable].id;
			m_available[handle.id].id = MAX_SIZE;

			outHandle = handle;
			return result;
		}

		void removeElement( HANDLE_TYPE handle )
		{
			assert( handle.id >= 0 && handle.id < MAX_SIZE );
			assert( handle.gen = m_available[handle.id].gen );
			assert( m_available[handle.id].id == MAX_SIZE );

			m_available[handle.id].id = m_firstAvailable;
			m_firstAvailable = handle.id;

			m_data[handle.id].~T();
		}

		T& get( HANDLE_TYPE handle )
		{
			assert( handle.id >= 0 && handle.id < MAX_SIZE );
			assert( handle.gen = m_available[handle.id].gen );
			assert( m_available[handle.id].id == MAX_SIZE );
			
			return m_data[handle.id];
		}

		const T& get( HANDLE_TYPE handle ) const
		{
			assert( handle.id >= 0 && handle.id < MAX_SIZE );
			assert( handle.gen = m_available[handle.id].gen );
			assert( m_available[handle.id].id == MAX_SIZE );
			
			return m_data[handle.id];
		}

	private:
		T m_data[MAX_SIZE];
		HANDLE_TYPE m_available[MAX_SIZE];
		UInt32 m_firstAvailable;

		static_assert( isPowerOfTwo( MAX_SIZE ), "MaxSize of HandleArray should be power of two" );
		static_assert( sizeof( HANDLE_TYPE ) == sizeof( UInt32 ), "Handle size should equal to UInt32 size" );
		static_assert( MAX_SIZE <= ( 1 << HANDLE_TYPE::BITS_PER_ID ), "Handle array size is not suitable for handle type" );
	};
}