/*=============================================================================
    FrGame.h: A game application main class.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    CGame.
-----------------------------------------------------------------------------*/

//
// A game application class.
//
class CGame: public CApplication, public ILogCallback
{
public:
	// CGame public interface.
	CGame();
	~CGame();
	void Init( HINSTANCE InhInstance );
	void MainLoop();
	void Exit();

public:
	// OS relative variables.
	HINSTANCE			hInstance;
	HWND				hWnd;

	// Game stuff.
	FLevel*				Level;
	Array<FLevel*>		LevelList;
	CConsole*			Console;
	Int32				WinWidth;
	Int32				WinHeight;

	// CApplication interface.
	void SetCaption( String NewCaption );
	void SetSize( Int32 NewWidth, Int32 NewHeight, EAppWindowType NewType );
	Bool LoadGame( String Directory, String Name );
	void ConsoleExecute( String Cmd );

	// Game functions.
	void Tick( Float Delta );
	void RunLevel( FLevel* Source, Bool bCopy );
	FLevel* FindLevel( String LevName );

	// ILogCallback interface
	void handleMessage( ELogLevel level, const Char* message ) override;
	void handleScriptMessage( ELogLevel level, const Char* message ) override;
	void handleFatalMessage( const Char* message ) override;
	void handleFatalScriptMessage( const Char* message ) override;
};


//
// Global game instance.
//
extern CGame*	GGame;


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/