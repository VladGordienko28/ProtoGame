/*=============================================================================
    FrGame.cpp: Game application class.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

#include "Game.h"
#include "FrJoy.h"
#include "Res\resource.h"

/*-----------------------------------------------------------------------------
    Declarations.
-----------------------------------------------------------------------------*/

//
// Game window minimum size.
//
#define MIN_GAME_X		320
#define MIN_GAME_Y		240


//
// Game directories.
//
#define SPLASH_DIR			L"Doc\\Splash.bmp"


//
// Forward declaration.
//
static LRESULT CALLBACK WndProc( HWND hWnd, UINT Message, WPARAM WParam, LPARAM LParam );
static Char* StackTrace( LPEXCEPTION_POINTERS InException );
static Int32 HandleException( LPEXCEPTION_POINTERS InException );
static String GetFileName( String FileName );
static String GetFileDir( String FileName );


//
// Globals.
//
CGame*	GGame	= nullptr;


/*-----------------------------------------------------------------------------
    CGame implementation.
-----------------------------------------------------------------------------*/

//
// Game pre-initialization.
//
CGame::CGame()
	:	CApplication(),
		Console( nullptr ),
		Level( nullptr )
{
	// Say hello to user.
	log( L"========================="			);
	log( L"=    Fluorine Engine    ="			);
	log( L"=      %s        =",			FLU_VERSION	);
	log( L"========================="			);
	log( L"" );

	// Initialize global variables.
	GIsEditor			= false;
	GGame				= this;

	// Exe-directory.
	Char Directory[256];
	_wgetcwd( Directory, arr_len(Directory) );
	GDirectory	= Directory;

	// Parse command line.
	GCmdLine = GetCommandLine();

	// Initialize C++ stuff.
	srand(GetTickCount() ^ 0x20162016);
}


//
// Game destruction.
//
CGame::~CGame()
{
	GGame	= nullptr;
}


//
// Load game from file.
//
Bool CGame::LoadGame( String Directory, String Name )
{
	// Load game file.
	if( !CApplication::LoadGame( Directory, Name ) )
		return false;

	// Build list of all levels.
	Level	= nullptr;
	LevelList.Empty();
	for( Int32 i=0; i<GObjectDatabase->GObjects.Num(); i++ )
	{
		FObject* Object = GObjectDatabase->GObjects[i];
		if( Object && Object->IsA(FLevel::MetaClass) )
			LevelList.Push((FLevel*)Object);
	}

	// Refresh window.
	SetSize
	( 
		Project->Info->DefaultWidth, 
		Project->Info->DefaultHeight, 
		Project->Info->WindowType
	);

	return true;
}


/*-----------------------------------------------------------------------------
    Game initialization.
-----------------------------------------------------------------------------*/

