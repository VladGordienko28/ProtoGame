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
		union
		{
			UInt32 ip;
			UInt8 numbers[4];
		};
		UInt16 port;

		Address()
			:	ip( 0 ), port( 0 )
		{
		}

		Address( UInt32 inIp, UInt16 inPort )
			:	ip( inIp ), port( inPort )
		{
		}

		Bool isValid() const
		{
			return ip != 0;
		}

		String toString() const
		{
			String result = String::format( TXT("%d.%d.%d.%d"), 
				numbers[3], numbers[2], numbers[1], numbers[0] );

			if( port != 0 )
			{
				result += String::format( TXT(":%d"), port );
			}

			return result;
		}

		static Address fromString( String str );
	};
}
}