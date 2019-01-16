/*=============================================================================
	FrDbgDraw.cpp: Lines and Points.
	Created by Vlad Gordienko, May. 2018.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
	CDebugDrawHelper implementation.
-----------------------------------------------------------------------------*/

//
// Debug draw constructor.
//
CDebugDrawHelper::CDebugDrawHelper()
{
	Reset();
}


//
// Reset Debug Drawer.
//
void CDebugDrawHelper::Reset()
{
	Points.empty();
	Lines.empty();
}


//
// Render all primitives.
//
void CDebugDrawHelper::Render( CCanvas* Canvas )
{
	for( Int32 i=0; i<Points.size(); i++ )
	{
		auto& P = Points[i];
		Canvas->DrawPoint( P.Position, P.Size, P.Color );
	}
	for( Int32 i=0; i<Lines.size(); i++ )
	{
		auto& L = Lines[i];
		Canvas->DrawLine( L.PFrom, L.PTo, L.Color, false );
	}
}


//
// Tick all pending primitives.
//
void CDebugDrawHelper::Tick( Float Delta )
{
	for( Int32 i=0; i<Points.size(); )
	{
		if( (Points[i].Life -= Delta) <= 0.f )
		{
			Points[i] = Points.last();
			Points.pop();
		}
		else
		{
			i++;
		}
	}
	for( Int32 i=0; i<Lines.size(); )
	{
		if( (Lines[i].Life -= Delta) <= 0.f )
		{
			Lines[i] = Lines.last();
			Lines.pop();
		}
		else
		{
			i++;
		}
	}
}


//
// Draw point.
//
Bool CDebugDrawHelper::DrawLine( const TVector& A, const TVector& B, TColor Color, Float LifeTime )
{
	if( Lines.size() >= MAX_LINES )
		return false;

	TTempLine Line;
	Line.PFrom = A;
	Line.PTo = B;
	Line.Color = Color;
	Line.Life = LifeTime;

	Lines.push(Line);
	return true;
}


//
// Draw point.
//
Bool CDebugDrawHelper::DrawPoint( const TVector& P, TColor Color, Float Size, Float LifeTime )
{
	if( Points.size() >= MAX_POINTS )
		return false;

	TTempPoint Point;
	Point.Position = P;
	Point.Size = Size;
	Point.Color = Color;
	Point.Life = LifeTime;

	Points.push(Point);
	return true;
}


//
// Return helper instance.
//
CDebugDrawHelper& CDebugDrawHelper::Instance()
{
	static CDebugDrawHelper Helper;
	return Helper;
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/