//
// Initialize the game.
//
void CGame::Init( HINSTANCE InhInstance )
{
	// Init window class.
    WNDCLASSEX wcex;
	static Char*	FluWinCls = L"FluEngine_Game";

	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance		= InhInstance;
	wcex.hIcon			= LoadIcon( hInstance, MAKEINTRESOURCE(IDI_FLUICON) );
	wcex.hCursor		= LoadCursor( nullptr, IDC_ARROW );
	wcex.hbrBackground	= nullptr;				
	wcex.lpszMenuName	= nullptr;
	wcex.lpszClassName	= FluWinCls;
	wcex.hIconSm		= LoadIcon( hInstance, MAKEINTRESOURCE(IDI_FLUICON) );

	// Register window.
	if( !RegisterClassEx(&wcex) )
	{	
		log( L"GetLastError %i", GetLastError() );
		error( L"RegisterClassEx failure" );
	}

	// Create the window.
	hWnd	= CreateWindow
	(
		FluWinCls,
		L"Fluorine Game",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 600,
		nullptr,
		nullptr,
		hInstance,
		0
	);
	assert(hWnd);

	// Load ini-file.
	Config		= new CConfigManager(GDirectory);

	// Allocate subsystems.
	GRender		= new COpenGLRender( hWnd );

#if FLU_X64
	GAudio		= new CNullAudio();
#else
	GAudio		= new COpenALAudio();
#endif

	GInput		= new CInput();

	// Set default audio volume.
	GAudio->MasterVolume	= Config->ReadFloat( L"Game", L"Audio",	L"MasterVolume",	1.f );
	GAudio->MusicVolume		= Config->ReadFloat( L"Game", L"Audio",	L"MusicVolume",		1.f );
	GAudio->FXVolume		= Config->ReadFloat( L"Game", L"Audio",	L"FXVolume",		1.f );

	// Show the window.
	ShowWindow( hWnd, /*SW_SHOWNORMAL*/SW_SHOWMAXIMIZED );
	UpdateWindow( hWnd );

	// Allocate console.
	Console			= new CConsole();
	CConsole::Font	= LoadFontFromResource
	(
		hInstance,
		MAKEINTRESOURCE(IDR_FONT2),
		MAKEINTRESOURCE(IDB_FONT2)
	);

	// Notify.
	log( L"Game: Game initialized" );

	// Load game? What way we will use?
	String ProjectFileName = CCmdLineParser::ParseStringParam( GCmdLine, L"project" );
	if( ProjectFileName )
	{
		// Open Command-Line game.
		String FileName	= String::Pos( L":\\", ProjectFileName ) != -1 ? 
							ProjectFileName : 
							GDirectory+L"\\"+ProjectFileName;

		log( L"Game: Loading game from '%s'", *FileName );

		if( !GPlat->FileExists(FileName) )
			error(L"Game file '%s' not found", *FileName);

		String Directory	= GetFileDir(ProjectFileName);
		String Name			= GetFileName(ProjectFileName);

		// Load it.
		LoadGame( Directory, Name );

		// Run some initial level.
		String LevelName = CCmdLineParser::ParseStringParam( GCmdLine, L"level" );
		FLevel* Entry = nullptr;
		if( LevelName )
			Entry = FindLevel( LevelName );
		else
			log( L"Game: Entry level is not specified." );

		if( !Entry && LevelList.Num() )
			Entry = LevelList[0];

		if( Entry )
		{
			RunLevel( Entry, true );
		}
		else
			log( L"Game: Entry level not found!" );
	}
	else
	{
		// Try to find some file in directory.
		TArray<String> GameFiles;
		GameFiles = GPlat->FindFiles(GDirectory, String::Format(L"*%s", PROJ_FILE_EXT));

		if( GameFiles.Num() == 0 )
			error(L"Game file not found");

		String FileName = GameFiles[0];
		log( L"Game: Game file '%s' detected!", *FileName );

		String Directory	= GetFileDir(FileName);
		String Name			= GetFileName(FileName);

		// Load it.
		LoadGame( Directory, Name );

		// Try to find entry level.
		FLevel* Entry = FindLevel( L"Entry" );
		if( !Entry )
		{
			log( L"Game: Entry level not found!" );
			if( LevelList.Num() )
				Entry	= LevelList[0];
		}

		// Run it.
		if( Entry )
			RunLevel( Entry, true );
		else
			log( L"No level's found in '%s'", *FileName );
	}
}


/*-----------------------------------------------------------------------------
    Game tick.
-----------------------------------------------------------------------------*/

//
// Tick game.
//
void CGame::Tick( Float Delta )
{
	CCanvas* Canvas		= GRender->Lock();
	{
		// Tick, things, which need tick.
		if( Level )	
		{
			Level->Tick( Delta );
			GAudio->Tick( Delta, Level );
		}

		if( Project )
			Project->BlockMan->Tick( Delta );

		// Render level.
		if( Level )
			GRender->RenderLevel
			(
				Canvas,
				Level,
				0, 0,
				WinWidth, WinHeight
			);

		// Render console.
		if( Console->IsActive() )
			Console->Render( Canvas );
	}
	GRender->Unlock();

	// Handle level's travel. Play page don't allow to
	// travel, so just notify player about it.
	if( GIncomingLevel )
	{
		RunLevel
		(
			GIncomingLevel.Destination,
			GIncomingLevel.bCopy
		);

		// Unmark it.
		GIncomingLevel.Destination	= nullptr;
		GIncomingLevel.Teleportee	= nullptr;
		GIncomingLevel.bCopy		= false;
	}
}


/*-----------------------------------------------------------------------------
    Game deinitialization.
-----------------------------------------------------------------------------*/

//
// Exit the game.
//
void CGame::Exit()
{
	// Shutdown project, if any.
	if( Project )
	{
		// But first of all - kill current level.
		if( Level )
		{
			assert(Level->bIsPlaying);
			Level->EndPlay();
			if( Level->IsTemporal() )
				DestroyObject( Level );
		}

		// Kill entire project.
		delete Project;
	}

	// Delete console.
	freeandnil(Console);
	freeandnil(CConsole::Font);

	// Shutdown subsystems.
	freeandnil(GInput);
	freeandnil(GAudio);
	freeandnil(GRender);
	freeandnil(Config);

	log( L"Game: Application shutdown" ); 
}


