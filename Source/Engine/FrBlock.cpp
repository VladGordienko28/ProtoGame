/*=============================================================================
    FrBlock.cpp: A large data manager.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    CBlockManager implementation.
-----------------------------------------------------------------------------*/

//
// An initial block cost, by this value set how long block
// will stay in game, without using in seconds.
//
#define BLOCK_BASE_COST		60.0


//
// Data block in-editor constructor.
//
CBlockManager::CBlockManager()
	:	Blocks(),
		FileName(),
		ResFile( nullptr )
{
	//assert(GIsEditor);
}


//
// Data block in-game constructor.
//
CBlockManager::CBlockManager( String InFileName )
	:	Blocks(),
		FileName( InFileName )
{
	// Allocate loader.
	if( !fm::fileExists( *InFileName ) )
		fatal( L"Resource file '%s' not found", *FileName );

	ResFile = fm::readBinaryFile( *InFileName );
}


//
// Manager destructor.
//
CBlockManager::~CBlockManager()
{
	// Close file if any.
	if( false /*!GIsEditor*/ )
	{
		ResFile = nullptr;
	}

	// Destroy blocks.
	for( Int32 i=0; i<Blocks.size(); i++ )
		if( Blocks[i] )
			freeandnil(Blocks[i]);
}


//
// Return pointer to the block's data.
// If iBlock is not loaded, block will be loaded.
//
void* CBlockManager::GetBlock( Int32 iBlock )
{
	assert(iBlock>=0 && iBlock<Blocks.size());
	assert(Blocks[iBlock]);
	TDataBlock*	Block = Blocks[iBlock];

	// Load block if, it not loaded.
	if( !(Block->Flags & BLOCK_Loaded) )
		UploadBlock( iBlock );

	// Let popular block live longer.
	Block->Cost	= (Block->Flags & BLOCK_Longevity) ? BLOCK_BASE_COST*3.0 : BLOCK_BASE_COST;

	// Return actual data.
	return Block->Data;
}


//
// Return the size of block, even if it not actually
// loaded. This routine doesn't load block no way.
//
UInt32 CBlockManager::GetBlockSize( Int32 iBlock )
{
	assert(iBlock>=0 && iBlock<Blocks.size());
	assert(Blocks[iBlock]);
	
	return Blocks[iBlock]->Size;
}


/*-----------------------------------------------------------------------------
    Editor routines.
-----------------------------------------------------------------------------*/

//
// Allocate new data block, used only for editor.
//
Int32 CBlockManager::AllocateBlock( Int32 BytesCount, UInt32 ExtraFlags )
{
	//assert(GIsEditor);
	assert(BytesCount>0);

	// Allocate and initialize block.
	TDataBlock*	B	= new TDataBlock( BytesCount );
	B->Flags		= ExtraFlags | BLOCK_Loaded;
	B->Cost			= 0.0;
	B->FileRecord	= 0;

	Int32 iSlot = -1;

	// Try find available slot.
	for( Int32 i=0; i<Blocks.size(); i++ )
		if( !Blocks[i] )
		{
			iSlot	= i;
			break;
		}

	// Add to the list.
	if( iSlot != -1 )
	{
		Blocks[iSlot]	= B;
		return iSlot;
	}
	else
	{
		return Blocks.push(B);
	}
}


//
// Release the block. Editor only.
//
void CBlockManager::ReleaseBlock( Int32 iBlock )
{
	assert(iBlock>=0 && iBlock<Blocks.size());

	TDataBlock*	B	= Blocks[iBlock];
	assert(B && B->Size>0 && B->Data);

	// Release it.
	delete B;
	Blocks[iBlock]	= nullptr;
}


