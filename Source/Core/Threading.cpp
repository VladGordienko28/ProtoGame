//-----------------------------------------------------------------------------
//	Threading.cpp: A threads implementation
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Core.h"

namespace flu
{
namespace threading
{
#if FLU_PLATFORM_WINDOWS
	static const DWORD gMainThreadId = GetCurrentThreadId();
#endif

#if FLU_PLATFORM_WINDOWS

	DWORD _stdcall Thread::_threadEntryPoint( LPVOID pThis )
	{
		Thread* thisThread = reinterpret_cast<Thread*>( pThis );
		thisThread->m_entryFunction( thisThread->m_args );
		return 0;
	}

	Thread::Thread( EntryFunction entryFunction, void* args )
		:	m_entryFunction( entryFunction ),
			m_args( args )
	{
		DWORD outThreadId;
		m_thread = CreateThread( nullptr, 0, _threadEntryPoint, this, 0, &outThreadId );
		//todo: add thread naming
	}

	Thread::~Thread()
	{
		WaitForSingleObject( m_thread, INFINITE );
		CloseHandle( m_thread );
	}

	void Thread::setPriority( EThreadPriority newPriority )
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

	void Thread::wait()
	{
		WaitForSingleObject( m_thread, INFINITE );
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