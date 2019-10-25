/*=============================================================================
    FrCore.cpp: Various engine core functions.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "..\Engine.h"

//
// Globals.
//
CPlatformBase*		GPlat		= nullptr;
UInt32				GFrameStamp = 0;

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
			Query.put( Key, Value );

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

	if( !Query.isEmpty() )
	{
		Result += L"?";
		for( const auto& it : Query )
			Result += String::format(L"%s%s=%s", &it != Query.begin() ? L"&" : L"", *it.key, *it.value );
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