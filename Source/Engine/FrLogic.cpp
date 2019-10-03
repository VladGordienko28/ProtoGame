/*=============================================================================
    FrLogic.cpp: Logic circuit classes.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FLogicComponent implementation.
-----------------------------------------------------------------------------*/

//
// Logic element constructor.
//
FLogicComponent::FLogicComponent()
	:	bEnabled( true ),
		NumPlugs( 0 ),
		NumJacks( 0 ),
		NextLogicElement( nullptr )
{
	bRenderable	= true;
	DrawOrder = 0.02f;
}


//
// Logic element destructor.
//
FLogicComponent::~FLogicComponent()
{
	// Remove this logic element from the level's list.
	com_remove(LogicElement);
}


//
// Initialize a logic element for the level.
//
void FLogicComponent::InitForEntity( FEntity* InEntity )
{
	FExtraComponent::InitForEntity( InEntity );
	com_add(LogicElement);
}


//
// Remove all bad connectors that are referenced
// to the null logic elements.
//
void FLogicComponent::CleanBadConnectors()
{
	for( Int32 iPlug=0; iPlug<MAX_LOGIC_PLUGS; iPlug++ )
	{
		for( Int32 iConn = 0; iConn<Plugs[iPlug].size(); )
			if( Plugs[iPlug][iConn].Target == nullptr )
				Plugs[iPlug].removeFast( iConn );
			else
				iConn++;
	}
}


//
// Some property has been changed in the logic element.
//
void FLogicComponent::EditChange()
{
	FExtraComponent::EditChange();

	// Count the number of the plugs and jacks.
	NumPlugs	= 0;
	NumJacks	= 0;

	for( Int32 i=0; i<MAX_LOGIC_JACKS; i++ )
		if( JacksName[i] )
			NumJacks++;

	for( Int32 i=0; i<MAX_LOGIC_PLUGS; i++ )
		if( PlugsName[i] )
			NumPlugs++;
}


//
// Add a new connector from the iPlug-th plug to the logic element
// InTarget to the iJack-th jack.
//
void FLogicComponent::AddConnector( FLogicComponent* InTarget, Int32 iPlug, Int32 iJack )
{
	TLogicConnector Connector;

	Connector.Target	= InTarget;
	Connector.iJack		= iJack;

	// Prevent duplicates also.
	Plugs[iPlug].addUnique( Connector );

	// Sometimes cleanup unused refs.
	CleanBadConnectors();
}


//
// Remove all connectors from the iPlug plug.
//
void FLogicComponent::RemoveConnectors( Int32 iPlug )
{
	Plugs[iPlug].empty();
}


//
// Logic element serialization.
//
void FLogicComponent::SerializeThis( CSerializer& S )
{
	FExtraComponent::SerializeThis( S );

	Serialize( S, bEnabled );

	for( Int32 i=0; i<MAX_LOGIC_PLUGS; i++ )
		Serialize( S, PlugsName[i] );

	for( Int32 i=0; i<MAX_LOGIC_JACKS; i++ )
		Serialize( S, JacksName[i] );

	Serialize( S, NumPlugs );
	Serialize( S, NumJacks );

	for( Int32 i=0; i<MAX_LOGIC_PLUGS; i++ )
		Serialize( S, Plugs[i] );

	// Here should destroy unused refs.
	if( S.GetMode() == SM_Undefined )
		CleanBadConnectors();
}


//
// Import the logic element.
//
void FLogicComponent::Import( CImporterBase& Im )
{
	FExtraComponent::Import( Im );

	IMPORT_INTEGER( NumPlugs );
	IMPORT_INTEGER( NumJacks );

	for( Int32 i=0; i<MAX_LOGIC_PLUGS; i++ )
		PlugsName[i] = Im.ImportString( *String::format( L"PlugsName[%d]", i ) );

	for( Int32 i=0; i<MAX_LOGIC_JACKS; i++ )
		JacksName[i] = Im.ImportString( *String::format( L"JacksName[%d]", i ) );

	// Import all connections.
	for( Int32 i=0; i<MAX_LOGIC_PLUGS; i++ )
		if( PlugsName[i] )
		{
			Plugs[i].setSize(Im.ImportInteger(*String::format( L"Plugs[%i].Num" , i )));
			for( Int32 j=0; j<Plugs[i].size(); j++ )
			{
				TLogicConnector& Conn = Plugs[i][j];

				Conn.iJack	= Im.ImportInteger(*String::format( L"Plugs[%i][%i].iJack" , i, j ));
				Conn.Target	= As<FLogicComponent>(Im.ImportObject(*String::format( L"Plugs[%i][%i].Target" , i, j )));
			}
		}
}


//
// Export the logic component.
//
void FLogicComponent::Export( CExporterBase& Ex )
{
	FExtraComponent::Export( Ex );

	EXPORT_INTEGER( NumPlugs );
	EXPORT_INTEGER( NumJacks );

	for( Int32 i=0; i<MAX_LOGIC_PLUGS; i++ )
		if( PlugsName[i] )
			Ex.ExportString( *String::format( L"PlugsName[%d]", i ), PlugsName[i] );

	for( Int32 i=0; i<MAX_LOGIC_JACKS; i++ )
		if( JacksName[i] )
			Ex.ExportString( *String::format( L"JacksName[%d]", i ), JacksName[i] );

	// This way sucks! To store connectors and
	// plugs! But works well.
	for( Int32 i=0; i<MAX_LOGIC_PLUGS; i++ )
		if( PlugsName[i] )
		{
			Ex.ExportInteger( *String::format( L"Plugs[%i].Num" , i ), Plugs[i].size() );
			for( Int32 j=0; j<Plugs[i].size(); j++ )
			{
				TLogicConnector& Conn = Plugs[i][j];

				Ex.ExportInteger( *String::format( L"Plugs[%i][%i].iJack" , i, j ), Conn.iJack );
				Ex.ExportObject( *String::format( L"Plugs[%i][%i].Target" , i, j ), Conn.Target );
			}
		}
}


