//-----------------------------------------------------------------------------
//	GPUProfiler.h: An abstract gpu profiler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace rend
{
	/**
	 *	An abstract GPU profiler
	 */
	class IGPUProfiler: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<IGPUProfiler>;

		virtual ~IGPUProfiler() = default;

		virtual void beginFrame() = 0;
		virtual void endFrame() = 0;

		virtual void enterZone( const Char* zoneName ) = 0;
		virtual void leaveZone() = 0;
	};
}
}