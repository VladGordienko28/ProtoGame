//-----------------------------------------------------------------------------
//	Main.cpp: A Fluorine Shell entry point
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Shell.h"

namespace flu
{
namespace shell
{
	/**
	 *	A shell logging
	 */
	class LogCallbackShell: public ILogCallback
	{
	public:
		static const constexpr Char SHELL_CAPTION[] = TXT("Fluorine Environment Shell");

		LogCallbackShell()
		{
			m_consoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );
			SetConsoleTitle( SHELL_CAPTION );
		}

		~LogCallbackShell()
		{
		}

		void handleMessage( ELogLevel level, const Char* message ) override
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

		void handleScriptMessage( ELogLevel level, const Char* message ) override
		{
			handleMessage( level, message );
		}

		void handleFatalMessage( const Char* message ) override
		{
			SetConsoleTextAttribute( m_consoleHandle, FOREGROUND_RED | FOREGROUND_INTENSITY );

			wprintf( L"FATAL!\n %s\n\n", message );
			wprintf( L"StackTrace:\n %s\n\n", flu::win::stackTrace( nullptr ) );

			system( "pause" );
			ExitProcess( 0 );
		}

		void handleFatalScriptMessage( const Char* message ) override
		{
			handleMessage( ELogLevel::Error, message );
		}

	private:
		HANDLE m_consoleHandle;
	};
}
}

Int32 wmain( Int32 ArgC, Char *ArgV[] )
{
	// legacy
	RegisterAll();

	using namespace flu;
	using namespace flu::shell;

	LogManager::instance().addCallback( new LogCallbackFile( L"Shell.log" ) );
	LogManager::instance().addCallback( new LogCallbackDebug( IsDebuggerPresent() ) );
	LogManager::instance().addCallback( new LogCallbackShell() );

	// say hello to user
	info( L"========================="			);
	info( L"=    Fluorine Engine    ="			);
	info( L"=      %s        =",	FLU_VERSION	);
	info( L"========================="			);
	info( L"" );

	ConfigManager::create( fm::getCurrentDirectory(), TXT("Shell") );

	Int32 returnCode = 0;
	IApp* app = nullptr;

	// select app
	String commandLine = GetCommandLine();
	// todo: add normal cmd line parser here

	if( String::pos( TXT("resource_server"), commandLine ) != -1 )
	{
		app = new ResourceServerApp();
	}
	else
	{
		// ...
	}

	if( app )
	{
		if( app->create( commandLine ) )
		{
			returnCode = app->run();

			if( !app->destroy() )
			{
				error( L"Shell's application shutdown failure" );
				returnCode = 1;
			}
		}
		else
		{
			error( L"Unable to create Shell's app" );
		}

		delete app;
	}
	else
	{
		warn( L"It seems nothing needs to be done" );
		warn( L"Bye :)" );
	}

	ConfigManager::destroy();

	return returnCode;
}