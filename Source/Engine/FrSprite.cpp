/*=============================================================================
    FrSprite.cpp: Sprite components.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FSpriteComponent implementation.
-----------------------------------------------------------------------------*/
//
// A global division table, used to reduce multiple
// divisions, when convert pixel's coords to texture's.
//
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


//
// Sprite constructor.
//
FSpriteComponent::FSpriteComponent()
	:	FExtraComponent(),
		bHidden( false ),
		bUnlit( false ),
		bFlipH( false ),
		bFlipV( false ),
		Color( math::colors::WHITE ),
		Texture( nullptr )
{
	bRenderable		= true;

	Offset			= math::Vector( 0.f, 0.f );
	Scale			= math::Vector( 1.f, 1.f );
	Rotation		= math::Angle( 0 );
	TexCoords.min	= math::Vector( 0.f, 0.f );
	TexCoords.max	= math::Vector( 16.f, 16.f );
}


//
// Serialize sprite component.
//
void FSpriteComponent::SerializeThis( CSerializer& S )
{
	FExtraComponent::SerializeThis( S );
	Serialize( S, bHidden );	
	Serialize( S, bUnlit );
	Serialize( S, bFlipH );
	Serialize( S, bFlipV );
	Serialize( S, Color );
	Serialize( S, Texture );
	Serialize( S, Offset );
	Serialize( S, Scale );
	Serialize( S, Rotation );
	Serialize( S, TexCoords );
}


//
// Render the sprite.
//
void FSpriteComponent::Render( CCanvas* Canvas )
{
	// Test hidden.
	if( bHidden && !(Level->RndFlags & RND_Hidden) )
		return;

	// Precompute.
	math::Vector Location	= Offset + Base->Location;
	math::Vector Size		= math::Vector( Base->Size.x*Scale.x, Base->Size.y*Scale.y );

	// Sprite is visible?
	math::Rect Bounds = math::Rect( Location, Size.x, Size.y );
	if( !Canvas->View.Bounds.isOverlap(math::Rect( Location, math::sqrt(Size.x*Size.x+Size.y*Size.y) )) )
		return;

	// Initialize rect.
	TRenderRect Rect;
	Rect.Flags			= POLY_Unlit * (bUnlit | !(Level->RndFlags & RND_Lighting));
	Rect.Rotation		= Base->Rotation + Rotation;		
	Rect.Color			= Base->bSelected ? math::Color( 0x80, 0xe6, 0x80, 0xff ) : Color;
	Rect.Texture		= Texture ? Texture : FBitmap::NullBitmap();
	Rect.Bounds			= Bounds;
	
	// Integer texture coords to float.
	Rect.TexCoords.min.x	= TexCoords.min.x * GRescale[Rect.Texture->UBits];
	Rect.TexCoords.max.x	= TexCoords.max.x * GRescale[Rect.Texture->UBits];
	Rect.TexCoords.min.y	= TexCoords.max.y * GRescale[Rect.Texture->VBits];
	Rect.TexCoords.max.y	= TexCoords.min.y * GRescale[Rect.Texture->VBits];	

	if( Base->bFrozen )
		Rect.Color	= Rect.Color * 0.55f;

	// Apply flipping.
	if( bFlipH )
		exchange( Rect.TexCoords.min.x, Rect.TexCoords.max.x );
	if( bFlipV )
		exchange( Rect.TexCoords.min.y, Rect.TexCoords.max.y );

	// Draw it!
	Canvas->DrawRect( Rect );
}


/*-----------------------------------------------------------------------------
    FDecoComponent implementation.
-----------------------------------------------------------------------------*/

//
// Deco component constructor.
// 
FDecoComponent::FDecoComponent()
	:	FExtraComponent(),
		bUnlit( false ),
		bFlipH( false ),
		bFlipV( false ),
		Color( math::colors::WHITE ),
		Texture( nullptr ),
		DecoType( DECO_Shrub ),
		Frequency( 1.f ),
		Amplitude( 1.f )		
{
	bRenderable		= true;
	TexCoords.min	= math::Vector( 0.f, 0.f );
	TexCoords.max	= math::Vector( 16.f, 16.f );
}


