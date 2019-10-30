//-----------------------------------------------------------------------------
//	DxProfiler.h: A DirectX 11 profiler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace dx11
{
	/**
	 *	DirectX 11 GPU profiler
	 */
	class GPUProfiler final: public rend::IGPUProfiler
	{
	public:
		GPUProfiler( ID3D11Device* device, ID3D11DeviceContext* context );
		~GPUProfiler();

		void setGroup( profile::IProfiler::GroupId groupId ) override;

		void beginFrame() override;
		void endFrame() override;

		void enterZone( const Char* zoneName ) override;
		void leaveZone() override;

	private:
		static const Int32 MAX_CPU_FRAMES_AHEAD = 4;
		static const Int32 MAX_CPU_FRAMES_AHEAD_MASK = MAX_CPU_FRAMES_AHEAD - 1;

		static const Int32 MAX_ZONES_DEPTH = 8;

		using Queries = DxRef<ID3D11Query>[MAX_CPU_FRAMES_AHEAD];

		struct Metric
		{
		public:
			UInt32 index = -1;
			const Char* name = nullptr;
			Queries startQueries;
			Queries endtQueries;
			profile::Samples samples;
		};

		Queries m_disjointQueries;
		Queries m_frameStartQueries;
		Queries m_frameEndQueries;

		FixedStack<UInt32, MAX_ZONES_DEPTH> m_zoneStack;

		Array<Metric> m_metrics;
		Int32 m_frameCounter;
		Int32 m_numInvalidFrames;

		profile::IProfiler::GroupId m_groupId;

		static_assert( isPowerOfTwo( MAX_CPU_FRAMES_AHEAD ), "Number of CPU frames ahead should be power of two" );

	private:
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_context;

		GPUProfiler() = delete;

		Metric& findOrAddMetric( const Char* name );
	};
}
}