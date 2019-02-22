/*=============================================================================
    FrCnvPrim.h: Canvas primitives drawing routines.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    CCanvas implementation.
-----------------------------------------------------------------------------*/

//
// Static memory for temporal rendering objects.
// Such as vertex, color, texture buffers.
// 4 mB should be enough.
//
CStaticPool<4*1024*1024>	CCanvas::GPool;


//
// Draw a circle.
//
void CCanvas::DrawCircle
( 
	const math::Vector& Center,
	Float Radius,
	TColor Color,
	Bool bStipple,
	Int32 Detail 
)
{
	math::Vector P1, P2;
	P1.x = Center.x + Radius * math::sin( 2.f * 0 * math::PI / Detail );
	P1.y = Center.y + Radius * math::cos( 2.f * 0 * math::PI / Detail );

	for( Int32 i=1; i<=Detail; i++ )
	{
		P2.x = Center.x + Radius * math::sin( 2.f * i * math::PI / Detail );
		P2.y = Center.y + Radius * math::cos( 2.f * i * math::PI / Detail );

		DrawLine( P1, P2, Color, bStipple );
		P1 = P2;
	}
}


//
// Draw a smooth line. It's amazing,
// looks like Bezier curve or something,
// but it's just a chunk of cosine function.
//
void CCanvas::DrawSmoothLine
( 
	const math::Vector& A,
	const math::Vector& B,
	TColor Color,
	Bool bStipple,
	Int32 Detail 
)
{
	DrawLine( A, B, Color, bStipple );

	/*
	// Precompute values.
	Float LineX = B.x - A.x;
	Float LineY = (B.y - A.y) / 2.f;
	Float DFX = LineX / Detail;
	math::Vector P1 = A, P2;
	Float XWalk = A.x;
	Int32 X = 2048;
	Int32 DX = 4096 / Detail;

	// Draw chain of lines.
	for( Int32 i=0; i<=Detail; i++ )
	{
		Float Y = B.y - (Sin8192(X) + 1.f) * LineY;
		P2 = math::Vector( XWalk, Y );

		DrawLine( P1, P2, Color, bStipple );

		P1 = P2;
		XWalk += DFX;
		X += DX;
	}
*/
}


//
// Draw wire rectangle.
//
void CCanvas::DrawLineRect
( 
	const math::Vector& Center,
	const math::Vector& Size,
	math::Angle Rotation,
	TColor Color,
	Bool bStipple 
)
{
	math::Vector Size2 = Size * 0.5f;
	math::Coords Coords = math::Coords( Center, Rotation );
	
	math::Vector XAxis = Coords.xAxis * Size2.x,
			YAxis = Coords.yAxis * Size2.y;

	math::Vector V1 = Center - YAxis - XAxis;
	math::Vector V2 = Center + YAxis - XAxis;
	math::Vector V3 = Center + YAxis + XAxis;
	math::Vector V4 = Center - YAxis + XAxis;

	DrawLine( V1, V2, Color, bStipple );
	DrawLine( V2, V3, Color, bStipple );
	DrawLine( V3, V4, Color, bStipple );
	DrawLine( V4, V1, Color, bStipple );
}


//
// Draw a lines cross.
//
void CCanvas::DrawLineStar
( 
	const math::Vector& Center,
	math::Angle Rotation,
	Float Size,
	TColor Color,
	Bool bStipple 
)
{
	Float Size2 = Size / 2.f;
	math::Coords Coords = math::Coords( Rotation );

	DrawLine( Center - Coords.xAxis * Size2,
		      Center + Coords.xAxis * Size2,
			  Color, bStipple );

	DrawLine( Center - Coords.yAxis * Size2,
		      Center + Coords.yAxis * Size2,
			  Color, bStipple );
}


//
// Draw a point with border.
//
void CCanvas::DrawCoolPoint( const math::Vector& P, Float Size, TColor Color )
{
	DrawPoint( P, Size + 2.0f, COLOR_Black );
	DrawPoint( P, Size, Color );

}

