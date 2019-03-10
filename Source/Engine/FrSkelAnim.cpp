/*=============================================================================
	FrSkelAnim.cpp: Skeletal animation component.
	Created by Vlad Gordienko, Jan. 2018.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
	FSkeletonComponent implementation.
-----------------------------------------------------------------------------*/

//
// Skeleton component constructor.
//

FSkeletonComponent::FSkeletonComponent()
	:	FExtraComponent(),
		bHidden( false ),
		Color( math::colors::WHITE )
{

	bRenderable		= true;
	Skeleton		= nullptr;
	Scale			= math::Vector( 1.f, 1.f );
	Rate			= 0.f;
	Frame			= 0.f;
	iAction			= -1;

	bTickable	= true;
}


//
// Initialize skeleton for entity.
//
void FSkeletonComponent::InitForEntity( FEntity* InEntity )
{
	FExtraComponent::InitForEntity(InEntity);
}


// BEGIN HACKS.

void FSkeletonComponent::Render( CCanvas* Canvas )
{
	if( Skeleton )
	{
		if( iAction != -1 )
			Skeleton->RefPose.CumputeAnimFrame( Skeleton, Skeleton->Actions[iAction], Frame );
		else
			Skeleton->RefPose.ComputeRefTransform( Skeleton );

		Skeleton->Render( Canvas, Base->Location, Scale, Skeleton->RefPose );
	}
}


void FSkeletonComponent::Tick( Float Delta )
{
	if( iAction != -1 )
	{
		Frame += Delta * Rate;
		if( Frame > 1.f )
			Frame = 0.f;		// Only 1-sec animation works now.
	}
}

// END HACKS.


//
// Play animation.
//
void FSkeletonComponent::nativePlayAnim( CFrame& Frame )
{
	String InActionName = POP_STRING;
	Float InRate = POP_FLOAT;

	if( !Skeleton )
	{
		warn( L"Skel: No skeleton in \"%s\"", *GetFullName() );
		return;
	}
	Int32 iFound = Skeleton->FindAction(InActionName);
	if( iFound == -1 )
	{
		warn( L"Skel: Action \"%s\" not found in \"%s\"", *InActionName, *Skeleton->GetFullName() );
		return;
	}

	// Store.
	Rate	= InRate;
	iAction	= iFound;
}


//
// Component serialization.
//
void FSkeletonComponent::SerializeThis( CSerializer& S )
{
	FExtraComponent::SerializeThis(S);

	Serialize( S, bHidden );	
	Serialize( S, Color );
	
	Serialize( S, Skeleton );
	Serialize( S, Scale );

	// Don't serialize animation
	// runtime info.
}


/*-----------------------------------------------------------------------------
	Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FSkeletonComponent, FExtraComponent, CLASS_None )
{
	ADD_PROPERTY( bHidden, PROP_None );
	ADD_PROPERTY( Color, PROP_None );
	ADD_PROPERTY( Skeleton, PROP_Editable );
	ADD_PROPERTY( Scale, PROP_Editable );
	ADD_PROPERTY( Rate,	 PROP_Editable | PROP_NoImEx );//
	ADD_PROPERTY( Frame, PROP_Editable | PROP_NoImEx );//
	ADD_PROPERTY( iAction, PROP_Editable | PROP_NoImEx );//

	DECLARE_METHOD( PlayAnim, TYPE_None, ARG( name, TYPE_String, ARG( rate, TYPE_Float, END ) ) );
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/