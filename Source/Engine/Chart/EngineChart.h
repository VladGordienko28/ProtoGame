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
		RAMMemory,
		GPUMemory,
		DrawCalls,
		MAX
	};

	/**
	 *	An engine chart
	 */
	class EngineChart final: public in::InputClient, public NonCopyable
	{
	public:
		using UPtr = UniquePtr<EngineChart>;

		EngineChart();
		~EngineChart();

		void render( CCanvas* canvas, gfx::DrawContext& drawContext );
		
		void setTimelineLength( Float newLength );

		static const Char* getGroupName( EProfilerGroup group );

	private:
		static const SizeT COLOR_SET_SIZE = 64;
		static const SizeT COLOR_SET_MASK = COLOR_SET_SIZE - 1;

		profile::EngineProfiler m_profiler;
		Float m_invTimelineLength;
		Float m_invTimelineMaxValue;
		Bool m_enabled;

		fnt::Font::Ptr m_font;
		ffx::Effect::Ptr m_effect;

		gfx::GrowOnlyVB<math::Vector, 256> m_vertexBuffer;
		gfx::TextDrawer m_textDrawer;

		math::Color m_colorSet[COLOR_SET_SIZE];
		String m_helpString;

		Bool onGamepadDown( in::GamepadId id, in::EGamepadButton button ) override;
		Bool onKeyboardDown( in::EKeyboardButton button, Bool repeat ) override;

		void enable();
		void disable();
		Bool isEnabled() const;
	};

} // namespace flu