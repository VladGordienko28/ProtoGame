//-----------------------------------------------------------------------------
//	FxCompiler.cpp: A ffx compiler implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "FFX.h"

namespace flu
{
namespace ffx
{
	// list of all ffx keywords
	#define KEYWORD( word ) static const Char KW_##word[] = TEXT( #word );
	#include "FxKeyword.h"
	#undef KEYWORD

	Compiler::Compiler( rend::Device* device )
		:	m_device( device )
	{
		assert( device );
	}

	Compiler::~Compiler()
	{
	}

	Bool Compiler::isSupportedFile( String relativePath ) const
	{
		String ext = fm::getFileExt( *relativePath );
		return ext == TEXT("ffx");
	}

	Bool Compiler::compile( String relativePath, res::IDependencyProvider& dependencyProvider, 
		res::CompilationOutput& output ) const
	{
		assert( relativePath );

		// create code emitter
		UserBufferWriter emitter( output.compiledResource.data );

		// obtain api compiler
		rend::ShaderCompiler::UPtr apiCompiler = m_device->createCompiler();
		assert( apiCompiler );

		// emit header
		emitter << String( FFX_VERSION );
		emitter << apiCompiler->compilerMark();

		// parse all files and compile all FFX stuff
		Context context( output );

		if( !parseFile( relativePath, dependencyProvider, context ) )
		{
			return false;
		}

		// emit vertex_decl
		if( context.vertexDecl.isValid() )
		{
			emitter << context.vertexDecl;
		}
		else
		{
			output.errorMsg = TEXT( "Missing vertex declaration in shader" );
			return false;
		}

		// compile pixel shader
		auto compiledPS = apiCompiler->compile( rend::EShaderType::Pixel, 
			context.writer.getText(), Effect::PS_ENTRY, nullptr, &output.errorMsg );

		if( !compiledPS.isValid() )
		{
			return false;
		}

		// compile vertext shader
		auto compiledVS = apiCompiler->compile( rend::EShaderType::Vertex, 
			context.writer.getText(), Effect::VS_ENTRY, nullptr, &output.errorMsg );

		if( !compiledVS.isValid() )
		{
			return false;
		}

		emitter << compiledPS;
		emitter << compiledVS;

		return true;
	}

	// A compiler macro, which helps form a error message
	#define compiler_error( msg, ... )\
		context.output.errorMsg = String::format( TEXT( "%s(%d): " ) TEXT( msg ),\
		*context.inclusionStack.last(), lexer.getPosition().line + 1, __VA_ARGS__ );

	Bool Compiler::parseFile( String relativePath, 
		res::IDependencyProvider& dependencyProvider, Context& context ) const
	{
		// first of all, check for circular dependency
		for( auto& it : context.inclusionStack )
		{
			if( String::insensitiveCompare( relativePath, it ) == 0 )
			{
				context.output.errorMsg = String::format( TEXT( "Circular dependency is found in \"%s\"" ), *relativePath );
				return false;
			}
		}

		// read source ffx text
		Text::Ptr text = dependencyProvider.getTextFile( relativePath );
		if( !text )
		{
			context.output.errorMsg = String::format( TEXT( "File \"%s\" is not found" ), *relativePath );
			return false;
		}

		// parse all lines
		context.inclusionStack.push( relativePath );
		{
			ffx::Lexer lexer( text );

			lexer::Token prevToken;
			Int32 prevLineNum = 0;

			while( !lexer.isEof() )
			{
				lexer::Token token;

				if( !lexer.getToken( token, false ) )
				{
					// no tokens remain
					break;
				}

				if( token.getType() == lexer::ETokenType::Symbol && 
					token.getText() == TEXT( "#" ) )
				{
					// parse directive
					if( !parseDirective( lexer, dependencyProvider, context ) )
					{
						return false;
					}
				}
				else if( token.getText() == KW_vertex_decl )
				{
					// compile vertex declaration
					if( !compileVertexDecl( lexer, context ) )
					{
						return false;
					}
				}
				else
				{
					// everything else
					Char insertSymbol = ( token.getType() != lexer::ETokenType::Symbol && 
						prevToken.getType() != lexer::ETokenType::String ) ? TEXT(' ') : TEXT('\0');

					if( prevLineNum != token.getLine() )
					{
						if( EMIT_LINES )
						{
							context.writer << String::format( TEXT("\n#line %d \"%s\""), 
								token.getLine() + 1, *relativePath );
						}
					
						insertSymbol = TEXT('\n');
						prevLineNum = token.getLine();
					}

					context.writer << insertSymbol << token.getText();	
					prevToken = token;
				}
			}
		}
		verify( context.inclusionStack.pop() == relativePath );

		return true;
	}

