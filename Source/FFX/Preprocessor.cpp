//-----------------------------------------------------------------------------
//	Preprocessor.h: A ffx compiler preprocessor implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "FFX.h"

namespace flu
{
namespace ffx
{
	/**
	 *	A helping preprocessor context
	 */
	struct PreprocessorContext
	{
	public:
		Map<String, String> defines;
		Array<String> includeStack;
		Array<String> dependencies;
	};

	// forward declaration
	Bool parseFile( String fileName, Text::Ptr text, TextWriter& writer, const PreprocessorInput& input, 
		PreprocessorContext& context, String& error );


	Bool processInclude( lexer::Lexer& lexer, TextWriter& writer, const PreprocessorInput& input, 
		PreprocessorContext& context, String& error )
	{
		String path;
		if( lexer.readString( path ) )
		{
			Text::Ptr includeText = input.includeProvider->getInclude( path );

			if( includeText )
			{
				if( !parseFile( path, includeText, writer, input, context, error ) )
				{
					return false;
				}
			}
			else
			{
				error = String::format( TEXT( "File \"%s\" is not found" ), *path );
				return false;
			}
		}
		else
		{
			error = TEXT( "Missing include path" );
			return false;
		}

		return true;
	}

	Bool parseFile( String fileName, Text::Ptr text, TextWriter& writer, const PreprocessorInput& input, 
		PreprocessorContext& context, String& error )
	{
		for( auto it : context.includeStack )
		{
			if( String::insensitiveCompare( fileName, it ) == 0 )
			{
				error = String::format( TEXT( "Circular dependency in \"%s\" found" ), *fileName );
				return false;
			}
		}

		context.includeStack.push( fileName );
		{
			ffx::Lexer lexer( text );

			lexer::Token prevToken;
			Int32 prevLine = 0;

			while( !lexer.isEof() )
			{
				lexer::Token token;

				if( !lexer.getToken( token, false ) )
				{
					break;
				}

				if( token.getType() == lexer::ETokenType::Symbol && token.getText() == TEXT( "#" ) )
				{
					String directive = lexer.getIdentifier();

					if( directive == TEXT( "include" ) )
					{
						if( !processInclude( lexer, writer, input, context, error ) )
						{
							return false;
						}
					}
					else
					{
						error = String::format( TEXT( "Unrecognized directive \"%s\"" ), *directive );
						return false;
					}
				}
				else
				{



					Char extraSymbol = (token.getType() != lexer::ETokenType::Symbol && prevToken.getType() != lexer::ETokenType::Symbol) ? ' ' : '\0';
					if( prevLine != token.getLine() )
					{
						if( input.emitLines )
						{
							writer << String::format( TEXT("\n#line %d \"%s\""), token.getLine() + 1, *fileName );
						}

						extraSymbol = '\n';
						prevLine = token.getLine();
					}

					writer << extraSymbol << token.getText();				
				
				
					
					prevToken = token;
				}
			}
		}
		verify( context.includeStack.pop() == fileName );

		context.dependencies.addUnique( fileName );
		return true;
	}

	Bool preprocess( const PreprocessorInput& input, PreprocessorOutput& output, String* outError )
	{
		assert( input.fileName );
		assert( input.source );
		assert( input.includeProvider );

		PreprocessorContext context;
		context.defines = input.defines;
		context.includeStack.empty();

		TextWriter writer;
		String error;

		if( parseFile( input.fileName, input.source, writer, input, context, error ) )
		{
			assert( context.includeStack.size() == 0 );

			output.source = writer.getText();
			output.dependencies = context.dependencies;
			return true;
		}
		else
		{
			if( outError )
			{
				*outError = error;
			}

			return false;
		}
	}
}
}