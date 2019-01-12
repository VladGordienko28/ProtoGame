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
	const TVector& Center,
	Float Radius,
	TColor Color,
	Bool bStipple,
	Int32 Detail 
)
{
	TVector P1, P2;
	P1.X = Center.X + Radius * FastSinF( 2.f * 0 * PI / Detail );
	P1.Y = Center.Y + Radius * FastCosF( 2.f * 0 * PI / Detail );

	for( Int32 i=1; i<=Detail; i++ )
	{
		P2.X = Center.X + Radius * FastSinF( 2.f * i * PI / Detail );
		P2.Y = Center.Y + Radius * FastCosF( 2.f * i * PI / Detail );

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
	const TVector& A,
	const TVector& B,
	TColor Color,
	Bool bStipple,
	Int32 Detail 
)
{
	// Precompute values.
	Float LineX = B.X - A.X;
	Float LineY = (B.Y - A.Y) / 2.f;
	Float DFX = LineX / Detail;
	TVector P1 = A, P2;
	Float XWalk = A.X;
	Int32 X = 2048;
	Int32 DX = 4096 / Detail;

	// Draw chain of lines.
	for( Int32 i=0; i<=Detail; i++ )
	{
		Float Y = B.Y - (Sin8192(X) + 1.f) * LineY;
		P2 = TVector( XWalk, Y );

		DrawLine( P1, P2, Color, bStipple );

		P1 = P2;
		XWalk += DFX;
		X += DX;
	}
}


//
// Draw wire rectangle.
//
void CCanvas::DrawLineRect
( 
	const TVector& Center,
	const TVector& Size,
	TAngle Rotation,
	TColor Color,
	Bool bStipple 
)
{
	TVector Size2 = Size * 0.5f;
	TCoords Coords = TCoords( Center, Rotation );
	
	TVector XAxis = Coords.XAxis * Size2.X,
			YAxis = Coords.YAxis * Size2.Y;

	TVector V1 = Center - YAxis - XAxis;
	TVector V2 = Center + YAxis - XAxis;
	TVector V3 = Center + YAxis + XAxis;
	TVector V4 = Center - YAxis + XAxis;

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
	const TVector& Center,
	TAngle Rotation,
	Float Size,
	TColor Color,
	Bool bStipple 
)
{
	Float Size2 = Size / 2.f;
	TCoords Coords = TCoords( Rotation );

	DrawLine( Center - Coords.XAxis * Size2,
		      Center + Coords.XAxis * Size2,
			  Color, bStipple );

	DrawLine( Center - Coords.YAxis * Size2,
		      Center + Coords.YAxis * Size2,
			  Color, bStipple );
}


//
// Draw a point with border.
//
void CCanvas::DrawCoolPoint( const TVector& P, Float Size, TColor Color )
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
	const TVector& Start, 
	const TVector& Scale
)
{
	assert(Font);

	if( Font->Bitmaps.Num() > 1 )
	{
		// Slow method, with atlases switching.

		// Setup tile info.
		TRenderRect R;
		R.Color			= Color;
		R.Flags			= POLY_Unlit;
		R.Rotation		= 0;

		TVector Walk = Start;

		// For each character in string.
		for( Int32 i=0; i<Len; i++ )
		{
			// Handle whitespace glyph.
			if( Text[i] == L' ' )
			{
				TGlyph& Glyph = Font->GetGlyph(L'a');
				Walk.X += Glyph.W * Scale.X;
				continue;
			}

			// Get glyph info.
			TGlyph& Glyph = Font->GetGlyph(Text[i]);
			TVector CharSize = TVector( Glyph.W * Scale.X, Glyph.H * Scale.Y );

			// Character location.
			R.Texture		= Font->Bitmaps[Glyph.iBitmap];
			R.Bounds.Min	= Walk;
			R.Bounds.Max	= Walk + CharSize;

			// Glyph texture coords.
			R.TexCoords.Min = TVector( Glyph.X, Glyph.Y ) * (1.f/GLYPHS_ATLAS_SIZE);
			R.TexCoords.Max = TVector( Glyph.X+Glyph.W, Glyph.Y+Glyph.H ) * (1.f/GLYPHS_ATLAS_SIZE);

			// Draw tile.
			DrawRect(R);

			// Walk to next.
			Walk.X += CharSize.X;
		}
	}
	else
	{
		// Fast version with render list.

		// Setup list.
		TRenderList List( Len, Color );
		List.Texture	= Font->Bitmaps.Num() ? Font->Bitmaps[0] : nullptr;
		List.Flags		= POLY_Unlit;
		List.DrawColor	= Color;

		TVector Walk	= Start;
		Int32 NumRnd	= 0;

		// For each character in string.
		for( Int32 i=0; i<Len; i++ )
		{
			// Handle whitespace glyph.
			if( Text[i] == L' ' )
			{
				TGlyph& Glyph = Font->GetGlyph(L'a');
				Walk.X += Glyph.W * Scale.X;
				continue;
			}

			// Get glyph info.
			TGlyph& Glyph = Font->GetGlyph(Text[i]);
			TVector CharSize = TVector( Glyph.W * Scale.X, Glyph.H * Scale.Y );

			List.Vertices[NumRnd*4+0]		= TVector( Walk.X, Walk.Y );
			List.Vertices[NumRnd*4+1]		= TVector( Walk.X, Walk.Y+CharSize.Y );
			List.Vertices[NumRnd*4+2]		= TVector( Walk.X+CharSize.X, Walk.Y+CharSize.Y );
			List.Vertices[NumRnd*4+3]		= TVector( Walk.X+CharSize.X, Walk.Y );

			// Glyph coords.
			Float X1	= (Glyph.X)*(1.f/GLYPHS_ATLAS_SIZE);
			Float Y1	= (Glyph.Y)*(1.f/GLYPHS_ATLAS_SIZE);
			Float X2	= (Glyph.X+Glyph.W)*(1.f/GLYPHS_ATLAS_SIZE);
			Float Y2	= (Glyph.Y+Glyph.H)*(1.f/GLYPHS_ATLAS_SIZE);

			List.TexCoords[NumRnd*4+0]		= TVector( X1, Y1 );
			List.TexCoords[NumRnd*4+1]		= TVector( X1, Y2 );
			List.TexCoords[NumRnd*4+2]		= TVector( X2, Y2 );
			List.TexCoords[NumRnd*4+3]		= TVector( X2, Y1 );

			NumRnd++;

			// Walk to next.
			Walk.X += CharSize.X;
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
		Coords( TVector( Width/2.f, Height/2.f ), TVector( 1.f, 0.f ), TVector( 0.f, 1.f ) ),
		FOV( Width, -Height ),
		Zoom( 1.f ),
		UnCoords( Coords.Transpose() ),
		Bounds( TVector( 0.f, 0.f ), TVector( Width, Height ) )
{}


//
// Level or part of level constructor.
//
TViewInfo::TViewInfo( TVector InLocation, TAngle InRotation, TVector InFOV, Float InZoom, Bool InbMirage, Float InX, Float InY, Float InWidth, Float InHeight  )
	:	X( InX ), Y( InY ), Width( InWidth ), Height( InHeight ), 
		bMirage( InbMirage ),
		Coords( InLocation, InRotation ),
		FOV( InFOV ),
		Zoom( InZoom ),
		UnCoords( Coords.Transpose() )
{
	if( InRotation )
		Bounds	= TRect( Coords.Origin, Zoom * FastSqrt(Sqr(FOV.X)+Sqr(FOV.Y)));
	else
		Bounds	= TRect( Coords.Origin, FOV * Zoom );
}


//
// Transform a point in the world's coords to the screen
// coords system.
//
void TViewInfo::Project( const TVector V, Float& OutX, Float& OutY ) const
{
	TVector R, PixToFOV;

	R			= TransformPointBy( V, Coords );
	PixToFOV.X	= Width		/ FOV.X;
	PixToFOV.Y	= Height	/ FOV.Y;

	OutX	= Width	 * 0.5f + R.X * PixToFOV.X/Zoom;
	OutY	= Height * 0.5f - R.Y * PixToFOV.Y/Zoom;
}


//
// Transform a point in the screen's coords to the world
// coords system.
//
TVector TViewInfo::Deproject( Float InX, Float InY ) const
{
	TVector R, FOVToPix;

	FOVToPix.X	= FOV.X / Width;
	FOVToPix.Y	= FOV.Y / Height;

	R.X	= ( InX - Width * 0.5f  ) * FOVToPix.X * Zoom;
	R.Y = ( Height * 0.5f - InY ) * FOVToPix.Y * Zoom;

	return TransformPointBy( R, UnCoords );
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
	Vertices	= (TVector*)CCanvas::GPool.Push(NumRects*4*sizeof(TRect));
	TexCoords	= (TVector*)CCanvas::GPool.Push(NumRects*4*sizeof(TRect));
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
	Vertices	= (TVector*)CCanvas::GPool.Push(NumRects*4*sizeof(TRect));
	TexCoords	= (TVector*)CCanvas::GPool.Push(NumRects*4*sizeof(TRect));
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