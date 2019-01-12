/*=============================================================================
	FrModi.h: Modifier base class.
	Created by Vlad Gordienko, Jan. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	FModifier.
-----------------------------------------------------------------------------*/

//
// An abstract resource modifier.
//
class FModifier: public FObject
{
REGISTER_CLASS_H(FModifier);
public:
	// FModifier interface.
	FModifier();
	~FModifier();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/