/*-----------------------------------------------------------------------------
    Game main loop.
-----------------------------------------------------------------------------*/

//
// Global timing variables.
//
static Double		GOldTime;
static Double		GfpsTime;
static Int32		GfpsCount;

//
// Exception handle variables.
//
static Char*		GErrorText;


//
// Loop, until exit.
//
void CGame::MainLoop()
{
	// Start time.
	GOldTime		= GPlat->TimeStamp();
	GfpsTime		= 0.0;
	GfpsCount		= 0;
	
	// Entry point.
#ifndef FLU_DEBUG
	__try
#endif
	{
		for( ; ; )
		{	
			// Here we expressly reduce FPS, to not eat too
			// many CPU time.
			if( FPS > 48 )
			{	
				// 175 FPS maximum.
				Sleep(1000 / 175);
			}

			// Compute 'now' time.
			Double Time			= GPlat->TimeStamp();
			Double DeltaTime	= Time - GOldTime;
			GOldTime			= Time;
			GPlat->SetNow( Time );

			// Count FPS stats.
			GFrameStamp++;
			GfpsCount++;
			GfpsTime += DeltaTime;
			if( GfpsTime > 1.0 )
			{
				FPS			= GfpsCount;
				GfpsTime	= 0.0;
				GfpsCount	= 0;	
			}

			// Update the client.
			Tick( (Float)DeltaTime );
			
			// Process joystick input.
			if( !(GFrameStamp & 3) )
				JoystickTick();

			// Process incoming messages.
			MSG	Msg;
			while( PeekMessage( &Msg, 0, 0, 0, PM_REMOVE ) )
			{
				if( Msg.message == WM_QUIT )
					goto ExitLoop;

				TranslateMessage( &Msg );
				DispatchMessage( &Msg );
			}
		} 

	ExitLoop:;
	}
#ifndef FLU_DEBUG
	__except( HandleException(GetExceptionInformation()) )
	{
		// GPF Error.
		error( L"General protection fault in '%s'", GErrorText );
	}
#endif
}


/*-----------------------------------------------------------------------------
    Game WndProc.
-----------------------------------------------------------------------------*/

