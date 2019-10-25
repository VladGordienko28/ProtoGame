/*=============================================================================
    FrApp.h: An abstract application class.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    CApplication implementation.
-----------------------------------------------------------------------------*/

//
// Application constructor.
//
CApplication::CApplication()
	:	GAudio( nullptr ),
		GInput( nullptr ),
		Project( nullptr ),
		FPS( 0 )
{
	// Set up pointer to self.
	assert(!GApp);
	GApp	= this;
}


//
// Application destructor.
//
CApplication::~CApplication()
{
	assert(GApp==this);
	GApp	= nullptr;
}


//
// Release temporal audio/render cache when
// level finished or game reloading.
//
void CApplication::Flush()
{
	if( /*GIsEditor*/ true )
	{
		// Flush in editor.
		GAudio->Flush();
		//GRender->Flush();
	}
	else
	{
		// Flush in game.
		GAudio->Flush();
		//GRender->Flush();

		if( Project )
			Project->BlockMan->Flush();
	}

	// Cleanup strings sometimes.
	g_ansiStringManager.cleanupPool( true );
	g_wideStringManager.cleanupPool( true );

	// Flush singletons.
	CDebugDrawHelper::Instance().Reset();
}


//
// Change application caption.
//
void CApplication::SetCaption( String NewCaption )
{
}


//
// Execute shell command.
//
void CApplication::ConsoleExecute( String Cmd )
{
}


//
// Global application instance.
//
CApplication*	GApp	= nullptr;


/*-----------------------------------------------------------------------------
    Game loading and saving.
-----------------------------------------------------------------------------*/

//
// Main information associated to
// the game file.
//
struct TGameFileInfo
{
public:
	UInt32			Control;
	String			EngineVer;

	friend void Serialize( CSerializer& S, TGameFileInfo& V )
	{
		Serialize( S, V.Control );
		Serialize( S, V.EngineVer );
	}
};


//
// An Information relative to Flu object.
//
struct TObjectHeader
{
public:
	Int32		Id;
	String		Name;
	UInt16		iClass;
	Int32		iOwner;

	friend void Serialize( CSerializer& S, TObjectHeader& V )
	{
		Serialize( S, V.Id );
		Serialize( S, V.Name );
		Serialize( S, V.iClass );
		Serialize( S, V.iOwner );
	}
};


//
// Return class index in ClassDatabase.
//
Int32 GetClassIndex( CClass* Class )
{
	for( Int32 i=0; i<CClassDatabase::GClasses.size(); i++ )
		if( Class == CClassDatabase::GClasses[i] )
			return i;

	return -1;
}


//
// Save entire game to file and
// entire resources. Warning Name should be without
// file extension.
//
Bool CApplication::SaveGame( String Directory, String Name )
{
	// Don't save if nothing to save.
	if( !Project )
		return false;
	
	String RealDir = Directory + L"\\";
	String ProjFile = Name + PROJ_FILE_EXT;

	FileSaverAdapter Saver( fm::writeBinaryFile( *(RealDir + ProjFile) ) );

	// Save general file information.
	{
		TGameFileInfo	Info;
		Info.Control	= 2528;
		Info.EngineVer	= FLU_VERSION;
		Serialize( Saver, Info );
	}

	// Save remap table for each class.
	{
		Int32 NumCls = CClassDatabase::GClasses.size();
		Serialize( Saver, NumCls );

		for( Int32 i=0; i<NumCls; i++ )
		{
			CClass*	Class = CClassDatabase::GClasses[i];
			Serialize( Saver, Class->Name );
		}
	}

	// Save global objects database info.
	{
		Int32 DbSize = Project->GObjects.size();
		Serialize( Saver, DbSize );

		// Count how much non-null objects.
		Int32 DbRealSize = 0;
		for( Int32 i=0; i<Project->GObjects.size(); i++ )
			if( Project->GObjects[i] )
				DbRealSize++;
		Serialize( Saver, DbRealSize );

		// List of non-null object headers.
		for( Int32 i=0; i<Project->GObjects.size(); i++ )
			if( Project->GObjects[i] )
			{
				TObjectHeader Header;
				FObject* Object = Project->GObjects[i];

				Header.Id			= Object->GetId();
				Header.Name			= Object->GetName();
				Header.iClass		= GetClassIndex(Object->GetClass());
				Header.iOwner		= Object->GetOwner() ? Object->GetOwner()->GetId() : -1;
				Serialize( Saver, Header );
			}
	}

	// Save content of each script.
	for( Int32 i=0; i<Project->GObjects.size(); i++ )
		if( Project->GObjects[i] && Project->GObjects[i]->IsA(FScript::MetaClass) )
			Project->GObjects[i]->SerializeThis( Saver );

	// Save content of each object, without scripts.
	for( Int32 i=0; i<Project->GObjects.size(); i++ )
		if( Project->GObjects[i] && !Project->GObjects[i]->IsA(FScript::MetaClass) )
			Project->GObjects[i]->SerializeThis( Saver );

	// Save all project stuff.
	Project->SerializeProject( Saver );

	// Save all resources to the other file.
	String ResFile = Name + RES_FILE_EXT;
	Project->BlockMan->SaveAllBlocks(RealDir+ResFile);

	// Ok.
	return true;
}


