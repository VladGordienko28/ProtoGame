//-----------------------------------------------------------------------------
//	Satellite.cpp: Orbital object implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine/Engine.h"

namespace flu
{
namespace envi
{
	static const Float DEFAULT_SATELLITE_SIZE = 8.f;
	static const TimeOfDay DEFAULT_ZENITH_TIME = TimeOfDay( 12, 0, 0 ); // midday
	static const Float DEFAULT_SATELLITE_ORBIT_SIZE = 10.f;

	Satellite::Satellite()
		:	m_bitmap( nullptr ),
			m_size( DEFAULT_SATELLITE_SIZE, DEFAULT_SATELLITE_SIZE ),
			m_zenithTime( DEFAULT_ZENITH_TIME ),
			m_orbitCenter( 0.f, 0.f ),
			m_orbitWidth( DEFAULT_SATELLITE_ORBIT_SIZE ),
			m_orbitHeight( DEFAULT_SATELLITE_ORBIT_SIZE )
	{
	}

	Satellite::~Satellite()
	{
	}

	void Satellite::renderOrbit( CCanvas* canvas )
	{
		const math::Vector centroid = canvas->View.Coords.origin + m_orbitCenter;

		canvas->DrawEllipse( centroid, m_orbitWidth, m_orbitHeight, 
			math::colors::ANTIQUE_WHITE, false, 64 );

		canvas->DrawLineStar( centroid, 0, 2.f, math::colors::ANTIQUE_WHITE, false );
	}

	void Satellite::render( CCanvas* canvas, TimeOfDay dayTime )
	{
		const Float phase = 2.f * math::PI * ( dayTime.toPercent() - m_zenithTime.toPercent() );
		const math::Vector centroid = canvas->View.Coords.origin + m_orbitCenter;
		const math::Vector position = math::Vector( m_orbitWidth * math::sin( phase ), m_orbitHeight * math::cos( phase ) ) + centroid;

		// todo: add visibility check

		TRenderRect rr;
		rr.Bounds = math::Rect( position, m_size.x, m_size.y );
		rr.Color = math::colors::WHITE;
		rr.Flags = POLY_Unlit;
		rr.Rotation = 0;
		rr.Texture = m_bitmap;
		rr.TexCoords.min = { 0.f, 0.f };
		rr.TexCoords.max = { 1.f, 1.f };

		canvas->DrawRect( rr );
	}

	void Serialize( CSerializer& s, Satellite& v )
	{
		Serialize( s, v.m_bitmap );
		Serialize( s, v.m_size );
		Serialize( s, v.m_zenithTime );
		Serialize( s, v.m_orbitCenter );
		Serialize( s, v.m_orbitWidth );
		Serialize( s, v.m_orbitHeight );
	}

} // namespace envi
} // namespace flu