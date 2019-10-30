//-----------------------------------------------------------------------------
//	Utils.h: Various high-level rendering utils
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace gfx
{
	/**
	 *	A helper class which allowe to mark some interesting 
	 *	zone in rendering debugging tools, such as RenderDoc
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

	/**
	 *	Returns SRV from the any handle type
	 */
	template<typename HANDLE_TYPE> rend::ShaderResourceView srvOf( HANDLE_TYPE handle )
	{
		return api::getShaderResourceView( handle );
	}

	/**
	 *	A wrapper class for GPU zone tracking
	 */
	class GPUZoneTracker final
	{
	public:
		GPUZoneTracker( const Char* zoneName )
		{
			api::enterGPUProfileZone( zoneName );
		}

		~GPUZoneTracker()
		{
			api::leaveGPUProfileZone();
		}
	};
}
}

/**
  *	Profiler macro
  */
#if FLU_ENABLE_PROFILER
	#define profile_gpu_zone( zoneName ) flu::gfx::GPUZoneTracker gpuZoneTracker( L#zoneName );
#else
	#define profile_gpu_zone( zoneName )
#endif