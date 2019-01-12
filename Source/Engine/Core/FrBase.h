

/*-----------------------------------------------------------------------------
    Macroses.
-----------------------------------------------------------------------------*/

// Its upset me :(
#define WIDEN2(x) L ## x 
#define WIDEN(x) WIDEN2(x) 
#define __WFILE__ WIDEN(__FILE__) 

//
// Output macro.
//
#define assert(expr) { if(!(expr)) error( L"Assertion failed: \"%s\" [File: %s][Line: %i]", L#expr, __WFILE__, __LINE__ ); }
#define error	if( ::GOutput ) ::GOutput->Errorf
#define warn	if( ::GOutput ) ::GOutput->Warnf

#define trace(...)		if( ::GOutput ) ::GOutput->Logf( SVR_Trace, __VA_ARGS__ );
#define info(...)		if( ::GOutput ) ::GOutput->Logf( SVR_Info, __VA_ARGS__ );
#define log(...)		if( ::GOutput ) ::GOutput->Logf( SVR_Log, __VA_ARGS__ );
#define notice(...)		if( ::GOutput ) ::GOutput->Logf( SVR_Notice, __VA_ARGS__ );
#define debug(...)		if( ::GOutput ) ::GOutput->Logf( SVR_Debug, __VA_ARGS__ );


//
// Debug macro.
//
#define benchmark_begin(op) \
{\
	Char* BenchOp = L#op; \
	DWord InitTime = GPlat->Cycles();\

#define benchmark_end \
	log( L"Operation \"%s\" take %d cycles", BenchOp, GPlat->Cycles()-InitTime ); \
}\


//
// Various macro.
//
#define align(value, bound) ((value)+(bound)-1)&(~((bound)-1))
#define freeandnil(Obj) { if( Obj ){ delete Obj; Obj = nullptr; } }
#define arr_len(arr) (sizeof(arr)/sizeof(arr[0]))


/*-----------------------------------------------------------------------------
    Memory functions.
-----------------------------------------------------------------------------*/

inline void* MemAlloc( SizeT Count )
{
	return calloc( Count, 1 );
}
inline void* MemMalloc( SizeT Count )
{
	return malloc( Count );
}
inline void* MemRealloc( void* Addr, SizeT NewCount )
{
	return realloc( Addr, NewCount );
}
inline void MemFree( void* Addr )
{
	free( Addr );
}
inline void MemZero( void* Addr, SizeT Count )
{
	memset( Addr, 0, Count );
}
inline void MemSet( void* Addr, SizeT Count, UInt8 Value )
{
	memset( Addr, Value, Count );
}
inline Bool MemCmp( const void* A, const void* B, SizeT Count )
{
	return memcmp( A, B, Count ) == 0;
}
inline void MemCopy( void* Dst, const void* Src, SizeT Count )
{
	memcpy( Dst, Src, Count );
}
inline void MemSwap( void* A, void* B, SizeT Count )
{
	UInt8 Buffer[1024];
	MemCopy( Buffer, A, Count );
	MemCopy( A, B, Count );
	MemCopy( B, Buffer, Count );
}

#define MemAlloca( size )	_alloca(size);

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
	virtual TTimeOfDay GetTimeOfDay() = 0;
	virtual TArray<String> FindFiles( String Directory, String Wildcard ) = 0;
};


/*-----------------------------------------------------------------------------
    Global variables.
-----------------------------------------------------------------------------*/

extern Bool				GIsEditor;
extern CPlatformBase*	GPlat;
extern String			GDirectory;
extern UInt32			GFrameStamp;
extern String			GCmdLine;
