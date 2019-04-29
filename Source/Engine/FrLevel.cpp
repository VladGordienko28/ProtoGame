/*=============================================================================
    FrLevel.cpp: FLevel implementation.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FLevel implementation.
-----------------------------------------------------------------------------*/

//
// Level's constructor.
//
FLevel::FLevel()
	:	Original( nullptr ),
		RndFlags( RND_Game ),
		Camera(),
		Sky( nullptr ),
		bIsPlaying( false ),
		bIsPause( false ),
		GameSpeed( 1.f ),
		Soundtrack( nullptr ),
		CollHash( nullptr ),
		GFXManager( nullptr ),
		AmbientLight( math::colors::BLACK ),
		BlurIntensity( 0.f )
{
	Effect[0] = Effect[1] = Effect[2] = 1.f;
	Effect[3] = Effect[4] = Effect[5] = 1.f;
	Effect[6] = Effect[7] = Effect[8] = 0.f;
	Effect[9] = 0.f;

	// Setup linked-list.
	FirstInput			= nullptr;
	FirstPortal			= nullptr;
	FirstPuppet			= nullptr;
	FirstLight			= nullptr;
	FirstLogicElement	= nullptr;
	FirstPainter		= nullptr;

	m_environmentContext.setDaySpeed( /*1200.f*/1.f );
	m_environmentContext.setTimeOfDay( 13, 0, 0.f ); // remove this!!

	// -----------------
	m_ambientColors.addSample( envi::TimeOfDay( 0, 0 ).toPercent(), math::Color( 60, 87, 144, 255 ) );
	m_ambientColors.addSample( envi::TimeOfDay( 8, 0 ).toPercent(), math::Color( 186, 152, 140, 255 ) );
	m_ambientColors.addSample( envi::TimeOfDay( 15, 0 ).toPercent(), math::Color( 159, 159, 190, 255 ) );
	m_ambientColors.addSample( envi::TimeOfDay( 21, 0 ).toPercent(), math::Color( 250, 100, 145, 255 ) );


	m_ambientColors.addSample( 1.f, math::Color( 60, 87, 144, 255 ) );
}

void FLevel::EditChange()
{
	FResource::EditChange();
/*
	static const Float scale = 1.5f;

	if( m_midnightBitmap )
	{
		m_ambientColors.Samples[0].Output = m_ambientColors.Samples[4].Output = m_midnightBitmap->getAverageColor() * scale;
	}

	if( m_dawnBitmap )
	{
		m_ambientColors.Samples[1].Output =  m_dawnBitmap->getAverageColor() * scale;
	}

	if( m_noonBitmap )
	{
		m_ambientColors.Samples[2].Output =  m_noonBitmap->getAverageColor() * scale;
	}

	if( m_duskBitmap )
	{
		m_ambientColors.Samples[3].Output =  m_duskBitmap->getAverageColor() * scale;
	}
	*/
}


//
// Level's destructor.
//
FLevel::~FLevel()
{
	// Test state.
	assert(CollHash == nullptr);
	assert(GFXManager == nullptr);

	// Destroy all my entities.
	for( Int32 i=0; i<Entities.size(); i++ )
		if( Entities[i] )
			DestroyObject( Entities[i], true );
}


//
// Level serialization.
//
void FLevel::SerializeThis( CSerializer& S )
{
	FResource::SerializeThis( S );

	Serialize( S, Original );
	Serialize( S, RndFlags );
	Serialize( S, Entities );
	Serialize( S, Camera );
	Serialize( S, Sky );
	Serialize( S, bIsPlaying );
	Serialize( S, bIsPause );
	Serialize( S, GameSpeed );
	Serialize( S, Soundtrack );
	Serialize( S, m_navigator );
	Serialize( S, AmbientLight );
	Serialize( S, BlurIntensity );
	S.SerializeData( Effect, sizeof(Effect) );

	Serialize( S, m_vignette.intensity );
	Serialize( S, m_vignette.innerRadius );
	Serialize( S, m_vignette.outerRadius );

	Serialize( S, m_enableFXAA );

	Serialize( S, m_dawnBitmap );
	Serialize( S, m_noonBitmap );
	Serialize( S, m_duskBitmap );
	Serialize( S, m_midnightBitmap );

	Serialize( S, m_environment );

	// Warning: Don't serialize level databases of
	// entities or components, because it
	// already serialized in the FComponent or
	// FEntity ::SerializeThis.
	
	// For some reasons it's failed.
	// Cleanup entities db to avoid null objects.
	//if( S.GetMode() == SM_Undefined )
	//	CLEANUP_ARR_NULL(Entities);
}


//
// Restore level after game loading.
//
void FLevel::PostLoad()
{
	FResource::PostLoad();
}


