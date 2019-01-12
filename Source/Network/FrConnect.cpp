/*=============================================================================
	FrConnect.h: Simple TCP/UDP connections.
	Created by Vlad Gordienko, Apr. 2018.
=============================================================================*/

#include "Network.h"

/*-----------------------------------------------------------------------------
	CUdpConnection implementation.
-----------------------------------------------------------------------------*/

//
// Udp connection constructor.
//
CUdpConnection::CUdpConnection()
	:	Socket( INVALID_SOCKET )
{
}


//
// Udp connection destructor.
//
CUdpConnection::~CUdpConnection()
{
	// Close connection if opened.
	if( Socket != INVALID_SOCKET )
	{
		closesocket(Socket);
		Socket = INVALID_SOCKET;
	}
}


//
// Create and bind socket to specified socket.
//
Bool CUdpConnection::BindToPort( Word Port )
{
	// Make sure WinSock initialized.
	if( !Net::WSAInitialize() )
		return false;

	// Already created?
	if( Socket != INVALID_SOCKET )
		return false;

	// Create a socket.
	Socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( Socket == INVALID_SOCKET )
		return false;

	// Bind to address.
	sockaddr_in Addr;
	Addr.sin_family				= AF_INET;
	Addr.sin_addr.S_un.S_addr	= htonl(INADDR_ANY);
	Addr.sin_port				= htons(Port);

	if( bind( Socket, (sockaddr*)&Addr, sizeof(Addr) ) != 0 )
	{
		closesocket(Socket);
		Socket = INVALID_SOCKET;
		return false;
	}

	// Make non-blocking socket.
	u_long NoBlock = 1;
	ioctlsocket( Socket, FIONBIO, &NoBlock );

	// Everything fine.
	Address.Address		= 0;
	Address.Port		= Port;
	return true;
}


//
// Poll datagram.
//
SizeT CUdpConnection::ReceiveData( Byte* Buffer, SizeT MaxSize, TNetAddress& RemoteAddr )
{
	if( Socket == INVALID_SOCKET )
		return 0;

	sockaddr_in Addr;
	Integer SockAddrSize = sizeof(Addr);

	// Ask it.
	Integer BytesGot = recvfrom( Socket, (char*)Buffer, MaxSize, 0, (sockaddr*)&Addr, &SockAddrSize );

	if( BytesGot != SOCKET_ERROR )
	{
		// Successfully.
		RemoteAddr.Address	= ntohl(Addr.sin_addr.S_un.S_addr);
		RemoteAddr.Port		= ntohs(Addr.sin_port);
		return BytesGot;
	}
	else
	{
		// Failed.
		return 0;
	}
}


//
// Send data in a datagram packet.
//
Bool CUdpConnection::SendData( const Byte* Buffer, SizeT Size, const TNetAddress& RemoteAddr )
{
	if( Socket == INVALID_SOCKET )
		return false;

	// Remove address.
	sockaddr_in Addr;
	Addr.sin_addr.S_un.S_addr		= htonl(RemoteAddr.Address);
	Addr.sin_family					= AF_INET;
	Addr.sin_port					= htons(RemoteAddr.Port);

	// Send it.
	Integer NumBytes = sendto( Socket, (char*)Buffer, Size, 0, (sockaddr*)&Addr, sizeof(Addr) );
	return NumBytes != 0;
}


/*-----------------------------------------------------------------------------
	CTcpConnection implementation.
-----------------------------------------------------------------------------*/

//
// TCP connection constructor.
//
CTcpConnection::CTcpConnection()
	:	Socket( INVALID_SOCKET ),
		RemoteSocket( INVALID_SOCKET ),
		bListener( false ),
		WorkingSocket( nullptr )
{
}


//
// TCP connection destructor.
//
CTcpConnection::~CTcpConnection()
{
	if( Socket != INVALID_SOCKET )
	{
		closesocket(Socket);
		Socket = INVALID_SOCKET;
	}
	if( RemoteSocket != INVALID_SOCKET )
	{
		closesocket(RemoteSocket);
		RemoteSocket = INVALID_SOCKET;
	}
}


//
// Create and bind socket to specified port.
//
Bool CTcpConnection::BindToPort( Word Port )
{
	// Make sure WinSock library initialized.
	if( !Net::WSAInitialize() )
		return false;

	// Already created or in invalid state?
	if( Socket != INVALID_SOCKET )
		return false;

	// Create a socket.
	Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( Socket == INVALID_SOCKET )
		return false;

	Integer ReuseAddr = 1;
	setsockopt( Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&ReuseAddr, sizeof(Integer) );

	// Turn on linger.
	LINGER Ling;
	Ling.l_onoff	= 1;
	Ling.l_linger	= 0;
	if( setsockopt( Socket, SOL_SOCKET, SO_LINGER, (char*)&Ling, sizeof(Ling) ) != 0 )
		return false;

	// Bind to address.
	sockaddr_in Addr;
	Addr.sin_family				= AF_INET;
	Addr.sin_addr.S_un.S_addr	= htonl(INADDR_ANY);
	Addr.sin_port				= htons(Port);

	if( bind( Socket, (sockaddr*)&Addr, sizeof(Addr) ) != 0 )
	{
		closesocket(Socket);
		Socket = INVALID_SOCKET;
		return false;
	}
	
	// Make non-blocking socket.
	u_long NoBlock = 1;
	ioctlsocket( Socket, FIONBIO, &NoBlock );

	// Everything fine.
	Address.Address		= 0;
	Address.Port		= Port;
	return true;
}


