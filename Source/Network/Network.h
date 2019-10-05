//-----------------------------------------------------------------------------
//	Network.h: Network main include file
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------
#pragma once

// fluorine includes
#include "Core/Core.h"

// network includes
#include "Types.h"
#include "Interfaces.h"
#include "NetworkManager.h"




#include "FrConnect.h"



#if 0
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
#endif