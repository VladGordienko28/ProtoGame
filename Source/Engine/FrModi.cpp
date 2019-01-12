/*=============================================================================
	FrModi.cpp: Modifier class.
	Created by Vlad Gordienko, Jan. 2018.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
	FModifier implementation.
-----------------------------------------------------------------------------*/

//
// Modifier constructor.
//
FModifier::FModifier()
	:	FObject()
{
}


//
// Modifier destructor.
//
FModifier::~FModifier()
{
}


//
// Modifier destructor.
//
void FModifier::SerializeThis( CSerializer& S )
{
	FObject::SerializeThis( S );
}


//
// When some modifier field changed.
//
void FModifier::EditChange()
{
	FObject::EditChange();

	// Changing of modifier is changing of owner.
	Owner->EditChange();
}


//
// When modifier just loaded.
//
void FModifier::PostLoad()
{
	FObject::PostLoad();
	assert(Owner);
}


/*-----------------------------------------------------------------------------
	Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FModifier, FObject, CLASS_Abstract )
{
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/