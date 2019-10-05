//-----------------------------------------------------------------------------
//	Types.h: A base network types
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace net
{
	/**
	 *	A network address
	 */
	struct Address
	{
	public:
		UInt32 ip = 0;
		UInt16 port = 0;

		Address()
			:	ip( 0 ), port( 0 )
		{
		}

		Address( UInt32 inIp, UInt16 inPort )
			:	ip( inIp ), port( inPort )
		{
		}
	};
}
}