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
static Char* StackTrace( LPEXCEPTION_POINTERS InException );
static Int32 HandleException( LPEXCEPTION_POINTERS InException );


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
	// Say hello to user.
	info( L"=========================" );
	info( L"=    Fluorine Engine    =" );
	info( L"=      %s        =", FLU_VERSION );
	info( L"=========================" );
	info( L"" );

	// Initialize global variables.
	GIsEditor			= true;
	GEditor				= this;

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
// Editor destructor.
//
CEditor::~CEditor()
{ 
	GEditor	= nullptr;
}


/*-----------------------------------------------------------------------------
    Editor initialization.
-----------------------------------------------------------------------------*/

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
		error( L"RegisterClassEx failure" );
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

	// Load configure file.
	Config			= new CConfigManager(GDirectory);

	// Create render & audio.
	GRender		= new COpenGLRender( hWnd );

#if FLU_X64
	GAudio		= new CNullAudio();
#else
	GAudio		= new COpenALAudio();
#endif

	GInput		= new CInput();

	// Default for editor audio volume.
	GAudio->MasterVolume	= Config->ReadFloat( L"Editor", L"Audio", L"MasterVolume", 1.f );
	GAudio->MusicVolume		= Config->ReadFloat( L"Editor", L"Audio", L"MusicVolume", 1.f );
	GAudio->FXVolume		= Config->ReadFloat( L"Editor", L"Audio", L"FXVolume", 1.f );

	// Initialize editor GUI.
	GUIRender	= new CGUIRender();
	GUIWindow	= new WWindow();	

	// Load gui resources.
	WWindow::Font1				= LoadFontFromResource( hInstance, MAKEINTRESOURCE(IDR_FONT1), MAKEINTRESOURCE(IDB_FONT1) );
	WWindow::Font2				= LoadFontFromResource( hInstance, MAKEINTRESOURCE(IDR_FONT2), MAKEINTRESOURCE(IDB_FONT2) );
	WWindow::Icons				= LoadBitmapFromResource( hInstance, MAKEINTRESOURCE(IDB_GUIICONS) );
	WWindow::Icons->BlendMode	= BLEND_Masked;

	// Allocate top panels.
	MainMenu		= new WEditorMainMenu( GUIWindow, GUIWindow );
	ToolBar			= new WEditorToolBar( GUIWindow, GUIWindow );

	// Editor status bar.
	StatusBar			= new WStatusBar( GUIWindow, GUIWindow );
	StatusBar->Color	= TColor( 0x00, 0x5b, 0xae, 0xff );
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
	trace( L"Ed: Editor initialized" );


	//Bool bMake = CCmdLineParser::ParseCommand( GCmdLine, 0 ) == L"make";
	//if( bMake )
	{
		String Path = CCmdLineParser::ParseStringParam( GCmdLine, L"path", L"123" );
		if( Path != L"123" )
		{
			OpenProjectFrom( Path );

			// Allocate target directory.
			String Directory = GetFileDir(GProject->FileName) + L"\\Release";
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
	while( EditorPages->Pages.Num() > 0 )
		EditorPages->CloseTabPage( 0, true );

	// Shutdown project, if any.
	if( Project )
		delete Project;

	// Delete GUI, and it resources.
	delete WWindow::Font1;
	delete WWindow::Font2;
	delete WWindow::Icons;

	delete GUIWindow;
	delete GUIRender;

	// Shutdown subsystems.
	freeandnil(GInput);
	freeandnil(GAudio);
	freeandnil(GRender);
	freeandnil(Config);

	trace( L"Ed: Editor shutdown" ); 
}


/*-----------------------------------------------------------------------------
    Editor main loop.
-----------------------------------------------------------------------------*/

//
// Global timing variables.
//
static Double	GOldTime;
static Double	GStartupTime;
static Double	GNowTime;
static Double	GfpsTime;
static Int32	GfpsCount;

//
// Exception handle variables.
//
static Char* GErrorText;


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
	GOldTime		= GPlat->TimeStamp();
	GStartupTime	= GNowTime = GPlat->TimeStamp();
	GPlat->SetNow( GNowTime );
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
			Double Time			= GPlat->TimeStamp();
			Double DeltaTime	= Time - GOldTime;
			GOldTime			= Time;
			GNowTime			= Time - GStartupTime;
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
					*String::Format( L"Save changes to '%s'?", *GProject->ProjName ),
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
			break;
		}
		case WM_CHAR:
		{
			//
			// When user type some char.
			//
			GEditor->GUIWindow->WidgetProc( WPE_CharType, (Char)WParam );
			GEditor->GInput->OnCharType( (Char)WParam );
			break;
		}
		case WM_SIZE:
		{
			//
			//  Window resizing.
			//
			Int32	NewX = LOWORD( LParam ), 
					NewY = HIWORD( LParam );

			GEditor->GRender->Resize( NewX, NewY );
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
		for( Int32 i=0; i<EditorPages->Pages.Num(); i++ )
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
		for( Int32 i=0; i<EditorPages->Pages.Num(); i++ )
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
		for( Int32 i=0; i<EditorPages->Pages.Num(); i++ )
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
		for( Int32 i=0; i<EditorPages->Pages.Num(); i++ )
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
		for( Int32 i=0; i<EditorPages->Pages.Num(); i++ )
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
	for( Int32 iPage=0; iPage<EditorPages->Pages.Num(); iPage++ )
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

#pragma optimize ( "", off )

//
// Calling stack trace.
//
static Char* StackTrace( LPEXCEPTION_POINTERS InException )	
{
	static Char Text[2048];
	MemZero( Text, sizeof(Text) );
	DWORD64 Offset64 = 0;
	DWORD Offset	 = 0;

	// Prepare.
	HANDLE	Process		= GetCurrentProcess();
	HANDLE	Thread		= GetCurrentThread();

	// Symbol info.
	SYMBOL_INFO* Symbol	= (SYMBOL_INFO*)MemAlloc( sizeof(SYMBOL_INFO) + 1024 );
	Symbol->SizeOfStruct	= sizeof(SYMBOL_INFO);
	Symbol->MaxNameLen		= 1024;
	DWORD SymOptions		= SymGetOptions();
	SymOptions				|= SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_EXACT_SYMBOLS;
	SymSetOptions( SymOptions );
	SymInitialize( Process, ".", 1 );

	// Line info.
	IMAGEHLP_LINE64 Line;
	MemZero( &Line, sizeof(IMAGEHLP_LINE64) );
	Line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	// Setup frame info.
	STACKFRAME64 StackFrame;
	MemZero( &StackFrame, sizeof(STACKFRAME64) );
	
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
			debug( L"%s [File: %s][Line: %d]", FuncName, FileName, Line.LineNumber );
		}
	}

	MemFree( Symbol );
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
// Editor debug output.
//
class CEditorDebugOutput: public CDebugOutputBase
{
public:
	// Output constructor.
	CEditorDebugOutput()
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
	~CEditorDebugOutput()
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

		WEditorPage* Page	= GEditor->GetActivePage();
		if( Page && Page->PageType==PAGE_Play )
		{
			WPlayPage* Play = (WPlayPage*)Page;
			Play->AddScriptMessage( Severity, Dest );
		}

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
static	CEditorDebugOutput	DebugOutput;
static	CDebugOutputBase*	_DebugOutputPtr = GOutput = &DebugOutput;


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/