/*=============================================================================
    FrAI.cpp: AI implementation.
    Copyright Oct.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FPuppetComponent implementation.
-----------------------------------------------------------------------------*/

//
// Puppet constructor.
//
FPuppetComponent::FPuppetComponent()
	:	FExtraComponent(),
		NextPuppet( nullptr )
{
	// This variables are vary for all puppets
	// in scene, but here I set some initial 
	// values, that the most often used.
	Health = 100;
	Team = -1;
	MoveSpeed = 5.f; // cells per sec.
	JumpHeight = 8.f; // cells
	GravityScale = 10.f; // cells per sec ^ 2
	SightDirection = ESightDirection::SIGHT_None;
	SightPeriod = 0.f;
	SightRadius = 16.f;

	m_targetPoint = { 0.f, 0.f };
	m_targetEntity = nullptr;
	m_targetRadius = 0.f;
	m_moveStatus = EMoveStatus::MOVE_Unknown;
	m_moveType = navi::EPathType::None;
	m_sightTimer = 0.f;

	bTickable	= true;
}


//
// Puppet destructor.
//
FPuppetComponent::~FPuppetComponent()
{
	com_remove( Puppet );
}


//
// Initialize puppet for entity.
//
void FPuppetComponent::InitForEntity( FEntity* InEntity )
{
	FExtraComponent::InitForEntity(InEntity);

	com_add( Puppet );
}


//
// Serialize the puppet.
//
void FPuppetComponent::SerializeThis( CSerializer& S )
{
	FExtraComponent::SerializeThis( S );

	// Serialize only savable/loadable variables.
	Serialize( S, Health );
	Serialize( S, Team );
	Serialize( S, MoveSpeed );
	Serialize( S, JumpHeight );
	Serialize( S, GravityScale );
	SerializeEnum( S, SightDirection );
	Serialize( S, SightPeriod );
	Serialize( S, SightRadius );

	// Also serialize list of Watched for GC.
	if( S.GetMode() == SM_Undefined )
	{
		for( auto& other : m_puppetsInSight )
		{
			Serialize( S, other );
		}
	}
}


//
// Prepare 'warriors' for the battle!
//
void FPuppetComponent::BeginPlay()
{
	FExtraComponent::BeginPlay();

	// Slightly modify a look time counter, to avoid all puppets 
	// watching at the same time.
	m_sightTimer = SightPeriod * RandomF();

	// Get arcade body.
	if( !Base->IsA( FArcadeBodyComponent::MetaClass ) )
	{
		// Suicide, if no arcade base.
		error( L"Puppet '%s' has no arcade body base!", *Entity->GetFullName() );
		Level->DestroyEntity( Entity );
	}
}


/*-----------------------------------------------------------------------------
	Puppet AI functions.
-----------------------------------------------------------------------------*/

//
// AI tick.
//
void FPuppetComponent::Tick( Float Delta )
{
	// Do puppet wants to stare others?
	if( SightDirection != SIGHT_None && 
		SightRadius > 0.f && SightPeriod > 0.f )
	{
		processSight( Delta );
	}

	// Do puppet wants to move?
	if( MoveSpeed > 0.f )
	{
		processMove( Delta );
	}
}

void FPuppetComponent::processSight( Float delta )
{
	assert( SightDirection != SIGHT_None );
	assert( SightPeriod > 0.f );
	assert( SightRadius > 0.f );

	m_sightTimer -= delta;

	if( m_sightTimer < 0.f )
	{
		Int32 otherIndex = 0;
		Float sightRadiusSq = SightRadius * SightRadius;

		mem::zero( &m_puppetsInSight, sizeof( m_puppetsInSight ) );

		for( auto other = Level->FirstPuppet; other; other = other->NextPuppet )
		{
			if( other == this )
			{
				continue;
			}

			// direction test
			math::Vector direction = other->Base->Location - Base->Location;
			if( ( direction.x > 0.f && SightDirection == SIGHT_Left ) || 
				( direction.x < 0.f && SightDirection == SIGHT_Right ) )
			{
				continue;
			}

			// radius test
			if( direction.sizeSquared() > sightRadiusSq )
			{
				continue;
			}

			// LOS test
			if( !Level->TestLineGeom( headPosition(), other->headPosition(), true ) )
			{
				if( otherIndex < MAX_PUPPETS_IN_SIGHT )
				{
					m_puppetsInSight[otherIndex++] = other;
				}
				else
				{
					break;
				}

				Entity->OnLookAt( other->Entity );
			}
		}

		m_sightTimer = SightPeriod;
	}
}

