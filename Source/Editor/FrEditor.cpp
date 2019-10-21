/*=============================================================================
    FrEditor.cpp: Editor application class.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"
#include "Res\resource.h"
#include "FrSplash.h"

/*-----------------------------------------------------------------------------
    Editor declarations.
-----------------------------------------------------------------------------*/

//
// Editor window minimum size.
//
#define MIN_EDITOR_X		800
#define MIN_EDITOR_Y		600


//
// Forward declaration.
//
static LRESULT CALLBACK WndProc( HWND HWnd, UINT Message, WPARAM WParam, LPARAM LParam );
static Int32 handleException( LPEXCEPTION_POINTERS exception );


//
// Globals.
//
CEditor*	GEditor	= nullptr;


/*-----------------------------------------------------------------------------
    CEditor implementation.
-----------------------------------------------------------------------------*/

//
// Editor pre-initialization.
//
CEditor::CEditor()
	:	CApplication()
{
	// Initial logging
	LogManager::instance().addCallback( new LogCallbackFile( L"Editor.log" ) );

#if FLU_DEBUG
	LogManager::instance().addCallback( new LogCallbackDebug( true ) );

	if( !IsDebuggerPresent() )
	{
		LogManager::instance().addCallback( new LogCallbackConsole() );
	}
#endif

	LogManager::instance().addCallback( this );

	// Say hello to user.
	info( L"=========================" );
	info( L"=    Fluorine Engine    =" );
	info( L"=      %s        =", FLU_VERSION );
	info( L"=========================" );
	info( L"" );

	// Initialize global variables.
	GIsEditor			= true;
	GEditor				= this;

	// Parse command line.
	GCmdLine = GetCommandLine();

	// Initialize C++ stuff.
	srand(GetTickCount() ^ 0x20162016);
}


//
// Editor destructor.
//
CEditor::~CEditor()
{
	LogManager::instance().removeCallback( this );

	GEditor	= nullptr;
}


/*-----------------------------------------------------------------------------
    Editor initialization.
-----------------------------------------------------------------------------*/

class ResourceListener: public res::IListener
{
public:
	ResourceListener() = default;
	~ResourceListener() = default;

	void onError( String resourceName, String message )
	{
		error( L"%s: %s", *resourceName, *message );
	}

	void onWarning( String resourceName, String message )
	{
		warn( L"%s: %s", *resourceName, *message );
	}

	void onInfo( String resourceName, String message )
	{
		info( L"%s: %s", *resourceName, *message );
	}
};

ResourceListener g_resListener;

