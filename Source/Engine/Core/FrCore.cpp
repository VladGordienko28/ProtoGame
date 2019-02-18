/*=============================================================================
    FrCore.cpp: Various engine core functions.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "..\Engine.h"

//
// Globals.
//
Bool				GIsEditor	= false;
CPlatformBase*		GPlat		= nullptr;
String				GDirectory	= L"";
UInt32				GFrameStamp = 0;
String				GCmdLine;


/*-----------------------------------------------------------------------------
	String implementation.
-----------------------------------------------------------------------------*/

// Internal pool of dead strings.
String::TInternal* String::DeadPool[String::DEAD_POOL_MAX] = {};


//
// Format string.
//
String String::Format( String Fmt, ... )
{
	static Char Dest[4096] = {};
	va_list ArgPtr;
	va_start( ArgPtr, Fmt );
	_vsnwprintf_s( Dest, 4096, *Fmt, ArgPtr );
	va_end( ArgPtr );
	return Dest;
}


//
// Search needle in haystack :), of string
// of course. If needle not found return -1.
//
Int32 String::Pos( String Needle, String HayStack, Int32 iFrom )
{
	const Char* P = wcsstr( &((*HayStack)[iFrom]), *Needle );
	return P ? (Int32)(((SizeT)P - (SizeT)*HayStack)/sizeof(Char)) : -1;
}


//
// Return string copy with upper case.
//
String String::UpperCase( String Str )
{
	if( !Str )
		return L"";

	String N;
	Initialize( N.Internal, Str.Len() );
	for( Int32 i=0; i<Str.Len(); i++ )
	{
		N.Internal->Data[i] = towupper(Str(i));
	}
	return N;
}


//
// Return string copy with lower case.
//
String String::LowerCase( String Str )
{
	if( !Str )
		return L"";

	String N;
	Initialize( N.Internal, Str.Len() );
	for( Int32 i=0; i<Str.Len(); i++ )
	{
		N.Internal->Data[i] = towlower(Str(i));
	}
	return N;
}


//
// Strings case-insensitive comparison.
// Return:
//   < 0: Str1 < Str2.
//   = 0: Str1 = Str2.
//	 > 0: Str1 > Str2.
//
Int32 String::CompareText( String Str1, String Str2 )
{
	return _wcsicmp( *Str1, *Str2 );
}


//
// String comparison.
//
Int32 String::CompareStr( String Str1, String Str2 )
{
	return wcscmp( *Str1, *Str2 );
}


//
// Return string filled with specified characters.
//
String String::OfChar( Char Repeat, Int32 Count )
{
	if( Count == 0 )
		return L"";

	String N;
	Initialize( N.Internal, Count );
	for( Int32 i=0; i<Count; i++ )
		N.Internal->Data[i] = Repeat;
	return N;
}


//
// Convert integer value to string.
//
String String::FromInteger( Int32 Value )
{
	return String::Format( L"%i", Value );
}


//
// Convert float value to string.
//
String String::FromFloat( Float Value )
{
	return String::Format( L"%.4f", Value );
}


//
// Copy substring.
//
String String::Copy( String Source, Int32 StartChar, Int32 Count )
{
	StartChar = Clamp<Int32>( StartChar, 0, Source.Len()-1 );
	Count = Clamp<Int32>( Count, 0, Source.Len()-StartChar );
	return Count ? String( &Source.Internal->Data[StartChar], Count ) : String();
}


//
// Remove Count symbols from StartChar position. 
//
String String::Delete( String Str, Int32 StartChar, Int32 Count )
{
	StartChar = Clamp<Int32>( StartChar, 0, Str.Len()-1 );
	Count = Clamp<Int32>( Count, 0, Str.Len()-StartChar );
	return Copy( Str, 0, StartChar ) + Copy( Str, StartChar+Count, Str.Len()-StartChar-Count );	
}


