/*=============================================================================
    FrObject.cpp: Objects database implementation.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "..\Engine.h"

//
// Global access database.
//
CObjectDatabase*		GObjectDatabase	= nullptr;
TArray<CRefsHolder*>	CRefsHolder::GHolders;


/*-----------------------------------------------------------------------------
    FObject implementation.
-----------------------------------------------------------------------------*/

//
// FObject constructor.
//
FObject::FObject()
	:	Id( -1 ),
		Class( FObject::MetaClass ),
		Name(),
		Owner( nullptr ),
		HashNext( nullptr )
{
}


//
// FObject destructor.
//
FObject::~FObject()
{
}


//
// FObject serialization.
//
void FObject::SerializeThis( CSerializer& S )
{
	// Is should serialize FObject info?
}


//
// Called when property changed via ObjectInspector.
//
void FObject::EditChange()
{
}


//
// Called after loading, to perform some
// precalculations.
//
void FObject::PostLoad()
{
}


//
// Should be implemented in subclasses,
// process only this fields. Used
// to restore data from text file.
//
void FObject::Import( CImporterBase& Im )
{
	// Export all field.
	CClass* WalkClass = GetClass();
	while( WalkClass )
	{
		for( Int32 iProp=0; iProp<WalkClass->Properties.Num(); iProp++ )
		{
			CProperty* Prop = WalkClass->Properties[iProp];

			if( !(Prop->Flags & PROP_NoImEx) )
				Prop->Import( (UInt8*)this + Prop->Offset, Im );
		}

		WalkClass = WalkClass->Super;
	}
}


//
// Should be implemented in subclasses,
// process only this fields. Used
// to store data to text file.
//
void FObject::Export( CExporterBase& Ex )
{
	// Export all field.
	CClass* WalkClass = GetClass();
	while( WalkClass )
	{
		for( Int32 iProp=0; iProp<WalkClass->Properties.Num(); iProp++ )
		{
			CProperty* Prop = WalkClass->Properties[iProp];

			if( !(Prop->Flags & PROP_NoImEx) )
				Prop->Export( (UInt8*)this + Prop->Offset, Ex );
		}

		WalkClass = WalkClass->Super;
	}
}


//
// Return full name of object.
//
String FObject::GetFullName()
{
	FObject* Obj = Owner;
	String Res = Name;

	while( Obj )
	{
		Res = Obj->Name + L"." + Res;
		Obj = Obj->Owner;
	}

	return Res;
}


//
// Return true, if this object owned by TestOwner.
//
Bool FObject::IsOwnedBy( FObject* TestOwner )
{
	FObject* O = Owner;

	while( O )
	{
		if( O == TestOwner )
			return true;
		O = O->Owner;
	}

	return false;
}


//
// Set an object owner. Be carefully with this function,
// set it only initially.
//
void FObject::SetOwner( FObject* NewOwner )
{
	Owner	= NewOwner;
}


/*-----------------------------------------------------------------------------
    CRefsHolder implementation.
-----------------------------------------------------------------------------*/

//
// Refs holder constructor.
//
CRefsHolder::CRefsHolder()
{
	// Just register it.
	GHolders.Push( this );
}


//
// Refs holder destructor.
//
CRefsHolder::~CRefsHolder()
{
	// Remove from list.
	assert(GHolders.FindItem(this) != -1);
	GHolders.RemoveUnique(this);
}


//
// A virtual function to provide cleanup
// references, just serialize references
// like this: "Serialize( S, MyObject );",
// and its enough.
//
void CRefsHolder::CountRefs( CSerializer& S )
{
}


/*-----------------------------------------------------------------------------
    CObjectDatabase implementation.
-----------------------------------------------------------------------------*/

//
// Database constructor.
//
CObjectDatabase::CObjectDatabase()
	:	GObjects(),
		GAvailable()
{
	// Store as global accessible.
	GObjectDatabase = this;
	mem::zero( GHash, sizeof(GHash) );
}


//
// Database destructor.
//
CObjectDatabase::~CObjectDatabase()
{
	DropDatabase();
}


