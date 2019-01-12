/*=============================================================================
    FrGFX.cpp: Post-effects interpolator.
    Copyright Oct.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    CGFXManager implementation.
-----------------------------------------------------------------------------*/

//
// GFX manager constructor.
//
CGFXManager::CGFXManager( FLevel* InLevel )
{
	assert(InLevel);

	BaseEffect		= InLevel->Effect;
	ResultEffect	= BaseEffect;

	// Reset interpolation variables.
	Fade		= 0.f;
	Alpha		= 1.f;
	OldEffect	= ResultEffect;
	NewEffect	= ResultEffect;
}


//
// GFX manager destructor.
//
CGFXManager::~CGFXManager()
{
}


//
// Set effect to interpolate.
//
void CGFXManager::PushEffect( const TPostEffect& InEffect, Float FadeTime )
{
	if( FadeTime <= 0.f )
	{
		// Instant effect, no interpolation required.
		Alpha			= 1.f;
		Fade			= 0.f;
		ResultEffect	=
		NewEffect		= InEffect;
	}
	else
	{
		// Prepare for battle!.., ..I mean interpolation :)
		Alpha			= 0.f;
		Fade			= FadeTime;
		OldEffect		= ResultEffect;
		NewEffect		= InEffect;
	}
}


//
// Interpolate gfx.
//
void CGFXManager::Tick( Float Delta )
{
	// Process interpolation while Alpha not reached 100%
	if( Alpha <= 0.99999f )
	{
		Alpha	= Clamp( Alpha, 0.f, 1.f );

		// Interpolate all.
		for( Integer i=0; i<3; i++ )
		{
			ResultEffect.Highlights[i]	= Lerp( OldEffect.Highlights[i],	NewEffect.Highlights[i],	Alpha );
			ResultEffect.MidTones[i]	= Lerp( OldEffect.MidTones[i],		NewEffect.MidTones[i],		Alpha );
			ResultEffect.Shadows[i]		= Lerp( OldEffect.Shadows[i],		NewEffect.Shadows[i],		Alpha );
		}
		ResultEffect.BWScale			= Lerp( OldEffect.BWScale,			NewEffect.BWScale,			Alpha );

		// Update alpha.
		Alpha	+= Delta/Fade;
	}
	else
	{
		// 100% of new effect.
		ResultEffect	= NewEffect;
	}
}


//
// Restore default level base effect.
//
void CGFXManager::PopEffect( Float FadeTime )
{
	PushEffect( BaseEffect, FadeTime );
}


//
// Return computed effect.
//
TPostEffect& CGFXManager::GetResult()
{
	return ResultEffect;
}


/*-----------------------------------------------------------------------------
    TPostEffect implementation.
-----------------------------------------------------------------------------*/

//
// Post effect constructor.
//
TPostEffect::TPostEffect()
{
	Highlights[0]	= 1.f;
	Highlights[1]	= 1.f;
	Highlights[2]	= 1.f;

	MidTones[0]		= 1.f;
	MidTones[1]		= 1.f;
	MidTones[2]		= 1.f;

	Shadows[0]		= 0.f;
	Shadows[1]		= 0.f;
	Shadows[2]		= 0.f;

	BWScale			= 0.f;
}


//
// Post effect copy-constructor.
//
TPostEffect::TPostEffect( const Float* EffArr )
{
	Highlights[0]	= EffArr[0];
	Highlights[1]	= EffArr[1];
	Highlights[2]	= EffArr[2];

	MidTones[0]		= EffArr[3];
	MidTones[1]		= EffArr[4];
	MidTones[2]		= EffArr[5];

	Shadows[0]		= EffArr[6];
	Shadows[1]		= EffArr[7];
	Shadows[2]		= EffArr[8];

	BWScale			= EffArr[9];
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/