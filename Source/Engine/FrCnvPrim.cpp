/*=============================================================================
    FrCnvPrim.h: Canvas primitives drawing routines.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/////  tobe removed!!!!!!!!!!!!!!!!!!!!!!
ffx::Effect::Ptr g_coloredEffect;

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
	math::Color Color,
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

void CCanvas::DrawEllipse
( 
	const math::Vector& Center,
	Float XSize,
	Float YSize,
	math::Color Color,
	Bool bStipple,
	Int32 Detail
)
{
	math::Vector P1, P2;
	P1.x = Center.x + XSize * math::sin( 2.f * 0 * math::PI / Detail );
	P1.y = Center.y + YSize * math::cos( 2.f * 0 * math::PI / Detail );

	for( Int32 i=1; i<=Detail; i++ )
	{
		P2.x = Center.x + XSize * math::sin( 2.f * i * math::PI / Detail );
		P2.y = Center.y + YSize * math::cos( 2.f * i * math::PI / Detail );

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
	math::Color Color,
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
	math::Color Color,
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
	math::Color Color,
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
void CCanvas::DrawCoolPoint( const math::Vector& P, Float Size, math::Color Color )
{
	DrawPoint( P, Size + 2.0f, math::colors::BLACK );
	DrawPoint( P, Size, Color );

}

//
// Draw a text.
//
void CCanvas::DrawText
(	
	const Char* Text,
	Int32 Len,
	fnt::Font::Ptr Font, 
	math::Color Color,
	const math::Vector& Start, 
	const math::Vector& Scale
)
{
	assert(Font);

	if(  true ) // todo: remove this stuff !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	{
		// Slow method, with atlases switching.

		// Setup tile info.
		TRenderRect R;
		R.Color			= Color;
		R.Flags			= POLY_Unlit | POLY_AlphaGhost;
		R.Rotation		= 0;

		math::Vector Walk = Start;

		img::Image::Ptr image = Font->getImage();
		Int32 uSize = image->getUSize();
		Int32 vSize = image->getVSize();

		// For each character in string.
		for( Int32 i=0; i<Len; i++ )
		{
			// Handle whitespace glyph.
			if( Text[i] == L' ' )
			{
				const fnt::Glyph& Glyph = Font->getGlyph(L'a');
				Walk.x += Glyph.width * Scale.x * uSize;
				continue;
			}

			// Get glyph info.
			const fnt::Glyph& Glyph = Font->getGlyph(Text[i]);
			math::Vector CharSize = math::Vector( Glyph.width * Scale.x * uSize, Glyph.height * Scale.y * vSize );

			// Character location.
			R.Image			= image->getHandle();
			R.Bounds.min	= Walk;
			R.Bounds.max	= Walk + CharSize;

			// Glyph texture coords.
			R.TexCoords.min = math::Vector( Glyph.x, Glyph.y ) ;
			R.TexCoords.max = math::Vector( Glyph.x+Glyph.width, Glyph.y+Glyph.height );

			// Draw tile.
			DrawRect(R);

			// Walk to next.
			Walk.x += CharSize.x;
		}
	}
	else
	{
		// Fast version with render list.
#if 0
		// Setup list.
		TRenderList List( Len, Color );
		List.Image		= Font->Bitmaps.size() ? Font->Bitmaps[0]->m_image->getHandle() : INVALID_HANDLE<rend::Texture2DHandle>();
		List.Flags		= POLY_Unlit | POLY_Ghost;
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
#endif
	}
}


/*-----------------------------------------------------------------------------
    Stack of transformations.
-----------------------------------------------------------------------------*/

//
// Store old transformation and
// make current given new one.
//
void CCanvas::PushTransform( const gfx::ViewInfo& Info )
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
    TRenderList implementation.
-----------------------------------------------------------------------------*/

//
// Initialize render list for rectangles
// with various colors.
//
TRenderList::TRenderList( Int32 InNumRcts )
{
	Flags		= POLY_None;
	Image		= INVALID_HANDLE<rend::Texture2DHandle>();
	NumRects	= InNumRcts;
	Vertices	= (math::Vector*)CCanvas::GPool.Push(NumRects*4*sizeof(math::Rect));
	TexCoords	= (math::Vector*)CCanvas::GPool.Push(NumRects*4*sizeof(math::Rect));
	Colors		= (math::Color*)CCanvas::GPool.Push(NumRects*4*sizeof(math::Color));
}


//
// Initialize render list for rectangles
// with shared color.
//
TRenderList::TRenderList( Int32 InNumRcts, math::Color InColor )
{
	Flags		= POLY_None;
	Image		= INVALID_HANDLE<rend::Texture2DHandle>();
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