/*=============================================================================
	FrBase.h: Platform specific functions.
	Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

// Basic integral types
using Int8		= signed char;
using UInt8		= unsigned char;
using Int16		= signed short;
using UInt16	= unsigned short;
using Int32		= signed int;
using UInt32	= unsigned int;
using Int64		= signed long long;
using UInt64	= unsigned long long;
using SizeT		= size_t;

// Basic float-point types
using Float		= float;
using Double	= double;

// Other basic types
using Bool		= bool;

// Symbol types
typedef wchar_t		WideChar;
typedef char		AnsiChar;

#if defined(TXT)
#undef TXT
#endif
#if FLU_USE_WIDECHAR
typedef WideChar	Char;
#define TXT( s ) L##s
#else
typedef AnsiChar	Char;
#define TXT( s ) s
#endif

// Validation
static_assert( sizeof(Int8) == 1, "sizeof(Int8) == 1" );
static_assert( sizeof(UInt8) == 1, "sizeof(UInt8) == 1" );
static_assert( sizeof(Int16) == 2, "sizeof(Int16) == 2" );
static_assert( sizeof(UInt16) == 2, "sizeof(UInt16) == 2" );
static_assert( sizeof(Int32) == 4, "sizeof(Int32) == 4" );
static_assert( sizeof(UInt32) == 4, "sizeof(UInt32) == 4" );
static_assert( sizeof(Int64) == 8, "sizeof(Int64) == 8" );
static_assert( sizeof(UInt64) == 8, "sizeof(UInt64) == 8" );

static_assert( sizeof(AnsiChar) == 1, "sizeof(AnsiChar) == 1" );
static_assert( sizeof(WideChar) == 2, "sizeof(WideChar) == 2" );

static_assert( sizeof(Float) == 4, "sizeof(Float) == 4" );
static_assert( sizeof(Double) == 8, "sizeof(Double) == 8" );

static_assert( sizeof(void*) == sizeof(SizeT), "sizeof(void*) == sizeof(SizeT)" );

namespace flu
{
	// Constants
	static const Int8 MAX_INT8 = 0x7f;
	static const UInt8 MAX_UINT8 = 0xff;
	static const Int16 MAX_INT16 = 0x7fff;
	static const UInt16 MAX_UINT16 = 0xffff;
	static const Int32 MAX_INT32 = 0x7fffffff;
	static const UInt32 MAX_UINT32 = 0xffffffff;
	static const Int64 MAX_INT64 = 0x7fffffffffffffff;
	static const UInt64 MAX_UINT64 = 0xffffffffffffffff;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/