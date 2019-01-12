/*=============================================================================
	FrArray.h: Dynamic array template.
	Created by Vlad Gordienko, Jun. 2016.
	Redesigned for script compability Nov. 2017.
=============================================================================*/

/*-----------------------------------------------------------------------------
	TArray.
-----------------------------------------------------------------------------*/

//
// A POD version of real dynamic array template,
// which used by script language.
//
struct TArrayBase
{
public:
	// Variables.
	void*		Data;
	Integer		Count;

	// Functions.
	static void Reallocate( void*& Data, Integer& Count, Integer NewCount, SizeT InnerSize );
};


//
// A dynamic array template.
//
template<class T> class TArray: private TArrayBase
{
public:
	// Constructors.
	TArray();
	TArray( Integer InitSize );
	TArray( const TArray<T>& Other );

	// Destructors.
	~TArray();

	// Functions.
	Integer Num() const;
	Integer AddUnique( const T& InItem );
	void Empty();
	Integer FindItem( const T& InItem ) const;
	T& Last( Integer InvIndex=0 );
	T Pop();
	T Shift();
	Integer Push( const T& InItem );
	Integer Unshift( const T& InItem );
	void Remove( Integer Idx );
	void RemoveShift( Integer Idx );
	void RemoveUnique( const T& InItem );
	void Swap( Integer A, Integer B );
	void Insert( Integer FromIndex, Integer InCount=1 );
	void Sort( Bool(*SortFunc)( const T&, const T& ) );
	void SetNum( Integer NewNum );

	// Operators.
	T& operator[]( Integer i );
	const T& operator[]( Integer i ) const;
	Bool operator==( const TArray<T>& Other ) const;
	Bool operator!=( const TArray<T>& Other ) const;
	TArray<T>& operator=( const TArray<T>& Other );

	// Friends.
	friend void Serialize( CSerializer& S, TArray<T>& V )
	{
		if( S.GetMode() == SM_Load )
		{
			Integer ArrLen;
			Serialize( S, ArrLen );
			V.Empty();
			V.SetNum( ArrLen );
		}
		else
		{
			Integer ArrLen = V.Num();
			Serialize( S, ArrLen );
		}
		for( Integer i=0; i<V.Num(); i++ )
			Serialize( S, V[i] );
	}

private:
	// Array internal.
	void qSort( Integer Min, Integer Max, Bool(*SortFunc)( const T&, const T& ) );
};


/*-----------------------------------------------------------------------------
    TArray implementation.
-----------------------------------------------------------------------------*/

//
// Constructors.
//
template<class T> inline TArray<T>::TArray()
{
	Data	= nullptr;
	Count	= 0;
}
template<class T> inline TArray<T>::TArray( Integer InitSize )
{
	Data	= nullptr;
	Count	= 0;
	SetNum( InitSize );
}
template<class T> inline TArray<T>::TArray( const TArray<T>& Other )
{
	Data	= nullptr;
	Count	= 0;
	SetNum( Other.Num() );
	for( Integer i=0; i<Other.Num(); i++ )
		((T*)Data)[i]	= ((T*)Other.Data)[i];
}


//
// Destructor.
//
template<class T> inline TArray<T>::~TArray()
{
	Empty();
}


//
// Operators.
//
template<class T> inline T& TArray<T>::operator[]( Integer i )
{
	assert(i>=0 && i<Count);
	return ((T*)Data)[i];
}
template<class T> inline const T& TArray<T>::operator[]( Integer i ) const
{
	assert(i>=0 && i<Count);
	return ((T*)Data)[i];
}
template<class T> inline Bool TArray<T>::operator==( const TArray<T>& Other ) const
{
	if( Num() != Other.Num() )
		return false;
	for( Integer i=0; i<Num(); i++ )
		if( ((T*)Data)[i] != ((T*)Other.Data)[i] )
			return false;
	return true;
}
template<class T> inline Bool TArray<T>::operator!=( const TArray<T>& Other ) const
{
	if( Num() != Other.Num() )
		return true;
	for( Integer i=0; i<Num(); i++ )
		if( ((T*)Data)[i] != ((T*)Other.Data)[i] )
			return true;
	return false;
}
template<class T> inline TArray<T>& TArray<T>::operator=( const TArray<T>& Other )
{
	SetNum( Other.Num() );
	for( Integer i=0; i<Other.Num(); i++ )
		((T*)Data)[i] = ((T*)Other.Data)[i];
	return *this;
}

//
// Returns array length.
//
template<class T> inline Integer TArray<T>::Num() const
{
	return Count;
}


//
// Add an unique item to the array.
//
template<class T> inline Integer TArray<T>::AddUnique( const T& InItem )
{
	Integer i = FindItem(InItem);
	if( i == -1 )
		i = Push(InItem);
	return i;
}


//
// Cleanup array.
//
template<class T> inline void TArray<T>::Empty()
{
	SetNum(0);
}


