//-----------------------------------------------------------------------------
//	EnvironmentContext.h: Environment context
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace envi
{
	class EnvironmentContext final
	{
	public:
		EnvironmentContext();
		~EnvironmentContext();

		void tick( Float deltaTime );
		TimeOfDay getCurrentTime() const;
		void setTimeOfDay( Int32 hour24, Int32 minute, Float transitionTime );

		void setDaySpeed( Float daySpeed );
		Float getDaySpeed() const;

		void syncWithComputerTime( Float transitionTime );

	private:
		Float m_daySpeed;

		Float m_transitionTimeRemain;
		Float m_transitionTime;

		TimeOfDay m_currentDayTime;
		TimeOfDay m_transitionalDayTime;
	};

} // namespace envi
} // namespace flu