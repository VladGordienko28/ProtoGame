//-----------------------------------------------------------------------------
//	Rect.h: A rectangle
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace math
{
	/**
	 *	An aabb rectangle
	 */
	class Rect
	{
	public:
		Vector min;
		Vector max;

		Rect()
		{
		}

		Rect( const Vector& center, Float side )
		{
			Float halfSide = 0.5f * side;
			min = { center.x - halfSide, center.y - halfSide };
			max = { center.x + halfSide, center.y + halfSide };
		}

		Rect( const Vector& center, Float xSide, Float ySide )
		{
			Float xHalf = 0.5f * xSide;
			Float yHalf = 0.5f * ySide;
			min = { center.x - xHalf, center.y - yHalf };
			max = { center.x + xHalf, center.y + yHalf };
		}

		Rect( const Vector* vertsList, SizeT numVerts );

		Bool operator==( const Rect& other ) const
		{
			return min == other.min && max == other.max;
		}

		Bool operator!=( const Rect& other ) const
		{
			return min != other.min || max != other.max;
		}

		operator Bool() const
		{
			return min != max;
		}

		Rect operator+( const Vector& point ) const;
		Rect& operator+=( const Vector& point );

		Vector center() const
		{
			return ( min + max ) * 0.5f;
		}

		Vector size() const
		{
			return max - min;
		}
		
		Float sizeX() const
		{
			return max.x - min.x;
		}

		Float sizeY() const
		{
			return max.y - min.y;
		}

		Float area() const
		{
			Vector diagonal = max - min;
			return diagonal.x * diagonal.y;
		}

		Bool isInside( const Vector& p ) const
		{
			return p.x >= min.x && p.x <= max.x &&
				p.y >= min.y && p.y <= max.y;
		}

		Bool isOverlap( const Rect& other ) const;
		Bool atBorder( const Vector& point, Float thresh ) const;

		// legacy
		friend void Serialize( CSerializer& s, Rect& v )
		{
			Serialize( s, v.min );
			Serialize( s, v.max );
		}
	};
}
}