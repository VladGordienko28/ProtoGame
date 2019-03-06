/*=============================================================================
	FrPlat.h: Window platfrom specific functions.
	Created by Vlad Gordienko, Feb. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	CWinPlatform.
-----------------------------------------------------------------------------*/

//
// Window platform functions.
//
class CWinPlatform: public CPlatformBase
{
public:
	// CWinPlatform interface.
	CWinPlatform();

	// CPlatformBase interface.
	Double TimeStamp();
	Double Now();
	UInt32 Cycles();
	void ClipboardCopy( Char* Str );
	String ClipboardPaste();
	void Launch( const Char* Target, const Char* Parms );
	envi::TimeOfDay GetTimeOfDay();
	void SetNow( Double InNow );

private:
	// Internal variables.
	Double		SecsPerCycle;
	Double		NowTime;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/