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
		NextPuppet(nullptr)
{
	// This variables are vary for all puppets
	// in scene, but here I set some initial 
	// values, that the most often used.
	Health				= 100;
	Clan				= 0;
	MoveSpeed			= 5.f;
	JumpHeight			= 8.f;
	GravityScale		= 10.f;
	Goal				= 
	GoalStart			= { 0.f, 0.f };
	GoalReach			= PATH_None;
	GoalHint			= 0.f;
	iGoalNode			=
	iHoldenNode			= -1;
	Body				= nullptr;
	LookDirection		= LOOK_None;
	LookPeriod			= 0.f;
	LookRadius			= 16.f;
	LookCounter			= 0.f;
	mem::zero( LookList, sizeof(LookList) );

	bTickable	= true;
}


//
// Puppet destructor.
//
FPuppetComponent::~FPuppetComponent()
{
	com_remove(Puppet);
}


//
// Initialize puppet for entity.
//
void FPuppetComponent::InitForEntity( FEntity* InEntity )
{
	FExtraComponent::InitForEntity(InEntity);

	com_add(Puppet);
}


//
// Serialize the puppet.
//
void FPuppetComponent::SerializeThis( CSerializer& S )
{
	FExtraComponent::SerializeThis( S );

	// Serialize only savable/loadable variables.
	Serialize( S, Health );
	Serialize( S, Clan );
	Serialize( S, MoveSpeed );
	Serialize( S, JumpHeight );
	Serialize( S, GravityScale );
	SerializeEnum( S, LookDirection );
	Serialize( S, LookPeriod );
	Serialize( S, LookRadius );

	// Also serialize list of Watched for GC.
	if( S.GetMode() == SM_Undefined )
		for( Int32 i=0; i<MAX_WATCHED; i++ )
			Serialize( S, LookList[i] );
}


//
// Prepare 'warriors' for the battle!
//
void FPuppetComponent::BeginPlay()
{
	FExtraComponent::BeginPlay();

	// Slightly modify a look time counter, to avoid all puppets 
	// watching at the same time.
	LookCounter	= LookPeriod * RandomF();

	// Get arcade body.
	Body	= As<FArcadeBodyComponent>(Base);
	if( !Body )
	{
		// Suicide, if no arcade base.
		debug( L"Puppet '%s' has no arcade body base!", *Entity->GetFullName() );
		Level->DestroyEntity( Entity );
	}

	// Reset navi variables.
	iGoalNode	=
	iHoldenNode	= -1;
}


/*-----------------------------------------------------------------------------
	Puppet AI functions.
-----------------------------------------------------------------------------*/

//
// AI tick.
//
void FPuppetComponent::Tick( Float Delta )
{
	if( !Body )
		return;

	// Did we want to watch puppets?
	if( LookPeriod > 0.f && LookRadius > 0.f && LookDirection != LOOK_None )
	{
		// Update timer.
		LookCounter += Delta;

		if( LookCounter > LookPeriod )
		{
			LookCounter	= 0.f;
			LookAtPuppets();
		}
	}
}


//
// Look to the puppets.
//
void FPuppetComponent::LookAtPuppets()
{
	// Prepare.
	Int32	iLookee = 0;
	Float	Dist2	= LookRadius*LookRadius; 
	mem::zero( LookList, sizeof(LookList) );

	for( FPuppetComponent* Other=Level->FirstPuppet; Other; Other = Other->NextPuppet )
	{
		if( Other == this )
			continue;

		// Look direction testing.
		math::Vector	Dir	= Other->Base->Location - Base->Location;
		if( Dir.x>0.f && LookDirection==LOOK_Left )		continue;
		if( Dir.x<0.f && LookDirection==LOOK_Right )	continue;

		// Distance and LOS testing.
		math::Vector UnusedHit, UnusedNormal;
		if( Dir.sizeSquared() > Dist2 ) continue;
		if	( Level->TestLineGeom
				(
					Base->Location + math::Vector( 0.f, Base->Size.y*0.5f ),
					Other->Base->Location + math::Vector( 0.f, Other->Base->Size.y*0.5f ),
					true,
					UnusedHit, UnusedNormal
				)
			)
				continue;

		// Yes! Other is visible for this.
		if( iLookee >= MAX_WATCHED )
		{
			LookList[iLookee] = Other;
			iLookee++;
		}
		else
			break;

		Entity->OnLookAt( Other->Entity );
	}
}