//
// Export the level.
//
void FLevel::Export( CExporterBase& Ex )
{
	FResource::Export( Ex );
	EXPORT_INTEGER(RndFlags);
	EXPORT_OBJECT(Sky);

	// All entity databases will be
	// exported in exporter.
}


//
// Import the level.
//
void FLevel::Import( CImporterBase& Im )
{
	FResource::Import( Im );
	IMPORT_INTEGER(RndFlags);
	IMPORT_OBJECT(Sky);

	// All entity databases will be
	// imported in importer.
}


/*-----------------------------------------------------------------------------
    Gameplay!
-----------------------------------------------------------------------------*/

//
// Begin play level.
//
void FLevel::BeginPlay()
{
	// Allocate collision hash.
	CollHash	= new CCollisionHash( this );

	// Level's GFX.
	GFXManager	= new CGFXManager( this );

	// Notify all entities and their components.
	for( Int32 i=0; i<Entities.size(); i++ )
		Entities[i]->BeginPlay();

	// Mark level as played.
	bIsPlaying		= true;

	// Play music!
	if( Soundtrack )
		GApp->GAudio->PlayMusic( Soundtrack, 2.f );

	// Notify all entities because everything are initialized
	// for playing.
	for( Int32 i=0; i<Entities.size(); i++ )
		if( Entities[i]->Script->IsScriptable() )
			Entities[i]->OnBeginPlay();

	// tmp
	m_environmentContext.setTimeOfDay( Random(24), 0, 0.f );
}


//
// Stop level playing.
//
void FLevel::EndPlay()
{
	// Mark level as not played.
	bIsPlaying		= false;

	// Call in script OnEndPlay, while all objects are
	// valid.
	for( Int32 i=0; i<Entities.size(); i++ )
		if( Entities[i]->Script->IsScriptable() )
			Entities[i]->OnEndPlay();

	// Notify all entities and their components.
	for( Int32 i=0; i<Entities.size(); i++ )
		Entities[i]->EndPlay();

	// Release the collision hash.
	assert(CollHash);
	delete CollHash;
	CollHash	= nullptr;

	// Release GFX man.
	assert(GFXManager);
	delete GFXManager;
	GFXManager	= nullptr;

	// Flush all level's ambients.
	GApp->GAudio->FlushAmbients();

	// Stop music.
	if( Soundtrack )
		GApp->GAudio->PlayMusic( nullptr, 2.f );
}


//
// Incoming level global variable.
//
TIncomingLevel			GIncomingLevel;
Map<String, String>	GStaticBuffer;


/*-----------------------------------------------------------------------------
    Level collisions.
-----------------------------------------------------------------------------*/

//
// Figure out is point inside brush. If point is
// outside of any brush return nullptr.
//
FBrushComponent* FLevel::TestPointGeom( const math::Vector& P )
{
	assert(bIsPlaying && CollHash);

	// Get list of brushes.
	FBrushComponent* Brushes[MAX_COLL_LIST_OBJS];
	Int32 NumBrshs;
	CollHash->GetOverlappedByClass
								( 
									math::Rect( P, 0.1f ),
									FBrushComponent::MetaClass,
									NumBrshs,
									(FBaseComponent**)Brushes
								);

	for( Int32 iBrush=0; iBrush<NumBrshs; iBrush++ )
	{
		FBrushComponent* Brush = Brushes[iBrush];

		if( Brush->Type == BRUSH_Solid )
		{
			// Transform test point to the Brush's local coords.
			math::Vector LP = P - Brush->Location;

			if( math::isPointInsidePoly( LP, Brush->Vertices, Brush->NumVerts ) )
				return Brush;
		}
	}

	// Not found.
	return nullptr;
}


//
// Test a line with a level geometry.
//
FBrushComponent* FLevel::TestLineGeom( const math::Vector& A, const math::Vector& B, Bool bFast, math::Vector* Hit, math::Vector* Normal )
{
	assert(bIsPlaying && CollHash);

	Float BestTime			= 100000.0f;
	FBrushComponent* Result	= nullptr;

	// Get line bounds.
	math::Rect Bounds;
	Bounds.min.x	= min( A.x, B.x );
	Bounds.min.y	= min( A.y, B.y );
	Bounds.max.x	= max( A.x, B.x );
	Bounds.max.y	= max( A.y, B.y );

	// Get list of brushes.
	FBrushComponent* Brushes[MAX_COLL_LIST_OBJS];
	Int32 NumBrshs;
	CollHash->GetOverlappedByClass
								( 
									Bounds,
									FBrushComponent::MetaClass,
									NumBrshs,
									(FBaseComponent**)Brushes
								);

	for( Int32 iBrush=0; iBrush<NumBrshs; iBrush++ )
	{
		FBrushComponent* Brush = Brushes[iBrush];

		if( Brush->Type != BRUSH_NotSolid )
		{
			// Test collision with solid/semi-solid brush.
			Float TestTime;
			math::Vector TestHit, TestNormal;
			math::Vector LA = A - Brush->Location;
			math::Vector LB = B - Brush->Location;

			if( math::isLineIntersectPoly( LA, LB, Brush->Vertices, Brush->NumVerts, &TestHit, &TestNormal ) )
			{
				if( Brush->Type==BRUSH_Solid || (Brush->Type==BRUSH_SemiSolid && phys::isWalkableSurface(TestNormal)) )
				{
					TestTime	= ( TestHit + Brush->Location - A ).sizeSquared();

					if( TestTime < BestTime )
					{
						// Least time.
						if( Hit )		*Hit	= TestHit + Brush->Location;
						if( Normal )	*Normal	= TestNormal;
						BestTime	= TestTime;
						Result		= Brush;

						if( bFast ) 
							return Brush;
					}
				}
			}
		}
	}

	return Result;
}


