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
		EngineProfiler();
		~EngineProfiler();

		void beginFrame() override;
		void endFrame() override;

		void enterZone( EGroup group, const Char* zoneName ) override;
		void leaveZone() override;

		void updateCounter( EGroup group, const Char* counterName, Double value ) override;

		struct Metric
		{
			const Char* name = nullptr;
			UInt32 color = 0;
			Samples samples;
		};

		using Group = Array<Metric>;
		using Groups = StaticArray<Group, static_cast<SizeT>( EGroup::MAX )>;

		const Groups& getMetrics() const;
		void setSamplesLifetime( Double lifeTime );

		void selectGroup( Int32 groupId );
		void selectNextGroup();
		void selectPrevGroup();

		EGroup selectedGroup() const
		{
			return m_selectedGroup;
		}

	private:
		static const Int32 MAX_ZONES_DEPTH = 16;

		struct Zone
		{
			Metric* metric = nullptr;
			UInt64 enterTimeStamp = 0;
			EGroup groupId = EGroup::Common;
		};		
		
		Double m_frameEnterTime;
		Double m_samplesLifetime;
		Bool m_frameLocked;

		FixedStack<Zone, MAX_ZONES_DEPTH> m_zonesStack;

		Groups m_groups;
		EGroup m_selectedGroup;

		Metric& findOrAddMetric( EGroup group, const Char* name );
	};

} // namespace profile
} // namespace flu