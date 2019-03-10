//-----------------------------------------------------------------------------
//	Environment.cpp: Environment implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine/Engine.h"

namespace flu
{
namespace envi
{
	Environment::Environment()
	{
		m_moon.m_zenithTime = TimeOfDay( 0, 0, 0 );
	}

	Environment::~Environment()
	{
	}

	void Environment::render( CCanvas* canvas, const EnvironmentContext* context )
	{
		m_sun.render( canvas, context->getCurrentTime() );
		m_moon.render( canvas, context->getCurrentTime() );
	}

	void Environment::renderInfo( CCanvas* canvas )
	{
		m_sun.renderOrbit( canvas );
		m_moon.renderOrbit( canvas );
	}

} // namespace envi
} // namespace flu