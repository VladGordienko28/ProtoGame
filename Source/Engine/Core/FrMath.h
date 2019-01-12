/*=============================================================================
    FrMath.h: Fluorine math library.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Constants.
-----------------------------------------------------------------------------*/

// Common math constans.
#define EPSILON 0.0001f
#define PI 3.141592653f

// Flu math constants.
#define WORLD_SIZE 4096
#define WORLD_HALF (WORLD_SIZE/2)


/*-----------------------------------------------------------------------------
    Declarations.
-----------------------------------------------------------------------------*/

// Forward declaration.
struct TVector;
struct TAngle;
struct TCoords;
struct TRect;
struct TCircle;
struct TColor;


/*-----------------------------------------------------------------------------
    TVector.
-----------------------------------------------------------------------------*/

//
// A point or direction.
//
struct TVector
{
public:
	Float	X;
	Float	Y;

	// Constructors.
	TVector()
	{}
	TVector( Float InX, Float InY )
		:	X(InX), Y(InY)
	{}
	
	// Operators.
	TVector operator-() const
	{
		return TVector( -X, -Y );
	}
	TVector operator+() const
	{
		return TVector( X, Y );
	}
	TVector operator+( const TVector& V ) const
	{
		return TVector( X + V.X, Y + V.Y );
	}
	TVector operator-( const TVector& V ) const
	{
		return TVector( X - V.X, Y - V.Y );
	}
	Float operator*( const TVector& V ) const
	{
		return X * V.X + Y * V.Y;
	}
	TVector operator*( Float F ) const
	{
		return TVector( X * F, Y * F );
	}
	Float operator/( const TVector& V ) const
	{
		return X * V.Y - Y * V.X;
	}
	TVector operator/( Float F ) const
	{
		return TVector( -F * Y, F * X );	
	}
	TVector operator+=( const TVector& V )
	{
		X += V.X;
		Y += V.Y;
		return *this;
	}
	TVector operator-=( const TVector& V )
	{
		X -= V.X;
		Y -= V.Y;
		return *this;
	}
	TVector operator*=( Float F )
	{
		X *= F;
		Y *= F;
		return *this;
	}
	Bool operator==( const TVector& V ) const
	{
		return X == V.X && Y == V.Y;
	}
	Bool operator!=( const TVector& V ) const
	{
		return X != V.X || Y != V.Y;
	}

	// Functions.
	TVector Cross() const
	{
		return TVector( -Y, X );
	}
	Float Size() const
	{
		return sqrtf( X * X + Y * Y ); 
	}
	Float SizeSquared() const
	{
		return X * X + Y * Y;
	}
	void Normalize()
	{
		Float f = Size();
		if( f > EPSILON )
		{
			X /= f;
			Y /= f;
		}
	}
	void Snap( Float Grid );

	// Friends.
	friend void Serialize( CSerializer& S, TVector& V )
	{
		Serialize( S, V.X );
		Serialize( S, V.Y );
	}
};


/*-----------------------------------------------------------------------------
    TAngle.
-----------------------------------------------------------------------------*/

//
// An angle value 0..0xffff.
//
struct TAngle
{
public:
	Int32	Angle;

	// Constructors.
	TAngle()
		:	Angle(0)
	{}
	TAngle( Int32 InAngle )
		:	Angle(InAngle)
	{}
	TAngle( Float InAngle );

	// Operators.
    operator Int32() const
	{
		return Angle & 0xffff;
	}
	operator Bool() const
	{
		return (Angle & 0xffff) != 0;
	}
	TAngle operator+() const
	{
		return TAngle( Angle );
	}
	TAngle operator-() const
	{
		return TAngle( 0xffff - (Angle & 0xffff) );
	}
	TAngle operator+( TAngle R ) const
	{
		return TAngle( (Angle + R.Angle) & 0xffff );
	}
	TAngle operator-( TAngle R ) const
	{
		return TAngle( (Angle - R.Angle) & 0xffff );
	}
	TAngle operator*( Float F ) const;

	Bool operator==( TAngle R ) const
	{
		return (R.Angle & 0xffff) == (Angle & 0xffff);
	}
	Bool operator!=( TAngle R ) const
	{
		return (R.Angle & 0xffff) != (Angle & 0xffff);
	}
	TAngle operator+=( TAngle R )
	{
		Angle = (Angle + R.Angle) & 0xffff;
		return *this;
	}
	TAngle operator-=( TAngle R )
	{
		Angle = (Angle - R.Angle) & 0xffff;
		return *this;
	}

