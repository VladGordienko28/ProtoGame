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

		void enterZone( EGroup group, const Char* zoneName ) override
		{
		}

		void leaveZone() override
		{
		}

		void updateCounter( EGroup group, const Char* counterName, Double value ) override
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

	Bool isDefaultProfiler()
	{
		return g_activeProfiler == &g_nullProfiler;
	}

	const Char* getGroupName( EGroup group )
	{
		switch( group )
		{
			case EGroup::Common:		return TXT("Common");
			case EGroup::Entity:		return TXT("Entity");
			case EGroup::UI:			return TXT("UI");
			case EGroup::RAM_Memory:	return TXT("RAM Memory");
			case EGroup::GPU_Memory:	return TXT("GPU Memory");
			case EGroup::Draw_Calls:	return TXT("Draw Calls");
			case EGroup::Render:		return TXT("Render");
			default:					return TXT("Unknown");
		}
	}

} // namespace profile
} // namespace flu