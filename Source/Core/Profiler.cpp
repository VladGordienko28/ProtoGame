//-----------------------------------------------------------------------------
//	Profiler.cpp: Flu basic profiling system
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Core.h"

namespace flu
{
namespace profile
{
	/**
	 *	An useless profiler
	 */
	class NullProfiler: public IProfiler
	{
	public:
		NullProfiler()
		{
		}

		~NullProfiler()
		{
		}

		void beginFrame() override
		{
		}

		void endFrame() override
		{
		}

		void enterZone( GroupId groupId, const Char* zoneName ) override
		{
		}

		void leaveZone() override
		{
		}

		void updateCounter( GroupId groupId, const Char* counterName, Double value ) override
		{
		}
	};

	static NullProfiler g_nullProfiler;
	static IProfiler* g_activeProfiler = &g_nullProfiler;

	IProfiler* getProfiler()
	{
		return g_activeProfiler;
	}

	void setProfiler( IProfiler* newProfiler )
	{
		assert( newProfiler );
		g_activeProfiler = newProfiler;
	}

	void setDefaultProfiler()
	{
		g_activeProfiler = &g_nullProfiler;
	}

} // namespace profile
} // namespace flu