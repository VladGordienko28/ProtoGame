//-----------------------------------------------------------------------------
//	IStorage.h: An abstract resource storage
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	An abstract resource storage
	 */
	class IStorage: public NonCopyable
	{
	public:
		virtual void setListener( IListener* listener ) = 0;

		virtual CompiledResource requestCompiled( EResourceType type, String resourceName ) = 0;
		virtual CompiledResource requestCompiled( ResourceId resourceId ) = 0;

		virtual String resolveResourceId( ResourceId resourceId ) = 0;

		virtual void update( ResourceSystemList& systemList ) = 0;
	};
}
}