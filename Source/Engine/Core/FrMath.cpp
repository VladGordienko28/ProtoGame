/*=============================================================================
	FrMath.cpp: Math library.
	Created by Vlad Gordienko, Jun. 2016.
	Refactoring Jan. 2018.
=============================================================================*/

#include "..\Engine.h"

/*-----------------------------------------------------------------------------
    Math functions.
-----------------------------------------------------------------------------*/

//
// Return the power of size.
// For example 256->8, 32->5...
//
UInt32 IntLog2( UInt32 A )
{
	for( Int32 i=31; i>=0; i-- )
		if( A & ( 1 << i ) )
			return i;
	return 0;
}


//
// Wrap a value to the range.
//
Float Wrap( Float V, Float Min, Float Max )
{
	Float Size = Max - Min;

	if( V > 0.f )
		return Min + V - Size * math::trunc( V / Size );
	else
		return Max + V - Size * math::trunc( V / Size );
}

//
// Angles linear interpolation.
//
math::Angle AngleLerp( math::Angle AFrom, math::Angle ATo, Float Alpha, Bool bCCW )
{
	if( bCCW )
	{
		// Counter clockwise.
		Int32 Delta = ATo - AFrom;
		if( Delta < 0 ) 
			Delta += 65536;
		return math::Angle((AFrom + math::Angle(math::trunc(Delta*Alpha))) & 0xffff);
	}
	else
	{
		// Clockwise.
		Int32 Delta = ATo - AFrom;
		if( Delta > 0 ) 
			Delta -= 65536;
		return math::Angle((AFrom + math::Angle(math::trunc(Delta*Alpha))) & 0xffff);
	}
}


/*-----------------------------------------------------------------------------
    TRect implementation.
-----------------------------------------------------------------------------*/

//
// Rect constructor.
//
TRect::TRect( const math::Vector* Verts, Int32 NumVerts )
{
	Min = Max = Verts[0];

	for( Int32 i = 1; i < NumVerts; i++ )
	{
		Min.x = ::Min( Min.x, Verts[i].x );
		Min.y = ::Min( Min.y, Verts[i].y );

		Max.x = ::Max( Max.x, Verts[i].x );
		Max.y = ::Max( Max.y, Verts[i].y );
	}
}


//
// Return true if point lie on rectangle border.
//
Bool TRect::AtBorder( const math::Vector& P, Float Thresh ) const
{
	Bool bOuter = ( P.x >= Min.x-Thresh )&&
		          ( P.x <= Max.x+Thresh )&&
				  ( P.y >= Min.y-Thresh )&&
				  ( P.y <= Max.y+Thresh );

	Bool bInner = ( P.x >= Min.x+Thresh )&&
		          ( P.x <= Max.x-Thresh )&&
				  ( P.y >= Min.y+Thresh )&&
				  ( P.y <= Max.y-Thresh );

	return bOuter && !bInner;
}


//
// Return true, if this rect overlap other.
//
Bool TRect::IsOverlap( const TRect& Other ) const
{
	if( Min.x >= Other.Max.x || Other.Min.x >= Max.x )
		return false;

	if( Min.y >= Other.Max.y || Other.Min.y >= Max.y )
		return false;

	return true;
}


//
// Return true, if line AB intersect rect.
// Where Hit - location of hit, Normal - hit normal,
// Time - time of intersection.
//
Bool TRect::LineIntersect
( 
	const math::Vector& A, 
	const math::Vector& B, 
	math::Vector* V, 
	math::Vector* Normal, 
	Float* Time 
) const
{
	Bool bInside	= true;
	Float XSign		= 1.f;
	Float YSign		= 1.f;
	math::Vector Dir		= B - A;
	math::Vector VTime;

	// X test.
	if( A.x < Min.x )
	{
		if( Dir.x > 0.f )
		{
			bInside	= false;
			XSign	= -1.f;
			VTime.x	= (Min.x - A.x) / Dir.x;
		}
		else
			return false;
	}
	else if( A.x > Max.x )
	{
		if( Dir.x < 0.f )
		{
			bInside	= false;
			VTime.x	= (Max.x - A.x) / Dir.x;
		}
		else
			return false;
	}
	else
		VTime.x	= 0.f;

	// Y test.
	if( A.y < Min.y )
	{
		if( Dir.y > 0.f )
		{
			bInside	= false;
			YSign	= -1.f;
			VTime.y	= (Min.y - A.y) / Dir.y;
		}
		else
			return false;
	}
	else if( A.y > Max.y )
	{
		if( Dir.y < 0.f )
		{
			bInside	= false;
			VTime.y	= (Max.y - A.y) / Dir.y;
		}
		else
			return false;
	}
	else
		VTime.y	= 0.f;

	if( bInside )
	{
		if( V ) *V = A;
		if( Normal ) *Normal = math::Vector( 0.f, 1.f );
		if( Time ) *Time = 0.f;
		return true;
	}
	else
	{
		Float TestTime;
		math::Vector TestNormal;

		if( VTime.y > VTime.x )
		{
			TestTime	= VTime.y;
			TestNormal	= math::Vector( 0.f, YSign );
		}
		else
		{
			TestTime	= VTime.x;
			TestNormal	= math::Vector( XSign, 0.f );
		}

		if( TestTime >= 0.f && TestTime <= 1.f )
		{
			math::Vector TestHit = A + Dir*TestTime;

			if	(	TestHit.x >= Min.x-0.1f &&
					TestHit.x <= Max.x+0.1f &&
					TestHit.y >= Min.y-0.1f &&
					TestHit.y <= Max.y+0.1f
				)
			{
				if( V ) *V = TestHit;
				if( Normal ) *Normal = TestNormal;				
				if( Time ) *Time = TestTime;
				return true;
			}
		}
		else
			return false;
	}
	return false;
}


//
// Rect extension.
//
TRect TRect::operator+( const math::Vector& V ) const
{
	TRect Result;
	Result.Min.x	= ::Min( Min.x, V.x );
	Result.Min.y	= ::Min( Min.y, V.y );
	Result.Max.x	= ::Max( Max.x, V.x );
	Result.Max.y	= ::Max( Max.y, V.y );
	return Result;
}


//
// Rect extension.
//
TRect TRect::operator+=( const math::Vector& V )
{
	Min.x	= ::Min( Min.x, V.x );
	Min.y	= ::Min( Min.y, V.y );
	Max.x	= ::Max( Max.x, V.x );
	Max.y	= ::Max( Max.y, V.y );
	return *this;
}

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/