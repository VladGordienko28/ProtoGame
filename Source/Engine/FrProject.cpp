/*=============================================================================
    FrProject.cpp: Project.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    CProject implementation.
-----------------------------------------------------------------------------*/

//
// Constructor for project loading.
//
CProject::CProject()
	:	Info( nullptr )
{
	// Store pointer.
	GProject	= this;
}


//
// Project destruction.
//
CProject::~CProject()
{  
	// Kill database of FObjects.
	DropDatabase();

	// Release BlockManager.
	delete BlockMan;
	GProject		= nullptr;
}


//
// Project serialization, used to store
// or restore project required data.
//
void CProject::SerializeProject( CSerializer& S ) 
{ 
	// Save information about project.
	Serialize( S, Info ); 
};


//
// Globals.
//
CProject*	GProject	= nullptr;


/*-----------------------------------------------------------------------------
	Level duplication.
-----------------------------------------------------------------------------*/

//
// CLevelRefChanger - simple modification of
// the CRefChanger to make deal with levels and their entities 
// and components.
//
class CLevelRefChanger: public CSerializer
{
public:
	// Variables.
	FLevel*			NewLevel;
	FLevel*			OldLevel;

	// CLevelRefChanger interface.
	CLevelRefChanger( FLevel* InNewLevel, FLevel* InOldLevel )
		:	NewLevel( InNewLevel ),
			OldLevel( InOldLevel )
	{
		Mode	= SM_Undefined;
	}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count );
	void SerializeRef( FObject*& Obj );		
};


void CLevelRefChanger::SerializeData( void* Mem, SizeT Count )
{
}


void CLevelRefChanger::SerializeRef( FObject*& Obj )
{
	if( !Obj )
		return;

	if( Obj->IsA(FEntity::MetaClass) )
	{
		FEntity* Entity = (FEntity*)Obj;
		if( Entity->Level == OldLevel )
		{
			Obj	= NewLevel->Entities[OldLevel->GetEntityIndex(Entity)];
		}
	}
	else if( Obj->IsA(FBaseComponent::MetaClass) )
	{
		FBaseComponent* Base = (FBaseComponent*)Obj;
		if( Base->Entity->Level == OldLevel )
		{
			Obj = NewLevel->Entities[OldLevel->GetEntityIndex(Base->Entity)]->Base;
		}
	}
	else if( Obj->IsA(FExtraComponent::MetaClass) )
	{
		FExtraComponent* Extra = (FExtraComponent*)Obj;
		if( Extra->Entity->Level == OldLevel )
		{
			Obj = NewLevel->Entities[OldLevel->GetEntityIndex(Extra->Entity)]->Components[Extra->Entity->Components.find(Extra)];
		}
	}
	// All others types of objects are shared, such
	// as FResource derived.
}


