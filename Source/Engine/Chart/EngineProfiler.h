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

		struct Sample
		{
			Double time = 0.f;
			Double value = 0.0;
		};

		struct Metric
		{
			const Char* name = nullptr;
			Array<Sample> samples;
		};

		using Group = Array<Metric>;

		const Array<Group>& getMetrics() const;
		void setSamplesLifetime( Double lifeTime );

		void enableGroup( GroupId groupId )
		{
			if( groupId >= 0 && groupId < m_groupFilter.size() )
				m_groupFilter[groupId] = true;
		}

		void disableGroup( GroupId groupId )
		{
			if( groupId >= 0 && groupId < m_groupFilter.size() )
				m_groupFilter[groupId] = false;
		}

		Bool isGroupEnabled( GroupId groupId ) const
		{
			return groupId >= 0 && groupId < m_groupFilter.size() ? 
				m_groupFilter[groupId] : false;
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

		FixedStack<Zone> m_zonesStack;

		Array<Group> m_groups; // needs better solution
		Array<Bool> m_groupFilter;

		EngineProfiler() = delete;
		Metric& findOrAddMetric( GroupId groupId, const Char* name );
	};

} // namespace profile
} // namespace flu