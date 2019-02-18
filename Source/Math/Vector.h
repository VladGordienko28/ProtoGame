//-----------------------------------------------------------------------------
//	Vector.h: A point or direction
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace math
{
	/**
	 *	A point or direction in 2d space
	 */
	class Vector
	{
	public:
		Float x;
		Float y;

		Vector()
		{
		}

		Vector( Float inX, Float inY )
			:	x( inX ), y( inY )
		{
		}

		Vector operator-() const
		{
			return Vector( -x, -y );
		}

		Vector operator+() const
		{
			return Vector( +x, +y );
		}

		Vector operator+( const Vector& v ) const
		{
			return Vector( x + v.x, y + v.y );
		}

		Vector operator-( const Vector& v ) const
		{
			return Vector( x - v.x, y - v.y );
		}

		Float operator*( const Vector& other ) const
		{
			return x * other.x + y * other.y;
		}

		Vector operator*( Float scale ) const
		{
			return Vector( x * scale, y * scale );
		}

		Float operator/( const Vector& other ) const
		{
			return x * other.y - y * other.x;
		}

		Vector operator/( Float scale ) const
		{
			return Vector( -scale * y, scale * x );
		}

		Vector operator+=( const Vector& other )
		{
			x += other.x;
			y += other.y;
			return *this;
		}

		Vector operator-=( const Vector& other )
		{
			x -= other.x;
			y -= other.y;
			return *this;
		}

		Vector operator*=( Float scale )
		{
			x *= scale;
			y *= scale;
			return *this;
		}

		Bool operator==( const Vector& other ) const
		{
			return x == other.x && y == other.y;
		}

		Bool operator!=( const Vector& other ) const
		{
			return x != other.x || y != other.y;
		}

		Float sizeSquared() const
		{
			return x * x + y * y;
		}

		Float size() const
		{
			return sqrt( x * x + y * y );
		}

		Vector normalized() const
		{
			Vector result = *this;
			result.normalize();
			return result;
		}

		void normalize()
		{
			Float scale = size();

			if( scale > math::EPSILON )
			{
				x /= scale;
				y /= scale;
			}
		}

		void snap( Float grid )
		{
			x = math::snap( grid, x );
			y = math::snap( grid, y );
		}

		Vector cross() const
		{
			return Vector( -y, x );
		}

		// legacy
		friend void Serialize( CSerializer& s, Vector& v )
		{
			Serialize( s, v.x );
			Serialize( s, v.y );
		}
	};

	inline Float distance( const Vector& a, const Vector& b )
	{
		return sqrt( sqr( a.x - b.x ) + sqr( a.y - b.y ) );
	}

	inline Float distanceSq( const Vector& a, const Vector& b )
	{
		return sqr( a.x - b.x ) + sqr( a.y - b.y );
	}

	inline Angle vectorToAngle( const Vector& vec )
	{
		return Angle( arcTan2( vec.y, vec.x ) );
	}

	inline Vector angleToVector( Angle ang )
	{
		return Vector( ang.getCos(), ang.getSin() );
	}

	inline Bool isPointsAreNear( const Vector& a, const Vector& b, Float testDistance )
	{
		return ( abs( a.x - b.x ) <= testDistance ) && ( abs( a.y - b.y ) <= testDistance );
	}

	inline Float pointLineDistance( const Vector& point, const Vector& linePoint, const Vector& lineNormal )
	{
		return ( point - linePoint ) * lineNormal;
	}
}
}