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
			output.errorMsg = TXT( "Unable to open font file" );
			return false;
		}

		JSon::Ptr rootNode = JSon::loadFromText( ffntText, &output.errorMsg );
		if( !rootNode )
		{
			return false;
		}

		String realFontName = rootNode->dotgetString( TXT( "Name" ), TXT( "" ) );
		String imageName = rootNode->dotgetString( TXT( "Image" ), TXT( "" ) );
		JSon::Ptr glyphsNode = rootNode->getField( TXT( "Glyphs" ), JSon::EMissingPolicy::USE_NULL );

		if( !realFontName )
		{
			output.errorMsg = TXT( "Missing font name" );
			return false;
		}
		if( !imageName )
		{
			output.errorMsg = TXT( "Missing font image name" );
			return false;
		}
		if( !glyphsNode )
		{
			output.errorMsg = TXT( "Missing glyphs definition" );
			return false;
		}

		// get image
		res::ResourceId imageResourceId( res::EResourceType::Image, imageName );
		output.references.put( imageResourceId, imageName );
		// todo: add missing image handling


		// make table of all glyphs
		Array<Glyph> glyphs;
		Array<UInt16> remap;

		for( Int32 i = 0; i < glyphsNode->arraySize(); ++i )
		{
			JSon::Ptr charNode = glyphsNode->getElement( i, JSon::EMissingPolicy::USE_NULL );
			assert( charNode.hasObject() );

			Int32 c = charNode->dotgetInt( TXT( "C" ), -1 );
			Float x = charNode->dotgetFloat( TXT( "X" ), -1.f );
			Float y = charNode->dotgetFloat( TXT( "Y" ), -1.f );
			Float w = charNode->dotgetFloat( TXT( "W" ), -1.f );
			Float h = charNode->dotgetFloat( TXT( "H" ), -1.f );

			if( c == -1 || x == -1.f || y == -1.f || w == -1.f || h == -1.f )
			{
				output.errorMsg = TXT( "Bad glyphs table" );
				return false;
			}

			if( remap.size() < c + 1 )
			{
				remap.setSize( c + 1 );
			}

			Glyph glyph;
			glyph.x = x;
			glyph.y = y;
			glyph.width = w;
			glyph.height = h;

			remap[c] = glyphs.push( glyph );
		}

		// serialize data
		UserBufferWriter writer( output.compiledResource.data );

		writer << imageResourceId;
		writer << glyphs;
		writer << remap;

		return true;
	}
}
}