//-----------------------------------------------------------------------------
//	Utils.h: Various high-level rendering utils
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace gfx
{
	/**
	 *	A helper class which allowed to mark some interesting 
	 *	in rendering debugging tools
	 */
	class ScopedRenderingZone final: public NonCopyable
	{
	public:
		ScopedRenderingZone( const Char* zoneName )
		{
			assert( zoneName );
			api::enterZone( zoneName );
		}

		~ScopedRenderingZone()
		{
			api::leaveZone();
		}

	private:
		ScopedRenderingZone() = delete;
	};
}
}