//-----------------------------------------------------------------------------
//	EngineChart.h: Engine statistics chart
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine/Engine.h"

namespace flu
{
	static const Double DEFAULT_TIMELINE_TIME = 7.0;
	static const TColor TIMELINE_PANEL_COLOR = TColor( 16, 16, 16, 200 );

	EngineChart::EngineChart()
		:	m_profiler( static_cast<Int32>(EProfilerGroup::MAX) ),
			m_invTimelineLength( 0.0 ),
			m_invTimelineMaxValue( 0.f ),
			m_enabled( false )
	{
		setTimelineLength( DEFAULT_TIMELINE_TIME );

		// generate color set
		for( Int32 i = 0; i < arraySize(m_colorSet); ++i )
		{
			m_colorSet[i] = TColor::HSLToRGB( i * ( 256 / COLOR_SET_SIZE ), 200, 128 );
		}
		for( Int32 i = 0; i < arraySize(m_colorSet); ++i )
		{
			Exchange( m_colorSet[i], m_colorSet[Random( arraySize(m_colorSet) )] );
		}

		// help
		m_helpString = L"Press appropriate key to toggle groups: ";
		for( Int32 i = 0; i < static_cast<Int32>(EProfilerGroup::MAX); ++i )
		{
			m_helpString += String::Format( L"[%d] %s; ", i, getGroupName( static_cast<EProfilerGroup>( i ) ) );
		}

		// turn on at least one group
		m_profiler.enableGroup( static_cast<Int32>(EProfilerGroup::General) );
	}

	EngineChart::~EngineChart()
	{
		disable();
	}

	void EngineChart::render( CCanvas* canvas, FFont* font )
	{
		if( !m_enabled )
			return;

		canvas->SetTransform( TViewInfo ( 
			0.f, 0.f, canvas->ScreenWidth, canvas->ScreenHeight ) );

		const Float screenW = canvas->ScreenWidth;
		const Float screenH = canvas->ScreenHeight;

		const Float metricDrawStep = 30.f;

		// draw background
		TRenderRect renderRect;
		renderRect.Rotation = 0;
		renderRect.Flags = POLY_FlatShade | POLY_AlphaGhost | POLY_Unlit;

		renderRect.Color = TIMELINE_PANEL_COLOR;
		renderRect.Bounds.Min = { 0.f, 0.f };
		renderRect.Bounds.Max = { screenW , screenH };
		canvas->DrawRect( renderRect );

		TRenderRect metricItem;
		metricItem.Flags = POLY_FlatShade | POLY_Unlit;
		metricItem.Rotation = 0;
		metricItem.Bounds.Min = TVector( 10.f, 10.f );
		metricItem.Bounds.Max = TVector( 30.f, 30.f );

		const Array<profile::EngineProfiler::Group>& groups = m_profiler.getMetrics();

		Double maxValue = 0.0;
		Double frameTime = GPlat->Now();

		const Float timelineScale = 0.90f;
		const Float timelineH = screenH * timelineScale;
		const Float timelineOffset = screenH * (1.f - timelineScale) / 2.f + timelineH;

		const Float graphXRescale = m_invTimelineLength * screenW;
		const Float graphYRescale = m_invTimelineMaxValue * timelineH;

		for( Int32 groupId = 0; groupId < groups.size(); ++groupId )
		{
			if( !m_profiler.isGroupEnabled( groupId ) )
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
				const Int32 colorIndex = COLOR_SET_MASK & ( groupId * 17 + reinterpret_cast<Int32>( it.name ) );
				const TColor drawColor = m_colorSet[colorIndex];

				Double maxMetricValue = it.samples[0].value;
				Double cumulativeValue = it.samples[0].value;

				Float x1 = screenW - ( frameTime - it.samples[0].time ) * graphXRescale;
				Float y1 = timelineOffset - it.samples[0].value * graphYRescale;

				// process all samples
				for( Int32 i = 1; i < it.samples.size(); ++i )
				{
					Float x2 = screenW - ( frameTime - it.samples[i].time ) * graphXRescale;
					Float y2 = timelineOffset - it.samples[i].value * graphYRescale;

					canvas->DrawLine( { x1, y1 }, { x2, y2 }, drawColor, false );

					x1 = x2;
					y1 = y2;

					if( maxMetricValue < it.samples[i].value )
						maxMetricValue = it.samples[i].value;

					cumulativeValue += it.samples[i].value;
				}

				// draw metric legend
				metricItem.Color = drawColor;
				canvas->DrawRect( metricItem );

				canvas->DrawText( *String::Format( L"%s: %s (avg %.2f)", 
					getGroupName(static_cast<EProfilerGroup>( groupId )), it.name, ( cumulativeValue / it.samples.size() ) ), 
					font, COLOR_White, TVector( 40.f, metricItem.Bounds.Min.Y + 2.f ) );

				metricItem.Bounds.Min.Y += metricDrawStep;
				metricItem.Bounds.Max.Y += metricDrawStep;

				maxValue = Max( maxValue, maxMetricValue );
			}		
		}

		m_invTimelineMaxValue = 1.f / maxValue;

		// draw help string
		canvas->DrawText( m_helpString, font, COLOR_White, { 10.f, screenH - 20.f } );
	}

	void EngineChart::setTimelineLength( Float newLength )
	{
		assert( newLength > 0.f );

		m_invTimelineLength = 1.f / newLength;
		m_profiler.setSamplesLifetime( newLength );
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

	void EngineChart::toggleGroup( profile::IProfiler::GroupId groupId )
	{
		if( m_profiler.isGroupEnabled( groupId ) )
		{
			m_profiler.disableGroup( groupId );
		}
		else
		{
			m_profiler.enableGroup( groupId );
		}
	}

	const Char* EngineChart::getGroupName( EProfilerGroup group )
	{
		switch( group )
		{
			case EProfilerGroup::General:	return L"General";
			case EProfilerGroup::Entity:	return L"Entity";
			case EProfilerGroup::Render:	return L"Render";
			case EProfilerGroup::Memory:	return L"Memory";
			default:						return L"Unknown";
		}
	}

} // namespace flu