/*-----------------------------------------------------------------------------
	Puppet movement functions.
-----------------------------------------------------------------------------*/

//
// Moves puppet to the goal, just set velocity to reach it. Returns true, if
// puppet hits the goal. If goal is unreachabled for now returns false. 
//
Bool FPuppetComponent::MoveToGoal()
{
	if( !Body )
	{
		LogManager::instance().handleScriptMessage( 
			ELogLevel::Error, L"Puppet used without appropriate arcade body in '%s'", *Entity->Script->GetName() );

		return false;
	}

	// Did we have something to move?
	if( GoalReach == PATH_None )
		return false;

	if( GoalReach == PATH_Walk )
	{
		//
		// Walking.
		//
		if( Goal.x > GoalStart.x )
		{
			// Walk rightward.
			if( Body->Location.x < Goal.x+Body->Size.x*0.25f )
			{
				Body->Velocity.x	= MoveSpeed;
				return false;
			}
			else
			{
				Body->Velocity.x	= 0.f;
				return true;
			}
		}
		else
		{
			// Walk leftward.
			if( Body->Location.x > Goal.x-Body->Size.x*0.25f )
			{
				Body->Velocity.x	= -MoveSpeed;
				return false;
			}
			else
			{
				Body->Velocity.x	= 0.f;
				return true;
			}
		}
	}
	else if( GoalReach == PATH_Jump )
	{
		//
		// Jumping.
		//
		Bool bXOk	= false;
		if( Goal.x > GoalStart.x )
		{
			// X move rightward.
			Body->Velocity.x	= MoveSpeed;
			bXOk				= Body->Location.x > Goal.x+Body->Size.x*0.25f;
		}
		else
		{
			// X move leftward.
			Body->Velocity.x	= -MoveSpeed;	
			bXOk				= Body->Location.x < Goal.x-Body->Size.x*0.25f;
		}

		if( Body->Floor && !bXOk )
			Body->Velocity.y	= GoalHint * 1.f;

		if( bXOk )
			Body->Velocity.x	= 0.f;

		return bXOk && abs(Goal.y - Body->Location.y)<Body->Size.y*0.5f;
	}
	else
	{
		//
		// Miscellaneous, handled by script, here we detect only ovelap.
		//
		return Body->GetAABB().isInside( Goal );
	}
}


//
// Returns true, if figure can make a jump from the 'From' to 'To' spot.
// SuggestedSpeed - Recommended initial speed for jumping.
//
Bool FPuppetComponent::CanJumpTo( math::Vector From, math::Vector To, Float& SuggestedSpeed )
{
	Float Speed	= SpeedForJump( From, To, MoveSpeed, GravityScale );

	if( Speed < SuggestJumpSpeed( JumpHeight, GravityScale ) )
	{
		// Possible.
		SuggestedSpeed	= Speed;
		return true;
	}
	else
	{
		// Impossible.
		SuggestedSpeed	= 0.f;
		return false;
	}
}


//
// Computes a maximum reached jump height with an initial velocity JumpSpeed
// and Gravity.
//
Float FPuppetComponent::SuggestJumpHeight( Float JumpSpeed, Float Gravity )
{
	Float Time	= JumpSpeed / Gravity;
	return JumpSpeed*Time - 0.5f*Time*Time*Gravity;
}


//
// Computes a jump speed to reach Height height with
// an given Gravity.
//
Float FPuppetComponent::SuggestJumpSpeed( Float DesiredHeight, Float Gravity )
{
	return math::sqrt(2.f * Gravity * DesiredHeight);
}


