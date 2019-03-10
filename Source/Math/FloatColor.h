//-----------------------------------------------------------------------------
//	FloatColor.h: A float point color
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace math
{
	/**
	 *	A float color
	 */
	class FloatColor
	{
	public:
		Float r;
		Float g;
		Float b;
		Float a;

		FloatColor()
			:	r( 0.f ), g( 0.f ), b( 0.f ), a( 0.f )
		{
		}

		FloatColor( Float inR, Float inG, Float inB, Float inA )
			:	r( inR ), g( inG ), b( inB ), a( inA )
		{
		}

		FloatColor( Color other )
			:	r( other.r / 255.f ),
				g( other.g / 255.f ),
				b( other.b / 255.f ),
				a( other.a / 255.f )
		{
		}

		Bool operator==( const FloatColor& other ) const
		{
			return r == other.r && g == other.g && b == other.b && a == other.a;
		}

		Bool operator!=( const FloatColor& other ) const
		{
			return r != other.r || g != other.g || b != other.b || a != other.a;
		}
	};
}
}