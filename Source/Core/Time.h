//-----------------------------------------------------------------------------
//	Time.h: Time relative functions
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace time
{
	extern UInt32 cycles();
	extern UInt64 cycles64();

	extern Double cyclesToSec( UInt64 c );
	extern Double cyclesToMs( UInt64 c );

	extern Double elapsedSecFrom( UInt64 c );
	extern Double elapsedMsFrom( UInt64 c );
}
}