//-----------------------------------------------------------------------------
//	Common.h: A resource system network communication common stuff
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	Constants
	 */
	static const SizeT MAX_PACKET_SIZE = 16384;

	/**
	 *	A list of messages comming from server 
	 */
	enum class EServerMessage : UInt8
	{
		RequestInfo,		// ask client's credentials
		NotifyReload,		// notify client about changed resource
		ProvideResource,	// provide base resource information
		NextResourceBlock,	// next resource block, if resource is too big
		ResourceError,		// unable to provide requested resource
		ResolvedName		// send resolved resource name
	};

	/**
	 *	A list of messages comming from client 
	 */
	enum class EClientMessage : UInt8
	{
		ProvideInfo,			// client credentials
		RequestResourceById,	// request resource by ResourceId
		RequestResourceByName,	// request resource by name
		RequestNextBlock,		// request next resource block is resource is too big
		ResolveName				// request name resolving
	};

	/**
	 *	A server respond header
	 */
	struct ServerMessageHeader
	{
	public:
		EServerMessage message;
		UInt16 payloadSize;
	};

	/**
	 *	A client respond header
	 */
	struct ClientMessageHeader
	{
	public:
		EClientMessage message;
		UInt16 payloadSize;
	};

	static_assert( sizeof( ServerMessageHeader ) == sizeof( UInt32 ), "ServerMessageHeader size should be 4 bytes" );
	static_assert( sizeof( ClientMessageHeader ) == sizeof( UInt32 ), "ClientMessageHeader size should be 4 bytes" );

	static_assert( 1 << ( sizeof( decltype(ServerMessageHeader::payloadSize) ) * 8 ) >= MAX_PACKET_SIZE, 
		"payload size should be greater MAX_PACKET_SIZE" );

	static_assert( 1 << ( sizeof( decltype(ClientMessageHeader::payloadSize) ) * 8 ) >= MAX_PACKET_SIZE, 
		"payload size should be greater MAX_PACKET_SIZE" );
}
}