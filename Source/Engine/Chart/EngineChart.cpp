//-----------------------------------------------------------------------------
//	EngineChart.h: Engine statistics chart
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine/Engine.h"

namespace flu
{
	static const Double DEFAULT_TIMELINE_TIME = 7.0;
	static const math::Color TIMELINE_PANEL_COLOR = { 16, 16, 16, 200 };

	static const Char CHART_FONT_NAME[] = TXT( "Fonts.CourierNew_9" );
	static const Char CHART_EFFECT_NAME[] = TXT( "System.Shaders.Colored" );

	EngineChart::EngineChart()
		:	m_profiler(),
			m_invTimelineLength( 0.0 ),
			m_invTimelineMaxValue( 0.f ),
			m_enabled( false ),
			m_vertexBuffer( "Profiler_VB" )
	{
		setTimelineLength( DEFAULT_TIMELINE_TIME );

		// generate color set
		for( UInt8 i = 0; i < arraySize(m_colorSet); ++i )
		{
			m_colorSet[i] = math::Color::hsl2rgb( i * ( 256 / COLOR_SET_SIZE ), 200, 128 );
		}

		// help
		m_helpString = L"Press appropriate key to toggle groups: ";
		for( Int32 i = 0; i < static_cast<Int32>( profile::EGroup::MAX ); ++i )
		{
			m_helpString += String::format( L"[%d] %s; ", i, getGroupName( static_cast<profile::EGroup>( i ) ) );
		}

		// obtain resources
		m_font = res::ResourceManager::get<fnt::Font>( CHART_FONT_NAME, res::EFailPolicy::FATAL );
		m_effect = res::ResourceManager::get<ffx::Effect>( CHART_EFFECT_NAME, res::EFailPolicy::FATAL );

		// turn on at least one group
		m_profiler.selectGroup( static_cast<Int32>( profile::EGroup::Common ) );
	}

	EngineChart::~EngineChart()
	{
		disable();

		m_font = nullptr;
	}