//
// Validate parameters.
//
void FDecoComponent::EditChange()
{	
	FExtraComponent::EditChange();

	// Clamp parameters.
	Amplitude	= clamp( Amplitude, -100.f, +100.f );
	Frequency	= clamp( Frequency, -100.f, +100.f );
}


//
// Serialize component.
//
void FDecoComponent::SerializeThis( CSerializer& S )
{
	FExtraComponent::SerializeThis( S );	
	Serialize( S, bUnlit );
	Serialize( S, bFlipH );
	Serialize( S, bFlipV );
	Serialize( S, Color );
	Serialize( S, Texture );
	SerializeEnum( S, DecoType );
	Serialize( S, Frequency );
	Serialize( S, Amplitude );
	Serialize( S, TexCoords );
}


//
// Render decoration sprite.
//
void FDecoComponent::Render( CCanvas* Canvas )
{
	// Prepare.
	math::Vector Scale	= Base->Size;
	Float	Now		= GPlat->Now();

	// Is visible?
	math::Rect Bounds = math::Rect( Base->Location, max(Scale.x, Scale.y)*2.f );
	if( !Canvas->View.Bounds.isOverlap( Bounds ) )
		return;

	// Initialize polygon.
	TRenderPoly Poly;
	Poly.Texture		= Texture ? Texture : FBitmap::NullBitmap();
	Poly.Color			= Base->bSelected ? math::Color( 0x80, 0xe6, 0x80, 0xff ) : Color;
	Poly.Flags			= POLY_Unlit * (bUnlit | !(Level->RndFlags & RND_Lighting));
	Poly.NumVerts		= 4;

	// Texture coords.
	Float	U1	= TexCoords.min.x * GRescale[Poly.Texture->UBits],
			U2	= TexCoords.max.x * GRescale[Poly.Texture->UBits],
			V1	= TexCoords.min.y * GRescale[Poly.Texture->VBits],
			V2	= TexCoords.max.y * GRescale[Poly.Texture->VBits];

	if( bFlipH )
		exchange( U1, U2 );
	if( bFlipV )
		exchange( V1, V2 );

	Poly.TexCoords[0]	= math::Vector( U1, V2 );
	Poly.TexCoords[1]	= math::Vector( U1, V1 );
	Poly.TexCoords[2]	= math::Vector( U2, V1 );
	Poly.TexCoords[3]	= math::Vector( U2, V2 );

	// Untransformed vertices in local coords.
	Poly.Vertices[0]	= math::Vector( -Scale.x, -Scale.y )*0.5;
	Poly.Vertices[1]	= math::Vector( -Scale.x, +Scale.y )*0.5;
	Poly.Vertices[2]	= math::Vector( +Scale.x, +Scale.y )*0.5;
	Poly.Vertices[3]	= math::Vector( +Scale.x, -Scale.y )*0.5;
	  
	// Apply distortion effect.
	switch ( DecoType )
	{
		case DECO_Shrub:
		{
			// Windy shrub.
			Float Sine		= math::sin(Now*Frequency) * Amplitude;
			Float Cosine	= math::cos(Now*Frequency) * Amplitude;

			Poly.Vertices[1] += math::Vector( +Cosine, +Sine );
			Poly.Vertices[2] += math::Vector( -Cosine, +Sine );
			break;
		}
		case DECO_Trunc:
		{
			// Tree trunc.
			Float Phase		= math::sin(Now*Frequency) * Amplitude * (math::PI/8.f);
			math::Coords Matrix	= math::Coords( math::Vector( Base->Location.x, Base->Location.y-Scale.y*0.5f ), Phase );

			Poly.Vertices[1]	= math::transformVectorBy( Poly.Vertices[1], Matrix ); 
			Poly.Vertices[2]	= math::transformVectorBy( Poly.Vertices[2], Matrix );
			
			break;
		}
		case DECO_Liana:
		{
			// Waver liana.
			Float Phase		= math::sin(Now*Frequency) * Amplitude * (math::PI/8.f);
			math::Coords Matrix	= math::Coords( math::Vector( Base->Location.x, Base->Location.y+Scale.y*0.5f ), Phase );

			Poly.Vertices[0]	= math::transformVectorBy( Poly.Vertices[0], Matrix ); 
			Poly.Vertices[3]	= math::transformVectorBy( Poly.Vertices[3], Matrix );

			break;
		}
	}

	// Transform to world coords.
	Poly.Vertices[0]	+= Base->Location;
	Poly.Vertices[1]	+= Base->Location;
	Poly.Vertices[2]	+= Base->Location;
	Poly.Vertices[3]	+= Base->Location;

	// Draw it!
	Canvas->DrawPoly( Poly );
}


