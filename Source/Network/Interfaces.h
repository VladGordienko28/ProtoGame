//-----------------------------------------------------------------------------
//	Interfaces.h: A network interfaces
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace net
{
	/**
	 *	A list of connection errors
	 */
	enum class EError
	{
		Ok,
		Failed,		// API call error
		BadState,	// Connection is in bad state to perform operation
		Refused		// Disconnected by the other side
	};

	/**
	 *	An UDP connection interface
	 */
	class UDPConnection: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<UDPConnection>;

		virtual Bool bindToPort( UInt16 port ) = 0;
		virtual Bool shutdown() = 0;
		
		virtual Bool sendData( const void* data, SizeT size, const Address& receiverAddress ) = 0;
		virtual SizeT receiveData( void* data, SizeT size, Address& senderAddress ) = 0;

		virtual Bool isValid() const = 0;
	};

	/**
	 *	A TCP client interface
	 */
	class TCPClient: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<TCPClient>;

		enum class EState
		{
			Disconnected,
			Connecting,
			Connected
		};

		virtual EError connect( const Address& serverAddress ) = 0; // todo: add timeout here!
		virtual EError shutdown() = 0;

		virtual EError waitForConnection() = 0;

		virtual EError sendData( const void* data, SizeT size, SizeT& bytesSended, Bool blocking = false ) = 0;
		virtual EError receiveData( void* data, SizeT size, SizeT& bytesReceived, Bool blocking = false ) = 0;

		virtual EState getState() const = 0;
		
		Bool isConnected() const
		{
			return getState() == EState::Connected;
		}
	};

	/**
	 *	A TCP server interface
	 */
	class TCPServer: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<TCPServer>;

		using ClientId = Handle<16, 16, 'TCPC'>;

		enum class EState
		{
			Disabled,
			Listening
		};

		virtual EError listen( UInt16 port, UInt32 maxQueueSize ) = 0;
		virtual EError shutdown() = 0;

		virtual EError sendData( ClientId clientId, const void* data, SizeT size, SizeT& bytesSended, Bool blocking = false ) = 0;
		virtual EError receiveData( ClientId clientId, void* data, SizeT size, SizeT& bytesReceived, Bool blocking = false ) = 0;

		virtual ClientId pollConnection() = 0;
		virtual EError disconnectClient( ClientId clientId ) = 0;
		virtual Address getClientAddress( ClientId clientId ) const = 0;

		virtual EState getState() const = 0;

		Bool isListening() const
		{
			return getState() == EState::Listening;
		}
	};
}
}