//-----------------------------------------------------------------------------
//	TCP.cpp: A TCP client and server implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Network.h"

#include <WinSock2.h>

namespace flu
{
namespace net
{
	/**
	 *	An TCP client implementation
	 */
	class TCPClientImpl: public TCPClient
	{
	public:
		TCPClientImpl();
		~TCPClientImpl();

		EError connect( const Address& serverAddress ) override;
		EError shutdown() override;

		EError waitForConnection() override;

		EError sendData( const void* data, SizeT size, SizeT& sended, Bool blocking ) override;
		EError receiveData( void* data, SizeT size, SizeT& received, Bool blocking ) override;

		EState getState() const override;

	private:
		SOCKET m_socket;
		EState m_state;

		void disconnect();
	};

	/**
	 *	An TCP server implementation
	 */
	class TCPServerImpl: public TCPServer
	{
	public:
		TCPServerImpl();
		~TCPServerImpl();

		EError listen( UInt16 port, UInt32 maxQueueSize ) override;
		EError shutdown() override;

		EError sendData( ClientId clientId, const void* data, SizeT size, SizeT& bytesSended, Bool blocking ) override;
		EError receiveData( ClientId clientId, void* data, SizeT size, SizeT& bytesReceived, Bool blocking ) override;

		ClientId pollConnection() override;
		EError disconnectClient( ClientId clientId ) override;
		Address getClientAddress( ClientId clientId ) const override;

		EState getState() const override;

	private:
		static const SizeT MAX_CLIENTS = 8;

		SOCKET m_socket;
		EState m_state;

		HandleArray<ClientId, SOCKET, MAX_CLIENTS> m_clientSockets;
		Map<ClientId, Address> m_clients;

		void disconnect( SOCKET& socket );
	};

	static void blockSocket( SOCKET socket, Bool forRecv, Bool forSend )
	{
		assert( socket != INVALID_SOCKET );

		fd_set forRecvSet, forSendSet;

		FD_ZERO( &forRecvSet );
		FD_ZERO( &forSendSet );

		if( forRecv )
		{
			FD_SET( socket, &forRecvSet );
		}
		if( forSend )
		{
			FD_SET( socket, &forSendSet );
		}

		int result = select( socket + 1, &forRecvSet, &forSendSet, nullptr, nullptr ); // blocking
		assert( result >= 0 );
	}

	TCPClientImpl::TCPClientImpl()
		:	m_socket( INVALID_SOCKET ),
			m_state( EState::Disconnected )
	{
	}

	TCPClientImpl::~TCPClientImpl()
	{
		assert( m_socket == INVALID_SOCKET );
	}

	EError TCPClientImpl::connect( const Address& serverAddress )
	{
		if( m_socket != INVALID_SOCKET )
		{
			error( L"TCP client is already connected" );
			return EError::BadState;
		}

		m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		m_state = EState::Disconnected;

		if( m_socket == INVALID_SOCKET )
		{
			error( L"Unable to create TCP socket" );
			return EError::Failed;
		}

		// set non-blocking socket
		u_long noBlock = 1;
		if( ioctlsocket( m_socket, FIONBIO, &noBlock ) != 0 )
		{
			closesocket( m_socket );
			m_socket = INVALID_SOCKET;

			error( L"Unable to set TCP port non-blocking" );
			return EError::Failed;
		}		

		sockaddr_in addr;
		mem::zero( &addr, sizeof( sockaddr_in ) );

		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = htonl( serverAddress.ip );
		addr.sin_port = htons( serverAddress.port );

		Int32 errorCode = ::connect( m_socket, reinterpret_cast<sockaddr*>( &addr ), sizeof( sockaddr_in ) );
		if( errorCode == SOCKET_ERROR )
		{
			Int32 lastError = WSAGetLastError();

			if( lastError != WSAEWOULDBLOCK )
			{
				error( L"Unable to open TCP connection with error %d", lastError );

				closesocket( m_socket );
				m_socket = INVALID_SOCKET;
				return EError::Failed;
			}
		}

		m_state = EState::Connecting;
		return EError::Ok;
	}

