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
class CShell: public CApplication
{
public:
	// CShell public interface.
	CShell();
	~CShell();
	Int32 Run( Int32 ArgC, Char* ArgV[] );

	// CApplication interface.
	void SetCaption( String NewCaption ) override;

private:
	// Shell internal.

};


//
// Global shell instance.
//
extern CShell*	GShell;


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/