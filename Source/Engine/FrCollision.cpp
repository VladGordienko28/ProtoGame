/*=============================================================================
    FrCollision.cpp: Collision hash functions.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    Hash internal.
-----------------------------------------------------------------------------*/

//
// Hash function tables.
//
static Int32 HashXTab[COLL_HASH_SIZE];
static Int32 HashYTab[COLL_HASH_SIZE];


//
// Discretize world's coords to hash index.
//
void CCollisionHash::GetHashIndex( TVector V, Int32& iX, Int32& iY )
{
	// Check bounds.
	if	( 
			V.X >= +WORLD_HALF ||
			V.Y >= +WORLD_HALF ||
			V.X <= -WORLD_HALF ||
			V.Y <= -WORLD_HALF
		)
	{
		debug( L"Hash: Point [%.4f, %.4f] is out of world", V.X, V.Y );
		V.X	= Clamp<Float>( V.X, -WORLD_HALF, +WORLD_HALF );
		V.Y	= Clamp<Float>( V.Y, -WORLD_HALF, +WORLD_HALF );
	}

    // Discretize.
	iX	= (COLL_HASH_SIZE-1) & (Floor( V.X + WORLD_HALF ) >> COLL_FACTOR);
	iY	= (COLL_HASH_SIZE-1) & (Floor( V.Y + WORLD_HALF ) >> COLL_FACTOR);
}


/*-----------------------------------------------------------------------------
    CCollisionHash implementation.
-----------------------------------------------------------------------------*/

//
// Collision hash constructor.
//
CCollisionHash::CCollisionHash( FLevel* InLevel )
	:	Level( InLevel ),
		Pool( 16384 * sizeof(THashItem) ),
		FirstAvail( nullptr ),
		Mark( Random(777) ),
		HashActivItems( 0 ),
		HashObjects( 0 ),
		HashNumItems( 0 )
{
	// Initialize tables.
	mem::zero( Hash, sizeof(Hash) );

	// Initialize hash function tables.
	static Bool bTabInit = false;
	if( !bTabInit )
	{
		for( Int32 i=0; i<COLL_HASH_SIZE; i++ )
		{
			HashXTab[i]	= i;
			HashYTab[i]	= i;
		}
		
		// Shuffle indexes, but keep all values.
		for( Int32 i=0; i<COLL_HASH_SIZE; i++ )
		{
			Exchange( HashXTab[i], HashXTab[Random(COLL_HASH_SIZE)] );
			Exchange( HashYTab[i], HashYTab[Random(COLL_HASH_SIZE)] );
		}

		bTabInit = true;
	}
}


//
// Collision hash destructor.
//
CCollisionHash::~CCollisionHash()
{
	// Clean up pool only, because all items and
	// references are in there.
	Pool.PopAll();
}


//
// Add an object to the hash.
//
void CCollisionHash::AddToHash( FBaseComponent* Object )
{
	// Reject non hashable.
	if( !Object->bHashable )
		return;

	// Don't hash object twice.
	if( Object->bHashed )
	{
		debug( L"Hash: Object \"%s\" already in hash", *Object->GetFullName() );
		return;
	}
	Object->bHashed	= true;

	// Get bounds.
	Int32 X1, X2, Y1, Y2;
	Object->HashAABB = Object->GetAABB();
	GetHashIndex( Object->HashAABB.Min, X1, Y1 );
	GetHashIndex( Object->HashAABB.Max, X2, Y2 );

	for( Int32 Y=Y1; Y<=Y2; Y++ )
	for( Int32 X=X1; X<=X2; X++ )
	{
		THashItem* Item;
		Int32 iSlot = HashXTab[X] ^ HashYTab[Y];

		if( FirstAvail )
		{
			// Take item from the available list.
			Item		= FirstAvail;
			FirstAvail	= Item->Next;
		}
		else
		{
			// Allocate new item.
			Item	= Pool.PushT<THashItem>();
			HashNumItems++;
		}

		// Add item to the hash.
		HashActivItems++;
		Item->Object			= Object;
		Item->Next				= Hash[iSlot];
		Hash[iSlot]				= Item;
	}

	HashObjects++;
}


//
// Remove object from the hash.
//
void CCollisionHash::RemoveFromHash( FBaseComponent* Object )
{
	// Reject non hashable.
	if( !Object->bHashable )
		return;

	// Don't remove not added object.
	if( !Object->bHashed )
	{
		debug( L"Hash: Object \"%s\" is not in hash", *Object->GetFullName() );
		return;
	}
	Object->bHashed	= false;

	// Get bounds.
	Int32 X1, X2, Y1, Y2;
	TRect R = Object->GetAABB();
	if( R != Object->HashAABB )
	{
		debug( L"Hash: Object \"%s\" modified without hashing", *Object->GetFullName() );
		R = Object->HashAABB;
	}
	GetHashIndex( R.Min, X1, Y1 );
	GetHashIndex( R.Max, X2, Y2 );

	for( Int32 Y=Y1; Y<=Y2; Y++ )
	for( Int32 X=X1; X<=X2; X++ )
	{	
		Int32 iSlot = HashXTab[X] ^ HashYTab[Y];
		THashItem** ItemPtr = &Hash[iSlot];

		// Cleanup items.
		while( *ItemPtr )
		{
			if( (*ItemPtr)->Object != Object )
			{
				// Other object here, goto next in linked list.
				ItemPtr	= &((*ItemPtr)->Next);
			}
			else
			{
				// Found!
				THashItem* Trash	= *ItemPtr;
				*ItemPtr			= (*ItemPtr)->Next;
				Trash->Next			= FirstAvail;
				FirstAvail			= Trash;
				break;
			}
		}

		HashActivItems--;
	}

	HashObjects--;
}


