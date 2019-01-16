/*=============================================================================
    FrUndoRedo.h: Level Undo/Redo transactor.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    TTransaction.
-----------------------------------------------------------------------------*/

// Whether compress transactions?
#define TRAN_COMPRESS		1


//
// A single transaction.
//
class TTransaction
{
public:
	// General info.
	Array<UInt8>		Data;
	FLevel*				Level;
	Array<FScript*>		Entities;
	Array<String>		Names;
	Array<FObject*>		Detached;

	// Compressed data.
	SizeT				ComSourceSize;
	SizeT				ComSize;
	void*				ComData;

	// TTransaction interface.
	TTransaction( FLevel* InLevel );
	~TTransaction();
	void Store();
	void Restore();
	SizeT CountMem() const;
};


/*-----------------------------------------------------------------------------
    CLevelTransactor.
-----------------------------------------------------------------------------*/

//
// A level Undo/Redo subsystem.
//
class CLevelTransactor: public CRefsHolder
{
public:
	// Constants.
	enum{ HISTORY_LIMIT	= 20 };

	// CLevelTransactor interface.
	CLevelTransactor( FLevel* InLevel );
	~CLevelTransactor();
	void Debug();
	Bool CanUndo() const;
	Bool CanRedo() const;
	void Undo();
	void Redo();
	void TrackEnter();
	void TrackLeave();
	void Reset();

	// CRefsHolder interface.
	void CountRefs( CSerializer& S );

private:
	// Variables.
	Bool					bLocked;
	Array<TTransaction*>	Transactions;
	Int32					TopTransaction;
	FLevel*					Level;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/