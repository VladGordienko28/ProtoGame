/*=============================================================================
	FrString.h: Dynamic sizeable string.
	Created by Vlad Gordienko, Jun. 2016.
	Reimplemented by Vlad Gordienko Nov. 2017.
=============================================================================*/

/*-----------------------------------------------------------------------------
	String.
-----------------------------------------------------------------------------*/

//
// An advanced Wide-Char string.
//
class String
{
public:
	// Constructor.
	String();
	String( const String& Other );
	String( const Char* Other );
	String( const Char* Other, Integer InLen );
	
	// Destructor.
	~String();

	// Methods.
	Integer Len() const;
	DWord RefsCount() const;
	DWord HashCode() const;

	// Operators.
	Char* operator*() const;
	String& operator=( const String& Other );
	String& operator=( const Char* Str );
	Char operator()( Integer i ) const;
	const Char& operator[]( Integer i ) const;
	Char& operator[]( Integer i );
	Bool operator==( const String& Other ) const;
	Bool operator==( const Char* Str ) const;
	Bool operator!=( const String& Other ) const;
	Bool operator!=( const Char* Str ) const;
	Bool operator>( const String& Other ) const;
	Bool operator<( const String& Other ) const;
	Bool operator>=( const String& Other ) const;
	Bool operator<=( const String& Other ) const;
	String& operator+=( const Char* Str );
	String& operator+=( const String& Other );
	String operator+( const Char* Str );
	String operator+( const String& Other );
	operator Bool() const;

	// Functions.
	Bool ToInteger( Integer& OutValue, Integer Default = 0 ) const;
	Bool ToFloat( Float& OutValue, Float Default = 0.f ) const;

	// Static.
	static String Format( String Fmt, ... );
	static Integer Pos( String Needle, String HayStack, Integer iFrom = 0 );
	static String UpperCase( String Str );
	static String LowerCase( String Str );
	static Integer CompareText( String Str1, String Str2 );
	static Integer CompareStr( String Str1, String Str2 );
	static String OfChar( Char Repeat, Integer Count );
	static String FromInteger( Integer Value );
	static String FromFloat( Float Value );
	static String Copy( String Source, Integer StartChar, Integer Count );
	static String Delete( String Str, Integer StartChar, Integer Count );
	static TArray<String> String::WrapText( String Text, Integer MaxColumnSize );

	// Pool cleanup.
	static void Flush();

	// Friends.
	friend void Serialize( CSerializer& S, String& V );

private:
	// String internal.
	struct TInternal
	{
		union
		{
			struct{ Integer Length; DWord RefsCount; };
			struct{ TInternal* DeadNext; };
		};
		Char Data[0];
	} *Internal;
	
	enum{ DEAD_POOL_MAX = 8192 };
	static TInternal* DeadPool[DEAD_POOL_MAX];

	// String memory managment.
	static void Initialize( TInternal*& Internal, Integer InLen );
	static void Deinitialize( TInternal*& Internal );
	static void Reinitialize( TInternal*& Internal, Integer NewLen );
};


/*-----------------------------------------------------------------------------
	String utils.
-----------------------------------------------------------------------------*/

//
// Character is a digit?
//
inline Bool IsDigit( Char Ch )
{
	return (Ch >= L'0') && (Ch <= L'9');
}


//
// Character is a letter?
//
inline Bool IsLetter( Char Ch )
{
	return	( (Ch >= L'A' )&&( Ch <= L'Z') )||
			( (Ch >= L'a' )&&( Ch <= L'z') )||
			(Ch == L'_');
}


//
// Character is a letter or digit.
//
inline Bool IsDigitLetter( Char Ch )
{
	return IsDigit(Ch) || IsLetter(Ch);
}


//
// Convert hex character to the
// integer value.
//
inline Integer FromHex( Char Ch )
{
	if( Ch >= '0' && Ch <= '9' )
		return Ch-'0';
	else if( Ch >= 'a' && Ch <= 'f' )
		return Ch-'a'+10;
	else
		return 0;
}


//
// Return true if string contains only ANSI characters.
//
inline Bool IsAnsiOnly( const Char* Str )
{
	for( ; *Str; Str++ )
		if( *Str>0xff || *Str<0x00 )
			return false;
	return true;
}


/*-----------------------------------------------------------------------------
	String implementation.
-----------------------------------------------------------------------------*/

//
// Constructor.
//
inline String::String()
	:	Internal(nullptr)
{}
inline String::String( const String& Other )
	:	Internal(Other.Internal)
{
	if( Internal )
		Internal->RefsCount++;
}
inline String::String( const Char* Other )
	:	Internal(nullptr)
{
	if( Other && *Other )
	{
		Integer RealLen	= (Integer)wcslen(Other);
		Initialize( Internal, RealLen );
		MemCopy( Internal->Data, Other, RealLen*sizeof(Char) );
	}
}
inline String::String( const Char* Other, Integer InLen )
	:	Internal(nullptr)
{
	if( Other && *Other && InLen )
	{
		Initialize( Internal, InLen );
		MemCopy( Internal->Data, Other, InLen*sizeof(Char) );
	}
}


//
// Destructor.
//
inline String::~String()
{
	Deinitialize(Internal);
}


//
// Methods.
//
inline Integer String::Len() const
{
	return Internal ? Internal->Length : 0;
}
inline DWord String::RefsCount() const
{
	return Internal ? Internal->RefsCount : 0;
}
inline DWord String::HashCode() const
{
	DWord Hash = 2139062143;
	if( Internal )
		for( Char* C = Internal->Data; *C; C++ )
			Hash = 37 * Hash + *C;
	return Hash;
}

