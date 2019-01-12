/*=============================================================================
    FrRes.cpp: Base resource classes.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FResource implementation.
-----------------------------------------------------------------------------*/

//
// Resource constructor.
//
FResource::FResource()
	:	FObject(),
		FileName(),
		Group()
{
}


//
// Resource serialization.
//
void FResource::SerializeThis( CSerializer& S )
{
	FObject::SerializeThis(S);

	Serialize( S, FileName );
	Serialize( S, Group );
}


//
// Resource after loading initialization.
//
void FResource::PostLoad()
{
	FObject::PostLoad();
}


//
// Resource import.
//
void FResource::Import( CImporterBase& Im )
{
	FObject::Import(Im);

	IMPORT_OBJECT( Owner );
	IMPORT_STRING( FileName );
}


//
// Resource export.
//
void FResource::Export( CExporterBase& Ex )
{
	FObject::Export(Ex);

	EXPORT_OBJECT( Owner );
	EXPORT_STRING( FileName );
}


/*-----------------------------------------------------------------------------
    CResourceBlock implementation.
-----------------------------------------------------------------------------*/

//
// Constructor.
//
CResourceBlock::CResourceBlock()
	:	iBlock(-1)
{
}


//
// Destructor.
//
CResourceBlock::~CResourceBlock()
{
	// Release data block if was allocated.
	if( IsValidBlock() && iBlock != -1 )
		ReleaseBlock();
}


//
// Initialize resource data block, return true if
// block created successfully, otherwise return false.
//
Bool CResourceBlock::AllocateBlock( SizeT NumBytes, UInt32 ExtraFlags )
{
	// Maybe block already allocated.
	if( IsValidBlock() )
		return false;

	// Create new one.
	iBlock = GProject->BlockMan->AllocateBlock( NumBytes, ExtraFlags );
	return true;
}


//
// Release data block data. Return true if
// released successfully, otherwise return false.
//
Bool CResourceBlock::ReleaseBlock()
{
	if( !IsValidBlock() )
		return false;

	if( GIsEditor && iBlock != -1 )
	{
		GProject->BlockMan->ReleaseBlock( iBlock );
		iBlock	= -1;
		return true;
	}
	else
		return false;
}


//
// Return the size of block of data.
// If block not allocated, return 0.
//
SizeT CResourceBlock::GetBlockSize()
{
	return IsValidBlock() ? GProject->BlockMan->GetBlockSize(iBlock) : 0;
}


//
// Return block data.
//
void* CResourceBlock::GetData()
{
	assert(IsValidBlock());
	return GProject->BlockMan->GetBlock( iBlock );
}


//
// Return true, if block is valid.
//
Bool CResourceBlock::IsValidBlock() const
{
	return iBlock != -1;
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FResource, FObject, CLASS_Abstract )
{
	ADD_PROPERTY( Group, PROP_Editable );
}

REGISTER_CLASS_CPP( FAnimation, FResource, CLASS_Sterile )
{
	ADD_PROPERTY( Sheet, PROP_Editable );
	ADD_PROPERTY( FrameW, PROP_Editable );
	ADD_PROPERTY( FrameH, PROP_Editable );
	ADD_PROPERTY( SpaceX, PROP_Editable );
	ADD_PROPERTY( SpaceY, PROP_Editable );
}

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/