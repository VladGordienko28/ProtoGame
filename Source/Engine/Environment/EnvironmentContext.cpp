//-----------------------------------------------------------------------------
//	EnvironmentContext.cpp: Environment context implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine/Engine.h"

namespace flu
{
namespace envi
{
	static const Float DEFAULT_DAY_SPEED = 1.f;

	EnvironmentContext::EnvironmentContext()
		:	m_daySpeed( DEFAULT_DAY_SPEED ),
			m_transitionTime( -1.f ),
			m_transitionTimeRemain( -1.f )
	{
	}

	EnvironmentContext::~EnvironmentContext()
	{
	}

	void EnvironmentContext::tick( Float deltaTime )
	{
		if( m_transitionTimeRemain > 0.f )
		{
			m_transitionTimeRemain -= deltaTime;

			if( m_transitionTimeRemain <= 0.f )
			{
				m_currentDayTime = m_transitionalDayTime;
			}
		}
		else
		{
			m_currentDayTime.advance( deltaTime * m_daySpeed );
		}
	}

	TimeOfDay EnvironmentContext::getCurrentTime() const
	{/*
		if( m_transitionTimeRemain > 0.f )
		{
			return lerp( m_currentDayTime, m_transitionalDayTime, m_transitionTimeRemain / m_transitionTime );
		}
		else*/
		{
			return m_currentDayTime;
		}
	}

	void EnvironmentContext::setDaySpeed( Float daySpeed )
	{
		assert( daySpeed >= 0.f );
		m_daySpeed = daySpeed;
	}

	Float EnvironmentContext::getDaySpeed() const
	{
		return m_daySpeed;
	}


	void EnvironmentContext::setTimeOfDay( Int32 hour24, Int32 minute, Float transitionTime )
	{
		if( transitionTime > 0.f )
		{
			m_transitionTime = m_transitionTimeRemain = transitionTime;
			m_transitionalDayTime = TimeOfDay( hour24, minute );
		}
		else
		{
			m_currentDayTime = TimeOfDay( hour24, minute );
		}
	}

	void EnvironmentContext::syncWithComputerTime( Float transitionTime )
	{
		if( transitionTime > 0.f )
		{
			m_transitionTime = m_transitionTimeRemain = transitionTime;
			m_transitionalDayTime = GPlat->GetTimeOfDay();
		}
		else
		{
			m_currentDayTime = GPlat->GetTimeOfDay();
		}
	}

} // namespace envi
} // namespace flu