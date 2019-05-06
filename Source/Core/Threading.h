//-----------------------------------------------------------------------------
//	Threading.h: A multithreading threads
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
namespace threading
{
	/**
	 *	An index of the thread
	 */
	using ThreadId = UInt32;

	/**
	 *	A list of available thread priorities
	 */
	enum class EThreadPriority
	{
		Low,
		Normal,
		High
	};

	/**
	 *	An entry thread function
	 */
	typedef void( *EntryFunction )( void* );

	/**
	 *	This is thread
	 */
	class Thread
	{
	public:
		Thread( EntryFunction entryFunction, void* arg );
		~Thread();
		void setPriority( EThreadPriority newPriority );
		void wait();

	private:
		Thread() = delete;
		Thread( const Thread& ) = delete;
		Thread( Thread&& ) = delete;
		Thread& operator=( const Thread& ) = delete;
		Thread& operator=( Thread&& ) = delete;

#if FLU_PLATFORM_WINDOWS
		HANDLE m_thread;

		EntryFunction m_entryFunction;
		void* m_args;

		static DWORD _stdcall _threadEntryPoint( LPVOID pThis );
#else
#error Thread is not implemented for current platform
#endif
	};

	/**
	 *	Return id of the main thread
	 */
	extern ThreadId getMainThreadId();

	/**
	 *	Return the id of the caller's thread
	 */
	extern ThreadId getCurrentThreadId();

	/**
	 *	Return true, if caller's thread is main
	 */
	extern Bool isMainThread();

	/**
	 *	Return user's logical CPU cores count
	 */
	extern UInt32 getCPUCoresCount();

	/**
	 *	Suspend current thread for some time
	 */
	extern void sleep( UInt32 milliseconds );

} // namespace threading
} // namespace flu