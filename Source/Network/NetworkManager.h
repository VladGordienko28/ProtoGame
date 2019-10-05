//-----------------------------------------------------------------------------
//	Network.h: A Network Manager
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace net
{
	/**
	 *	A network manager
	 */
	class NetworkManager final: NonCopyable
	{
	public:
		static Bool create();
		static void destroy();

		static UDPConnection* createUDPConnection();

		static TCPClient* createTCPClient();
		static TCPServer* createTCPServer();

		static Address getLocalIP();

	private:
		Bool m_isInitialized;

		NetworkManager();
		~NetworkManager();

		NetworkManager& instance();
	};
}
}