	EError TCPClientImpl::shutdown()
	{
		if( m_state != EState::Disconnected )
		{
			assert( m_socket != INVALID_SOCKET );
	
			disconnect();
			return EError::Ok;
		}
		else
		{
			assert( m_socket == INVALID_SOCKET );
			return EError::BadState;
		}
	}

	EError TCPClientImpl::waitForConnection()
	{
		if( m_state == EState::Connecting )
		{
			assert( m_socket != INVALID_SOCKET );

			fd_set socketSet;
			timeval selectTime = { 0, 0 };

			FD_ZERO( &socketSet );
			FD_SET( m_socket, &socketSet );

			Int32 selectError = select( m_socket + 1, nullptr, &socketSet, nullptr, &selectTime );

			if( selectError != SOCKET_ERROR /*&& selectError != 0*/ ) // todo: investigate for XBox
			{
				// socket is writable, so connected
				m_state = EState::Connected;
			}
			else
			{
				return EError::Failed; // todo : why???
			}
		}

		return EError::Ok;
	}

	EError TCPClientImpl::sendData( const void* data, SizeT size, SizeT& bytesSended, Bool blocking )
	{
		if( m_state == EState::Connected )
		{
			const UInt8* dataPtr = reinterpret_cast<const UInt8*>( data );
			Int32 bytesRemain = size;

			while( bytesRemain > 0 )
			{
				Int32 bytesSend = send( m_socket, reinterpret_cast<const char*>( dataPtr ), bytesRemain, 0 );

				if( bytesSend == SOCKET_ERROR )
				{
					if( WSAGetLastError() != WSAEWOULDBLOCK )
					{
						disconnect();

						bytesSended = 0;
						return EError::Refused;
					}

					if( !blocking )
					{
						bytesSended = size - bytesRemain;
						return EError::Ok;	
					}

					blockSocket( m_socket, true, false );
				}
				else if( bytesSend == 0 )
				{
					disconnect();

					bytesSended = 0;
					return EError::Refused;
				}
				else
				{
					bytesRemain -= bytesSend;
					dataPtr += bytesSend;				
				}
			}

			bytesSended = size - bytesRemain;
			return EError::Ok;
		}
		else
		{
			return EError::BadState;
		}
	}

	EError TCPClientImpl::receiveData( void* data, SizeT size, SizeT& bytesReceived, Bool blocking )
	{
		if( m_state == EState::Connected )
		{
			UInt8* dataPtr = reinterpret_cast<UInt8*>( data );
			Int32 bytesRemain = size;

			while( bytesRemain > 0 )
			{
				Int32 bytesReceive = recv( m_socket, reinterpret_cast<char*>( dataPtr ), bytesRemain, 0 );

				if( bytesReceive == SOCKET_ERROR )
				{
					if( WSAGetLastError() != WSAEWOULDBLOCK )
					{
						disconnect();

						bytesReceived = 0;
						return EError::Refused;
					}

					if( !blocking )
					{
						bytesReceived = size - bytesRemain;
						return EError::Ok;	
					}

					blockSocket( m_socket, true, false );
				}
				else if( bytesReceive == 0 )
				{
					disconnect();

					bytesReceived = 0;
					return EError::Refused;
				}
				else
				{
					bytesRemain -= bytesReceive;
					dataPtr += bytesReceive;				
				}
			}

			bytesReceived = size - bytesRemain;
			return EError::Ok;
		}
		else
		{
			return EError::BadState;
		}
	}

	TCPClient::EState TCPClientImpl::getState() const
	{
		return m_state;
	}

	void TCPClientImpl::disconnect() 
	{
		assert( m_state != EState::Disconnected );

		closesocket( m_socket );
		m_socket = INVALID_SOCKET;
		m_state = EState::Disconnected;
	}

	TCPServerImpl::TCPServerImpl()
		:	m_socket( INVALID_SOCKET ),
			m_state( EState::Disabled ),
			m_clients()
	{
	}

