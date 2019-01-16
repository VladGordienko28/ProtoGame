/*=============================================================================
    FrAnim.h: A sprite sheet animation class.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Animation structures.
-----------------------------------------------------------------------------*/

//
// An animation sequence.
//
struct TAnimSequence
{
public:
	// Variables.
	String			Name;
	Int32			Start;
	Int32			Count;
};
   

/*-----------------------------------------------------------------------------
    FAnimation.
-----------------------------------------------------------------------------*/

//
// A sprite sheet animation.
//
class FAnimation: public FResource
{
REGISTER_CLASS_H(FAnimation);
public:
	// General.
	FTexture*				Sheet;
	Array<TAnimSequence>	Sequences;

	// Slicing.
	Int32					FrameW;
	Int32					FrameH;
	Int32					SpaceX;
	Int32					SpaceY;
	Array<TRect>			Frames;

	// FAnimation interface.
	FAnimation();
	Int32 FindSequence( String InName );
	TRect GetTexCoords( Int32 iFrame );

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

private:
	// Animation internal.
	void SetFramesTable();
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/