//
// Draw a text.
//
void CCanvas::DrawText
(	
	const Char* Text,
	Int32 Len,
	FFont* Font, 
	TColor Color,
	const math::Vector& Start, 
	const math::Vector& Scale
)
{
	assert(Font);

	if( Font->Bitmaps.size() > 1 )
	{
		// Slow method, with atlases switching.

		// Setup tile info.
		TRenderRect R;
		R.Color			= Color;
		R.Flags			= POLY_Unlit;
		R.Rotation		= 0;

		math::Vector Walk = Start;

		// For each character in string.
		for( Int32 i=0; i<Len; i++ )
		{
			// Handle whitespace glyph.
			if( Text[i] == L' ' )
			{
				TGlyph& Glyph = Font->GetGlyph(L'a');
				Walk.x += Glyph.W * Scale.x;
				continue;
			}

			// Get glyph info.
			TGlyph& Glyph = Font->GetGlyph(Text[i]);
			math::Vector CharSize = math::Vector( Glyph.W * Scale.x, Glyph.H * Scale.y );

			// Character location.
			R.Texture		= Font->Bitmaps[Glyph.iBitmap];
			R.Bounds.min	= Walk;
			R.Bounds.max	= Walk + CharSize;

			// Glyph texture coords.
			R.TexCoords.min = math::Vector( Glyph.X, Glyph.Y ) * (1.f/GLYPHS_ATLAS_SIZE);
			R.TexCoords.max = math::Vector( Glyph.X+Glyph.W, Glyph.Y+Glyph.H ) * (1.f/GLYPHS_ATLAS_SIZE);

			// Draw tile.
			DrawRect(R);

			// Walk to next.
			Walk.x += CharSize.x;
		}
	}
	else
	{
		// Fast version with render list.

		// Setup list.
		TRenderList List( Len, Color );
		List.Texture	= Font->Bitmaps.size() ? Font->Bitmaps[0] : nullptr;
		List.Flags		= POLY_Unlit;
		List.DrawColor	= Color;

		math::Vector Walk	= Start;
		Int32 NumRnd	= 0;

		// For each character in string.
		for( Int32 i=0; i<Len; i++ )
		{
			// Handle whitespace glyph.
			if( Text[i] == L' ' )
			{
				TGlyph& Glyph = Font->GetGlyph(L'a');
				Walk.x += Glyph.W * Scale.x;
				continue;
			}

			// Get glyph info.
			TGlyph& Glyph = Font->GetGlyph(Text[i]);
			math::Vector CharSize = math::Vector( Glyph.W * Scale.x, Glyph.H * Scale.y );

			List.Vertices[NumRnd*4+0]		= math::Vector( Walk.x, Walk.y );
			List.Vertices[NumRnd*4+1]		= math::Vector( Walk.x, Walk.y+CharSize.y );
			List.Vertices[NumRnd*4+2]		= math::Vector( Walk.x+CharSize.x, Walk.y+CharSize.y );
			List.Vertices[NumRnd*4+3]		= math::Vector( Walk.x+CharSize.x, Walk.y );

			// Glyph coords.
			Float X1	= (Glyph.X)*(1.f/GLYPHS_ATLAS_SIZE);
			Float Y1	= (Glyph.Y)*(1.f/GLYPHS_ATLAS_SIZE);
			Float X2	= (Glyph.X+Glyph.W)*(1.f/GLYPHS_ATLAS_SIZE);
			Float Y2	= (Glyph.Y+Glyph.H)*(1.f/GLYPHS_ATLAS_SIZE);

			List.TexCoords[NumRnd*4+0]		= math::Vector( X1, Y1 );
			List.TexCoords[NumRnd*4+1]		= math::Vector( X1, Y2 );
			List.TexCoords[NumRnd*4+2]		= math::Vector( X2, Y2 );
			List.TexCoords[NumRnd*4+3]		= math::Vector( X2, Y1 );

			NumRnd++;

			// Walk to next.
			Walk.x += CharSize.x;
		}

		// Don't render whitespace.
		List.NumRects	= NumRnd;
		DrawList( List );
	}
}


/*-----------------------------------------------------------------------------
    Stack of transformations.
-----------------------------------------------------------------------------*/

//
// Store old transformation and
// make current given new one.
//
void CCanvas::PushTransform( const TViewInfo& Info )
{
	assert(StackTop < VIEW_STACK_SIZE);
	ViewStack[StackTop++] = View;
	SetTransform( Info );
}


