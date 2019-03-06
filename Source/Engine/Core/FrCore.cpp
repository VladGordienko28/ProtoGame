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
UInt32				GFrameStamp = 0;
String				GCmdLine;

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
	
	Float MinValue	= min( R, min( G, B ) );
	Float MaxValue	= max( R, max( G, B ) );

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
	Int32 iProtocolEnd = String::pos(L"://", InUrl);
	if( iProtocolEnd != -1 )
	{
		Protocol = String::copy( InUrl, 0, iProtocolEnd );
		iProtocolEnd += 3;
	}
	else
	{
		// No protocol.
		Protocol = URL_DEFAULT_PROTOCOL;
	}

	// Login and password.
	Int32 iUserEnd = String::pos(L"@", InUrl, iProtocolEnd);
	if( iUserEnd != -1 )
	{
		Int32 iPasswordBegin = String::pos(L":", InUrl, iProtocolEnd);
		if( iPasswordBegin != -1 && iPasswordBegin < iUserEnd )
		{
			Login = String::copy( InUrl, iProtocolEnd, iPasswordBegin-iProtocolEnd );
			Password = String::copy( InUrl, iPasswordBegin+1, iUserEnd-iPasswordBegin-1 );
		}
		else
		{
			// No password.
			Login = String::copy( InUrl, iProtocolEnd, iUserEnd-iProtocolEnd );
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
	Int32 iFragmentBegin = String::pos( L"#", InUrl );
	if( iFragmentBegin != -1 )
	{
		Fragment = String::copy( InUrl, iFragmentBegin+1, InUrl.len()-iFragmentBegin-1 );
	}
	else
	{
		Fragment = L"";
	}

	// Port.
	Int32 iPortBegin = String::pos( L":", InUrl, max(iUserEnd, iProtocolEnd) );
	Int32 iPortEnd = iPortBegin+1;
	if( iPortBegin != -1 )
	{
		while(iPortEnd < InUrl.len() && cstr::isDigit( InUrl[iPortEnd] ))
			iPortEnd++;

		String::copy(InUrl, iPortBegin+1, iPortEnd-iPortBegin-1).toInteger( Port, 0 );
	}
	else
		Port = 0;


	// Host.
	Int32 iHostBegin = max( iUserEnd+1, iProtocolEnd );
	Int32 iHostEnd = iPortBegin != -1 ? iPortBegin : String::pos(L"/", InUrl, iHostBegin);
	Host = String::copy( InUrl, iHostBegin, iHostEnd-iHostBegin );

	// Query.
	Int32 iQueryBegin = String::pos(L"?", InUrl)+1;
	if( iQueryBegin != -1 )
	{
		Int32 iPairBegin = iQueryBegin;
		
		do 
		{
			Int32 iPairEnd = String::pos(L"&", InUrl, iPairBegin);
			if( iPairEnd == -1 )
				iPairEnd = iFragmentBegin != -1 ? iFragmentBegin : InUrl.len();
			if( iPairEnd == -1 || iPairEnd > InUrl.len() || iPairBegin >= iPairEnd )
				break;

			Int32 iMiddle = String::pos( L"=", InUrl, iPairBegin )+1;

			String Key = String::copy( InUrl, iPairBegin, iMiddle-iPairBegin-1 );
			String Value = String::copy( InUrl, iMiddle, iPairEnd-iMiddle );
			Query.Add( Key, Value );

			iPairBegin = iPairEnd+1;
		} while( true );
	}

	// Path.
	Int32 iPathBegin = max(iHostEnd+1, iPortEnd+1);
	Int32 iPathEnd = iQueryBegin != -1 ? iQueryBegin-1 : iFragmentBegin != -1 ? iFragmentBegin : InUrl.len();
	Path = String::copy( InUrl, iPathBegin, iPathEnd-iPathBegin );
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
		Result += String::format(L"%s://", *Protocol);

	if( Login )
		Result += Password ? String::format(L"%s:%s@", *Login, *Password) : String::format(L"%s@", *Login);

	Result += Host;

	if( Port != 0 )
		Result += String::format(L":%d", Port);

	Result += String(L"/") + Path;

	if( Query.Entries.size() )
	{
		Result += L"?";
		for( Int32 i=0; i<Query.Entries.size(); i++ )
			Result += String::format(L"%s%s=%s", i>0 ? L"&" : L"", *Query.Entries[i].Key, *Query.Entries[i].Value );
	}

	if( Fragment )
		Result += String::format(L"#%s", *Fragment);

	return Result;
}


//
// Compare only Url, not parameters or something.
//
Bool TUrl::operator==( const TUrl& Other ) const
{
	return	String::insensitiveCompare( Protocol, Other.Protocol ) == 0 &&
			String::insensitiveCompare( Host, Other.Host ) == 0 &&
			Port == Other.Port &&
			String::insensitiveCompare( Path, Other.Path ) == 0 &&
			String::insensitiveCompare( Fragment, Other.Fragment ) == 0;
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
	String Value = String::lowerCase(ParseStringParam( Line, Name ));

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
	Value.toInteger( Result, Default );
	return Result;
}


//
// Parse a float parameter from the line. 
//
Float CCmdLineParser::ParseFloatParam( String Line, String Name, Float Default )
{
	Float Result = 0.f;
	String Value = ParseStringParam( Line, Name );
	Value.toFloat( Result, Default );
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
		while( iCommandBegin < Line.len() && Line[iCommandBegin] == ' ' )
			iCommandBegin++;

		Int32 iCommandEnd = iCommandBegin;
		while( iCommandEnd < Line.len() && cstr::isDigitLetter( Line[iCommandEnd] ) )
			iCommandEnd++;

		if( iCommandBegin != iCommandEnd && iCommandBegin < Line.len() )
		{
			if( iArgNum != 0 )
			{
				iCommandBegin = iCommandEnd + 1;
				iArgNum--;
			}
			else
			{
				return String::copy( Line, iCommandBegin, iCommandEnd-iCommandBegin );
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
	Int32 iNameBegin = String::pos
	(
		String::lowerCase( Name ),
		String::lowerCase( Line )
	);
	if( iNameBegin == -1 )
		return Default;
	
	Int32 iNameEnd = iNameBegin + Name.len();
	if( iNameEnd >= Line.len() || Line[iNameEnd] != '=' )
		return Default;

	Int32 iValueBegin = iNameEnd + 1;
	if( iValueBegin >= Line.len() || Line[iValueBegin] == ' ' )
		return Default;

	if( Line[iValueBegin] == '"' )
	{
		Int32 iQuoteEnd = String::pos( L"\"", Line, iValueBegin+1 );
		if( iQuoteEnd != -1 )
		{
			return String::copy( Line, iValueBegin+1, iQuoteEnd-iValueBegin-1 );
		}
		else
			return Default;
	}
	else if( cstr::isDigitLetter( Line[iValueBegin] ) )
	{
		Int32 iValueEnd = iValueBegin;
		while( iValueEnd < Line.len() && cstr::isDigitLetter( Line[iValueEnd] ) )
			iValueEnd++;

		return String::copy( Line, iValueBegin, iValueEnd-iValueBegin );
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

	String LowLine = String::lowerCase(Line);
	String LowOption = String::lowerCase(Option);

	String LongOpt = String(L"--") + LowOption;
	if( String::pos( LongOpt, LowLine ) != -1 )
		return true;

	Char Tmp[2] = { **LowOption, '\0' };
	String ShortOpt = String(L"-") + Tmp;
	if( String::pos( ShortOpt, LowLine ) != -1 )
		return true;

	return false;
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/