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
	// Say hello to user.
	log( L"========================="			);
	log( L"=    Fluorine Engine    ="			);
	log( L"=      %s        =",			FLU_VER	);
	log( L"========================="			);
	log( L"" );

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
	GShell	= nullptr;
}


//
// Shell running.
//
Integer CShell::Run( Integer ArgC, Char* ArgV[] )
{
	//
	// ToDo: Insert main shell work here.
	//

	// Everything fine.
	return 0;
}


//
// Set console title.
//
void CShell::SetCaption( String NewCaption )
{
	SetConsoleTitle(*NewCaption);
}


/*-----------------------------------------------------------------------------
	CGameDebugOutput.
-----------------------------------------------------------------------------*/

//
// Std IO output.
//
class CShellDebugOutput: public CDebugOutputBase
{
public:
	// Output constructor.
	CShellDebugOutput()
	{
#if FDEBUG_LOG
		// Open log file.
		LogFile	= _wfopen( *(String(FLU_NAME)+L".log"), L"w" );
#endif
		ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	}

	// Output destructor.
	~CShellDebugOutput()
	{
#if FDEBUG_LOG
		fclose( LogFile );
#endif
	}


	//
	// All C++ outputs.
	//

	// Output C++ log message.
	void Logf( ESeverity Severity, Char* Text, ... )
	{
		Char Dest[2048] = {};
		va_list ArgPtr;
		va_start( ArgPtr, Text );
		_vsnwprintf( Dest, arr_len(Dest), Text, ArgPtr );
		va_end( ArgPtr );

		static WORD SeverityColors[SVR_MAX] = 
		{
			FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,	// SVR_Trace;
			FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,	// SVR_Info;
			FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,	// SVR_Log;
			FOREGROUND_INTENSITY,									// SVR_Notice;
			FOREGROUND_INTENSITY									// SVR_Debug;
		};

		Print( Dest, SeverityColors[Severity] );
	}

	// Show warning message.
	void Warnf( Char* Text, ... )
	{
		Char Dest[2048] = {};
		va_list ArgPtr;
		va_start( ArgPtr, Text );
		_vsnwprintf( Dest, arr_len(Dest), Text, ArgPtr );
		va_end( ArgPtr );

		Print( Dest, FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED );
#if FDEBUG_LOG
		fflush( LogFile );
#endif
	}

	// Raise fatal error.
	void Errorf( Char* Text, ... )
	{
		Char Dest[2048] = {};
		va_list ArgPtr;
		va_start( ArgPtr, Text );
		_vsnwprintf( Dest, arr_len(Dest), Text, ArgPtr );
		va_end( ArgPtr );

		Print( Dest, FOREGROUND_INTENSITY | FOREGROUND_RED );

#if FDEBUG_LOG
		fflush( LogFile );
#endif  

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
		Char Dest[2048] = {};
		va_list ArgPtr;
		va_start( ArgPtr, Text );
		_vsnwprintf( Dest, arr_len(Dest), Text, ArgPtr );
		va_end( ArgPtr );

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

	void Print( Char* Text, Word Color )
	{
		Char Dest[2048] = {};

		wcscpy( Dest, Text );
		wcscat_s( Dest, L"\n" );

#if FDEBUG_LOG
		fwprintf( LogFile, Dest );
#endif

		SetConsoleTextAttribute( ConsoleHandle, Color );

		wprintf( Dest );
		OutputDebugString( Dest );
	}

private:
	// Output internal.
	HANDLE	ConsoleHandle;

#if FDEBUG_LOG
	FILE* LogFile;
#endif
};


// Initialize debug output.
static	CShellDebugOutput	DebugOutput;
static	CDebugOutputBase*	_DebugOutputPtr = GOutput = &DebugOutput;


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/