void FPuppetComponent::processMove( Float delta )
{
	if( m_moveType != navi::EPathType::None && m_moveStatus == EMoveStatus::MOVE_InProgress )
	{
		switch( m_moveType )
		{
			case navi::EPathType::Walk:
			{
				m_moveStatus = moveWalk( delta );
				return;
			}
			case navi::EPathType::Jump:
			{
				m_moveStatus = moveJump( delta );
				return;
			}
			default:
			{
				LogManager::instance().handleScriptMessage( ELogLevel::Error, 
					L"Unimplemented move type %d in \"%s\"", m_moveType, *GetFullName() );
				break;
			}
		}
	}
}

EMoveStatus FPuppetComponent::moveWalk( Float delta )
{
	math::Vector destination;

	FArcadeBodyComponent* body = As<FArcadeBodyComponent>( Base );
	assert( body );

	if( m_targetEntity )
	{
		const Float otherRadius = max( m_targetEntity->Base->Size.x, m_targetEntity->Base->Size.y );
		const math::Vector& otherLocation = m_targetEntity->Base->Location;

		if( abs( Base->Location.x - otherLocation.x ) < ( m_targetRadius + otherRadius ) )
		{
			body->Velocity.x = 0.f; // ??
			return EMoveStatus::MOVE_Complete;
		}

		destination = m_targetEntity->Base->Location;
	}
	else
	{
		if( abs( m_targetPoint.x - Base->Location.x ) <= m_targetRadius )
		{
			body->Velocity.x = 0.f; // ??
			return EMoveStatus::MOVE_Complete;
		}

		destination = m_targetPoint;
	}

	body->Velocity.x = destination.x > body->Location.x ? MoveSpeed : -MoveSpeed;
	return EMoveStatus::MOVE_InProgress;
}

EMoveStatus FPuppetComponent::moveJump( Float delta )
{
	math::Vector destination;

	FArcadeBodyComponent* body = As<FArcadeBodyComponent>( Base );
	assert( body );

	Bool xReached = false;

	if( m_targetEntity )
	{
#if 0
		// broken
		const Float otherRadius = max( m_targetEntity->Base->Size.x, m_targetEntity->Base->Size.y );
		const math::Vector& otherLocation = m_targetEntity->Base->Location;

		if( ( Base->Location - otherLocation ).sizeSquared() <= sqr( m_targetRadius + otherRadius ) )
		{
			body->Velocity.x = 0.f; // ??
			return EMoveStatus::MOVE_Complete;
		}

		destination = m_targetEntity->Base->Location;
#else
		return EMoveStatus::MOVE_Aborted;
#endif
	}
	else
	{
		if( abs( Base->Location.x - m_targetPoint.x ) <= m_targetRadius )
		{
			xReached = true;
		}

		destination = m_targetPoint;
	}

	// x movement
	if( !xReached )
	{
		body->Velocity.x = destination.x > body->Location.x ? MoveSpeed : -MoveSpeed;
	}
	else
	{
		body->Velocity.x = 0.f; // ??
	}

	// y movement
	if( body->Floor != nullptr )
	{
		const Float desiredHeight = max( 0.f, destination.y - footPosition().y );

		if( xReached )
		{
			body->Velocity.y = ai::suggestJumpSpeed( desiredHeight, GravityScale );
		}
		else
		{
			body->Velocity.y = ai::verticalSpeedForJumping( footPosition(), destination, MoveSpeed, GravityScale );
		}
	}

	Bool yReached = abs( Base->Location.y - m_targetPoint.y) < max( m_targetRadius, Base->Size.y * 0.5f );
	return xReached && yReached ? EMoveStatus::MOVE_Complete : EMoveStatus::MOVE_InProgress;
}