	// Functions.
	void Snap( Int32 Grid );
	Float ToRads() const;
	Float ToDegs() const;
	Float GetCos() const;
	Float GetSin() const;

	// Statics.
	static TAngle FromDegs( Float Degs );
	static TAngle FromRads( Float Rads );

	// Friends.
	friend void Serialize( CSerializer& S, TAngle& R )
	{
		Serialize( S, R.Angle );
	}
};


/*-----------------------------------------------------------------------------
    TRect.
-----------------------------------------------------------------------------*/

//
// An AABB rectangle.
//
struct TRect
{
public:
	TVector	Min;
	TVector	Max;

	// Constructors.
	TRect()
	{}
	TRect( const TVector& InCenter, const TVector& InSize )
	{
		TVector Half = InSize * 0.5f;
		Min = InCenter - Half;
		Max = InCenter + Half;
	}
	TRect( const TVector& InCenter, Float InSide )
	{
		Float Half = InSide * 0.5f;
		Min = TVector( InCenter.X - Half, InCenter.Y - Half );
		Max = TVector( InCenter.X + Half, InCenter.Y + Half );
	}
	TRect( const TVector* Verts, Int32 NumVerts );

	// Operators.
	Bool operator==( const TRect& R ) const
	{
		return Min == R.Min && Max == R.Max;
	}
	Bool operator!=( const TRect& R ) const
	{
		return Min != R.Min || Max != R.Max;
	}
	operator Bool() const
	{
		return Min != Max;
	}
	TRect operator+( const TVector& V ) const;
	TRect operator+=( const TVector& V );

	// Functions.
	TVector Center() const
	{
		return (Min + Max) * 0.5;
	}
	TVector Size() const
	{
		return Max - Min;
	}
	Float Area() const
	{
		TVector Diagonal = Max - Min;
		return Diagonal.X * Diagonal.Y;
	}
	Bool IsInside( const TVector& P ) const
	{
		return P.X >= Min.X && P.X <= Max.X &&
			   P.Y >= Min.Y && P.Y <= Max.Y;
	}
	Float GetExtrema( Int32 i ) const
	{
		return ((Float*)&Min)[i];
	}
	Bool AtBorder( const TVector& P, Float Thresh = 0.01 ) const;
	Bool IsOverlap( const TRect& Other ) const;
	Bool LineIntersect
	( 
		const TVector& A, 
		const TVector& B, 
		TVector* V = nullptr, 
		TVector* Normal = nullptr,
		Float* Time = nullptr 
	) const;

	// Friends.
	friend void Serialize( CSerializer& S, TRect& R )
	{
		Serialize( S, R.Min );
		Serialize( S, R.Max );
	}
};


/*-----------------------------------------------------------------------------
	TCircle.
-----------------------------------------------------------------------------*/

//
// A circle.
//
struct TCircle
{
public:
	TVector	Center;
	Float	Radius;

	// Constructors.
	TCircle()
	{}
	TCircle( const TVector& InCenter, Float InRadius )
		:	Center( InCenter ),
			Radius( InRadius )
	{}
	TCircle( const TVector* Verts, Int32 NumVerts );

	// Operators.
	Bool operator==( const TCircle& C ) const
	{
		return Center==C.Center && Radius==C.Radius;
	}
	Bool operator!=( const TCircle& C ) const
	{
		return Radius!=C.Radius || Center!=C.Center;
	}
	operator Bool() const
	{
		return Radius > 0.f;
	}
	TCircle operator+( const TVector& V ) const;
	TCircle operator+=( const TVector& V );

	// Functions.
	Float Area() const
	{
		return PI * Radius*Radius;
	}
	Bool IsInside( const TVector& P ) const;
	Bool AtBorder( const TVector& P, Float Thresh = 0.01f ) const;
	Bool IsOverlap( const TCircle& Other ) const;

	// Friends.
	friend void Serialize( CSerializer& S, TCircle& C )
	{
		Serialize( S, C.Center );
		Serialize( S, C.Radius );
	}
};


/*-----------------------------------------------------------------------------
    TCoords.
-----------------------------------------------------------------------------*/

//
// A coords system matrix.
// Technically it a coords basis.
// 
struct TCoords
{
public:
	TVector	Origin;
	TVector	XAxis;
	TVector YAxis;

