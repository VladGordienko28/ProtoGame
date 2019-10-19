//-----------------------------------------------------------------------------
//	Network.cpp: A Network Manager implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Network.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment( lib, "Ws2_32.lib" )

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
			fatal( TXT( "NetworkManager wasn't deinitialized properly" ) );
		}
	}

	Bool NetworkManager::create()
	{
		assert( !instance().m_isInitialized );

		WSADATA	wsaData;
		Int32 wsaError = WSAStartup( 0x0202, &wsaData );

		if( wsaError )
		{
			error( L"WSAStartup failed with error %d", wsaError );
			WSACleanup();
			return false;
		}

		if( wsaData.wVersion != 0x0202 )
		{
			error( L"Unsupported WSA version 0x%x", wsaData.wVersion );
			WSACleanup();
			return false;
		}

		info( L"WinSock2 library initialized: Version: %d.%d (MaxSocks=%d; MaxUdpSocks=%d)",
			wsaData.wVersion / 256, wsaData.wHighVersion / 256, wsaData.iMaxSockets, wsaData.iMaxUdpDg );

		instance().m_isInitialized = true;
		return true;
	}

	void NetworkManager::destroy()
	{
		assert( instance().m_isInitialized );

		WSACleanup();
		instance().m_isInitialized = false;
	}

	Address NetworkManager::getLocalIP( String* hostName )
	{
		assert( instance().m_isInitialized );

		// retrieve host name
		AnsiChar hostNameBuffer[128];

		if ( auto errorCode = gethostname( hostNameBuffer, arraySize( hostNameBuffer ) ) )
		{
			error( L"Unable to get local IP with error %d", errorCode );
			return Address();
		}

		// retrieve host address
		hostent* hostEntries = gethostbyname( hostNameBuffer );
		if( !hostEntries || hostEntries->h_addrtype != PF_INET )
		{
			error( L"Unable to resolve local IP" );
			return Address();
		}

		in_addr localAddr = *reinterpret_cast<in_addr*>( *hostEntries->h_addr_list );

		if( hostName )
		{
			*hostName = ansi2WideString( hostNameBuffer );
		}

		return Address( ntohl( localAddr.S_un.S_addr ), 0 );
	}

	Address NetworkManager::resolveIP( String hostName )
	{
		assert( instance().m_isInitialized );

		hostent* hostEntries = gethostbyname( *wide2AnsiString( hostName ) );
		if( !hostEntries || hostEntries->h_addrtype != PF_INET )
		{
			error( L"Unable to resolve host \"%s\" ip", *hostName );
			return Address();
		}

		in_addr hostAddr = *reinterpret_cast<in_addr*>( *hostEntries->h_addr_list );

		return Address( ntohl( hostAddr.S_un.S_addr ), 0 );
	}

	String NetworkManager::unresolveIP( const Address& address )
	{
		assert( instance().m_isInitialized );

		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = htonl( address.ip );
		addr.sin_port = htons( address.port );

		AnsiChar hostNameBuffer[NI_MAXHOST];
		AnsiChar serviceInfoBuffer[NI_MAXSERV];

		auto errorCode = getnameinfo( reinterpret_cast<sockaddr*>( &addr ), 
			sizeof( sockaddr ), hostNameBuffer, NI_MAXHOST, 
			serviceInfoBuffer, NI_MAXSERV, NI_NUMERICSERV );

		if( errorCode == 0 )
		{
			return ansi2WideString( hostNameBuffer );
		}
		else
		{
			return TXT("");
		}
		
		return L"";
	}

	NetworkManager& NetworkManager::instance()
	{
		static NetworkManager manager;
		return manager;
	}

	Address Address::fromString( String str )
	{
		lexer::Lexer lex( new Text( str ), lexer::LexerConfig() );
		Address result;

		// parse ip
		for( Int32 i = 0; i < 4; ++i )
		{
			Int32 number;

			if( !lex.readInt( number ) || !inRange( number, 0, 255 )  )
			{
				return Address();
			}

			result.numbers[3 - i] = number;

			if( i < 3 && !lex.matchSymbol( TXT(".") ) )
			{
				return Address();
			}
		}
		
		// parse optional port
		if( lex.matchSymbol( TXT(":") ) )
		{
			Int32 port;

			if( !lex.readInt( port ) || !inRange( port, 0, 65535 )  )
			{
				return result;
			}

			result.port = port;
		}

		return result;
	}
}
}