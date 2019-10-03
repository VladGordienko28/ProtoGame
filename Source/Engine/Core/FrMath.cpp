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
    The End.
-----------------------------------------------------------------------------*/