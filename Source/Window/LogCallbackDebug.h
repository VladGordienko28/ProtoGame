//-----------------------------------------------------------------------------
//	LogCallbackDebug.h: A debug output log callback
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	class LogCallbackDebug: public ILogCallback
	{
	public:
		LogCallbackDebug( Bool breakDebug )
			:	m_breakDebug( breakDebug )
		{
		}

		~LogCallbackDebug()
		{
		}

		void handleMessage( ELogLevel level, const Char* message ) override
		{
			Char buffer[LogManager::MAX_MESSAGE_LENGTH] = {};
			
			formLogMessage( buffer, LogManager::MAX_MESSAGE_LENGTH, 
				logLevelAsString( level ), message );

			OutputDebugString( buffer );
		}

		void handleScriptMessage( ELogLevel level, const Char* message )
		{
			handleMessage( level, message );
		}

		void handleFatalMessage( const Char* message )
		{
			Char buffer[LogManager::MAX_MESSAGE_LENGTH] = {};
			
			formLogMessage( buffer, LogManager::MAX_MESSAGE_LENGTH, 
				L"FATAL", message );

			OutputDebugString( buffer );

			if( m_breakDebug && IsDebuggerPresent() )
			{
				DebugBreak();
			}
		}

		void handleFatalScriptMessage( const Char* message )
		{
			handleMessage( ELogLevel::Error, message );
		}

	private:
		Bool m_breakDebug;
	};
}