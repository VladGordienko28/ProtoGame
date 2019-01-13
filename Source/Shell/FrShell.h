/*=============================================================================
	FrShell.h: A shell application main class.
	Created by Vlad Gordienko, Apr. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	CShell.
-----------------------------------------------------------------------------*/

//
// A shell application class.
//
class CShell: public CApplication, public ILogCallback
{
public:
	// CShell public interface.
	CShell();
	~CShell();
	Int32 Run( Int32 ArgC, Char* ArgV[] );

	// CApplication interface.
	void SetCaption( String NewCaption ) override;

	// ILogCallback interface
	void handleMessage( ELogLevel level, const Char* message ) override;
	void handleScriptMessage( ELogLevel level, const Char* message ) override;
	void handleFatalMessage( const Char* message ) override;
	void handleFatalScriptMessage( const Char* message ) override;

private:
	// Shell internal.
	HANDLE m_consoleHandle;
};


//
// Global shell instance.
//
extern CShell*	GShell;


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/