/*-----------------------------------------------------------------------------
    Tick.
-----------------------------------------------------------------------------*/

//
// Update level.
//
void FLevel::Tick( Float Delta )
{
	// Clamp delta.
	Delta	= clamp( Delta, 1.f/500.f, 1.f/30.f );

	// Modify delta according to game speed.
	//GameSpeed	= clamp( GameSpeed, 0.01f, 10.f );
	Delta		*= GameSpeed;

	// update game time
	m_environmentContext.tick( Delta );

	// Are we play now?
	if( bIsPlaying && !bIsPause )
	{
		// Normally play level.
		{
			profile_zone( EProfilerGroup::Entity, ExecuteThread );
			for( Int32 i=0; i<Entities.size(); i++ )
				if( Entities[i]->Thread )
					Entities[i]->Thread->Tick( Delta );
		}

		{
			profile_zone( EProfilerGroup::Entity, PreTick );
			for( Int32 i=0; i<TickObjects.size(); i++ )
				TickObjects[i]->PreTick( Delta );
		}

		{
			profile_zone( EProfilerGroup::Entity, Tick );
			for( Int32 i=0; i<TickObjects.size(); i++ )
				TickObjects[i]->Tick( Delta );
		}

		// Update GFX interpolation.
		GFXManager->Tick( Delta );
	}
	else
	{
		// We not play, just edit level or in pause.
		profile_zone( EProfilerGroup::Entity, TickNonPlay );
		for( Int32 i=0; i<TickObjects.size(); i++ )
			TickObjects[i]->TickNonPlay( Delta );
	}

	// Destroy all marked entities.
	{
		profile_zone( EProfilerGroup::Entity, Cleanup );
		for( Int32 iEntity=0; iEntity<Entities.size(); )
			if( Entities[iEntity]->Base->bDestroyed )
			{
				ReleaseEntity( iEntity );
			}
			else
				iEntity++;
	}

	// Update debug stuff.
	CDebugDrawHelper::Instance().Tick(Delta);
}


/*-----------------------------------------------------------------------------
    Level entity functions.
-----------------------------------------------------------------------------*/

//
// Create a new entity. Use only it, because it
// create, register and prepare entity for playing.
//
FEntity* FLevel::CreateEntity( FScript* InScript, String InName, math::Vector InLocation )
{
	assert(InScript);

	// Select name.
	String EntityName;

	if( InName )
	{
		// Name specified. 
		EntityName	= InName;
	}
	else
	{
		// Generate new unique name.
		for( Int32 iUniq=0; ; iUniq++ )
		{
			String TestName = String::format( L"%s%d", *InScript->GetName(), iUniq );
			if( !GObjectDatabase->FindObject( TestName, FEntity::MetaClass, this ) )
			{
				EntityName = TestName;
				break;
			}
		}
	}

	// Allocate new entity.
	FEntity* Entity = NewObject<FEntity>( EntityName, this );
	Entities.push( Entity );
	Entity->Init( InScript, this );

	// Initialize fields.
	Entity->Base->Location	= InLocation;
	Entity->Base->Layer		+= RandomRange( -0.01f, +0.01f );

	// Prepare for playing.
	if( bIsPlaying )
	{
		Entity->BeginPlay();
	}

	// Return it!
	return Entity;
}


//
// Destroy the entity. Actually just mark,
// entity will destroyed after entire level tick.
//
void FLevel::DestroyEntity( FEntity* Entity )
{
	assert(Entity);
	assert(Entity->Level==this);

	Entity->Base->bDestroyed = true;
}


//
// Totally release entity, don't invoke it in
// FLevel::Tick, call it after level tick, it's
// cleanup all references also.
//
void FLevel::ReleaseEntity( Int32 iEntity )
{
	FEntity* Entity = Entities[iEntity];
	assert(Entity && Entity->Base->bDestroyed);

	// Notify about end of play, if play.
	if( bIsPlaying )
	{
		Entity->OnDestroy();
		Entity->EndPlay();
	}

	// Let's CObjectDatabase handle it.
	Entities.removeFast(iEntity);
	DestroyObject( Entity, true );
}