/*-----------------------------------------------------------------------------
    In-Script AI functions.
-----------------------------------------------------------------------------*/

void FPuppetComponent::nativeMakePathTo( CFrame& Frame )
{
	math::Vector destination = POP_VECTOR;
	math::Vector* target = POPO_VECTOR;

	navi::TargetInfo targetInfo;
	Level->m_navigator.makePathTo( Level, seekerInfo(), destination, targetInfo );

	*target = targetInfo.location;
	*POPA_BYTE = static_cast<UInt8>( targetInfo.moveType );
}

void FPuppetComponent::nativeMakeRandomPath( CFrame& Frame )
{
	math::Vector* target = POPO_VECTOR;

	navi::TargetInfo targetInfo;
	Level->m_navigator.makeRandomPath( Level, seekerInfo(), targetInfo );

	*target = targetInfo.location;
	*POPA_BYTE = static_cast<UInt8>( targetInfo.moveType );
}

void FPuppetComponent::nativeMoveToPoint( CFrame& Frame )
{
	math::Vector destination = POP_VECTOR;
	Float radius = POP_FLOAT;
	navi::EPathType moveType = static_cast<navi::EPathType>( POP_BYTE );

	m_targetPoint = destination;
	m_targetEntity = nullptr;
	m_targetRadius = radius;
	m_moveStatus = EMoveStatus::MOVE_InProgress;
	m_moveType = moveType;
}

void FPuppetComponent::nativeMoveToEntity( CFrame& Frame )
{
	FEntity* pursued = POP_ENTITY;
	Float radius = POP_FLOAT;
	navi::EPathType moveType = static_cast<navi::EPathType>( POP_BYTE );

	if( !pursued )
	{
		LogManager::instance().handleScriptMessage( ELogLevel::Error, 
			L"MoveToEntity called without entity in \"%s\"", *GetFullName() );
		return;
	}

	m_targetPoint = pursued->Base->Location;
	m_targetEntity = pursued;
	m_targetRadius = radius;
	m_moveStatus = EMoveStatus::MOVE_InProgress;
	m_moveType = moveType;
}

void FPuppetComponent::nativeAbortMove( CFrame& Frame )
{
	m_moveStatus = EMoveStatus::MOVE_Aborted;
	m_moveType = navi::EPathType::None;
}

void FPuppetComponent::nativeMoveStatus( CFrame& Frame )
{
	*POPA_BYTE = m_moveStatus;
}

void FPuppetComponent::nativeGetWalkArea( CFrame& Frame )
{
	Float* minX = POPO_FLOAT;
	Float* maxX = POPO_FLOAT;
	Float* maxHeight = POPO_FLOAT;

	*POPA_BOOL = Level->m_navigator.getWalkArea( Level, seekerInfo(), *minX, *maxX, *maxHeight );
}

void FPuppetComponent::nativeSendOrder( CFrame& Frame )
{
	Int32 numRecipient = 0;
	String order = POP_STRING;
	Float radiusSq = sqr( POP_FLOAT );

	for( auto other = Level->FirstPuppet; other; other = other->NextPuppet )
	{
		if( other->Team == Team && other != this &&
			( headPosition() - other->headPosition() ).sizeSquared() < radiusSq )
		{
			other->Entity->OnGetOrder( Entity, order );
			numRecipient++;
		}
	}

	*POPA_INTEGER = numRecipient;
}

void FPuppetComponent::nativeInSight( CFrame& Frame )
{
	FEntity* testee = POP_ENTITY;

	if( testee )
	{
		for( const auto other : m_puppetsInSight )
		{
			if( other->Entity == testee )
			{
				*POPA_BOOL = true;
				return;
			}
		}
	}

	*POPA_BOOL = false;
}

