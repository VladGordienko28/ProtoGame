/*=============================================================================
    FrCmpBas.cpp: Basic components implementation.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FExtraComponent implementation.
-----------------------------------------------------------------------------*/

//
// FExtraComponent constructor.
//
FExtraComponent::FExtraComponent()
	:	Base( nullptr ),
		DrawOrder( -0.1f )
{
}


//
// Initialize for entity.
//
void FExtraComponent::InitForEntity( FEntity* InEntity )
{
	FComponent::InitForEntity(InEntity);

	assert(Entity->Base);
	Entity->Components.Push(this);
	Base = Entity->Base;
}


//
// Initialize for script.
//
void FExtraComponent::InitForScript( FScript* InScript )
{
	FComponent::InitForScript(InScript);

	assert(Script->Base);
	Script->Components.Push(this);
	Base = Script->Base;
}


//
// Return component's layer.
//
Float FExtraComponent::GetLayer() const
{
	return Base->Layer + DrawOrder;
}


//
// Restore after loading.
//
void FExtraComponent::PostLoad()
{
	FComponent::PostLoad();

	Base = Script ? Script->Base : Entity->Base;
	assert(Base);
}


/*-----------------------------------------------------------------------------
    FBaseComponent implementation.
-----------------------------------------------------------------------------*/

//
// Base component initialization.
//
FBaseComponent::FBaseComponent()
	:	bSelected( false ),
		bFrozen( false ),
		bDestroyed( false ),
		bFixedAngle( true ),
		bFixedSize( false ),
		bHashable( false ),
		Location( 0.f, 0.f ),
		Rotation( 0 ),
		Size( 1.f, 1.f ),
		Layer( 0.5f ),
		bHashed( false ),
		HashMark( -1 ),
		HashAABB( TVector( 0.f, 0.f ), 1.f )
{}


//
// Return component bounding rect.
//
TRect FBaseComponent::GetAABB()
{
	return TRect( Location, Size );
}


//
// Return component bounding rect.
//
Float FBaseComponent::GetLayer() const
{
	return Layer;
}


//
// When some property changed in component.
//
void FBaseComponent::EditChange()
{
	FComponent::EditChange();

	// Don't allow rotate if bFixedAngle.
	if( bFixedAngle && Rotation.Angle != 0 )
		Rotation = 0;
}


//
// Base component serialization.
//
void FBaseComponent::SerializeThis( CSerializer& S )
{
	FComponent::SerializeThis( S );

	Serialize( S, bFixedAngle );
	Serialize( S, bFixedSize );
	Serialize( S, bHashable );
	Serialize( S, Location );
	Serialize( S, Rotation );
	Serialize( S, Size );
	Serialize( S, Layer );

	// It's useless in many ways, but
	// vital for editor Undo/Redo.
	Serialize( S, bDestroyed );
}


//
// Base component entity initialization.
//
void FBaseComponent::InitForEntity( FEntity* InEntity )
{
	FComponent::InitForEntity( InEntity );
	assert(!Entity->Base);
	Entity->Base = this;
}


//
// Base component script initialization.
//
void FBaseComponent::InitForScript( FScript* InScript )
{
	FComponent::InitForScript( InScript );
	assert(!Script->Base);
	Script->Base = this;
}


//
// Init base component when level started.
//
void FBaseComponent::BeginPlay()
{
	// Add object to collision hash, if any.
	if( bHashable )
		Level->CollHash->AddToHash( this );
}


//
// Level has been finished.
//
void FBaseComponent::EndPlay()
{
	// Remove object from the collision hash, if any.
	if( bHashable )
		Level->CollHash->RemoveFromHash( this );
}


//
// Set a new object location.
//
void FBaseComponent::nativeSetLocation( CFrame& Frame )
{
	TVector NewLocation = POP_VECTOR;
		
	if( bHashable && bHashed )
	{
		// Deal with a hash.
		Level->CollHash->RemoveFromHash( this );
		{
			Location	= NewLocation;
		}
		Level->CollHash->AddToHash( this );
	}
	else
		Location	= NewLocation;
}


//
// Move object by delta.
//
void FBaseComponent::nativeMove( CFrame& Frame )
{
	TVector DeltaMove = POP_VECTOR;
		
	if( bHashable && bHashed )
	{
		// Deal with a hash.
		Level->CollHash->RemoveFromHash( this );
		{
			Location	+= DeltaMove;
		}
		Level->CollHash->AddToHash( this );
	}
	else
		Location	+= DeltaMove;
}


//
// Set a new object size.
//
void FBaseComponent::nativeSetSize( CFrame& Frame )
{
	TVector NewSize = POP_VECTOR;

	if( bHashable && bHashed )
	{
		// Deal with a hash.
		Level->CollHash->RemoveFromHash( this );
		{
			Size	= NewSize;
		}
		Level->CollHash->AddToHash( this );
	}
	else
		Size	= NewSize;
}


//
// Set a new object rotation.
//
void FBaseComponent::nativeSetRotation( CFrame& Frame )
{
	TAngle NewRotation = POP_ANGLE;

	if( bHashable && bHashed )
	{
		// Deal with a hash.
		Level->CollHash->RemoveFromHash( this );
		{
			Rotation	= NewRotation;
		}
		Level->CollHash->AddToHash( this );
	}
	else
		Rotation	= NewRotation;	
}


