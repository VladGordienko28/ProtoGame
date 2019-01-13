/*=============================================================================
	FrShell.cpp: Shell application class.
	Created by Vlad Gordienko, Apr. 2018.
=============================================================================*/

#include "Shell.h"

//
// Globals.
//
CShell*	GShell	= nullptr;


/*-----------------------------------------------------------------------------
	CShell implementation.
-----------------------------------------------------------------------------*/

//
// Shell constructor.
//
CShell::CShell()
	:	CApplication()
{	
	// Initial logging
	m_consoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );

	LogManager::instance().addCallback( new LogCallbackFile( L"Shell.log" ) );
	LogManager::instance().addCallback( new LogCallbackDebug( IsDebuggerPresent() ) );
	LogManager::instance().addCallback( this );

	// Say hello to user.
	info( L"========================="			);
	info( L"=    Fluorine Engine    ="			);
	info( L"=      %s        =",	FLU_VERSION	);
	info( L"========================="			);
	info( L"" );

	SetCaption(L"Fluorine Environment Shell");

	// Initialize global variables.
	GIsEditor	= false;
	GShell		= this;
}


//
// Shell destructor.
//
CShell::~CShell()
{
	LogManager::instance().removeCallback( this );

	GShell	= nullptr;
}


//
// Shell running.
//
Int32 CShell::Run( Int32 ArgC, Char* ArgV[] )
{
	//
	// ToDo: Insert main shell work here.
	//

	warn( L"It's seems like nothing to do" );
	warn( L"Bye :)" );

	// Everything fine.
	return 0;
}


//
// Set console title.
//
void CShell::SetCaption( String NewCaption )
{
	SetConsoleTitle( *NewCaption );
}

//
// Make instance.
//
static	CWinPlatform	WinPlat;
static	CPlatformBase*	_WinPlatPtr = GPlat = &WinPlat;

/*-----------------------------------------------------------------------------
	CGameDebugOutput.
-----------------------------------------------------------------------------*/

void CShell::handleMessage( ELogLevel level, const Char* message )
{
	UInt16 color;

	switch( level )
	{
		case ELogLevel::Info:		color = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED; break;
		case ELogLevel::Debug:		color = FOREGROUND_BLUE | FOREGROUND_GREEN; break;
		case ELogLevel::Warning:	color = FOREGROUND_GREEN | FOREGROUND_RED; break;
		case ELogLevel::Error:		color = FOREGROUND_RED; break;
		default:					color = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
	}

	SetConsoleTextAttribute( m_consoleHandle, color );

	wprintf( L"%s\n", message );
}

void CShell::handleScriptMessage( ELogLevel level, const Char* message )
{
	handleMessage( level, message );
}

void CShell::handleFatalMessage( const Char* message )
{
	SetConsoleTextAttribute( m_consoleHandle, FOREGROUND_RED | FOREGROUND_INTENSITY );

	wprintf( L"FATAL!\n %s\n\n", message );
	wprintf( L"StackTrace:\n %s", flu::win::stackTrace( nullptr ) );

	ExitProcess( 0 );
}

void CShell::handleFatalScriptMessage( const Char* message )
{
	handleMessage( ELogLevel::Error, message );
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/