//
// Returns the vertical speed to jump from the 'From' location to 
// the 'To' location.
//
Float FPuppetComponent::SpeedForJump( math::Vector From, math::Vector To, Float XSpeed, Float Gravity )
{
	Float XTime	= abs(From.x - To.x) / XSpeed;
	Float Speed = (To.y + 0.5f*XTime*XTime*Gravity - From.y) / XTime;

	// Slightly modify speed to make unpredictable results.
	return max( 0.f, Speed ) * 1.1f;
}


/*-----------------------------------------------------------------------------
    In-Script AI functions.
-----------------------------------------------------------------------------*/

//
// Emit a noise, from the puppet.
//
void FPuppetComponent::nativeMakeNoise( CFrame& Frame )
{
	Float Radius2 = sqr(POP_FLOAT);
	for( FPuppetComponent* Other=Level->FirstPuppet; Other; Other = Other->NextPuppet )
	{
		if	( 
				(Base->Location-Other->Base->Location).sizeSquared() < Radius2 && 
				Other != this 
			)
		{
			Other->Entity->OnHearNoise(this->Entity);
		}
	}
}


//
// Suggest a jump height using initial jump speed.
//
void FPuppetComponent::nativeSuggestJumpHeight( CFrame& Frame )
{
	Float Speed = POP_FLOAT;
	*POPA_FLOAT	= FPuppetComponent::SuggestJumpHeight
	(
		Speed,
		GravityScale
	);
}


//
// Suggest a jump speed to reach height.
//
void FPuppetComponent::nativeSuggestJumpSpeed( CFrame& Frame )
{
	Float Height = POP_FLOAT;
	*POPA_FLOAT	= FPuppetComponent::SuggestJumpSpeed
	(
		Height,
		GravityScale
	);
}


//
// Send an order to puppet's teammates in radius.
//
void FPuppetComponent::nativeSendOrder( CFrame& Frame )
{
	Int32	NumRecipients	= 0;
	String	Order			= POP_STRING;
	Float	Radius2			= sqr(POP_FLOAT);

	for( FPuppetComponent* Testee=Level->FirstPuppet; Testee; Testee = Testee->NextPuppet )
	{
		if	(
				Testee->Clan == Clan &&
				(Base->Location-Testee->Base->Location).sizeSquared() < Radius2 &&
				Testee != this
			)
		{
			Testee->Entity->OnGetOrder( this->Entity, Order );
			NumRecipients++;
		}
	}

	*POPA_INTEGER	= NumRecipients;
}


//
// Return true, if other is visible.
//
void FPuppetComponent::nativeIsVisible( CFrame& Frame )
{
	FEntity* Other	= POP_ENTITY;
	if( !Other )
	{
		*POPA_BOOL	= false;
		return;
	}

	for( Int32 i=0; i<MAX_WATCHED && LookList[i]; i++ )
		if( LookList[i]->Entity == Other )
		{
			*POPA_BOOL	= true;
			return;
		}

	*POPA_BOOL	= false;
}


//
// Move puppet to the goal.
//
void FPuppetComponent::nativeMoveToGoal( CFrame& Frame )
{
	*POPA_BOOL	= MoveToGoal();
}


//
// Tries to create a random path.
//
void FPuppetComponent::nativeCreateRandomPath( CFrame& Frame )
{
	if( Level->Navigator )
	{
		*POPA_BOOL	= Level->Navigator->MakeRandomPath( this );
	}
	else
	{
		debug( L"AI: Level has no navigator" );
		*POPA_BOOL	= false;
	}
}


//
// Tries to create a path to specified location, returns true
// if path was successfully created, otherwise returns false.
//
void FPuppetComponent::nativeCreatePathTo( CFrame& Frame )
{
	math::Vector	Dest	= POP_VECTOR;
	if( Level->Navigator )
	{
		*POPA_BOOL	= Level->Navigator->MakePathTo( this, Dest );
	}
	else
	{
		debug( L"AI: Level has no navigator" );
		*POPA_BOOL	= false;
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/