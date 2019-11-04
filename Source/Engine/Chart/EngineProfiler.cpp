//-----------------------------------------------------------------------------
//	EngineProfiler.cpp: Profiler implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine/Engine.h"

namespace flu 
{
namespace profile
{
	EngineProfiler::EngineProfiler()
		:	m_frameEnterTime( 0.0 ),
			m_samplesLifetime( 1.0 ),
			m_frameLocked( false ),
			m_selectedGroup( EGroup::Common )
	{
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

	void EngineProfiler::enterZone( EGroup group, const Char* zoneName )
	{
		Zone zone;
		zone.enterTimeStamp = time::cycles64();
		zone.groupId = group;
		zone.metric = &findOrAddMetric( group, zoneName );

		m_zonesStack.push( zone );
	}

	void EngineProfiler::leaveZone()
	{
		Zone zone = m_zonesStack.pop();

		if( zone.groupId == m_selectedGroup )
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

	void EngineProfiler::updateCounter( EGroup group, const Char* counterName, Double value )
	{
		if( group == m_selectedGroup )
		{
			Metric& metric = findOrAddMetric( group, counterName );

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

	void EngineProfiler::selectGroup( Int32 groupId )
	{
		m_selectedGroup = static_cast<EGroup>( clamp( groupId, 0, (Int32)EGroup::MAX ) );
	}

	void EngineProfiler::selectNextGroup()
	{
		m_selectedGroup = static_cast<EGroup>( min<Int32>( (Int32)m_selectedGroup + 1, m_groups.size() - 1 ) );
	}

	void EngineProfiler::selectPrevGroup()
	{
		m_selectedGroup = static_cast<EGroup>( max<Int32>( (Int32)m_selectedGroup - 1, 0 ) );
	}

	const EngineProfiler::Groups& EngineProfiler::getMetrics() const
	{
		return m_groups;
	}

	void EngineProfiler::setSamplesLifetime( Double lifeTime )
	{
		assert( lifeTime > 0.0 );
		m_samplesLifetime = lifeTime;
	}

	EngineProfiler::Metric& EngineProfiler::findOrAddMetric( EGroup group, const Char* name )
	{
		for( auto& it : m_groups[static_cast<Int32>( group )] )
		{
			if( it.name == name )
				return it;
		}

		Metric newMetric;
		newMetric.name = name;
		newMetric.color = hashing::murmur32( name, cstr::length( name ) * sizeof( Char ) );
		
		m_groups[static_cast<Int32>( group )].push( newMetric );
		return m_groups[static_cast<Int32>( group )].last();
	}

} // namespace profile
} // namespace flu