/*-----------------------------------------------------------------------------
    FAnimatedSpriteComponent implementation.
-----------------------------------------------------------------------------*/

//
// Animated sprite constructor.
//
FAnimatedSpriteComponent::FAnimatedSpriteComponent()
	:	FExtraComponent(),
		bHidden( false ),
		bUnlit( false ),
		bFlipH( false ),
		bFlipV( false ),
		Animation( nullptr ),
		Color( math::colors::WHITE ),
		Offset( 0.f, 0.f ),
		Scale( 1.f, 1.f ),
		Rotation( 0 ),
		AnimType( ANIM_Once ),
		iSequence( 0 ),
		bBackward( false ),
		Rate( 0.f ),
		Frame( 0.f )
{
	bTickable	= true;
	bRenderable	= true;
}


//
// Process animation.
//
void FAnimatedSpriteComponent::Tick( Float Delta )
{
	// Does we can play?
	if( !Animation || iSequence >= Animation->Sequences.size() )
		return;

	// Prepare.
	TAnimSequence* Seq	= &Animation->Sequences[iSequence];
	Int32	MaxFrames	= Seq->Count-1, iFrame;

	// Process according type.
	switch( AnimType )
	{
		case ANIM_Once:
		{
			// Simple animation, just play it and stop with
			// OnAnimEnd notification.
			Frame		+= Rate * Delta;
			iFrame		= math::floor(Frame);

			if( iFrame > MaxFrames )
			{
				// Animation expired.
				Rate	= 0.f;
				Frame	= MaxFrames;
				Entity->OnAnimEnd();
			}
			break;
		}
		case ANIM_Loop:
		{
			// Looped animation.
			Frame		+= Rate * Delta;
			iFrame		= math::floor(Frame);

			if( iFrame > MaxFrames )
			{
				// Reloop animation.
				Frame	= 0.f;
			}
			break;
		}
		case ANIM_PingPong:
		{
			// Ping-pong animation.
			if( bBackward )
			{
				// Backward animation.
				Frame		+= Rate * Delta;
				iFrame		= math::floor(Frame);
				if( iFrame < 0 )
				{
					// Toggle to forward.
					bBackward	= false;
					Frame		= MaxFrames - 0.999f;
				}
			}
			else
			{
				// Forward animation.
				Frame		+= Rate * Delta;
				iFrame		= math::floor(Frame);
				if( iFrame > MaxFrames )
				{
					// Toggle to backward.
					bBackward	= true;
					Frame		= MaxFrames - 0.001f;
				}
			}
			break;
		}
	}
}


//
// Render sprite.
//
void FAnimatedSpriteComponent::Render( CCanvas* Canvas )
{
	// Test hidden.
	if( bHidden && !(Level->RndFlags & RND_Hidden) )
		return;

	// Sprite is visible?
	math::Vector DrawSize/*( Base->Size.X*Scale.X, Base->Size.Y*Scale.Y )*/ = Scale;
	math::Vector DrawPos( Base->Location.x+Offset.x, Base->Location.y+Offset.y );
	math::Rect Bounds( DrawPos, DrawSize.x, DrawSize.y );
	if( !Canvas->View.Bounds.isOverlap( Bounds ) )
		return;

	// Initialize rect struct.
	TRenderRect Rect;

	Rect.Flags			= POLY_Unlit * (bUnlit | !(Level->RndFlags & RND_Lighting));
	Rect.Rotation		= Base->Rotation + Rotation;
	Rect.Color			= Base->bSelected ? math::Color( 0x80, 0xe6, 0x80, 0xff ) : Color;
	Rect.Bounds			= Bounds;

	// Is animation valid?
	if( Animation && Animation->Sheet && iSequence < Animation->Sequences.size() )
	{
		// Draw animation.
		TAnimSequence* Seq	= &Animation->Sequences[iSequence];
		Int32 iDrawFrame	= math::floor(Frame) + Seq->Start;

		Rect.Texture	= Animation->Sheet;
		Rect.TexCoords	= Animation->GetTexCoords( iDrawFrame );
	}
	else
	{
		// Bad animation, draw chess board instead.
		Rect.Texture		= nullptr;
		Rect.TexCoords.min	= math::Vector( 0.f, 1.f );
		Rect.TexCoords.max	= math::Vector( 1.f, 0.f );
	}

	// Apply flipping.
	if( bFlipH )
		exchange( Rect.TexCoords.min.x, Rect.TexCoords.max.x );
	if( bFlipV )
		exchange( Rect.TexCoords.min.y, Rect.TexCoords.max.y );

	if( Base->bFrozen )
		Rect.Color	= Rect.Color * 0.55f;

	// Draw it!
	Canvas->DrawRect( Rect );

}