//
// Database destructor.
//
void CObjectDatabase::DropDatabase()
{
	// Really, really sad, but drop database it totally complex
	// thing. Its should be destroyed in proper order, since some
	// objects needs others for destruction! Its hard to kill
	// objects graph linear. However We can use opportunity
	// of flu architecture. I have a blueprint of engine
	// front of my eyes, and I see: we can destroy entities
	// first of all, since their destructor are crazy. Its
	// uses FScript and InstanceBuffer. Then destroy all resources
	// since they are pretty independent. Its should no remain objects
	// in database, but we anyway checks for objects, we lost references.
	// Not even dare to throw out refs changer, since even editor uses
	// pointer to resources.

	// Kill all entities.
	for( Int32 i=0; i<GObjects.Num(); i++ )
		if( GObjects[i] && GObjects[i]->IsA(FEntity::MetaClass) )
			DestroyObject( GObjects[i], true );

	// Kill all resources.
	for( Int32 i=0; i<GObjects.Num(); i++ )
		if( GObjects[i] && GObjects[i]->IsA(FResource::MetaClass) )
			DestroyObject( GObjects[i], true );

	// Kill all rest objects, just in case.
	for( Int32 i=0; i<GObjects.Num(); i++ )
		if( GObjects[i] )
		{
			notice( L"Unreferenced object '%s'", *GObjects[i]->GetName() );
			DestroyObject( GObjects[i], true );
		}

	// Empty tables.
	GAvailable.Empty();
	GObjects.Empty();
}


//
// Serialize all objects.
//
void CObjectDatabase::SerializeAll( CSerializer& S )
{
	for( Int32 i=0; i<GObjects.Num(); i++ )
		if( GObjects[i] )
			GObjects[i]->SerializeThis( S );

	// List of holders.
	if( S.GetMode() == SM_Undefined )
		for( Int32 i=0; i<CRefsHolder::GHolders.Num(); i++ )
			CRefsHolder::GHolders[i]->CountRefs( S );
}


//
// Create an object. Use only this function
// for scripted object because it initialize
// all fields, prepare to script execution and
// register object.
//
FObject* CObjectDatabase::CreateObject( CClass* InCls, String InName, FObject* InOwner )
{
	assert(InCls);

    // Get name, if not specified.
	String ObjName = InName ? InName : MakeName( InCls, InOwner );

	// Test for abstractness.
	if( InCls->Flags & CLASS_Abstract )
		error( L"Attempt to create an abstract object '%s' of class '%s'", *InName, *InCls->GetAltName() );

	// Create instance.
	FObject* Result = InCls->Constructor();
	assert(Result);

	// Initialize FObject fields.
	Result->Name	= ObjName;
	Result->Class	= InCls;
	Result->Owner	= InOwner;

	// Register object.
	if( GAvailable.Num() > 0 )
	{
		// Put to the available slot.
		Result->Id				= GAvailable.Pop();
		GObjects[Result->Id]	= Result;
	}
	else
	{
		// Allocate new one.
		Result->Id	= GObjects.Push(Result);
	}
	
	// Add to hash.
	HashObject( Result );

#if 0
	// dbg: temporal.
	log	( L"ObjMan: Object created Name=\"%s\", Id=%d, Class=\"%s\", Owner=\"%s\" ", 
			*Result->GetName(), 
			Result->GetId(), 
			*Result->GetClass()->Name, 
			Result->GetOwner() ? *Result->GetOwner()->GetName() : L"nullptr" 
		);
#endif

	return Result;
}


//
// Add object to the hash.
//
void CObjectDatabase::HashObject( FObject* Obj )
{
	Int32 iHash	= 2047 & Obj->GetName().HashCode();
	Obj->HashNext	= GHash[iHash];
	GHash[iHash]	= Obj;
}


//
// Remove object from the hash.
//
void CObjectDatabase::UnhashObject( FObject* Obj )
{
	Int32 iHash	= 2047 & Obj->GetName().HashCode();
	FObject** Link	= &GHash[iHash];
	while( *Link )
	{
		if( *Link != Obj )
		{
			// Goto next.
			Link	= &(*Link)->HashNext;
		}
		else
		{
			// Found.
			*Link	= (*Link)->HashNext;
			break;
		}
	}
}


//
// Rename an object.
//
void CObjectDatabase::RenameObject( FObject* Obj, String NewName )
{
	assert(Obj && NewName);
	assert(FindObject(NewName)==nullptr);

	UnhashObject( Obj );
	{
		Obj->Name	= NewName;
	}
	HashObject( Obj );
}


//
// Find the object in object's table, really SLOW, so try to call it very
// seldom.
//
FObject* CObjectDatabase::FindObject( String InName, CClass* InCls, FObject* InOwner )
{ 
	assert(InCls);

	if( InOwner )
	{
		// Search in the InOwner scope.
		Int32 iHash = 2047 & InName.HashCode();
		FObject* Obj = GHash[iHash];

		while( Obj )
		{
			if	(	Obj->IsOwnedBy(InOwner) &&
					Obj->IsA(InCls) &&
					Obj->GetName() == InName
				)
					return Obj;

			Obj		= Obj->HashNext;
		}
	}
	else
	{
		// Search in the global scope.
		Int32 iHash = 2047 & InName.HashCode();
		FObject* Obj = GHash[iHash];

		while( Obj )
		{
			if	(	Obj->IsA(InCls) &&
					Obj->GetName() == InName
				)
					return Obj;

			Obj		= Obj->HashNext;
		}
	}

	// Not found.
	return nullptr;
}


