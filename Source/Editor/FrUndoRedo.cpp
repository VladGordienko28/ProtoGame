/*=============================================================================
    FrUndoRedo.cpp: Editor Undo/Redo implementation.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    CLevelTransactor implementation.
-----------------------------------------------------------------------------*/

//
// Undo/Redo subsystem initialization.
//
CLevelTransactor::CLevelTransactor( FLevel* InLevel )
{
	assert(InLevel);
	assert(!InLevel->IsTemporal());
	bLocked			= false;
	TopTransaction	= 0;;
	Level			= InLevel;

	// Notify.
	trace( L"Editor: Undo/Redo subsystem initialized for '%s'", *Level->GetFullName() );
}


//
// Undo/Redo subsystem destructor.
//
CLevelTransactor::~CLevelTransactor()
{
	// Destroy transactions.
	for( Int32 i=0; i<Transactions.Num(); i++ )
		delete Transactions[i];

	Transactions.Empty();

	// Notify.
	trace( L"Editor: Undo/Redo subsystem shutdown for '%s'", *Level->GetFullName() );
}


//
// Reset transactor.
//
void CLevelTransactor::Reset()
{
	assert(!bLocked);

	// Destroy transactions.
	for( Int32 i=0; i<Transactions.Num(); i++ )
		delete Transactions[i];
	Transactions.Empty();

	// Pop transaction stack.
	TopTransaction	= 0;

	// Notify.
	info( L"Editor: Undo transactor '%s' has been reset", *Level->GetName() );
}


//
// Perform undo rollback operation.
//
void CLevelTransactor::Undo()
{
	if( CanUndo() )
	{
		TopTransaction--;
		Transactions[TopTransaction]->Restore();
	}
}


//
// Perform redo rollback operation.
//
void CLevelTransactor::Redo()
{
	if( CanRedo() )
	{
		TopTransaction++;
		Transactions[TopTransaction]->Restore();
	}
}


//
// Return whether allow to redo.
//
Bool CLevelTransactor::CanUndo() const
{
	return TopTransaction > 0;
}


//
// Return whether allow to redo.
//
Bool CLevelTransactor::CanRedo() const
{
	return TopTransaction < (Transactions.Num()-1);
}


//
// Enter to undo/redo tracking section.
//
void CLevelTransactor::TrackEnter()
{
	assert(!bLocked);
	bLocked	= true;

	// Store only first initial state.
	if( TopTransaction == 0 )
	{
		Transactions.SetNum( 1 );
		Transactions[0]			= new TTransaction(Level);
		Transactions[0]->Store();
		TopTransaction	= 0;
	}
}


//
// Leave undo/redo tracking section.
//
void CLevelTransactor::TrackLeave()
{
	assert(bLocked);
	bLocked	= false;

	// Destroy transactions after top.
	for( Int32 i=TopTransaction+1; i<Transactions.Num(); i++ )	
		freeandnil(Transactions[i]);

	// Store current state.
	Transactions.SetNum(TopTransaction+2);
	if( Transactions.Num() > HISTORY_LIMIT )
	{
		// Destroy first record and shift others.
		Transactions.SetNum(HISTORY_LIMIT);
		delete Transactions[0];

		mem::copy
		(
			&Transactions[0],
			&Transactions[1],
			(Transactions.Num()-1)*sizeof(TTransaction*)
		);
	}
	TopTransaction	= Transactions.Num()-1;

	// Create new transaction.
	Transactions[TopTransaction]	= new TTransaction(Level);
	Transactions[TopTransaction]->Store();
}


