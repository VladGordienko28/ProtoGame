/*=============================================================================
    FrAudio.cpp: Audio implementation.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FMusic implementation.
-----------------------------------------------------------------------------*/

//
// Music constructor.
//
FMusic::FMusic()
	:	FResource()
{
}


//
// Music destructor.
//
FMusic::~FMusic()
{
}


//
// Initialize music after loading.
//
void FMusic::PostLoad()
{
	FResource::PostLoad();

	// Track file is exists?
	if( !fm::fileExists( *(fm::getCurrentDirectory()+L"\\"+FileName) ) )
		debug( L"Music: Track '%s' not found", *FileName );
}


/*-----------------------------------------------------------------------------
    FSound implementation.
-----------------------------------------------------------------------------*/

//
// Sound constructor.
//
FSound::FSound()
	:	FResource(),
		CResourceBlock(),
		AudioInfo( -1 )
{
}


//
// Sound destructor.
//
FSound::~FSound()
{
}


//
// Sound after loading.
//
void FSound::PostLoad()
{
	FResource::PostLoad();

	// Mark sound as not registered.
	AudioInfo	= -1;
}


//
// Sound serializaion.
//
void FSound::SerializeThis( CSerializer& S )
{
	FResource::SerializeThis( S );
	Serialize( S, iBlock );
}

/*-----------------------------------------------------------------------------
	Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FMusic, FResource, CLASS_Sterile )
{
	ADD_PROPERTY( FileName, PROP_Editable | PROP_Const | PROP_NoImEx );
}

REGISTER_CLASS_CPP( FSound, FResource, CLASS_Sterile )
{
	ADD_PROPERTY( FileName, PROP_Editable | PROP_Const | PROP_NoImEx );
}

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/