//
// Process a game window messages.
//
LRESULT CALLBACK WndProc( HWND HWnd, UINT Message, WPARAM WParam, LPARAM LParam )
{
	static Bool bCapture = false;

	switch( Message )
	{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			//
			// Key has been pressed.
			//
			if( !GGame->Console->IsActive() )
				GGame->GInput->OnKeyDown( (Int32)WParam );
			if(  WParam==VK_ESCAPE && GGame->Project && GGame->Project->Info->bQuitByEsc )
				SendMessage( GGame->hWnd, WM_CLOSE, 0, 0 );
			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			//
			// Key has been released.
			//
			if( !GGame->Console->IsActive() )
				GGame->GInput->OnKeyUp( (Int32)WParam );
			break;
		}
		case WM_MOUSEMOVE:
		{
			//
			// Mouse has been moved.
			//
			if( !GGame->Console->IsActive() )
			{
				static Int32 OldX, OldY;
				Int32	X	= GET_X_LPARAM( LParam ), 
						Y	= GET_Y_LPARAM( LParam );

				if( OldX != X || OldY != Y )
				{
					FLevel*	Level		= GGame->Level;
					Float	WinWidth	= GGame->WinWidth;
					Float	WinHeight	= GGame->WinHeight;
					
					if( Level )
					{
						TVector	RealFOV		= Level->Camera.GetFitFOV( WinWidth, WinHeight );
						TVector	CamFOV		= Level->Camera.FOV;
						Float	lx			= 0.f,
								ly			= WinHeight*((RealFOV.Y-CamFOV.Y)/2.f)/RealFOV.Y,
								lw			= WinWidth,
								lh			= WinHeight*(CamFOV.Y/RealFOV.Y);
						Int32	TestX		= X,
								TestY		= Y-ly;

						// Test bounds.
						if( TestX>=0 && TestY>=0 && TestX<lw && TestY<lh )
						{
							// Screen cursor.
							GGame->GInput->MouseX		= TestX;
							GGame->GInput->MouseY		= TestY;

							// World cursor.
							GGame->GInput->WorldCursor	= TViewInfo
							(
								Level->Camera.Location,
								Level->Camera.Rotation,
								RealFOV,
								Level->Camera.Zoom,
								false,
								0.f, 0.f,
								WinWidth, WinHeight
							).Deproject( X, Y );
						}
					}

					OldX	= X;
					OldY	= Y;
				}
			}
			break;
		}
		case WM_SIZE:
		{
			//
			//  Window resizing.
			//
			GGame->WinWidth		= LOWORD(LParam);
			GGame->WinHeight	= HIWORD(LParam);

			GGame->GRender->Resize( GGame->WinWidth, GGame->WinHeight );
			break;
		}
		case WM_CHAR:
		{
			//
			// When user type some char.
			//
#if FLU_CONSOLE
			// Show or hide console.
			if( (Int32)WParam == CON_TOGGLE_BUTTON )
				GGame->Console->ShowToggle();
#endif
			if( GGame->Console->IsActive() )
			{
				if( WParam != CON_TOGGLE_BUTTON )
					GGame->Console->CharType((Char)WParam);
			}
			else
			{
				GGame->GInput->OnCharType( (Char)WParam );
			}
			break;
		}
		case WM_GETMINMAXINFO:
		{
			//
			// Tell minimum window size.
			//
			LPMINMAXINFO MMIPtr	= (LPMINMAXINFO)LParam;

			MMIPtr->ptMinTrackSize.x	= MIN_GAME_X;
			MMIPtr->ptMinTrackSize.y	= MIN_GAME_Y;
			break;
		}
		case WM_DESTROY:
		{
			//
			// When window destroing.
			//
			PostQuitMessage( 0 );
			break;
		}
		case WM_LBUTTONDBLCLK:
		{
			//
			// Left button double click.
			//
			if ( !GGame->Console->IsActive() )
			{
				GGame->GInput->OnKeyDown( KEY_DblClick );
				GGame->GInput->OnKeyUp( KEY_DblClick );
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			//
			// Process mouse wheel event.
			//
			if ( !GGame->Console->IsActive() )
			{
				Int32 SrlDlt	= GET_WHEEL_DELTA_WPARAM(WParam);
				Int32	Times	= Clamp( SrlDlt/120, -5, 5 );
				Int32	Key		= Times > 0 ? KEY_WheelUp : KEY_WheelDown;

				GGame->GInput->WheelScroll	+= SrlDlt;
				for( Int32 i=0; i<Abs(Times); i++ )
				{
					GGame->GInput->OnKeyDown( Key );
					GGame->GInput->OnKeyUp( Key );
				}
			}	
			break;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			//
			// Some mouse button has been released.
			//
			if ( !GGame->Console->IsActive() )
			{
				Int32	Key	=	Message == WM_LBUTTONUP	?	KEY_LButton :
								Message == WM_RBUTTONUP	?	KEY_RButton :
															KEY_MButton;
				GGame->GInput->OnKeyUp( Key );

				if( bCapture && !(	GGame->GInput->KeyIsPressed(KEY_LButton) || 
									GGame->GInput->KeyIsPressed(KEY_RButton)) )
				{
					ReleaseCapture();
					bCapture	= false;
				}
			}
			break;
		}
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		{	
			//
			// Some mouse button has been pressed.
			//
			if ( !GGame->Console->IsActive() )
			{
				Int32	Key	=	Message == WM_LBUTTONDOWN	?	KEY_LButton :
								Message == WM_RBUTTONDOWN	?	KEY_RButton :
																KEY_MButton;
				GGame->GInput->OnKeyDown( Key );

				if( !bCapture )
				{
					SetCapture( HWnd );
					bCapture	= true;
				}
			}
			break;
		}
		case WM_ACTIVATE:
		{
			//
			// Window has activated or deactivated.
			//
			Bool bActive	= LOWORD(WParam) != WA_INACTIVE;

			if( !bActive && GGame->Level && !GGame->Project->Info->bNoPause )
				GGame->Level->bIsPause	= true;
			break;
		}
		default:
		{
			//
			// Default event processing.
			//
			return DefWindowProc( HWnd, Message, WParam, LParam );
		}
	}

	return 0;
}


/*-----------------------------------------------------------------------------
    Levels managment.
-----------------------------------------------------------------------------*/

