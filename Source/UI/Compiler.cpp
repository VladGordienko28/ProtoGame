//-----------------------------------------------------------------------------
//	Compiler.cpp: An UI layout compiler implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "UI.h"

namespace flu
{
namespace ui
{
	Compiler::Compiler()
	{
	}

	Compiler::~Compiler()
	{
	}

	Bool Compiler::compile( String relativePath, res::IDependencyProvider& dependencyProvider, 
		res::CompilationOutput& output ) const
	{
		auto text = dependencyProvider.getTextFile( relativePath );
		assert( text.hasObject() );

		JSon::Ptr json = JSon::loadFromText( text, &output.errorMsg );

		// todo: add some simple checks here

		if( json.hasObject() )
		{
			UserBufferWriter writer( output.compiledResource.data );

			return JSon::saveToStream( writer, json, &output.errorMsg );
		}
		else
		{
			return false;
		}
	}
}
}