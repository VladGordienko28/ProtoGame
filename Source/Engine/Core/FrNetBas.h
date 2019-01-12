/*=============================================================================
	FrNetBas.h: Abstract network classes and structures.
	Created by Vlad Gordienko, Apr. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Basic structures.
-----------------------------------------------------------------------------*/

// Not specified network port.
#define NETWORK_ANY_PORT	((UInt16)0)

//
// A network address.
//
struct TNetAddress
{
public:
	union
	{
		UInt32	Address;
		UInt8	Numbers[4];
	};
	UInt16	Port;

	TNetAddress() 
		:	Address(0), Port(NETWORK_ANY_PORT)
	{}
	TNetAddress( UInt32 InAddress, UInt16 InPort = NETWORK_ANY_PORT )
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
	virtual Bool BindToPort( UInt16 Port ) = 0;
	virtual Bool SendData( const UInt8* Buffer, SizeT Size, const TNetAddress& RemoteAddr ) = 0;
	virtual SizeT ReceiveData( UInt8* Buffer, SizeT MaxSize, TNetAddress& RemoteAddr  ) = 0;

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
	virtual Bool BindToPort( UInt16 Port ) = 0;
	virtual Bool Close() = 0;
	virtual Bool SendData( const UInt8* Buffer, SizeT Size ) = 0;
	virtual SizeT ReceiveData( UInt8* Buffer, SizeT MaxSize ) = 0;
	virtual Bool IsConnected() const = 0;
	virtual Bool IsListener() const = 0;

	// Client only functions.
	virtual Bool ConnectTo( const TNetAddress& RemoteAddr ) = 0;

	// Server only functions.
	virtual Bool Listen() = 0;
	virtual Int32 AcceptConnections() = 0;

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