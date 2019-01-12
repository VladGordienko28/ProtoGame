/*=============================================================================
    FrObject.h: Base class for all classes.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    FObject.
-----------------------------------------------------------------------------*/

//
// An object.
//
class FObject
{
REGISTER_CLASS_H(FObject);
protected:
	// Variables.
	Int32		Id;
	CClass*		Class;
	String		Name;
	FObject*	Owner;
	FObject*	HashNext;

public:
	// FObject interface.
	FObject();
	virtual ~FObject();
	virtual void SerializeThis( CSerializer& S );
	virtual void EditChange();
	virtual void PostLoad();
	virtual void Import( CImporterBase& Im );
	virtual void Export( CExporterBase& Ex );
	String GetFullName();
	Bool IsOwnedBy( FObject* TestOwner );
	void SetOwner( FObject* NewOwner );

	// Accessors.
	inline String GetName()
	{
		return Name;
	}
	inline CClass* GetClass()
	{
		return Class;
	}
	inline Int32 GetId()
	{
		return Id;
	}
	inline FObject* GetOwner()
	{
		return Owner;
	}
	inline Bool IsA( CClass* OtherClass )
	{
		return Class->IsA( OtherClass );
	}

	// Friendly classes.
	friend class CObjectDatabase;
	friend class CApplication;
};


/*-----------------------------------------------------------------------------
    CObjectDatabase.
-----------------------------------------------------------------------------*/

//
// An objects subsystem.
//
class CObjectDatabase: public CClassDatabase
{
public:
	// Tables.
	TArray<FObject*>	GObjects;
	TArray<Int32>		GAvailable;
	FObject*			GHash[2048];

	// Constructor.
	CObjectDatabase();
	virtual ~CObjectDatabase();
	void DropDatabase();

	// CObjectDatabase interface.
	FObject* CreateObject( CClass* InCls, String InName, FObject* InOwner = nullptr );
	FObject* FindObject( String InName, CClass* InCls = FObject::MetaClass, FObject* InOwner = nullptr );
	FObject* CopyObject( FObject* Source, String CopyName = L"", FObject* NewOwner = nullptr );
	String MakeName( CClass* InClass, FObject* InOwner = nullptr );
	void DestroyObject( FObject* InObj, Bool bReleaseRefs = false );
	void SerializeAll( CSerializer& S );
	void HashObject( FObject* Obj );
	void UnhashObject( FObject* Obj );
	void RenameObject( FObject* Obj, String NewName );
	Int32 ReferenceCountTo( FObject* Obj );
};

extern CObjectDatabase*	GObjectDatabase;


/*-----------------------------------------------------------------------------
    Utility.
-----------------------------------------------------------------------------*/

//
// Create a new Flu object.
//
template<class T> inline T* NewObject( String InName = L"", FObject* InOwner = nullptr )
{
	assert(GObjectDatabase); 
	return (T*)GObjectDatabase->CreateObject( T::MetaClass, InName, InOwner );
}


//
// Create a new Flu object.
//
template<class T> inline T* NewObject( CClass* InClass, String InName = L"", FObject* InOwner = nullptr )
{
	assert(GObjectDatabase); 
	return (T*)GObjectDatabase->CreateObject( InClass, InName, InOwner );
}


//
// Destroy a flu object.
//
template<class T> inline void DestroyObject( T* InObj, Bool bReleaseRefs = false )
{
	assert(GObjectDatabase);
	GObjectDatabase->DestroyObject( InObj, bReleaseRefs );
}


//
// Object typecast, if cast are failed return nullptr.
//
template<class T> inline T* As( FObject* ObjRef )
{
	return ObjRef ? ObjRef->IsA(T::MetaClass) ? (T*)ObjRef : nullptr : nullptr;
}


//
// Object duplication.
//
template<class T> inline T* CopyObject( T* Source, String CopyName = L"", FObject* NewOwner = nullptr )
{
	return (T*)GObjectDatabase->CopyObject( Source, CopyName, NewOwner );
}


//
// Object searching.
//
template<class T> inline T* FindObject( String Name, FObject* Owner=nullptr )
{
	return (T*)GObjectDatabase->FindObject( Name, T::MetaClass, Owner );
}


/*-----------------------------------------------------------------------------
    CRefsHolder.
-----------------------------------------------------------------------------*/

//
// An abstract non project class, which hold
// some references to FObjects. When FObject destroyed, its
// references are no more valid, this class help you to 
// cleanup bad references in case object was destroyed.
//
class CRefsHolder
{
public:
	// CRefsHolder interface.
	CRefsHolder();
	virtual ~CRefsHolder();
	virtual void CountRefs( CSerializer& S );

	// List of refs holders.
	static TArray<CRefsHolder*>	GHolders;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/