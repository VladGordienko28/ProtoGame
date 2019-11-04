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

	inline Float fMod( Float x, Float y )
	{
		return fmodf( x, y );
	}

	inline Int32 round( Float f )
	{
		return static_cast<Int32>( roundf( f ) );
	}

	inline Int32 floor( Float f )
	{
		return static_cast<Int32>( floorf( f ) );
	}

	inline Int32 ceil( Float f )
	{
		return static_cast<Int32>( ceilf( f ) );
	}

	inline Int32 trunc( Float f )
	{
		return static_cast<Int32>( truncf( f ) );
	}

	inline Float frac( Float f )
	{
		return f - floor( f );
	}

	inline Bool isEqual( Float a, Float b, Float epsilon )
	{
		return abs( a - b ) < epsilon;
	}

	inline Float snap( Float grid, Float value )
	{
		return grid != 0.f ? round( value / grid ) * grid : value;
	}
}
}