//
// Run a level. if bCopy then level will duplicated.
// It's in most cases important to not affect source level.
// Will be weird to lost your enemies, coins, bonuses..
// Due to lose in previous game.
//
void CGame::RunLevel( FLevel* Source, Bool bCopy )
{
	assert(Source);

	// Shutdown previous level.
	if( Level )
	{
		assert(Level->bIsPlaying);
		Level->EndPlay();

		// If level is temporal - eliminate it.
		if( Level->IsTemporal() )
			DestroyObject( Level, true );

		Level	= nullptr;
	}

	// Duplicate level, if required.
	if( bCopy )
		Source	= Project->DuplicateLevel(Source);

	// Unload cache.
	Flush();

	// Let's play!
	Level	= Source;
	Level->RndFlags			= RND_Game;
	Level->BeginPlay();
	GInput->SetLevel( Level );

	// Notify.
	log( L"Game: Level '%s' running", *Level->GetName() );
}


//
// Find level by it name. If level not found 
// return nullptr. This function are case insensitive.
//
FLevel* CGame::FindLevel( String LevName )
{
	for( Int32 i=0; i<LevelList.Num(); i++ )
		if(	String::UpperCase(LevelList[i]->GetName()) == 
			String::UpperCase(LevName) )
				return LevelList[i];

	return nullptr;
}


/*-----------------------------------------------------------------------------
    Console commands execution.
-----------------------------------------------------------------------------*/
	
//
// Retrieve the name of the file.
//
static String GetFileName( String FileName )
{
	Int32 i, j;

	for( i=FileName.Len()-1; i>=0; i-- )
		if( FileName[i] == L'\\' )
			break;

	j = String::Pos( L".", FileName );
	return String::Copy( FileName, i+1, j-i-1 );
}


//
// Retrieve the directory of the file.
//
static String GetFileDir( String FileName )
{ 
	Int32 i;
	for( i=FileName.Len()-1; i>=0; i-- )
		if( FileName[i] == L'\\' )
			break;

	return String::Copy( FileName, 0, i );
}


//
// Main console command execution function.
//
void CGame::ConsoleExecute( String CmdLine )
{
	String Token = CCmdLineParser::ParseCommand(CmdLine);

	if( String::CompareText(Token, L"Clr") == 0 )
	{
		// Clear console.
		Console->Clear();
	}
	else if( String::CompareText(Token, L"BlockMan") == 0 )
	{
		// Debug resource manager.
		if( Project )
			Project->BlockMan->DebugManager();
	}
	else if( String::CompareText(Token, L"Quit") == 0 )
	{
		// Shutdown application.
		SendMessage( hWnd, WM_CLOSE, 0, 0 );
	}
	else if( String::CompareText(Token, L"Flush") == 0 )
	{
		// Unload cached data.
		Flush();
	}
	else if( String::CompareText(Token, L"Run") == 0 )
	{
		// Run a level.

		String	LevName = CCmdLineParser::ParseStringParam(CmdLine, L"name");
		FLevel*	Lev		= FindLevel(LevName);
		if( Lev )
			RunLevel( Lev, true );
		else
			log( L"Game: Level '%s' not found.", *LevName );
	}
	else if( String::CompareText(Token, L"Restart") == 0 )
	{
		// Restart a currect level.
		if( Level && Level->IsTemporal() )
		{
			RunLevel( Level->Original, true );
		}
		else
			log( L"Game: Current level is not restartable" );
	}
	else if( String::CompareText(Token, L"Hash") == 0 )
	{
		// Collision hash info.
		if( Level )
			Level->CollHash->DebugHash();
	}
	else if( String::CompareText(Token, L"Render") == 0 )
	{
		// Change render mode.
		if( Level )
			Level->RndFlags	= CCmdLineParser::ParseStringParam(CmdLine, L"mode")==L"editor" ? RND_Editor : RND_Game;
	}
	else if( String::CompareText(Token, L"Lighting") == 0 )
	{
		// Toggle lighting.
		if( Level )
		{
			Bool bOn = CCmdLineParser::ParseBoolParam(CmdLine, L"enabled");
			if( bOn )
				Level->RndFlags |= RND_Lighting;
			else
				Level->RndFlags &= ~RND_Lighting;
		}
	}
	else if( String::CompareText(Token, L"GFX") == 0 )
	{
		// Toggle post effect.
		if( Level )
		{
			Bool bOn = CCmdLineParser::ParseBoolParam(CmdLine, L"enabled");
			if( bOn )
				Level->RndFlags |= RND_Effects;
			else
				Level->RndFlags &= ~RND_Effects;
		}
	}
	else if( String::CompareText(Token, L"Pause") == 0 )
	{
		// Toggle level pause.
		if( Level )
		{
			Level->bIsPause	^= 1;
			Console->LogCallback( Level->bIsPause ? L"Pause On" : L"Pause Off", TCR_Yellow );
		}
	}
	else if( String::CompareText(Token, L"GPF") == 0 )
	{
		// Crash application!
		*((Int32*)0)	= 1720;
	}
	else if( Level && Level->bIsPlaying )
	{
		// Try to execute for entity.
		String Arg = L"???";

		for( Int32 iEntity=0; iEntity<Level->Entities.Num(); iEntity++ )
			Level->Entities[iEntity]->OnProcess( Token, Arg );
	}
	else
	{
		// Bad command.
		log( L"Game: Unrecognized Command '%s'", *Token );
	}
}


