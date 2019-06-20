//-----------------------------------------------------------------------------
//	StringManager.h: An amazing fluorine strings manager
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	An universal string manager template
	 */
	template<typename T> class StringManager final
	{
	public:
		using CHAR_TYPE = T;

#pragma warning( disable : 4200 )
		struct StringData
		{
			union
			{
				struct{ SizeT length; UInt32 refsCount; };
				struct{ StringData* nextInPool; };
			};
			CHAR_TYPE data[0];
		};

		StringManager()
		{
			mem::zero( m_reusePool, sizeof(m_reusePool) );
		}

		~StringManager()
		{
			for( SizeT i = 0; i < REUSE_POOL_SIZE; ++i )
			{
				StringData* stringData = m_reusePool[i];

				while( stringData != nullptr )
				{
					 StringData* tmp = stringData;
					 stringData = stringData->nextInPool;
					 mem::free( tmp );
				}
			}

			mem::zero( m_reusePool, sizeof(m_reusePool) );
		}

		void initializeString( StringData*& data, SizeT requiredLength )
		{
			if( data )
			{
				deinitializeString( data );
			}

			concurrency::ScopeLock lock( m_reusePoolLock );

			if( requiredLength < REUSE_POOL_SIZE && m_reusePool[requiredLength] )
			{
				data = m_reusePool[requiredLength];
				m_reusePool[requiredLength] = data->nextInPool;
			}
			else
			{
				data = reinterpret_cast<StringData*>( 
					mem::malloc( sizeof(StringData) + sizeof(CHAR_TYPE) * ( requiredLength + 1 ) ) );
				data->data[requiredLength] = 0;
			}
			
			data->length = requiredLength;
			data->refsCount = 1;
		}

		void deinitializeString( StringData*& data )
		{
			if( data )
			{
				if( --data->refsCount == 0 )
				{
					if( data->length < REUSE_POOL_SIZE )
					{
						concurrency::ScopeLock lock( m_reusePoolLock );

						SizeT len = data->length;
						data->nextInPool = m_reusePool[len];
						m_reusePool[len] = data;
					}
					else
					{
						mem::free( data );
					}
				}
			
				data = nullptr;
			}
		}

		void reinitializeString( StringData*& data, SizeT newLength )
		{
			if( newLength > 0 )
			{
				if( data )
				{
					StringData* newData = nullptr;
					initializeString( newData, newLength );
					SizeT minLength = min( newData->length, data->length );
					mem::copy( newData->data, data->data, minLength * sizeof(CHAR_TYPE) );
					newData->data[minLength] = 0;
					deinitializeString( data );
					data = newData;
				}
				else
				{
					initializeString( data, newLength );
				}
			}
			else
			{
				deinitializeString( data );
			}
		}

		void cleanupPool( Bool showInfo = true )
		{
			Int32 slotsInUse = 0;
			Int32 maxDepth = 0;
			Int32 totalLength = 0;
			Int32 totalStrings = 0;

			concurrency::ScopeLock lock( m_reusePoolLock );

			for( SizeT i = 0; i < REUSE_POOL_SIZE; ++i )
			{
				StringData* stringData = m_reusePool[i];

				if( stringData )
				{
					Int32 depth = 0;
					++slotsInUse;

					while( stringData != nullptr )
					{
						 StringData* tmp = stringData;
						 stringData = stringData->nextInPool;
						 mem::free( tmp );

						 ++depth;
						 ++totalStrings;
						 totalLength += i;
					}

					if( depth > maxDepth )
					{
						maxDepth = depth;
					}

					m_reusePool[i] = nullptr;
				}
			}

			if( showInfo )
			{
				info( L"String ReusePool info: " );
				info( L"  SlotsInUse: %d; MaxDepth: %d", slotsInUse, maxDepth );
				info( L"  TotalStrings: %d; TotalLength: %d", totalStrings, totalLength );
			}
		}

	private:
		enum{ REUSE_POOL_SIZE = 8192 };
		static_assert( isPowerOfTwo( REUSE_POOL_SIZE ), "Reuse pool size should be power of two" );

		StringData* m_reusePool[REUSE_POOL_SIZE];
		concurrency::SpinLock m_reusePoolLock;

		StringManager( StringManager<T>&& ) = delete;
		StringManager( const StringManager<T>& ) = delete;
		StringManager<T>& operator=( const StringManager<T>& ) = delete;
		StringManager<T>& operator=( StringManager<T>&& ) = delete;
	};

	/**
	 *	String managers
	 */
	using AnsiStringManager = StringManager<AnsiChar>;
	using WideStringManager = StringManager<WideChar>;

	extern AnsiStringManager g_ansiStringManager;
	extern WideStringManager g_wideStringManager;
}