/*-----------------------------------------------------------------------------
    Level editor functions.
-----------------------------------------------------------------------------*/

//
// Find entity by name. Too slow, but used
// in editor, so it's well.
//
FEntity* FLevel::FindEntity( String InName )
{
	String UpperName = String::upperCase( InName );

	for( Int32 iEnt=0; iEnt<Entities.size(); iEnt++ )
		if( UpperName == String::upperCase( Entities[iEnt]->GetName() ) )
			return Entities[iEnt];

	// Not found.
	return nullptr; 
}


//
// Return index of the given entity in the level's
// database, if entity not found return -1.
//
Int32 FLevel::GetEntityIndex( FEntity* Entity )
{ 
	for( Int32 i=0; i<Entities.size(); i++ )
		if( Entities[i] == Entity )
			return i;

	return -1;
}


/*-----------------------------------------------------------------------------
    FEntity implementation.
-----------------------------------------------------------------------------*/

//
// Entity constructor.
//
FEntity::FEntity()
	:	Level( nullptr ),
		Script( nullptr ),
		Thread( nullptr ),
		InstanceBuffer(nullptr),
		Base( nullptr ),
		Components()
{
}


//
// Entity destructor.
//
FEntity::~FEntity()
{
	assert(Thread == nullptr);

	for( Int32 i=0; i<Components.size(); i++ )
		DestroyObject( Components[i], true );

	DestroyObject( Base, true );

	// Release instance buffer.
	if( InstanceBuffer )
		delete InstanceBuffer;
}


//
// Entity initialization.
//
void FEntity::Init( FScript* InScript, FLevel* InLevel )	
{
	// Initialize fields.
	Script		= InScript;
	Level		= InLevel;

	// Base component.
	FBaseComponent* BasCom = (FBaseComponent*)GObjectDatabase->CopyObject
																	( 
																		Script->Base, 
																		Script->Base->GetName(), 
																		this 
																	);
	BasCom->InitForEntity( this );

	// Extra components.
	for( Int32 i=0; i<Script->Components.size(); i++ )
	{
		FExtraComponent* Source = Script->Components[i];
		FExtraComponent* Com = (FExtraComponent*)GObjectDatabase->CopyObject
																		( 
																			Source, 
																			Source->GetName(), 
																			this 
																		);
		Com->InitForEntity( this );
	}

	// Initialize instance buffer.
	if( Script->InstanceBuffer )
	{
		InstanceBuffer = new CInstanceBuffer( Script->Properties );
		InstanceBuffer->Data.setSize( Script->InstanceSize );

		if( Script->Properties.size() )
			InstanceBuffer->CopyValues( &Script->InstanceBuffer->Data[0] );
	}
}


//
// Entity serialization.
//
void FEntity::SerializeThis( CSerializer& S )
{
	FObject::SerializeThis( S );

	Serialize( S, Level );
	Serialize( S, Script );	

	Serialize( S, Base );
	Serialize( S, Components );

	// Instance buffer.
	if( S.GetMode() == SM_Load )
	{
		freeandnil(InstanceBuffer);
		InstanceBuffer	= new CInstanceBuffer( Script->Properties );
		InstanceBuffer->Data.setSize( Script->InstanceSize );
		InstanceBuffer->SerializeValues( S );	
	}
	else
	{
		if( InstanceBuffer )
			InstanceBuffer->SerializeValues( S );	
	}

	// Thread shouldn't be serialized since it's in-game
	// only used, and initialized after FLevel::BeginPlay.
}


//
// Entity after loading initialization.
//
void FEntity::PostLoad()
{
	FObject::PostLoad();
}


//
// Entity import.
//
void FEntity::Import( CImporterBase& Im )
{
	FObject::Import( Im );
}


//
// Entity export.
//
void FEntity::Export( CExporterBase& Ex )
{
	FObject::Export( Ex );
}


//
// Notify entity about game start.
// This event prepare entity for playing!
//
void FEntity::BeginPlay()
{
	if( Script->IsScriptable() )
	{
		// Allocate an entity thread if any.
		if( Script->Thread )
			Thread	= new CThreadFrame( this, Script->Thread );
	}

	// Notify all components.
	Base->BeginPlay();
	for( Int32 e=0; e<Components.size(); e++ )
		Components[e]->BeginPlay();
}


//
// Notify entity about the end of game.
//
void FEntity::EndPlay()
{
	// Notify all components.
	Base->EndPlay();
	for( Int32 e=0; e<Components.size(); e++ )
		Components[e]->EndPlay();

	// Destroy thread if any.
	if( Thread )
	{
		delete Thread;
		Thread	= nullptr;
	}
}


/*-----------------------------------------------------------------------------
	TCamera implementation.
-----------------------------------------------------------------------------*/