//
// Duplicate a level, used clone used to play on it.
//
FLevel* CProject::DuplicateLevel( FLevel* Source )
{
	// Validate.
	assert(Source);
	assert(!Source->IsTemporal());

	// Allocate a new level and make some initialization.
	FLevel* Result			= NewObject<FLevel>( Source->GetName()+L"Copy" );
	Result->Original		= Source;
	Result->RndFlags		= Source->RndFlags;

	// Allocate a list of the entities matched as Source level.
	// it's important to store proper order, since refs will be
	// changed using it's index in level's database.
	// Warning should be works like FEntity::Init.
	for( Int32 iEntity=0; iEntity<Source->Entities.size(); iEntity++ )
	{
		FEntity* OldEntity = Source->Entities[iEntity];
		FEntity* NewEntity = NewObject<FEntity>( OldEntity->GetName(), Result );

		NewEntity->Level		= Result;
		NewEntity->Script		= OldEntity->Script;

		// Copy components.
		FBaseComponent* BasCom = (FBaseComponent*)GObjectDatabase->CopyObject
																		( 
																			OldEntity->Base, 
																			OldEntity->Base->GetName(), 
																			NewEntity 
																		);
		BasCom->InitForEntity( NewEntity );

		// Extra components.
		for( Int32 i=0; i<OldEntity->Components.size(); i++ )
		{
			FExtraComponent* Extra = OldEntity->Components[i];
			FExtraComponent* Com = (FExtraComponent*)GObjectDatabase->CopyObject
																			( 
																				Extra, 
																				Extra->GetName(), 
																				NewEntity 
																			);
			Com->InitForEntity( NewEntity );
		}

		// Initialize instance buffer.
		if( NewEntity->Script->InstanceBuffer )
		{
			NewEntity->InstanceBuffer = new CInstanceBuffer( NewEntity->Script->Properties );
			NewEntity->InstanceBuffer->Data.setSize( NewEntity->Script->InstanceSize );

			// Copy data from the source entity.
			if( NewEntity->Script->Properties.size() )
				NewEntity->InstanceBuffer->CopyValues( &OldEntity->InstanceBuffer->Data[0] );
		}

		// Add entity to the level's db.
		Result->Entities.push( NewEntity );
	}

	// Level's entities and their components are initialized
	// now, but still object's has references to the objects
	// on the old level, here we fix them.
	CLevelRefChanger RefChanger( Result, Source );
	for( Int32 iEntity=0; iEntity<Result->Entities.size(); iEntity++ )
	{
		FEntity* Entity	= Result->Entities[iEntity];

		// Serialize only component's because they are refer.
		Entity->Base->SerializeThis( RefChanger );
		for( Int32 e=0; e<Entity->Components.size(); e++ )
			Entity->Components[e]->SerializeThis( RefChanger );

		// ..but also change references in the instance buffer.
		if( Entity->InstanceBuffer )
			Entity->InstanceBuffer->SerializeValues( RefChanger );
	}

	// Send after loading notification to the each
	// entity and component to set up temporal stuff.
	// This action is logically has sense.
	for( Int32 iEntity=0; iEntity<Result->Entities.size(); iEntity++ )
	{
		FEntity* Entity	= Result->Entities[iEntity];

		Entity->PostLoad();
		Entity->Base->PostLoad();
		for( Int32 e=0; e<Entity->Components.size(); e++ )
			Entity->Components[e]->PostLoad();
	}

	// Copy navigator.
	if( Source->Navigator )
	{
		Result->Navigator			= new CNavigator(Result);
		Result->Navigator->Nodes	= Source->Navigator->Nodes;
		Result->Navigator->Edges	= Source->Navigator->Edges;
		Serialize( RefChanger, Result->Navigator );
	}

	// Copy level's variables.
	Result->GameSpeed		= Source->GameSpeed;
	Result->Soundtrack		= Source->Soundtrack;
	Result->Camera			= Source->Camera;
	Result->AmbientLight	= Source->AmbientLight;
	Result->BlurIntensity	= Source->BlurIntensity;
	mem::copy( Result->Effect, Source->Effect, sizeof(FLevel::Effect) );

	Result->m_ambientColors = Source->m_ambientColors;
	Result->m_dawnBitmap = Source->m_dawnBitmap;
	Result->m_midnightBitmap = Source->m_midnightBitmap;
	Result->m_noonBitmap = Source->m_noonBitmap;
	Result->m_duskBitmap = Source->m_duskBitmap;

	Result->m_vignetteIntensity = Source->m_vignetteIntensity;
	Result->m_vignetteInnerRadius = Source->m_vignetteInnerRadius;
	Result->m_vignetteOuterRadius = Source->m_vignetteOuterRadius;

	warn( L"World: Level \"%s\" duplicated", *Source->GetName() );
	return Result;
}


/*-----------------------------------------------------------------------------
    FProjectInfo implementation.
-----------------------------------------------------------------------------*/

//
// Project info construction.
//
FProjectInfo::FProjectInfo()
	:	FResource(),
		GameName( L"<Game>" ),
		Author( L"<Author>" ),
		bNoPause( false ),
		DefaultWidth( 800 ),
		DefaultHeight( 600 ),
		WindowType( WT_Sizeable ),
		bQuitByEsc( false )
{
}


//
// Project info serialization.
//
void FProjectInfo::SerializeThis( CSerializer& S )
{
	FResource::SerializeThis( S );
	Serialize( S, GameName );
	Serialize( S, Author );
	Serialize( S, bNoPause );
	Serialize( S, bQuitByEsc );
	SerializeEnum( S, WindowType );
	Serialize( S, DefaultWidth );
	Serialize( S, DefaultHeight );
}


//
// Some field changed in the inspector.
//
void FProjectInfo::EditChange()
{
	FResource::EditChange();
}


//
// Initialize project info after loading.
//
void FProjectInfo::PostLoad()
{
	FResource::EditChange();
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FProjectInfo, FResource, CLASS_Sterile )
{
	BEGIN_ENUM(EAppWindowType);
		ENUM_ELEM(WT_Sizeable);
		ENUM_ELEM(WT_Single);
		ENUM_ELEM(WT_FullScreen);
	END_ENUM;

	ADD_PROPERTY( GameName, PROP_Editable );
	ADD_PROPERTY( Author, PROP_Editable );
	ADD_PROPERTY( bNoPause, PROP_Editable );
	ADD_PROPERTY( WindowType, PROP_Editable );
	ADD_PROPERTY( bQuitByEsc, PROP_Editable );
	ADD_PROPERTY( DefaultWidth, PROP_Editable );
	ADD_PROPERTY( DefaultHeight, PROP_Editable );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/