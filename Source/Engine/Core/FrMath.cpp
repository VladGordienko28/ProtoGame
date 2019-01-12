/*=============================================================================
	FrMath.cpp: Math library.
	Created by Vlad Gordienko, Jun. 2016.
	Refactoring Jan. 2018.
=============================================================================*/

#include "..\Engine.h"

/*-----------------------------------------------------------------------------
    TMathTables.
-----------------------------------------------------------------------------*/

//
// A math tables class.
//
class TMathTables
{
public:
	// Variables.
	Float	SinTable[8192];
	Float	BToF[256];

	// Initialization.
	TMathTables()
	{
		// Setup sine table.
		for( Int32 i=0; i<8192; i++ )
			SinTable[i] = Sin( 2.f * PI * i / 8192.f );

		// Byte to float conversion.
		for( Int32 i=0; i<256; i++ )
			BToF[i]	=	i/256.f;
	}
} GMathTables;


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
		return Min + V - Size * Trunc( V / Size );
	else
		return Max + V - Size * Trunc( V / Size );
}


//
// Pure table sine.
//
Float Sin8192( Int32 i )
{
	return GMathTables.SinTable[i & 8191];
}

//
// Angles linear interpolation.
//
TAngle AngleLerp( TAngle AFrom, TAngle ATo, Float Alpha, Bool bCCW )
{
	if( bCCW )
	{
		// Counter clockwise.
		Int32 Delta = ATo.Angle - AFrom.Angle;
		if( Delta < 0 ) 
			Delta += 65536;
		return TAngle((AFrom.Angle+Trunc(Delta*Alpha)) & 0xffff);
	}
	else
	{
		// Clockwise.
		Int32 Delta = ATo.Angle - AFrom.Angle;
		if( Delta > 0 ) 
			Delta -= 65536;
		return TAngle((AFrom.Angle+Trunc(Delta*Alpha)) & 0xffff);
	}
}


/*-----------------------------------------------------------------------------
    Fast math functions.
-----------------------------------------------------------------------------*/

//
// Union to get access to the float's bytes.
//
union TFloatInfo
{
	Float	F;
	Int32	I;
	UInt32	D;
	UInt8	B[sizeof(Float)];

	TFloatInfo( Float InF )
		: F(InF)
	{}
};


//
// Fast table sine.
//
Float FastSinF( Float F )
{
	return GMathTables.SinTable[8191 & Floor(F*(8192.f/(2.f*PI)))];
}


//
// Fast table cosine.
//
Float FastCosF( Float F )
{
	return GMathTables.SinTable[8191 & (Floor(F*(8192.f/(2.f*PI)))+2048)];
}


//
// Very fast square root, but
// not very precise :(
//
Float FastSqrt( Float F )
{/*
#ifdef FLU_ASM && 0
	Float Result;
	__asm
	{
		mov		eax, [F]
		add		eax, 127 << 23
		shr		eax, 1
		mov		[Result], eax
	}
	return Result;
#else*/
	TFloatInfo Info = F;
	Info.D += 127 << 23;
	Info.D >>= 1;
	return Info.F;
/*#endif*/
}


//
// Fast arctangent using linear
// function approximation.
//
Float FastArcTan( Float X )
{
	return (0.5f*PI)*X/(Abs(X)+1.f);
}


//
// Fast arctan2.
//
Float FastArcTan2( Float Y, Float X )
{
	if( X > 0.f )
	{
		return FastArcTan(Y/X);
	}
	else if( X < 0.f )
	{
		Float A = FastArcTan(Y/X);
		return Y >= 0.f ? A+PI : A-PI;
	}
	else
	{
		return Y > 0.f ? +PI/2.f : -PI/2.f;
	}
}


/*-----------------------------------------------------------------------------
    Transform functions.
-----------------------------------------------------------------------------*/

//
// Transform vector to other coords system.
//
TVector TransformVectorBy( const TVector& V, const TCoords& C )
{
	return TVector( V * C.XAxis, V * C.YAxis );
}


//
// Transform point to other coords system.
//
TVector TransformPointBy( const TVector& P, const TCoords& C )
{
	TVector V = P - C.Origin;
	return TVector( V * C.XAxis, V * C.YAxis );
}


/*-----------------------------------------------------------------------------
    TRect implementation.
-----------------------------------------------------------------------------*/

