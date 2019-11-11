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

	ImageStream::ImageStream( rend::Device* device )
		:	StreamBase<ImageStream, ImageOp::Vertex>( device )
	{
		// read techniques and variables offset
	}

	ImageStream::~ImageStream()
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