	// Constructors.
	TCoords()
		:	Origin( 0.f, 0.f ),
			XAxis( 1.f, 0.f ),
			YAxis( 0.f, 1.f )
	{}
	TCoords( const TVector& InOrigin )
		:	Origin( InOrigin ),
			XAxis( 1.f, 0.f ),
			YAxis( 0.f, 1.f )
	{}
	TCoords( const TVector& InOrigin, const TVector& InX, const TVector& InY )
		:	Origin( InOrigin ),
			XAxis( InX ),
			YAxis( InY )
	{}
	TCoords( const TVector& InOrigin, const TVector& InX )
		:	Origin( InOrigin ),
			XAxis( InX ),
			YAxis( InX.Cross() )
	{}
	TCoords( TAngle R )
		:	Origin( 0.f, 0.f )
	{
		XAxis = TVector( R.GetCos(), R.GetSin() );
		YAxis = XAxis.Cross();
	}
	TCoords( const TVector& InOrigin, TAngle R )
		:	Origin( InOrigin )
	{
		XAxis = TVector( R.GetCos(), R.GetSin() );
		YAxis = XAxis.Cross();
	}	

	// Operators.
	Bool operator==( const TCoords& C ) const
	{
		return Origin == C.Origin &&
			   XAxis  == C.XAxis &&
			   YAxis  == C.YAxis;
	}
	Bool operator!=( const TCoords& C ) const
	{
		return Origin != C.Origin ||
			   XAxis  != C.XAxis ||
			   YAxis  != C.YAxis;
	}
	TCoords operator<<( const TVector& V ) const;
	TCoords operator<<( TAngle R ) const;
	TCoords operator>>( const TVector& V ) const;
	TCoords operator>>( TAngle R ) const;

	// Functions.
	TCoords Transpose() const;

	// Friends.
	friend void Serialize( CSerializer& S, TCoords& C )
	{
		Serialize( S, C.Origin );
		Serialize( S, C.XAxis );
		Serialize( S, C.YAxis );
	}
	static const TCoords Identity;
};


/*-----------------------------------------------------------------------------
	TInterpCurve.
-----------------------------------------------------------------------------*/

//
// A point interpolation type.
//
enum EInterpType
{
	EIT_Stepped,
	EIT_Linear,
	EIT_Fast,
	EIT_Slow,
	EIT_Smooth
};


//
// An interpolation point.
//
template<class T> struct TInterpPoint
{
public:
	// Variables.
	EInterpType	Type;
	Float		Input;
	T			Output;

	// TInterpPoint interface.
	TInterpPoint()
	{}
	TInterpPoint( EInterpType InType, Float In, const T& Out )
		:	Type(InType), Input(In), Output(Out)
	{}

	// Friends.
	friend void Serialize( CSerializer& S, TInterpPoint& V )
	{
		SerializeEnum( S, V.Type );
		Serialize( S, V.Input );
		Serialize( S, V.Output );
	}
};


//
// An interpolation curve.
//
template<class T> class TInterpCurve
{
public:
	// Variables.
	TArray<TInterpPoint<T>>	Samples;

	// TInterpCurve interface.
	TInterpCurve();
	Int32 AddSample( Float Input, const T& Output );
	void Empty();

	// Sample value at specified X(Input).
	T SampleAt( Float Input, const T& Default ) const;
	T SampleSteppedAt( Float Input, const T& Default ) const;
	T SampleLinearAt( Float Input, const T& Default ) const;

	// Frieds.
	friend void Serialize( CSerializer& S, TInterpCurve& V )
	{
		Serialize( S, V.Samples );
	}
};


//
// Interpolator constructor.
//
template<class T> inline TInterpCurve<T>::TInterpCurve()
{
}


//
// Add a new point to the curve.
//
template<class T> inline Int32 TInterpCurve<T>::AddSample( Float Input, const T& Output )
{		
	Int32 i;
	for( i=0; i<Samples.Num() && Samples[i].Input<Input; i++ );

	Samples.Insert(i);
	Samples[i] = TInterpPoint<T>( EIT_Linear, Input, Output );
	return i;
}


//
// Empty samples.
//
template<class T> inline void TInterpCurve<T>::Empty()
{
	Samples.Empty();
}