/*-----------------------------------------------------------------------------
    Game utility.
-----------------------------------------------------------------------------*/

//
// Set an game window caption.
//
void CGame::SetCaption( String NewCaption )
{
	SetWindowText( hWnd, *NewCaption );
}


//
// Resize an application window.
//
void CGame::SetSize( Int32 NewWidth, Int32 NewHeight, EAppWindowType NewType )
{
	// Device caps.	
	Int32	ScreenWidth		= GetSystemMetrics(SM_CXSCREEN);
	Int32	ScreenHeight	= GetSystemMetrics(SM_CYSCREEN);
	
	// Clamp size.
	NewWidth	= Clamp( NewWidth, MIN_GAME_X, ScreenWidth );
	NewHeight	= Clamp( NewHeight, MIN_GAME_Y, ScreenHeight );

	switch( NewType )
	{
		case WT_Sizeable:
		{
			//
			// Regular sizeable window.
			//
			SetFocus( hWnd );
			SetWindowLong
			(
				hWnd, 
				GWL_STYLE, 
				(WS_OVERLAPPEDWINDOW | GetWindowLong( hWnd, GWL_STYLE )) & ~WS_MAXIMIZE
			);
			SetWindowPos
			( 
				hWnd, 
				HWND_NOTOPMOST, 
				(ScreenWidth-NewWidth)/2, (ScreenHeight-NewHeight)/2, 
				NewWidth, NewHeight, 
				SWP_FRAMECHANGED
			);
			break;
		}
		case WT_Single:
		{
			//
			// Regular non-sizeable window.
			//
			SetFocus( hWnd );
			SetWindowLong
			(
				hWnd, 
				GWL_STYLE, 
				(WS_OVERLAPPEDWINDOW | GetWindowLong( hWnd, GWL_STYLE )) & ~(WS_MAXIMIZE|WS_SIZEBOX|WS_MAXIMIZEBOX)
			);
			SetWindowPos
			( 
				hWnd, 
				HWND_NOTOPMOST, 
				(ScreenWidth-NewWidth)/2, (ScreenHeight-NewHeight)/2, 
				NewWidth, NewHeight, 
				SWP_FRAMECHANGED
			);
			break;
			break;
		}
		case WT_FullScreen:
		{
			//
			// Fullscreen application.
			//
			SetFocus( hWnd );
			SetForegroundWindow( hWnd );
			SetWindowPos
			( 
				hWnd, 
				HWND_TOPMOST, 
				0, 0, 
				0, 0, 
				SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
			);
			SetWindowLong
			(
				hWnd, 
				GWL_STYLE, 
				WS_MAXIMIZE | WS_VISIBLE
			);
			SetWindowPos
			( 
				hWnd, HWND_TOPMOST, 
				0, 0, 
				ScreenWidth, ScreenHeight, 
				SWP_FRAMECHANGED 
			);
			break;
		}	
	}
}


//
// Make instance.
//
static	CWinPlatform	WinPlat;
static	CPlatformBase*	_WinPlatPtr = GPlat = &WinPlat;


/*-----------------------------------------------------------------------------
    CGameDebugOutput.
-----------------------------------------------------------------------------*/

#pragma optimize ( "", off )


