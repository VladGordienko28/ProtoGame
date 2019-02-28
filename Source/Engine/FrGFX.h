/*=============================================================================
    FrGFX.h: A post effects base structures and classes.
    Copyright Oct.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    CGFXManager.
-----------------------------------------------------------------------------*/




//
// An information post effect entry.
//
struct TPostEffect
{
public:
	// Color post processing.
	Float		Highlights[3];
	Float		MidTones[3];
	Float		Shadows[3];
	Float		BWScale;

	// TPostEffect interface.
	TPostEffect();
	TPostEffect( const Float* EffArr );
};


//
// Post-processing manager.
//
class CGFXManager
{
public:
	// CGFXManager interface.
	CGFXManager( FLevel* InLevel );
	~CGFXManager();

	void PushEffect( const TPostEffect& InEffect, Float FadeTime );
	void PopEffect( Float FadeTime );
	TPostEffect& GetResult();
	void Tick( Float Delta );

private:
	// GFX internal.
	TPostEffect		BaseEffect;
	TPostEffect		ResultEffect;
	Float			Fade;
	Float			Alpha;
	TPostEffect		NewEffect;
	TPostEffect		OldEffect;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/