//-----------------------------------------------------------------------------
//	Polygon.h: A polygon utils functions
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace math
{
	/**
	 *	Return true, if polygon is convex
	 */
	extern Bool isConvexPoly( const Vector* verts, Int32 numVerts );

	/**
	 *	Return true, if given point is inside the convex poly,
	 *	otherwise return false.
	 */
	extern Bool isPointInsidePoly( const Vector& p, const Vector* verts, Int32 numVerts );

	/**
	 *	Returns true, if line ab intersect givin poly
	 *	By the way, hit location and normal are available too.
	 */
	extern Bool isLineIntersectPoly( const Vector& a, const Vector& b, const Vector* verts, 
		Int32 numVerts, Vector* hit = nullptr, Vector* hitNormal = nullptr );

	/**
	 *	Returns true and and point of intersection( hit ) if
	 *	segments A1A2 and B1B2 intersect, otherwise returns
	 *	false
	 */
	extern Bool isSegmentsIntersect( const Vector& a1, const Vector& a2, const Vector& b1, const Vector& b2, 
		Vector* hit = nullptr );

	/**
	 *	Return true, if point is being segment AB. Segment width is thresh
	 */
	extern Bool isPointOnSegment( const Vector& point, const Vector& a, const Vector& b, Float thresh = EPSILON );
} 
}