//
// Generate an unique name for object.
//
String CObjectDatabase::MakeName( CClass* InClass, FObject* InOwner )
{
	assert(InClass);

	for( Int32 iUniq = 0;; iUniq++ )
	{
		String TestName = String::Format( L"%s%d", *InClass->GetAltName(), iUniq );
		
		// Test for unique.
		if( !FindObject( TestName, InClass, InOwner ) )
			return TestName;
	}
}


//
// Serializer to change references to the object.
//
class CRefChanger: public CSerializer
{
public:
	FObject*	Fungible;
	FObject*	Vicarious;

	// CRefChanger interface.
	CRefChanger( FObject* InFngbl, FObject* InVcrs )
		:	Fungible( InFngbl ),
			Vicarious( InVcrs )
	{
		Mode = SM_Undefined;
	}
	~CRefChanger()
	{}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count )
	{}
	void SerializeRef( FObject*& Obj )
	{
		if( Obj == Fungible )
			Obj = Vicarious;
	}
};


//
// Destroy an object, use only it, not destructor directly.
//
void CObjectDatabase::DestroyObject( FObject* InObj, Bool bReleaseRefs )
{ 
	if( !InObj )	return;
	
#if 0
	// Dbg.
	log( L"ObjMan: Object \"%s\" destroyed", *InObj->GetName() );
#endif

	// Release refs if any.
	if( bReleaseRefs )
	{
		CRefChanger R( InObj, nullptr );
		SerializeAll( R );
	}

	// Unregister.
	UnhashObject( InObj );
	GAvailable.Push(InObj->Id);
	GObjects[InObj->Id] = nullptr;

	// Lets C++ perform rest job.
	delete InObj;
}


/*-----------------------------------------------------------------------------
	Object duplication.
-----------------------------------------------------------------------------*/

// The temporal buffer.
// Pretty "dirty" solution, but its really fast,
// critical for in game objects spawn.
UInt8	GBuffer[65536];

// Serializer to store object data.
class CObjectSaver: public CSerializer
{
public:
	Int32		Offset;

	CObjectSaver()
	{
		Mode	= SM_Save;
		Offset	= 0;
	}
	void SerializeData( void* Mem, SizeT Count )
	{
		assert(Offset<65536);
		mem::copy( &GBuffer[Offset], Mem, Count );
		Offset += Count;
	}
	void SerializeRef( FObject*& Obj )
	{
		Int32 Id = Obj ? Obj->GetId() : -1;
		Serialize( *this, Id );
	}
};


// Serializer to restore object data.
class CObjectLoader: public CSerializer
{
public:
	Int32		Offset;

	CObjectLoader()
	{
		Mode	= SM_Load;
		Offset	= 0;
	}
	void SerializeData( void* Mem, SizeT Count )
	{
		assert(Offset<65536);
		mem::copy( Mem, &GBuffer[Offset], Count );
		Offset += Count;
	}
	void SerializeRef( FObject*& Obj )
	{
		Int32 Id;
		Serialize( *this, Id );
		Obj = Id != -1 ? GObjectDatabase->GObjects[Id] : nullptr;
	}
};


//
// Duplicate an object.
//
FObject* CObjectDatabase::CopyObject( FObject* Source, String CopyName, FObject* NewOwner )
{ 
	assert(Source);

	// Create object.
	FObject* Result = NewObject<FObject>( Source->GetClass(), CopyName, NewOwner );

	// Copy values.
	CObjectSaver Saver;
	CObjectLoader Loader;

	Source->SerializeThis( Saver );
	Result->SerializeThis( Loader );

	return Result;
}


/*-----------------------------------------------------------------------------
    References counter.
-----------------------------------------------------------------------------*/

//
// Serializer to count references to the object.
//
class CRefsCounter: public CSerializer
{
public:
	FObject*	Target;
	Int32&	Counter;

	// CRefsCounter interface.
	CRefsCounter( FObject* InTarget, Int32& InCounter )
		:	Target( InTarget ),
			Counter( InCounter )
	{
		Mode = SM_Undefined;
	}
	~CRefsCounter()
	{}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count )
	{}
	void SerializeRef( FObject*& Obj )
	{
		if( Obj == Target )
			Counter++;
	}
};


//
// Count references to concrete object.
//
Int32 CObjectDatabase::ReferenceCountTo( FObject* Obj )
{
	if( !Obj )
		return 0;

	// Walk through entire database.
	Int32 Counter = 0;
	CRefsCounter RCon( Obj, Counter );
	SerializeAll( RCon );

	return Counter;
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_BASE_CLASS_CPP( FObject, CLASS_Abstract );


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/