//
// Camera constructor.
//
TCamera::TCamera()
	:	Location( 0.f, 0.f ),
		Rotation( 0 ),
		FOV( 64.f, 32.f ),
		Zoom( 1.f ),
		ScrollBound( math::Vector(0.f, 0.f), math::WORLD_SIZE )
{
}


//
// Return a "good" camera FOV according to viewport
// resolution.
//
math::Vector TCamera::GetFitFOV( Float ScreenX, Float ScreenY ) const
{
	math::Vector R;
	R.x = FOV.x;
	R.y = ScreenY * abs(FOV.x) / ScreenX;
	return R;
}


//
// Camera serialization.
//
void Serialize( CSerializer& S, TCamera& V )
{
	Serialize( S, V.Location );
	Serialize( S, V.Rotation );
	Serialize( S, V.FOV );
	Serialize( S, V.Zoom );
	Serialize( S, V.ScrollBound );
}


/*-----------------------------------------------------------------------------
	Extended Engine function.
-----------------------------------------------------------------------------*/

//
// Draw debug line.
//
void fluDebugLine( CFrame& Frame )
{
	math::Vector A = POP_VECTOR;
	math::Vector B = POP_VECTOR;
	math::Color Color = POP_COLOR;
	Float Time = POP_FLOAT;

	CDebugDrawHelper::Instance().DrawLine( A, B, Color, Time );
}


//
// Draw debug point.
//
void fluDebugPoint( CFrame& Frame )
{
	math::Vector P = POP_VECTOR;
	math::Color Color = POP_COLOR;
	Float Size = POP_FLOAT;
	Float Time = POP_FLOAT;

	CDebugDrawHelper::Instance().DrawPoint( P, Color, Size, Time );
}


//
// Draw debug rect.
//
void fluDebugRect( CFrame& Frame )
{
	math::Rect R = POP_AABB;
	math::Color Color = POP_COLOR;
	Float Time = POP_FLOAT;

	CDebugDrawHelper::Instance().DrawLine( { R.min.x, R.min.y }, { R.min.x, R.max.y }, Color, Time );
	CDebugDrawHelper::Instance().DrawLine( { R.min.x, R.max.y }, { R.max.x, R.max.y }, Color, Time );
	CDebugDrawHelper::Instance().DrawLine( { R.max.x, R.max.y }, { R.max.x, R.min.y }, Color, Time );
	CDebugDrawHelper::Instance().DrawLine( { R.max.x, R.min.y }, { R.min.x, R.min.y }, Color, Time );
}

