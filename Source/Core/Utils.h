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
}