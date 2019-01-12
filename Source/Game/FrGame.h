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
class CGame: public CApplication
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
	TArray<FLevel*>		LevelList;
	CConsole*			Console;
	Integer				WinWidth;
	Integer				WinHeight;

	// CApplication interface.
	void SetCaption( String NewCaption );
	void SetSize( Integer NewWidth, Integer NewHeight, EAppWindowType NewType );
	Bool LoadGame( String Directory, String Name );
	void ConsoleExecute( String Cmd );

	// Game functions.
	void Tick( Float Delta );
	void RunLevel( FLevel* Source, Bool bCopy );
	FLevel* FindLevel( String LevName );
};


//
// Global game instance.
//
extern CGame*	GGame;


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/