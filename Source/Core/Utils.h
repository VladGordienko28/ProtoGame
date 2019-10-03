//-----------------------------------------------------------------------------
//	Utils.h: Core utils
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
	template<class T> inline constexpr T alignValue( T value, T bound )
	{
		return (value + bound - 1) & ~(bound - 1);
	}

	template<class T, SizeT S> inline constexpr const SizeT arraySize( const T(&)[S] )
	{
		return S;
	}

	template<class T> inline constexpr T min( T a, T b )
	{
		return a < b ? a : b;
	}

	template<class T> inline constexpr T max( T a, T b )
	{
		return a > b ? a : b;
	}

	template<class T> inline constexpr T clamp( T v, T a, T b )
	{
		return v < a ? a : v > b ? b : v;
	}

	template<class T> inline constexpr Bool isPowerOfTwo( T i )
	{
		return ( i & ( i - 1 ) ) == 0;
	}

	template<class T> inline Bool inRange( T value, T a, T b )
	{
		return ( value >= a ) && ( value <= b );
	}

	template<class T> inline constexpr T sqr( T value )
	{
		return value * value;
	}

	template<class T> inline T abs( T value )
	{
		return ( value < T(0) ) ? -value : value;
	}

	template<class T> inline T lerp( T x, T y, Float alpha )
	{
		return x + ( y - x ) * alpha;
	}

	template<class T> inline void exchange( T& a, T& b )
	{
		T c = a;
		a = b;
		b = c;
	}

	template<class T> inline UInt32 intLog2( T a )
	{
		for( UInt32 i = sizeof(T) * 8 - 1; i >= 0; --i )
		{
			if( a & ( 1 << i ) )
			{
				return i;
			}
		}

		return 0;
	}
}