	void EngineChart::render( CCanvas* canvas, gfx::DrawContext& drawContext )
	{
		if( !m_enabled )
			return;

		profile_zone( Common, CPU_RenderChart );
		profile_gpu_zone( GPU_Chart );
		gfx::ScopedRenderingZone srz( TXT( "Profiler" ) );

		const Float screenW = drawContext.backbufferWidth();
		const Float screenH = drawContext.backbufferHeight();

		drawContext.pushViewInfo( gfx::ViewInfo( 
			0.f, 0.f, screenW, screenH ) );

		const Float metricDrawStep = 30.f;

		// draw background
		TRenderRect renderRect;
		renderRect.Rotation = 0;
		renderRect.Flags = POLY_FlatShade | POLY_AlphaGhost | POLY_Unlit;

		renderRect.Color = TIMELINE_PANEL_COLOR;
		renderRect.Bounds.min = { 0.f, 0.f };
		renderRect.Bounds.max = { screenW , screenH };
		canvas->DrawRect( renderRect );

		TRenderRect metricItem;
		metricItem.Flags = POLY_FlatShade | POLY_Unlit;
		metricItem.Rotation = 0;
		metricItem.Bounds.min = { 10.f, 10.f };
		metricItem.Bounds.max = { 30.f, 30.f };

		const profile::EngineProfiler::Groups& groups = m_profiler.getMetrics();

		Double maxValue = 0.0;
		Double frameTime = time::cyclesToSec( time::cycles64() );

		const Float timelineScale = 0.90f;
		const Float timelineH = screenH * timelineScale;
		const Float timelineOffset = screenH * (1.f - timelineScale) / 2.f + timelineH;

		const Float graphXRescale = m_invTimelineLength * screenW;
		const Float graphYRescale = m_invTimelineMaxValue * timelineH;

		for( Int32 groupId = 0; groupId < groups.size(); ++groupId )
		{
			profile::EGroup group = static_cast<profile::EGroup>( groupId );

			if( group != m_profiler.selectedGroup() )
			{
				continue;
			}

			for( const auto& it : groups[groupId] )
			{
				// don't draw disabled or invalid metric
				if( it.samples.size() < 2 )
				{
					continue;
				}

				// choose color
				const Int32 colorIndex = COLOR_SET_MASK & it.color;
				const math::Color drawColor = m_colorSet[colorIndex];

				Double maxMetricValue = 0.0;
				Double cumulativeValue = 0.0;

				math::Vector* samplesBuffer = m_vertexBuffer.reserve( it.samples.size() );

				for( Int32 i = 0; i < it.samples.size(); ++i )
				{
					Float x = screenW - ( frameTime - it.samples[i].time ) * graphXRescale;
					Float y = timelineOffset - it.samples[i].value * graphYRescale;

					samplesBuffer[i] = { x, y };

					if( maxMetricValue < it.samples[i].value )
						maxMetricValue = it.samples[i].value;

					cumulativeValue += it.samples[i].value;
				}

				m_effect->setColor( 0, drawColor );
				m_effect->apply();

				m_vertexBuffer.flushAndBind();
				gfx::api::setTopology( rend::EPrimitiveTopology::LineStrip );
				gfx::api::draw( it.samples.size(), 0 );

				// draw metric legend
				metricItem.Color = drawColor;
				canvas->DrawRect( metricItem );

				m_textDrawer.batchText( String::format( L"%s: %s (avg %.2f; peak %.2f)", 
					getGroupName( group ), it.name, ( cumulativeValue / it.samples.size() ), maxMetricValue ), 
					m_font, math::colors::WHITE, { 40.f, metricItem.Bounds.min.y + 2.f } );


				metricItem.Bounds.min.y += metricDrawStep;
				metricItem.Bounds.max.y += metricDrawStep;

				maxValue = max( maxValue, maxMetricValue );
			}		
		}

		m_invTimelineMaxValue = 1.f / maxValue;

		// draw help string
		m_textDrawer.batchText( m_helpString, m_font, math::colors::WHITE, { 10.f, screenH - 20.f } );
		m_textDrawer.flush();

		drawContext.popViewInfo();
	}

	void EngineChart::setTimelineLength( Float newLength )
	{
		assert( newLength > 0.f );

		m_invTimelineLength = 1.f / newLength;
		m_profiler.setSamplesLifetime( newLength );
	}

	Bool EngineChart::onGamepadDown( in::GamepadId id, in::EGamepadButton button )
	{
		if( button == in::EGamepadButton::GB_Y )
		{
			if( isEnabled() )
			{
				disable();
			}
			else
			{
				enable();
			}

			return true;
		}

		if( isEnabled() )
		{
			if( button == in::EGamepadButton::GB_LeftShoulder )
			{
				m_profiler.selectPrevGroup();
				return true;
			}

			if( button == in::EGamepadButton::GB_RightShoulder )
			{
				m_profiler.selectNextGroup();
				return true;
			}
		}

		return false;
	}

	Bool EngineChart::onKeyboardDown( in::EKeyboardButton button, Bool repeat )
	{
		if( button == in::EKeyboardButton::KB_Tilde && !repeat )
		{
			if( isEnabled() )
			{
				disable();
			}
			else
			{
				enable();
			}

			return true;
		}

		if( isEnabled() && inRange( button, in::EKeyboardButton::KB_NumPad0, in::EKeyboardButton::KB_NumPad9 ) )
		{
			m_profiler.selectGroup( button - in::EKeyboardButton::KB_NumPad0 );
			return true;
		}

		return false;
	}

	void EngineChart::enable()
	{
		if( !m_enabled )
		{
			m_enabled = true;
			profile::setProfiler( &m_profiler );
		}
	}

	void EngineChart::disable()
	{
		if( m_enabled )
		{
			m_enabled = false;
			profile::setDefaultProfiler();
		}
	}

	Bool EngineChart::isEnabled() const
	{
		return m_enabled;
	}

} // namespace flu