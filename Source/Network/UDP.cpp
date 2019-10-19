//-----------------------------------------------------------------------------
//	UDP.cpp: An UDP connection implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Network.h"

#include <WinSock2.h>

namespace flu
{
namespace net
{
	/**
	 *	An UDP connection implementation
	 */
	class UDPConnectionImpl: public UDPConnection
	{
	public:
		UDPConnectionImpl();
		~UDPConnectionImpl();

		Bool bindToPort( UInt16 port ) override;
		Bool shutdown() override;
		
		Bool sendData( const void* data, SizeT size, const Address& receiverAddress ) override;
		SizeT receiveData( void* data, SizeT size, Address& senderAddress ) override;

		Bool isValid() const override;

	private:
		SOCKET m_socket;
	};

	UDPConnectionImpl::UDPConnectionImpl()
		:	m_socket( INVALID_SOCKET )
	{
	}

	UDPConnectionImpl::~UDPConnectionImpl()
	{
		assert( m_socket == INVALID_SOCKET );
	}

	Bool UDPConnectionImpl::bindToPort( UInt16 port )
	{
		if( m_socket != INVALID_SOCKET )
		{
			error( L"UDP connection is already binded" );
			return false;
		}

		m_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
		if( m_socket == INVALID_SOCKET )
		{
			error( L"Unable to create UDP socket" );
			return false;
		}

		sockaddr_in addr;
		mem::zero( &addr, sizeof( sockaddr_in ) );

		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = htonl( INADDR_ANY );
		addr.sin_port = htons( port );

		if( bind( m_socket, reinterpret_cast<sockaddr*>( &addr ), sizeof( sockaddr_in ) ) != 0 )
		{
			closesocket( m_socket );
			m_socket = INVALID_SOCKET;

			error( L"Unable to bind UDP port %d", port );
			return false;
		}

		// set non-blocking socket
		u_long noBlock = 1;
		if( ioctlsocket( m_socket, FIONBIO, &noBlock ) != 0 )
		{
			closesocket( m_socket );
			m_socket = INVALID_SOCKET;

			error( L"Unable to set UDP port non-blocking" );
			return false;
		}

		return true;
	}

	Bool UDPConnectionImpl::shutdown()
	{
		if( m_socket != INVALID_SOCKET )
		{
			closesocket( m_socket );
			m_socket = INVALID_SOCKET;

			return true;
		}
		else
		{
			return false;
		}
	}
		
	Bool UDPConnectionImpl::sendData( const void* data, SizeT size, const Address& receiverAddress )
	{
		if( m_socket != INVALID_SOCKET )
		{
			sockaddr_in addr;
			mem::zero( &addr, sizeof( sockaddr_in ) );

			addr.sin_family = AF_INET;
			addr.sin_addr.S_un.S_addr = htonl( receiverAddress.ip );
			addr.sin_port = htons( receiverAddress.port );

			Int32 bytesSend = sendto( m_socket, reinterpret_cast<const char*>( data ), size, 
				0, reinterpret_cast<sockaddr*>( &addr ), sizeof( sockaddr_in ) );

			return bytesSend == size;
		}
		else
		{
			return false;
		}
	}

	SizeT UDPConnectionImpl::receiveData( void* data, SizeT size, Address& senderAddress )
	{
		if( m_socket != INVALID_SOCKET )
		{
			sockaddr_in addr;
			int sockAddrSize = sizeof( sockaddr_in );

			Int32 bytesReceived = recvfrom( m_socket, reinterpret_cast<char*>( data ), size, 
				0, reinterpret_cast<sockaddr*>( &addr ), &sockAddrSize );

			if( bytesReceived != SOCKET_ERROR )
			{
				senderAddress.ip = ntohl( addr.sin_addr.S_un.S_addr );
				senderAddress.port = ntohs( addr.sin_port );

				return bytesReceived;
			}
			else
			{
				Int32 socketError = WSAGetLastError();

				if( socketError != WSAEWOULDBLOCK )
				{
					error( L"UDP connection error %d", socketError );
				}

				return 0;
			}
		}
		else
		{
			return 0;
		}
	}

	Bool UDPConnectionImpl::isValid() const
	{
		return m_socket != INVALID_SOCKET;
	}

	UDPConnection* NetworkManager::createUDPConnection()
	{
		assert( instance().m_isInitialized );
		return new UDPConnectionImpl();
	}
}
}