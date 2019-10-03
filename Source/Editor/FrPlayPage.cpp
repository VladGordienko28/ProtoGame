/*=============================================================================
    FrPlay.cpp: Level test launcher.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    WPlayPage implementation.
-----------------------------------------------------------------------------*/

// Whether debug AI code?
#define AI_DEBUG_DRAW		1 && FLU_DEBUG


//
// Play page constructor.
//
WPlayPage::WPlayPage( FLevel* InOrigianl, EPlayMode InPlayMode, WContainer* InOwner, WWindow* InRoot )
	:	WEditorPage( InOwner, InRoot ),
		PlayMode( InPlayMode ),
		SourceLevel( InOrigianl ),
		PlayLevel( nullptr ),
		WatchList( nullptr ),
		Messages(),
		LastPushTime( 0.0 ),
		PlayTime( 0.f )
{
	// Initialize level's variables.
	PageType		= PAGE_Play;
	Caption			= String::format( L"Play '%s'", *SourceLevel->GetName() );
	Color			= PAGE_COLOR_PLAY;
	TabWidth		= Root->Font1->textWidth( *Caption ) + 30;

	LogManager::instance().addCallback( this );

	// Run it!
	RunLevel();

	// Allocate watch dialog if debug.
	if( PlayMode == PLAY_Debug )
	{
		WatchList	= new WWatchListDialog( this, Root );
		WatchList->SetLocation( Root->Size.Width-350, Root->Size.Height-200 );

#if AI_DEBUG_DRAW
		PlayLevel->RndFlags |= RND_Paths;
#endif
	}
}


//
// Play page destructor.
//
WPlayPage::~WPlayPage()
{
	if( WatchList )
		delete WatchList;

	ShutdownLevel();

	LogManager::instance().removeCallback( this );
}


//
// Tick played level.
//
void WPlayPage::TickPage( Float Delta )
{
	// Tick the level.
	if( PlayLevel )
	{
		PlayLevel->Tick( Delta );

		if( !PlayLevel->bIsPause )
			PlayTime	+= Delta;
	}

	// Handle level's travel. Play page don't allow to
	// travel, so just notify player about it.
	if( GIncomingLevel )
	{
		LogManager::instance().handleScriptMessage( ELogLevel::Warning, *String::format
		(
			L"An attempt to travel to level '%s'. Teleportee is '%s'", 
			*GIncomingLevel.Destination->GetName(), 
			GIncomingLevel.Teleportee ? *GIncomingLevel.Teleportee->GetFullName() : L"null"
		) );

		// Reject travel.
		GIncomingLevel.bCopy		= false;
		GIncomingLevel.Destination	= nullptr;
		GIncomingLevel.Teleportee	= nullptr;
	}

	// Update errors list.
	if( Messages.size()>0 && (GPlat->Now()-LastPushTime)>7.0 )
	{
		LastPushTime	= GPlat->Now();
		Messages.removeShift( 0 );
	}

	// Output information into status bar.
	if( !(GFrameStamp & 15) )
	{
		if( PlayMode == PLAY_Debug )
		{
			GEditor->StatusBar->Panels[0].Text	= String::format( L"Play Time: %.4f", PlayTime );
			GEditor->StatusBar->Panels[1].Text	= L"";
		}
		else
		{
			GEditor->StatusBar->Panels[0].Text	= L"";
			GEditor->StatusBar->Panels[1].Text	= L"";
		}
	}
}


//
// Render played level.
//
void WPlayPage::RenderPageContent( CCanvas* Canvas )
{
	TPoint P = ClientToWindow(TPoint( 0, 0 ));

	if( PlayLevel )
		GEditor->GRender->RenderLevel
								(
									Canvas, 
									PlayLevel, 
									P.X, 
									P.Y, 
									Size.Width, 
									Size.Height
								);

	// Draw info.
	Canvas->SetTransform( gfx::ViewInfo
								(
									P.X, 
									P.Y, 
									Size.Width, 
									Size.Height 
								) );

	// FPS.
	Canvas->DrawText
				( 
					String::format( L"FPS: %d", GEditor->FPS ),
					Root->Font1, 
					math::colors::WHITE, 
					math::Vector( 10, 10 ) 
				);

#if FLU_PROFILE_MEMORY
	Canvas->DrawText
				( 
					String::format( L"Mem: %.2f kB", Double(mem::stats().totalAllocatedBytes) / 1024 ), 
					Root->Font1, 
					math::colors::WHITE, 
					math::Vector( 10.f, 30.f ) 
				);
#endif

	Canvas->DrawText
				( 
					String::format( L"Game Time: %s", *PlayLevel->m_environmentContext.getCurrentTime().toString() ), 
					Root->Font1, 
					math::colors::WHITE, 
					math::Vector( 10.f, 50.f ) 
				);

	// experimental
	/*
	static SizeT oldAllocations = 0;
	SizeT numAllocs = mem::stats().totalAllocations;

	Canvas->DrawText
				( 
					String::format( L"Allocations: %d", numAllocs - oldAllocations ), 
					Root->Font1, 
					math::colors::WHITE, 
					math::Vector( 10.f, 70.f ) 
				);
	oldAllocations = numAllocs;
	*/


	// In-pause mode.
	if( PlayLevel->bIsPause )
		Canvas->DrawText
					(
						L"Pause",
						Root->Font1, 
						math::colors::WHITE, 
						math::Vector( Size.Width/2.f, Size.Height/2.f )
					);


	// Output all script errors.
	for( Int32 iMsg=0; iMsg<Messages.size(); iMsg++ )
	{
		Canvas->DrawText
					( 
						Messages[iMsg].message, 
						Root->Font2, 
						Messages[iMsg].color, 
						math::Vector( 10.f, Size.Height - (Root->Font2->maxHeight()+2)*(iMsg+1) )
					);
	}
}


