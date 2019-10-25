//-----------------------------------------------------------------------------
//	ResourceServerApp.h: A resource server app
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace shell
{
	/**
	 *	A resource server app
	 */
	class ResourceServerApp: public IApp
	{
	public:
		ResourceServerApp();
		~ResourceServerApp();

		Bool create( String commandLine ) override;
		Bool destroy() override;

		Int32 run() override;

	private:
		static const UInt32 SERVER_IDLE_MS = 10;
	};
}
}