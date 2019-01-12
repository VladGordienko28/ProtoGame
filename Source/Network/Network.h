/*=============================================================================
	Network.h: Network general include file.
	Copyright Apr.2018 Vlad Gordienko.
=============================================================================*/
#ifndef _FLU_NETWORK_
#define _FLU_NETWORK_

// C++ includes.
#include <WinSock2.h>
#pragma comment (lib, "Ws2_32.lib")

// Flu includes.
#include "..\Engine\Engine.h"

// Network includes.
#include "FrConnect.h"


//
// Network utils.
//
namespace Net
{
	extern Bool WSAInitialize( String* Error = nullptr );
	extern TNetAddress GetLocalIP();
	extern Integer GetLastError();
	extern String IPToString( const TNetAddress& Address );
	extern TNetAddress StringToIP( String Str );
}


#endif
/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/