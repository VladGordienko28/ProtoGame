/*=============================================================================
    FrCSG.cpp: 2D csg polygon operations.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    TCSGPoly.
-----------------------------------------------------------------------------*/

//
// A CSG polygon to perform split operations, used
// instead of FBrush's vertex list.
//
struct TCSGPoly
{
public:
	// Variables.
	math::Vector	Vertices[FBrushComponent::MAX_BRUSH_VERTS*2];
	Int32			NumVerts;

	// TCSGPoly interface.
	void FromBrush( FBrushComponent* Source );
	FBrushComponent* ToBrush( FLevel* Level, FBrushComponent* Sample );
	Bool TestSegment( math::Vector A, math::Vector B );
	void Split( math::Vector A, math::Vector B, TCSGPoly& Front, TCSGPoly& Back );
	TRect GetAABB();
	void MergeOverlap();
};


//
// Copy brush vertices and prepare polygon for CSG
// processing.
//
void TCSGPoly::FromBrush( FBrushComponent* Source )
{
	assert(Source);
	assert(Source->NumVerts<=FBrushComponent::MAX_BRUSH_VERTS);
	NumVerts	= Source->NumVerts;

	// Copy and transform vertices.
	for( Int32 i=0; i<NumVerts; i++ )
		Vertices[i] = Source->Vertices[i] + Source->Location;

}


//
// Part of "Separated Axis Theorem" used for CSG.
//
inline Bool TestAxis( math::Vector Axis, math::Vector A, math::Vector B, const TCSGPoly& Poly )
{
	// Project segment onto axis.
	Float MinS	= A * Axis;
	Float MaxS	= B * Axis;
	
	// Assume MinS <= MaxS.
	if( MinS > MaxS )
		Exchange( MinS, MaxS );

	// Project poly onto axis.
	Float MinP	= Poly.Vertices[0] * Axis;
	Float MaxP	= MinP;

	for( Int32 i=1; i<Poly.NumVerts; i++ )
	{
		Float T = Poly.Vertices[i] * Axis;

		if( T < MinP )	MinP	= T;
		if( T > MaxP )	MaxP	= T;
	}

	// Test intervals.
	return	( MinP < MaxS ) && 
			( MinS < MaxP );
}


//
// Figure out whether segment intersect this poly.
//
Bool TCSGPoly::TestSegment( math::Vector A, math::Vector B )
{
	// Fast detection, if some point is 
	// inside the poly.
	if( math::isPointInsidePoly( A, Vertices, NumVerts ) ||
		math::isPointInsidePoly( B, Vertices, NumVerts ) )
			return true;

	// Test all poly's axis.
	math::Vector Axis, V1 = Vertices[NumVerts-1];
	for( Int32 i=0; i<NumVerts; i++ )
	{
		math::Vector V2 = Vertices[i];
		Axis = (V2 - V1).cross();

		if( !TestAxis( Axis, A, B, *this ) )
			return false;

		V1 = V2;
	}

	// Test with segment axis.
	Axis = (A-B).cross();
	if( !TestAxis( Axis, A, B, *this ) )
		return false;

	return true;
}


