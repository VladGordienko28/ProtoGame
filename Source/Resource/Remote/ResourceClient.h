//-----------------------------------------------------------------------------
//	RemoteClient.h: A remote resource client
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	A resource client based on TCP
	 */
	class ResourceClient final: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<ResourceClient>;

		ResourceClient( String appName, const net::Address& serverAddress );
		~ResourceClient();

		Array<ResourceId> trackChanges();

		CompiledResource requestCompiled( EResourceType type, String resourceName );
		CompiledResource requestCompiled( ResourceId resourceId );

		String resolveResourceId( ResourceId resourceId );

	private:
		static const UInt32 NET_IDLING_MS = 3;

		net::TCPClient::UPtr m_tcpClient;

		CompiledResource receiveResource();

		enum EReceiveResult
		{
			Ok,
			Pending
		};

		EReceiveResult receiveServerMessage( ServerMessageHeader& header, Array<UInt8>& data, Bool blocking = true );
		void sendClientMessage( EClientMessage message, const void* data, SizeT dataSize );
	};
}
}