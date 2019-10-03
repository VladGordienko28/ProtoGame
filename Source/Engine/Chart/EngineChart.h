//-----------------------------------------------------------------------------
//	EngineChart.h: Engine statistics chart
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	A profiler groups
	 */
	enum class EProfilerGroup
	{
		General,
		Entity,
		Render,
		Memory,
		DrawCalls,
		MAX
	};

	/**
	 *	An engine chart
	 */
	class EngineChart final
	{
	public:
		EngineChart();
		~EngineChart();

		void render( CCanvas* canvas );
		
		void setTimelineLength( Float newLength );
		void toggleGroup( profile::IProfiler::GroupId groupId );

		void enable();
		void disable();
		Bool isEnabled() const;

		static const Char* getGroupName( EProfilerGroup group );

	private:
		static const SizeT COLOR_SET_SIZE = 64;
		static const SizeT COLOR_SET_MASK = COLOR_SET_SIZE - 1;

		profile::EngineProfiler m_profiler;
		Float m_invTimelineLength;
		Float m_invTimelineMaxValue;
		Bool m_enabled;

		fnt::Font::Ptr m_font;

		gfx::GrowOnlyVB<math::Vector, 128> m_vertexBuffer;
		math::Color m_colorSet[COLOR_SET_SIZE];
		String m_helpString;
	};

} // namespace flu