//
// Convert string into integer value, return true if 
// converted successfully, otherwise return false and out value
// will be set default.
//
Bool String::ToInteger( Int32& Value, Int32 Default ) const
{
	if (!Len())
		return false;

	Int32		iChar	= 0;
	Bool		bNeg	= false;
	Value				= Default;

	// Detect sign.
	if( Internal->Data[0] == L'-' )
	{
		iChar++;
		bNeg = true;
	}
	else if( Internal->Data[0] == L'+' )
	{
		iChar++;
		bNeg = false;
	}

	// Parse digit by digit.
	Int32 Result = 0;
	for (Int32 i = iChar; i < Len(); i++)
	if (Internal->Data[i] >= L'0' && Internal->Data[i] <= L'9')
	{
		Result *= 10;
		Result += (Int32)(Internal->Data[i] - L'0');
	}
	else
		return false;

	Value = bNeg ? -Result : Result;
	return true;
}


//
// Convert string into float value, return true if 
// converted successfully, otherwise return false and out value
// will be set default.
//
Bool String::ToFloat( Float& Value, Float Default ) const
{
	if (!Len())
		return false;

	Int32		iChar	= 0;
	Bool		bNeg	= false;
	Float		Frac = 0.f, Ceil = 0.f;
	Value				= Default;

	// Detect sign.
	if (Internal->Data[0] == L'-')
	{
		iChar++;
		bNeg = true;
	}
	else if (Internal->Data[0] == L'+')
	{
		iChar++;
		bNeg = false;
	}

	if( iChar < Len() )
	{
		if( Internal->Data[iChar] == L'.' )
		{
			// Parse fractional part.
		ParseFrac:
			iChar++;
			Float m = 0.1f;
			for( ; iChar < Len(); iChar++ )
				if( Internal->Data[iChar] >= L'0' && Internal->Data[iChar] <= L'9' )
				{
					Frac += (Int32)(Internal->Data[iChar] - L'0') * m;
					m /= 10.f;
				}
				else
					return false;
		}
		else if( Internal->Data[iChar] >= L'0' && Internal->Data[iChar] <= L'9' )
		{
			// Parse ceil part.
			for( ; iChar < Len(); iChar++ )
				if( Internal->Data[iChar] >= L'0' && Internal->Data[iChar] <= L'9' )
				{
					Ceil *= 10.f;
					Ceil += (Int32)(Internal->Data[iChar] - L'0');
				}
				else if( Internal->Data[iChar] == L'.' )
				{
					goto ParseFrac;
				}
				else
					return false;
		}
		else
			return false;
	}
	else
		return false;

	Value = bNeg ? -(Ceil+Frac) : +(Ceil+Frac);
	return true;
}


//
// String serialization.
//
void Serialize( CSerializer& S, String& V )
{
	if( S.GetMode() == SM_Load )
	{
		// Load string with compression.
		Int32 Len;
		Serialize( S, Len );

		String::Deinitialize( V.Internal );

		if( Len < 0 )
		{
			// Load Ansi string.
			String::Initialize( V.Internal, -Len );
			for( Int32 i=0; i<V.Len(); i++ )
			{
				AnsiChar AC;
				Serialize( S, AC );
				(*V)[i] = AC;
			}
		}
		else if( Len > 0 )
		{
			// Load wide string.
			String::Initialize( V.Internal, Len );
			S.SerializeData( *V, sizeof(Char)*Len );
		}
	}
	else if( S.GetMode() == SM_Save )
	{
		// Save string with compression.
		Int32 Len	= IsAnsiOnly(*V) ? -(Int32)V.Len() : (Int32)V.Len();
		Serialize( S, Len );

		if( Len < 0 )
		{
			// Save Ansi string.
			for( Int32 i=0; i<V.Len(); i++ )
			{
				AnsiChar AC = (AnsiChar)V(i);
				Serialize( S, AC );
			}
		}
		else if( Len > 0 )
		{
			// Save wide string.
			S.SerializeData( *V, sizeof(Char)*Len );
		}
	}
	else
	{
		// Investigate string without compression.
		Int32 Len = V.Len();
		Serialize( S, Len );

		if( Len > 0 )
			S.SerializeData( *V, sizeof(Char)*Len );
	}
}