//
// Save all resources to the file.
//
void CBlockManager::SaveAllBlocks( String InFileName )
{
	// Don't save resources in game.
	//assert(GIsEditor);

	fm::IBinaryFileWriter::Ptr saver = fm::writeBinaryFile( *InFileName );
	assert( saver.hasObject() );


	// Count resources info.
	{
		Int32 NumRes = 0, DbSize = 0;
		for( Int32 i=0; i<Blocks.size(); i++ )
			if( Blocks[i] )
			{
				NumRes++;
				DbSize	= i+1;
			}

		saver->writeData( &DbSize, sizeof( DbSize ) );
		saver->writeData( &NumRes, sizeof( NumRes ) );
	}

	// Save header of each resource, and store
	// position of data offset in file.
	Array<Int32> OffsetMap(Blocks.size());

	for( Int32 i=0; i<Blocks.size(); i++ )
		if( Blocks[i] )
		{
			// Valid block.
			TDataBlock*	B	= Blocks[i];

			// Select best compression for resource.
			{
				B->Flags	&= ~(BLOCK_LZW | BLOCK_RLE);

				SizeT RealSize, RLESize, LZWSize;
				CRLECompressor RLE;
				CLZWCompressor LZW;
				
				RealSize	= B->Size;
				RLESize		= RLE.ForecastSize( B->Data, B->Size );
				LZWSize		= LZW.ForecastSize( B->Data, B->Size );
				
				// Make decision.
				if( RealSize-min(RLESize, LZWSize) <= 8*1024 )
				{
					// Uncompressed data is smaller than compressed, yes
					// this can happen, but really seldom.
				}
				else
				{
					// Select better compression, but we prefer RLE, since it much faster.
					// 16 Kb is not too much.
					B->Flags	|= ( RLESize-LZWSize <= 16*1024 ) ? BLOCK_RLE : BLOCK_LZW;
				}

#if 1
				// Dbg.
				info
				( 
					L"ResMan: %i's resource used %s (%i -> %i)kB", 
					i,  
					(B->Flags&BLOCK_LZW)?L"LZW":(B->Flags&BLOCK_RLE)?L"RLE":L"None",
					RealSize/1024,
					((B->Flags&BLOCK_LZW)?LZWSize:(B->Flags&BLOCK_RLE)?RLESize:RealSize)/1024		
				);
#endif
			}

			// Save block info.
			saver->writeData( &i, sizeof( i ) );
			saver->writeData( &B->Flags, sizeof( B->Flags ) );
			saver->writeData( &B->Size, sizeof( B->Size ) );

			// Temporary serialize stuff, next time it will be 
			// offset to block's data.
			B->FileRecord	= -1;
			OffsetMap[i]	= saver->tell();
			saver->writeData( &B->FileRecord, sizeof( B->FileRecord ) );
		}

	// Save all block's data.
	for( Int32 i=0; i<Blocks.size(); i++ )
		if( Blocks[i] )
		{
			// Valid block.
			TDataBlock*	B	= Blocks[i];

			// Update FileRecord.
			B->FileRecord = saver->tell();
			saver->seek( OffsetMap[i] );
			saver->writeData( &B->FileRecord, sizeof( B->FileRecord ) );
			saver->seek( B->FileRecord );

			if( B->Flags & BLOCK_LZW )
			{
				// Apply LZW compression.
				CLZWCompressor LZW;
				void* OutBuffer;
				SizeT OutSize;
				LZW.Encode( B->Data, B->Size, OutBuffer, OutSize );	
				saver->writeData( &OutSize, sizeof( OutSize ) );
				saver->writeData( OutBuffer, OutSize );
				mem::free( OutBuffer );
			}
			else if( B->Flags & BLOCK_RLE )
			{
				// Apply RLE compression.
				CRLECompressor RLE;
				void* OutBuffer;
				SizeT OutSize;
				RLE.Encode( B->Data, B->Size, OutBuffer, OutSize );
				saver->writeData( &OutSize, sizeof( OutSize ) );
				saver->writeData( OutBuffer, OutSize );
				mem::free( OutBuffer );
			}
			else
			{
				// Save without compression.
				saver->writeData( B->Data, B->Size );
			}
		}
}


