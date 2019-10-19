//-----------------------------------------------------------------------------
//	Concurrency.h: A multithreading synchronization objects
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Core.h"

#if FLU_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace flu
{
namespace concurrency
{
#if FLU_PLATFORM_WINDOWS
	Dummy* Dummy::create()
	{
		return new Dummy;
	}
#else
#error Dummy is not implemented for current platform
#endif

#if FLU_PLATFORM_WINDOWS
	/**
	 *	A WinApi critical section.
	 */
	class WinCriticalSection: public CriticalSection
	{
	public:
		WinCriticalSection()
		{
			InitializeCriticalSection( &m_criticalSection );
			m_acquired = false;
		}

		~WinCriticalSection()
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
	};

	CriticalSection* CriticalSection::create()
	{
		return new WinCriticalSection();
	}
#else
#error CriticalSection is not implemented for current platform
#endif

#if FLU_PLATFORM_WINDOWS
	/**
	 *	A WinApi mutex
	 */
	class WinMutex: public Mutex
	{
	public:
		WinMutex()
		{
			m_mutex = CreateMutex( nullptr, false, nullptr );
		}

		~WinMutex()
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
	};

	Mutex* Mutex::create()
	{
		return new WinMutex();
	}
#else
#error Mutex is not implemented for current platform
#endif

#if FLU_PLATFORM_WINDOWS
	/**
	 *	A WinApi based spin lock
	 */
	class WinSpinLock: public SpinLock
	{
	public:
		WinSpinLock()
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
	};

	SpinLock* SpinLock::create()
	{
		return new WinSpinLock();
	}
#else
#error SpinLock is not implemented for current platform
#endif

} // namespace concurrency
} // namespace flu