//
// Serialize animation.
//
void FAnimatedSpriteComponent::SerializeThis( CSerializer& S )
{
	FExtraComponent::SerializeThis( S );

	Serialize( S, bHidden );
	Serialize( S, bUnlit );
	Serialize( S, bFlipH );
	Serialize( S, bFlipV );
	Serialize( S, Animation );
	Serialize( S, Color );

	Serialize( S, Offset );
	Serialize( S, Scale );
	Serialize( S, Rotation );

	// Don't serialize animation
	// runtime info.
}


//
// Play an animation.
//
void FAnimatedSpriteComponent::nativePlayAnim( CFrame& Frame )
{
	// Pop arguments.
	String		ASeqName	= POP_STRING;
	Float		ARate		= POP_FLOAT;
	EAnimType	AType		= (EAnimType)POP_BYTE;

	if( !Animation )
	{
		debug( L"Anim: Error in '%s': No animation to play", *Entity->GetFullName() );
		return;
	}

	Int32 iSeq = Animation->FindSequence( ASeqName );
	if( iSeq == -1 )
	{
		debug( L"Anim: Sequence '%s' not found in '%s'", *ASeqName, *Animation->GetName() );
		return;
	}

	if( iSequence<Animation->Sequences.size() && iSeq == iSequence )
	{
		// Modify current play.
		if( Rate != ARate )
			this->Frame	= 0.f;

		Rate		= ARate;
		AnimType	= AType;
	}
	else
	{
		// Start not sequence.
		AnimType	= AType;
		iSequence	= iSeq;
		bBackward	= false;
		Rate		= ARate;
		this->Frame	= 0.f;
	}
}


//
// Return the name of current played sequence.
//
void FAnimatedSpriteComponent::nativeGetAnimName( CFrame& Frame )
{
	if( Animation && Rate != 0.f && iSequence < Animation->Sequences.size() )
	{
		*POPA_STRING	= Animation->Sequences[iSequence].Name;
	}
	else
		*POPA_STRING	= L"";
}


//
// Pause current animation.
//
void FAnimatedSpriteComponent::nativePauseAnim( CFrame& Frame )
{
	Rate	= 0.f;
}


//
// Return true, if animation is currently playing
//
void FAnimatedSpriteComponent::nativeIsPlaying( CFrame& Frame )
{
	*POPA_BOOL = Rate != 0.f;
}


/*-----------------------------------------------------------------------------
    FParallaxLayerComponent implementation.
-----------------------------------------------------------------------------*/

//
// Parallax layer component.
//
FParallaxLayerComponent::FParallaxLayerComponent()
	:	FExtraComponent(),
		bUnlit( false ),
		bFlipH( false ),
		bFlipV( false ),
		Color( math::colors::WHITE ),
		Texture( nullptr )		
{
	bRenderable		= true;
	Scale			= math::Vector( 20.f, 20.f );
	Parallax		= math::Vector( 0.05f, 0.05f );
	Gap				= math::Vector( 5.f, 5.f );
	Offset			= math::Vector( 0.f, 0.f );
	TexCoords.min	= math::Vector( 0.f, 0.f );
	TexCoords.max	= math::Vector( 16.f, 16.f );
}


//
// Parallax layer serialization.
//
void FParallaxLayerComponent::SerializeThis( CSerializer& S )
{
	FExtraComponent::SerializeThis( S );	
	Serialize( S, bUnlit );
	Serialize( S, bFlipH );
	Serialize( S, bFlipV );
	Serialize( S, Color );
	Serialize( S, Texture );
	
	Serialize( S, Scale );
	Serialize( S, Parallax );
	Serialize( S, Gap );
	Serialize( S, Offset );
	Serialize( S, TexCoords );
}


