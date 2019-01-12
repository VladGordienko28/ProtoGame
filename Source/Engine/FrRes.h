/*=============================================================================
    FrRes.h: FResource base class.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    FResource.
-----------------------------------------------------------------------------*/

//
// An abstract resource.
//
class FResource: public FObject
{
REGISTER_CLASS_H(FResource);
public:
	// Variables.
	String		FileName;
	String		Group;

	// FResource interface.
	FResource();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );
};


/*-----------------------------------------------------------------------------
	IResourceBlock.
-----------------------------------------------------------------------------*/

//
// An interface that allow derived class to hold large blocks of dynamically
// loaded memory.
//
class IResourceBlock
{
public:
	// IResourceBlock interface.
	virtual Bool AllocateBlock( SizeT NumBytes, UInt32 ExtraFlags = BLOCK_None ) = 0;
	virtual Bool ReleaseBlock() = 0;
	virtual SizeT GetBlockSize() = 0;
	virtual void* GetData() = 0;
	virtual Bool IsValidBlock() const = 0;
};


/*-----------------------------------------------------------------------------
    CResourceBlock.
-----------------------------------------------------------------------------*/

//
// An implemented CBlockManager's resource block of data.
//
class CResourceBlock: public IResourceBlock
{
public:
	// CResourceBlock interface.
	CResourceBlock();
	virtual ~CResourceBlock();

	// IResourceBlock interface.
	Bool AllocateBlock( SizeT NumBytes, UInt32 ExtraFlags = BLOCK_None );
	Bool ReleaseBlock();
	SizeT GetBlockSize();
	void* GetData();
	Bool IsValidBlock() const;

	// Friends.
	friend class CBlockManager;

protected:
	// Block internal.
	Int32 iBlock;
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/