//
// Rect constructor.
//
TRect::TRect( const TVector* Verts, Int32 NumVerts )
{
	Min = Max = Verts[0];

	for( Int32 i = 1; i < NumVerts; i++ )
	{
		Min.X = ::Min( Min.X, Verts[i].X );
		Min.Y = ::Min( Min.Y, Verts[i].Y );

		Max.X = ::Max( Max.X, Verts[i].X );
		Max.Y = ::Max( Max.Y, Verts[i].Y );
	}
}


//
// Return true if point lie on rectangle border.
//
Bool TRect::AtBorder( const TVector& P, Float Thresh ) const
{
	Bool bOuter = ( P.X >= Min.X-Thresh )&&
		          ( P.X <= Max.X+Thresh )&&
				  ( P.Y >= Min.Y-Thresh )&&
				  ( P.Y <= Max.Y+Thresh );

	Bool bInner = ( P.X >= Min.X+Thresh )&&
		          ( P.X <= Max.X-Thresh )&&
				  ( P.Y >= Min.Y+Thresh )&&
				  ( P.Y <= Max.Y-Thresh );

	return bOuter && !bInner;
}


//
// Return true, if this rect overlap other.
//
Bool TRect::IsOverlap( const TRect& Other ) const
{
	if( Min.X >= Other.Max.X || Other.Min.X >= Max.X )
		return false;

	if( Min.Y >= Other.Max.Y || Other.Min.Y >= Max.Y )
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
	const TVector& A, 
	const TVector& B, 
	TVector* V, 
	TVector* Normal, 
	Float* Time 
) const
{
	Bool bInside	= true;
	Float XSign		= 1.f;
	Float YSign		= 1.f;
	TVector Dir		= B - A;
	TVector VTime;

	// X test.
	if( A.X < Min.X )
	{
		if( Dir.X > 0.f )
		{
			bInside	= false;
			XSign	= -1.f;
			VTime.X	= (Min.X - A.X) / Dir.X;
		}
		else
			return false;
	}
	else if( A.X > Max.X )
	{
		if( Dir.X < 0.f )
		{
			bInside	= false;
			VTime.X	= (Max.X - A.X) / Dir.X;
		}
		else
			return false;
	}
	else
		VTime.X	= 0.f;

	// Y test.
	if( A.Y < Min.Y )
	{
		if( Dir.Y > 0.f )
		{
			bInside	= false;
			YSign	= -1.f;
			VTime.Y	= (Min.Y - A.Y) / Dir.Y;
		}
		else
			return false;
	}
	else if( A.Y > Max.Y )
	{
		if( Dir.Y < 0.f )
		{
			bInside	= false;
			VTime.Y	= (Max.Y - A.Y) / Dir.Y;
		}
		else
			return false;
	}
	else
		VTime.Y	= 0.f;

	if( bInside )
	{
		if( V ) *V = A;
		if( Normal ) *Normal = TVector( 0.f, 1.f );
		if( Time ) *Time = 0.f;
		return true;
	}
	else
	{
		Float TestTime;
		TVector TestNormal;

		if( VTime.Y > VTime.X )
		{
			TestTime	= VTime.Y;
			TestNormal	= TVector( 0.f, YSign );
		}
		else
		{
			TestTime	= VTime.X;
			TestNormal	= TVector( XSign, 0.f );
		}

		if( TestTime >= 0.f && TestTime <= 1.f )
		{
			TVector TestHit = A + Dir*TestTime;

			if	(	TestHit.X >= Min.X-0.1f &&
					TestHit.X <= Max.X+0.1f &&
					TestHit.Y >= Min.Y-0.1f &&
					TestHit.Y <= Max.Y+0.1f
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
TRect TRect::operator+( const TVector& V ) const
{
	TRect Result;
	Result.Min.X	= ::Min( Min.X, V.X );
	Result.Min.Y	= ::Min( Min.Y, V.Y );
	Result.Max.X	= ::Max( Max.X, V.X );
	Result.Max.Y	= ::Max( Max.Y, V.Y );
	return Result;
}


//
// Rect extension.
//
TRect TRect::operator+=( const TVector& V )
{
	Min.X	= ::Min( Min.X, V.X );
	Min.Y	= ::Min( Min.Y, V.Y );
	Max.X	= ::Max( Max.X, V.X );
	Max.Y	= ::Max( Max.Y, V.Y );
	return *this;
}


/*-----------------------------------------------------------------------------
	TCircle implementation.
-----------------------------------------------------------------------------*/

//
// Compute bounding circle from the set of vertices.
//
TCircle::TCircle( const TVector* Verts, Int32 NumVerts )
	:	Center( 0.f, 0.f ),
		Radius( 0.f )
{
	if( NumVerts )
	{
		TRect Rect( Verts, NumVerts );
		Center = Rect.Center();
		
		for( Int32 i=0; i<NumVerts; i++ )
		{
			Float Dist = DistanceSq(Center, Verts[i]);
			if( Dist > Radius )
				Radius = Dist;
		}
		Radius	= Sqrt(Radius);
	}
}


//
// Circle extension.
//
TCircle TCircle::operator+( const TVector& V ) const
{
	Float Dist	= Distance( Center, V );
	return TCircle( Center, Max( Radius, Dist ) );
}


//
// Circle extension.
//
TCircle TCircle::operator+=( const TVector& V )
{
	Float TestDist = DistanceSq( Center, V );
	if( Sqr(Radius) < TestDist )
	{
		Radius = Sqrt(TestDist);
	}
	return *this;
}


//
// Return true, if point inside circle.
//
Bool TCircle::IsInside( const TVector& P ) const
{
	return Distance(P, Center) <= Radius;
}


//
// Return true, if point lies on circle border.
//
Bool TCircle::AtBorder( const TVector& P, Float Thresh ) const
{
	Float Dist = Distance( P, Center );
	return InRange( Dist, Radius-Thresh, Radius+Thresh );
}


//
// Return true, if two circles overlaps.
//
Bool TCircle::IsOverlap( const TCircle& Other ) const
{
	return Distance(Other.Center, Center) <= (Radius+Other.Radius);
}


/*-----------------------------------------------------------------------------
    TCoords implementation.
-----------------------------------------------------------------------------*/

//
// An identity matrix.
//
const TCoords TCoords::Identity = TCoords
(
	TVector( 0.f, 0.f ),
	TVector( 1.f, 0.f ),
	TVector( 0.f, 1.f )
);


//
// Translate coords system by vector in local space.
//
TCoords TCoords::operator<<( const TVector& V ) const
{
	return TCoords( Origin + TransformVectorBy( V, *this ),
					XAxis, 
					YAxis );
}


//
// Rotate coords system by angle in local space.
//
TCoords TCoords::operator<<( TAngle R ) const
{
	TCoords TR( -R );
	return TCoords( Origin,
					TransformVectorBy( XAxis, TR ),
					TransformVectorBy( YAxis, TR ) );
}


//
// Translate coords system by vector in world space.
//
TCoords TCoords::operator>>( const TVector& V ) const
{
	return TCoords( Origin + V,
					XAxis,
					YAxis );
}


//
// Rotate coords system by angle in world space.
//
TCoords TCoords::operator>>( TAngle R ) const
{
	TCoords TR( -R );
	return TCoords( TransformPointBy( Origin, TR ),
					TransformVectorBy( XAxis, TR ),
					TransformVectorBy( YAxis, TR ) );
}


//
// Transpose coords system.
//
TCoords TCoords::Transpose() const
{
	return TCoords( -TransformVectorBy( Origin, *this ),
					TVector( XAxis.X, YAxis.X ),
					TVector( XAxis.Y, YAxis.Y ) );
}


/*-----------------------------------------------------------------------------
    TAngle implementation.
-----------------------------------------------------------------------------*/

//
// Angle constructor.
//
TAngle::TAngle( Float InAngle )
{
	Angle = (Int32)( InAngle/(2.f*PI)*65536.f ) & 0xffff;
}


//
// Angle float multiplication.
//
TAngle TAngle::operator*( const Float F ) const
{
	return TAngle( (Int32)(Angle*F) & 0xffff );
}


//
// Snap angle to grid.
//
void TAngle::Snap( Int32 Grid )
{
	if( Grid )
		Angle = (Round((Float)Angle/(Float)Grid)*Grid) & 0xffff;
}


//
// Convert angle to radians.
//
Float TAngle::ToRads() const
{
	return ( Angle & 0xffff ) / 65536.f * ( 2.f * PI );
}


//
// Convert angle to degrees.
//
Float TAngle::ToDegs() const
{
	return ( Angle & 0xffff ) / 182.0444f;
}


//
// Return cos of the angle.
//
Float TAngle::GetCos() const
{
	return GMathTables.SinTable[((Angle >> 3) + 2048) & 0x1fff];
}


//
// Return sin of the angle.
//
Float TAngle::GetSin() const
{
	return GMathTables.SinTable[(Angle >> 3) & 0x1fff];
}


//
// Convert degrees to angle.
//
TAngle TAngle::FromDegs( Float Degs )
{
	return Floor(Degs * 182.0444f) & 0xffff;
}


//
// Convert radians to angle.
//
TAngle TAngle::FromRads( Float Rads )
{
	return Floor(Rads * (65536.f/(2.f*PI))) & 0xffff;
}


/*-----------------------------------------------------------------------------
    TVector implementation.
-----------------------------------------------------------------------------*/

//
// Snap a vector to squared grid.
//
void TVector::Snap( Float Grid )
{
	::Snap( Grid, X );
	::Snap( Grid, Y );
}


/*-----------------------------------------------------------------------------
    Polygon functions.
-----------------------------------------------------------------------------*/

//
// Return true, if poly is convex.
//
Bool IsConvexPoly( const TVector* Verts, Int32 NumVerts )
{
	TVector Edge1, Edge2;

	Edge1 = Verts[0] - Verts[NumVerts-1];
	Edge2 = Verts[NumVerts-1] - Verts[NumVerts-2];

	if( Edge1 / Edge2 < 0.0f )
		return false;

	for( Int32 i=0; i<NumVerts-1; i++ )
	{
		Edge2 = Verts[i+1] - Verts[i];

		if( Edge2 / Edge1 < 0.0f )
			return false;

		Edge1 = Edge2;
	}

	return true;
}


//
// Return true, if given point is inside convex poly,
// otherwise return false.
//
Bool IsPointInsidePoly( const TVector& P, const TVector* Verts, Int32 NumVerts )
{
	TVector V1, V2;

	V1 = Verts[NumVerts-1];

	for( Int32 i=0; i<NumVerts; i++ )
	{
		V2 = Verts[i];

		if( (V2-V1)/(P-V2) > 0.f )
			return false;

		V1 = V2;
	}

	return true;
}


//
// Return true and and point of intersection if
// segments A1A2 and B1B2 intersect, otherwise return
// false.
//
Bool SegmentsIntersect( const TVector& A1, const TVector& A2, const TVector& B1, const TVector& B2, TVector* V )
{
	TVector ADir = A2 - A1;
	TVector BDir = B2 - B1;
	TVector ACross = ADir.Cross();
	TVector BCross = BDir.Cross();

	Float ADot = A1 * ACross;
	Float BDot = B1 * BCross;

	Float TimeA1 = A1*BCross - BDot;
	Float TimeA2 = A2*BCross - BDot;
	Float TimeB1 = B1*ACross - ADot;
	Float TimeB2 = B2*ACross - ADot;

	if( TimeA1*TimeA2 >= 0.f || TimeB1*TimeB2 >= 0.f )
	{
		if( V ) *V = TVector( 0.f, 0.f );
		return false;
	}
	else
	{
		Float Time = TimeA1 / (TimeA1 - TimeA2);
		if( V ) *V = A1 + ADir * Time;
		return true;
	}
}


//
// Return true, if line AB intersect poly.
// Point of hit is V.
//
Bool LineIntersectPoly
( 
	const TVector& A, 
	const TVector& B, 
	const TVector* Verts, 
	Int32 NumVerts, 
	TVector* V, 
	TVector* Normal 
)
{
	Bool bFound = false;
	Float TestDist, BestDist = 99999.9;
	TVector Vt, P2, P1 = Verts[NumVerts-1];

	TVector ResultPoint, ResultNormal;

	for( Int32 i=0; i<NumVerts; i++ )
	{
		P2 = Verts[i];

		if( SegmentsIntersect( A, B, P1, P2, &Vt ) )
		{
			bFound		= true;
			TestDist	= (A-Vt).SizeSquared();

			if( TestDist < BestDist )
			{
				ResultPoint			= Vt;
				ResultNormal		= (P2 - P1).Cross();
				BestDist	= TestDist;
			}
		}

		P1 = P2;
	}

	if( bFound )
	{
		// Normalize only in final.
		if( V )		*V = ResultPoint;
		if( Normal )
		{
			ResultNormal.Normalize();
			*Normal = ResultNormal;
		}
		return true;
	}
	else
		return false;
}


//
// Return true, if point P on segment AB. Segment width are Thresh.
//
Bool PointOnSegment( const TVector& P, const TVector& A, const TVector& B, Float Thresh )	
{
	// Get segment vector.
	// Here we uses sqrt, its not cool.
	TVector Dir = B-A;
	Float Size = Dir.Size();
	Dir.Normalize();
	
	// Transform point to segment coords system.
	TCoords Line = TCoords( A, Dir );
	TVector T = TransformPointBy( P, Line );

	// Test bounds.
	return	( T.X >= 0 ) &&
			( T.X <= Size ) &&
			( Abs(T.Y) < Thresh );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/