//
// When some field changed in parallax layer.
//
void FParallaxLayerComponent::EditChange()
{
	FExtraComponent::EditChange();
}


//
// Parallax layer rendering.
//
void FParallaxLayerComponent::Render( CCanvas* Canvas )
{
	// Don't draw parallax layers in mirage!
	if( Canvas->View.bMirage || !(Level->RndFlags & RND_Backdrop) )
		return;

	// How much tiles draw.
	math::Vector Period = Scale + Gap;
	math::Vector Area = Canvas->View.Bounds.size();
	Int32 NumX = math::ceil(Area.x / Period.x) + 1;
	Int32 NumY = math::ceil(Area.y / Period.y) + 1;

	// Compute parallax parameters.
	math::Vector Bias;
	Bias.x = Canvas->View.Coords.origin.x * Parallax.x + Offset.x;
	Bias.y = Canvas->View.Coords.origin.y * Parallax.y + Offset.y;
	
	Bias.x = fmod( Bias.x, Period.x );
	Bias.y = fmod( Bias.y, Period.y );

	// Setup render rect.
	TRenderRect Rect;
	Rect.Texture		= Texture ? Texture : FBitmap::NullBitmap();
	Rect.Color			= Base->bSelected ? math::Color( 0x80, 0xe6, 0x80, 0xff ) : Color;
	Rect.Flags			= POLY_Unlit * (bUnlit | !(Level->RndFlags & RND_Lighting));
	Rect.Rotation		= 0;

	Rect.TexCoords.min.x	= TexCoords.min.x * GRescale[Rect.Texture->UBits];
	Rect.TexCoords.max.x	= TexCoords.max.x * GRescale[Rect.Texture->UBits];
	Rect.TexCoords.min.y	= TexCoords.max.y * GRescale[Rect.Texture->VBits];
	Rect.TexCoords.max.y	= TexCoords.min.y * GRescale[Rect.Texture->VBits];	

	// Apply flipping.
	if( bFlipH )
		exchange( Rect.TexCoords.min.x, Rect.TexCoords.max.x );
	if( bFlipV )
		exchange( Rect.TexCoords.min.y, Rect.TexCoords.max.y );

	// Draw all rectangles.
	for( Int32 Y=0; Y<NumY; Y++ )
	for( Int32 X=0; X<NumX; X++ )		
	{
		math::Vector Origin = Canvas->View.Bounds.min - Bias + math::Vector( X*Period.x, Y*Period.y ) - Scale*0.5f;
		Rect.Bounds	= math::Rect( Origin, Scale.x, Scale.y );
		Canvas->DrawRect( Rect );
	}
}


/*-----------------------------------------------------------------------------
    FLabelComponent implementation.
-----------------------------------------------------------------------------*/

//
// Label component constructor.
//
FLabelComponent::FLabelComponent()
	:	FExtraComponent()
{
	bRenderable	= true;
	Color		= math::colors::WHITE;
	Font		= nullptr;
	Scale		= 1.f;
	Text		= L"Text";
}


//
// Label addon rendering.
//
void FLabelComponent::Render( CCanvas* Canvas )
{
	// Reject if any.
	if( Canvas->View.bMirage || !Font || !Text )
		return;

	TViewInfo OldView = Canvas->View;
	Canvas->PushTransform(TViewInfo( OldView.X, OldView.Y, OldView.Width, OldView.Height ));
	{
		math::Vector	P;
		Float	W = Font->TextWidth(*Text)*Scale, H = Font->Glyphs[0].H*Scale;
		OldView.Project( Base->Location, P.x, P.y );
		
		Canvas->DrawText
		(
			Text,
			Font,
			Color,
			math::Vector( P.x-W*0.5f, P.y-H*0.5f ),
			math::Vector( Scale, Scale )
		);
	}
	Canvas->PopTransform();
}


