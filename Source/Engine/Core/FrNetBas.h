/*=============================================================================
	FrNetBas.h: Abstract network classes and structures.
	Created by Vlad Gordienko, Apr. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Basic structures.
-----------------------------------------------------------------------------*/

// Not specified network port.
#define NETWORK_ANY_PORT	((Word)0)

//
// A network address.
//
struct TNetAddress
{
public:
	union
	{
		DWord	Address;
		Byte	Numbers[4];
	};
	Word	Port;

	TNetAddress() 
		:	Address(0), Port(NETWORK_ANY_PORT)
	{}
	TNetAddress( DWord InAddress, Word InPort = NETWORK_ANY_PORT )
		:	Address(InAddress), Port(InPort)
	{}
};


/*-----------------------------------------------------------------------------
	CUdpConnectionBase.
-----------------------------------------------------------------------------*/

//
// An abstrct UDP-connection.
//
class CUdpConnectionBase
{
public:
	// CUdpConnectionBase interface.
	virtual ~CUdpConnectionBase(){};
	virtual Bool BindToPort( Word Port ) = 0;
	virtual Bool SendData( const Byte* Buffer, SizeT Size, const TNetAddress& RemoteAddr ) = 0;
	virtual SizeT ReceiveData( Byte* Buffer, SizeT MaxSize, TNetAddress& RemoteAddr  ) = 0;

protected:
	// Internal.
	TNetAddress		Address;
};


/*-----------------------------------------------------------------------------
	CTcpConnectionBase.
-----------------------------------------------------------------------------*/

//
// An endpoint to endpoint abstract TCP-connection.
//
class CTcpConnectionBase
{
public:
	// CTcpConnectionBase interface.
	virtual ~CTcpConnectionBase(){}
	virtual Bool BindToPort( Word Port ) = 0;
	virtual Bool Close() = 0;
	virtual Bool SendData( const Byte* Buffer, SizeT Size ) = 0;
	virtual SizeT ReceiveData( Byte* Buffer, SizeT MaxSize ) = 0;
	virtual Bool IsConnected() const = 0;
	virtual Bool IsListener() const = 0;

	// Client only functions.
	virtual Bool ConnectTo( const TNetAddress& RemoteAddr ) = 0;

	// Server only functions.
	virtual Bool Listen() = 0;
	virtual Integer AcceptConnections() = 0;

	// Accessors.
	inline TNetAddress GetRemoteAddress() const
	{
		return RemoteAddress;
	}

protected:
	// Internal.
	TNetAddress		Address;
	TNetAddress		RemoteAddress;
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/