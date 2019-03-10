/*=============================================================================
    FrPortal.cpp: Portals.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FPortalComponent implementation.
-----------------------------------------------------------------------------*/

//
// Portal constructor.
//
FPortalComponent::FPortalComponent()
	:	Width( 16.f ),
		NextPortal( nullptr )
{
	Size		= math::Vector( 0.f, 0.f );
	bRenderable	= true;
	bFixedAngle	= false;
}


//
// Portal destructor.
//
FPortalComponent::~FPortalComponent()
{
	// Remove from level's list.
	com_remove(Portal);
}


//
// Transfer a point through the portal.
//
math::Vector FPortalComponent::TransferPoint( math::Vector P )
{
	return P;
}


//
// Transfer a vector through the portal.
//
math::Vector FPortalComponent::TransferVector( math::Vector V )
{
	return V;
}


//
// Compute render info to render the scene through the
// portal, return true, if computed successfully.
//
Bool FPortalComponent::ComputeViewInfo( const TViewInfo& Parent, TViewInfo& Result )
{
	return false;
}


//
// Initialize portal for entity.
//
void FPortalComponent::InitForEntity( FEntity* InEntity )
{
	FBaseComponent::InitForEntity( InEntity );
	com_add(Portal);
}


//
// When some field changed in portal.
//
void FPortalComponent::EditChange()
{
	FBaseComponent::EditChange();
	Width	= clamp( Width, 2.f, 64.f );
}


//
// Serialize portal.
//
void FPortalComponent::SerializeThis( CSerializer& S )
{
	FBaseComponent::SerializeThis( S );
	Serialize( S, Width );
}


/*-----------------------------------------------------------------------------
    FMirrorComponent implementation.
-----------------------------------------------------------------------------*/

//
// Mirror constructor.
//
FMirrorComponent::FMirrorComponent()
	:	FPortalComponent()
{
	bFixedAngle	= true;
}


//
// Transfer point through the mirror.
//
math::Vector FMirrorComponent::TransferPoint( math::Vector P )
{
	return math::Vector( P.x + (Location.x - P.x)*2.f, P.y );
}


//
// Transfer vector through the mirror.
//
math::Vector FMirrorComponent::TransferVector( math::Vector V )
{
	return math::Vector( -V.x, V.y );
}


//
// Compute TViewInfo for other side of the mirror.
//
Bool FMirrorComponent::ComputeViewInfo( const TViewInfo& Parent, TViewInfo& Result )
{
	Result.X				= Parent.X;
	Result.Y				= Parent.Y;
	Result.Width			= Parent.Width;
	Result.Height			= Parent.Height;
	Result.bMirage			= true;
	Result.Coords.origin	= TransferPoint( Parent.Coords.origin );
	Result.Coords.xAxis		= -Parent.Coords.xAxis;
	Result.Coords.yAxis		= Parent.Coords.yAxis;
	Result.UnCoords			= Result.Coords.transpose();
	Result.FOV				= Parent.FOV;
	Result.Zoom				= Parent.Zoom;

	// Mirror bounds.
	if( Location.x > Parent.Coords.origin.x )
	{
		// Parent lie on the left half-plane.
		Result.Bounds.min.x	= Parent.Bounds.min.x;
		Result.Bounds.max.x	= Location.x + (Location.x-Parent.Bounds.min.x);
	}
	else
	{
		// Parent lie on the right half-plane.
		Result.Bounds.min.x	= Location.x - (Parent.Bounds.max.x - Location.x);
		Result.Bounds.max.x	= Parent.Bounds.max.x;
	}

	Result.Bounds.min.y	= Parent.Bounds.min.y;
	Result.Bounds.max.y	= Parent.Bounds.max.y;

	return true;
}


//
// Render the mirror.
//
void FMirrorComponent::Render( CCanvas* Canvas )
{
	// Never draw fake mirror.
	if( Canvas->View.bMirage || !(Level->RndFlags & RND_Other) )
		return;

	math::Color DrawColor = math::colors::DODGER_BLUE;
	if( bSelected )
		DrawColor *= 1.5f;

	// Master line.
	math::Vector V1 = math::Vector( Location.x, Location.y - 0.5f * Width );
	math::Vector V2 = math::Vector( Location.x, Location.y + 0.5f * Width );

	Canvas->DrawLine( V1, V2, DrawColor, false );

	// Tips.
	Canvas->DrawCoolPoint( V1, 5.f, DrawColor );
	Canvas->DrawCoolPoint( V2, 5.f, DrawColor );

	// Centroid.
	Canvas->DrawPoint( Location, 4.f, DrawColor );
}


void FMirrorComponent::nativeMirrorPoint( CFrame& Frame )
{
	math::Vector P = POP_VECTOR;
	*POPA_VECTOR = TransferPoint( P );
}


