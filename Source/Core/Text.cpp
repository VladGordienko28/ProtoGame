//-----------------------------------------------------------------------------
//	Text.cpp: An abstract text reading/writing interface
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Core.h"

namespace flu
{
	Text::Text()
		:	m_lines()
	{
	}

	Text::Text( const Char* other )
		:	m_lines()
	{
		internalAppend( other );
	}
	
	Text::Text( String other )
		:	m_lines()
	{
		internalAppend( *other );
	}
	
	Text::Text( const Array<String>& other )
		:	m_lines()
	{
		for( const String& it : other )
		{
			internalAppend( *it );
		}
	}
	
	Text::~Text()
	{
		empty();
	}
	
	void Text::appendLine( String newLine )
	{
		internalAppend( *newLine );
	}
	
	void Text::appendLine( const Char* newLine )
	{
		internalAppend( newLine );
	}
	
	void Text::empty()
	{
		m_lines.empty();
	}
	
	String Text::toString() const
	{
		String result;

		for( Int32 i = 0; i < m_lines.size(); ++i )
		{
			result += m_lines[i];

			if( i != m_lines.size() - 1 )
			{
				result += L"\n";
			}
		}

		return result;
	}

	void Text::internalAppend( const Char* newLine )
	{
		if( newLine && *newLine )
		{
			const Char* buffer = newLine;

			for( ; ; )
			{
				const Char* lineEnd = cstr::findChar( buffer, '\n' );

				if( lineEnd == nullptr )
				{
					if( buffer && *buffer )
					{
						m_lines.push( buffer );
					}
					break;
				}
				else
				{
					m_lines.push( String( buffer, lineEnd ) );
					buffer = lineEnd + 1;
				}
			}
		}
	}
}