void FPuppetComponent::nativeMakeNoise( CFrame& Frame )
{
	Float radiusSq = sqr( POP_FLOAT );

	for( auto other = Level->FirstPuppet; other; other = other->NextPuppet )
	{
		if( ( ( Base->Location - other->Base->Location ).sizeSquared() < radiusSq ) &&
			( other != this ) )
		{
			other->Entity->OnHearNoise( this->Entity );
		}
	}
}

navi::SeekerInfo FPuppetComponent::seekerInfo() const
{
	navi::SeekerInfo info;
	info.location = Base->Location;
	info.size = Base->Size;
	info.xSpeed = MoveSpeed;
	info.jumpHeight = JumpHeight;
	info.gravity = GravityScale;

	return info;
}

math::Vector FPuppetComponent::headPosition() const
{
	return math::Vector( Base->Location.x, Base->Location.y + Base->Size.y * 0.45f );
}

math::Vector FPuppetComponent::footPosition() const
{
	return math::Vector( Base->Location.x, Base->Location.y - Base->Size.y * 0.45f );
}

REGISTER_CLASS_CPP( FPuppetComponent, FExtraComponent, CLASS_SingleComp )
{
	using namespace navi;

	BEGIN_ENUM( ESightDirection );
		ENUM_ELEM( SIGHT_None );
		ENUM_ELEM( SIGHT_Left );
		ENUM_ELEM( SIGHT_Right );
		ENUM_ELEM( SIGHT_Both );
	END_ENUM;

	BEGIN_ENUM( EMoveStatus );
		ENUM_ELEM( MOVE_Unknown );
		ENUM_ELEM( MOVE_Complete );
		ENUM_ELEM( MOVE_InProgress );
		ENUM_ELEM( MOVE_Aborted );
	END_ENUM;

	BEGIN_ENUM( EPathType )
		ENUM_ELEM( PATH_None );
		ENUM_ELEM( PATH_Walk );
		ENUM_ELEM( PATH_Jump );
		ENUM_ELEM( PATH_Ladder );
		ENUM_ELEM( PATH_Teleport );
		ENUM_ELEM( PATH_Other );
	END_ENUM;

	ADD_PROPERTY( Health, PROP_Editable );
	ADD_PROPERTY( Team, PROP_Editable );
	ADD_PROPERTY( MoveSpeed, PROP_Editable );
	ADD_PROPERTY( JumpHeight, PROP_Editable );
	ADD_PROPERTY( GravityScale, PROP_Editable | PROP_Deprecated );
	ADD_PROPERTY( SightDirection, PROP_Editable );
	ADD_PROPERTY( SightPeriod, PROP_Editable );
	ADD_PROPERTY( SightRadius, PROP_Editable );

	DECLARE_METHOD( InSight, TYPE_Bool, ARG( testee, TYPE_Entity, END ) );
	DECLARE_METHOD( SendOrder, TYPE_Integer, ARG( order, TYPE_String, ARG( radius, TYPE_Float, END ) ) );
	DECLARE_METHOD( MakeNoise, TYPE_None, ARG( radius, TYPE_Float, END ) );
	DECLARE_METHOD( MakePathTo, TYPE_Byte, ARG( destination, TYPE_Vector, ARGOUT( target, TYPE_Vector, END ) ) );
	DECLARE_METHOD( MakeRandomPath, TYPE_Byte, ARGOUT( target, TYPE_Vector, END ) );
	DECLARE_METHOD( GetWalkArea, TYPE_Bool, ARGOUT( minX, TYPE_Float, ARGOUT( maxX, TYPE_Float, ARGOUT( maxHeight, TYPE_Float, END ) ) ) );
	DECLARE_METHOD( MoveStatus, TYPE_Byte, END );
	DECLARE_METHOD( AbortMove, TYPE_None, END );
	DECLARE_METHOD( MoveToPoint, TYPE_None, ARG( dest, TYPE_Vector, ARG( radius, TYPE_Float, ARG( moveType, TYPE_Byte, END ) ) ) );
	DECLARE_METHOD( MoveToEntity, TYPE_None, ARG( pursued, TYPE_Entity, ARG( radius, TYPE_Float, ARG( moveType, TYPE_Byte, END ) ) ) );
}

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/