void FMirrorComponent::nativeMirrorVector( CFrame& Frame )
{
	math::Vector V = POP_VECTOR;
	*POPA_VECTOR = TransferVector( V );
}


/*-----------------------------------------------------------------------------
    FWarpComponent implementation.
-----------------------------------------------------------------------------*/

//
// Warp constructor.
//
FWarpComponent::FWarpComponent()
	:	FPortalComponent(),
		Other( nullptr )
{
	bFixedAngle	= false;
}


//
// Transfer a point through the warp portal. 
// Caution: Destination should be specified.
//
math::Vector FWarpComponent::TransferPoint( math::Vector P )
{
	assert(Other);
	return math::transformPointBy( math::transformPointBy(P, ToLocal()), Other->Base->ToWorld() );
}


//
// Transfer a vector through the warp portal. 
// Caution: Destination should be specified.
//
math::Vector FWarpComponent::TransferVector( math::Vector V )
{
	assert(Other);
	return math::transformVectorBy( math::transformVectorBy(V, ToLocal()), Other->Base->ToWorld() );
}


//
// Compute TViewInfo for the other side of the warp
// portal.
//
Bool FWarpComponent::ComputeViewInfo( const TViewInfo& Parent, TViewInfo& Result )
{
	if( !Other )
		return false;

	Result.X				= Parent.X;
	Result.Y				= Parent.Y;
	Result.Width			= Parent.Width;
	Result.Height			= Parent.Height;
	Result.bMirage			= true;
	Result.Coords.origin	= TransferPoint( Parent.Coords.origin );
	Result.Coords.xAxis		= TransferVector( Parent.Coords.xAxis );
	Result.Coords.yAxis		= TransferVector( Parent.Coords.yAxis );
	Result.UnCoords			= Result.Coords.transpose();
	Result.FOV				= Parent.FOV;
	Result.Zoom				= Parent.Zoom;
	Result.Bounds			= math::Rect
								( 
									Result.Coords.origin, 
									math::sqrt( sqr(Result.FOV.x)+sqr(Result.FOV.y)*Result.Zoom  )
								);

	return true;
}


//
// When some field changed in warp.
//
void FWarpComponent::EditChange()
{
	FPortalComponent::EditChange();

	if( Other && !Other->Base->IsA(FWarpComponent::MetaClass) )
		Other = nullptr;
}


//
// Warp serialization.
//
void FWarpComponent::SerializeThis( CSerializer& S )
{
	FPortalComponent::SerializeThis( S );
	Serialize( S, Other );
}


//
// Render the warp.
//
void FWarpComponent::Render( CCanvas* Canvas )
{
	// Don't draw fake.
	if( Canvas->View.bMirage || !(Level->RndFlags & RND_Other) )
		return;

	math::Coords C = ToWorld();
	math::Vector V1 = math::transformPointBy( math::Vector( 0.f, -Width*0.5f ), C );
	math::Vector V2 = math::transformPointBy( math::Vector( 0.f, +Width*0.5f ), C );

	math::Color DrawColor = Other ? math::colors::GREEN : math::colors::CRIMSON;
	if( bSelected )
		DrawColor *= 2.f;

	Canvas->DrawLine( V1, V2, DrawColor, false );

	// Tips.
	Canvas->DrawCoolPoint( V1, 6.f, DrawColor );
	Canvas->DrawCoolPoint( V2, 6.f, DrawColor );

	// Centroid.
	Canvas->DrawPoint( Location, 4.f, DrawColor );
}


void FWarpComponent::nativeWarpPoint( CFrame& Frame )
{
	math::Vector P = POP_VECTOR;
	*POPA_VECTOR = Other ? TransferPoint( P ) : P;
}


void FWarpComponent::nativeWarpVector( CFrame& Frame )
{
	math::Vector V = POP_VECTOR;
	*POPA_VECTOR = Other ? TransferVector( V ) : V;
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FPortalComponent, FBaseComponent, CLASS_Abstract )
{
	ADD_PROPERTY( Width, PROP_Editable );
}


REGISTER_CLASS_CPP( FMirrorComponent, FPortalComponent, CLASS_None )
{
	DECLARE_METHOD( MirrorPoint, TYPE_Vector, ARG(point, TYPE_Vector, END) );
	DECLARE_METHOD( MirrorVector, TYPE_Vector, ARG(vect, TYPE_Vector, END) );
}


REGISTER_CLASS_CPP( FWarpComponent, FPortalComponent, CLASS_None ) 
{
	ADD_PROPERTY( Other, PROP_Editable );

	DECLARE_METHOD( WarpPoint, TYPE_Vector, ARG(point, TYPE_Vector, END) );
	DECLARE_METHOD( WarpVector, TYPE_Vector, ARG(vect, TYPE_Vector, END) );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/