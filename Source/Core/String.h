//-----------------------------------------------------------------------------
//	String.h: An amazing fluorine strings
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	An universal string template
	 */
	template<typename MANAGER_TYPE, MANAGER_TYPE& MANAGER> class StringBase
	{
	public:
		using CHAR_TYPE = typename MANAGER_TYPE::CHAR_TYPE;

		StringBase()
			:	m_data( nullptr )
		{
		}

		StringBase( StringBase<MANAGER_TYPE, MANAGER>&& other )
		{
			m_data = other.m_data;
			other.m_data = nullptr;
		}

		StringBase( const StringBase<MANAGER_TYPE, MANAGER>& other )
			:	m_data( other.m_data )
		{
			if( m_data )
			{
				m_data->refsCount++;
			}
		}

		StringBase( const CHAR_TYPE* other )
			:	m_data( nullptr )
		{
			if( other && *other )
			{
				SizeT realLength = cstr::length( other );
				MANAGER.initializeString( m_data, realLength );
				mem::copy( m_data->data, other, realLength * sizeof(CHAR_TYPE) );
			}
		}

		StringBase( const CHAR_TYPE* other, SizeT length )
			:	m_data( nullptr )
		{
			if( other && *other && length > 0 )
			{
				MANAGER.initializeString( m_data, length );
				mem::copy( m_data->data, other, length * sizeof(CHAR_TYPE) );
			}
		}

		StringBase( const CHAR_TYPE* firstSymbol, const CHAR_TYPE* lastSymbol )
			:	m_data( nullptr )
		{
			if( firstSymbol && lastSymbol && *firstSymbol && *lastSymbol && 
				lastSymbol > firstSymbol )
			{
				SizeT length = ( reinterpret_cast<SizeT>( lastSymbol ) - reinterpret_cast<SizeT>( firstSymbol ) ) / sizeof(CHAR_TYPE);
				MANAGER.initializeString( m_data, length );
				mem::copy( m_data->data, firstSymbol, length * sizeof(CHAR_TYPE) );
			}
		}

		~StringBase()
		{
			MANAGER.deinitializeString( m_data );
		}

		SizeT len() const
		{
			return m_data ? m_data->length : 0;
		}

		UInt32 refsCount() const
		{
			return m_data ? m_data->refsCount : 0;
		}

		UInt32 hashCode() const
		{
			UInt32 hash = 2139062143;

			if( m_data )
			{
				for( CHAR_TYPE* c = m_data->data; *c; c++ )
					hash = 37 * hash + *c;
			}

			return hash;
		}

		CHAR_TYPE* operator*() const
		{
			static const CHAR_TYPE EMPTY_STRING = CHAR_TYPE(0);
			return m_data ? m_data->data : const_cast<CHAR_TYPE*>( &EMPTY_STRING );
		}

		StringBase<MANAGER_TYPE, MANAGER>& operator=( const StringBase<MANAGER_TYPE, MANAGER>& other )
		{
			MANAGER.deinitializeString( m_data );

			if( other.m_data )
			{
				m_data = other.m_data;
				m_data->refsCount++;
			}

			return *this;
		}

		StringBase<MANAGER_TYPE, MANAGER>& operator=( const CHAR_TYPE* other )
		{
			MANAGER.deinitializeString( m_data );
			SizeT realLength = cstr::length( other );

			if( realLength )
			{
				MANAGER.initializeString( m_data, realLength );
				mem::copy( m_data->data, other, realLength * sizeof(CHAR_TYPE) );
			}

			return *this;
		}

		CHAR_TYPE operator()( SizeT i ) const
		{
			return m_data ? m_data->data[i] : 0;
		}

		const CHAR_TYPE& operator[]( SizeT i ) const
		{
			static CHAR_TYPE tmp = 0;
			return m_data ? m_data->data[i] : tmp;
		}

		CHAR_TYPE& operator[]( SizeT i )
		{
			if( m_data )
			{
				if( m_data->refsCount > 1 )
				{
					StringData* indirect = nullptr;
					MANAGER.initializeString( indirect, m_data->length );
					mem::copy( indirect->data, m_data->data, m_data->length * sizeof(CHAR_TYPE) );
					m_data->refsCount--;
					m_data = indirect;
				}

				return m_data->data[i];
			}
			else
			{
				static CHAR_TYPE junk;
				return junk;
			}
		}

		Bool operator==( const StringBase<MANAGER_TYPE, MANAGER>& other ) const
		{
			if( m_data != other.m_data )
			{
				if( !other.m_data ) return m_data == nullptr;
				if( !m_data ) return other.m_data == nullptr;

				SizeT len1 = m_data->length;
				SizeT len2 = other.m_data->length;

				if( len1 == len2 )
				{
					return cstr::compare( m_data->data, other.m_data->data ) == 0;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return true;
			}
		}

		Bool operator==( const CHAR_TYPE* other ) const
		{
			if( !other || !*other ) return m_data == nullptr;
			if( !m_data ) return false;

			SizeT len1 = m_data->length;
			SizeT len2 = cstr::length( other );

			if( len1 == len2 )
			{
				return cstr::compare( m_data->data, other ) == 0;
			}
			else
			{
				return false;
			}
		}

		Bool operator!=( const StringBase<MANAGER_TYPE, MANAGER>& other ) const
		{
			return !operator==( other );
		}

		Bool operator!=( const CHAR_TYPE* other ) const
		{
			return !operator==( other );
		}

		Bool operator>( const StringBase<MANAGER_TYPE, MANAGER>& other ) const
		{
			return m_data && other.m_data ? cstr::compare( m_data->data, other.m_data->data ) > 0 : m_data != other.m_data;
		}

		Bool operator<( const StringBase<MANAGER_TYPE, MANAGER>& other ) const
		{
			return m_data && other.m_data ? cstr::compare( m_data->data, other.m_data->data ) < 0 : m_data != other.m_data;
		}

		Bool operator>=( const StringBase<MANAGER_TYPE, MANAGER>& other ) const
		{
			return m_data && other.m_data ? cstr::compare( m_data->data, other.m_data->data ) >= 0 : m_data == other.m_data;
		}

		Bool operator<=( const StringBase<MANAGER_TYPE, MANAGER>& other ) const
		{
			return m_data && other.m_data ? cstr::compare( m_data->data, other.m_data->data ) <= 0 : m_data == other.m_data;
		}

		StringBase<MANAGER_TYPE, MANAGER>& operator+=( const CHAR_TYPE* other )
		{
			if( SizeT len2 = cstr::length( other ) )
			{
				if( m_data )
				{
					SizeT len1 = m_data->length;
					SizeT resultLen = len1 + len2;
					MANAGER.reinitializeString( m_data, resultLen );
					mem::copy( &m_data->data[len1], other, len2 * sizeof(CHAR_TYPE) );
				}
				else
				{
					*this = other;
				}

			}

			return *this;
		}

		StringBase<MANAGER_TYPE, MANAGER>& operator+=( const StringBase<MANAGER_TYPE, MANAGER>& other )
		{
			if( SizeT len2 = other.len() )
			{
				if( m_data )
				{
					SizeT len1 = m_data->length;
					SizeT resultLen = len1 + len2;
					MANAGER.reinitializeString( m_data, resultLen );
					mem::copy( &m_data->data[len1], other.m_data->data, len2 * sizeof(CHAR_TYPE) );
				}
				else
				{
					*this = other;
				}
			}

			return *this;
		}

		StringBase<MANAGER_TYPE, MANAGER> operator+( const CHAR_TYPE* other ) const
		{
			return StringBase<MANAGER_TYPE, MANAGER>( *this ) += other;
		}

		StringBase<MANAGER_TYPE, MANAGER> operator+( const StringBase<MANAGER_TYPE, MANAGER>& other ) const
		{
			return operator+( *other );
		}

		operator Bool() const
		{
			return m_data != nullptr;
		}

		template<typename INT_TYPE> Bool toInteger( INT_TYPE& value, INT_TYPE _default = 0 ) const
		{
			if( !len() )
			{
				value = _default;
				return false;
			}
		
			SizeT charId = 0;
			Bool isNegative = false;
			value = _default;

			if( m_data->data[0] == '-' )
			{
				++charId;
				isNegative = true;
			}
			else if( m_data->data[0] == '+' )
			{
				++charId;
				isNegative = false;
			}

			INT_TYPE result = 0;
			for( SizeT i = charId; i < len(); ++i )
			{
				if( cstr::isDigit( m_data->data[i] ) )
				{
					result *= 10;
					result += static_cast< INT_TYPE >( m_data->data[i] - '0' );
				}
				else
				{
					return false;
				}
			}

			value = isNegative ? -result : result;
			return true;
		}

		Bool toFloat( Float& value, Float _default = 0.f ) const
		{
			if( !len() )
			{
				value = _default;
				return false;
			}

			SizeT charId = 0;
			Bool isNegative = false;
			Float fracPart = 0.f;
			Float ceilPart = 0.f;
			value = _default;

			if( m_data->data[0] == '-' )
			{
				charId++;
				isNegative = true;
			}
			else if( m_data->data[0] == '+' )
			{
				charId++;
				isNegative = false;
			}

			if( charId < len() )
			{
				if( m_data->data[charId] == '.' )
				{
				ParseFrac:
					charId++;
					Float m = 0.1f;

					for( ; charId < len(); ++charId )
					{
						if( cstr::isDigit( m_data->data[charId] ) )
						{
							fracPart += static_cast<Int32>( m_data->data[charId] - '0' ) * m;
							m /= 10.f;
						}
						else
						{
							return false;
						}
					}
				}
				else if( cstr::isDigit( m_data->data[charId] ) )
				{
					for( ; charId < len(); ++charId )
					{
						if( cstr::isDigit( m_data->data[charId] ) )
						{
							ceilPart *= 10.f;
							ceilPart += static_cast<Int32>( m_data->data[charId] - '0' );
						}
						else if( m_data->data[charId] == '.' )
						{
							goto ParseFrac;
						}
						else
						{
							return false;
						}
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			value = isNegative ? -( ceilPart + fracPart ) : +( ceilPart + fracPart );
			return true;
		}

		static Array<StringBase<MANAGER_TYPE, MANAGER>> wrapText( StringBase<MANAGER_TYPE, MANAGER> text, UInt32 maxColumnSize )
		{
			Array<StringBase<MANAGER_TYPE, MANAGER>> result;

			SizeT i = 0;
			do
			{
				SizeT cleanWordEnd = 0;
				Bool gotWord = false;
				SizeT testWord = 0;

				while( ( i + testWord < text.len() ) && ( text(i + testWord) != '\n' ) )
				{
					if( testWord++ > maxColumnSize )
					{
						break;
					}

					Bool wordBreak = ( text(testWord + i) == ' ' ) || 
						( text(testWord + i) == '\n' ) || ( testWord + i >= text.len() );

					if( wordBreak || !gotWord )
					{
						cleanWordEnd = testWord;
						gotWord = gotWord || wordBreak;
					}
				}

				if( cleanWordEnd == 0 )
				{
					break;
				}
				else
				{
					result.push( copy( text, i, cleanWordEnd ) );
					i += cleanWordEnd;
				}

				while( text(i) == ' ' )
				{
					++i;
				}

			} while( i < text.len() );

			return result;
		}

		static StringBase<MANAGER_TYPE, MANAGER> format( StringBase<MANAGER_TYPE, MANAGER> fmt, ... )
		{
			Char buffer[4096];
			va_list argsPtr;

			va_start( argsPtr, fmt );
			cstr::format( buffer, arraySize( buffer ), *fmt, argsPtr );
			va_end( argsPtr );

			return buffer;
		}

		static StringBase<MANAGER_TYPE, MANAGER> upperCase( StringBase<MANAGER_TYPE, MANAGER> string )
		{
			if( string )
			{
				StringBase<MANAGER_TYPE, MANAGER> newString;
				MANAGER.initializeString( newString.m_data, string.m_data->length );

				for( SizeT i = 0; i < newString.m_data->length; ++i )
				{
					newString.m_data->data[i] = cstr::toUpper( string.m_data->data[i] );
				}

				return newString;
			}
			else
			{
				return string;
			}
		}

		static StringBase<MANAGER_TYPE, MANAGER> lowerCase( StringBase<MANAGER_TYPE, MANAGER> string )
		{
			if( string )
			{
				StringBase<MANAGER_TYPE, MANAGER> newString;
				MANAGER.initializeString( newString.m_data, string.m_data->length );

				for( SizeT i = 0; i < newString.m_data->length; ++i )
				{
					newString.m_data->data[i] = cstr::toLower( string.m_data->data[i] );
				}

				return newString;
			}
			else
			{
				return string;
			}
		}

		static StringBase<MANAGER_TYPE, MANAGER> ofChar( CHAR_TYPE repeat, SizeT count )
		{
			if( count )
			{
				StringBase<MANAGER_TYPE, MANAGER> newString;
				MANAGER.initializeString( newString.m_data, count );

				for( SizeT i = 0; i < count; ++i )
				{
					newString.m_data->data[i] = repeat;
				}

				return newString;
			}
			else
			{
				return StringBase<MANAGER_TYPE, MANAGER>();
			}
		}

		static Int32 pos( StringBase<MANAGER_TYPE, MANAGER> needle, StringBase<MANAGER_TYPE, MANAGER> hayStack, Int32 iFrom = 0 )
		{
			const CHAR_TYPE* result = cstr::findSubstring( &((*hayStack)[iFrom]), *needle );

			if( result )
			{
				SizeT byteOffset = reinterpret_cast<SizeT>( result ) - reinterpret_cast<SizeT>( *hayStack );
				return static_cast<Int32>( byteOffset / sizeof( CHAR_TYPE ) );
			}
			else
			{
				return -1;
			}
		}

		static StringBase<MANAGER_TYPE, MANAGER> copy( StringBase<MANAGER_TYPE, MANAGER> source, Int32 startChar, Int32 count )
		{
			startChar = clamp<Int32>( startChar, 0, source.len() - 1 );
			count = clamp<Int32>( count, 0, source.len() - startChar );

			if( count )
			{
				return StringBase<MANAGER_TYPE, MANAGER>( &source.m_data->data[startChar], count );
			}
			else
			{
				return StringBase<MANAGER_TYPE, MANAGER>();
			}
		}

		static StringBase<MANAGER_TYPE, MANAGER> del( StringBase<MANAGER_TYPE, MANAGER> source, Int32 startChar, Int32 count )
		{
			startChar = clamp<Int32>( startChar, 0, source.len() - 1 );
			count = clamp<Int32>( count, 0, source.len() - startChar );

			return copy( source, 0, startChar ) + 
				copy( source, startChar + count, source.len() - startChar - count );
		}

		static Int32 insensitiveCompare( StringBase<MANAGER_TYPE, MANAGER> str1, StringBase<MANAGER_TYPE, MANAGER> str2 )
		{
			return cstr::insensitiveCompare( *str1, *str2 );
		}

		static Int32 compare( StringBase<MANAGER_TYPE, MANAGER> str1, StringBase<MANAGER_TYPE, MANAGER> str2 )
		{
			return cstr::compare( *str1, *str2 );
		}

		static StringBase<MANAGER_TYPE, MANAGER> fromInteger( Int32 value )
		{
			return format( L"%i", value );
		}

		static StringBase<MANAGER_TYPE, MANAGER> fromFloat( Float value )
		{
			return format( L"%.4f", value );
		}

		friend IOutputStream& operator<<( IOutputStream& stream, const StringBase<MANAGER_TYPE, MANAGER>& x )
		{
			const Bool saveAsBlob = sizeof( CHAR_TYPE ) == sizeof( AnsiChar ) || !cstr::isAnsiOnly( *x );

			if( saveAsBlob )
			{
				Int32 length = static_cast<Int32>( x.len() );
				stream << length;

				if( length > 0 )
				{
					stream.writeData( *x, length * sizeof( CHAR_TYPE ) );
				}
			}
			else
			{
				Int32 length = -static_cast<Int32>( x.len() );
				stream << length;

				for( SizeT i = 0; i < x.len(); ++i )
				{
					AnsiChar tmp = static_cast<AnsiChar>( x(i) );
					stream << tmp;
				}
			}

			return stream;
		}

		friend IInputStream& operator>>( IInputStream& stream, StringBase<MANAGER_TYPE, MANAGER>& x )
		{
			Int32 length;
			stream >> length;

			if( length > 0 )
			{
				MANAGER.deinitializeString( x.m_data );
				MANAGER.initializeString( x.m_data, length );
				stream.readData( *x, sizeof( CHAR_TYPE ) * length );
			}
			else
			{
				MANAGER.deinitializeString( x.m_data );
				MANAGER.initializeString( x.m_data, -length );

				for( SizeT i = 0; i < x.len(); ++i )
				{
					AnsiChar tmp;
					stream >> tmp;
					(*x)[i] = tmp;
				}
			}

			return stream;
		}

		// legacy
		friend void Serialize( CSerializer& s, StringBase<MANAGER_TYPE, MANAGER>& v )
		{
			if( s.GetMode() == SM_Load )
			{
				Int32 length;
				Serialize( s, length );

				if( length > 0 )
				{
					MANAGER.deinitializeString( v.m_data );
					MANAGER.initializeString( v.m_data, length );
					s.SerializeData( *v, sizeof(CHAR_TYPE) * length );
				}
				else if( length < 0 )
				{
					MANAGER.deinitializeString( v.m_data );
					MANAGER.initializeString( v.m_data, -length );

					for( SizeT i = 0; i < v.len(); ++i )
					{
						AnsiChar tmp;
						Serialize( s, tmp );
						(*v)[i] = tmp;
					}
				}
			}
			else if( s.GetMode() == SM_Save )
			{
				const Bool saveAsBlob = sizeof(CHAR_TYPE) == sizeof(AnsiChar) || !cstr::isAnsiOnly( *v );

				if( saveAsBlob )
				{
					Int32 length = static_cast<Int32>( v.len() );
					Serialize( s, length );

					if( length > 0 )
					{
						s.SerializeData( *v, length * sizeof(CHAR_TYPE) );
					}
				}
				else
				{
					Int32 length = -static_cast<Int32>( v.len() );
					Serialize( s, length );

					for( SizeT i = 0; i < v.len(); ++i )
					{
						AnsiChar tmp = static_cast<AnsiChar>( v(i) );
						Serialize( s, tmp );
					}
				}
			}
			else
			{
				Int32 length = static_cast<Int32>( v.len() );
				Serialize( s, length );

				if( length > 0 )
				{
					s.SerializeData( *v, sizeof(CHAR_TYPE) * length );
				}
			}
		}

	protected:
		using StringData = typename MANAGER_TYPE::StringData;

		StringData* m_data;
	};

#if 0
	/**
	 *	An Ansi String
	 */
	class AnsiString: public StringBase<AnsiStringManager, g_ansiStringManager>
	{
	public:
		// todo: add conversation functions here
	};

	/**
	 *	A Wide String
	 */
	class WideString: public StringBase<WideStringManager, g_wideStringManager>
	{
	public:
		// todo: add conversation functions here
	};
#else
	using AnsiString = StringBase<AnsiStringManager, g_ansiStringManager>;
	using WideString = StringBase<WideStringManager, g_wideStringManager>;
#endif

#if FLU_USE_WIDECHAR
	using String = WideString;
#else
	using String = AnsiString;
#endif

	// Strings convertion
	inline AnsiString wide2AnsiString( WideString source )
	{
		const auto bufferSize = sizeof(AnsiChar) * 2 * ( source.len() + 1 );
		auto buffer = reinterpret_cast<AnsiChar*>( mem::alloc( bufferSize ) );

		AnsiString result = cstr::wideToMultiByte( buffer, bufferSize, *source );

		mem::free( buffer );
		return result;
	}

	inline WideString ansi2WideString( AnsiString source )
	{
		const auto bufferSize = sizeof(WideChar) * 2 * ( source.len() + 1 );
		auto buffer = reinterpret_cast<WideChar*>( mem::alloc( bufferSize ) );

		WideString result = cstr::multiByteToWide( buffer, bufferSize / sizeof(WideChar), *source );

		mem::free( buffer );
		return result;
	}
}