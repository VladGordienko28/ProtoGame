/*=============================================================================
	FrConnect.h: Simple TCP/UDP wrappers.
	Created by Vlad Gordienko, Apr. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	CUdpConnection.
-----------------------------------------------------------------------------*/

//
// A simple UDP-connection.
//
class CUdpConnection: public CUdpConnectionBase
{
public:
	// CUdpConnection interface.
	CUdpConnection();
	~CUdpConnection();

	// CUdpConnectionBase interface.
	Bool BindToPort( UInt16 Port ) override;
	Bool SendData( const UInt8* Buffer, SizeT Size, const TNetAddress& RemoteAddr ) override;
	SizeT ReceiveData( UInt8* Buffer, SizeT MaxSize, TNetAddress& RemoteAddr ) override;

private:
	// Internal.
	SOCKET		Socket;
};


/*-----------------------------------------------------------------------------
	CTcpConnection.
-----------------------------------------------------------------------------*/

//
// An simple peer-to-peer TCP-connection.
//
class CTcpConnection: public CTcpConnectionBase
{
public:
	// CTcpConnection interface.
	CTcpConnection();
	~CTcpConnection();

	// CTcpConnectionBase interface.
	Bool BindToPort( UInt16 Port ) override;
	Bool Listen() override;
	Bool ConnectTo( const TNetAddress& RemoteAddr ) override;
	Bool Close() override;
	Bool SendData( const UInt8* Buffer, SizeT Size ) override;
	SizeT ReceiveData( UInt8* Buffer, SizeT MaxSize ) override;
	Bool IsConnected() const override;
	Bool IsListener() const override;
	Int32 AcceptConnections() override;

private:
	// Internal.
	SOCKET		Socket;
	SOCKET		RemoteSocket;
	SOCKET*		WorkingSocket;
	Bool		bListener;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/