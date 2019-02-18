//-----------------------------------------------------------------------------
//	Angle.h: An angle
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace math
{
	/**
	 *	An angular value in range [0..65535] instead of
	 *	[0..360] or [0..2pi]
	 */
	class Angle
	{
	public:
		Angle()
			:	value( 0 )
		{
		}

		Angle( Int32 inValue )
			:	value( inValue & MASK )
		{
		}

		Angle( Float inValue )
		{
			value = MASK & Int32( inValue / ( 2.f * PI ) * WINDING );
		}

		operator Int32() const
		{
			return value;
		}

		Angle operator+() const
		{
			return Angle( value );
		}

		Angle operator-() const
		{
			return Angle( WINDING - value );
		}

		Angle operator+( Angle other ) const
		{
			return Angle( value + other.value );
		}

		Angle operator-( Angle other ) const
		{
			return Angle( value - other.value );
		}

		Angle operator+=( Angle other )
		{
			value = MASK & ( value + other.value );
			return *this;
		}

		Angle operator-=( Angle other )
		{
			value = MASK & ( value - other.value );
			return *this;
		}

		Angle operator*( Float scale ) const
		{
			return Angle( Int32( value * scale ) );
		}

		Angle operator*=( Float scale )
		{
			value = MASK & Int32( value * scale );
			return *this;
		}

		Bool operator==( Angle other ) const
		{
			return value == other.value;
		}

		Bool operator!=( Angle other ) const
		{
			return value != other;
		}

		void snap( Int32 grid )
		{
			if( grid != 0 )
			{
				value = MASK & Int32( round( Float(value) / Float(grid) ) * grid );
			}
		}
		
		Float toRads() const
		{
			return value / Float( WINDING ) * ( 2.f * math::PI );
		}

		Float toDegs() const
		{
			return value / 182.0444f;
		}

		Float getCos() const;
		Float getSin() const;

		static Angle fromDegs( Float degs )
		{
			return floor( degs * 182.0444f ) & MASK;
		}

		static Angle fromRads( Float rads )
		{
			return floor( rads * ( WINDING / (2.f * PI) ) ) & MASK;
		}

		// legacy
		friend void Serialize( CSerializer& s, Angle& v )
		{
			Serialize( s, v.value );
		}

	private:
		static const Int32 MASK = 0xffff;
		static const Int32 WINDING = 0x10000;

		static const Int32 TABLE_BITS_BIAS = 3;
		static const Int32 TABLE_SIZE = 8192;
		static const Int32 TABLE_MASK = TABLE_SIZE - 1;

		Int32 value;
	};
}
}