/*=============================================================================
	Network.h: Network general include file.
	Copyright Apr.2018 Vlad Gordienko.
=============================================================================*/
#pragma once

// C++ includes.
#include <WinSock2.h>
#pragma comment (lib, "Ws2_32.lib")

// Flu includes.
#include "Engine\Engine.h"

// Network includes.
#include "FrConnect.h"


//
// Network utils.
//
namespace Net
{
	extern Bool WSAInitialize( String* Error = nullptr );
	extern TNetAddress GetLocalIP();
	extern Int32 GetLastError();
	extern String IPToString( const TNetAddress& Address );
	extern TNetAddress StringToIP( String Str );
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/