//-----------------------------------------------------------------------------
//	Lexer.cpp: Lexer implementation
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Core/Core.h"

namespace flu
{
namespace lexer
{
	Lexer::Lexer( Text::Ptr text, const LexerConfig& config )
		:	m_text( text ),
			m_config( config )
	{
		assert( text.hasObject() );
		reset();
	}

	Lexer::~Lexer()
	{
	}

	void Lexer::reset()
	{
		m_position.line = m_lastPosition.line = 0;
		m_position.pos = m_lastPosition.pos = 0;
	}

	Bool Lexer::getToken( Token& outToken, Bool allowNegative, Bool allowFloat )
	{
		Char buffer[MAX_TOKEN_LENGHT] = {};
		Char* bufferPtr = buffer;
		const Char* bufferEnd = buffer + MAX_TOKEN_LENGHT - 1;
		Char c;

		// skip all meaningless characters
		do
		{
			c = getChar();
		} 
		while( ( c == 0x20 ) || ( c == 0x09 ) ||
			( c == 0x0d ) || ( c == 0x0a ) );

		Position tokenPosition = m_lastPosition;

		if( cstr::isDigit( c ) )
		{
			// numberic constant
			Bool hasPoint = false;

			while( bufferPtr != bufferEnd && ( cstr::isDigit( c ) || c == '.' || c == 'f' ) )
			{
				if( c == 'f' )
				{
					c = '0';
				}

				if( c == '.' )
				{
					if( !allowFloat )
					{
						break;
					}

					if( hasPoint )
					{
						return false;
					}
					else
					{
						hasPoint = true;
					}
				}

				*bufferPtr++ = c;

				c = getChar();
			}
			ungetChar();

			if( hasPoint )
			{
				// float constant
				Float value;
				String text = buffer;

				if( text.toFloat( value, 0.f ) )
				{
					outToken = Token( value, text, tokenPosition );
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				// integer constant
				Int32 value;
				String text = buffer;

				if( text.toInteger( value, 0 ) )
				{
					outToken = Token( value, text, tokenPosition );
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		else if( cstr::isLetter( c ) )
		{
			// identifier
			while( bufferPtr != bufferEnd && cstr::isDigitLetter( c ) )
			{
				*bufferPtr++ = c;
				c = getChar();
			}
			ungetChar();

			outToken = Token( ETokenType::Identifier, buffer, tokenPosition );
			return true;
		}
		else if( c != 0 )
		{
			// symbol or something which is starts with
			const Char d = getChar();
			const Char dualSymbol[3] = { c, d, '\0' };

			if( m_config.m_dualSymbols.find( dualSymbol ) != -1 )
			{
				buffer[0] = c;
				buffer[1] = d;
			}
			else
			{
				buffer[0] = c;
				ungetChar();
			}

			String tokenText = buffer;

			if( tokenText == L"-" && allowNegative )
			{
				// negative numberic value
				Token numberToken;
				getToken( numberToken, false );

				if( numberToken.getType() == ETokenType::Float )
				{
					outToken = Token( -numberToken.getFloatConst(), tokenText + numberToken.getText(), tokenPosition );
					return true;
				}
				else if( numberToken.getType() == ETokenType::Integer )
				{
					outToken = Token( -numberToken.getIntConst(), tokenText + numberToken.getText(), tokenPosition );
					return true;
				}
				else
				{
					gotoToken( numberToken );
					outToken = Token( ETokenType::Symbol, tokenText, tokenPosition );
					return true;
				}
			}
			else if( tokenText( 0 ) == m_config.m_literalString )
			{
				// string constant
				do
				{
					c = getChar();

					if( bufferPtr == bufferEnd || c == m_config.m_literalString || c == '\0' )
					{
						break;
					}

					*bufferPtr++ = c;
				}
				while( true );

				outToken = Token( buffer, String::format( L"\"%s\"", buffer ), tokenPosition );
				return true;
			}
			else
			{
				// just a symbol
				outToken = Token( ETokenType::Symbol, tokenText, tokenPosition );
				return true;
			}
		}
		else
		{
			// eof reached
			return false;
		}
	}

	Bool Lexer::peekToken( Token& outToken, Bool allowNegative )
	{
		if( getToken( outToken, allowNegative ) )
		{
			gotoToken( outToken );
			return true;
		}
		else
		{
			return false;
		}
	}

	void Lexer::gotoToken( const Token& token )
	{
		assert( token.getType() != ETokenType::Unknown );

		m_position.line = m_lastPosition.line = token.getPosition().line;
		m_position.pos = m_lastPosition.pos = token.getPosition().pos;
	}

	Bool Lexer::readInt( Int32& result, Int32 _default )
	{
		Token token;

		if( getToken( token, true, false ) && token.getType() == ETokenType::Integer )
		{
			result = token.getIntConst();
			return true;
		}
		else
		{
			result = _default;
			return false;
		}
	}

	Bool Lexer::readFloat( Float& result, Float _default )
	{
		Token token;

		if( getToken( token, true ) && token.getType() == ETokenType::Integer ||
			token.getType() == ETokenType::Float )
		{
			result = token.getFloatConst();
			return true;
		}
		else
		{
			result = _default;
			return false;
		}
	}

	Bool Lexer::readString( String& result, const Char* _default )
	{
		Token token;

		if( getToken( token, true ) && token.getType() == ETokenType::String )
		{
			result = token.getStringConst();
			return true;
		}
		else
		{
			result = _default;
			return false;
		}
	}

	String Lexer::peekIdentifier()
	{
		Token token;
		getToken( token, false );
		gotoToken( token );

		return token.getType() == ETokenType::Identifier ? token.getText() : L"";
	}

	String Lexer::peekSymbol()
	{
		Token token;
		getToken( token, false );
		gotoToken( token );

		return token.getType() == ETokenType::Symbol ? token.getText() : L"";
	}

	String Lexer::getIdentifier()
	{
		Token token;

		if( getToken( token, false ) && token.getType() == ETokenType::Identifier )
		{
			return token.getText();
		}
		else
		{
			return L"";
		}
	}

	Bool Lexer::matchIdentifier( const Char* identifier, Bool matchCase )
	{
		Token token;

		if( getToken( token, false ) )
		{
			if( token.getType() == ETokenType::Identifier )
			{
				if( matchCase )
				{
					if( String::compare( token.getText(), identifier ) == 0 )
					{
						return true;
					}
				}
				else
				{
					if( String::insensitiveCompare( token.getText(), identifier ) == 0 )
					{
						return true;
					}
				}
			}
			
			gotoToken( token );
			return false;
		}
		else
		{
			return false;
		}
	}

	Bool Lexer::matchSymbol( const Char* symbol )
	{
		Token token;

		if( getToken( token, false ) )
		{
			if( token.getType() == ETokenType::Symbol &&
				token.getText() == symbol )
			{
				return true;
			}
			else
			{
				gotoToken( token );
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	Bool Lexer::isEof()
	{
		Char nextChar = getChar();
		ungetChar();

		return nextChar == '\0';
	}

	Char Lexer::getRawChar()
	{
		m_lastPosition = m_position;

		// if end of file reached just return '\0'
		if( m_position.line >= m_text->size() )
		{
			return '\0';
		}

		// if end of line reached just move to the next line
		// and return some separation symbol
		const String& textLine = (*m_text)[m_position.line];
		if( m_position.pos >= static_cast<Int32>( textLine.len() ) )
		{
			m_position.pos = 0;
			++m_position.line;
			return ' ';
		}

		return textLine( m_position.pos++ );
	}

	void Lexer::ungetChar()
	{
		m_position.line = m_lastPosition.line;
		m_position.pos = m_lastPosition.pos;
	}
}
}