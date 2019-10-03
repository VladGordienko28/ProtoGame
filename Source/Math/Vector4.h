//-----------------------------------------------------------------------------
//	Vector3.h: A 4 floats container
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace math
{
	/**
	 *	A four floats
	 */
	class Vector4
	{
	public:
		Float x;
		Float y;
		Float z;
		Float w;

		Vector4()
		{
		}

		Vector4( Float inX, Float inY, Float inZ, Float inW )
			:	x( inX ), y( inY ), z( inZ ), w( inW )
		{
		}

		Vector4( const Vector& inXY, const Vector& inZW )
			:	x( inXY.x ), y( inXY.y ), z( inZW.x ), w( inZW.y )
		{
		}

		Vector4( const Vector& inXY, Float inZ, Float inW )
			:	x( inXY.x ), y( inXY.y ), z( inZ ), w( inW )
		{
		}

		Bool operator==( const Vector4& other ) const
		{
			return x == other.x && y == other.y && z == other.z && w == other.w;
		}

		Bool operator!=( const Vector4& other ) const
		{
			return x != other.x || y != other.y || z != other.z || w != other.w;
		}
	};
}
}