//-----------------------------------------------------------------------------
//	TimeOfDay.h: Game time
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace envi
{
	enum class EDayPeriod
	{
		Am,
		Pm
	};

	/**
	 *	This class represents the time of the day
	 */
	class TimeOfDay
	{
	public:
		TimeOfDay()
			:	m_seconds( 0.f )
		{
		}

		TimeOfDay( Int32 hour24, Int32 minute, Int32 second = 0 )
			:	m_seconds( second + ( ( hour24 ) * 60.f + minute ) * 60.f )
		{
		}

		void advance( Float deltaSecs )
		{
			m_seconds += deltaSecs;

			if( m_seconds >= SECS_PER_DAY )
				m_seconds -= SECS_PER_DAY;
		}

		Float toPercent() const
		{
			return m_seconds / SECS_PER_DAY;
		}

		void fromPercent( Float p )
		{
			m_seconds = p * SECS_PER_DAY;
		}

		Int32 getHour() const
		{
			return static_cast<Int32>( m_seconds ) / 60 / 60 % 12;
		}

		Int32 getMinute() const
		{
			return ( static_cast<Int32>( m_seconds ) % 3600 ) / 60;
		}

		Int32 getSecond() const
		{
			return static_cast<Int32>( m_seconds ) % 60;
		}

		EDayPeriod getPeriod() const
		{
			return m_seconds < SECS_PER_DAY / 2.f ? EDayPeriod::Am : EDayPeriod::Pm;
		}

		String toString() const
		{
			return String::format( L"%02d:%02d:%02d %s", 
				getHour(), getMinute(), getSecond(), getPeriod() == EDayPeriod::Am ? L"Am" : L"Pm" );
		}

		// legacy
		friend void Serialize( CSerializer& s, TimeOfDay& v )
		{
			Serialize( s, v.m_seconds );
		}

	private:
		static const constexpr Float SECS_PER_DAY = 60.f * 60.f * 24.f;

		Float m_seconds;
	};

} // namespace envi
} // namespace flu