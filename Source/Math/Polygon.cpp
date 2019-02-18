//-----------------------------------------------------------------------------
//	Polygon.cpp: A polygon utils functions
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Math.h"

namespace flu
{
namespace math
{
	Bool isConvexPoly( const Vector* verts, Int32 numVerts )
	{
		assert( verts && numVerts >= 3 );
		Vector edge1, edge2;

		edge1 = verts[0] - verts[numVerts - 1];
		edge2 = verts[numVerts - 1] - verts[numVerts - 2];

		if( edge1 / edge2 < 0.f )
		{
			return false;
		}

		for( Int32 i = 0; i < numVerts - 1; ++i )
		{
			edge2 = verts[i + 1] - verts[i];

			if( edge2 / edge1 < 0.f )
			{
				return false;
			}

			edge1 = edge2;
		}

		return true;
	}

	Bool isPointInsidePoly( const Vector& p, const Vector* verts, Int32 numVerts )
	{
		assert( verts && numVerts >= 3 );
		Vector v1, v2;

		v1 = verts[numVerts - 1];

		for( Int32 i = 0; i < numVerts; ++i )
		{
			v2 = verts[i];

			if( ( v2 - v1 ) / ( p - v2 ) > 0.f )
			{
				return false;
			}

			v1 = v2;
		}

		return true;
	}

	Bool isLineIntersectPoly( const Vector& a, const Vector& b, const Vector* verts, 
		Int32 numVerts, Vector* hit, Vector* hitNormal )
	{
		assert( verts && numVerts > 2 );
		Bool hitFound = false;
		Float testDistance, bestDistance = 999999.f;
		Vector resultPoint, resultNormal;
		Vector p2, p1 = verts[numVerts - 1];

		for( Int32 i = 0; i < numVerts; ++i )
		{
			Vector testPoint;
			p2 = verts[i];

			if( isSegmentsIntersect( a, b, p1, p2, &testPoint ) )
			{
				hitFound = true;
				testDistance = (a - testPoint).sizeSquared();

				if( testDistance < bestDistance )
				{
					resultPoint = testPoint;
					resultNormal = ( p2 - p1 ).cross();
					bestDistance = testDistance;
				}
			}

			p1 = p2;
		}

		if( hitFound )
		{
			if( hit ) *hit = resultPoint;
			if( hitNormal ) *hitNormal = resultNormal.normalized();
			return true;
		}
		else
		{
			return false;
		}
	}

	Bool isPointOnSegment( const Vector& point, const Vector& a, const Vector& b, Float thresh )
	{
		// not cool :(
		Vector dir = b - a;
		Float length = dir.size();
		dir *= 1.f / length;

		Coords lineSpace = Coords( a, dir );
		Vector t = transformPointBy( point, lineSpace );

		return ( t.x >= 0.f ) && ( t.x <= length ) && ( abs( t.y ) < thresh );
	}

	Bool isSegmentsIntersect( const Vector& a1, const Vector& a2, const Vector& b1, const Vector& b2, Vector* hit )
	{
		Vector aDir = a2 - a1;
		Vector bDir = b2 - b1;
		Vector aCross = aDir.cross();
		Vector bCross = bDir.cross();

		Float aDot = a1 * aCross;
		Float bDot = b1 * bCross;

		Float timeA1 = a1 * bCross - bDot;
		Float timeA2 = a2 * bCross - bDot;
		Float timeB1 = b1 * aCross - aDot;
		Float timeB2 = b2 * aCross - aDot;

		if( timeA1 * timeA2 >= 0.f || timeB1 * timeB2 >= 0.f )
		{
			return false;
		}
		else
		{
			Float time = timeA1 / ( timeA1 - timeA2 );

			if( hit )
			{
				*hit = a1 + aDir * time;
			}

			return true;
		}
	}

} // namespace math
} // namespace flu