/*=============================================================================
	FrLog.h: An abstract output class.
	Created by Vlad Gordienko Jun.2016.
	Redesigned and extended by Vlad, Jan.2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	CDebugOutputBase.
-----------------------------------------------------------------------------*/

//
// An output severity. Enumeration is sorted by
// by priority.
//
enum ESeverity
{
	SVR_Trace,
	SVR_Info,
	SVR_Log,
	SVR_Notice,
	SVR_Debug,
	SVR_MAX
};


//
// An abstract output.
//
class CDebugOutputBase
{
public:
	// CDebugOutputBase interface.
	virtual void Logf( ESeverity Severity, Char* Text, ... ) = 0;
	virtual void Warnf( Char* Text, ... ) = 0;
	virtual void Errorf( Char* Text, ... ) = 0;

	virtual void ScriptLogf( ESeverity Severity, Char* Text, ... ) = 0;
	virtual void ScriptWarnf( Char* Text, ... ) = 0;
	virtual void ScriptErrorf( Char* Text, ... ) = 0;
};

// Global instance.
extern CDebugOutputBase* GOutput;


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/