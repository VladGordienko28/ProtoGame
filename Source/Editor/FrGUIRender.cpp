/*=============================================================================
    FrGUIRender.cpp: Editor GUI render.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    CGUIRender implementation.
-----------------------------------------------------------------------------*/

//
// GUI render constructor.
//
CGUIRender::CGUIRender()
	:	Canvas( nullptr ),
		Brightness( 1.f )
{
}


//
// GUI render destructor.
//
CGUIRender::~CGUIRender()
{
}


//
// Prepare for GUI rendering.
//
void CGUIRender::BeginPaint( CCanvas* InCanvas )
{
	Canvas	= InCanvas;

	// Set window coords system.
	Canvas->SetTransform( 
							TViewInfo
							( 
								0.f, 
								0.f, 
								Canvas->ScreenWidth, 
								Canvas->ScreenHeight 
							) 
						);
}


//
// End GUI rendering.
//
void CGUIRender::EndPaint()
{
	Canvas->SetClip( CLIP_NONE );
	Canvas = nullptr;
}


//
// Draw a rectangle.
//
void CGUIRender::DrawRegion( TPoint P, TSize S, TColor Color, TColor BorderColor, EBrushPattern Pattern )
{		
	// Pattern flag remap.
	static const DWord PatternFlag[BPAT_MAX] =
	{
		POLY_None,
		POLY_None,
		POLY_StippleI,
		POLY_StippleII
	};

	// Apply color rescale.
	if( Brightness != 1.f )
	{
		Color		*= Brightness;
		BorderColor	*= Brightness;
	}

	if( Color == BorderColor && Pattern == BPAT_Solid )
	{
		//
		// Case 1: Draw fully solid rect.
		//
		TRenderRect Rect;
		Rect.Texture	= nullptr;
		Rect.Bounds.Min	= TVector( P.X, P.Y );
		Rect.Bounds.Max	= TVector( P.X + S.Width, P.Y + S.Height );
		Rect.Color		= Color;
		Rect.Flags		= POLY_FlatShade;
		Rect.Rotation	= 0;

		Canvas->DrawRect( Rect );
	}
	else if( Pattern == BPAT_None )
	{
		//
		// Case 2: Just draw border.
		//
		TRect Rect;
		Rect.Min	= TVector( P.X, P.Y );
		Rect.Max	= TVector( P.X + S.Width, P.Y + S.Height );

		Canvas->DrawLineRect
		( 
			Rect.Center(), 
			Rect.Size(), 
			0, 
			BorderColor, 
			false
		);
	}
	else if( Pattern == BPAT_Solid )
	{
		//
		// Case 3: Draw two overlap rectangles.
		//
		TRenderRect Rect;
		Rect.Texture	= nullptr;
		Rect.Bounds.Min	= TVector( P.X, P.Y );
		Rect.Bounds.Max	= TVector( P.X + S.Width, P.Y + S.Height );
		Rect.Color		= BorderColor;
		Rect.Flags		= POLY_FlatShade;
		Rect.Rotation	= 0;

		Canvas->DrawRect( Rect );

		Rect.Bounds.Min.X++;
		Rect.Bounds.Min.Y++;
		Rect.Bounds.Max.X--;
		Rect.Bounds.Max.Y--;
		Rect.Color		= Color;

		Canvas->DrawRect( Rect );
	}
	else
	{
		//
		// Case 4: Draw pattern.
		//
		TRenderRect Rect;
		Rect.Texture	= nullptr;
		Rect.Bounds.Min	= TVector( P.X, P.Y );
		Rect.Bounds.Max	= TVector( P.X + S.Width, P.Y + S.Height );
		Rect.Color		= BorderColor;
		Rect.Flags		= POLY_FlatShade | PatternFlag[Pattern];
		Rect.Rotation	= 0;

		Canvas->DrawRect( Rect );

		Canvas->DrawLineRect
		( 
			Rect.Bounds.Center(), 
			Rect.Bounds.Size(), 
			0, 
			BorderColor, 
			false
		);
	}
}



//
// Draw GUI picture.
//
void CGUIRender::DrawPicture( TPoint P, TSize S, TPoint BP, TSize BS, FTexture* Texture )
{	
	// Setup rect.
	TRenderRect Rect;
	Rect.Texture	= Texture;
	Rect.Bounds.Min	= TVector( P.X, P.Y );
	Rect.Bounds.Max	= TVector( P.X + S.Width, P.Y + S.Height );
	Rect.Color		= Brightness != 1.f ? COLOR_White*Brightness : COLOR_White;
	Rect.Color.A	= 0xff;
	Rect.Flags		= POLY_Unlit;	
	Rect.Rotation	= 0;	
	
	// Division table, used to reduce multiple
	// divisions, since gui has many icons to draw.
	static const Float Rescale[] =
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

	// Texture coords.
	Rect.TexCoords.Min.X	= BP.X * Rescale[Texture->UBits];
	Rect.TexCoords.Min.Y	= BP.Y * Rescale[Texture->VBits];
	Rect.TexCoords.Max.X	= (BP.X+BS.Width)  * Rescale[Texture->UBits];
	Rect.TexCoords.Max.Y	= (BP.Y+BS.Height) * Rescale[Texture->VBits];

	// Draw it.
	Canvas->DrawRect( Rect );
}


//
// Draw a GUI text.
//
void CGUIRender::DrawText( TPoint P, const Char* Text, Integer Len, TColor Color, FFont* Font )
{
	if( Brightness != 1.f )
		Color *= Brightness;
	Canvas->DrawText( Text, Len, Font, Color, TVector(P.X, P.Y) );
}


//
// Set render clipping area.
//
void CGUIRender::SetClipArea( TPoint P, TSize S )
{
	Canvas->SetClip
	( 
		TClipArea
		( 
			P.X, P.Y, 
			S.Width, S.Height 
		) 
	);
}


//
// Set a brightness for gui elements
// rendering.
//
void CGUIRender::SetBrightness( Float Brig )
{
	Brightness	= Clamp( Brig, 0.f, 1.f );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/
