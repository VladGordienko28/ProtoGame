//-----------------------------------------------------------------------------
//	DxProfiler.cpp: A DirectX 11 profiler implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "DirectX11.h"

namespace flu
{
namespace dx11
{
	GPUProfiler::GPUProfiler( ID3D11Device* device, ID3D11DeviceContext* context )
	{
		assert( device );
		assert( context );

		m_device = device;
		m_context = context;
		m_frameCounter = 0;
		m_numInvalidFrames = MAX_CPU_FRAMES_AHEAD;
		m_groupId = profile::IProfiler::INVALID_GROUP_ID;

		// create main queries
		D3D11_QUERY_DESC queryDesc;
		mem::zero( &queryDesc, sizeof( D3D11_QUERY_DESC ) );
		queryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		queryDesc.MiscFlags = 0;

		HRESULT result;

		for( Int32 i = 0; i < MAX_CPU_FRAMES_AHEAD; ++i )
		{
			HRESULT result = m_device->CreateQuery( &queryDesc, &m_disjointQueries[i] );
			assert( SUCCEEDED( result ) );		
		}

		queryDesc.Query = D3D11_QUERY_TIMESTAMP;

		for( Int32 i = 0; i < MAX_CPU_FRAMES_AHEAD; ++i )
		{
			result = m_device->CreateQuery( &queryDesc, &m_frameStartQueries[i] );
			assert( SUCCEEDED( result ) );

			result = m_device->CreateQuery( &queryDesc, &m_frameEndQueries[i] );
			assert( SUCCEEDED( result ) );
		}
	}

	GPUProfiler::~GPUProfiler()
	{
		assert( m_zoneStack.isEmpty() );

		for( Int32 i = 0; i < MAX_CPU_FRAMES_AHEAD; ++i )
		{
			m_disjointQueries[i] = nullptr;
			m_frameStartQueries[i] = nullptr;
			m_frameEndQueries[i] = nullptr;
		}

		m_metrics.empty();
	}

	void GPUProfiler::setGroup( profile::IProfiler::GroupId groupId )
	{
		m_groupId = groupId;
	}

	void GPUProfiler::beginFrame()
	{
		assert( m_zoneStack.isEmpty() );
		assert( m_frameCounter >= 0 && m_frameCounter < MAX_CPU_FRAMES_AHEAD );

		m_context->Begin( m_disjointQueries[m_frameCounter] );
		m_context->End( m_frameStartQueries[m_frameCounter] );
	}

	void GPUProfiler::endFrame()
	{
		assert( m_zoneStack.isEmpty() );
		assert( m_frameCounter >= 0 && m_frameCounter < MAX_CPU_FRAMES_AHEAD );

		m_context->End( m_frameEndQueries[m_frameCounter] );
		m_context->End( m_disjointQueries[m_frameCounter] );

		// advance to next frame
		m_frameCounter = MAX_CPU_FRAMES_AHEAD_MASK & ( m_frameCounter + 1 );

		// skip some uninitialized frames at startup
		if( m_numInvalidFrames > 1 )
		{
			m_numInvalidFrames--;
			return;
		}

		// check next frame for data
		while( m_context->GetData( m_disjointQueries[m_frameCounter], nullptr, 0, 0 ) == S_FALSE )
		{
			// data is not ready yet, even MAX_CPU_FRAMES_AHEAD frames behind
			threading::sleep(1);
		}

		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT timestampDisjoint;
		if( m_context->GetData( m_disjointQueries[m_frameCounter], &timestampDisjoint, 
			sizeof( D3D11_QUERY_DATA_TIMESTAMP_DISJOINT ), 0 ) != S_OK )
		{
			debug( L"Dx11: Unable to retrieve timestamp disjoint query" );
			return;
		}
		if( timestampDisjoint.Disjoint )
		{
			// bad frame data
			debug( L"Dx11: Timestamp query disjoint" );
			return;
		}

		// Retrieve frame time
		{
			UInt64 startTimestamp;
			UInt64 endTimestamp;

			HRESULT result;

			if( ( result = m_context->GetData( m_frameStartQueries[m_frameCounter], &startTimestamp, sizeof( UInt64 ), 0 ) ) != S_OK ||
				( result = m_context->GetData( m_frameEndQueries[m_frameCounter], &endTimestamp, sizeof( UInt64 ), 0 ) ) != S_OK )
			{
				debug( L"Dx11: Unable to retrieve frame time query with error %d", result );
				return;
			}

			const Double frameTime = static_cast<Double>( endTimestamp - startTimestamp ) / 
				static_cast<Double>( timestampDisjoint.Frequency ) * 1000.0;

			if( m_groupId != profile::IProfiler::INVALID_GROUP_ID )
			{
				profile::getProfiler()->updateCounter( m_groupId, TXT("Total"), frameTime );
			}
		}

		// Retrieve user's metrics time
		for( const auto& metric : m_metrics )
		{
			UInt64 startTimestamp;
			UInt64 endTimestamp;

			HRESULT result;

			if( ( result = m_context->GetData( metric.startQueries[m_frameCounter], &startTimestamp, sizeof( UInt64 ), 0 ) ) != S_OK ||
				( result = m_context->GetData( metric.endtQueries[m_frameCounter], &endTimestamp, sizeof( UInt64 ), 0 ) ) != S_OK )
			{
				debug( L"Dx11: Unable to retrieve time query for \"%s\" with error %d", metric.name, result );
				return;
			}

			const Double metricTime = static_cast<Double>( endTimestamp - startTimestamp ) / 
				static_cast<Double>( timestampDisjoint.Frequency ) * 1000.0;

			if( m_groupId != profile::IProfiler::INVALID_GROUP_ID )
			{
				profile::getProfiler()->updateCounter( m_groupId, metric.name, metricTime );
			}
		}
	}

	void GPUProfiler::enterZone( const Char* zoneName )
	{
		Metric& metric = findOrAddMetric( zoneName );

		assert( metric.startQueries[m_frameCounter].hasObject() );
		m_context->End( metric.startQueries[m_frameCounter] );

		m_zoneStack.push( metric.index );
	}

	void GPUProfiler::leaveZone()
	{
		assert( !m_zoneStack.isEmpty() );
		Metric& metric = m_metrics[m_zoneStack.pop()];

		assert( metric.endtQueries[m_frameCounter].hasObject() );
		m_context->End( metric.endtQueries[m_frameCounter] );
	}

	GPUProfiler::Metric& GPUProfiler::findOrAddMetric( const Char* name )
	{
		assert( name );

		for( auto& it : m_metrics )
		{
			if( it.name == name )
			{
				return it;
			}
		}

		Metric metric;
		metric.index = m_metrics.size();
		metric.name = name;

		D3D11_QUERY_DESC queryDesc;
		mem::zero( &queryDesc, sizeof( D3D11_QUERY_DESC ) );
		queryDesc.Query = D3D11_QUERY_TIMESTAMP;
		queryDesc.MiscFlags = 0;

		HRESULT result;

		for( Int32 i = 0; i < MAX_CPU_FRAMES_AHEAD; ++i )
		{
			result = m_device->CreateQuery( &queryDesc, &metric.startQueries[i] );
			assert( SUCCEEDED( result ) );

			result = m_device->CreateQuery( &queryDesc, &metric.endtQueries[i] );
			assert( SUCCEEDED( result ) );
		}

		m_metrics.push( metric );
		return m_metrics.last();
	}
}
}