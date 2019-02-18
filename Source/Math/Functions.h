//-----------------------------------------------------------------------------
//	Functions.h: Common math functions
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace math
{
	inline Float sqrt( Float f )
	{
		return sqrtf( f );
	}

	inline Float pow( Float base, Float f )
	{
		return powf( base, f );
	}

	inline Float ln( Float f )
	{
		return logf( f );
	}

	inline Float sin( Float f )
	{
		return sinf( f );
	}

	inline Float cos( Float f )
	{
		return cosf( f );
	}

	inline Float arcTan( Float x )
	{
		return atanf( x );
	}

	inline Float arcTan2( Float y, Float x )
	{
		return atan2f( y, x );
	}

	inline Int32 round( Float f )
	{
		return roundf( f );
	}

	inline Int32 floor( Float f )
	{
		return floorf( f );
	}

	inline Int32 ceil( Float f )
	{
		return ceilf( f );
	}

	inline Int32 trunc( Float f )
	{
		return truncf( f );
	}

	inline Float frac( Float f )
	{
		return f - floor( f );
	}

	inline Float snap( Float grid, Float value )
	{
		return grid != 0.f ? round( value / grid ) * grid : value;
	}
}
}