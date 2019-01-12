/*=============================================================================
    FrPlay.cpp: Level test launcher.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    WPlayPage implementation.
-----------------------------------------------------------------------------*/

// Whether debug AI code?
#define AI_DEBUG_DRAW		1 && _DEBUG


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
	Caption			= String::Format( L"Play '%s'", *SourceLevel->GetName() );
	Color			= PAGE_COLOR_PLAY;
	TabWidth		= Root->Font1->TextWidth( *Caption ) + 30;

	// Run it!
	RunLevel();

	// Allocate watch dialog if debug.
	if( PlayMode == PLAY_Debug )
	{
		WatchList	= new WWatchListDialog( this, Root );
		WatchList->SetLocation( Root->Size.Width-350, Root->Size.Height-200 );
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
		GOutput->ScriptWarnf(*String::Format
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
	if( Messages.Num()>0 && (GPlat->Now()-LastPushTime)>7.0 )
	{
		LastPushTime	= GPlat->Now();
		Messages.RemoveShift( 0 );
	}

	// Output information into status bar.
	if( !(GFrameStamp & 15) )
	{
		if( PlayMode == PLAY_Debug )
		{
			GEditor->StatusBar->Panels[0].Text	= String::Format( L"Play Time: %.4f", PlayTime );
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
// Draw a paths network in the level.
//
#if AI_DEBUG_DRAW
static void DrawPathNetwork( CCanvas* Canvas, CNavigator* Navig )
{
	// Draw all nodes.
	for( Integer iNode=0; iNode<Navig->Nodes.Num(); iNode++ )
	{
		TPathNode& Node = Navig->Nodes[iNode];

		Canvas->DrawPoint
		(	 
			Node.Location,
			10.f,
			Node.GetDrawColor()
		);
	}

	// Draw all edges.
	TVector Bias( 0.f, (5.f*Canvas->View.FOV.Y*Canvas->View.Zoom)/(Canvas->View.Height) );
	for( Integer iEdge=0; iEdge<Navig->Edges.Num(); iEdge++ )
	{
		TPathEdge& Edge = Navig->Edges[iEdge];

		TPathNode& NodeA = Navig->Nodes[Edge.iStart];
		TPathNode& NodeB = Navig->Nodes[Edge.iFinish];

		Canvas->DrawLine
		(
			NodeA.Location,
			NodeB.Location + Bias,
			Edge.GetDrawColor(),
			false
		);
	}
}
#endif


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

	// AI debug.
#if AI_DEBUG_DRAW
	if( PlayLevel->Navigator && PlayMode == PLAY_Debug )
	{
		// Set level transform, for editor stuff rendering.
		TPoint Base = ClientToWindow(TPoint::Zero);
		Canvas->SetTransform
		( 
			TViewInfo
			( 
				PlayLevel->Camera.Location, 
				PlayLevel->Camera.Rotation, 
				PlayLevel->Camera.GetFitFOV(Size.Width, Size.Height), 
				PlayLevel->Camera.Zoom, 
				false, 
				Base.X, 
				Base.Y, 
				Size.Width, 
				Size.Height
			) 
		);
		DrawPathNetwork( Canvas, PlayLevel->Navigator );
	}
#endif

	// Draw info.
	Canvas->SetTransform( TViewInfo
								(
									P.X, 
									P.Y, 
									Size.Width, 
									Size.Height 
								) );

	// FPS.
	Canvas->DrawText
				( 
					String::Format( L"FPS: %d", GEditor->FPS ),
					Root->Font1, 
					COLOR_White, 
					TVector( 10, 10 ) 
				);

	// In-pause mode.
	if( PlayLevel->bIsPause )
		Canvas->DrawText
					(
						L"Pause",
						Root->Font1, 
						COLOR_White, 
						TVector( Size.Width/2.f, Size.Height/2.f )
					);


	// Output all script errors.
	for( Integer iMsg=0; iMsg<Messages.Num(); iMsg++ )
	{
		Canvas->DrawText
					( 
						Messages[iMsg], 
						Root->Font2, 
						COLOR_White, 
						TVector( 10.f, Size.Height - (Root->Font2->Height+2)*(iMsg+1) )
					);
	}
}


//
// Add a new message to the errors.
//
void WPlayPage::AddScriptMessage( ESeverity Severity, String Message )
{
	if( PlayMode == PLAY_Debug || Severity >= SVR_Log )
	{
		Messages.Push( Message );
		LastPushTime	= GPlat->Now();

		// Check for overflow.
		if( Messages.Num() > MAX_SCRIPT_MSG_LIST )
			Messages.RemoveShift( 0 );
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

	// Test some level's errors.
	if( !PlayLevel->Navigator )
		GOutput->ScriptErrorf(L"Level has no navigator");
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
void WPlayPage::OnKeyDown( Integer Key )
{
	// Close page, when <Esc> pressed.
	if( Key == KEY_Escape )
		Close( false );					
}


//
// User moves mouse over level.
//
void WPlayPage::OnMouseMove( EMouseButton Button, Integer X, Integer Y )
{
	WEditorPage::OnMouseMove( Button, X, Y );

	// Compute clipped area.
	TPoint Base				= ClientToWindow(TPoint::Zero);
	TVector	RealFOV			= PlayLevel->Camera.GetFitFOV( Size.Width, Size.Height );
	TVector	CamFOV			= PlayLevel->Camera.FOV;

	Float	lx	= 0,
			ly	= Size.Height*((RealFOV.Y-CamFOV.Y)/2.f)/RealFOV.Y,
			lw	= Size.Width,
			lh	= Size.Height*(CamFOV.Y/RealFOV.Y);

	Integer	TestX	= X,
			TestY	= Y-ly;

	// Test bounds.
	if( TestX>=0 && TestY>=0 && TestX<lw && TestY<lh )
	{
		// Screen cursor.
		GApp->GInput->MouseX		= TestX;
		GApp->GInput->MouseY		= TestY;

		// World cursor.
		GApp->GInput->WorldCursor	= TViewInfo
		(
			PlayLevel->Camera.Location,
			PlayLevel->Camera.Rotation,
			RealFOV,
			PlayLevel->Camera.Zoom,
			false,
			Base.X, Base.Y,
			Size.Width, Size.Height
		).Deproject( X, Y );
	}
}


//
// User has unpress mouse button.
//
void WPlayPage::OnMouseUp( EMouseButton Button, Integer X, Integer Y )
{
	WEditorPage::OnMouseUp( Button, X, Y );

	Integer	Key	=	Button == MB_Left	?	KEY_LButton :
					Button == MB_Right	?	KEY_RButton :
											KEY_MButton;

	GEditor->GInput->OnKeyUp( Key );
}


//
// User has press mouse button.
//
void WPlayPage::OnMouseDown( EMouseButton Button, Integer X, Integer Y )
{
	WEditorPage::OnMouseDown( Button, X, Y );

	Integer	Key	=	Button == MB_Left	?	KEY_LButton :
					Button == MB_Right	?	KEY_RButton :
											KEY_MButton;

	GEditor->GInput->OnKeyDown( Key );
}


//
// Double click.
//
void WPlayPage::OnDblClick( EMouseButton Button, Integer X, Integer Y )
{
	WEditorPage::OnDblClick( Button, X, Y );
	GEditor->GInput->OnKeyDown( KEY_DblClick );
	GEditor->GInput->OnKeyUp( KEY_DblClick );
}


//
// Mouse has been scrolled.
//
void WPlayPage::OnMouseScroll( Integer Delta )
{
	WEditorPage::OnMouseScroll(Delta);

	GEditor->GInput->WheelScroll	+= Delta;
	Integer Times	= Clamp( Delta / 120, -5, 5 );
	Integer	Key		= Times > 0 ? KEY_WheelUp : KEY_WheelDown;

	for( Integer i=0; i<Abs(Times); i++ )
	{
		GEditor->GInput->OnKeyDown( Key );
		GEditor->GInput->OnKeyUp( Key );
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/
