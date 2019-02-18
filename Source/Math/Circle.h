//-----------------------------------------------------------------------------
//	Circle.h: A circle
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace math
{
	/**
	 *	A Circle
	 */
	class Circle
	{
	public:
		Vector center;
		Float radius;

		Circle()
		{
		}

		Circle( const Vector& inCenter, Float inRadius )
			:	center( inCenter ),
				radius( inRadius )
		{
		}

		Bool operator==( const Circle& other ) const
		{
			return center == other.center && radius == other.radius;
		}

		Bool operator!=( const Circle& other ) const
		{
			return center != other.center || radius != other.radius;
		}

		operator Bool() const
		{
			return radius > 0.f;
		}

		Float area() const
		{
			return PI * radius * radius;
		}

		Bool isInside( const Vector& point ) const
		{
			return distance( center, point ) <= radius;
		}

		Bool atBorder( const Vector& point, Float thresh ) const
		{
			return inRange( distance( point, center ), radius - thresh, radius + thresh );
		}

		Bool isOverlap( const Circle& other ) const
		{
			return distance( center, other.center ) <= ( radius + other.radius );
		}

		// legacy
		friend void Serialize( CSerializer& s, Circle& v )
		{
			Serialize( s, v.center );
			Serialize( s, v.radius );
		}
	};
}
}