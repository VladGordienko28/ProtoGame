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
}