//
// Restore last saved transform.
//
void CCanvas::PopTransform()
{
	assert(StackTop > 0);
	SetTransform( ViewStack[--StackTop] );
}


/*-----------------------------------------------------------------------------
    Projection.
-----------------------------------------------------------------------------*/

//
// View info constructor.
//
TViewInfo::TViewInfo()
{}


//
// Screen or piece of it constructor.
//
TViewInfo::TViewInfo( Float InX, Float InY, Float InWidth, Float InHeight )
	:	X( InX ), Y( InY ), Width( InWidth ), Height( InHeight ), 
		bMirage( false ),
		Coords( math::Vector( Width/2.f, Height/2.f ), math::Vector( 1.f, 0.f ), math::Vector( 0.f, 1.f ) ),
		FOV( Width, -Height ),
		Zoom( 1.f ),
		UnCoords( Coords.transpose() ),
		Bounds( math::Vector( 0.f, 0.f ), Width, Height )
{}


//
// Level or part of level constructor.
//
TViewInfo::TViewInfo( math::Vector InLocation, math::Angle InRotation, math::Vector InFOV, Float InZoom, Bool InbMirage, Float InX, Float InY, Float InWidth, Float InHeight  )
	:	X( InX ), Y( InY ), Width( InWidth ), Height( InHeight ), 
		bMirage( InbMirage ),
		Coords( InLocation, InRotation ),
		FOV( InFOV ),
		Zoom( InZoom ),
		UnCoords( Coords.transpose() )
{
	if( InRotation )
		Bounds	= math::Rect( Coords.origin, Zoom * math::sqrt(sqr(FOV.x)+sqr(FOV.y)));
	else
		Bounds	= math::Rect( Coords.origin, FOV.x * Zoom, FOV.y * Zoom );
}


//
// Transform a point in the world's coords to the screen
// coords system.
//
void TViewInfo::Project( const math::Vector V, Float& OutX, Float& OutY ) const
{
	math::Vector R, PixToFOV;

	R			= math::transformPointBy( V, Coords );
	PixToFOV.x	= Width		/ FOV.x;
	PixToFOV.y	= Height	/ FOV.y;

	OutX	= Width	 * 0.5f + R.x * PixToFOV.x/Zoom;
	OutY	= Height * 0.5f - R.y * PixToFOV.y/Zoom;
}


//
// Transform a point in the screen's coords to the world
// coords system.
//
math::Vector TViewInfo::Deproject( Float InX, Float InY ) const
{
	math::Vector R, FOVToPix;

	FOVToPix.x	= FOV.x / Width;
	FOVToPix.y	= FOV.y / Height;

	R.x	= ( InX - Width * 0.5f  ) * FOVToPix.x * Zoom;
	R.y = ( Height * 0.5f - InY ) * FOVToPix.y * Zoom;

	return math::transformPointBy( R, UnCoords );
}


/*-----------------------------------------------------------------------------
    TRenderList implementation.
-----------------------------------------------------------------------------*/

//
// Initialize render list for rectangles
// with various colors.
//
TRenderList::TRenderList( Int32 InNumRcts )
{
	Flags		= POLY_None;
	Texture		= nullptr;
	NumRects	= InNumRcts;
	Vertices	= (math::Vector*)CCanvas::GPool.Push(NumRects*4*sizeof(math::Rect));
	TexCoords	= (math::Vector*)CCanvas::GPool.Push(NumRects*4*sizeof(math::Rect));
	Colors		= (TColor*)CCanvas::GPool.Push(NumRects*4*sizeof(TColor));
}


//
// Initialize render list for rectangles
// with shared color.
//
TRenderList::TRenderList( Int32 InNumRcts, TColor InColor )
{
	Flags		= POLY_None;
	Texture		= nullptr;
	NumRects	= InNumRcts;
	Vertices	= (math::Vector*)CCanvas::GPool.Push(NumRects*4*sizeof(math::Rect));
	TexCoords	= (math::Vector*)CCanvas::GPool.Push(NumRects*4*sizeof(math::Rect));
	Colors		= nullptr;
	DrawColor	= InColor;
}


//
// Render list destruction.
//
TRenderList::~TRenderList()
{
	// Just release taken memory.
	CCanvas::GPool.Pop(Vertices);
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/