//
// Wrap text and put lines into array of string.
//
Array<String> String::WrapText( String Text, Int32 MaxColumnSize )
{
	Array<String> Result;

	Int32 i=0;
	do
	{
		Int32 iCleanWordEnd	= 0;
		Bool	bGotWord		= false;
		Int32	iTestWord		= 0;

		while	( 
					( i+iTestWord < Text.Len() )&&
					(Text(i+iTestWord) != '\n') 
				)
		{
			if( iTestWord++ > MaxColumnSize )
				break;

			Bool bWordBreak = (Text(iTestWord+i)==' ')||(Text(iTestWord+i)=='\n')||(iTestWord+i>=Text.Len());
			if( bWordBreak || !bGotWord )
			{
				iCleanWordEnd	= iTestWord;
				bGotWord		= bGotWord || bWordBreak;
			}
		}

		if( iCleanWordEnd == 0 )
			break;

		// Add to array.
		Result.push(String::Copy( Text, i, iCleanWordEnd ));
		i += iCleanWordEnd;

		// Skip whitespace after word.
		while( Text(i) == ' ' )
			i++;

	} while( i<Text.Len() );

	return Result;
}


//
// Cleanup dead pool of strings.
//
void String::Flush()
{
	// Counters for statistics.
	Int32	SlotsUsed	= 0,
			MaxDepth	= 0,
			TotalLen	= 0,
			NumStrs		= 0;

	// Dive to pool.
	for( Int32 i=0; i<DEAD_POOL_MAX; i++ )
	{
		TInternal* Item = DeadPool[i];

		if( Item )
		{
			Int32 Depth = 0;
			DeadPool[i]	= nullptr;

			while( Item )
			{
				TotalLen += i;
				NumStrs++;
				Depth++;

				TInternal* NextItem = Item->DeadNext;
				mem::free(Item);
				Item = NextItem;
			}

			if( Depth > MaxDepth )
				MaxDepth	= Depth;
			SlotsUsed++;
		}
	}

	// Dump.
	info( L"String DeadPool stats: " );
	info( L"  SlotsInUse: %i; MaxDepth: %i", SlotsUsed, MaxDepth );
	info( L"  TotalStrs: %i; TotalLen: %i", NumStrs, TotalLen );
}


/*-----------------------------------------------------------------------------
    TColor implementation.
-----------------------------------------------------------------------------*/

//
// Convert RGB color value to HSL value
// where H(Hue), S(Saturation), L(Lightness).
//
void TColor::RGBToHSL( TColor Color, UInt8& H, UInt8& S, UInt8& L )
{
	Float R	= Color.R / 256.f;
	Float G	= Color.G / 256.f;
	Float B = Color.B / 256.f;
	
	Float MinValue	= Min( R, Min( G, B ) );
	Float MaxValue	= Max( R, Max( G, B ) );

	if( R == G && G == B )
	{
		H	= 0;
		S	= 0;
		L	= Color.G;
	}
	else
	{
		Float FH, FS, FL;

		FL	= ( MinValue + MaxValue ) * 0.5f;

		if( FL < 0.5f )
			FS	= ( MaxValue - MinValue ) / ( MaxValue + MinValue );
		else
			FS	= ( MaxValue - MinValue ) / ( 2.f - MaxValue - MinValue );

		if( R == MaxValue )
			FH	= ( G - B ) / ( MaxValue - MinValue );
		else if( G == MaxValue )
			FH	= 2.f + ( B - R ) / ( MaxValue - MinValue );
		else if( B == MaxValue )
			FH	= 4.f + ( R - G ) / ( MaxValue - MinValue );

		FH /= 6.f;
		if( FH < 0.f ) FH += 1.f;

		H	= math::trunc( FH * 254.9f );
		S	= math::trunc( FS * 254.9f );
		L	= math::trunc( FL * 254.9f );
	}
}