//
// Label serialization.
//
void FLabelComponent::SerializeThis( CSerializer& S )
{
	FExtraComponent::SerializeThis( S );	
	Serialize( S, Color );
	Serialize( S, Text );
	Serialize( S, Font );
	Serialize( S, Scale );
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FSpriteComponent, FExtraComponent, CLASS_None )
{
	ADD_PROPERTY( Offset, PROP_None );
	ADD_PROPERTY( Scale, PROP_None );
	ADD_PROPERTY( Rotation, PROP_None );
	ADD_PROPERTY( TexCoords, PROP_Editable );
	ADD_PROPERTY( bHidden, PROP_Editable );
	ADD_PROPERTY( bUnlit, PROP_Editable );
	ADD_PROPERTY( bFlipH, PROP_Editable );
	ADD_PROPERTY( bFlipV, PROP_Editable );
	ADD_PROPERTY( Color, PROP_Editable );
	ADD_PROPERTY( Texture, PROP_Editable );
}


REGISTER_CLASS_CPP( FDecoComponent, FExtraComponent, CLASS_None )
{
	BEGIN_ENUM(EDecoType)
		ENUM_ELEM(DECO_None);
		ENUM_ELEM(DECO_Trunc);
		ENUM_ELEM(DECO_Shrub);
		ENUM_ELEM(DECO_Liana);
	END_ENUM;

	ADD_PROPERTY( DecoType, PROP_Editable );
	ADD_PROPERTY( Frequency, PROP_Editable );	
	ADD_PROPERTY( Amplitude, PROP_Editable );
	ADD_PROPERTY( TexCoords, PROP_Editable );
	ADD_PROPERTY( bUnlit, PROP_Editable );
	ADD_PROPERTY( bFlipH, PROP_Editable );
	ADD_PROPERTY( bFlipV, PROP_Editable );
	ADD_PROPERTY( Color, PROP_Editable );
	ADD_PROPERTY( Texture, PROP_Editable );
}


REGISTER_CLASS_CPP( FAnimatedSpriteComponent, FExtraComponent, CLASS_None )
{
	BEGIN_ENUM(EAnimType)
		ENUM_ELEM(ANIM_Once);
		ENUM_ELEM(ANIM_Loop);
		ENUM_ELEM(ANIM_PingPong);
	END_ENUM;

	ADD_PROPERTY( bHidden, PROP_None );
	ADD_PROPERTY( Color, PROP_None );
	ADD_PROPERTY( bUnlit, PROP_Editable );
	ADD_PROPERTY( bFlipH, PROP_Editable );
	ADD_PROPERTY( bFlipV, PROP_Editable );
	ADD_PROPERTY( Animation, PROP_Editable );
	ADD_PROPERTY( Offset, PROP_Editable );
	ADD_PROPERTY( Scale, PROP_Editable );
	ADD_PROPERTY( Rotation, PROP_Editable );
	ADD_PROPERTY( iSequence, PROP_None | PROP_NoImEx );
	ADD_PROPERTY( Frame, PROP_None | PROP_NoImEx );

	DECLARE_METHOD( PlayAnim, TYPE_None, ARG(name, TYPE_String, ARG(rate, TYPE_Float, ARG(type, TYPE_Byte, END))) );
	DECLARE_METHOD( GetAnimName, TYPE_String, END );
	DECLARE_METHOD( PauseAnim, TYPE_None, END );
	DECLARE_METHOD( IsPlaying, TYPE_Bool, END );
}


REGISTER_CLASS_CPP( FParallaxLayerComponent, FExtraComponent, CLASS_SingleComp )
{
	ADD_PROPERTY( Parallax, PROP_Editable );
	ADD_PROPERTY( Scale, PROP_Editable );
	ADD_PROPERTY( Offset, PROP_Editable );
	ADD_PROPERTY( Gap, PROP_Editable );
	ADD_PROPERTY( TexCoords, PROP_Editable );
	ADD_PROPERTY( bUnlit, PROP_Editable );
	ADD_PROPERTY( bFlipH, PROP_Editable );
	ADD_PROPERTY( bFlipV, PROP_Editable );
	ADD_PROPERTY( Color, PROP_Editable );
	ADD_PROPERTY( Texture, PROP_Editable );
}


REGISTER_CLASS_CPP( FLabelComponent, FExtraComponent, CLASS_None )
{
	ADD_PROPERTY( Color, PROP_None );
	ADD_PROPERTY( Font, PROP_None );
	ADD_PROPERTY( Text, PROP_Editable );
	ADD_PROPERTY( Scale, PROP_Editable );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/