//-----------------------------------------------------------------------------
//	LogCallbackConsole.h: A console log callback
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	class LogCallbackConsole: public ILogCallback
	{
	public:
		LogCallbackConsole( Bool colored = true )
			:	m_colored( colored )
		{
			_setmode( _fileno(stdout), _O_U16TEXT );
			AllocConsole();
			freopen( "CONOUT$", "w", stdout );
			SetConsoleTitle( L"Fluorine Engine Output" );
			m_handle = GetStdHandle( STD_OUTPUT_HANDLE );
		}

		~LogCallbackConsole()
		{
			FreeConsole();
		}

		void handleMessage( ELogLevel level, const Char* message ) override
		{
			if( m_colored )
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

				SetConsoleTextAttribute( m_handle, color );
			}

			wprintf( L"%s\n", message );
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

			SetConsoleTextAttribute( m_handle, FOREGROUND_RED | FOREGROUND_INTENSITY );
			wprintf( buffer );
		}

		void handleFatalScriptMessage( const Char* message )
		{
			handleMessage( ELogLevel::Error, message );
		}

	private:
		HANDLE m_handle;
		Bool m_colored;
	};
}