//
// Convert HSL color value to RGB value
// where H(Hue), S(Saturation), L(Lightness).
//
TColor TColor::HSLToRGB( UInt8 H, UInt8 S, UInt8 L )
{
	Float	FH	= H / 256.f;
	Float	FS	= S / 256.f;
	Float	FL	= L / 256.f;

	if( S == 0 )
	{
		return TColor( L, L, L, 0xff );
	}
	else
	{
		Float	FR, FG, FB, temp1, temp2, tempR, tempG, tempB;

		if( FL < 0.5f )
			temp2	= FL * ( 1.f + FS );
		else
			temp2	= ( FL + FS ) - ( FL * FS );

		temp1	= 2.f * FL - temp2;
		tempR	= FH + 1.f / 3.f;

		if( tempR > 1.f ) tempR -= 1.f;
		tempG	= FH;
		tempB	= FH - 1.f / 3.f;
		if( tempB < 0.f ) tempB += 1.f;

		// Red channel.
		if( tempR < 1.f/6.f )
			FR	= temp1 + (temp2-temp1)*6.f * tempR;
		else if( tempR < 0.5f )
			FR	= temp2;
		else if( tempR < 2.f/3.f )
			FR	= temp1 + (temp2-temp1)*((2.f/3.f)-tempR)*6.f;
		else
			FR	= temp1;

		// Green channel.
		if( tempG < 1.f/6.f )
			FG	= temp1 + (temp2-temp1)*6.f * tempG;
		else if( tempG < 0.5f )
			FG	= temp2;
		else if( tempG < 2.f/3.f )
			FG	= temp1 + (temp2-temp1)*((2.f/3.f)-tempG)*6.f;
		else
			FG	= temp1;

		// Blue channel.
		if( tempB < 1.f/6.f )
			FB	= temp1 + (temp2-temp1)*6.f * tempB;
		else if( tempB < 0.5f )
			FB	= temp2;
		else if( tempB < 2.f/3.f )
			FB	= temp1 + (temp2-temp1)*((2.f/3.f)-tempB)*6.0;
		else
			FB	= temp1;

		return TColor( math::trunc( FR * 254.9f ),
					   math::trunc( FG * 254.9f ),
					   math::trunc( FB * 254.9f ),
					   0xff );
	}
}

/*-----------------------------------------------------------------------------
    TDelegate implementation.
-----------------------------------------------------------------------------*/

//
// Delegate serialization.
//
void Serialize( CSerializer& S, TDelegate& V )
{
	Serialize( S, V.iMethod );
	Serialize( S, V.Script );
	Serialize( S, V.Context );
}


/*-----------------------------------------------------------------------------
	TUrl implementation.
-----------------------------------------------------------------------------*/

// URL constants.
#define URL_DEFAULT_PROTOCOL			L"flu"
#define URL_DEFAULT_HOST				L"localhost"


//
// Url default constructor.
//
TUrl::TUrl()
	:	Protocol(URL_DEFAULT_PROTOCOL),
		Login(L""),
		Password(L""),
		Host(URL_DEFAULT_HOST),
		Port(0),
		Path(L""),
		Fragment(L""),
		Query()
{
}


//
// Construct Url from the string.
//
TUrl::TUrl( String InUrl )
{
	// Parse protocol.
	Int32 iProtocolEnd = String::Pos(L"://", InUrl);
	if( iProtocolEnd != -1 )
	{
		Protocol = String::Copy( InUrl, 0, iProtocolEnd );
		iProtocolEnd += 3;
	}
	else
	{
		// No protocol.
		Protocol = URL_DEFAULT_PROTOCOL;
	}

	// Login and password.
	Int32 iUserEnd = String::Pos(L"@", InUrl, iProtocolEnd);
	if( iUserEnd != -1 )
	{
		Int32 iPasswordBegin = String::Pos(L":", InUrl, iProtocolEnd);
		if( iPasswordBegin != -1 && iPasswordBegin < iUserEnd )
		{
			Login = String::Copy( InUrl, iProtocolEnd, iPasswordBegin-iProtocolEnd );
			Password = String::Copy( InUrl, iPasswordBegin+1, iUserEnd-iPasswordBegin-1 );
		}
		else
		{
			// No password.
			Login = String::Copy( InUrl, iProtocolEnd, iUserEnd-iProtocolEnd );
			Password = L"";
		}
	}
	else
	{
		// No user credentials.
		Login = L"";
		Password = L"";
	}

	// Fragment.
	Int32 iFragmentBegin = String::Pos( L"#", InUrl );
	if( iFragmentBegin != -1 )
	{
		Fragment = String::Copy( InUrl, iFragmentBegin+1, InUrl.Len()-iFragmentBegin-1 );
	}
	else
	{
		Fragment = L"";
	}

	// Port.
	Int32 iPortBegin = String::Pos( L":", InUrl, Max(iUserEnd, iProtocolEnd) );
	Int32 iPortEnd = iPortBegin+1;
	if( iPortBegin != -1 )
	{
		while(iPortEnd < InUrl.Len() && IsDigit(InUrl[iPortEnd]))
			iPortEnd++;

		String::Copy(InUrl, iPortBegin+1, iPortEnd-iPortBegin-1).ToInteger(Port, 0);
	}
	else
		Port = 0;


	// Host.
	Int32 iHostBegin = Max( iUserEnd+1, iProtocolEnd );
	Int32 iHostEnd = iPortBegin != -1 ? iPortBegin : String::Pos(L"/", InUrl, iHostBegin);
	Host = String::Copy( InUrl, iHostBegin, iHostEnd-iHostBegin );

	// Query.
	Int32 iQueryBegin = String::Pos(L"?", InUrl)+1;
	if( iQueryBegin != -1 )
	{
		Int32 iPairBegin = iQueryBegin;
		
		do 
		{
			Int32 iPairEnd = String::Pos(L"&", InUrl, iPairBegin);
			if( iPairEnd == -1 )
				iPairEnd = iFragmentBegin != -1 ? iFragmentBegin : InUrl.Len();
			if( iPairEnd == -1 || iPairEnd > InUrl.Len() || iPairBegin >= iPairEnd )
				break;

			Int32 iMiddle = String::Pos( L"=", InUrl, iPairBegin )+1;

			String Key = String::Copy( InUrl, iPairBegin, iMiddle-iPairBegin-1 );
			String Value = String::Copy( InUrl, iMiddle, iPairEnd-iMiddle );
			Query.Add( Key, Value );

			iPairBegin = iPairEnd+1;
		} while( true );
	}

	// Path.
	Int32 iPathBegin = Max(iHostEnd+1, iPortEnd+1);
	Int32 iPathEnd = iQueryBegin != -1 ? iQueryBegin-1 : iFragmentBegin != -1 ? iFragmentBegin : InUrl.Len();
	Path = String::Copy( InUrl, iPathBegin, iPathEnd-iPathBegin );
}


