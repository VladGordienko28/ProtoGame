/*=============================================================================
    FrApp.h: An abstract application class.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    CApplication.
-----------------------------------------------------------------------------*/

//
// An abstract application class.
//
class CApplication
{
public:
	// Global subsystems.
	CAudioBase*			GAudio;
	CInput*				GInput;

	// Project.
	CProject*			Project;

	// Misc.
	Int32				FPS;

	// CApplication interface.
	CApplication();
	virtual ~CApplication();
	virtual void Flush();
	virtual void SetCaption( String NewCaption );
	virtual void ConsoleExecute( String Cmd );

	// Project/Game loading and saving.
	virtual Bool LoadGame( String Directory, String Name );
	virtual Bool SaveGame( String Directory, String Name );
};


//
// Global application instance.
//
extern CApplication*	GApp;


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/