//
// Split poly by line. If no split happened and this poly totally on the one side
// of line, some out poly( front or back ) are totally copy this, 
// other one is empty( NumVers=0 ).
//
void TCSGPoly::Split( math::Vector A, math::Vector B, TCSGPoly& Front, TCSGPoly& Back )
{
	// Precompute.
	math::Vector SplitOrigin	= A;
	math::Vector SplitNormal = ( B - A ).cross();
	SplitNormal.normalize();

	// Zero out polys.
	Front.NumVerts	= 0;
	Back.NumVerts	= 0;

	math::Vector V1 = Vertices[NumVerts-1];
	Float PrevDist = math::pointLineDistance( V1, SplitOrigin, SplitNormal );

	for( Int32 i=0; i<NumVerts; i++ )
	{
		math::Vector V2 = Vertices[i];
		Float Dist = math::pointLineDistance( V2, SplitOrigin, SplitNormal );

		if( PrevDist <= -math::EPSILON )
		{
			// Add V1 to the back poly.
			Back.Vertices[Back.NumVerts++] = V1;
		}
		else if( PrevDist >= +math::EPSILON )
		{
			// Add V1 to the front poly.
			Front.Vertices[Front.NumVerts++] = V1;
		}
		else
		{
			// Point line on the splitter, add to both polys.
			Back.Vertices[Back.NumVerts++] = V1;
			Front.Vertices[Front.NumVerts++] = V1;
		}

		// Now, test for segment intersection.
		if( (Dist * PrevDist) <= -math::EPSILON )
		{
			math::Vector Middle = LineSegmentInter( V1, V2, SplitOrigin, SplitNormal );

			Back.Vertices[Back.NumVerts++] = Middle;
			Front.Vertices[Front.NumVerts++] = Middle;	
		}

		V1 = V2;
		PrevDist = Dist;	
	}

	Front.MergeOverlap();
	Back.MergeOverlap();
}


//
// Return polygon bounds.
//
TRect TCSGPoly::GetAABB()
{
	return TRect( Vertices, NumVerts );
}


//
// Merge all overlaps vertices.
//
void TCSGPoly::MergeOverlap()
{
	for( Int32 v=0; v<NumVerts; v++ )
	{
		math::Vector&	V1 = Vertices[v],
					V2 = Vertices[(v+1) % NumVerts];

		if( (V1-V2).sizeSquared() <= 0.25f )
		{
			for( Int32 i=v; i<NumVerts-1; i++ )
				Vertices[i] = Vertices[i+1];

			v--;
			NumVerts--;
		}
	}
}


//
// Convert polygon to brush and add to level.
// Sample need to inherited properties.
//
FBrushComponent* TCSGPoly::ToBrush( FLevel* Level, FBrushComponent* Sample )
{
	// Don't add, if poly are invalid.
	if( NumVerts < 3 )
		return nullptr;

	FBrushComponent* B = (FBrushComponent*)Level->CreateEntity
															( 
																Sample->Entity->Script, 
																L"", 
																Sample->Location 
															)->Base;

	// Copy properties from sample.
	B->bFlipH		= Sample->bFlipH;
	B->bFlipV		= Sample->bFlipV;
	B->Texture		= Sample->Texture;
	B->bUnlit		= Sample->bUnlit;
	B->Color		= Sample->Color;
	B->Layer		= Sample->Layer + RandomF()*0.01f;
	B->TexCoords	= Sample->TexCoords;
	B->Type			= Sample->Type;

	// Copy vertices.
	B->NumVerts		= Min<Int32>( NumVerts, FBrushComponent::MAX_BRUSH_VERTS );
	for( Int32 i=0; i<B->NumVerts; i++ )
		B->Vertices[i] = Vertices[i] - B->Location;

	return B;
}


/*-----------------------------------------------------------------------------
    Global CSG functions.
-----------------------------------------------------------------------------*/

//
// Perform brush union with the world.
// Warning: Its used recursion to cut chunks, from
// previous iteration.
// Result = A | B.
//
void CSGUnion( FBrushComponent* Brush, FLevel* Level )
{
	assert(Level);
	assert(Brush);

	debug( L"CSG: Union" );

	//
	// Not implemented. I think this CSG operation are
	// the most useless. 
	//
}


