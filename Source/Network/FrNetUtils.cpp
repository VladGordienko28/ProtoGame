/*=============================================================================
	FrNetUtils.cpp: Network Utils.
	Created by Vlad Gordienko, Apr. 2018.
=============================================================================*/

#include "Network.h"

/*-----------------------------------------------------------------------------
	Network utils.
-----------------------------------------------------------------------------*/

namespace Net
{

//
// Convert IP address to string.
//
String IPToString( const TNetAddress& Address )
{
	String Result = String::Format( L"%d.%d.%d.%d", Address.Numbers[0], Address.Numbers[1], Address.Numbers[2], Address.Numbers[3] );
	if( Address.Port != 0 && Address.Port != NETWORK_ANY_PORT )
		Result += String::Format( L":%d", Address.Port );
	return Result;
}


//
// Convert string to network address.
//
TNetAddress StringToIP( String Str )
{
	return TNetAddress();
}


//
// Return current local IP.
//
TNetAddress GetLocalIP()
{
	// Get host name.
	char NameBuffer[64];
	if( !WSAInitialize() || gethostname( NameBuffer, sizeof(NameBuffer) ) )
		return TNetAddress();

	// Figure out host address.
	HOSTENT* HostEnt = gethostbyname(NameBuffer);
	if( !HostEnt || HostEnt->h_addrtype != PF_INET )
		return TNetAddress();

	in_addr HostAddr = *(in_addr*)(*HostEnt->h_addr_list);

	TNetAddress NetAddr;
	NetAddr.Address	= ntohl(HostAddr.S_un.S_addr); 
	NetAddr.Port	= 0;

	return NetAddr;
}


//
// Return last network error.
//
Integer GetLastError()
{
	return WSAGetLastError();
}


//
// Initialize WinSock2 library.
//
Bool WSAInitialize( String* Error )
{
	static Bool		GInitialized = false;
	static WSADATA	WSAData;

	if( !GInitialized )
	{
		Integer WSAError = WSAStartup( 0x0202, &WSAData );
		if( WSAError )
		{
			if( Error )
				*Error	= String::Format( L"WSA initialization failed with code %d", WSAError );
			return false;
		}
		if( WSAData.wVersion != 0x0202 )
		{
			if( Error )
				*Error	= String::Format( L"Bad WSA version %d", WSAData.wVersion );
			WSACleanup();
			return false;
		}

		log
		(
			L"WinSock2: Version: %d.%d (MaxSocks=%d, MaxUdpSocks=%d)",
			WSAData.wVersion / 256,
			WSAData.wHighVersion / 256,
			WSAData.iMaxSockets,
			WSAData.iMaxUdpDg
		);

		GInitialized = true;
	}

	return true;
}


}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/