//
// Sample using per-sample interpolation.
//
template<class T> inline T TInterpCurve<T>::SampleAt( Float Input, const T& Default ) const
{
	// Edge cases.
	if( Samples.Num() == 0 )
		return Default;
	if( Samples.Num() < 2 || Input <= Samples[0].Input  )
		return Samples[0].Output;
	if( Input >= Samples[Samples.Num()-1].Input )
		return Samples[Samples.Num()-1].Output;

	// Linear search.
	for( Int32 i=1; i<Samples.Num(); i++ )
	{
		const TInterpPoint<T>& A = Samples[i-1];
		const TInterpPoint<T>& B = Samples[i];

		if( B.Input >= Input )
		{
			T ASample=Default, BSample=Default;
			Float Alpha = (Input-A.Input)/(B.Input-A.Input);

			switch( A.Type )
			{
				case EIT_Stepped:	ASample = A.Output; break;
				case EIT_Linear:	ASample = Lerp( A.Output, B.Output, Alpha ); break;
				case EIT_Fast:		ASample = Lerp( A.Output, B.Output, Alpha*Alpha*Alpha ); break;
				case EIT_Slow:		ASample = Lerp( A.Output, B.Output, 1.f-Sqr(1.f-Alpha) ); break;
				case EIT_Smooth:	ASample = Lerp( A.Output, B.Output, 3.f*Alpha*Alpha - 2.f*Alpha*Alpha*Alpha ); break;
			}

			switch( B.Type )
			{
				case EIT_Stepped:	BSample = A.Output; break;
				case EIT_Linear:	BSample = Lerp( A.Output, B.Output, Alpha ); break;
				case EIT_Fast:		BSample = Lerp( A.Output, B.Output, Alpha*Alpha*Alpha ); break;
				case EIT_Slow:		BSample = Lerp( A.Output, B.Output, 1.f-Sqr(1.f-Alpha) ); break;
				case EIT_Smooth:	BSample = Lerp( A.Output, B.Output, 3.f*Alpha*Alpha - 2.f*Alpha*Alpha*Alpha ); break;
			}

			// Mix samples.
			return Lerp( ASample, BSample, Alpha );
		}
	}

	// Never reached.
	return Default;
}


//
// Sample using linear interpolation.
//
template<class T> inline T TInterpCurve<T>::SampleLinearAt( Float Input, const T& Default ) const
{
	if( Samples.Num() == 0 )
		return Default;
	if( Samples.Num() < 2 || Input <= Samples[0].Input  )
		return Samples[0].Output;
	if( Input >= Samples[Samples.Num()-1].Input )
		return Samples[Samples.Num()-1].Output;

	for( Int32 i=1; i<Samples.Num(); i++ )
	{
		const TInterpPoint<T>& A = Samples[i-1];
		const TInterpPoint<T>& B = Samples[i];

		if( B.Input >= Input )
		{
			Float Alpha = (Input-A.Input)/(B.Input-A.Input);
			return Lerp( A.Output, B.Output, Alpha );
		}
	}

	return Default;
}


//
// Sample using stepped interpolation.
//
template<class T> inline T TInterpCurve<T>::SampleSteppedAt( Float Input, const T& Default ) const
{
	if( Samples.Num() == 0 )
		return Default;
	if( Samples.Num() < 2 || Input <= Samples[0].Input  )
		return Samples[0].Output;
	if( Input >= Samples[Samples.Num()-1].Input )
		return Samples[Samples.Num()-1].Output;

	for( Integer i=1; i<Samples.Num(); i++ )
	{
		const TInterpPoint<T>& A = Samples[i-1];
		const TInterpPoint<T>& B = Samples[i];

		if( B.Input >= Input )
			return A.Output;
	}

	return Default;
}


/*-----------------------------------------------------------------------------
    Math templates.
-----------------------------------------------------------------------------*/

//
// General purpose.
//
template<class T> inline T Min( T A, T B )
{
	return A < B ? A : B;
}
template<class T> inline T Max( T A, T B )
{
	return A > B ? A : B;
}
template<class T> inline T Clamp( T V, T A, T B )
{
	return V < A ? A : V > B ? B : V;
}
template<class T> inline void Exchange( T& A, T& B )
{
	T C = A;
	A = B;
	B = C;
}
template<class T> inline T Sqr( T V )
{
	return V * V;
}
template<class T> inline T Lerp( T A, T B, Float Alpha )
{
	return A + ( B - A ) * Alpha;
}
template<class T> inline T Abs( T V )
{
	return (V < (T)0) ? -V : V;
}
template<class T> inline Bool InRange( T V, T A, T B )
{
	return (V>=A) && (V<=B);
}


/*-----------------------------------------------------------------------------
    Math functions.
-----------------------------------------------------------------------------*/

