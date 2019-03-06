/*=============================================================================
    FrBlock.h: A data block manager.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    TDataBlock.
-----------------------------------------------------------------------------*/

//
// Flags for describing data block.
//
#define BLOCK_None				0x0000		// No special flag.
#define BLOCK_Loaded			0x0001		// Block loaded and used.
#define BLOCK_Longevity			0x0002		// Extra long life for block.
#define BLOCK_Persistent		0x0004		// Don't load/unload block during playing.
#define BLOCK_RLE				0x0008		// Simple and fast RLE compression used.
#define BLOCK_LZW				0x0010		// Improved compression, but not so fast.
#define BLOCK_Reserved			0x0020


//
// An large data block.
//
class TDataBlock
{
public:
	// TDataBlock interface.
	TDataBlock();
	TDataBlock( Int32 InSize );
	~TDataBlock();	

	// Friends.
	friend CBlockManager;

	// Accessors.
	inline void* GetData() const
	{
		return Data;
	}
	inline UInt64 GetSize() const
	{
		return Size;
	}

private:
	// Variables.
	void*			Data;
	UInt32			Flags;
	SizeT			Size;
	Double			Cost;
	UInt64			FileRecord;
};


/*-----------------------------------------------------------------------------
    CBlockManager.
-----------------------------------------------------------------------------*/

//
// A data block manager.
//
class CBlockManager
{
public:
	// CBlockManager interface.
	CBlockManager();
	CBlockManager( String InFileName );
	~CBlockManager();
	void* GetBlock( Int32 iBlock );
	UInt32 GetBlockSize( Int32 iBlock );

	// In-game routines.
	void LoadMetadata();
	void Flush();
	void UploadBlock( Int32 iBlock );
	void Tick( Float Delta );

	// In-editor routines.
	Int32 AllocateBlock( Int32 BytesCount, UInt32 ExtraFlags = BLOCK_None );
	void ReleaseBlock( Int32 iBlock );
	void LoadAllBlocks( String InFileName );
	void SaveAllBlocks( String InFileName );

	// Debug.
	void DebugManager();

private:
	// Internal.
	Array<TDataBlock*>			Blocks;
	String						FileName;
	fm::IBinaryFileReader::Ptr	ResFile;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/