//
// Play an ambient sound.
//
void FBaseComponent::nativePlayAmbient( CFrame& Frame )
{
	FSound*	Sound	= As<FSound>(POP_RESOURCE);
	Float	Gain	= POP_FLOAT;
	Float	Pitch	= POP_FLOAT;
	Float	Radius	= POP_FLOAT;

	if( Sound )
		GApp->GAudio->PlayAmbient
		( 
			Sound,
			Location,
			Radius,
			Gain,
			Pitch,
			Entity
		);
}


//
// Stop an ambient sound.
//
void FBaseComponent::nativeStopAmbient( CFrame& Frame )
{
	GApp->GAudio->StopAmbient( Entity );
}


//
// Test base component class.
//
void FBaseComponent::nativeIsBasedOn( CFrame& Frame )
{
	String TestAlt	= POP_STRING;
	for( CClass* C = GetClass(); C; C=C->Super )
		if( C->GetAltName() == TestAlt )
		{
			*POPA_BOOL	= true;
			return;
		}
	*POPA_BOOL	= false;
}


//
// Return entity's bounding rect.
//
void FBaseComponent::nativeGetAABB( CFrame& Frame )
{
	*POPA_AABB	= GetAABB();
}


/*-----------------------------------------------------------------------------
    FComponent implementation.
-----------------------------------------------------------------------------*/

//
// FComponent constructor.
//
FComponent::FComponent()
	:	Script( nullptr ),
		Entity( nullptr ),
		Level( nullptr ),
		bTickable( false ),
		bRenderable( false )
{
}


//
// FComponent destructor.
//
FComponent::~FComponent()
{
	if( bTickable && Level )
	{
		Integer i = Level->TickObjects.FindItem(this);
		if( i != -1 )
			Level->TickObjects.Remove(i);
	}
	if( bRenderable && Level )
	{
		Integer i = Level->RenderObjects.FindItem(this);
		if( i != -1 )
			Level->RenderObjects.Remove(i);
	}
}


//
// Return component's layer.
//
Float FComponent::GetLayer() const
{
	return 0.f;
}


//
// Initialize component for using in entity.
//
void FComponent::InitForEntity( FEntity* InEntity )
{
	assert(InEntity);
	assert(InEntity->Level);

	Entity	= InEntity;
	Level	= InEntity->Level;
	Script	= nullptr;

	if( bTickable )
		Level->TickObjects.Push(this);

	if( bRenderable )
		Level->RenderObjects.Push(this);
}


//
// Initialize component for using in script.
//
void FComponent::InitForScript( FScript* InScript )
{
	assert(InScript);

	Entity	= nullptr;
	Level	= nullptr;
	Script	= InScript;
}


//
// Game just launched.
//
void FComponent::BeginPlay()
{
}


//
// Game has been finished.
//
void FComponent::EndPlay()
{
}


//
// Serialize component.
//
void FComponent::SerializeThis( CSerializer& S )
{
	FObject::SerializeThis( S );

	Serialize( S, Script );
	Serialize( S, Entity );
	Serialize( S, Level );
}


//
// When user changed some property via
// ObjectInspector.
//
void FComponent::EditChange()
{
	FObject::EditChange();
}


//
// Initialize component after loading.
//
void FComponent::PostLoad()
{
	FObject::PostLoad();
}


//
// Import component.
//
void FComponent::Import( CImporterBase& Im )
{
	FObject::Import( Im );
}


//
// Export component.
//
void FComponent::Export( CExporterBase& Ex )
{
	FObject::Export( Ex );
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FComponent, FObject, CLASS_Abstract )
{
	BEGIN_STRUCT(TCoords);
		STRUCT_MEMBER(Origin);
		STRUCT_MEMBER(XAxis);
		STRUCT_MEMBER(YAxis);
	END_STRUCT;
}


REGISTER_CLASS_CPP( FExtraComponent, FComponent, CLASS_Abstract )
{
}


REGISTER_CLASS_CPP( FBaseComponent, FComponent, CLASS_Abstract )
{
	ADD_PROPERTY( Location, PROP_Editable );
	ADD_PROPERTY( Rotation, PROP_Editable );
	ADD_PROPERTY( Size, PROP_Editable );
	ADD_PROPERTY( Layer, PROP_Editable );
	ADD_PROPERTY( bFixedAngle, PROP_None );
	ADD_PROPERTY( bFixedSize, PROP_None );
	ADD_PROPERTY( bHashable, PROP_None );
	ADD_PROPERTY( Level, PROP_Const | PROP_NoImEx );
	ADD_PROPERTY( bFrozen, PROP_None );

	DECLARE_METHOD( SetLocation, TYPE_None, ARG(newLocation, TYPE_Vector, END) );
	DECLARE_METHOD( SetSize, TYPE_None, ARG(newSize, TYPE_Vector, END) );
	DECLARE_METHOD( SetRotation, TYPE_None, ARG(newRotation, TYPE_Angle, END) );
	DECLARE_METHOD( Move, TYPE_None, ARG(deltaMove, TYPE_Vector, END) );
	DECLARE_METHOD( PlayAmbient, TYPE_None, ARG(sound, TYPE_SOUND, ARG(gain, TYPE_Float, ARG(pitch, TYPE_Float, ARG(radius, TYPE_Float, END)))) );
	DECLARE_METHOD( StopAmbient, TYPE_None, END );
	DECLARE_METHOD( IsBasedOn, TYPE_Bool, ARG(baseName, TYPE_String, END) );
	DECLARE_METHOD( GetAABB, TYPE_AABB, END );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/