//
// Return any object inside the bounds.
// Return first MAX_COLL_LIST_OBJS objects.
//
void CCollisionHash::GetOverlapped( TRect Bounds, Int32& OutNumObjs, FBaseComponent** OutList )
{
	// Get bounds.
	Int32 X1, X2, Y1, Y2;
	GetHashIndex( Bounds.Min, X1, Y1 );
	GetHashIndex( Bounds.Max, X2, Y2 );

	// Prepare.
	OutNumObjs	= 0;
	Mark++;

	for( Int32 Y=Y1; Y<=Y2; Y++ )
	for( Int32 X=X1; X<=X2; X++ )
	{
		Int32 iSlot = HashXTab[X] ^ HashYTab[Y];
		THashItem* Item = Hash[iSlot];

		while( Item )
		{
			FBaseComponent* Object = Item->Object;		

			if	(
					Object->HashMark != Mark &&
					!Object->bDestroyed &&
					Bounds.IsOverlap(Object->HashAABB)
				)
			{
				// Add to list.
				Object->HashMark	= Mark;
				OutList[OutNumObjs]	= Object;
				OutNumObjs++;

				if( OutNumObjs >= MAX_COLL_LIST_OBJS )
					goto Enough;
			}

			Item = Item->Next;
		}
	}

Enough:;
}


//
// Return any object inside the bounds of class 'Class' only.
// Return first MAX_COLL_LIST_OBJS objects.
//
void CCollisionHash::GetOverlappedByClass( TRect Bounds, CClass* Class, Int32& OutNumObjs, FBaseComponent** OutList )
{
	// Get bounds.
	Int32 X1, X2, Y1, Y2;
	GetHashIndex( Bounds.Min, X1, Y1 );
	GetHashIndex( Bounds.Max, X2, Y2 );

	// Prepare.
	OutNumObjs	= 0;
	Mark++;

	for( Int32 Y=Y1; Y<=Y2; Y++ )
	for( Int32 X=X1; X<=X2; X++ )
	{
		Int32 iSlot = HashXTab[X] ^ HashYTab[Y];
		THashItem* Item = Hash[iSlot];

		while( Item )
		{
			FBaseComponent* Object = Item->Object;

			if	(
					Object->HashMark != Mark &&
					!Object->bDestroyed &&
					Object->IsA(Class) &&
					Bounds.IsOverlap(Object->HashAABB)
				)
			{
				// Add to list.
				Object->HashMark	= Mark;
				OutList[OutNumObjs]	= Object;
				OutNumObjs++;

				if( OutNumObjs >= MAX_COLL_LIST_OBJS )
					goto Enough;
			}

			Item = Item->Next;
		}
	}

Enough:;
}


//
// Return any object inside the bounds of script 'Script' only.
// Return first MAX_COLL_LIST_OBJS objects.
//
void CCollisionHash::GetOverlappedByScript( TRect Bounds, FScript* Script, Int32& OutNumObjs, FBaseComponent** OutList )
{
	// Get bounds.
	Int32 X1, X2, Y1, Y2;
	GetHashIndex( Bounds.Min, X1, Y1 );
	GetHashIndex( Bounds.Max, X2, Y2 );

	// Prepare.
	OutNumObjs	= 0;
	Mark++;

	for( Int32 Y=Y1; Y<=Y2; Y++ )
	for( Int32 X=X1; X<=X2; X++ )
	{
		Int32 iSlot = HashXTab[X] ^ HashYTab[Y];
		THashItem* Item = Hash[iSlot];

		while( Item )
		{
			FBaseComponent* Object = Item->Object;

			if	(
					Object->HashMark != Mark &&
					!Object->bDestroyed &&
					Object->Entity->Script == Script &&
					Bounds.IsOverlap(Object->HashAABB)
				)
			{
				// Add to list.
				Object->HashMark	= Mark;
				OutList[OutNumObjs]	= Object;
				OutNumObjs++;

				if( OutNumObjs >= MAX_COLL_LIST_OBJS )
					goto Enough;
			}

			Item = Item->Next;
		}
	}

Enough:;
}


//
// Output debug information into console.
//
void CCollisionHash::DebugHash()
{
	info( L"** Collision hash \"%s\" info", *Level->GetFullName() );
	info( L"Hash: %d items in use", HashActivItems );
	info( L"Hash: %d items allocated", HashNumItems );
	info( L"Hash: %d objects in hash", HashObjects );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/