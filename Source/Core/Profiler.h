//-----------------------------------------------------------------------------
//	Profiler.h: An abstract profiler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace profile
{
	/**
	 *	A profiler groups
	 */
	enum class EGroup
	{
		Common,
		Entity,
		UI,
		RAM_Memory,
		GPU_Memory,
		Draw_Calls,
		Render, // todo: clarify metric
		MAX
	};

	/**
	 *	An abstraact profiler interface
	 */
	class IProfiler
	{
	public:
		struct Sample
		{
		public:
			Double time = 0.0;
			Double value = 0.0;
		};

		using Samples = Array<Sample>;

		virtual ~IProfiler()
		{
		}

		virtual void beginFrame() = 0;
		virtual void endFrame() = 0;

		virtual void enterZone( EGroup group, const Char* zoneName ) = 0;
		virtual void leaveZone() = 0;

		virtual void updateCounter( EGroup group, const Char* counterName, Double value ) = 0;
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
		ZoneTracker( EGroup group, const Char* zoneName )
		{
			getProfiler()->enterZone( group, zoneName );
		}

		~ZoneTracker()
		{
			getProfiler()->leaveZone();
		}
	};

	extern const Char* getGroupName( EGroup group );

} // namespace profile
} // namespace flu

/**
  *	Profiler macro
  */
#if FLU_ENABLE_PROFILER

	#define profile_zone( group, zoneName ) flu::profile::ZoneTracker zoneTracker( \
		flu::profile::EGroup::##group, L#zoneName );

	#define profile_counter( group, counterName, value ) flu::profile::getProfiler()->updateCounter( \
		flu::profile::EGroup::##group, L#counterName, value );

	#define profile_begin_frame() flu::profile::getProfiler()->beginFrame();

	#define profile_end_frame() flu::profile::getProfiler()->endFrame();

#else

	#define profile_zone( group, zoneName )
	#define profile_counter( group, counterName, value )
	#define profile_begin_frame()
	#define profile_end_frame()

#endif