//
// Find an item in the array, sad but O(n).
//
template<class T> inline Integer TArray<T>::FindItem( const T& InItem ) const
{
	for( Integer i=0; i<Num(); i++ )
		if( ((T*)Data)[i] == InItem )
			return i;
	return -1;
}


//
// Get a array item by inverse index.
//
template<class T> inline T& TArray<T>::Last( Integer InvIndex=0 )
{
	assert(InvIndex>=0 && InvIndex<Count);
	return ((T*)Data)[Count-InvIndex-1];
}


//
// Pop the last item from the array.
//
template<class T> inline T TArray<T>::Pop()
{
	assert(Count > 0);
	T tmp = ((T*)Data)[Count-1];
	SetNum(Count-1);
	return tmp;
}


//
// Shift the first item from array.
//
template<class T> inline T TArray<T>::Shift()
{
	assert(Count > 0);
	T tmp = ((T*)Data)[0];

	for( Integer i=1; i<Count; i++ )
		((T*)Data)[i-1] = ((T*)Data)[i];

	SetNum(Count-1);
	return tmp;
}


//
// Push a new item to the array and return it
// index.
//
template<class T> inline Integer TArray<T>::Push( const T& InItem )
{
	SetNum( Count+1 );
	((T*)Data)[Count-1] = InItem;
	return Count-1;
}


//
// Unshift a new item to the array and return new
// array length.
//
template<class T> inline Integer TArray<T>::Unshift( const T& InItem )
{
	SetNum( Count+1 );

	for( Integer i=Count-1; i>0; i-- )
		((T*)Data)[i] = ((T*)Data)[i-1];

	((T*)Data)[0] = InItem;
	return Count;
}


//
// Remove item from the array fast, but this
// routine shuffles array.
//
template<class T> inline void TArray<T>::Remove( Integer Idx )
{
	assert(Idx>=0 && Idx<Count);
	assert(Count > 0);
	((T*)Data)[Idx] = ((T*)Data)[Count-1];
	SetNum( Count-1 );
}


//
// Remove item from the array slowly, by shifting
// all items to the released slot.
//
template<class T> inline void TArray<T>::RemoveShift( Integer Idx )
{
	assert(Idx>=0 && Idx<Count);
	assert(Count>0);

	for( Integer i=Idx; i<Count-1; i++ )
		((T*)Data)[i] = ((T*)Data)[i+1];

	SetNum(Count-1);
}


//
// Swap an array's items by its indexes.
//
template<class T> inline void TArray<T>::Swap( Integer A, Integer B )
{
	assert(A>=0 && A<Count);
	assert(B>=0 && B<Count);
	T tmp = ((T*)Data)[A];
	((T*)Data)[A] = ((T*)Data)[B];
	((T*)Data)[B] = tmp;
}


//
// Remove all items matched to InItem from
// the array.
//
template<class T> inline void TArray<T>::RemoveUnique( const T& InItem )
{
	for( ; ; )
	{
		Integer i = FindItem(InItem);
		if( i == -1 ) break;
		Remove(i);
	}
}


//
// Insert item/s to the array starting from Index
// InCount is a how mush items added.
//
template<class T> inline void TArray<T>::Insert( Integer Index, Integer InCount )
{
	assert(InCount >= 0);
	assert(Count >= 0);
	assert(Index>=0 && Index<=Count);

	Integer OldCount = Count;
	SetNum( Count + InCount );

	for( Integer i=Count-1; i>=Index+InCount; i-- )
		((T*)Data)[i] = ((T*)Data)[i-InCount];
}


//
// Quick array sort.
//
template<class T> inline void TArray<T>::Sort( Bool(*SortFunc)( const T&, const T& ) )
{
	if( Count > 1 )
		qSort( 0, Count-1, SortFunc );
}


//
// Reallocate array.
//
template<class T> inline void TArray<T>::SetNum( Integer NewNum )
{
	if( NewNum == Count )
		return;

	for( Integer i=NewNum; i<Count; i++ )
		(&(((T*)Data)[Count-1]))->~T();

	Reallocate( Data, Count, NewNum, sizeof(T) );
}


//
// Internal array sorting.
//
template<class T> inline void TArray<T>::qSort( Integer Min, Integer Max, Bool(*SortFunc)( const T&, const T& ) )
{
	Integer i = Min, j = Max;
	T Middle = ((T*)Data)[Max-(Max-Min)/2];
	while( i < j )
	{
		while( SortFunc( ((T*)Data)[i], Middle ) ) i++;
		while( SortFunc( Middle, ((T*)Data)[j] ) ) j--;
		if( i <= j )
		{
			Swap( i, j );
			i++;
			j--;
		}
	}
	if( Min < j ) qSort( Min, j, SortFunc );
	if( i < Max ) qSort( i, Max, SortFunc );
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/