//
// Initialize the editor.
//
void CEditor::Init( HINSTANCE InhInstance )
{	
	// Init window class.
    WNDCLASSEX wcex;
	static Char*	FluWinCls = L"FluEngine_App";

	wcex.cbSize			= sizeof( WNDCLASSEX );
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance		= InhInstance;
	wcex.hIcon			= LoadIcon( hInstance, MAKEINTRESOURCE(IDI_EDICON) );
	wcex.hCursor		= LoadCursor( nullptr, IDC_ARROW );
	wcex.hbrBackground	= nullptr;				
	wcex.lpszMenuName	= nullptr;
	wcex.lpszClassName	= FluWinCls;
	wcex.hIconSm		= LoadIcon( hInstance, MAKEINTRESOURCE(IDI_EDICON) );
	
	// Show splash! To user!
	CSplash Splash(MAKEINTRESOURCE(IDB_SPLASH));		

	// Register window.
	if( !RegisterClassEx(&wcex) )
	{	
		debug( L"GetLastError %i", GetLastError() );
		fatal( L"RegisterClassEx failure" );
	}

	// Create the window.
	hWnd	= CreateWindow
	(
		FluWinCls,
		L"Fluorine Engine",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 600,
		nullptr,
		nullptr,
		hInstance,
		0
	);
	assert(hWnd);

	ConfigManager::create( fm::getCurrentDirectory(), L"Editor" );

	// Load configure file.
	m_renderDevice = new dx11::Device( hWnd, 800, 600, false );
	m_inputDevice = new in::Device();

	m_world = new World( m_renderDevice.get(), m_inputDevice.get() );

	res::ResourceManager::addListener( &g_resListener );



	m_legacyRender			= new CDirectX11Render( m_renderDevice.get(), m_world->drawContext() );



#if FLU_X64
	GAudio		= new CNullAudio();
#else
	GAudio		= new COpenALAudio();
#endif

	GInput		= new CInput();

	// Default for editor audio volume.
	//GAudio->MasterVolume	= Config->ReadFloat( L"Editor", L"Audio", L"MasterVolume", 1.f );
	//GAudio->MusicVolume		= Config->ReadFloat( L"Editor", L"Audio", L"MusicVolume", 1.f );
	//GAudio->FXVolume		= Config->ReadFloat( L"Editor", L"Audio", L"FXVolume", 1.f );



///////////



	// Initialize editor GUI.
	GUIRender	= new CGUIRender();
	GUIWindow	= new WWindow();	


	WWindow::Icons = res::ResourceManager::get<img::Image>( L"System.Editor.GuiIcons", res::EFailPolicy::FATAL );
	WWindow::Font1 = res::ResourceManager::get<fnt::Font>( L"Fonts.RopaSans_9", res::EFailPolicy::FATAL ); // no no no no
	WWindow::Font2 = res::ResourceManager::get<fnt::Font>( L"Fonts.Consolas_9", res::EFailPolicy::FATAL ); // no no no no
	//WWindow::Font2 = res::ResourceManager::get<fnt::Font>( L"Fonts.Consolas_9", res::EFailPolicy::FATAL );

	//res::ResourceManager::generatePackages();

	// Allocate top panels.
	MainMenu		= new WEditorMainMenu( GUIWindow, GUIWindow );
	ToolBar			= new WEditorToolBar( GUIWindow, GUIWindow );

	// Editor status bar.
	StatusBar			= new WStatusBar( GUIWindow, GUIWindow );
	StatusBar->Color	= math::Color( 0x00, 0x5b, 0xae, 0xff );
	StatusBar->AddPanel( L"", 150, SPS_Left );
	StatusBar->AddPanel( L"", 150, SPS_Left );

	// Splitter for right panel and others.
	WHSplitBox* SplitBox2 = new WHSplitBox( GUIWindow, GUIWindow );
	SplitBox2->RightMin = 250;
	SplitBox2->RightMax = 450;
	SplitBox2->LeftMin = 230;
	SplitBox2->RatioRule = HRR_PreferRight;
	SplitBox2->Align = AL_Client;

	// Splitter for pages and resource browser.
	WHSplitBox* SplitBox1 = new WHSplitBox( SplitBox2, GUIWindow );
	SplitBox1->RatioRule = HRR_PreferLeft;
	SplitBox1->LeftMin = 187;
	SplitBox1->LeftMax = 550;
	SplitBox1->RightMin = 60;

	// Left panel.
	Browser		= new WResourceBrowser( SplitBox1, GUIWindow );

	// Editor pages.
	EditorPages			= new WTabControl( SplitBox1, GUIWindow );

	// Right panel.
	Inspector	= new WObjectInspector( SplitBox2, GUIWindow );

	// Editor dialogs.
	TaskDialog		= new WTaskDialog( GUIWindow );
	GameBuilder		= new WGameBuilderDialog( GUIWindow );

	// Show 'Welcome page'
	EditorPages->AddTabPage(new WHelloPage( EditorPages, GUIWindow ));

	// Show the window.
	ShowWindow( hWnd, SW_SHOWMAXIMIZED );
	UpdateWindow( hWnd );

	// Notify.
	info( L"Ed: Editor initialized" );


	//Bool bMake = CCmdLineParser::ParseCommand( GCmdLine, 0 ) == L"make";
	//if( bMake )
	{
		String Path = CCmdLineParser::ParseStringParam( GCmdLine, L"path", L"123" );
		if( Path != L"123" )
		{
			OpenProjectFrom( Path );

			// Allocate target directory.
			String Directory = fm::getFilePath( *GProject->FileName ) + L"\\Release";
			CreateDirectory( *Directory, nullptr );


			// Now save project.
			if( !GEditor->SaveGame( Directory, GProject->ProjName ) )
			{
				GUIWindow->ShowMessage( L"Couldn't save project", L"Project", true );
				return;
			}


			ExitProcess(0);
		}


	}
}


