//-----------------------------------------------------------------------------
//	EngineProfiler.cpp: Profiler implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine/Engine.h"

namespace flu 
{
namespace profile
{
	EngineProfiler::EngineProfiler( Int32 numGroups )
		:	m_frameEnterTime( 0.0 ),
			m_samplesLifetime( 1.0 ),
			m_frameLocked( false )
	{
		m_groups.setSize( numGroups );
		m_groupFilter.setSize( numGroups );
	}

	EngineProfiler::~EngineProfiler()
	{
	}

	void EngineProfiler::beginFrame()
	{
		assert( !m_frameLocked );

		m_frameEnterTime = time::cyclesToSec( time::cycles64() );
		m_frameLocked = true;
	}

	void EngineProfiler::endFrame()
	{
		//assert( m_frameLocked ); // crashed at profiler startup
		assert( m_zonesStack.isEmpty() );

		Double minimalTime = m_frameEnterTime - m_samplesLifetime;

		for( auto& group : m_groups )
		{
			for( auto& it : group )
			{
				while( it.samples.size() != 0 && it.samples[0].time < minimalTime )
				{
					it.samples.removeShift( 0 );
				}
			}		
		}

		m_frameLocked = false;
	}

	void EngineProfiler::enterZone( GroupId groupId, const Char* zoneName )
	{
		Zone zone;
		zone.enterTimeStamp = time::cycles64();
		zone.groupId = groupId;
		zone.metric = &findOrAddMetric( groupId, zoneName );

		m_zonesStack.push( zone );
	}

	void EngineProfiler::leaveZone()
	{
		Zone zone = m_zonesStack.pop();

		if( isGroupEnabled( zone.groupId ) )
		{
			if( zone.metric->samples.size() == 0 || zone.metric->samples.last().time != m_frameEnterTime )
			{
				zone.metric->samples.push( { m_frameEnterTime, time::elapsedMsFrom( zone.enterTimeStamp ) } );
			}
			else
			{
				fatal( L"Zone \"%s\" updated twice per frame", zone.metric->name );
			}
		}
	}

	void EngineProfiler::updateCounter( GroupId groupId, const Char* counterName, Double value )
	{
		if( isGroupEnabled( groupId ) )
		{
			Metric& metric = findOrAddMetric( groupId, counterName );

			if( metric.samples.size() == 0 || metric.samples.last().time != m_frameEnterTime )
			{
				metric.samples.push( { m_frameEnterTime, value } );
			}
			else
			{
				fatal( L"Counter \"%s\" updated twice per frame", counterName );
			}
		}
	}

	const Array<EngineProfiler::Group>& EngineProfiler::getMetrics() const
	{
		return m_groups;
	}

	void EngineProfiler::setSamplesLifetime( Double lifeTime )
	{
		assert( lifeTime > 0.0 );
		m_samplesLifetime = lifeTime;
	}

	EngineProfiler::Metric& EngineProfiler::findOrAddMetric( GroupId groupId, const Char* name )
	{
		for( auto& it : m_groups[groupId] )
		{
			if( it.name == name )
				return it;
		}

		Metric newMetric;
		newMetric.name = name;
		newMetric.color = hashing::murmur32( name, cstr::length( name ) * sizeof( Char ) );
		
		m_groups[groupId].push( newMetric );
		return m_groups[groupId].last();
	}

} // namespace profile
} // namespace flu