//
// Serialize a transactor. Not for save to file
// just only for GC, and clean up dangling pointers.
//
void CLevelTransactor::CountRefs( CSerializer& S )
{
	// Serialize everything.
	for( Int32 iTran=0; iTran<Transactions.Num(); iTran++ )
	{
		TTransaction* Tran = Transactions[iTran];
		Serialize( S, Tran->Detached );
		Serialize( S, Tran->Entities );

		// Test if some script has been destroyed.
		for( Int32 i=0; i<Tran->Entities.Num(); i++ )
			if( Tran->Entities[i] == nullptr )
				goto ResetAll;
	}

	// Everything fine.
	return;

ResetAll:
	// Reset tracker.
	for( Int32 iTran=0; iTran<Transactions.Num(); iTran++ )
		delete Transactions[iTran];
	Transactions.Empty();
	TopTransaction	= 0;
	notice( L"Undo/Redo subsystem reset" );
}


//
// Output information about tracker.
//
void CLevelTransactor::Debug()
{
	SizeT Mem = 0;
	for( Int32 i=0; i<Transactions.Num(); i++ )
		Mem += Transactions[i]->CountMem();

	// Output.
	debug( L"** UNDO/REDO for '%s' **", *Level->GetFullName() );
	debug( L"Used %d kBs", Mem/1024 );
	debug( L"TopTran=%d, Trans.Count=%d", TopTransaction, Transactions.Num() );
}


/*-----------------------------------------------------------------------------
    CTransactionWriter.
-----------------------------------------------------------------------------*/

//
// A writer to store level into transaction.
//
class CTransactionWriter: public CSerializer
{
public:
	// Variables.
	TTransaction*	Transaction;

	// CTransactionWriter interface.
	CTransactionWriter( TTransaction* InTransaction )
	{
		Mode		= SM_Save;
		Transaction	= InTransaction;
	}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count )
	{
		SizeT i	= Transaction->Data.Num();
		Transaction->Data.SetNum(i+Count);
		mem::copy( &Transaction->Data[i], Mem, Count );
	}
	void SerializeRef( FObject*& Obj )
	{
		if( Obj == nullptr )
		{
			// Null object.
			UInt8	b = 0;
			Serialize( *this, b );
		}
		else if( Obj->IsA(FEntity::MetaClass) )
		{
			// Pointer to entity.
			UInt8	b = 1;
			Int32	i = Transaction->Level->GetEntityIndex((FEntity*)Obj);
			Serialize( *this, b );
			Serialize( *this, i );
		}
		else if( Obj->IsA(FComponent::MetaClass) )
		{
			// Pointer to component.
			FComponent* Component = As<FComponent>(Obj);
			UInt8	b = 2;
			Int32	i = Transaction->Level->GetEntityIndex(Component->Entity);
			Int32 j = Component == Component->Entity->Base ? 0xff : Component->Entity->Components.FindItem((FExtraComponent*)Component);
			assert(j != -1);
			Serialize( *this, b );
			Serialize( *this, i );
			Serialize( *this, j );
		}
		else
		{
			// Resource.
			assert(Obj->IsA(FResource::MetaClass));
			UInt8	b = 3;
			Int32	i = Transaction->Detached.AddUnique(Obj);
			Serialize( *this, b );
			Serialize( *this, i );
		}
	}
	SizeT TotalSize()
	{
		return Transaction->Data.Num();
	}
	SizeT Tell()
	{
		return Transaction->Data.Num();
	}
	void Seek( SizeT NewPos )
	{
	}
};


/*-----------------------------------------------------------------------------
    CTransactionReader.
-----------------------------------------------------------------------------*/

//
// A reader to restore level from transaction.
//
class CTransactionReader: public CSerializer
{
public:
	// Variables.
	TTransaction*	Transaction;
	SizeT			StreamPos;

