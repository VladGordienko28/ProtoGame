//-----------------------------------------------------------------------------
//	LogCallback.h: Log messages handler interface
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
	enum class ELogLevel
	{
		Info,
		Debug,
		Warning,
		Error
	};

	/**
	 * A log messages handler interface
	 */
	class ILogCallback
	{
	public:
		virtual ~ILogCallback() {}

		virtual void handleMessage( ELogLevel level, const Char* message ) = 0;
		virtual void handleScriptMessage( ELogLevel level, const Char* message ) = 0;

		virtual void handleFatalMessage( const Char* message ) = 0;
		virtual void handleFatalScriptMessage( const Char* message ) = 0;
	};
}