//-----------------------------------------------------------------------------
//	CString.h: A collection of C-style string functions
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace cstr
{
	inline SizeT length( const AnsiChar* str )
	{
		return strlen( str );
	}

	inline SizeT length( const WideChar* str )
	{
		return wcslen( str );
	}

	inline Int32 compare( const AnsiChar* str1, const AnsiChar* str2 )
	{
		return strcmp( str1, str2 );
	}

	inline Int32 compare( const WideChar* str1, const WideChar* str2 )
	{
		return wcscmp( str1, str2 );
	}

	inline Int32 insensitiveCompare( const AnsiChar* str1, const AnsiChar* str2 )
	{
		return _stricmp( str1, str2 );
	}

	inline Int32 insensitiveCompare( const WideChar* str1, const WideChar* str2 )
	{
		return _wcsicmp( str1, str2 );
	}

	inline void format( AnsiChar* buffer, SizeT bufferSize, const AnsiChar* fmt, va_list args )
	{
		vsnprintf_s( buffer, bufferSize, _TRUNCATE, fmt, args );
	}

	inline void format( WideChar* buffer, SizeT bufferSize, const WideChar* fmt, va_list args )
	{
		_vsnwprintf_s( buffer, bufferSize, _TRUNCATE, fmt, args );
	}

	inline AnsiChar* findSubstring( AnsiChar* hayStack, const AnsiChar* needle )
	{
		return strstr( hayStack, needle );
	}

	inline WideChar* findSubstring( WideChar* hayStack, const WideChar* needle )
	{
		return wcsstr( hayStack, needle );
	}

	inline const AnsiChar* findChar( const AnsiChar* string, AnsiChar ch )
	{
		return strchr( string, ch );
	}

	inline const WideChar* findChar( const WideChar* string, WideChar ch )
	{
		return wcschr( string, ch );
	}

	inline const AnsiChar* findRevChar( const AnsiChar* string, AnsiChar ch )
	{
		return strrchr( string, ch );
	}

	inline const WideChar* findRevChar( const WideChar* string, WideChar ch )
	{
		return wcsrchr( string, ch );
	}

	inline AnsiChar toUpper( AnsiChar ch )
	{
		return toupper( ch );
	}

	inline WideChar toUpper( WideChar ch )
	{
		return towupper( ch );
	}

	inline AnsiChar toLower( AnsiChar ch )
	{
		return tolower( ch );
	}

	inline WideChar toLower( WideChar ch )
	{
		return towlower( ch );
	}

	inline AnsiChar* concat( AnsiChar* destBuffer, SizeT bufferSize, const AnsiChar* other )
	{
		strcat_s( destBuffer, bufferSize, other );
		return destBuffer;
	}

	inline WideChar* concat( WideChar* destBuffer, SizeT bufferSize, const WideChar* other )
	{
		wcscat_s( destBuffer, bufferSize, other );
		return destBuffer;
	}

	inline AnsiChar* copy( AnsiChar* destBuffer, SizeT destSize, const AnsiChar* srcBuffer, SizeT numSymbs )
	{
		strncpy_s( destBuffer, destSize, srcBuffer, numSymbs );
		return destBuffer;
	}

	inline WideChar* copy( WideChar* destBuffer, SizeT destSize, const WideChar* srcBuffer, SizeT numSymbs )
	{
		wcsncpy_s( destBuffer, destSize, srcBuffer, numSymbs );
		return destBuffer;
	}

	inline Bool isDigit( AnsiChar ch )
	{
		return ( ch >= '0' ) && ( ch <= '9' );
	}

	inline Bool isDigit( WideChar ch )
	{
		return ( ch >= L'0' ) && ( ch <= L'9' );
	}

	inline Bool isLetter( AnsiChar ch )
	{
		return ( ( ch >= 'A' ) && ( ch <= 'Z' ) ) || 
			( ( ch >= 'a' ) && ( ch <= 'z' ) ) || ( ch == '_' );
	}

	inline Bool isLetter( WideChar ch )
	{
		return ( ( ch >= L'A' ) && ( ch <= L'Z' ) ) || 
			( ( ch >= L'a' ) && ( ch <= L'z' ) ) || ( ch == L'_' );
	}

	inline Bool isDigitLetter( AnsiChar ch )
	{
		return isDigit( ch ) || isLetter( ch );
	}

	inline Bool isDigitLetter( WideChar ch )
	{
		return isDigit( ch ) || isLetter( ch );
	}

	inline Int32 fromHex( AnsiChar ch )
	{
		if( ch >= '0' && ch <= '9' ) return ch - '0';
		if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
		return 0;
	}

	inline Int32 fromHex( WideChar ch )
	{
		if( ch >= L'0' && ch <= L'9' ) return ch - L'0';
		if( ch >= L'a' && ch <= L'f' ) return ch - L'a' + 10;
		return 0;
	}

	inline Bool isAnsiOnly( const AnsiChar* str )
	{
		return true;
	}

	inline Bool isAnsiOnly( const WideChar* str )
	{
		for( ; *str; str++ ) if( *str > 0xff || *str < 0x00 ) return false;
		return true;
	}

	inline WideChar* multiByteToWide( WideChar* buffer, SizeT bufferSize, const AnsiChar* source )
	{
		mbstowcs_s( nullptr, buffer, bufferSize, source, _TRUNCATE );
		return buffer;
	}

	inline AnsiChar* wideToMultiByte( AnsiChar* buffer, SizeT bufferSize, const WideChar* source )
	{
		wcstombs_s( nullptr, buffer, bufferSize, source, _TRUNCATE );
		return buffer;
	}
}
}