/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FEntity, FObject, CLASS_Sterile )
{
	BEGIN_ENUM(EInputKey);
		ENUM_ELEM(KEY_None);			ENUM_ELEM(KEY_LButton);		
		ENUM_ELEM(KEY_RButton);			ENUM_ELEM(KEY_DblClick);		
		ENUM_ELEM(KEY_MButton);			ENUM_ELEM(KEY_WheelUp);		
		ENUM_ELEM(KEY_WheelDown);		ENUM_ELEM(KEY_AV0x07);		
		ENUM_ELEM(KEY_Backspace);		ENUM_ELEM(KEY_Tab);		
		ENUM_ELEM(KEY_AV0x0a);			ENUM_ELEM(KEY_AV0x0b);		
		ENUM_ELEM(KEY_AV0x0c);			ENUM_ELEM(KEY_Return);		
		ENUM_ELEM(KEY_AV0x0e);			ENUM_ELEM(KEY_AV0x0f);		
		ENUM_ELEM(KEY_Shift);			ENUM_ELEM(KEY_Ctrl);		
		ENUM_ELEM(KEY_Alt);				ENUM_ELEM(KEY_Pause);		
		ENUM_ELEM(KEY_CapsLock);		ENUM_ELEM(KEY_AV0x15);		
		ENUM_ELEM(KEY_AV0x16);			ENUM_ELEM(KEY_AV0x17);	
		ENUM_ELEM(KEY_AV0x18);			ENUM_ELEM(KEY_AV0x19);		
		ENUM_ELEM(KEY_AV0x1a);			ENUM_ELEM(KEY_Escape);		
		ENUM_ELEM(KEY_AV0x1c);			ENUM_ELEM(KEY_AV0x1d);		
		ENUM_ELEM(KEY_AV0x1e);			ENUM_ELEM(KEY_AV0x1f);	
		ENUM_ELEM(KEY_Space);			ENUM_ELEM(KEY_PageUp);		
		ENUM_ELEM(KEY_PageDown);		ENUM_ELEM(KEY_End);		
		ENUM_ELEM(KEY_Home);			ENUM_ELEM(KEY_Left);		
		ENUM_ELEM(KEY_Up);				ENUM_ELEM(KEY_Right);	
		ENUM_ELEM(KEY_Down);			ENUM_ELEM(KEY_Select);		
		ENUM_ELEM(KEY_Print);			ENUM_ELEM(KEY_Execute);		
		ENUM_ELEM(KEY_PrintScrn);		ENUM_ELEM(KEY_Insert);		
		ENUM_ELEM(KEY_Delete);			ENUM_ELEM(KEY_Help);	
		ENUM_ELEM(KEY_0);				ENUM_ELEM(KEY_1);		
		ENUM_ELEM(KEY_2);				ENUM_ELEM(KEY_3);		
		ENUM_ELEM(KEY_4);				ENUM_ELEM(KEY_5);		
		ENUM_ELEM(KEY_6);				ENUM_ELEM(KEY_7);	
		ENUM_ELEM(KEY_8);				ENUM_ELEM(KEY_9);		
		ENUM_ELEM(KEY_AV0x3a);			ENUM_ELEM(KEY_AV0x3b);		
		ENUM_ELEM(KEY_AV0x3c);			ENUM_ELEM(KEY_AV0x3d);		
		ENUM_ELEM(KEY_AV0x3e);			ENUM_ELEM(KEY_AV0x3f);	
		ENUM_ELEM(KEY_AV0x40);			ENUM_ELEM(KEY_A);		
		ENUM_ELEM(KEY_B);				ENUM_ELEM(KEY_C);		
		ENUM_ELEM(KEY_D);				ENUM_ELEM(KEY_E);		
		ENUM_ELEM(KEY_F);				ENUM_ELEM(KEY_G);	
		ENUM_ELEM(KEY_H);				ENUM_ELEM(KEY_I);		
		ENUM_ELEM(KEY_J);				ENUM_ELEM(KEY_K);		
		ENUM_ELEM(KEY_L);				ENUM_ELEM(KEY_M);		
		ENUM_ELEM(KEY_N);				ENUM_ELEM(KEY_O);	
		ENUM_ELEM(KEY_P);				ENUM_ELEM(KEY_Q);		
		ENUM_ELEM(KEY_R);				ENUM_ELEM(KEY_S);		
		ENUM_ELEM(KEY_T);				ENUM_ELEM(KEY_U);		
		ENUM_ELEM(KEY_V);				ENUM_ELEM(KEY_W);	
		ENUM_ELEM(KEY_X);				ENUM_ELEM(KEY_Y);		
		ENUM_ELEM(KEY_Z);				ENUM_ELEM(KEY_AV0x5b);		
		ENUM_ELEM(KEY_AV0x5c);			ENUM_ELEM(KEY_AV0x5d);		
		ENUM_ELEM(KEY_AV0x5e);			ENUM_ELEM(KEY_AV0x5f);	
		ENUM_ELEM(KEY_NumPad0);			ENUM_ELEM(KEY_NumPad1);		
		ENUM_ELEM(KEY_NumPad2);			ENUM_ELEM(KEY_NumPad3);		
		ENUM_ELEM(KEY_NumPad4);			ENUM_ELEM(KEY_NumPad5);		
		ENUM_ELEM(KEY_NumPad6);			ENUM_ELEM(KEY_NumPad7);	
		ENUM_ELEM(KEY_NumPad8);			ENUM_ELEM(KEY_NumPad9);		
		ENUM_ELEM(KEY_Multiply);		ENUM_ELEM(KEY_Add);		
		ENUM_ELEM(KEY_Separator);		ENUM_ELEM(KEY_Subtract);		
		ENUM_ELEM(KEY_Decimal);			ENUM_ELEM(KEY_Divide);	
		ENUM_ELEM(KEY_F1);				ENUM_ELEM(KEY_F2);		
		ENUM_ELEM(KEY_F3);				ENUM_ELEM(KEY_F4);		
		ENUM_ELEM(KEY_F5);				ENUM_ELEM(KEY_F6);		
		ENUM_ELEM(KEY_F7);				ENUM_ELEM(KEY_F8);	
		ENUM_ELEM(KEY_F9);				ENUM_ELEM(KEY_F10);		
		ENUM_ELEM(KEY_F11);				ENUM_ELEM(KEY_F12);		
		ENUM_ELEM(KEY_F13);				ENUM_ELEM(KEY_F14);		
		ENUM_ELEM(KEY_F15);				ENUM_ELEM(KEY_F16);	
		ENUM_ELEM(KEY_F17);				ENUM_ELEM(KEY_F18);		
		ENUM_ELEM(KEY_F19);				ENUM_ELEM(KEY_F20);		
		ENUM_ELEM(KEY_F21);				ENUM_ELEM(KEY_F22);		
		ENUM_ELEM(KEY_F23);				ENUM_ELEM(KEY_F24);	
		ENUM_ELEM(KEY_AV0x88);			ENUM_ELEM(KEY_AV0x89);		
		ENUM_ELEM(KEY_AV0x8a);			ENUM_ELEM(KEY_AV0x8b);		
		ENUM_ELEM(KEY_AV0x8c);			ENUM_ELEM(KEY_AV0x8d);		
		ENUM_ELEM(KEY_AV0x8e);			ENUM_ELEM(KEY_AV0x8f);	
		ENUM_ELEM(KEY_NumLock);			ENUM_ELEM(KEY_ScrollLock);		
		ENUM_ELEM(KEY_AV0x92);			ENUM_ELEM(KEY_AV0x93);		
		ENUM_ELEM(KEY_AV0x94);			ENUM_ELEM(KEY_AV0x95);		
		ENUM_ELEM(KEY_AV0x96);			ENUM_ELEM(KEY_AV0x97);	
		ENUM_ELEM(KEY_AV0x98);			ENUM_ELEM(KEY_AV0x99);		
		ENUM_ELEM(KEY_AV0x9a);			ENUM_ELEM(KEY_AV0x9b);		
		ENUM_ELEM(KEY_AV0x9c);			ENUM_ELEM(KEY_AV0x9d);		
		ENUM_ELEM(KEY_AV0x9e);			ENUM_ELEM(KEY_AV0x9);	
		ENUM_ELEM(KEY_LShift);			ENUM_ELEM(KEY_RShift);		
		ENUM_ELEM(KEY_LControl);		ENUM_ELEM(KEY_RControl);		
		ENUM_ELEM(KEY_JoyUp);			ENUM_ELEM(KEY_JoyDown);		
		ENUM_ELEM(KEY_JoyLeft);			ENUM_ELEM(KEY_JoyRight);	
		ENUM_ELEM(KEY_JoySelect);		ENUM_ELEM(KEY_JoyStart);		
		ENUM_ELEM(KEY_JoyA);			ENUM_ELEM(KEY_JoyB);		
		ENUM_ELEM(KEY_JoyC);			ENUM_ELEM(KEY_JoyX);		
		ENUM_ELEM(KEY_JoyY);			ENUM_ELEM(KEY_JoyZ);	
		ENUM_ELEM(KEY_AV0xb0);			ENUM_ELEM(KEY_AV0xb1);		
		ENUM_ELEM(KEY_AV0xb2);			ENUM_ELEM(KEY_AV0xb3);		
		ENUM_ELEM(KEY_AV0xb4);			ENUM_ELEM(KEY_AV0xb5);		
		ENUM_ELEM(KEY_AV0xb6);			ENUM_ELEM(KEY_AV0xb7);	
		ENUM_ELEM(KEY_AV0xb8);			ENUM_ELEM(KEY_AV0xb9);		
		ENUM_ELEM(KEY_Semicolon);		ENUM_ELEM(KEY_Equals);		
		ENUM_ELEM(KEY_Comma);			ENUM_ELEM(KEY_Minus);		
		ENUM_ELEM(KEY_Period);			ENUM_ELEM(KEY_Slash);	
		ENUM_ELEM(KEY_Tilde);			ENUM_ELEM(KEY_AV0xc1);		
		ENUM_ELEM(KEY_AV0xc2);			ENUM_ELEM(KEY_AV0xc3);		
		ENUM_ELEM(KEY_AV0xc4);			ENUM_ELEM(KEY_AV0xc5);		
		ENUM_ELEM(KEY_AV0xc6);			ENUM_ELEM(KEY_AV0xc7);	
		ENUM_ELEM(KEY_AV0xc8);			ENUM_ELEM(KEY_AV0xc9);		
		ENUM_ELEM(KEY_AV0xca);			ENUM_ELEM(KEY_AV0xcb);		
		ENUM_ELEM(KEY_AV0xcc);			ENUM_ELEM(KEY_AV0xcd);		
		ENUM_ELEM(KEY_AV0xce);			ENUM_ELEM(KEY_AV0xcf);	
		ENUM_ELEM(KEY_AV0xd0);			ENUM_ELEM(KEY_AV0xd1);		
		ENUM_ELEM(KEY_AV0xd2);			ENUM_ELEM(KEY_AV0xd3);		
		ENUM_ELEM(KEY_AV0xd4);			ENUM_ELEM(KEY_AV0xd5);		
		ENUM_ELEM(KEY_AV0xd6);			ENUM_ELEM(KEY_AV0xd7);	
		ENUM_ELEM(KEY_AV0xd8);			ENUM_ELEM(KEY_AV0xd9);		
		ENUM_ELEM(KEY_AV0xda);			ENUM_ELEM(KEY_LeftBracket);		
		ENUM_ELEM(KEY_Backslash);		ENUM_ELEM(KEY_RightBracket);		
		ENUM_ELEM(KEY_SingleQuote);		ENUM_ELEM(KEY_AV0xdf);	
		ENUM_ELEM(KEY_AV0xe0);			ENUM_ELEM(KEY_AV0xe1);		
		ENUM_ELEM(KEY_AV0xe2);			ENUM_ELEM(KEY_AV0xe3);		
		ENUM_ELEM(KEY_AV0xe4);			ENUM_ELEM(KEY_AV0xe5);		
		ENUM_ELEM(KEY_AV0xe6);			ENUM_ELEM(KEY_AV0xe7);	
		ENUM_ELEM(KEY_AV0xe8);			ENUM_ELEM(KEY_AV0xe9);		
		ENUM_ELEM(KEY_AV0xea);			ENUM_ELEM(KEY_AV0xeb);		
		ENUM_ELEM(KEY_AV0xec);			ENUM_ELEM(KEY_AV0xed);		
		ENUM_ELEM(KEY_AV0xee);			ENUM_ELEM(KEY_AV0xef);	
		ENUM_ELEM(KEY_AV0xf0);			ENUM_ELEM(KEY_AV0xf1);		
		ENUM_ELEM(KEY_AV0xf2);			ENUM_ELEM(KEY_AV0xf3);		
		ENUM_ELEM(KEY_AV0xf4);			ENUM_ELEM(KEY_AV0xf5);		
		ENUM_ELEM(KEY_Attn);			ENUM_ELEM(KEY_CrSel);	
		ENUM_ELEM(KEY_ExSel);			ENUM_ELEM(KEY_ErEof);		
		ENUM_ELEM(KEY_Play);			ENUM_ELEM(KEY_Zoom);		
		ENUM_ELEM(KEY_NoName);			ENUM_ELEM(KEY_PA1);		
		ENUM_ELEM(KEY_OEMClear);		
	END_ENUM;
}