//
// Calling stack trace.
//
static Char* StackTrace( LPEXCEPTION_POINTERS InException )	
{
	static Char Text[2048];
	mem::zero( Text, sizeof(Text) );
	DWORD64 Offset64 = 0;
	DWORD Offset	 = 0;

	// Prepare.
	HANDLE	Process		= GetCurrentProcess();
	HANDLE	Thread		= GetCurrentThread();

	// Symbol info.
	SYMBOL_INFO* Symbol	= (SYMBOL_INFO*)mem::alloc( sizeof(SYMBOL_INFO) + 1024 );
	Symbol->SizeOfStruct	= sizeof(SYMBOL_INFO);
	Symbol->MaxNameLen		= 1024;
	DWORD SymOptions		= SymGetOptions();
	SymOptions				|= SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_EXACT_SYMBOLS;
	SymSetOptions( SymOptions );
	SymInitialize( Process, ".", 1 );

	// Line info.
	IMAGEHLP_LINE64 Line;
	mem::zero( &Line, sizeof(IMAGEHLP_LINE64) );
	Line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	// Setup frame info.
	STACKFRAME64 StackFrame;
	mem::zero( &StackFrame, sizeof(STACKFRAME64) );

#if FLU_X32
	if( InException )
	{
		StackFrame.AddrStack.Offset	= InException->ContextRecord->Esp;
		StackFrame.AddrFrame.Offset	= InException->ContextRecord->Ebp;
		StackFrame.AddrPC.Offset	= InException->ContextRecord->Eip;
	}
	else
	{
		__asm
		{ 
		Label: 
			mov dword ptr [StackFrame.AddrStack.Offset], esp
			mov dword ptr [StackFrame.AddrFrame.Offset], ebp
			mov eax, [Label]
			mov dword ptr [StackFrame.AddrPC.Offset], eax
		}
	}
#endif

	StackFrame.AddrPC.Mode		= AddrModeFlat;
	StackFrame.AddrStack.Mode	= AddrModeFlat;
	StackFrame.AddrFrame.Mode	= AddrModeFlat;
	StackFrame.AddrBStore.Mode	= AddrModeFlat;
	StackFrame.AddrReturn.Mode	= AddrModeFlat;
		
	// Walk the stack.
	for( ; ; )
	{
		if( !StackWalk64
					( 
						IMAGE_FILE_MACHINE_I386, 
						Process, 
						Thread, 
						&StackFrame, 
						InException ? InException->ContextRecord : nullptr,
						nullptr, 
						SymFunctionTableAccess64, 
						SymGetModuleBase64, 
						nullptr ) 
					)
			break;

		if( SymFromAddr( Process, StackFrame.AddrPC.Offset, &Offset64, Symbol ) && 
			SymGetLineFromAddr64( Process, StackFrame.AddrPC.Offset, &Offset, &Line ) )
		{
			Char FileName[1024];
			Char FuncName[256];
			mbstowcs( FileName, Line.FileName, 1024 );
			mbstowcs( FuncName, Symbol->Name, 256 );

			// Add to history.
			wcscat( Text, FuncName );
			wcscat( Text, L" <- " );

			// Output more detailed information into log.
			log( L"%s [File: %s][Line: %d]", FuncName, FileName, Line.LineNumber );
		}
	}

	mem::free( Symbol );
	return Text;
}
#pragma optimize ( "", on )


//
// __try..__except exception handler.
//
static Int32 HandleException( LPEXCEPTION_POINTERS InException )
{
	GErrorText = StackTrace( InException );
	return EXCEPTION_EXECUTE_HANDLER;
}


//
// Game debug output.
//
class CGameDebugOutput: public CDebugOutputBase
{
public:
	// Output constructor.
	CGameDebugOutput()
		:	bUseStdConsole(false)
	{
		// Open log file.
		LogFile	= _wfopen( *(String(FLU_NAME)+L".log"), L"w" );

#if FLU_DEBUG
		// Use Std console or VS Output?
		bUseStdConsole	= !IsDebuggerPresent();
		if( bUseStdConsole )
		{
			// Create console.
			_setmode( _fileno(stdout), _O_U16TEXT );
			AllocConsole();
			freopen( "CONOUT$", "w", stdout );
			SetConsoleTitle( L"Fluorine Engine Output" );
			ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		}
#endif
	}

	// Output destructor.
	~CGameDebugOutput()
	{
		if( bUseStdConsole )
			FreeConsole();

		fclose( LogFile );
	}

	//
	// All C++ outputs.
	//