inline Float Sin( Float F )
{
	return sinf( F );
}
inline Float Cos( Float F )
{
	return cosf( F );
}
inline Float Pow( Float Base, Float P )
{
	return (Float)pow( Base, P );
}
inline Int32 Floor( Float F )
{
	return (Int32)floorf(F);
}
inline Int32 Ceil( Float F )
{
	return (Int32)ceilf(F);
}
inline Int32 Round( Float F )
{
	return (Int32)roundf(F);
}
inline Int32 Trunc( Float F )
{
	return (Int32)truncf(F);
}
inline Float Sqrt( Float F )
{
	return sqrtf( F );
}
inline Float ArcTan( Float X )
{
	return (Float)atan( X );
}
inline Float ArcTan2( Float Y, Float X )
{
	return atan2f( Y, X );
}
inline Float Ln( Float X )
{	
// Fuck :(
#pragma push_macro("log")
#undef log
	return (Float)log(X);
#pragma pop_macro("log")
}
inline Float Frac( Float X )
{
	return X - Floor(X);
}
inline void Snap( Float Grid, Float& F )
{
	if( Grid != 0.f ) F = roundf(F / Grid) * Grid;
}
inline Float FMod( Float X, Float Y )
{
	return fmodf( X, Y );
}
inline Float InvPow2( Int32 A )
{
	static const Float GRescale[] =
	{
		1.f / 1.f,
		1.f / 2.f,
		1.f / 4.f,
		1.f / 8.f,
		1.f / 16.f,
		1.f / 32.f,
		1.f / 64.f,
		1.f / 128.f,
		1.f / 256.f,
		1.f / 512.f,
		1.f / 1024.f,
		1.f / 2048.f,
		1.f / 4096.f,
	};
	return GRescale[A];
}
inline Bool IsPowerOfTwo( UInt32 i )
{
	return ((i) & (i-1)) == 0;
}

extern inline Float FastSinF( Float F );
extern inline Float FastCosF( Float F );
extern inline Float FastSqrt( Float F );
extern inline Float FastArcTan( Float X );
extern inline Float FastArcTan2( Float Y, Float X );
extern inline Float Sin8192( Int32 i );
extern inline Float Wrap( Float V, Float Min, Float Max );
extern inline UInt32 IntLog2( UInt32 A );
extern TAngle AngleLerp( TAngle AFrom, TAngle ATo, Float Alpha, Bool bCCW );

//
// Transformation functions.
//
extern inline TVector TransformVectorBy( const TVector& V, const TCoords& C );
extern inline TVector TransformPointBy( const TVector& P, const TCoords& C );


//
// Vector math functions.
//
inline Float Distance( const TVector& A, const TVector& B )
{
	return Sqrt( Sqr(A.X - B.X) + Sqr(A.Y - B.Y) );
}
inline Float DistanceSq( const TVector& A, const TVector& B )
{
	return Sqr(A.X - B.X) + Sqr(A.Y - B.Y);
}
inline TAngle VectorToAngle( const TVector& V )
{
	return TAngle( ArcTan2( V.Y, V.X ) );
}
inline TVector AngleToVector( TAngle R )
{
	return TVector( R.GetCos(), R.GetSin() );
}
inline Bool IsWalkable( const TVector& FloorNormal )
{
	return ( FloorNormal.X <= +0.7f ) &&
		   ( FloorNormal.X >= -0.7f ) &&
		   ( FloorNormal.Y > 0.0f );
}
inline Bool PointsAreNear( const TVector& A, const TVector& B, Float Dist )
{
	return !(( Abs(A.X - B.X) > Dist )||
		     ( Abs(A.Y - B.Y) > Dist ));
}
inline Float PointLineDist( const TVector& P, const TVector& Origin, const TVector& Normal )
{
	return ( P - Origin ) * Normal;
}
inline TVector LineSegmentInter
( 
	const TVector& P1, 
	const TVector& P2, 
	const TVector& Origin, 
	const TVector& Normal )
{
	TVector V = P2 - P1;
	return P1 + V* (((Origin - P1) * Normal) / (V * Normal));
}


//
// Polygon functions.
//
extern inline Bool IsConvexPoly( const TVector* Verts, Int32 NumVerts );
extern inline Bool IsPointInsidePoly( const TVector& P, const TVector* Verts, Int32 NumVerts );
extern Bool LineIntersectPoly
( 
	const TVector& A, 
	const TVector& B, 
	const TVector* Verts, 
	Int32 NumVerts, 
	TVector* V = nullptr, 
	TVector* Normal = nullptr 
);
extern Bool SegmentsIntersect
( 
	const TVector& A1, 
	const TVector& A2, 
	const TVector& B1, 
	const TVector& B2, 
	TVector* V = nullptr 
);
extern Bool PointOnSegment( const TVector& P, const TVector& A, const TVector& B, Float Thresh = EPSILON );


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/