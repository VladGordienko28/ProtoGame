/*=============================================================================
    FrMath.h: Fluorine math library.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

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
	Array<TInterpPoint<T>>	Samples;

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
	for( i=0; i<Samples.size() && Samples[i].Input<Input; i++ );

	Samples.insert(i, 1);
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
	if( Samples.size() == 0 )
		return Default;
	if( Samples.size() < 2 || Input <= Samples[0].Input  )
		return Samples[0].Output;
	if( Input >= Samples[Samples.size()-1].Input )
		return Samples[Samples.size()-1].Output;

	// Linear search.
	for( Int32 i=1; i<Samples.size(); i++ )
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
				case EIT_Linear:	ASample = lerp( A.Output, B.Output, Alpha ); break;
				case EIT_Fast:		ASample = lerp( A.Output, B.Output, Alpha*Alpha*Alpha ); break;
				case EIT_Slow:		ASample = lerp( A.Output, B.Output, 1.f-sqr(1.f-Alpha) ); break;
				case EIT_Smooth:	ASample = lerp( A.Output, B.Output, 3.f*Alpha*Alpha - 2.f*Alpha*Alpha*Alpha ); break;
			}

			switch( B.Type )
			{
				case EIT_Stepped:	BSample = A.Output; break;
				case EIT_Linear:	BSample = lerp( A.Output, B.Output, Alpha ); break;
				case EIT_Fast:		BSample = lerp( A.Output, B.Output, Alpha*Alpha*Alpha ); break;
				case EIT_Slow:		BSample = lerp( A.Output, B.Output, 1.f-sqr(1.f-Alpha) ); break;
				case EIT_Smooth:	BSample = lerp( A.Output, B.Output, 3.f*Alpha*Alpha - 2.f*Alpha*Alpha*Alpha ); break;
			}

			// Mix samples.
			return lerp( ASample, BSample, Alpha );
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
	if( Samples.size() == 0 )
		return Default;
	if( Samples.size() < 2 || Input <= Samples[0].Input  )
		return Samples[0].Output;
	if( Input >= Samples[Samples.size()-1].Input )
		return Samples[Samples.size()-1].Output;

	for( Int32 i=1; i<Samples.size(); i++ )
	{
		const TInterpPoint<T>& A = Samples[i-1];
		const TInterpPoint<T>& B = Samples[i];

		if( B.Input >= Input )
		{
			Float Alpha = (Input-A.Input)/(B.Input-A.Input);
			return lerp( A.Output, B.Output, Alpha );
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

	for( Int32 i=1; i<Samples.Num(); i++ )
	{
		const TInterpPoint<T>& A = Samples[i-1];
		const TInterpPoint<T>& B = Samples[i];

		if( B.Input >= Input )
			return A.Output;
	}

	return Default;
}


/*-----------------------------------------------------------------------------
    Math functions.
-----------------------------------------------------------------------------*/

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

extern inline Float Wrap( Float V, Float Min, Float Max );
extern inline UInt32 IntLog2( UInt32 A );
extern math::Angle AngleLerp( math::Angle AFrom, math::Angle ATo, Float Alpha, Bool bCCW );

//
// Vector math functions.
//

inline math::Vector LineSegmentInter
( 
	const math::Vector& P1, 
	const math::Vector& P2, 
	const math::Vector& Origin, 
	const math::Vector& Normal )
{
	math::Vector V = P2 - P1;
	return P1 + V* (((Origin - P1) * Normal) / (V * Normal));
}

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/