//
// Construct Url from the Base Url and path.
//
TUrl::TUrl( const TUrl& BaseUrl, String InPath )
{
	*this = BaseUrl;
	Path = InPath;
}


//
// Convert to human-readable format.
//
String TUrl::ToString() const
{
	String Result = L"";

	if( Protocol )
		Result += String::Format(L"%s://", *Protocol);

	if( Login )
		Result += Password ? String::Format(L"%s:%s@", *Login, *Password) : String::Format(L"%s@", *Login);

	Result += Host;

	if( Port != 0 )
		Result += String::Format(L":%d", Port);

	Result += String(L"/") + Path;

	if( Query.Entries.size() )
	{
		Result += L"?";
		for( Int32 i=0; i<Query.Entries.size(); i++ )
			Result += String::Format(L"%s%s=%s", i>0 ? L"&" : L"", *Query.Entries[i].Key, *Query.Entries[i].Value );
	}

	if( Fragment )
		Result += String::Format(L"#%s", *Fragment);

	return Result;
}


//
// Compare only Url, not parameters or something.
//
Bool TUrl::operator==( const TUrl& Other ) const
{
	return	String::CompareText(Protocol, Other.Protocol) == 0 &&
			String::CompareText(Host, Other.Host) == 0 &&
			Port == Other.Port &&
			String::CompareText(Path, Other.Path) == 0 &&
			String::CompareText(Fragment, Other.Fragment) == 0;
}


//
// Not equal operator.
//
Bool TUrl::operator!=( const TUrl& Other ) const
{
	return !operator==(Other);
}


//
// Url serialization.
//
void Serialize( CSerializer& S, TUrl& V )
{
	Serialize( S, V.Protocol );
	Serialize( S, V.Login );
	Serialize( S, V.Password );
	Serialize( S, V.Host );
	Serialize( S, V.Port );
	Serialize( S, V.Path );
	Serialize( S, V.Fragment );
	Serialize( S, V.Query );
}


/*-----------------------------------------------------------------------------
	Hash functions.
-----------------------------------------------------------------------------*/

//
// Murmur2 hash function.
//
UInt32 MurmurHash( const UInt8* Data, SizeT Size )
{
	const UInt8*	Ptr	= Data;

	UInt32	M		= 0x5bd1e995,
			R		= 24,
			H		= 0 ^ (UInt32)Size;

	while( Size >= 4 )
	{
		UInt32 k	= *(UInt32*)Ptr;

		k	*= M;
		k	^= k >> R;
		k	*= M;

		H	*= M;
		H	^= k;

		Ptr	 += 4;
		Size -= 4;
	}

	switch( Size )
	{
		case 3:	H	^= Ptr[2] << 16;
		case 2:	H	^= Ptr[1] << 8;
		case 1:
				H	^= Ptr[0];
				H	*= M;
	}

	H	^= H >> 13;
	H	*= M;
	H	^= H >> 15;

	return H;
}


