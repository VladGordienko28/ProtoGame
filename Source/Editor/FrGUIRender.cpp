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
							gfx::ViewInfo
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

	FlushText();
	Canvas->SetClip( CLIP_NONE );
	Canvas = nullptr;
}


//
// Draw a rectangle.
//
void CGUIRender::DrawRegion( TPoint P, TSize S, math::Color Color, math::Color BorderColor, EBrushPattern Pattern )
{		
	// Pattern flag remap.
	static const UInt32 PatternFlag[BPAT_MAX] =
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
		Rect.Image		= INVALID_HANDLE<rend::Texture2DHandle>();
		Rect.Bounds.min	= math::Vector( P.X, P.Y );
		Rect.Bounds.max	= math::Vector( P.X + S.Width, P.Y + S.Height );
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
		math::Rect Rect;
		Rect.min	= math::Vector( P.X, P.Y );
		Rect.max	= math::Vector( P.X + S.Width, P.Y + S.Height );

		Canvas->DrawLineRect
		( 
			Rect.center(), 
			Rect.size(), 
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
		Rect.Image		= INVALID_HANDLE<rend::Texture2DHandle>();
		Rect.Bounds.min	= math::Vector( P.X, P.Y );
		Rect.Bounds.max	= math::Vector( P.X + S.Width, P.Y + S.Height );
		Rect.Color		= BorderColor;
		Rect.Flags		= POLY_FlatShade;
		Rect.Rotation	= 0;

		Canvas->DrawRect( Rect );

		Rect.Bounds.min.x++;
		Rect.Bounds.min.y++;
		Rect.Bounds.max.x--;
		Rect.Bounds.max.y--;
		Rect.Color		= Color;

		Canvas->DrawRect( Rect );
	}
	else
	{
		//
		// Case 4: Draw pattern.
		//
		TRenderRect Rect;
		Rect.Image		= INVALID_HANDLE<rend::Texture2DHandle>();
		Rect.Bounds.min	= math::Vector( P.X, P.Y );
		Rect.Bounds.max	= math::Vector( P.X + S.Width, P.Y + S.Height );
		Rect.Color		= BorderColor;
		Rect.Flags		= POLY_FlatShade | PatternFlag[Pattern];
		Rect.Rotation	= 0;

		Canvas->DrawRect( Rect );

		Canvas->DrawLineRect
		( 
			Rect.Bounds.center(), 
			Rect.Bounds.size(), 
			0, 
			BorderColor, 
			false
		);
	}
}


void CGUIRender::DrawImage( TPoint P, TSize S, TPoint BP, TSize BS, img::Image::Ptr image )
{
	// Setup rect.
	TRenderRect Rect;
	Rect.Image		= image->getHandle();
	Rect.Bounds.min	= math::Vector( P.X, P.Y );
	Rect.Bounds.max	= math::Vector( P.X + S.Width, P.Y + S.Height );
	Rect.Color		= Brightness != 1.f ? math::colors::WHITE*Brightness : math::colors::WHITE;
	Rect.Color.a	= 0xff;
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
	Rect.TexCoords.min.x	= BP.X * Rescale[image->getUBits()];
	Rect.TexCoords.min.y	= BP.Y * Rescale[image->getVBits()];
	Rect.TexCoords.max.x	= (BP.X+BS.Width)  * Rescale[image->getUBits()];
	Rect.TexCoords.max.y	= (BP.Y+BS.Height) * Rescale[image->getVBits()];

	// Draw it.
	Canvas->DrawRect( Rect );
}

void CGUIRender::DrawTexture( TPoint P, TSize S, TPoint BP, TSize BS, rend::Texture2DHandle image, UInt32 width, UInt32 height )
{
	// Setup rect.
	TRenderRect Rect;
	Rect.Image		= image;
	Rect.Bounds.min	= math::Vector( P.X, P.Y );
	Rect.Bounds.max	= math::Vector( P.X + S.Width, P.Y + S.Height );
	Rect.Color		= Brightness != 1.f ? math::colors::WHITE*Brightness : math::colors::WHITE;
	Rect.Color.a	= 0xff;
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

	UInt8 uBits = intLog2( width );
	UInt8 vBits = intLog2( height );



	// Texture coords.
	Rect.TexCoords.min.x	= BP.X * Rescale[uBits];
	Rect.TexCoords.min.y	= BP.Y * Rescale[vBits];
	Rect.TexCoords.max.x	= (BP.X+BS.Width)  * Rescale[uBits];
	Rect.TexCoords.max.y	= (BP.Y+BS.Height) * Rescale[vBits];

	// Draw it.
	Canvas->DrawRect( Rect );
}


//
// Draw a GUI text.
//
void CGUIRender::DrawText( TPoint P, const Char* Text, Int32 Len, math::Color Color, fnt::Font::Ptr Font )
{
	if( Brightness != 1.f )
		Color *= Brightness;

	m_textDrawer.batchText( Text, Len, Font, Color, math::Vector(P.X, P.Y) );
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
	Brightness	= clamp( Brig, 0.f, 1.f );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/