/*-----------------------------------------------------------------------------
    Editor deinitialization.
-----------------------------------------------------------------------------*/

//
// Exit the editor.
//
void CEditor::Exit()
{
	// Close all pages.
	while( EditorPages->Pages.size() > 0 )
		EditorPages->CloseTabPage( 0, true );

	// Shutdown project, if any.
	if( Project )
		delete Project;

	// Delete GUI, and it resources.
	WWindow::Font1 = nullptr;
	WWindow::Font2 = nullptr;
	WWindow::Icons = nullptr;


	delete GUIWindow;
	delete GUIRender;

	res::ResourceManager::removeListener( &g_resListener );

	// Shutdown subsystems.
	freeandnil(GInput);
	freeandnil(GAudio);


	delete m_legacyRender;


	m_world = nullptr;
	m_renderDevice = nullptr;

	net::NetworkManager::destroy();

	ConfigManager::destroy();

	info( L"Ed: Editor shutdown" ); 
}


/*-----------------------------------------------------------------------------
    Editor main loop.
-----------------------------------------------------------------------------*/

//
// Global timing variables.
//
static UInt64	GOldTimeStamp;
static Double	GStartupTime;
static Double	GNowTime;
static Double	GfpsTime;
static Int32	GfpsCount;

//
// Exception stack trace
//
static Char* g_exceptionStackTrace;


//
// Loop, until exit.
//
void CEditor::MainLoop()
{
	// WinApi handlers.
	DWORD	ThreadId	= GetCurrentThreadId();
	HANDLE	hThread		= GetCurrentThread();
	Bool	bInFocus	= true;

	// Initial timing.
	GOldTimeStamp	= time::cycles64();
	GStartupTime	= GNowTime = time::cyclesToSec( time::cycles64() );
	GPlat->SetNow( GNowTime );
	GfpsTime		= 0.0;
	GfpsCount		= 0;

	// Entry point.
#if !FLU_DEBUG
	__try
#endif
	{
		for( ; ; )
		{
			// Here we expressly reduce FPS, to not eat too
			// many CPU time.
			if( FPS > 48 )
			{
				// Select according to page type.
				static const UInt32 GPageLim[PAGE_MAX] =
				{
					1000 / 100,		// PAGE_None.
					1000 / 100,		// PAGE_Hello.
					1000 / 150,		// PAGE_Bitmap.
					1000 / 160,		// PAGE_Level.
					1000 / 100,		// PAGE_Animation.
					1000 / 120,		// PAGE_Script.
					1000 / 175,		// PAGE_Play.
					1000 / 100		// PAGE_Skeleton.
				};

				WEditorPage* Page = GetActivePage();
				Sleep( GPageLim[Page ? Page->PageType : PAGE_None] );
			}
		
			// Compute 'now' time.
			UInt64 TimeStamp	= time::cycles64();
			Double DeltaTime	= time::cyclesToSec( TimeStamp - GOldTimeStamp );
			GOldTimeStamp		= TimeStamp;
			GNowTime			= time::cyclesToSec( TimeStamp ) - GStartupTime;
			GPlat->SetNow( GNowTime );

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


			profile_begin_frame();
			{
				// Update the editor.
				Tick( (Float)DeltaTime );

				// Process incoming messages.
				MSG	Msg;
				while( PeekMessage( &Msg, 0, 0, 0, PM_REMOVE ) )
				{
					if( Msg.message == WM_QUIT )
						goto ExitLoop;

					TranslateMessage( &Msg );
					DispatchMessage( &Msg );
				}

				// Reduce process priority, if editor not in focus.
				if( !(GFrameStamp & 127) )
				{
					Bool	Focused	= GetWindowThreadProcessId(GetForegroundWindow(), nullptr) == ThreadId;
					if( bInFocus && !Focused )
					{
						// Reduce priority.
						SetThreadPriority( hThread, THREAD_PRIORITY_BELOW_NORMAL );
					}
					else if( !bInFocus && Focused )
					{
						// Restore normal priority.
						SetThreadPriority( hThread, THREAD_PRIORITY_NORMAL );
					}
					bInFocus	= Focused;
				}
			}
			profile_end_frame();
		} 
		
	ExitLoop:;
	}
#if !FLU_DEBUG
	__except( handleException(GetExceptionInformation()) )
	{
		// GPF Error.
		fatal( L"General protection fault in '%s'", g_exceptionStackTrace );
	}
#endif
}


