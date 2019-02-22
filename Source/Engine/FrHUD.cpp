/*=============================================================================
    FrHUD.cpp: HUD rendering component.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FPainterComponent implementation.
-----------------------------------------------------------------------------*/

//
// HUD component constructor.
//
FPainterComponent::FPainterComponent()
	:	FExtraComponent(),
		NextPainter( nullptr )
{
	Width			= 0.f;
	Height			= 0.f;
	Canvas			= nullptr;
	Color			= COLOR_White;
	Font			= nullptr;
	Texture			= nullptr;
	mem::zero( Effect, sizeof(Effect) );
}


//
// HUD component destructor.
//
FPainterComponent::~FPainterComponent()
{
	com_remove(Painter);
}


//
// Initialize HUD for level.
// 
void FPainterComponent::InitForEntity( FEntity* InEntity )
{
	FExtraComponent::InitForEntity(InEntity);
	com_add(Painter);
}


//
// Serialize painter stuff.
//
void FPainterComponent::SerializeThis( CSerializer& S )
{
	FExtraComponent::SerializeThis(S);

	// Serialize only references for GC.
	if( S.GetMode() == SM_Undefined )
	{
		Serialize( S, Font );
		Serialize( S, Texture );
	}
}


//
// Let's render HUD.
//
void FPainterComponent::RenderHUD( CCanvas* InCanvas )
{
	// Setup fields, for access from script.
	Canvas		= InCanvas;
	Width		= Canvas->View.Width;
	Height		= Canvas->View.Height;
	Effect[0]	= Effect[1]	= Effect[2]	= 1.f;
	Effect[3]	= Effect[4]	= Effect[5]	= 1.f;
	Effect[6]	= Effect[7]	= Effect[8]	= 0.f;
	Effect[9]	= 0.f;

	// Setup view info for valid screen to world
	// transform.
	ViewInfo	= TViewInfo
	(
		Level->Camera.Location,
		Level->Camera.Rotation,
		Level->Camera.GetFitFOV( Width, Height ),
		Level->Camera.Zoom,
		false,
		Canvas->View.X, Canvas->View.Y,
		Width, Height
	);

	// Let's script actually draw HUD.
	Entity->OnRender();
}


//
// Draw a colored point on HUD.
//
void FPainterComponent::nativePoint( CFrame& Frame )
{
	math::Vector P	= POP_VECTOR;
	Float	S	= POP_FLOAT;

	if( Canvas )
		Canvas->DrawPoint( P, S, Color );
}


//
// Draw a colored line on HUD.
//
void FPainterComponent::nativeLine( CFrame& Frame )
{
	math::Vector	A	= POP_VECTOR,
			B	= POP_VECTOR;

	if( Canvas )
		Canvas->DrawLine( A, B, Color, false );
}


//
// Draw a textured tile.
//
void FPainterComponent::nativeTile( CFrame& Frame )
{
	math::Vector	P	= POP_VECTOR,
			PL	= POP_VECTOR,
			T	= POP_VECTOR,
			TL	= POP_VECTOR;

	if( Canvas )
	{
		TRenderRect	R;
		R.Bounds.min	= P;
		R.Bounds.max	= P + PL;

		R.Color			= Color;
		R.Rotation		= 0;

		if( Texture )
		{
			R.Texture		= Texture;
			R.Flags			= POLY_Unlit;

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
			R.TexCoords.min.x	= T.x * Rescale[Texture->UBits];
			R.TexCoords.min.y	= T.y * Rescale[Texture->VBits];
			R.TexCoords.max.x	= (T.x+TL.x)  * Rescale[Texture->UBits];
			R.TexCoords.max.y	= (T.y+TL.y) * Rescale[Texture->VBits];
		}
		else
		{
			R.Texture		= nullptr;
			R.Flags			= POLY_Unlit | POLY_FlatShade;
		}

		Canvas->DrawRect(R);
	}
}


//
// Draw text.
//
void FPainterComponent::nativeTextOut( CFrame& Frame )
{
	math::Vector P	= POP_VECTOR;
	String	T	= POP_STRING;
	Float	S	= POP_FLOAT;

	if( Canvas && Font )
		Canvas->DrawText( *T, T.Len(), Font, Color, P, math::Vector( S, S ) );
}


//
// Return text width.
//
void FPainterComponent::nativeTextSize( CFrame& Frame )
{
	String	S	= POP_STRING;
	*POPA_FLOAT	= Font ? Font->TextWidth(*S) : 0.f;
}


//
// Restore level's global effect.
//
void FPainterComponent::nativePopEffect( CFrame& Frame )
{
	Level->GFXManager->PopEffect(POP_FLOAT);
}


//
// Set current effect.
//
void FPainterComponent::nativePushEffect( CFrame& Frame )
{
	Level->GFXManager->PushEffect( Effect, POP_FLOAT );
}


//
// Transform a point in the world's coords to the screen
// coords system.
//
void FPainterComponent::nativeProject( CFrame& Frame )
{
	Float	X, Y;
	ViewInfo.Project( POP_VECTOR, X, Y );
	*POPA_VECTOR	= math::Vector( X, Y );
}


//
// Transform a point in the screen's coords to the world
// coords system.
//
void FPainterComponent::nativeDeproject( CFrame& Frame )
{
	math::Vector V		= POP_VECTOR;
	*POPA_VECTOR	= ViewInfo.Deproject( V.x, V.y );
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FPainterComponent, FExtraComponent, CLASS_Sterile | CLASS_SingleComp )
{
	ADD_PROPERTY( Width, PROP_None );
	ADD_PROPERTY( Height, PROP_None );
	ADD_PROPERTY( Color, PROP_None );
	ADD_PROPERTY( Font, PROP_None );
	ADD_PROPERTY( Texture, PROP_None );
	ADD_PROPERTY( Effect, PROP_None );

	DECLARE_METHOD( Point, TYPE_None, ARG(position, TYPE_Vector, ARG(size, TYPE_Float, END)) );
	DECLARE_METHOD( Line, TYPE_None, ARG(from, TYPE_Vector, ARG(to, TYPE_Vector, END)) );
	DECLARE_METHOD( TextOut, TYPE_None, ARG(position, TYPE_Vector, ARG(text, TYPE_String, ARG(scale, TYPE_Float, END))) );
	DECLARE_METHOD( TextSize, TYPE_Float, ARG(text, TYPE_String, END) );
	DECLARE_METHOD( Tile, TYPE_None, ARG(position, TYPE_Vector, ARG(size, TYPE_Vector, ARG(tex0, TYPE_Vector, ARG(texSize, TYPE_Vector, END)))) );
	DECLARE_METHOD( PushEffect, TYPE_None, ARG(fadeTime, TYPE_Float, END) );
	DECLARE_METHOD( PopEffect, TYPE_None, ARG(fadeTime, TYPE_Float, END) );
	DECLARE_METHOD( Project, TYPE_Vector, ARG(worldPoint, TYPE_Vector, END) );
	DECLARE_METHOD( Deproject, TYPE_Vector, ARG(screenPoint, TYPE_Vector, END) );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/