//
// Perform intersection of brush and world brushes, 
// not very useful, but it's ok.
// Result = A & B.
//
void CSGIntersection( FBrushComponent* Brush, FLevel* Level )
{
	assert(Brush);
	assert(Level);

	debug( L"CSG: Intersection" );

	// Prepare.
	Int32 NumEnts = Level->Entities.size();
	TCSGPoly Poly, OtherPoly;
	TRect Rect;
	
	Poly.FromBrush( Brush );
	Rect = Brush->GetAABB();

	// Test with all Brushes in the level.
	for( Int32 i=0; i<NumEnts; i++ )
	{
		// Simple tests to reject.
		if( (Level->Entities[i]->Base == Brush) ||
			(!Level->Entities[i]->Base->IsA(FBrushComponent::MetaClass) ) ||
			(Level->Entities[i]->Base->bDestroyed) )
				continue;

		FBrushComponent* Other = (FBrushComponent*)Level->Entities[i]->Base;

		// Another fast test.
		if( !Rect.IsOverlap( Other->GetAABB() ) )
			continue;

		Bool bHasAffect = false;
		OtherPoly.FromBrush( Other );

		// Here we split other poly and add chunks to world.
		math::Vector V1 = Poly.Vertices[Poly.NumVerts-1];
		for( Int32 j=0; j<Poly.NumVerts; j++ )
		{
			math::Vector V2 = Poly.Vertices[j];

			if( OtherPoly.TestSegment( V1, V2 ) )
			{
				// This segment intersect other poly, split it now.
				TCSGPoly Front, Back;				
				bHasAffect = true;
				OtherPoly.Split( V1, V2, Front, Back );

				// Back chunks continue to split.
				OtherPoly = Back;
			}

			V1 = V2;
		}

		// Check special case, all vertices are totally inside.
		if( !bHasAffect )
		{
			Int32 NumIns = 0;
			for( Int32 j=0; j<OtherPoly.NumVerts; j++ )
				if( math::isPointInsidePoly( OtherPoly.Vertices[j], Poly.Vertices, Poly.NumVerts ) )
					NumIns++;

			bHasAffect = NumIns == OtherPoly.NumVerts;
		}

		// Replace the parent.
		if( bHasAffect )
		{
			OtherPoly.ToBrush( Level, Other );
			Level->DestroyEntity( Other->Entity );
		}
	}

	// And anyway destroy the source brush.
	Level->DestroyEntity( Brush->Entity );
}


//
// Perform subtraction brush from world.
// Overlap brushes will be replaced.
// A[i] = A[i] & !B.
//
void CSGDifference( FBrushComponent* Brush, FLevel* Level )
{
	assert(Level);
	assert(Brush);

	debug( L"CSG: Difference" );

	// Prepare.
	Int32 NumEnts = Level->Entities.size();
	TCSGPoly Poly, OtherPoly;
	TRect Rect;
	
	Poly.FromBrush( Brush );
	Rect = Brush->GetAABB();

	// Test with all Brushes in the level.
	for( Int32 i=0; i<NumEnts; i++ )
	{
		// Simple tests to reject.
		if( (Level->Entities[i]->Base == Brush) ||
			(!Level->Entities[i]->Base->IsA(FBrushComponent::MetaClass) ) ||
			(Level->Entities[i]->Base->bDestroyed) )
				continue;
			
		FBrushComponent* Other = (FBrushComponent*)Level->Entities[i]->Base;

		// Another cheap test.
		if( !Rect.IsOverlap( Other->GetAABB() ) )
			continue;

		Bool bHasAffect = false;
		OtherPoly.FromBrush(Other);

		// Here we split other poly and add chunks to world.
		math::Vector V1 = Poly.Vertices[Poly.NumVerts-1];
		for( Int32 j=0; j<Poly.NumVerts; j++ )
		{
			math::Vector V2 = Poly.Vertices[j];

			if( OtherPoly.TestSegment( V1, V2 ) )
			{
				// This segment intersect other poly, split it now.
				TCSGPoly Front, Back;				
				bHasAffect = true;
				OtherPoly.Split( V1, V2, Front, Back );

				// Front chunk add to world.
				Front.ToBrush( Level, Other );

				// Back chunks continue to split.
				OtherPoly = Back;
			}

			V1 = V2;
		}

		// Check special case, all vertices are totally inside.
		if( !bHasAffect )
		{
			Int32 NumIns = 0;
			for( Int32 j=0; j<OtherPoly.NumVerts; j++ )
				if( math::isPointInsidePoly( OtherPoly.Vertices[j], Poly.Vertices, Poly.NumVerts ) )
					NumIns++;

			bHasAffect = NumIns == OtherPoly.NumVerts;

		}

		// Remove parent.
		if( bHasAffect )
			Level->DestroyEntity( Other->Entity );
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/