//-----------------------------------------------------------------------------
//	Coords.h: A coords system
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace math
{
	/**
	 *	A coords system matrix. Technically it a 
	 *	coords system basis.
	 */
	class Coords
	{
	public:
		Vector origin;
		Vector xAxis;
		Vector yAxis;

		Coords()
			:	origin( 0.f, 0.f ),
				xAxis( 1.f, 0.f ),
				yAxis( 0.f, 1.f )
		{
		}

		Coords( const Vector& inOrigin )
			:	origin( inOrigin ),
				xAxis( 1.f, 0.f ),
				yAxis( 0.f, 1.f )
		{
		}

		Coords( const Vector& inOrigin, const Vector& inX, const Vector& inY )
			:	origin( inOrigin ),
				xAxis( inX ),
				yAxis( inY )
		{
		}

		Coords( const Vector& inOrigin, const Vector& inX )
			:	origin( inOrigin ),
				xAxis( inX ),
				yAxis( inX.cross() )
		{
		}

		Coords( Angle rot )
			:	origin( 0.f, 0.f )
		{
			xAxis = { rot.getCos(), rot.getSin() };
			yAxis = xAxis.cross();
		}

		Coords( const Vector& inOrigin, Angle rot )
			:	origin( inOrigin )
		{
			xAxis = { rot.getCos(), rot.getSin() };
			yAxis = xAxis.cross();
		}

		Bool operator==( const Coords& other ) const
		{
			return origin == other.origin && xAxis == other.xAxis &&
				yAxis == other.yAxis;
		}

		Bool operator!=( const Coords& other ) const
		{
			return origin != other.origin || xAxis != other.xAxis ||
				yAxis != other.yAxis;
		}

		Coords operator<<( const Vector& localVec ) const;
		Coords operator<<( Angle localRot ) const;
		Coords operator>>( const Vector& worldVec ) const;
		Coords operator>>( Angle worldRot ) const;

		Coords transpose() const;

		static const Coords IDENTITY;

		// legacy
		friend void Serialize( CSerializer& s, Coords& v )
		{
			Serialize( s, v.origin );
			Serialize( s, v.xAxis );
			Serialize( s, v.yAxis );
		}
	};

	/**
	 *	Transform vector to the other coords system
	 */
	inline Vector transformVectorBy( const Vector& vec, const Coords& coords )
	{
		return Vector( vec * coords.xAxis, vec * coords.yAxis );
	}

	/**
	 *	Transform point to the other coords system
	 */
	inline Vector transformPointBy( const Vector& point, const Coords& coords )
	{
		Vector vec = point - coords.origin;
		return Vector( vec * coords.xAxis, vec * coords.yAxis );
	}
}
}