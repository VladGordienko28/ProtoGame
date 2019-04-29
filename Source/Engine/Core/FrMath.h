/*=============================================================================
    FrMath.h: Fluorine math library.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

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