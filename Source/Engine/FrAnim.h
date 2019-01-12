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
	Integer			Start;
	Integer			Count;
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
	TArray<TAnimSequence>	Sequences;

	// Slicing.
	Integer					FrameW;
	Integer					FrameH;
	Integer					SpaceX;
	Integer					SpaceY;
	TArray<TRect>			Frames;

	// FAnimation interface.
	FAnimation();
	Integer FindSequence( String InName );
	TRect GetTexCoords( Integer iFrame );

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