	Bool Compiler::parseDirective( lexer::Lexer& lexer, 
		res::IDependencyProvider& dependencyProvider, Context& context ) const
	{
		String directive = lexer.getIdentifier();
		if( !directive )
		{
			compiler_error( "Missing directive" );
			return false;
		}

		if( directive == KW_include )
		{
			// include directive
			String path;
			if( lexer.readString( path ) )
			{
				return parseFile( path, dependencyProvider, context );
			}
			else
			{
				compiler_error( "Missing include path" );
				return false;
			}
		}
		else
		{
			// unknown directive
			compiler_error( "Unrecognized directive \"%s\"", *directive );
			return false;
		}
	}

	Bool Compiler::compileVertexDecl( lexer::Lexer& lexer, Context& context ) const
	{
		// check for redeclaration
		if( context.vertexDecl.isValid() )
		{
			compiler_error( "Redefinition of VertexDeclaration" );
			return false;
		}

		String name = lexer.getIdentifier();
		if( !name )
		{
			compiler_error( "Missing vertex_decl name" );
			return false;
		}
		if( !lexer.matchSymbol( TEXT( "{" ) ) )
		{
			compiler_error( "Missing vertex_decl declaration" );
			return false;
		}

		// create real vertex decl
		rend::VertexDeclaration vertexDecl( name );
	
		// parse element by element
		do
		{
			lexer::Token token;
			if( !lexer.getToken( token, false ) || token.getType() != lexer::ETokenType::Symbol )
			{
				compiler_error( "Missing vertex_decl element declaration" );
				return false;
			}
			if( token.getText() == TEXT("}") )
			{
				break;
			}

			if( token.getText() == TEXT("[") )
			{
				// parse format
				String formatName;
				lexer.readString( formatName );
				rend::EFormat format = rend::getFormatByName( formatName );

				if( format == rend::EFormat::Unknown )
				{
					compiler_error( "Unknown vertex format \"%s\"", *formatName );
					return false;
				}
				if( !lexer.matchSymbol( TEXT(",") ) )
				{
					compiler_error( "Missing \",\" in element declaration" );
					return false;
				}

				// parse usage
				String usageName;
				lexer.readString( usageName );
				rend::EVertexElementUsage usage = rend::getVertexElementUsageByName( usageName );

				if( usage == rend::EVertexElementUsage::MAX )
				{
					compiler_error( "Unknown vertex element usage \"%s\"", *usageName );
					return false;
				}
				if( !lexer.matchSymbol( TEXT(",") ) )
				{
					compiler_error( "Missing \",\" in element declaration" );
					return false;
				}

				// parse usage index
				Int32 usageIndex;
				if( !lexer.readInt( usageIndex ) )
				{
					compiler_error( "Missing usage index for vertex element" );
					return false;
				}
				if( !lexer.matchSymbol( TEXT(",") ) )
				{
					compiler_error( "Missing \",\" in element declaration" );
					return false;
				}

				// parse offset
				Int32 offset;
				if( !lexer.readInt( offset ) )
				{
					compiler_error( "Missing offset for vertex element" );
					return false;
				}
				if( !lexer.matchSymbol( TEXT("]") ) )
				{
					compiler_error( "Missing \"]\" in element declaration" );
					return false;
				}

				// Read elements comma
				lexer.matchSymbol( TEXT(",") );

				// add a new element to declaration
				rend::VertexElement element;
				element.format = format;
				element.usage = usage;
				element.usageIndex = usageIndex;
				element.offset = offset;

				vertexDecl.addElement( element );
			}
			else
			{
				compiler_error( "Missing vertex_decl element declaration" );
				return false;
			}
		}
		while( true );

		assert( vertexDecl.isValid() );
		context.vertexDecl = vertexDecl;

		return true;
	}
}
}