//
// Load all resources from the file.
//
void CBlockManager::LoadAllBlocks( String InFileName )
{
	// Don't load all resources in game.
	//assert(GIsEditor);

	fm::IBinaryFileReader::Ptr loader = fm::readBinaryFile( *InFileName );

	// Resources count.
	Int32 NumRes, DbSize;
	loader->readData( &DbSize, sizeof( DbSize ) );
	loader->readData( &NumRes, sizeof( NumRes ) );

	// Allocate slots for blocks.
	Blocks.setSize( DbSize );

	// Load main information about each block.
	for( Int32 i=0; i<NumRes; i++ )
	{
		TDataBlock* B	= new TDataBlock();
		Int32 iSlot;
		
		// Load.
		loader->readData( &iSlot, sizeof( iSlot ) );
		loader->readData( &B->Flags, sizeof( B->Flags ) );
		loader->readData( &B->Size, sizeof( B->Size ) );
		loader->readData( &B->FileRecord, sizeof( B->FileRecord ) );

		// Add to list.
		Blocks[iSlot]	= B;
	}

	// Load all blocks data.
	for( Int32 i=0; i<Blocks.size(); i++ )
		if( Blocks[i] )
		{
			TDataBlock* B	= Blocks[i];

			// Goto data.
			loader->seek( B->FileRecord );
			
			// Load and uncompress data.
			if( B->Flags & BLOCK_LZW )
			{
				// Load LZW data and uncompress it.
				UInt32 InSize;
				loader->readData( &InSize, sizeof( InSize ) );
				void* InBuffer	= mem::malloc( InSize );
				loader->readData( InBuffer, InSize );
				CLZWCompressor LZW;
				LZW.Decode( InBuffer, InSize, B->Data, B->Size );
				mem::free( InBuffer );
				B->Data	= InBuffer;
			}
			else if( B->Flags & BLOCK_RLE )
			{
				// Load RLE data and uncompress it.
				UInt32 InSize;
				loader->readData( &InSize, sizeof( InSize ) );
				void* InBuffer	= mem::malloc( InSize );
				loader->readData( InBuffer, InSize );
				CRLECompressor RLE;
				RLE.Decode( InBuffer, InSize, B->Data, B->Size );
				mem::free( InBuffer );
			}
			else
			{
				// Load not compressed data.
				B->Data	= mem::malloc( B->Size );
				loader->readData( B->Data, B->Size );
			}
		}

	// Mark each resource as loaded.
	for( Int32 i=0; i<Blocks.size(); i++ )
		if( Blocks[i] )
			Blocks[i]->Flags	|= BLOCK_Loaded;
}


/*-----------------------------------------------------------------------------
    Game routines.
-----------------------------------------------------------------------------*/

//
// Unload all blocks, except persistent of course.
// For game only.
//
void CBlockManager::Flush()
{
	//assert(!GIsEditor);

	for( Int32 i=0; i<Blocks.size(); i++ )
		if( Blocks[i] )
		{
			TDataBlock* B	= Blocks[i];

			if( (B->Flags & BLOCK_Loaded) && !(B->Flags & BLOCK_Persistent) )
			{
				mem::free( B->Data );
				B->Data		= nullptr;
				B->Flags	&= ~BLOCK_Loaded;
			}
		}
}


//
// Preload all information about blocks. But
// Don't load actual large data blocks.
//
void CBlockManager::LoadMetadata()
{
	//assert(!GIsEditor);

	// Resources count.
	Int32 NumRes, DbSize;
	ResFile->readData( &DbSize, sizeof( DbSize ) );
	ResFile->readData( &NumRes, sizeof( NumRes ) );

	// Allocate slots for blocks.
	Blocks.setSize( DbSize );

	// Load main information about each block.
	for( Int32 i=0; i<NumRes; i++ )
	{
		TDataBlock* B	= new TDataBlock();
		Int32 iSlot;
		
		// Load.
		ResFile->readData( &iSlot, sizeof( iSlot ) );
		ResFile->readData( &B->Flags, sizeof( B->Flags ) );
		ResFile->readData( &B->Size, sizeof( B->Size ) );
		ResFile->readData( &B->FileRecord, sizeof( B->FileRecord ) );

		// Add to list.
		Blocks[iSlot]	= B;
	}

	// Mark each resource as not loaded.
	for( Int32 i=0; i<Blocks.size(); i++ )
		if( Blocks[i] )
			Blocks[i]->Flags	&= ~BLOCK_Loaded;

	// Load all persistent blocks.
	for( Int32 i=0; i<Blocks.size(); i++ )
		if( Blocks[i] && (Blocks[i]->Flags & BLOCK_Persistent) )
			UploadBlock( i );
}


