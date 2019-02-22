//-----------------------------------------------------------------------------
//	Rect.cpp: A rectangle implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Math.h"

namespace flu
{
namespace math
{
	Rect::Rect( const Vector* vertsList, SizeT numVerts )
	{
		assert( vertsList && numVerts > 0 );
		min = max = vertsList[0];

		for( SizeT i = 1; i < numVerts; ++i )
		{
			min.x = flu::min( min.x, vertsList[i].x );
			min.y = flu::min( min.y, vertsList[i].y );

			max.x = flu::max( max.x, vertsList[i].x );
			max.y = flu::max( max.y, vertsList[i].y );
		}
	}

	Bool Rect::isOverlap( const Rect& other ) const
	{
		if( min.x >= other.max.x || other.min.x >= max.x )
			return false;

		if( min.y >= other.max.y || other.min.y >= max.y )
			return false;
	
		return true;
	}

	Bool Rect::atBorder( const Vector& point, Float thresh ) const
	{
		Bool outside = ( point.x >= min.x - thresh )&&
			( point.x <= max.x + thresh )&&
			( point.y >= min.y - thresh )&&
			( point.y <= max.y + thresh );

		Bool inside = ( point.x >= min.x + thresh )&&
			( point.x <= max.x - thresh )&&
			( point.y >= min.y + thresh )&&
			( point.y <= max.y - thresh );

		return outside && !inside;	
	}

	Rect Rect::operator+( const Vector& point ) const
	{
		Rect result;

		result.min.x = flu::min( min.x, point.x );
		result.min.y = flu::min( min.y, point.y );
		result.max.x = flu::max( max.x, point.x );
		result.max.y = flu::max( max.y, point.y );

		return result;
	}

	Rect& Rect::operator+=( const Vector& point )
	{
		min.x = flu::min( min.x, point.x );
		min.y = flu::min( min.y, point.y );
		max.x = flu::max( max.x, point.x );
		max.y = flu::max( max.y, point.y );

		return *this;
	}
}
}