//
// Enter to listening state.
//
Bool CTcpConnection::Listen()
{
	// Is in appropriate state?
	if( Socket == INVALID_SOCKET || bListener )
		return false;

	// Deaf socket :)
	if( listen(Socket, 1) == SOCKET_ERROR )
		return false;

	bListener = true;
	return true;
}


//
// Open a connection to foreign host.
//
Bool CTcpConnection::ConnectTo( const TNetAddress& RemoteAddr )
{
	// Is in appropriate state?
	if( Socket == INVALID_SOCKET )
		return false;

	// Foreign Host.
	sockaddr_in ForeignHost;
	ForeignHost.sin_family				= AF_INET;
	ForeignHost.sin_port				= htons(RemoteAddr.Port);
	ForeignHost.sin_addr.S_un.S_addr	= htonl(RemoteAddr.Address);

	// Connect.
	Integer Error = connect(Socket, (sockaddr*)&ForeignHost, sizeof(ForeignHost));

	if( Error == SOCKET_ERROR && Net::GetLastError() != WSAEWOULDBLOCK )
		return false;

	WorkingSocket = &Socket;
	bListener = false;
	return true;
}


//
// Close connection.
//
Bool CTcpConnection::Close()
{
	if( !WorkingSocket )
		return false;

	if( bListener )
	{
		if( RemoteSocket != INVALID_SOCKET )
			closesocket(RemoteSocket);
		if( Socket != INVALID_SOCKET )
			closesocket(Socket);
	}
	else
	{
		if( Socket != INVALID_SOCKET )
			closesocket(Socket);
	}

	Socket = RemoteSocket = INVALID_SOCKET;
	return true;
}


//
// Return true if connection is still alive.
//
Bool CTcpConnection::IsConnected() const
{
	fd_set SocketSet;
	timeval SelectTimeOut = {0, 0};

	if( !WorkingSocket )
		return false;

	FD_ZERO( &SocketSet );
	FD_SET( *WorkingSocket, &SocketSet );
	Integer Error = select( *WorkingSocket + 1, 0, &SocketSet, 0, &SelectTimeOut );
	return Error != SOCKET_ERROR && Error != 0;
}


//
// Send data through TCP connection.
//
Bool CTcpConnection::SendData( const Byte* Buffer, SizeT Size )
{
	if( !WorkingSocket || *WorkingSocket == INVALID_SOCKET  )
		return false;

	Integer NumBytes = send( *WorkingSocket, (char*)Buffer, Size, 0 );
	return NumBytes != 0;
}


//
// Tries to accept all awaiting connections. Return -1 if connection broken, 
// return 0, if no connection waiting, otherwise return number of connection, 
// but only 1 connections support.
//
Integer CTcpConnection::AcceptConnections()
{
	assert(bListener == true);

	fd_set SocketSet;
	timeval SelectTimeOut = {0, 0};

	FD_ZERO(&SocketSet);
	FD_SET(Socket, &SocketSet);

	Integer Error = select( Socket+1, &SocketSet, 0, 0, &SelectTimeOut );
	
	// Some problem happed.
	if( Error == SOCKET_ERROR )
		return -1;

	// No connection waiting.
	if( Error == 0 )
		return 0;

	// Accept connection.
	Integer Size = sizeof(sockaddr);
	sockaddr_in OtherAddr;
	RemoteSocket = accept( Socket, (sockaddr*)&OtherAddr, &Size );
	
	if( RemoteSocket == INVALID_SOCKET )
		return -1;

	// Make remote socket non-blocking.
	u_long NoBlock = 1;
	ioctlsocket( RemoteSocket, FIONBIO, &NoBlock );

	// Ok.
	RemoteAddress.Address	= ntohl(OtherAddr.sin_addr.S_un.S_addr);
	RemoteAddress.Port		= ntohs(OtherAddr.sin_port);
	WorkingSocket			= &RemoteSocket;
	return 1;
}


//
// Return true, if this is server side.
//
Bool CTcpConnection::IsListener() const
{
	return bListener;
}


//
// Read streaming data.
//
SizeT CTcpConnection::ReceiveData( Byte* Buffer, SizeT MaxSize )
{
	if( !WorkingSocket || *WorkingSocket == INVALID_SOCKET )
		return 0;

	// Read data.
	Integer BytesGot = recv( *WorkingSocket, (char*)Buffer, MaxSize, 0 );
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/