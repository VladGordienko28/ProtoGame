//-----------------------------------------------------------------------------
//	RemoteServer.h: A remote resource server
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	A resource server based on TCP
	 */
	class ResourceServer final: public NonCopyable
	{
	public:
		static Bool create();
		static void destroy();

		static void registerResourceType( EResourceType type, IResourceCompiler* compiler );

		static void update();

	private:
		struct Client
		{
		public:
			enum class EStatus
			{
				Unverified,
				Accepted
			};

			net::TCPServer::ClientId id;
			net::Address address;
			String name;

			EStatus status;

			// resource transfer
			Array<UInt8> resourceData;
			Int32 resourceBytesRemain = 0;
			UInt64 transferStartTime = 0;
		};

	private:
		static const UInt16 DEFAULT_LISTENING_PORT = 28280;

		static constexpr Double FILES_TRACKING_PERIOD_SEC = 2.f;

		Bool m_isInitialized;
	
		UniquePtr<class LocalStorage> m_localStorage;

		net::TCPServer::UPtr m_tcpServer;
		Array<Client> m_clients;

		UInt64 m_lastTickTime;
		Double m_timeToTrackRemain;

		ResourceServer();
		~ResourceServer();

		UInt32 pollClients();

		Bool serveClient( Client& client, const Array<ResourceId>& changedResources );

		Bool serveUnverified( Client& client );
		Bool serveAccepted( Client& client, const Array<ResourceId>& changedResources );

		Bool handleRequestResource( Client& client, const ClientMessageHeader& reqHeader, const Array<UInt8>& reqData );
		Bool handleRequestBlock( Client& client, const ClientMessageHeader& reqHeader, const Array<UInt8>& reqData );
		Bool handleResolveName( Client& client, const ClientMessageHeader& reqHeader, const Array<UInt8>& reqData );

		enum EReceiveResult
		{
			Ok,
			Pending,
			Failed
		};

		EReceiveResult receiveClientMessage( net::TCPServer::ClientId id, ClientMessageHeader& header, 
			Array<UInt8>& data, Bool blocking );

		Bool sendServerMessage( net::TCPServer::ClientId id, EServerMessage message, const void* data, SizeT dataSize );

		static ResourceServer& instance();

	private:
		class LogListener: public IListener
		{
		public:
			void onError( String resourceName, String message ) override
			{
				error( TXT("Res: %s %s"), *resourceName, *message );
			}

			void onWarning( String resourceName, String message ) override
			{
				warn( TXT("Res: %s %s"), *resourceName, *message );
			}

			void onInfo( String resourceName, String message ) override
			{
				info( TXT("Res: %s %s"), *resourceName, *message );
			}
		};

		LogListener m_logListener;
	};
}
}