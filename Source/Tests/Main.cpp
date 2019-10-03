//-----------------------------------------------------------------------------
//	Main.cpp: Unit Tests main file
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Tests.h"

namespace flu
{
namespace tests
{
	UnitTestInfo g_currentTest;
	UInt64 g_startTimeStamp = 0;
	Int32 g_passedUnitsCount = 0;
	Int32 g_failedUnitsCount = 0;
	Int32 g_skippedUnitsCount = 0;

} // namespace tests
} // namespace flu

class ConsoleOutputCallback: public flu::ILogCallback
{
public:
	ConsoleOutputCallback()
	{
	}

	~ConsoleOutputCallback()
	{
	}

	void handleMessage( ELogLevel level, const Char* message ) override
	{
		fwprintf( stdout, L"%s\n", message );
	}

	void handleScriptMessage( ELogLevel level, const Char* message ) override
	{
		handleMessage( level, message );
	}

	void handleFatalMessage( const Char* message ) override
	{
		fwprintf( stdout, L"[Fatal]: %s\n", message );
		throw nullptr;
	}

	void handleFatalScriptMessage( const Char* message ) override
	{
		handleFatalMessage( message );
	}
};

// Make instance.
static CWinPlatform g_winPlat;
static CPlatformBase* _WinPlatPtr = GPlat = &g_winPlat;

int main( int nArgs, char* args[] )
{
	flu::LogManager::instance().addCallback( new ConsoleOutputCallback() );

	int errorCode = 0;
	tests::g_startTimeStamp = time::cycles64();

	info( L"------ Fluorine Unit Tests Framework ------" );

	for( SizeT i = 0; i < arraySize( tests::g_tests ); i++ )
	{
		try
		{
			tests::g_tests[i]();
		}
		catch( ... )
		{
			tests::g_failedUnitsCount++;
			errorCode = 1;
		}
	}

	info( L"--- %.4f ms elapsed ---", time::elapsedMsFrom( tests::g_startTimeStamp ) );
	info( L"========== Tests: %d passed, %d failed, %d skipped ==========", 
		tests::g_passedUnitsCount, tests::g_failedUnitsCount, tests::g_skippedUnitsCount );

	return errorCode;
}