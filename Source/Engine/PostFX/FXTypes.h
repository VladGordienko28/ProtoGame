//-----------------------------------------------------------------------------
//	FXTypes.h: A helpers for post-processing
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace fx
{
	/**
	 *	A vignette
	 */
	struct Vignette
	{
	public:
		Float intensity = 0.f;
		Float innerRadius = 0.f;
		Float outerRadius = 1.f;
	};
}
}