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

		// compile all shaders in technique
		if( context.techniques.size() == 0 )
		{
			output.errorMsg = TEXT( "Effect should declare at least one technique" );
			return false;
		}
		if( context.techniques.size() > Technique::MAX_TECHNIQUES )
		{
			output.errorMsg = TEXT( "Too many techniques" );
			return false;
		}

		Array<rend::CompiledShader> shaderList;
		Map<String, ShaderId> shaderMap;
		Array<Technique> techs;

		for( const auto& it : context.techniques )
		{
			Technique tech;
			tech.name = it.name;

			UInt32 i = 0;
			for( const String& entryPointName : it.entries )
			{
				rend::EShaderType shaderType = static_cast<rend::EShaderType>( i );

				if( entryPointName )
				{
					if( ShaderId* id = shaderMap.get( entryPointName ) )
					{
						// reuse the same shader
						tech.shaderIds[i] = *id;
					}
					else
					{
						// compile new shader with new entry point
						auto compiledShader = apiCompiler->compile( shaderType, 
							context.writer.getText(), *entryPointName, nullptr, &output.errorMsg );

						if( !compiledShader.isValid() )
						{
							return false;
						}

						ShaderId newId = shaderList.push( compiledShader );
						shaderMap.put( entryPointName, newId );
						tech.shaderIds[i] = newId;
					}
				}
				else
				{
					// we not allow missing shaders yet
					if( shaderType != rend::EShaderType::ST_Compute )
					{
						output.errorMsg = String::format( TEXT( "Missing some shader in technique \"%s\"" ), *tech.name );
						return false;					
					}
					else
					{
						tech.shaderIds[i] = INVALID_SHADER;
					}
				}

				++i;
			}

			techs.push( tech );
		}

		// emit all techniques and shaders
		emitter << shaderList;
		emitter << techs;

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
				else if( token.getText() == KW_technique )
				{
					// compile technique
					if( !compileTechnique( lexer, context ) )
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

	Bool Compiler::compileTechnique( lexer::Lexer& lexer, Context& context ) const
	{
		TechniqueInfo info;

		// parse name
		info.name = lexer.getIdentifier();
		if( !info.name )
		{
			compiler_error( "Missing technique name" );
			return false;
		}
		if( !lexer.matchSymbol( TEXT("{") ) )
		{
			compiler_error( "Bad technique name" );
			return false;
		}

		// parse list of shader types
		while( true )
		{
			if( lexer.matchSymbol( TEXT("}") ) )
			{
				break;
			}

			// shader type
			String shaderType = lexer.getIdentifier();
			if( !shaderType )
			{
				compiler_error( "Missing shader type" );
				return false;
			}
			if( !lexer.matchSymbol( TEXT("=") ) )
			{
				compiler_error( "Bad shader type" );
				return false;
			}

			// entry point name
			String entryName = lexer.getIdentifier();
			if( !entryName )
			{
				compiler_error( "Missing shader entry point name" );
				return false;
			}
			if( !lexer.matchSymbol( TEXT(";") ) )
			{
				compiler_error( "Bad shader entry point name" );
				return false;
			}

			// save to info
			if( shaderType == KW_vertex_shader )
			{
				if( info.entries[rend::EShaderType::ST_Vertex] )
				{
					compiler_error( "Vertex Shader entry point is already defined" );
					return false;
				}

				info.entries[rend::EShaderType::ST_Vertex] = entryName;
			}
			else if( shaderType == KW_pixel_shader )
			{
				if( info.entries[rend::EShaderType::ST_Pixel] )
				{
					compiler_error( "Pixel Shader entry point is already defined" );
					return false;
				}

				info.entries[rend::EShaderType::ST_Pixel] = entryName;
			}
			else
			{
				compiler_error( "Unknown shader type \"%s\"", *shaderType );
				return false;
			}
		}

		// check for duplicates
		for( const auto& it : context.techniques )
		{
			if( it.name == info.name )
			{
				compiler_error( "Technique \"%s\" already defined", *info.name );
				return false;
			}
		}

		context.techniques.push( info );
		return true;
	}
}
}