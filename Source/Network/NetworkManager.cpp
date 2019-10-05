//-----------------------------------------------------------------------------
//	Network.cpp: A Network Manager implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Network.h"

namespace flu
{
namespace net
{
	NetworkManager::NetworkManager()
		:	m_isInitialized( false )
	{
	}

	NetworkManager::~NetworkManager()
	{
		if( m_isInitialized )
		{
			fatal( TEXT( "NetworkManager wasn't deinitialized properly" ) );
		}
	}

	Bool NetworkManager::create()
	{
		return false;
	}

	void NetworkManager::destroy()
	{
	}

	UDPConnection* NetworkManager::createUDPConnection()
	{
		return nullptr;
	}

	TCPClient* NetworkManager::createTCPClient()
	{
		return nullptr;
	}

	TCPServer* NetworkManager::createTCPServer()
	{
		return nullptr;
	}

	Address NetworkManager::getLocalIP()
	{
		return Address();
	}

	NetworkManager& NetworkManager::instance()
	{
		static NetworkManager manager;
		return manager;
	}
}
}