//-----------------------------------------------------------------------------
//	Coords.cpp: Matrix relative stuff
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Math.h"

namespace flu
{
namespace math
{
	const Coords Coords::IDENTITY = Coords(
		{ 0.f, 0.f },
		{ 1.f, 0.f },
		{ 0.f, 1.f } );

	Coords Coords::operator<<( const Vector& localVec ) const
	{
		return Coords( origin + transformVectorBy( localVec, *this ), xAxis, yAxis );
	}

	Coords Coords::operator<<( Angle localRot ) const
	{
		Coords rotCoords( -localRot );
		return Coords( origin, transformVectorBy( xAxis, rotCoords ), transformVectorBy( yAxis, rotCoords ) );
	}

	Coords Coords::operator>>( const Vector& worldVec ) const
	{
		return Coords( origin + worldVec, xAxis, yAxis );
	}

	Coords Coords::operator>>( Angle worldRot ) const
	{
		Coords rotCoords( -worldRot );

		return Coords( transformPointBy( origin, rotCoords ), 
			transformVectorBy( xAxis, rotCoords ), transformVectorBy( yAxis, rotCoords ) );
	}

	Coords Coords::transpose() const
	{
		return Coords( -transformVectorBy( origin, *this ),
			Vector( xAxis.x, yAxis.x ),
			Vector( xAxis.y, yAxis.y ) );
	}
}
}