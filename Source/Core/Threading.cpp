//-----------------------------------------------------------------------------
//	Threading.cpp: A threads implementation
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Core.h"

#if FLU_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace flu
{
namespace threading
{
#if FLU_PLATFORM_WINDOWS
	static const DWORD gMainThreadId = GetCurrentThreadId();
#endif

#if FLU_PLATFORM_WINDOWS
	/**
	 *	This is WinAPI thread
	 */
	class WinThread: public Thread
	{
	public:
		WinThread( EntryFunction entryFunction, void* args )
			:	m_entryFunction( entryFunction ),
				m_args( args )
		{
			DWORD outThreadId;
			m_thread = CreateThread( nullptr, 0, _threadEntryPoint, this, 0, &outThreadId );
			//todo: add thread naming
		}

		~WinThread()
		{
			WaitForSingleObject( m_thread, INFINITE );
			CloseHandle( m_thread );
		}

		void setPriority( EThreadPriority newPriority ) override
		{
			switch( newPriority )
			{
				case EThreadPriority::Low:
					SetThreadPriority( m_thread, THREAD_PRIORITY_BELOW_NORMAL );
					break;

				case EThreadPriority::Normal:
					SetThreadPriority( m_thread, THREAD_PRIORITY_NORMAL );
					break;

				case EThreadPriority::High:
					SetThreadPriority( m_thread, THREAD_PRIORITY_ABOVE_NORMAL );
					break;

				default:
					assert( false && "Unknown thread priority" );
			}
		}

		void wait() override
		{
			WaitForSingleObject( m_thread, INFINITE );
		}

	private:
		HANDLE m_thread;

		EntryFunction m_entryFunction;
		void* m_args;

		static DWORD _stdcall _threadEntryPoint( LPVOID pThis )
		{
			WinThread* thisThread = reinterpret_cast<WinThread*>( pThis );
			thisThread->m_entryFunction( thisThread->m_args );
			return 0;		
		}
	};

	Thread* Thread::create( EntryFunction entryFunction, void* arg )
	{
		return new WinThread( entryFunction, arg );
	}

#else
#error Thread is not implemented for current platform
#endif

	ThreadId getMainThreadId()
	{
#if FLU_PLATFORM_WINDOWS
		return static_cast<ThreadId>( gMainThreadId );
#else
#error threading::getMainThreadId is not implemented for current platform
#endif
	}

	ThreadId getCurrentThreadId()
	{
#if FLU_PLATFORM_WINDOWS
		return static_cast<ThreadId>( GetCurrentThreadId() );
#else
#error threading::getCurrentThreadId is not implemented for current platform
#endif
	}

	Bool isMainThread()
	{
#if FLU_PLATFORM_WINDOWS
		return gMainThreadId == GetCurrentThreadId();
#else
#error threading::isMainThread is not implemented for current platform
#endif
	}

	UInt32 getCPUCoresCount()
	{
		assert( false && "Not implemented" );
		return 0;
	}

	void sleep( UInt32 milliseconds )
	{
#if FLU_PLATFORM_WINDOWS
		Sleep( milliseconds );
#else
#error threading::sleep is not implemented for current platform
#endif
	}

} // namespace threading
} // namespace flu