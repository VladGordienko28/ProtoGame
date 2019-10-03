

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
	virtual Double Now() = 0;
	virtual void SetNow( Double InNow ) = 0;
	virtual void ClipboardCopy( Char* Str ) = 0;
	virtual String ClipboardPaste() = 0;
	virtual void Launch( const Char* Target, const Char* Parms ) = 0;
	virtual envi::TimeOfDay GetTimeOfDay() = 0;
};


/*-----------------------------------------------------------------------------
    Global variables.
-----------------------------------------------------------------------------*/

extern Bool				GIsEditor;
extern CPlatformBase*	GPlat;
extern UInt32			GFrameStamp;
extern String			GCmdLine;