//
// Upload single resource. Used to dynamic load
// blocks during playing.
//
void CBlockManager::UploadBlock( Int32 iBlock )
{  
	//assert(!GIsEditor);
	assert(iBlock>=0 && iBlock<Blocks.size());
	assert(Blocks[iBlock]);
	assert(!(Blocks[iBlock]->Flags & BLOCK_Loaded));

	// Load it.
	TDataBlock* B		= Blocks[iBlock];
	ResFile->seek( B->FileRecord );

	// Load and uncompress data.
	if( B->Flags & BLOCK_LZW )
	{
		// Load LZW data and uncompress it.
		UInt32 InSize;
		ResFile->readData( &InSize, sizeof( InSize ) );
		void* InBuffer	= mem::malloc( InSize );
		ResFile->readData( InBuffer, InSize );
		CLZWCompressor LZW;
		LZW.Decode( InBuffer, InSize, B->Data, B->Size );
		mem::free( InBuffer );
	}
	else if( B->Flags & BLOCK_RLE )
	{
		// Load RLE data and uncompress it.
		UInt32 InSize;
		ResFile->readData( &InSize, sizeof( InSize ) );
		void* InBuffer	= mem::malloc( InSize );
		ResFile->readData( InBuffer, InSize );
		CRLECompressor RLE;
		RLE.Decode( InBuffer, InSize, B->Data, B->Size );
		mem::free( InBuffer );
	}
	else
	{
		// Load not compressed data.
		B->Data	= mem::malloc(B->Size);
		ResFile->readData( B->Data, B->Size );
	}

	// Mark block as loaded. And let it live 
	// about one minute.
	B->Flags		|= BLOCK_Loaded;
	B->Cost			= (B->Flags & BLOCK_Longevity) ? BLOCK_BASE_COST*3.0 : BLOCK_BASE_COST;

	// Dbg.
	info( L"ResMan: Block %i has been upload", iBlock );
}


//
// Update the block manager.
//
void CBlockManager::Tick( Float Delta )
{
	//assert(!GIsEditor);

	for( Int32 i=0; i<Blocks.size(); i++ )
		if( Blocks[i] )
		{
			TDataBlock* B	= Blocks[i];

			if( (B->Flags & BLOCK_Loaded) && !(B->Flags & BLOCK_Persistent) )
			{
				B->Cost -= Delta;

				if( B->Cost <= 0.0 )
				{
					// Block expired.
					mem::free( B->Data );
					B->Data		= nullptr;
					B->Flags	&= ~BLOCK_Loaded;

					// Dbg.
					info( L"ResMan: Block %i expired. Size=%i Kb", i, B->Size/1024 );
				}
			}
		}
}


/*-----------------------------------------------------------------------------
    Utility.
-----------------------------------------------------------------------------*/

//
// Output debug information about the manager.
//
void CBlockManager::DebugManager()
{
	Int32	NumBytes		= 0, 
			NumAvailable	= 0, 
			NumLoaded		= 0;

	// Count statistic.
	for( Int32 i=0; i<Blocks.size(); i++ )
		if( Blocks[i] )
		{
			TDataBlock* B = Blocks[i];

			if( B->Flags & BLOCK_Loaded )
			{
				NumBytes += B->Size;
				NumLoaded++;
			}
		}
		else
			NumAvailable++;

	// Put to the console.
	info( L"**DataBlock manage info:" );
	info( L"   Total used %i slots",		Blocks.size() );
	info( L"   Used %i kB",					NumBytes / 1024 );
	info( L"   %i slots available",			NumAvailable );
	info( L"   %i blocks are loaded",		NumLoaded );
}


/*-----------------------------------------------------------------------------
    TDataBlock implementation.
-----------------------------------------------------------------------------*/

//
// Block constructor when it loading from
// the file.
//
TDataBlock::TDataBlock()
{
	Data		= nullptr;
	Flags		= BLOCK_None;
	Size		= 0;
	Cost		= 0.0;
	FileRecord	= 0;
}


//
// In editor block constructor.
//
TDataBlock::TDataBlock( Int32 InSize )
{
	//assert(GIsEditor);

	Data		= mem::alloc(alignValue(InSize, 16));
	Flags		= BLOCK_Loaded;
	Size		= InSize;
	Cost		= 0.0;
	FileRecord	= 0;
}


//
// Data block destructor.
//
TDataBlock::~TDataBlock()
{
	if( Data )
	{
		mem::free( Data );
		Data	= nullptr;
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/