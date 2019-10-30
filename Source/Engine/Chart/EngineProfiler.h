//-----------------------------------------------------------------------------
//	EngineProfiler.h: Custom Fluorine Engine profiler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu 
{
namespace profile
{
	/**
	 *	A FluorineEngine profiler
	 */
	class EngineProfiler final: public IProfiler
	{
	public:
		EngineProfiler( Int32 numGroups );
		~EngineProfiler();

		void beginFrame() override;
		void endFrame() override;

		void enterZone( GroupId groupId, const Char* zoneName ) override;
		void leaveZone() override;

		void updateCounter( GroupId groupId, const Char* counterName, Double value ) override;

		struct Metric
		{
			const Char* name = nullptr;
			UInt32 color = 0;
			Samples samples;
		};

		using Group = Array<Metric>;

		const Array<Group>& getMetrics() const;
		void setSamplesLifetime( Double lifeTime );

		void selectGroup( GroupId groupId );
		void selectNextGroup();
		void selectPrevGroup();

		GroupId selectedGroup() const
		{
			return m_selectedGroup;
		}

	private:
		static const Int32 MAX_ZONES_DEPTH = 16;

		struct Zone
		{
			Metric* metric = nullptr;
			UInt64 enterTimeStamp = 0;
			GroupId groupId = -1;
		};		
		
		Double m_frameEnterTime;
		Double m_samplesLifetime;
		Bool m_frameLocked;

		FixedStack<Zone, MAX_ZONES_DEPTH> m_zonesStack;

		Array<Group> m_groups; // todo: needs better solution
		GroupId m_selectedGroup;

		EngineProfiler() = delete;
		Metric& findOrAddMetric( GroupId groupId, const Char* name );
	};

} // namespace profile
} // namespace flu