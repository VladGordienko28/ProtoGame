//-----------------------------------------------------------------------------
//	Interfaces.h: A network interfaces
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace net
{
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

		virtual Bool connect( const Address& serverAddress ) = 0;
		virtual Bool shutdown() = 0;

		virtual void update() const = 0;

		virtual Bool sendData( const void* data, SizeT size ) = 0;
		virtual SizeT receiveData( void* data, SizeT size ) = 0;

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

		virtual Bool bindToPort( UInt16 port ) = 0;
		virtual Bool shutdown() = 0;

		virtual Bool listen( UInt32 maxQueueSize ) = 0;

		virtual void update() = 0;

		virtual Bool sendData( ClientId clientId, const void* data, SizeT size ) = 0;
		virtual SizeT receiveData( ClientId clientId, void* data, SizeT size ) = 0;

		virtual ClientId pollConnection() = 0;
		virtual Bool disconnectClient( ClientId clientId ) = 0;
		virtual Bool isConnected( ClientId clientId ) const = 0;

		virtual EState getState() const = 0;

		Bool isListening() const
		{
			return getState() == EState::Listening;
		}
	};
}
}