//
// Load entire game from file and
// entire resources. Warning Name should be without
// file extension.
//
Bool CApplication::LoadGame( String Directory, String Name )
{	
	// Unload old cache.
	Flush();

	// Release old project.
	delete Project;
	GObjectDatabase = nullptr;
	GProject		= nullptr;
	Project			= nullptr;

	// Allocate new project.
	Project				= new CProject();

	String RealDir = Directory + L"\\";
	String ProjFile = Name + PROJ_FILE_EXT;

	FileLoaderAdapter Loader( fm::readBinaryFile( *(RealDir + ProjFile) ) );

	// Load general file information.
	{
		TGameFileInfo	Info;
		Serialize( Loader, Info );

		if( Info.Control != 2528 )
			fatal( L"File '%s' is not a Fluorine Game", *(RealDir + ProjFile) );

		if( Info.EngineVer != FLU_VERSION )
			fatal( L"Old Fluorine File" );
	}
	
	// Load classes remap table.
	Array<CClass*>	Classes(CClassDatabase::GClasses.size());
	{
		Int32 NumCls;
		Serialize( Loader, NumCls );
		if( NumCls != CClassDatabase::GClasses.size() )
			fatal( L"Classes database is outdated" );

		for( Int32 i=0; i<NumCls; i++ )
		{
			String ClassName;
			Serialize( Loader, ClassName );
			CClass* Class = CClassDatabase::StaticFindClass( *ClassName );
			if( !Class )
				fatal( L"Class '%s' not found in database", *ClassName );

			// Add to remap table.
			Classes[i]	= Class;
		}
	}

	// Load global objects database.
	{
		Int32 DbSize, DbRealSize;
		Serialize( Loader, DbSize );
		Serialize( Loader, DbRealSize );

		// Allocate db.
		Project->GObjects.setSize( DbSize );

		// Load all headers and allocate objects, but don't initialize.
		Array<TObjectHeader>	Headers(DbRealSize);
		for( Int32 i=0; i<DbRealSize; i++ )
		{
			Serialize( Loader, Headers[i] );

			TObjectHeader& H = Headers[i]; 
			FObject* Result = Classes[H.iClass]->Constructor();
			assert(Result);

			Project->GObjects[H.Id]	= Result;
		}
		
		// Initialize general object's fields.
		for( Int32 i=0; i<DbRealSize; i++ )
		{
			TObjectHeader& H	= Headers[i]; 
			FObject*	Object	= Project->GObjects[H.Id];

			// Set fields.
			Object->Id		= H.Id;
			Object->Class	= Classes[H.iClass];
			Object->Name	= H.Name;
			Object->Owner	= H.iOwner!=-1 ? Project->GObjects[H.iOwner] : nullptr;

			// Add to hash.
			Project->HashObject( Object );
		}

		// Fill list of available slots.
		Project->GAvailable.empty();
		for( Int32 i=0; i<Project->GObjects.size(); i++ )
			if( Project->GObjects[i] == nullptr )
				Project->GAvailable.push( i );
	}
	
	// Load content of each script.
	for( Int32 i=0; i<Project->GObjects.size(); i++ )
		if( Project->GObjects[i] && Project->GObjects[i]->IsA(FScript::MetaClass) )
			Project->GObjects[i]->SerializeThis( Loader );
	
	// Load content of each object, without scripts.
	for( Int32 i=0; i<Project->GObjects.size(); i++ )
		if( Project->GObjects[i] && !Project->GObjects[i]->IsA(FScript::MetaClass) )
			Project->GObjects[i]->SerializeThis( Loader );
	
	// Refill database for each level.
	for( Int32 i=0; i<Project->GObjects.size(); i++ )
		if( Project->GObjects[i] && Project->GObjects[i]->IsA(FLevel::MetaClass) )
		{
			FLevel* Level = (FLevel*)Project->GObjects[i];

			for( Int32 iEntity=0; iEntity<Level->Entities.size(); iEntity++ )
			{
				FEntity* Entity = Level->Entities[iEntity];

				FBaseComponent* Base = Entity->Base;
				Entity->Base	= nullptr;
				Base->InitForEntity( Entity );

				Array<FExtraComponent*> Comps = Entity->Components;
				Entity->Components.empty();
				for( Int32 e=0; e<Comps.size(); e++ )
					Comps[e]->InitForEntity( Entity );
			}
		}

	// Load all project stuff.
	Project->SerializeProject( Loader );

	// Save all resources to the other file.
	String ResFile = Name + RES_FILE_EXT;
	if( /*GIsEditor*/ true )
	{
		// Initialize BlockManager for editor.
		Project->BlockMan	= new CBlockManager();
		Project->BlockMan->LoadAllBlocks(RealDir+ResFile);
	}
	else
	{
		// Initialize BlockManager for game.
		Project->BlockMan	= new CBlockManager(RealDir+ResFile);
		Project->BlockMan->LoadMetadata();
	}
	
	// Notify all objects about loading.
	for( Int32 i=0; i<Project->GObjects.size(); i++ )
		if( Project->GObjects[i] )
			Project->GObjects[i]->PostLoad();

	// Ok.
	return true;
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/