//
// Add a new message to the errors.
//
void WPlayPage::AddScriptMessage( Bool bImportant, String Message, math::Color Color )
{
	if( PlayMode == PLAY_Debug || bImportant )
	{
		Messages.push( { Message, Color } );
		LastPushTime	= GPlat->Now();

		// Check for overflow.
		if( Messages.size() > MAX_SCRIPT_MSG_LIST )
			Messages.removeShift( 0 );
	}
}


//
// Run level to play.
//
void WPlayPage::RunLevel()
{
	assert(SourceLevel);
	assert(!PlayLevel);

	// Duplicate the level.
	PlayLevel	= GProject->DuplicateLevel( SourceLevel );

	// Let's play it!
	PlayLevel->RndFlags		= RND_Game;
	PlayLevel->BeginPlay();

	// Turn on some non-game render flags for debug.
	if( PlayMode == PLAY_Debug )
		PlayLevel->RndFlags	|= RND_Logic;

#if 0
	// Test some level's errors.
	if( !PlayLevel->Navigator )
		AddScriptMessage( false, L"Level has no navigator" );
#endif
}


//
// Shutdown play session.
//
void WPlayPage::ShutdownLevel()
{
	if( PlayLevel )
	{
		PlayLevel->EndPlay();
		DestroyObject( PlayLevel, true );
		PlayLevel = nullptr;

		// Just in case.
		GApp->GAudio->PlayMusic( nullptr, 1.5 );
	}
}


//
// Page has been opened or restored.
//
void WPlayPage::OnOpen()
{
	GEditor->Inspector->SetEditObject( PlayLevel );
	GEditor->GInput->SetLevel( PlayLevel );
}


/*-----------------------------------------------------------------------------
    Played level input.
-----------------------------------------------------------------------------*/

//
// User press keyboard button.
//
void WPlayPage::OnKeyDown( Int32 Key )
{
	// Close page, when <Esc> pressed.
	if( Key == KEY_Escape )
		Close( false );					
}


//
// User moves mouse over level.
//
void WPlayPage::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	WEditorPage::OnMouseMove( Button, X, Y );

	// Compute clipped area.
	TPoint Base				= ClientToWindow(TPoint::Zero);
	math::Vector	RealFOV	= PlayLevel->Camera.GetFitFOV( Size.Width, Size.Height );
	math::Vector	CamFOV	= PlayLevel->Camera.FOV;

	Float	lx	= 0,
			ly	= Size.Height*((RealFOV.y-CamFOV.y)/2.f)/RealFOV.y,
			lw	= Size.Width,
			lh	= Size.Height*(CamFOV.y/RealFOV.y);

	Int32	TestX	= X,
			TestY	= Y-ly;

	// Test bounds.
	if( TestX>=0 && TestY>=0 && TestX<lw && TestY<lh )
	{
		// Screen cursor.
		GApp->GInput->MouseX		= TestX;
		GApp->GInput->MouseY		= TestY;

		// World cursor.
		GApp->GInput->WorldCursor	= gfx::ViewInfo
		(
			PlayLevel->Camera.Location,
			PlayLevel->Camera.Rotation,
			RealFOV,
			PlayLevel->Camera.Zoom,
			false,
			Base.X, Base.Y,
			Size.Width, Size.Height
		).deproject( X, Y );
	}
}


//
// User has unpress mouse button.
//
void WPlayPage::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	WEditorPage::OnMouseUp( Button, X, Y );

	Int32	Key	=	Button == MB_Left	?	KEY_LButton :
					Button == MB_Right	?	KEY_RButton :
											KEY_MButton;

	GEditor->GInput->OnKeyUp( Key );
}


//
// User has press mouse button.
//
void WPlayPage::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	WEditorPage::OnMouseDown( Button, X, Y );

	Int32	Key	=	Button == MB_Left	?	KEY_LButton :
					Button == MB_Right	?	KEY_RButton :
											KEY_MButton;

	GEditor->GInput->OnKeyDown( Key );
}


//
// Double click.
//
void WPlayPage::OnDblClick( EMouseButton Button, Int32 X, Int32 Y )
{
	WEditorPage::OnDblClick( Button, X, Y );
	GEditor->GInput->OnKeyDown( KEY_DblClick );
	GEditor->GInput->OnKeyUp( KEY_DblClick );
}


//
// Mouse has been scrolled.
//
void WPlayPage::OnMouseScroll( Int32 Delta )
{
	WEditorPage::OnMouseScroll(Delta);

	GEditor->GInput->WheelScroll	+= Delta;
	Int32 Times	= clamp( Delta / 120, -5, 5 );
	Int32	Key	= Times > 0 ? KEY_WheelUp : KEY_WheelDown;

	for( Int32 i=0; i<abs(Times); i++ )
	{
		GEditor->GInput->OnKeyDown( Key );
		GEditor->GInput->OnKeyUp( Key );
	}
}


void WPlayPage::handleMessage( ELogLevel level, const Char* message )
{
	AddScriptMessage( false, message, math::colors::LIGHT_STEEL_BLUE );
}

void WPlayPage::handleScriptMessage( ELogLevel level, const Char* message )
{
	const auto color = level >= ELogLevel::Error ? math::colors::FIRE_BRICK : level == ELogLevel::Warning ? 
		math::colors::GOLDENROD : math::colors::WHITE;

	AddScriptMessage( level >= ELogLevel::Warning, message, color );
}

void WPlayPage::handleFatalMessage( const Char* message )
{
}

void WPlayPage::handleFatalScriptMessage( const Char* message )
{
	AddScriptMessage( true, message, math::colors::RED );
}

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/