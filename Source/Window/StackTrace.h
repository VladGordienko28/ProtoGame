//-----------------------------------------------------------------------------
//	StackTrace.h: A stack tracking helper
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu {
namespace win {

	static const SizeT MAX_STACK_TRACE_LENGTH = 4096;

	/**
	 *	Returns current stack as string
	 */
	extern Char* stackTrace( LPEXCEPTION_POINTERS exception );
}
}