//-----------------------------------------------------------------------------
//	Streams.cpp: An UI batch streams implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "UI/UI.h"

namespace flu
{
namespace ui
{
namespace rendering
{
	FlatShadeStream::FlatShadeStream( rend::Device* device )
		:	StreamBase<FlatShadeStream, FlatShadeOp::Vertex>( device )
	{
		// read techniques and variables offset
	}

	FlatShadeStream::~FlatShadeStream()
	{
	}

	TextStream::TextStream( rend::Device* device )
		:	StreamBase<TextStream, TextOp::Vertex>( device )
	{
		// read techniques and variables offset
	}

	TextStream::~TextStream()
	{
	}
}
}
}