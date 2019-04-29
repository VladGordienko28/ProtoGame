//-----------------------------------------------------------------------------
//	Color.h: An UInt8 RGBA color
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace math
{
	/**
	 *	A color
	 */
	class Color
	{
	public:
		union
		{
			struct{ UInt8 r; UInt8 g; UInt8 b; UInt8 a; };
			UInt32 d;
		};

		Color()
			:	d( 0 )
		{
		}

		Color( UInt32 inD )
			:	d( inD )
		{
		}

		Color( UInt8 inR, UInt8 inG, UInt8 inB, UInt8 inA )
			:	r( inR ), g( inG ), b( inB ), a( inA )
		{
		}

		Color( const class FloatColor& other );

		Color operator-() const
		{
			return Color( ~d );
		}

		Color operator+() const
		{
			return Color( d );
		}

		Color operator+( Color other ) const
		{
			return Color( 
				min( 255, Int32( r ) + Int32( other.r ) ),
				min( 255, Int32( g ) + Int32( other.g ) ),
				min( 255, Int32( b ) + Int32( other.b ) ),
				min( 255, Int32( a ) + Int32( other.a ) ) );
		}

		Color operator-( Color other ) const
		{
			return Color( 
				max( 0, Int32( r ) - Int32( other.r ) ),
				max( 0, Int32( g ) - Int32( other.g ) ),
				max( 0, Int32( b ) - Int32( other.b ) ),
				max( 0, Int32( a ) - Int32( other.a ) ) );
		}

		Color operator*( UInt8 brightness ) const
		{
			return Color(
				( Int32(r) * Int32(brightness) ) >> 8,
				( Int32(g) * Int32(brightness) ) >> 8,
				( Int32(b) * Int32(brightness) ) >> 8,
				a );
		}

		Color operator*( Color other ) const
		{
			return Color(
				( Int32(r) * Int32(other.r) ) >> 8,
				( Int32(g) * Int32(other.g) ) >> 8,
				( Int32(b) * Int32(other.b) ) >> 8,
				a );
		}

		Color operator*( Float scale ) const
		{
			return Color(
				clamp( floor( r * scale ), 0, 255 ),
				clamp( floor( g * scale ), 0, 255 ),
				clamp( floor( b * scale ), 0, 255 ),
				a );
		}

		Bool operator==( Color other ) const
		{
			return d == other.d;
		}

		Bool operator!=( Color other ) const
		{
			return d != other.d;
		}

		Color operator+=( Color other )
		{
			r = max( 255, Int32( r ) + Int32( other.r ) );
			g = max( 255, Int32( g ) + Int32( other.g ) );
			b = max( 255, Int32( b ) + Int32( other.b ) );
			a = max( 255, Int32( a ) + Int32( other.a ) );
			return *this;
		}

		Color operator-=( Color other )
		{
			r = max( 0, Int32( r ) - Int32( other.r ) );
			g = max( 0, Int32( g ) - Int32( other.g ) );
			b = max( 0, Int32( b ) - Int32( other.b ) );
			a = max( 0, Int32( a ) - Int32( other.a ) );
			return *this;
		}

		Color operator*=( Color other )
		{
			r = ( Int32( r ) * Int32( other.r ) ) >> 8;
			g = ( Int32( g ) * Int32( other.g ) ) >> 8;
			b = ( Int32( b ) * Int32( other.b ) ) >> 8;
			a = ( Int32( a ) * Int32( other.a ) ) >> 8;
			return *this;
		}

		Color operator*=( Float brightness )
		{
			r = clamp( math::floor( r * brightness ), 0, 255 );
			g = clamp( math::floor( g * brightness ), 0, 255 );
			b = clamp( math::floor( b * brightness ), 0, 255 );
			a = a;
			return *this;
		}

		Color operator*=( UInt8 Brightness )
		{
			r = ( Int32( r ) * Int32( Brightness ) ) >> 8;
			g = ( Int32( g ) * Int32( Brightness ) ) >> 8;
			b = ( Int32( b ) * Int32( Brightness ) ) >> 8;
			a = a;
			return *this;
		}

		static void rgb2hsl( Color color, UInt8& h, UInt8& s, UInt8& l );
		static Color hsl2rgb( UInt8 h, UInt8 s, UInt8 l, UInt8 a = 255 );

		// legacy
		friend void Serialize( CSerializer& s, Color& v )
		{
			Serialize( s, v.d );
		}
	};
}

inline math::Color lerp( math::Color x, math::Color y, Float alpha )
{
	return math::Color(
		static_cast<Int32>( Float( x.r ) + ( Float( y.r ) - Float( x.r ) ) * alpha ),
		static_cast<Int32>( Float( x.g ) + ( Float( y.g ) - Float( x.g ) ) * alpha ),
		static_cast<Int32>( Float( x.b ) + ( Float( y.b ) - Float( x.b ) ) * alpha ),
		static_cast<Int32>( Float( x.a ) + ( Float( y.a ) - Float( x.a ) ) * alpha ) );
}
}