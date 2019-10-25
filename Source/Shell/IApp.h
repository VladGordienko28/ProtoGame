//-----------------------------------------------------------------------------
//	IApp.h: An abstract shell application
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace shell
{
	/**
	 *	An abstract Fluorine Shell application
	 */
	class IApp: public NonCopyable
	{
	public:
		virtual ~IApp() = default;

		virtual Bool create( String commandLine ) = 0;
		virtual Bool destroy() = 0;

		virtual Int32 run() = 0;
	};
}
}