//-----------------------------------------------------------------------------
//	RemoteStorage.h: A remote machine storage
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	A remote storage, which requests ResourceServer for resources
	 *	on the remote machine
	 */
	class RemoteStorage final: public IStorage
	{
	public:
		using UPtr = UniquePtr<RemoteStorage>;

		RemoteStorage( String serverAddress );
		~RemoteStorage();

		void setListener( IListener* listener ) override;

		CompiledResource requestCompiled( EResourceType type, String resourceName ) override;
		CompiledResource requestCompiled( ResourceId resourceId ) override;

		String resolveResourceId( ResourceId resourceId ) override;

		void update( ResourceSystemList& systemList ) override;

	private:
		ResourceClient::UPtr m_client;
		NamesResolver m_namesResolver;

		IListener* m_listener;
	};
}
}