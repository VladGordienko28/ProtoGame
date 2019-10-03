//-----------------------------------------------------------------------------
//	Compiler.h: A font files compiler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Font.h"

namespace flu
{
namespace fnt
{
	Bool Compiler::compile( String relativePath, res::IDependencyProvider& dependencyProvider, 
		res::CompilationOutput& output ) const
	{
		Text::Ptr ffntText = dependencyProvider.getTextFile( relativePath );
		if( !ffntText )
		{
			output.errorMsg = TEXT( "Unable to open font file" );
			return false;
		}

		JSon::Ptr rootNode = JSon::loadFromText( ffntText, &output.errorMsg );
		if( !rootNode )
		{
			return false;
		}

		String realFontName = rootNode->dotgetString( TEXT( "Name" ), TEXT( "" ) );
		String imageName = rootNode->dotgetString( TEXT( "Image" ), TEXT( "" ) );
		JSon::Ptr glyphsNode = rootNode->getField( TEXT( "Glyphs" ), JSon::EMissingPolicy::USE_NULL );

		if( !realFontName )
		{
			output.errorMsg = TEXT( "Missing font name" );
			return false;
		}
		if( !imageName )
		{
			output.errorMsg = TEXT( "Missing font image name" );
			return false;
		}
		if( !glyphsNode )
		{
			output.errorMsg = TEXT( "Missing glyphs definition" );
			return false;
		}

		// get image
		res::ResourceId imageResourceId( res::EResourceType::Image, imageName );
		auto image = res::ResourceManager::get<img::Image>( imageName );
		if( !image )
		{
			output.errorMsg = String::format( TEXT( "Image \"%s\" is not found" ), *imageName );
			return false;
		}

		output.references.put( imageResourceId, imageName );

		// make table of all glyphs
		Array<Glyph> glyphs;
		Array<UInt16> remap;

		for( Int32 i = 0; i < glyphsNode->arraySize(); ++i )
		{
			JSon::Ptr charNode = glyphsNode->getElement( i, JSon::EMissingPolicy::USE_NULL );
			assert( charNode.hasObject() );

			Int32 c = charNode->dotgetInt( TEXT( "C" ), -1 );
			Float x = charNode->dotgetInt( TEXT( "X" ), -1 );
			Float y = charNode->dotgetInt( TEXT( "Y" ), -1 );
			Float w = charNode->dotgetInt( TEXT( "W" ), -1 );
			Float h = charNode->dotgetInt( TEXT( "H" ), -1 );

			if( c == -1 || x == -1.f || y == -1.f || w == -1.f || h == -1.f )
			{
				output.errorMsg = TEXT( "Bad glyphs table" );
				return false;
			}

			if( remap.size() < c + 1 )
			{
				remap.setSize( c + 1 );
			}

			Glyph glyph;
			glyph.x = x / image->getUSize();
			glyph.y = y / image->getVSize();
			glyph.width = w / image->getUSize();
			glyph.height = h / image->getVSize();

			remap[c] = glyphs.push( glyph );
		}

		// serialize data
		BufferWriter writer( &output.compiledResource.data );

		writer << imageResourceId;
		writer << glyphs;
		writer << remap;

		return true;
	}
}
}