	// Output C++ log message.
	void Logf( ESeverity Severity, Char* Text, ... )
	{
#if FLU_DEBUG
		Char Dest[2048] = {};
		va_list ArgPtr;
		va_start( ArgPtr, Text );
		_vsnwprintf( Dest, arr_len(Dest), Text, ArgPtr );
		va_end( ArgPtr );

		if( GGame && GGame->Console )
			GGame->Console->LogCallback( Dest, TCR_Gray );

		wcscat_s( Dest, L"\n" );


		fwprintf( LogFile, Dest );
#endif
#if FLU_DEBUG
		if( bUseStdConsole )
		{
			static WORD SeverityColors[SVR_MAX] = 
			{
				FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,			// SVR_Trace;
				FOREGROUND_BLUE | FOREGROUND_GREEN,								// SVR_Info;
				FOREGROUND_GREEN,												// SVR_Log;
				FOREGROUND_GREEN | FOREGROUND_RED,								// SVR_Notice;
				FOREGROUND_INTENSITY											// SVR_Debug;
			};

			SetConsoleTextAttribute( ConsoleHandle, SeverityColors[Severity] );
			wprintf( Dest );
		}

		OutputDebugString(Dest);
#endif
	}


	// Show warning message.
	void Warnf( Char* Text, ... )
	{
		Char Dest[2048] = {};
		va_list ArgPtr;
		va_start( ArgPtr, Text );
		_vsnwprintf( Dest, arr_len(Dest), Text, ArgPtr );
		va_end( ArgPtr );

		debug( L"**WARNING: %s", Dest );
		fflush( LogFile );

		MessageBox( 0, Dest, L"Warning", MB_OK | MB_ICONWARNING | MB_TASKMODAL );
	}

	// Raise fatal error.
	void Errorf( Char* Text, ... )
	{
		Char Dest[2048] = {};
		va_list ArgPtr;
		va_start( ArgPtr, Text );
		_vsnwprintf( Dest, arr_len(Dest), Text, ArgPtr );
		va_end( ArgPtr );

		debug( L"**CRITICAL ERROR: %s", Dest );
		String Stack = StackTrace(nullptr);
		UInt32 Footprint = MurmurHash((UInt8*)*Stack, Stack.Len()*sizeof(Char));
		String FullText = String::Format( L"%s\nStack Footprint: 0x%08x\n\nHistory: %s", Dest, Footprint, *Stack );

		if( !IsDebuggerPresent() )
			MessageBox( 0, *FullText, L"Critical Error", MB_OK | MB_ICONERROR | MB_TASKMODAL );

		fflush( LogFile );

		// Exit process or enter debug.
		if( IsDebuggerPresent() )
			DebugBreak();
		else
			ExitProcess( 0 );
	}

	//
	// All FluScript outputs.
	//
	void ScriptLogf( ESeverity Severity, Char* Text, ... )
	{
#if _RELEASE
		// Don't show simple notification in release.
		if( Severity < SVR_Log )
			return;
#endif

		Char Dest[2048] = {};
		va_list ArgPtr;
		va_start( ArgPtr, Text );
		_vsnwprintf( Dest, arr_len(Dest), Text, ArgPtr );
		va_end( ArgPtr );

		if( GGame && GGame->Console )
			GGame->Console->LogCallback( Dest, TCR_Red );

		Logf( SVR_Trace, Dest );
	}
	void ScriptWarnf( Char* Text, ... )
	{
		Char Dest[2048] = {};
		va_list ArgPtr;
		va_start( ArgPtr, Text );
		_vsnwprintf( Dest, arr_len(Dest), Text, ArgPtr );
		va_end( ArgPtr );

		ScriptLogf( SVR_Debug, L"Warning: %s", Dest );
	}
	void ScriptErrorf( Char* Text, ... )
	{
		Char Dest[2048] = {};
		va_list ArgPtr;
		va_start( ArgPtr, Text );
		_vsnwprintf( Dest, arr_len(Dest), Text, ArgPtr );
		va_end( ArgPtr );

		ScriptLogf( SVR_Debug, L"Error: %s", Dest );
	}

private:
	// Output internal.
	Bool	bUseStdConsole;
	HANDLE	ConsoleHandle;

	FILE*	LogFile;
};


// Initialize debug output.
static	CGameDebugOutput	DebugOutput;
static	CDebugOutputBase*	_DebugOutputPtr = GOutput = &DebugOutput;


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/