/*-----------------------------------------------------------------------------
    Editor WndProc.
-----------------------------------------------------------------------------*/

//
// Process an editor window messages.
//
LRESULT CALLBACK WndProc( HWND HWnd, UINT Message, WPARAM WParam, LPARAM LParam )
{
	static Bool bCapture = false;

	switch( Message )
	{
		case WM_MOUSEMOVE:
		{
			//
			// Mouse has been moved.
			//
			static Int32 OldX, OldY;
			Int32	X	= GET_X_LPARAM( LParam ), 
					Y	= GET_Y_LPARAM( LParam );

			EMouseButton Button	=	WParam == MK_RBUTTON ? MB_Right : WParam == MK_LBUTTON ? MB_Left :
									WParam == MK_MBUTTON ? MB_Middle : MB_None;

			if( OldX != X || OldY != Y )
			{
				GEditor->GUIWindow->WidgetProc( WPE_MouseMove, TWidProcParms( Button, X, Y ) );
				OldX	= X;
				OldY	= Y;
			}

			// Handle cursor mode.
			if( GEditor->GUIWindow->GetCursorMode() == CM_Wrap )
			{
				Int32 ClientY = GEditor->GUIWindow->Size.Height;

				Int32 NewY=Y;
				if( Y<=0 )			NewY = ClientY-1;
				if( Y>=ClientY )	NewY = 1;

				if( NewY != Y )
				{
					POINT P = { X, NewY }; 
					ClientToScreen( GEditor->hWnd, &P );
					SetCursorPos( P.x, P.y );
					OldY		= NewY;
				}
			}
			break;
		}
		case WM_SETCURSOR:
		{
			//
			// Prevent cursor override by the system
			//
			static const LPCTSTR CursorRemap[CR_MAX] =
			{
				IDC_ARROW,							// CR_Arrow.
				IDC_CROSS,							// CR_Cross.
				IDC_HAND,							// CR_HandPoint
				IDC_IBEAM,							// CR_IBeam.
				IDC_SIZEALL,						// CR_SizeAll.
				IDC_SIZENS,							// CR_SizeNS.
				IDC_SIZEWE,							// CR_SizeWE.
				IDC_SIZENESW,						// CR_SizeNESW.
				IDC_SIZENWSE,						// CR_SizeNWSE.
				IDC_SIZEWE,							// CR_HSplit.
				IDC_SIZENS,							// CR_VSplit.
				MAKEINTRESOURCE(IDC_DRAGITEM),		// CR_Drag.
				MAKEINTRESOURCE(IDC_DRAGITEM),		// CR_MultiDrag.
				MAKEINTRESOURCE(IDC_NODROP),		// CR_NoDrop.
				IDC_WAIT,							// CR_HourGlass.
				IDC_NO								// CR_No.
			};

			ECursorStyle Style = GEditor->GUIWindow->GetDrawCursor();
			SetCursor
			(
				LoadCursor
				( 
					(Style>=CR_Drag && Style<=CR_NoDrop) ? GEditor->hInstance : 0, 
					CursorRemap[Style] 
				)
			);

			// todo: needs better cursor handling
			if( Style == ECursorStyle::CR_Arrow )
				return DefWindowProc( HWnd, Message, WParam, LParam );

			break;
		}
		case WM_CLOSE:
		{
			//
			// When user close project.
			//
			if( GProject )
			{
				int S	= MessageBox
				(
					HWnd,
					*String::format( L"Save changes to '%s'?", *GProject->ProjName ),
					L"Fluorine Engine",
					MB_YESNOCANCEL | MB_TASKMODAL | MB_ICONQUESTION
				);
				if( S==IDYES )
				{
					if( !GEditor->SaveProject() )
						break;
				}
				else if( S == IDCANCEL )
					break;
			}
			
			DestroyWindow(HWnd);
			break;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			//
			// Some mouse button has been released.
			//
			Int32	X	= LOWORD( LParam ), 
					Y	= HIWORD( LParam );
			EMouseButton Button =	Message == WM_LBUTTONUP ? MB_Left : 
									Message == WM_RBUTTONUP ? MB_Right : MB_Middle;

			GEditor->GUIWindow->WidgetProc( WPE_MouseUp, TWidProcParms( Button, X, Y ) );

			if( bCapture && !(GEditor->GUIWindow->bLMouse || GEditor->GUIWindow->bRMouse ))
			{
				ReleaseCapture(); 
				bCapture	= false;
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
			Int32	X	= LOWORD( LParam ), 
					Y	= HIWORD( LParam );
			EMouseButton Button =	Message == WM_LBUTTONDOWN ? MB_Left : 
									Message == WM_RBUTTONDOWN ? MB_Right : MB_Middle;

			GEditor->GUIWindow->WidgetProc( WPE_MouseDown, TWidProcParms( Button, X, Y ) );

			if( !bCapture )
			{
				SetCapture( HWnd );
				bCapture	= true;
			}
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
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			//
			// Key has been released.
			//
			GEditor->GUIWindow->WidgetProc( WPE_KeyUp, (Int32)WParam );
			GEditor->GInput->OnKeyUp( (Int32)WParam );
			GEditor->m_inputDevice->onKeyboardUp( (in::EKeyboardButton)WParam );
			break;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			//
			// Key has been pressed.
			//
			GEditor->GUIWindow->WidgetProc( WPE_KeyDown, (Int32)WParam );
			GEditor->GInput->OnKeyDown( (Int32)WParam );
			GEditor->m_inputDevice->onKeyboardDown( (in::EKeyboardButton)WParam );

			if( WParam == KEY_F4 )
			{
				mem::dumpAllocations( L"RuntimeMemoryDump.txt" );
			}

			break;
		}
		case WM_CHAR:
		{
			//
			// When user type some char.
			//
			GEditor->GUIWindow->WidgetProc( WPE_CharType, (Char)WParam );
			GEditor->GInput->OnCharType( (Char)WParam );
			GEditor->m_inputDevice->onKeyboardType( (Char)WParam );
			break;
		}
		case WM_SIZE:
		{
			//
			//  Window resizing.
			//
			Int32	NewX = LOWORD( LParam ), 
					NewY = HIWORD( LParam );

			NewX = clamp( NewX,  1, 3000 );
			NewY = clamp( NewY, 1, 3000 );

			GEditor->m_renderDevice->resize( NewX, NewY, false );
			GEditor->m_world->onResize( NewX, NewY, false );

			GEditor->GUIWindow->Size = TSize( NewX, NewY );
			GEditor->GUIWindow->WidgetProc( WPE_Resize, TWidProcParms() );
			break;
		}
		case WM_LBUTTONDBLCLK:
		{
			//
			// Left button double click.
			//
			Int32	X	= LOWORD(LParam), 
					Y	= HIWORD(LParam);
	
			GEditor->GUIWindow->WidgetProc( WPE_MouseDown,	TWidProcParms( MB_Left, X, Y ) );
			GEditor->GUIWindow->WidgetProc( WPE_DblClick,	TWidProcParms( MB_Left, X, Y ) );
			break;
		}
		case WM_MOUSEWHEEL:
		{
			//
			// Process mouse wheel event.
			//
			Int32 SrlDlt = GET_WHEEL_DELTA_WPARAM( WParam );
			GEditor->GUIWindow->WidgetProc( WPE_MouseScroll, SrlDlt );
			GEditor->m_inputDevice->onMouseScroll( SrlDlt );
			break;
		}
		case WM_GETMINMAXINFO:
		{
			//
			// Tell minimum window size.
			//
			LPMINMAXINFO MMIPtr	= (LPMINMAXINFO)LParam;

			MMIPtr->ptMinTrackSize.x	= MIN_EDITOR_X;
			MMIPtr->ptMinTrackSize.y	= MIN_EDITOR_Y;
			break;
		}
		case WM_ACTIVATE:
		{
			//
			// Window has activated or deactivated.
			//
			Bool bActive	= LOWORD(WParam) != WA_INACTIVE;

			if( !bActive )
				GEditor->GUIWindow->SetFocused(nullptr);
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
	Various editor functions.
-----------------------------------------------------------------------------*/

//
// Open or restore page with appropriate resource.
// Tries to find existed, if not found allocates new one.
//
WEditorPage* CEditor::OpenPageWith( FResource* InRes )
{
	assert(InRes);

	if( InRes->IsA(FLevel::MetaClass) )
	{
		//
		// Level page.
		//
		WEditorPage* Page = nullptr;
		for( Int32 i=0; i<EditorPages->Pages.size(); i++ )
		{
			WEditorPage* Test = (WEditorPage*)EditorPages->Pages[i];			
			if( Test->PageType==PAGE_Level && ((WLevelPage*)Test)->Level == InRes )	
			{
				Page = Test;
				break;
			}
		}
		if( !Page )
		{
			Page = new WLevelPage( (FLevel*)InRes, EditorPages, GUIWindow );
			EditorPages->AddTabPage(Page);
		}
		else
			EditorPages->ActivateTabPage(Page);
		return Page;
	}
	else if( InRes->IsA(FTexture::MetaClass) )
	{
		//
		// Texture page.
		//
		WEditorPage* Page = nullptr;	
		for( Int32 i=0; i<EditorPages->Pages.size(); i++ )
		{
			WEditorPage* Test = (WEditorPage*)EditorPages->Pages[i];
			if( Test->PageType==PAGE_Texture && ((WTexturePage*)Test)->Texture == InRes )
			{
				Page = Test;
				break;	
			}
		}
		if( !Page )
		{
			Page = new WTexturePage( (FTexture*)InRes, EditorPages, GUIWindow );
			EditorPages->AddTabPage( Page );
		}
		else
			EditorPages->ActivateTabPage( Page );	
		return Page;
	}
	else if( InRes->IsA(FScript::MetaClass) )
	{
		//
		// Script page.
		//
		WEditorPage* Page = nullptr;
		for( Int32 i=0; i<EditorPages->Pages.size(); i++ )
		{
			WEditorPage* Test = (WEditorPage*)EditorPages->Pages[i];
			if( Test->PageType==PAGE_Script && ((WScriptPage*)Test)->Script == InRes )
			{
				Page = Test;
				break;	
			}
		}
		if( !Page )
		{
			Page = new WScriptPage( (FScript*)InRes, EditorPages, GUIWindow );
			EditorPages->AddTabPage( Page );
		}
		else
			EditorPages->ActivateTabPage( Page );
		return Page;
	}
	else if( InRes->IsA(FAnimation::MetaClass) )
	{
		//
		// Animation page.
		//
		WEditorPage* Page = nullptr;
		for( Int32 i=0; i<EditorPages->Pages.size(); i++ )
		{
			WEditorPage* Test = (WEditorPage*)EditorPages->Pages[i];
			if( Test->PageType==PAGE_Animation && ((WAnimationPage*)Test)->Animation == InRes )
			{	
				Page = Test;
				break;
			}
		}
		if( !Page )
		{
			Page = new WAnimationPage( (FAnimation*)InRes, EditorPages, GUIWindow );
			EditorPages->AddTabPage( Page );
		}
		else
			EditorPages->ActivateTabPage( Page );
		return Page;
	}
	else if( InRes->IsA(FSkeleton::MetaClass) )
	{
		//
		// Skeleton page.
		//
		WEditorPage* Page = nullptr;
		for( Int32 i=0; i<EditorPages->Pages.size(); i++ )
		{
			WEditorPage* Test = (WEditorPage*)EditorPages->Pages[i];
			if( Test->PageType==PAGE_Skeleton && ((WSkeletonPage*)Test)->Skeleton == InRes )
			{	
				Page = Test;
				break;
			}
		}
		if( !Page )
		{
			Page = new WSkeletonPage( (FSkeleton*)InRes, EditorPages, GUIWindow );
			EditorPages->AddTabPage( Page );
		}
		else
			EditorPages->ActivateTabPage( Page );
		return Page;
	}
}


//
// Launch a level to play on it, or just restore 
// page, if it was created.
//
WPlayPage* CEditor::PlayLevel( FLevel* Original )
{
	WPlayPage*	Played	= nullptr;

	// Maybe page already loaded.
	for( Int32 iPage=0; iPage<EditorPages->Pages.size(); iPage++ )
	{
		WEditorPage* Test = (WEditorPage*)EditorPages->Pages[iPage];
		if( Test->PageType == PAGE_Play )
			if( ((WPlayPage*)Test)->SourceLevel == Original )
			{
				Played	= (WPlayPage*)Test;
				break;
			}
	}

	if( Played )
	{
		// Yes, page found, restore it.
		EditorPages->ActivateTabPage( Played );
	}
	else
	{
		// Create new page.
		Played = new WPlayPage
		( 
			Original, 
			ToolBar->PlayModeCombo->ItemIndex==0 ? PLAY_Debug : PLAY_Release, 
			EditorPages, 
			GUIWindow 
		);

		EditorPages->AddTabPage( Played );
	}

	return Played;
}


//
// Return current editor page, or nullptr if 
// no active page.
//
WEditorPage* CEditor::GetActivePage()
{
	return (WEditorPage*)EditorPages->GetActivePage();
}


//
// Set an editor window caption.
//
void CEditor::SetCaption( String NewCaption )
{
	SetWindowText( hWnd, *NewCaption );
}


//
// Make instance.
//
static	CWinPlatform	WinPlat;
static	CPlatformBase*	_WinPlatPtr = GPlat = &WinPlat;


/*-----------------------------------------------------------------------------
    CEditorDebugOutput implementation.
-----------------------------------------------------------------------------*/

// __try..__except exception handler.
static Int32 handleException( LPEXCEPTION_POINTERS exception )
{
	g_exceptionStackTrace = flu::win::stackTrace( exception );
	return EXCEPTION_EXECUTE_HANDLER;
}

void CEditor::handleMessage( ELogLevel level, const Char* message )
{
}

void CEditor::handleScriptMessage( ELogLevel level, const Char* message )
{
}

void CEditor::handleFatalMessage( const Char* message )
{
	Char buffer[4096] = {};

	cstr::concat( buffer, arraySize(buffer), L"Fatal Error: \"" );
	cstr::concat( buffer, arraySize(buffer), message );
	cstr::concat( buffer, arraySize(buffer), L"\" in " );
	cstr::concat( buffer, arraySize(buffer), flu::win::stackTrace( nullptr ) );


	MessageBox( 0, buffer, L"Critical Error", MB_OK | MB_ICONERROR | MB_TASKMODAL );
	ExitProcess( 0 );
}

void CEditor::handleFatalScriptMessage( const Char* message )
{
}

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/