/*-----------------------------------------------------------------------------
	CCmdLineParser implementation.
-----------------------------------------------------------------------------*/

//
// Parse a bool parameter from the line.
//
Bool CCmdLineParser::ParseBoolParam( String Line, String Name, Bool Default )
{
	String Value = String::LowerCase(ParseStringParam( Line, Name ));

	if( Value==L"true" || Value==L"on" || Value==L"1" )		
		return true;

	if( Value==L"false" || Value==L"off" || Value==L"0" )		
		return false;

	return Default;
}


//
// Parse an integer parameter from the line.
//
Int32 CCmdLineParser::ParseIntParam( String Line, String Name, Int32 Default )
{
	Int32 Result = 0;
	String Value = ParseStringParam( Line, Name );
	Value.ToInteger( Result, Default );
	return Result;
}


//
// Parse a float parameter from the line. 
//
Float CCmdLineParser::ParseFloatParam( String Line, String Name, Float Default )
{
	Float Result = 0.f;
	String Value = ParseStringParam( Line, Name );
	Value.ToFloat( Result, Default );
	return Result;
}


//
// Parse a controling word from the line.
//
String CCmdLineParser::ParseCommand( String Line, Int32 iArgNum )
{
	Int32 iCommandBegin = 0;

	for( ; ; )
	{
		while( iCommandBegin < Line.Len() && Line[iCommandBegin] == ' ' )
			iCommandBegin++;

		Int32 iCommandEnd = iCommandBegin;
		while( iCommandEnd < Line.Len() && IsDigitLetter(Line[iCommandEnd]) )
			iCommandEnd++;

		if( iCommandBegin != iCommandEnd && iCommandBegin < Line.Len() )
		{
			if( iArgNum != 0 )
			{
				iCommandBegin = iCommandEnd + 1;
				iArgNum--;
			}
			else
			{
				return String::Copy( Line, iCommandBegin, iCommandEnd-iCommandBegin );
			}
		}
		else
			return L"";
	}
}


//
// Parse a string parameter from the line.
//
String CCmdLineParser::ParseStringParam( String Line, String Name, String Default )
{
	Int32 iNameBegin = String::Pos
	(
		String::LowerCase(Name),
		String::LowerCase(Line)
	);
	if( iNameBegin == -1 )
		return Default;
	
	Int32 iNameEnd = iNameBegin + Name.Len();
	if( iNameEnd >= Line.Len() || Line[iNameEnd] != '=' )
		return Default;

	Int32 iValueBegin = iNameEnd + 1;
	if( iValueBegin >= Line.Len() || Line[iValueBegin] == ' ' )
		return Default;

	if( Line[iValueBegin] == '"' )
	{
		Int32 iQuoteEnd = String::Pos( L"\"", Line, iValueBegin+1 );
		if( iQuoteEnd != -1 )
		{
			return String::Copy( Line, iValueBegin+1, iQuoteEnd-iValueBegin-1 );
		}
		else
			return Default;
	}
	else if( IsDigitLetter(Line[iValueBegin]) )
	{
		Int32 iValueEnd = iValueBegin;
		while( iValueEnd < Line.Len() && IsDigitLetter(Line[iValueEnd]) )
			iValueEnd++;

		return String::Copy( Line, iValueBegin, iValueEnd-iValueBegin );
	}
	else
		return Default;
}


//
// Parse an option from the line.
//
Bool CCmdLineParser::ParseOption( String Line, String Option )
{
	if( !Option )
		return false;

	String LowLine = String::LowerCase(Line);
	String LowOption = String::LowerCase(Option);

	String LongOpt = String(L"--") + LowOption;
	if( String::Pos( LongOpt, LowLine ) != -1 )
		return true;

	Char Tmp[2] = { **LowOption, '\0' };
	String ShortOpt = String(L"-") + Tmp;
	if( String::Pos( ShortOpt, LowLine ) != -1 )
		return true;

	return false;
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/