	TCPServerImpl::~TCPServerImpl()
	{
		assert( m_socket == INVALID_SOCKET );
		assert( m_clients.size() == 0 );
	}

	EError TCPServerImpl::listen( UInt16 port, UInt32 maxQueueSize )
	{
		if( m_socket != INVALID_SOCKET || m_state != EState::Disabled )
		{
			error( L"TCP server is already binded" );
			return EError::BadState;
		}

		m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		m_state = EState::Disabled;

		if( m_socket == INVALID_SOCKET )
		{
			error( L"Unable to create TCP server socket" );
			return EError::Failed;
		}

		// set non-blocking socket
		u_long noBlock = 1;
		if( ioctlsocket( m_socket, FIONBIO, &noBlock ) != 0 )
		{
			closesocket( m_socket );
			m_socket = INVALID_SOCKET;

			error( L"Unable to set TCP socket non-blocking" );
			return EError::Failed;
		}

		// set reuse address socket
		u_long reuseAddr = 1;
		setsockopt( m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>( &reuseAddr ), sizeof( u_long ) );

		sockaddr_in addr;
		mem::zero( &addr, sizeof( sockaddr_in ) );

		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = htonl( INADDR_ANY );
		addr.sin_port = htons( port );

		if( bind( m_socket, reinterpret_cast<sockaddr*>( &addr ), sizeof( sockaddr_in ) ) != 0 )
		{
			closesocket( m_socket );
			m_socket = INVALID_SOCKET;

			error( L"Unable to bind TCP server port %d", port );
			return EError::Failed;
		}

		if( ::listen( m_socket, maxQueueSize ) == SOCKET_ERROR )
		{
			error( L"Unable to start TCP server listening" );
			return EError::Failed;
		}

		m_state = EState::Listening;
		return EError::Ok;
	}

	EError TCPServerImpl::shutdown()
	{
		if( m_state != EState::Disabled )
		{
			assert( m_socket != INVALID_SOCKET );
	
			// release all client sockets
			for( auto& clientId : m_clients )
			{
				disconnect( m_clientSockets.get( clientId.key ) );
			}
			m_clients.empty();

			// release listening socket
			disconnect( m_socket );
			m_state = EState::Disabled;

			return EError::Ok;
		}
		else
		{
			assert( m_socket == INVALID_SOCKET );
			assert( m_clients.size() == 0 );
			return EError::BadState;
		}
	}

	EError TCPServerImpl::sendData( ClientId clientId, const void* data, SizeT size, SizeT& bytesSended, Bool blocking )
	{
		if( m_state == EState::Listening )
		{
			SOCKET& remoteSocket = m_clientSockets.get( clientId );

			const UInt8* dataPtr = reinterpret_cast<const UInt8*>( data );
			Int32 bytesRemain = size;

			while( bytesRemain > 0 )
			{
				Int32 bytesSend = send( remoteSocket, reinterpret_cast<const char*>( dataPtr ), bytesRemain, 0 );

				if( bytesSend == SOCKET_ERROR )
				{
					if( WSAGetLastError() != WSAEWOULDBLOCK )
					{
						disconnectClient( clientId );

						bytesSended = 0;
						return EError::Refused;
					}

					if( !blocking )
					{
						bytesSended = size - bytesRemain;
						return EError::Ok;	
					}

					blockSocket( remoteSocket, true, false );
				}
				else if( bytesSend == 0 )
				{
					disconnectClient( clientId );

					bytesSended = 0;
					return EError::Refused;
				}
				else
				{
					bytesRemain -= bytesSend;
					dataPtr += bytesSend;				
				}
			}

			bytesSended = size - bytesRemain;
			return EError::Ok;
		}
		else
		{
			return EError::BadState;
		}
	}

