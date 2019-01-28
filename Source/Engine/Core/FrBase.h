

/*-----------------------------------------------------------------------------
    Macroses.
-----------------------------------------------------------------------------*/

//
// Various macro.
//
#define freeandnil(Obj) { if( Obj ){ delete Obj; Obj = nullptr; } }


/*-----------------------------------------------------------------------------
    CPlatformBase.
-----------------------------------------------------------------------------*/

//
// Platform global functions.
//
class CPlatformBase
{
public:
	virtual Double TimeStamp() = 0;
	virtual Double Now() = 0;
	virtual void SetNow( Double InNow ) = 0;
	virtual UInt32 Cycles() = 0;
	virtual Bool FileExists( String FileName ) = 0;
	virtual Bool DirectoryExists( String Dir ) = 0;
	virtual void ClipboardCopy( Char* Str ) = 0;
	virtual String ClipboardPaste() = 0;
	virtual void Launch( const Char* Target, const Char* Parms ) = 0;
	virtual envi::TimeOfDay GetTimeOfDay() = 0;
	virtual Array<String> FindFiles( String Directory, String Wildcard ) = 0;
};


/*-----------------------------------------------------------------------------
    Global variables.
-----------------------------------------------------------------------------*/

extern Bool				GIsEditor;
extern CPlatformBase*	GPlat;
extern String			GDirectory;
extern UInt32			GFrameStamp;
extern String			GCmdLine;