//
// Operators.
//
inline Char* String::operator*() const
{
	return Internal ? Internal->Data : L"";
}
inline String& String::operator=( const String& Other )
{
	Deinitialize(Internal);
	if( Other.Internal )
	{
		Internal	= Other.Internal;
		Internal->RefsCount++;
	}
	return *this;
}
inline String& String::operator=( const Char* Str )
{
	Deinitialize(Internal);
	Integer RealLen	= (Integer)wcslen(Str);
	if( RealLen )
	{
		Initialize( Internal, RealLen );
		MemCopy( Internal->Data, Str, RealLen*sizeof(Char) );
	}
	return *this;
}
inline Char String::operator()( Integer i ) const
{
	return Internal ? Internal->Data[i] : L'\0';
}
inline const Char& String::operator[]( Integer i ) const
{
	return Internal ? Internal->Data[i] : L'\0';
}
inline Char& String::operator[]( Integer i )
{
	if( Internal )
	{
		if( Internal->RefsCount > 1 )
		{
			TInternal* Indirected = nullptr;
			Initialize( Indirected, Internal->Length );
			MemCopy( Indirected->Data, Internal->Data, Internal->Length*sizeof(Char) );
			Internal->RefsCount--;
			Internal = Indirected;
		}
		return Internal->Data[i];
	}
	else
	{
		static Char Junk;
		return Junk;
	}
}
inline Bool String::operator==( const String& Other ) const
{
	if( Other.Internal != Internal )
	{
		if( !Other.Internal ) return Internal == nullptr;
		if( !Internal ) return Other.Internal == nullptr;
		Integer L1 = Len(), L2 = Other.Len();
		if( L1 != L2 )
			return false;
		return wcscmp( Internal->Data, Other.Internal->Data ) == 0;
	}
	else
		return true;
}
inline Bool String::operator==( const Char* Str ) const
{
	if( !Str || !*Str ) return Internal == nullptr;
	if( !Internal ) return false;
	Integer L1 = Len(), L2 = (Integer)wcslen(Str);
	if( L1 != L2 )
		return false;
	return wcscmp( Internal->Data, Str ) == 0;
}
inline Bool String::operator!=( const String& Other ) const
{
	return !operator==(Other);
}
inline Bool String::operator!=( const Char* Str ) const
{
	return !operator==(Str);
}
inline Bool String::operator>( const String& Other ) const
{
	return wcscmp( Internal->Data, Other.Internal->Data ) > 0;
}
inline Bool String::operator<( const String& Other ) const
{
	return wcscmp( Internal->Data, Other.Internal->Data ) < 0;
}
inline Bool String::operator>=( const String& Other ) const
{
	return wcscmp( Internal->Data, Other.Internal->Data ) >= 0;
}
inline Bool String::operator<=( const String& Other ) const
{
	return wcscmp( Internal->Data, Other.Internal->Data ) <= 0;
}
inline String& String::operator+=( const Char* Str )
{
	Integer L2 = (Integer)wcslen(Str);
	if( L2 )
	{
		if( Internal )
		{
			Integer L1 = Len(), LR = L1+L2;
			Reinitialize( Internal, LR );
			MemCopy( &Internal->Data[L1], Str, L2*sizeof(Char) );
		}
		else
			*this = Str;
	}
	return *this;
}
inline String& String::operator+=( const String& Other )
{
	if( Other.Len() )
	{
		if( Internal )
		{
			Integer L1 = Len(), L2 = Other.Len(), LR = L1+L2;
			Reinitialize( Internal, LR );
			MemCopy( &Internal->Data[L1], Other.Internal->Data, L2*sizeof(Char) );
		}
		else
			*this = Other;
	}
	return *this;
}
inline String String::operator+( const Char* Str )
{
	return String(*this) += Str;
}
inline String String::operator+( const String& Other )
{
	return operator+(*Other);
}
inline String::operator Bool() const
{
	return Internal != nullptr;
}


//
// String managment functions.
//
inline void String::Initialize( TInternal*& Internal, Integer InLen )
{
	if( Internal )
		Deinitialize( Internal );

	if( InLen < DEAD_POOL_MAX && DeadPool[InLen] )
	{
		Internal		= DeadPool[InLen];
		DeadPool[InLen]	= Internal->DeadNext;
	}
	else
	{
		Internal		= (TInternal*)MemMalloc(sizeof(TInternal)+(InLen+1)*sizeof(Char));
		Internal->Data[InLen] = L'\0';
	}

	Internal->Length	= InLen;
	Internal->RefsCount	= 1;
}


inline void String::Reinitialize( TInternal*& Internal, Integer NewLen )
{
	if( NewLen > 0 )
	{
		if( Internal )
		{
			TInternal* New = nullptr;
			Initialize( New, NewLen );
			Integer MinLen = New->Length < Internal->Length ? New->Length : Internal->Length;
			MemCopy( New->Data, Internal->Data, MinLen*sizeof(Char) );
			New->Data[MinLen] = L'\0';
			Deinitialize( Internal );
			Internal = New;
		}
		else
			Initialize( Internal, NewLen );
	}
	else
		Deinitialize( Internal );
}


inline void String::Deinitialize( TInternal*& Internal )
{
	if( Internal )
	{
		if( --Internal->RefsCount == 0 )
		{
			if( Internal->Length < DEAD_POOL_MAX )
			{
				Integer L			= Internal->Length;
				Internal->DeadNext	= DeadPool[L];
				DeadPool[L]			= Internal;
			}
			else
			{
				MemFree( Internal );
			}
		}
		Internal	= nullptr;
	}
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/