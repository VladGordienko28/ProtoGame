//-----------------------------------------------------------------------------
//	LogManager.h: General logging manager
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 * A singleton Log manager
	 */
	class LogManager
	{
	public:
		static const SizeT MAX_MESSAGE_LENGTH = 4096;

		virtual ~LogManager() {}

		virtual void handleMessage( ELogLevel level, const Char* fmt, ... ) = 0;
		virtual void handleScriptMessage( ELogLevel level, const Char* fmt, ... ) = 0;

		virtual void handleFatalMessage( const Char* fmt, ... ) = 0;
		virtual void handleFatalScriptMessage( const Char* fmt, ... ) = 0;

		virtual void addCallback( ILogCallback* callback ) = 0;
		virtual void removeCallback( ILogCallback* callback ) = 0;

		static LogManager& instance();
	};

	extern const Char* logLevelAsString( ELogLevel level );
	extern const Char* formLogMessage( Char* buffer, SizeT bufferSize, const Char* prefix, const Char* message );
}

// Log macros
#define info( ... )		flu::LogManager::instance().handleMessage( flu::ELogLevel::Info, __VA_ARGS__ );
#define debug( ... )	flu::LogManager::instance().handleMessage( flu::ELogLevel::Debug, __VA_ARGS__ );
#define warn( ... )		flu::LogManager::instance().handleMessage( flu::ELogLevel::Warning, __VA_ARGS__ );
#define error( ... )	flu::LogManager::instance().handleMessage( flu::ELogLevel::Error, __VA_ARGS__ );

#define fatal( ... )	flu::LogManager::instance().handleFatalMessage( __VA_ARGS__ );

#if FLU_ENABLE_ASSERT
	#define assert( expr )	\
	{\
		if( !(expr) )\
			fatal( L"Assertion failed: \"%s\" [File: %hs][Line: %i]", L#expr, __FILE__, __LINE__ );\
	}

	#define verify( expr )	assert( expr )
#else
	#define assert( expr ) {}
	#define verify( expr ) { expr; }
#endif