/*=============================================================================
    FrEditor.h: An editor application main class.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    CEditor.
-----------------------------------------------------------------------------*/

//
// An editor application class.
//
class CEditor: public CApplication, public ILogCallback
{
public:
	// CEditor public interface.
	CEditor();
	~CEditor();
	void Init( HINSTANCE InhInstance );
	void MainLoop();
	void Exit();

	// ILogCallback interface
	void handleMessage( ELogLevel level, const Char* message ) override;
	void handleScriptMessage( ELogLevel level, const Char* message ) override;
	void handleFatalMessage( const Char* message ) override;
	void handleFatalScriptMessage( const Char* message ) override;

public:
	// OS relative variables.
	HINSTANCE				hInstance;
	HWND					hWnd;

	// GUI.
	WWindow*				GUIWindow;
	CGUIRender*				GUIRender;

	// Editor panels.
	WEditorMainMenu*		MainMenu;
	WEditorToolBar*			ToolBar;
	WTabControl*			EditorPages;
	WObjectInspector*		Inspector;
	WResourceBrowser*		Browser;
	WStatusBar*				StatusBar;
	WTaskDialog*			TaskDialog;
	WGameBuilderDialog*		GameBuilder;

	// Editor functions.
	void Tick( Float Delta );
	WEditorPage* OpenPageWith( FResource* InResource );
	WEditorPage* GetActivePage();
	FResource* PreloadResource( String Filename );

	// CApplication interface.
	void SetCaption( String NewCaption );

	// Project functions.
	Bool CloseProject( Bool bAsk = true );
	Bool OpenProject();
	Bool SaveProject();
	Bool SaveAsProject();
	Bool NewProject();
	Bool OpenProjectFrom( String FileName );

	// Script functions.
	Bool CompileAllScripts( IProgressIndicator* Indicator );	
	Bool DropAllScripts();

	// Level functions.
	void BuildPaths( FLevel* Level );
	void DestroyPaths( FLevel* Level );
	WPlayPage* PlayLevel( FLevel* Original );

	// Resource import/export.
	FResource* ImportResource( String Filename, String ResName=L"" );
	Bool ExportResource( FResource* Res, String Directory, Bool bOverride );
};


// Global editor instance.
extern CEditor* GEditor;


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/