REGISTER_CLASS_CPP( FLevel, FResource, CLASS_Sterile )
{
	BEGIN_STRUCT(TCamera);
		STRUCT_MEMBER(Location);
		STRUCT_MEMBER(Rotation);
		STRUCT_MEMBER(FOV);
		STRUCT_MEMBER(Zoom);
		STRUCT_MEMBER(ScrollBound);
	END_STRUCT;

	using namespace fx;
	BEGIN_STRUCT(Vignette)
		STRUCT_MEMBER( intensity );
		STRUCT_MEMBER( innerRadius );
		STRUCT_MEMBER( outerRadius );
	END_STRUCT;

	using namespace envi;
	BEGIN_STRUCT( Satellite );
		STRUCT_MEMBER( m_bitmap );
		STRUCT_MEMBER( m_size );
		//STRUCT_MEMBER( m_zenithTime );
		STRUCT_MEMBER( m_orbitCenter );
		STRUCT_MEMBER( m_orbitWidth );
		STRUCT_MEMBER( m_orbitHeight );
	END_STRUCT;

	BEGIN_STRUCT( Environment );
		STRUCT_MEMBER( m_sun );
		STRUCT_MEMBER( m_moon );
	END_STRUCT;

	ADD_PROPERTY( bIsPlaying, PROP_Const );
	ADD_PROPERTY( bIsPause, PROP_Editable );
	ADD_PROPERTY( Original, PROP_Const|PROP_Editable|PROP_NoImEx );
	ADD_PROPERTY( GameSpeed, PROP_Editable );
	ADD_PROPERTY( Soundtrack, PROP_Editable );
	ADD_PROPERTY( Camera, PROP_Editable );
	ADD_PROPERTY( Effect, PROP_Editable );
	ADD_PROPERTY( AmbientLight, PROP_Editable );
	ADD_PROPERTY( BlurIntensity, PROP_Editable );

	ADD_PROPERTY( AberrationIntensity, PROP_Editable );
	ADD_PROPERTY( m_midnightBitmap, PROP_Editable );
	ADD_PROPERTY( m_dawnBitmap, PROP_Editable );
	ADD_PROPERTY( m_noonBitmap, PROP_Editable );
	ADD_PROPERTY( m_duskBitmap, PROP_Editable );

	ADD_PROPERTY( m_vignette, PROP_Editable );
	ADD_PROPERTY( m_enableFXAA, PROP_Editable );

	ADD_PROPERTY( m_environment, PROP_Editable );

	// Engine functions.
	DECLARE_EX_FUNCTION( DebugLine, TYPE_None, ARG(a, TYPE_Vector, ARG(b, TYPE_Vector, ARG(color, TYPE_Color, ARG(time, TYPE_Float, END)))));
	DECLARE_EX_FUNCTION( DebugPoint, TYPE_None, ARG(a, TYPE_Vector, ARG(color, TYPE_Color, ARG(size, TYPE_Float, ARG(time, TYPE_Float, END)))));
	DECLARE_EX_FUNCTION( DebugRect, TYPE_None, ARG(rect, TYPE_AABB, ARG(color, TYPE_Color, ARG(time, TYPE_Float, END))) );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/