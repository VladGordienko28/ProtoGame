//-----------------------------------------------------------------------------
//	Profiler.h: An abstract profiler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace profile
{
	/**
	 *	An abstraact profiler interface
	 */
	class IProfiler
	{
	public:
		using GroupId = Int32;

		virtual ~IProfiler()
		{
		}

		virtual void beginFrame() = 0;
		virtual void endFrame() = 0;

		virtual void enterZone( GroupId groupId, const Char* zoneName ) = 0;
		virtual void leaveZone() = 0;

		virtual void updateCounter( GroupId groupId, const Char* counterName, Double value ) = 0;
	};

	// global accessors
	extern IProfiler* getProfiler();
	extern void setProfiler( IProfiler* newProfiler );
	extern void setDefaultProfiler();
	extern Bool isDefaultProfiler();

	/**
	 *	A wrapper class for zone tracking
	 */
	class ZoneTracker final
	{
	public:
		ZoneTracker( IProfiler::GroupId groupId, const Char* zoneName )
		{
			getProfiler()->enterZone( groupId, zoneName );
		}

		~ZoneTracker()
		{
			getProfiler()->leaveZone();
		}
	};

} // namespace profile
} // namespace flu

/**
  *	Profiler macro
  */
#if FLU_ENABLE_PROFILER

	#define profile_zone( groupId, zoneName ) flu::profile::ZoneTracker zoneTracker( \
		static_cast<flu::profile::IProfiler::GroupId>( groupId ), L#zoneName );

	#define profile_counter( groupId, counterName, value ) flu::profile::getProfiler()->updateCounter( \
		static_cast<flu::profile::IProfiler::GroupId>( groupId ), L#counterName, value );

	#define profile_begin_frame() flu::profile::getProfiler()->beginFrame();

	#define profile_end_frame() flu::profile::getProfiler()->endFrame();

#else

	#define profile_zone( groupId, zoneName ) 
	#define profile_counter( groupId, counterName, value )
	#define profile_begin_frame()
	#define profile_end_frame()

#endif