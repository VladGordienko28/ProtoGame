//-----------------------------------------------------------------------------
//	Utils.h: Core utils
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

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

	template<class T> inline T sqr( T value )
	{
		return value * value;
	}

	template<class T> inline T abs( T value )
	{
		return ( value < T(0) ) ? -value : value;
	}

	template<class T> inline void exchange( T& a, T& b )
	{
		T c = a;
		a = b;
		b = c;
	}
}