//-----------------------------------------------------------------------------
//	LogCallbackFile.h: A file log callback
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	class LogCallbackFile: public ILogCallback
	{
	public:
		LogCallbackFile( String fileName )
			:	m_fileName( fileName )
		{
			_wfopen_s( &m_file, *m_fileName, L"w" );
			
			if( !m_file )
				fatal( L"Unable to open log file '%s'", *fileName );

			m_startupTimeStamp = time::cycles64();
		}

		~LogCallbackFile()
		{
			fclose( m_file );
			m_file = nullptr;
		}

		void handleMessage( ELogLevel level, const Char* message ) override
		{
			Char buffer[LogManager::MAX_MESSAGE_LENGTH] = {};
			
			formLogMessage( buffer, LogManager::MAX_MESSAGE_LENGTH, 
				logLevelAsString( level ), message );

			fwprintf( m_file, L"%06.1f %s", time::elapsedSecFrom( m_startupTimeStamp ), buffer );

			if( level >= ELogLevel::Warning )
			{
				fflush( m_file );
			}
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

			fwprintf( m_file, L"\n" );
			fwprintf( m_file, buffer );
			fflush( m_file );

			fwprintf( m_file, L"\n" );
			fwprintf( m_file, L"Stack Trace: \n" );
			fwprintf( m_file, win::stackTrace( nullptr ) );
			fflush( m_file );
		}

		void handleFatalScriptMessage( const Char* message )
		{
			handleMessage( ELogLevel::Error, message );
		}

	private:
		UInt64 m_startupTimeStamp;
		String m_fileName;
		FILE* m_file;
	};
}