//-----------------------------------------------------------------------------
//	Color.cpp: A color implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Math.h"

namespace flu
{
namespace math
{
	/**
	 *	Convert a RGB color value to the HSL value
	 *	where h( hue ), s( saturation ), l( lightness )
	 */
	void Color::rgb2hsl( Color color, UInt8& h, UInt8& s, UInt8& l )
	{
		Float r	= color.r / 256.f;
		Float g	= color.g / 256.f;
		Float b = color.b / 256.f;
	
		Float minValue = min( r, min( g, b ) );
		Float maxValue = max( r, max( g, b ) );

		if( r == g && g == b )
		{
			h = 0;
			s = 0;
			l = color.g;
		}
		else
		{
			Float fh, fs, fl;

			fl = ( minValue + maxValue ) * 0.5f;

			if( fl < 0.5f )
			{
				fs = ( maxValue - minValue ) / ( maxValue + minValue );
			}
			else
			{
				fs = ( maxValue - minValue ) / ( 2.f - maxValue - minValue );
			}

			if( r == maxValue )
			{
				fh	= ( g - b ) / ( maxValue - minValue );
			}
			else if( g == maxValue )
			{
				fh	= 2.f + ( b - r ) / ( maxValue - minValue );
			}
			else if( b == maxValue )
			{
				fh	= 4.f + ( r - g ) / ( maxValue - minValue );
			}

			fh /= 6.f;

			if( fh < 0.f )
			{
				fh += 1.f;
			}

			h = math::trunc( fh * 254.9f );
			s = math::trunc( fs * 254.9f );
			l = math::trunc( fl * 254.9f );
		}
	}
	

	/**
	 *	Convert a HSL color value to the RGB value
	 *	where h( hue ), s( saturation ), l( lightness )
	 */
	Color Color::hsl2rgb( UInt8 h, UInt8 s, UInt8 l, UInt8 a )
	{
		Float fh = h / 256.f;
		Float fs = s / 256.f;
		Float fl = l / 256.f;

		if( s != 0 )
		{
			Float fr, fg, fb, temp1, temp2;
			Float tempR, tempG, tempB;

			if( fl < 0.5f )
			{
				temp2 = fl * ( 1.f + fs );
			}
			else
			{
				temp2 = ( fl + fs ) - ( fl * fs );
			}

			temp1 = 2.f * fl - temp2;
			tempR = fh + 1.f / 3.f;

			if( tempR > 1.f ) tempR -= 1.f;
			tempG = fh;
			tempB = fh - 1.f / 3.f;
			if( tempB < 0.f ) tempB += 1.f;

			// Red channel.
			if( tempR < 1.f/6.f )
			{
				fr = temp1 + ( temp2 - temp1 ) * 6.f * tempR;
			}
			else if( tempR < 0.5f )
			{
				fr = temp2;
			}
			else if( tempR < 2.f/3.f )
			{
				fr = temp1 + ( temp2 - temp1 ) * ( ( 2.f / 3.f ) - tempR ) * 6.f;
			}
			else
			{
				fr = temp1;
			}

			// Green channel.
			if( tempG < 1.f/6.f )
			{
				fg = temp1 + ( temp2 - temp1 ) * 6.f * tempG;
			}
			else if( tempG < 0.5f )
			{
				fg = temp2;
			}
			else if( tempG < 2.f/3.f )
			{
				fg = temp1 + ( temp2 - temp1 ) * ( ( 2.f / 3.f ) - tempG ) * 6.f;
			}
			else
			{
				fg = temp1;
			}

			// Blue channel.
			if( tempB < 1.f/6.f )
			{
				fb = temp1 + ( temp2 - temp1 ) * 6.f * tempB;
			}
			else if( tempB < 0.5f )
			{
				fb = temp2;
			}
			else if( tempB < 2.f/3.f )
			{
				fb = temp1 + ( temp2 - temp1 ) * ( ( 2.f / 3.f ) - tempB ) * 6.f;
			}
			else
			{
				fb = temp1;
			}

			return Color( 
				math::trunc( fr * 254.9f ),
				math::trunc( fg * 254.9f ),
				math::trunc( fb * 254.9f ),
				a );
		}
		else
		{
			return Color( l, l, l, a );
		}
	}

	Color::Color( const FloatColor& other )
		:	r( clamp<Int32>( Int32( other.r * 255.f ), 0, 255 ) ),
			g( clamp<Int32>( Int32( other.g * 255.f ), 0, 255 ) ),
			b( clamp<Int32>( Int32( other.b * 255.f ), 0, 255 ) ),
			a( clamp<Int32>( Int32( other.a * 255.f ), 0, 255 ) )
	{
	}
		
} // namespace math
} // namespace flu