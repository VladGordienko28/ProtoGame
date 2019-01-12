/*=============================================================================
	FrMemory.h: Super-fast memory allocators.
	Created by Vlad Gordienko, Nov. 2017.
=============================================================================*/

/*-----------------------------------------------------------------------------
	CStackPoolBase.
-----------------------------------------------------------------------------*/

//
// An abstract stack-based memory allocator.
//
class CStackPoolBase
{
public:
	// Constants.
	enum{ DEFAULT_ALIGNMENT = 64 };
	enum{ BLOCK_ALIGNMENT = 8 };

	// CStackPoolBase interface.
	void Pop( const void* NewTop );
	void PopAll();
	void* Push( SizeT NumBytes );
	void* Push0( SizeT NumBytes );
	Bool CanPush( SizeT NumBytes ) const;

protected:
	// Variables.
	SizeT		Size;
	UInt8*		Top;
	UInt8		*StartAdd, *EndAddr;

	// No default constructor.
	CStackPoolBase()
		: Size(0), Top(nullptr), StartAdd(nullptr), EndAddr(nullptr)
	{}
};


/*-----------------------------------------------------------------------------
	CStaticPool.
-----------------------------------------------------------------------------*/

//
// Static stack-based pool. Uses static or local memory.
// So, no heap allocation required.
//
template<SizeT N> class CStaticPool: public CStackPoolBase
{
public:
	// CStaticPool interface.
	CStaticPool()
	{
		Size	= N;
		Top		= StartAdd = Buffer;
		EndAddr	= StartAdd + Size;
		MemZero( Buffer, sizeof(Buffer) );
	}
	~CStaticPool()
	{}

	// Allocation.
	template<class T> inline T* PushT()
	{
		return CanPush(sizeof(T)) ? (T*)Push0(sizeof(T)) : nullptr;
	}

private:
	// Variables.
	UInt8	Buffer[align(N, DEFAULT_ALIGNMENT)];
};


/*-----------------------------------------------------------------------------
	CDynamicPool.
-----------------------------------------------------------------------------*/

//
// Dynamic pool which uses once allocated heap memory.
//
class CDynamicPool: public CStackPoolBase
{
public:
	// CDynamicPool interface.
	CDynamicPool( SizeT InSize )
	{
		Size	= InSize;
		Top		= StartAdd = (UInt8*)MemAlloc(align(Size, DEFAULT_ALIGNMENT));
		EndAddr	= StartAdd + Size;
	}
	~CDynamicPool()
	{
		MemFree(StartAdd);
		Size	= 0;
		EndAddr	= StartAdd = Top = nullptr;
	}

	// Allocation.
	template<class T> inline T* PushT()
	{
		return CanPush(sizeof(T)) ? (T*)Push0(sizeof(T)) : nullptr;
	}
};


/*-----------------------------------------------------------------------------
	CPagePool.
-----------------------------------------------------------------------------*/

//
// Page based allocator.
//
template<SizeT PAGE_SIZE> class CPagePool
{
public:
	// Constants.
	enum{ BLOCK_ALIGNMENT = 8 };

	// CPagePool interface.
	CPagePool();
	~CPagePool();
	void* Alloc( SizeT NumBytes );

private:
	// Variables.
	struct TPage
	{
		UInt8*	Data;
		SizeT	NumUsed;
	};
	TArray<TPage>	Pages;
	Int32 AddPage();
};


/*-----------------------------------------------------------------------------
	CStackPoolBase implementation.
-----------------------------------------------------------------------------*/

//
// Allocate a dirty memory. If this allocation failes return
// nullptr.
//
inline void* CStackPoolBase::Push( SizeT NumBytes )
{
	NumBytes	= align( NumBytes, BLOCK_ALIGNMENT );
	void* Res	= Top;

	if( Top+NumBytes > EndAddr )
		return nullptr;

	Top += NumBytes;
	return Res;
}


//
// Allocate and initialize a memory. If this allocation failes return
// nullptr.
//
inline void* CStackPoolBase::Push0( SizeT NumBytes )
{
	NumBytes	= align( NumBytes, BLOCK_ALIGNMENT );
	void* Res	= Top;

	if( Top+NumBytes > EndAddr )
		return nullptr;

	Top += NumBytes;
	MemZero( Res, NumBytes );
	return Res;
}


//
// Return true if pool can push NumBytes.
//
inline Bool CStackPoolBase::CanPush( SizeT NumBytes ) const
{
	return ((SizeT)Top + align(NumBytes, BLOCK_ALIGNMENT)) < (SizeT)EndAddr;
}


//
// Pop a memory block which was allocated following
// NewTop address.
//
inline void CStackPoolBase::Pop( const void* NewTop )
{
	Top	= (UInt8*)NewTop;
	assert(Top>=StartAdd && Top<EndAddr);
}


//
// Pop all pushed block, your references are still
// valid but may be corrupted.
//
inline void CStackPoolBase::PopAll()
{
	Top	= StartAdd;
}


/*-----------------------------------------------------------------------------
	CPagePool implementation.
-----------------------------------------------------------------------------*/

//
// PagePool constructor.
//
template<SizeT PAGE_SIZE> inline CPagePool<PAGE_SIZE>::CPagePool()
{
	f_guard;
	// At least one page.
	AddPage();
}


//
// Page pool destructor.
//
template<SizeT PAGE_SIZE> inline CPagePool<PAGE_SIZE>::~CPagePool()
{
	f_guard;
	for( Integer iPage=0; iPage<Pages.Num(); iPage++ )
	{
		TPage& Page = Pages[iPage];
		MemFree( Page.Data );
		Page.NumUsed = 0;
	}
	Pages.Empty();
}


//
// Allocate a new memory block.
//
template<SizeT PAGE_SIZE> inline void* CPagePool<PAGE_SIZE>::Alloc( SizeT NumBytes )
{
	f_guard;
	NumBytes = align( NumBytes, BLOCK_ALIGNMENT );

	// Make new page if required.
	if( Pages.Last().NumUsed+NumBytes >= PAGE_SIZE )
		AddPage();

	TPage& Page = Pages.Last();

	void* Res = Page.Data + Page.NumUsed;
	Page.NumUsed += NumBytes;
	return Res;
}


//
// Add a new page to pages list.
//
template<SizeT PAGE_SIZE> inline Int32 CPagePool<PAGE_SIZE>::AddPage()
{
	f_guard;

	TPage New;
	New.Data	= (Byte*)MemAlloc(PAGE_SIZE);
	New.NumUsed	= 0;
	assert(New.Data);

	return Pages.Push(New);
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/