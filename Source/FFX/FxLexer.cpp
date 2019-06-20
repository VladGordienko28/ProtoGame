//-----------------------------------------------------------------------------
//	FxLexer.h: A ffx source lexer implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "FFX.h"

namespace flu
{
namespace ffx
{
	Lexer::Lexer( Text::Ptr text )
		:	lexer::Lexer( text, getDefaultConfig() )
	{
	}

	Lexer::~Lexer()
	{
	}

	Char Lexer::getChar()
	{
		lexer::Position oldPosition;
		Char result;

	Loop:
		result = getRawChar();
		oldPosition = m_lastPosition;

		if( result == L'/' )
		{
			Char c = getRawChar();
			ungetChar();

			if( c == L'/' )
			{
				m_position.line++;
				m_position.pos = 0;

				goto Loop;
			}
			else if( c == L'*' )
			{
				while( true )
				{
					c = getRawChar();

					if( c == L'*' )
					{
						Char d = getRawChar();
						ungetChar();

						if( d == L'/' )
						{
							getRawChar();
							goto Loop;
						}
					}
					else if( c == L'\0' )
					{
						return c;
					}
				}
			}
			else
			{
				m_lastPosition = oldPosition;
			}
		}

		return result;
	}

	const lexer::LexerConfig Lexer::getDefaultConfig()
	{
		lexer::LexerConfig config;

		config.m_dualSymbols.push( L"==" );
		config.m_dualSymbols.push( L"!=" );
		config.m_dualSymbols.push( L"+=" );
		config.m_dualSymbols.push( L"-=" );
		config.m_dualSymbols.push( L"*=" );
		config.m_dualSymbols.push( L"/=" );
		config.m_dualSymbols.push( L"++" );
		config.m_dualSymbols.push( L"--" );

		config.m_literalString = L'\"';

		return config;
	}
}
}