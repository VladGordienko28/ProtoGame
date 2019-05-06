//-----------------------------------------------------------------------------
//	Concurrency.h: A multithreading synchronization objects
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
namespace concurrency
{
	/**
	 *	An abstract asynchronization object
	 */
	class ISyncObject
	{
	public:
		virtual ~ISyncObject() = default;
		virtual void lock() = 0;
		virtual void unlock() = 0;
	};


	/**
	 *	A dummy synchronization object
	 */
	class Dummy: public ISyncObject
	{
	public:
		Dummy()
		{
		}

		void lock() override
		{
		}

		void unlock() override
		{
		}
	};

	/**
	 *	A critical section.
	 */
	class CriticalSection: public ISyncObject
	{
#if FLU_PLATFORM_WINDOWS
	public:
		CriticalSection()
		{
			InitializeCriticalSection( &m_criticalSection );
			m_acquired = false;
		}

		~CriticalSection()
		{
			DeleteCriticalSection( &m_criticalSection );
		}

		void lock() override
		{
			EnterCriticalSection( &m_criticalSection );
			assert( m_acquired == false );
			m_acquired = true;
		}

		Bool tryLock()
		{
			if( TryEnterCriticalSection( &m_criticalSection ) )
			{
				assert( m_acquired == false );
				m_acquired = true;
				return true;
			}
			else
			{
				return false;
			}
		}

		void unlock() override
		{
			m_acquired = false;
			LeaveCriticalSection( &m_criticalSection );
		}

	private:
		CRITICAL_SECTION m_criticalSection;
		Bool m_acquired;
#else
#error CriticalSection is not implemented for current platform
#endif
	};

	/**
	 *	A mutex
	 */
	class Mutex: public ISyncObject
	{
#if FLU_PLATFORM_WINDOWS
	public:
		Mutex()
		{
			m_mutex = CreateMutex( nullptr, false, nullptr );
		}

		~Mutex()
		{
			CloseHandle( m_mutex );
		}

		void lock() override
		{
			WaitForSingleObject( m_mutex, INFINITE );
		}

		void unlock() override
		{
			ReleaseMutex( m_mutex );
		}

	private:
		HANDLE m_mutex;
#else
#error Mutex is not implemented for current platform
#endif
	};

	/**
	 *	A lightweight spinlock
	 */
	class SpinLock: public ISyncObject
	{
#if FLU_PLATFORM_WINDOWS
	public:
		SpinLock()
		{
			m_locked = false;
		}

		void lock() override
		{
			static const UInt32 IDLING_COUNT = 128;
			UInt32 spinner = IDLING_COUNT;

			for( ; ; )
			{
				if( InterlockedExchange( &m_locked, 1 ) == 0 )
				{
					break;
				}

				if( --spinner == 0 )
				{
					spinner = IDLING_COUNT;
					Sleep( 0 );
				}
			}
		}

		void unlock() override
		{
			InterlockedExchange( &m_locked, 0 );
		}

	private:
		volatile UInt32 m_locked;
#else
#error SpinLock is not implemented for current platform
#endif
	};

	/**
	 *	A scoped lock which is useful for guard data in
	 *	scope
	 */
	class ScopeLock final
	{
	public:	
		ScopeLock( typename ISyncObject& inSyncObject )
			:	m_syncObject( inSyncObject )
		{
			m_syncObject.lock();
		}

		~ScopeLock()
		{
			m_syncObject.unlock();
		}

	private:
		ISyncObject& m_syncObject;

		ScopeLock() = delete;
		ScopeLock( const ScopeLock& ) = delete;
		ScopeLock( ScopeLock&& ) = delete;
		ScopeLock& operator=( const ScopeLock& ) = delete;
		ScopeLock& operator=( ScopeLock&& ) = delete;
	};

} // namespace concurrency
} // namespace flu