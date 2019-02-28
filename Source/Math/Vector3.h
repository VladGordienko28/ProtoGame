//-----------------------------------------------------------------------------
//	Vector3.h: A 3 floats container
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace math
{
	/**
	 *	A three floats
	 */
	class Vector3
	{
	public:
		Float x;
		Float y;
		Float z;

		Vector3()
		{
		}

		Vector3( Float inX, Float inY, Float inZ )
			:	x( inX ), y( inY ), z( inZ )
		{
		}

		Vector3( const Vector& inVector, Float inZ )
			:	x( inVector.x ), y( inVector.y ), z( inZ )
		{
		}

		Bool operator==( const Vector3& other ) const
		{
			return x == other.x && y == other.y && z == other.z;
		}

		Bool operator!=( const Vector3& other ) const
		{
			return x != other.x || y != other.y || z != other.z;
		}
	};
}
}