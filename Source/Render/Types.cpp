//-----------------------------------------------------------------------------
//	Types.cpp: Basic render types
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Render.h"

namespace flu
{
namespace rend
{
	FormatInfo g_formatsInfo[ static_cast<SizeT>( EFormat::MAX ) ] = 
	{
		//	name,			blockBytes,	blockSizeX,	blockSizeY,	componentsCount;
		{	"Unknown",		0,			0,			0,			0	},

		{	"R8_I",			1,			1,			1,			1	},
		{	"R8_U",			1,			1,			1,			1	},

		{	"R16_F",		2,			1,			1,			1	},
		{	"R16_I",		2,			1,			1,			1	},
		{	"R16_U",		2,			1,			1,			1	},

		{	"R32_F",		4,			1,			1,			1	},
		{	"R32_I",		4,			1,			1,			1	},
		{	"R32_U",		4,			1,			1,			1	},

		{	"RG8_I",		2,			1,			1,			2	},
		{	"RG8_U",		2,			1,			1,			2	},
	
		{	"RG16_F",		4,			1,			1,			2	},
		{	"RG16_I",		4,			1,			1,			2	},
		{	"RG16_U",		4,			1,			1,			2	},

		{	"RG32_F",		8,			1,			1,			2	},
		{	"RG32_I",		8,			1,			1,			2	},
		{	"RG32_U",		8,			1,			1,			2	},

		{	"RGBA8_I",		4,			1,			1,			4	},
		{	"RGBA8_U",		4,			1,			1,			4	},
		{	"RGBA8_UNORM",	4,			1,			1,			4	},

		{	"D24S8",		4,			1,			1,			2	},
	};

	const FormatInfo& getFormatInfo( EFormat format )
	{
		assert( format > EFormat::Unknown && format < EFormat::MAX );
		return g_formatsInfo[ static_cast<SizeT>( format ) ];
	}
}
}