	// CTransactionReader interface.
	CTransactionReader( TTransaction* InTransaction )
	{
		Mode		= SM_Load;
		Transaction	= InTransaction;
		StreamPos	= 0;
	}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count )
	{
		mem::copy( Mem, &Transaction->Data[StreamPos], Count );
		StreamPos	+= Count;
	}
	void SerializeRef( FObject*& Obj )
	{
		UInt8 b;
		Serialize( *this, b );
		if( b == 0 )
		{
			// Null object.
			Obj	= nullptr;
		}
		else if( b == 1 )
		{
			// Pointer to entity.
			Int32 iEntity;
			Serialize( *this, iEntity );
			Obj	= Transaction->Level->Entities[iEntity];
		}
		else if( b == 2 )
		{
			// Pointer to component.
			Int32 iEntity, iCom;
			Serialize( *this, iEntity );
			Serialize( *this, iCom );
			FEntity* Entity = Transaction->Level->Entities[iEntity];
			Obj	= iCom == 0xff ? (FComponent*)Entity->Base : (FComponent*)Entity->Components[iCom];
		}
		else
		{
			// Pointer to resource.
			assert(b==3);
			Int32 iObj;
			Serialize( *this, iObj );
			Obj	= Transaction->Detached[iObj];
		}
	}
	SizeT TotalSize()
	{
		return Transaction->Data.Num();
	}
	SizeT Tell()
	{
		return StreamPos;
	}
	void Seek( SizeT NewPos )
	{
	}
};


/*-----------------------------------------------------------------------------
    TTransaction implementation.
-----------------------------------------------------------------------------*/

//
// Transaction constructor.
//
TTransaction::TTransaction( FLevel* InLevel )
{
	assert(InLevel);
	Level			= InLevel;
	ComSourceSize	= 0;
	ComSize			= 0;
	ComData			= nullptr;
}


//
// Transaction destructor.
//
TTransaction::~TTransaction()
{
	Data.Empty();
	Entities.Empty();
	Detached.Empty();
	Names.Empty();

	if( ComData )
		mem::free( ComData );
}


//
// Store a level into this transaction.
//
void TTransaction::Store()
{
	assert(Level);

	// Cleanup old data.
	Entities.Empty();
	Names.Empty();
	Detached.Empty();
	Data.Empty();

	// Store list of script being each entity.
	Entities.SetNum(Level->Entities.Num());
	Names.SetNum(Level->Entities.Num());
	for( Int32 i=0; i<Entities.Num(); i++ )
	{
		Entities[i]	= Level->Entities[i]->Script;
		Names[i]	= Level->Entities[i]->GetName();
	}

	// Store each entity and their ref.
	{
		CTransactionWriter Writer(this);

		// Serialize each entity.
		for( Int32 i=0; i<Level->Entities.Num(); i++ )
		{
			FEntity* Entity	= Level->Entities[i];

			// Serialize only component's because they are refer.
			Entity->Base->SerializeThis( Writer );
			for( Int32 e=0; e<Entity->Components.Num(); e++ )
				Entity->Components[e]->SerializeThis( Writer );

			// ..but also store the instance buffer.
			if( Entity->InstanceBuffer )
				Entity->InstanceBuffer->SerializeValues( Writer );
		}

		// Level variables.
		//Serialize( Writer, Level->RndFlags );
		Serialize( Writer, Level->Camera );
		Serialize( Writer, Level->Sky );
		Serialize( Writer, Level->GameSpeed );
		Writer.SerializeData( Level->Effect, sizeof(Level->Effect) );
	}

#if TRAN_COMPRESS
	// Compress the buffer.
	ComSourceSize	= Data.Num();
	{
		CLZWCompressor LZW;
		LZW.Encode
		(
			&Data[0],
			Data.Num(),
			ComData,
			ComSize
		);

#if 0
		// Realloc to real size.
		assert(ComSize<=Data.Num());		// I'm afraid of it.
		ComData = MemRealloc( ComData, ComSize );
#endif
	}
	//log( L"Transaction compress: %d->%d", Data.Num(), ComSize );
	if( ComSize > Data.Num() )
		notice( L"Undo/Redo record failure. But don't worry, it's not fatal." );
	Data.Empty();
#endif
}