	EError TCPServerImpl::receiveData( ClientId clientId, void* data, SizeT size, SizeT& bytesReceived, Bool blocking )
	{
		if( m_state == EState::Listening )
		{
			SOCKET& remoteSocket = m_clientSockets.get( clientId );

			UInt8* dataPtr = reinterpret_cast<UInt8*>( data );
			Int32 bytesRemain = size;

			while( bytesRemain > 0 )
			{
				Int32 bytesReceive = recv( remoteSocket, reinterpret_cast<char*>( dataPtr ), bytesRemain, 0 );

				if( bytesReceive == SOCKET_ERROR )
				{
					if( WSAGetLastError() != WSAEWOULDBLOCK )
					{
						disconnectClient( clientId );

						bytesReceived = 0;
						return EError::Refused;
					}

					if( !blocking )
					{
						bytesReceived = size - bytesRemain;
						return EError::Ok;	
					}

					blockSocket( remoteSocket, true, false );
				}
				else if( bytesReceive == 0 )
				{
					disconnectClient( clientId );

					bytesReceived = 0;
					return EError::Refused;
				}
				else
				{
					bytesRemain -= bytesReceive;
					dataPtr += bytesReceive;				
				}
			}

			bytesReceived = size - bytesRemain;
			return EError::Ok;
		}
		else
		{
			return EError::BadState;
		}
	}

	TCPServer::ClientId TCPServerImpl::pollConnection()
	{
		if( m_state == EState::Listening )
		{
			fd_set socketSet;
			timeval selectTime = { 0, 0 };

			FD_ZERO( &socketSet );
			FD_SET( m_socket, &socketSet );

			Int32 errorCode = select( m_socket + 1, &socketSet, nullptr, nullptr, &selectTime );

			if( errorCode == SOCKET_ERROR )
			{
				error( L"Polling incoming connections failed with error %d", WSAGetLastError() );
				return INVALID_HANDLE<ClientId>();
			}
			if( errorCode == 0 )
			{
				// no incoming connections awaiting
				return INVALID_HANDLE<ClientId>();
			}

			sockaddr_in remoteAddr;
			int addrSize = sizeof( sockaddr_in );

			SOCKET remoteSocket = accept( m_socket, reinterpret_cast<sockaddr*>( &remoteAddr ), &addrSize );
			if( remoteSocket == INVALID_SOCKET )
			{
				error( L"Unable to accept client with error %d", WSAGetLastError() );
				return INVALID_HANDLE<ClientId>();
			}

			// set non-blocking remote socket
			u_long noBlock = 1;
			if( ioctlsocket( remoteSocket, FIONBIO, &noBlock ) != 0 )
			{
				closesocket( remoteSocket );
				remoteSocket = INVALID_SOCKET;

				error( L"Unable to set TCP client socket non-blocking" );
				return INVALID_HANDLE<ClientId>();
			}

			// add to list
			ClientId clientId = m_clientSockets.addElement( remoteSocket );
			m_clients.put( clientId, Address( ntohl( remoteAddr.sin_addr.S_un.S_addr ), ntohs( remoteAddr.sin_port ) ) );

			return clientId;
		}
		else
		{
			return INVALID_HANDLE<ClientId>();
		}
	}

	EError TCPServerImpl::disconnectClient( ClientId clientId )
	{
		SOCKET& clientSocket = m_clientSockets.get( clientId );
		disconnect( clientSocket );

		m_clientSockets.removeElement( clientId );
		m_clients.remove( clientId );
		return EError::Ok;
	}

	Address TCPServerImpl::getClientAddress( ClientId clientId ) const
	{
		const Address* addr = m_clients.get( clientId );
		return addr ? *addr : Address();
	}

	TCPServer::EState TCPServerImpl::getState() const
	{
		return m_state;
	}

	void TCPServerImpl::disconnect( SOCKET& socket )
	{
		assert( socket != INVALID_SOCKET );

		closesocket( socket );
		socket = INVALID_SOCKET;
	}

	TCPClient* NetworkManager::createTCPClient()
	{
		assert( instance().m_isInitialized );
		return new TCPClientImpl();
	}

	TCPServer* NetworkManager::createTCPServer()
	{
		assert( instance().m_isInitialized );
		return new TCPServerImpl();
	}
}
}