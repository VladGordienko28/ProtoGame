//-----------------------------------------------------------------------------
//	LogManager.cpp: General logging manager
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Core.h"

#include <stdarg.h>
#include <string.h>

namespace flu
{
	class LogManagerImpl: public LogManager
	{
	public:
		LogManagerImpl();
		~LogManagerImpl();

		void handleMessage( ELogLevel level, const Char* fmt, ... ) override;
		void handleScriptMessage( ELogLevel level, const Char* fmt, ... ) override;

		void handleFatalMessage( const Char* fmt, ... ) override;
		void handleFatalScriptMessage( const Char* fmt, ... ) override;

		void addCallback( ILogCallback* callback ) override;
		void removeCallback( ILogCallback* callback ) override;

	private:
		Array<ILogCallback*> m_callbacks;
	};

	LogManagerImpl::LogManagerImpl()
	{
	}

	LogManagerImpl::~LogManagerImpl()
	{
		for( auto& it : m_callbacks )
		{
			delete it;
			it = nullptr;
		}

		m_callbacks.empty();
	}

#define bufferize_vaargs	\
	Char buffer[MAX_MESSAGE_LENGTH];\
	va_list argsPtr;\
	va_start( argsPtr, fmt );\
	_vsnwprintf_s( buffer, MAX_MESSAGE_LENGTH, _TRUNCATE, fmt, argsPtr );\
	va_end( argsPtr );

	void LogManagerImpl::handleMessage( ELogLevel level, const Char* fmt, ... )
	{
		bufferize_vaargs;
		
		for( auto it : m_callbacks )
		{
			it->handleMessage( level, buffer );
		}
	}

	void LogManagerImpl::handleScriptMessage( ELogLevel level, const Char* fmt, ... )
	{
		bufferize_vaargs;
		
		for( auto it : m_callbacks )
		{
			it->handleScriptMessage( level, buffer );
		}
	}

	void LogManagerImpl::handleFatalMessage( const Char* fmt, ... )
	{
		bufferize_vaargs;
		
		for( auto it : m_callbacks )
		{
			it->handleFatalMessage( buffer );
		}
	}

	void LogManagerImpl::handleFatalScriptMessage( const Char* fmt, ... )
	{
		bufferize_vaargs;
		
		for( auto it : m_callbacks )
		{
			it->handleFatalScriptMessage( buffer );
		}
	}

#undef bufferize_vaargs

	void LogManagerImpl::addCallback( ILogCallback* callback )
	{
		assert( callback );
		m_callbacks.addUnique( callback );
	}

	void LogManagerImpl::removeCallback( ILogCallback* callback )
	{
		assert( callback );
		m_callbacks.removeUnique( callback, true );
	}

	LogManager& LogManager::instance()
	{
		static LogManagerImpl manager;
		return manager;
	}

	const Char* logLevelAsString( ELogLevel level )
	{
		switch( level )
		{
			case ELogLevel::Info:		return L"Info";
			case ELogLevel::Debug:		return L"Debug";
			case ELogLevel::Warning:	return L"Warn";
			case ELogLevel::Error:		return L"Error";
			default:					return L"Unknown";
		}
	}

	const Char* formLogMessage( Char* buffer, SizeT bufferSize, const Char* prefix, const Char* message )
	{
		assert( buffer );

		cstr::cat( buffer, LogManager::MAX_MESSAGE_LENGTH, prefix );
		cstr::cat( buffer, LogManager::MAX_MESSAGE_LENGTH, L": " );
		cstr::cat( buffer, LogManager::MAX_MESSAGE_LENGTH, message );
		cstr::cat( buffer, LogManager::MAX_MESSAGE_LENGTH, L"\n" );

		return buffer;
	}

} // namespace flu