//
// Restore an level from byte sequence.
//
void TTransaction::Restore()
{
	assert(Level);

	// Anyway destroy navigator.
	freeandnil(Level->Navigator);

	// Erase old level's objects.
	for( Int32 i=0; i<Level->Entities.Num(); i++ )
		DestroyObject( Level->Entities[i], true );

	Level->Entities.Empty();
	assert(Level->RenderObjects.Num()==0);
	assert(Level->TickObjects.Num()==0);
	assert(!Level->FirstPuppet);

	Level->Sky		= nullptr;

#if TRAN_COMPRESS
	// Uncompress the buffer.
	Data.SetNum( ComSourceSize );
	{
		void* DData;
		SizeT DSize;
		CLZWCompressor LZW;

		LZW.Decode
		(
			ComData,
			ComSize,
			DData,
			DSize
		);

		assert(DSize==ComSourceSize);
		mem::copy( &Data[0], DData, DSize );
		mem::free( DData );
	}
#endif

	// Restore all actors.
	{
		CTransactionReader Reader(this);

		// Allocate a list of entities and all it's 
		// components.
		for( Int32 iEntity=0; iEntity<Entities.Num(); iEntity++ )
		{
			FEntity* Entity = NewObject<FEntity>( Names[iEntity], Level );

			Entity->Level	= Level;
			Entity->Script	= Entities[iEntity];

			// Base.
			FBaseComponent* Base = NewObject<FBaseComponent>
													( 
														Entity->Script->Base->GetClass(), 
														Entity->Script->Base->GetName(), 
														Entity 
													);
			Base->InitForEntity( Entity );

			// Extra components.
			for( Int32 i=0; i<Entity->Script->Components.Num(); i++ )
			{
				FExtraComponent* Extra = Entity->Script->Components[i];
				FExtraComponent* Com = NewObject<FExtraComponent>
														( 
															Extra->GetClass(), 
															Extra->GetName(), 
															Entity 
														);
				Com->InitForEntity( Entity );
			}

			// Initialize instance buffer.
			if( Entity->Script->InstanceBuffer )
			{
				Entity->InstanceBuffer = new CInstanceBuffer( Entity->Script->Properties );
				Entity->InstanceBuffer->Data.SetNum( Entity->Script->InstanceSize );
			}

			// Add entity to the level's db.
			Level->Entities.Push( Entity );
		}

		// Serialize each entity.
		for( Int32 i=0; i<Level->Entities.Num(); i++ )
		{
			FEntity* Entity	= Level->Entities[i];

			// Serialize only component's because they are refer.
			Entity->Base->SerializeThis( Reader );
			for( Int32 e=0; e<Entity->Components.Num(); e++ )
				Entity->Components[e]->SerializeThis( Reader );

			// ..but also store the instance buffer.
			if( Entity->InstanceBuffer )
				Entity->InstanceBuffer->SerializeValues( Reader );
		}

		// Level variables.
		//Serialize( Writer, Level->RndFlags );
		Serialize( Reader, Level->Camera );
		Serialize( Reader, Level->Sky );
		Serialize( Reader, Level->GameSpeed );
		Reader.SerializeData( Level->Effect, sizeof(Level->Effect) );
	}

#if TRAN_COMPRESS
	// Release decompressed data.
	Data.Empty();
#endif

	// Notify each object.
	for( Int32 iEntity=0; iEntity<Level->Entities.Num(); iEntity++ )
	{
		FEntity* Entity	= Level->Entities[iEntity];

		Entity->PostLoad();
		Entity->Base->PostLoad();
		for( Int32 e=0; e<Entity->Components.Num(); e++ )
			Entity->Components[e]->PostLoad();
	}
}


//
// Count a memory used by this transaction.
//
SizeT TTransaction::CountMem() const
{
	return	Entities.Num() * sizeof(FScript*) +
			Detached.Num() * sizeof(FObject*) +
			Names.Num() * sizeof(String) +
			Data.Num() * sizeof(UInt8) +
			ComSourceSize;
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/