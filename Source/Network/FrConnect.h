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
	Bool BindToPort( Word Port ) override;
	Bool SendData( const Byte* Buffer, SizeT Size, const TNetAddress& RemoteAddr ) override;
	SizeT ReceiveData( Byte* Buffer, SizeT MaxSize, TNetAddress& RemoteAddr ) override;

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
	Bool BindToPort( Word Port ) override;
	Bool Listen() override;
	Bool ConnectTo( const TNetAddress& RemoteAddr ) override;
	Bool Close() override;
	Bool SendData( const Byte* Buffer, SizeT Size ) override;
	SizeT ReceiveData( Byte* Buffer, SizeT MaxSize ) override;
	Bool IsConnected() const override;
	Bool IsListener() const override;
	Integer AcceptConnections() override;

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