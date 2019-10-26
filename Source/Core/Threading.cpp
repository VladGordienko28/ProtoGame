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
	 *	WinApi hack for thread naming
	 *	see: https://docs.microsoft.com/ru-ru/visualstudio/debugger/how-to-set-a-thread-name-in-native-code
	 */
	void SetThreadName( DWORD dwThreadID, const char* threadName )
	{
		const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack( push, 8 )
		typedef struct tagTHREADNAME_INFO
		{
			DWORD dwType; // Must be 0x1000.
			LPCSTR szName; // Pointer to name (in user addr space).
			DWORD dwThreadID; // Thread ID (-1=caller thread).
			DWORD dwFlags; // Reserved for future use, must be zero.
		} THREADNAME_INFO;
#pragma pack( pop )

		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = threadName;
		info.dwThreadID = dwThreadID;
		info.dwFlags = 0;
#pragma warning( push )
#pragma warning( disable: 6320 6322 )
		__try
		{
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {}
#pragma warning( pop )
	}

	/**
	 *	This is WinAPI thread
	 */
	class WinThread: public Thread
	{
	public:
		WinThread( EntryFunction entryFunction, void* args, const AnsiChar* name )
			:	m_entryFunction( entryFunction ),
				m_args( args )
		{
			DWORD outThreadId;
			m_thread = CreateThread( nullptr, 0, _threadEntryPoint, this, 0, &outThreadId );
			SetThreadName( outThreadId, name );
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

	Thread* Thread::create( EntryFunction entryFunction, void* arg, const AnsiChar* name )
	{
		return new WinThread( entryFunction, arg, name );
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
#if FLU_PLATFORM_WINDOWS
		static UInt32 numCores = 0;

		if( numCores == 0 )
		{
			SYSTEM_INFO systemInfo;
			GetSystemInfo( &systemInfo );

			numCores = systemInfo.dwNumberOfProcessors;
		}

		return numCores;
#else
#error threading::getCPUCoresCount is not implemented for current platform
#endif
	}

	void sleep( UInt32 milliseconds )
	{
#if FLU_PLATFORM_WINDOWS
		Sleep( milliseconds );
#else
#error threading::sleep is not implemented for current platform
#endif
	}

	void yield()
	{
#if FLU_PLATFORM_WINDOWS
		SwitchToThread();
#else
#error threading::yield is not implemented for current platform
#endif
	}

} // namespace threading
} // namespace flu