//
// Return the world location of the iPlug (right-side) socket.
//
math::Vector FLogicComponent::GetPlugPos( Int32 iPlug )
{
	iPlug	= clamp( iPlug, 0, NumPlugs );

	math::Vector V;

	V.x	= Base->Location.x + Base->Size.x * 0.5f;
	V.y	= Base->Location.y + Base->Size.y * 0.5f - (iPlug+1)*Base->Size.y/(NumPlugs+1);

	return V;
}


//
// Return the world location of the iJack (left-side) socket.
//
math::Vector FLogicComponent::GetJackPos( Int32 iJack )
{
	iJack	= clamp( iJack, 0, NumJacks );

	math::Vector V;

	V.x	= Base->Location.x - Base->Size.x * 0.5f;
	V.y	= Base->Location.y + Base->Size.y * 0.5f - (iJack+1)*Base->Size.y/(NumJacks+1);

	return V;
}


//
// Render the logic element.
//
void FLogicComponent::Render( CCanvas* Canvas )
{
	// Is visible or not?
	if( (Level->RndFlags & RND_Logic) == 0 )
			return;

	math::Rect Rect = Base->GetAABB();

	// Pick a colors.
	math::Color WireColor = bEnabled ? math::colors::PALE_VIOLET_RED : math::colors::LIGHT_SLATE_GRAY;
	if( Base->bSelected )
		WireColor *= 1.5f;

	// Draw a gray pad.
	TRenderRect Pad;
	Pad.Flags		= POLY_Unlit | POLY_FlatShade | POLY_Ghost;
	Pad.Color		= bEnabled ? math::Color( 0x20, 0x20, 0x20, 0xff ) : math::Color( 0x40, 0x40, 0x40, 0xff );
	Pad.Bounds		= Rect;
	Pad.Image		= INVALID_HANDLE<rend::Texture2DHandle>();
	Pad.Rotation	= 0;
	Canvas->DrawRect( Pad );

	// Wire bounds.
	Canvas->DrawLineRect( Base->Location, Base->Size, 0, WireColor, false );

	// Draw the plugs sockets.
	for( Int32 i=0; i<NumPlugs; i++ )
		Canvas->DrawPoint( GetPlugPos(i), 7.f, WireColor );

	// Draw the jacks sockets.
	for( Int32 i=0; i<NumJacks; i++ )
		Canvas->DrawPoint( GetJackPos(i), 7.f, WireColor );

	// Draw connectors.
	for( Int32 iPlug=0; iPlug<NumPlugs; iPlug++ )
		for( Int32 iConn=0; iConn<Plugs[iPlug].size(); iConn++ )
		{
			TLogicConnector& Conn = Plugs[iPlug][iConn];

			if( Conn.Target )
				Canvas->DrawSmoothLine
							( 
								GetPlugPos(iPlug), 
								Conn.Target->GetJackPos(Conn.iJack), 
								WireColor, 
								false 
							);
		}
}


//
// Induce the signal and send it from the appropriate plug.
//
void FLogicComponent::nativeInduceSignal( CFrame& Frame )
{
	// Pop parameters.
	FEntity*	Creator		= POP_ENTITY;
	String		PlugName	= POP_STRING;

	// Don't induce if disabled.
	if( !bEnabled )
		return;
	   
	// Find the plug by it name.
	Int32 iPlug = -1;
	for( Int32 i=0; i<NumPlugs; i++ )
		if( PlugsName[i] == PlugName )
		{
			iPlug	= i;
			break;
		}

	if( iPlug == -1 )
	{
		debug( L"Logic: Plug '%s' not found in '%s'", *PlugName, *Entity->GetFullName() );
		return;
	}

	// Send the signal over the wires.
	for( Int32 i=0; i<Plugs[iPlug].size(); i++ )
	{
		TLogicConnector& Conn = Plugs[iPlug][i];

		if( Conn.Target )
			Conn.Target->Entity->OnReceiveSignal
							( 
								Creator,
								Entity,
								Conn.Target->JacksName[Conn.iJack]
							);
	}
}


/*-----------------------------------------------------------------------------
    TLogicConnector implementation.
-----------------------------------------------------------------------------*/

//
// Connector serialization.
//
void Serialize( CSerializer& S, TLogicConnector& V )
{
	Serialize( S, V.Target );
	Serialize( S, V.iJack );
}


//
// Logic connector comparison.
//
Bool TLogicConnector::operator==( const TLogicConnector& LC ) const
{
	return (Target == LC.Target) && (iJack == LC.iJack);
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FLogicComponent, FExtraComponent, CLASS_SingleComp )
{
	ADD_PROPERTY( bEnabled, PROP_Editable );
	ADD_PROPERTY( JacksName, PROP_NoImEx );	
	ADD_PROPERTY( PlugsName, PROP_NoImEx );

	DECLARE_METHOD( InduceSignal, TYPE_None, ARG(creator, TYPE_Entity, ARG(plugName, TYPE_String, END)) );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/