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
		{	L"Unknown",		0,			0,			0,			0	},

		{	L"R8_I",		1,			1,			1,			1	},
		{	L"R8_U",		1,			1,			1,			1	},

		{	L"R16_F",		2,			1,			1,			1	},
		{	L"R16_I",		2,			1,			1,			1	},
		{	L"R16_U",		2,			1,			1,			1	},

		{	L"R32_F",		4,			1,			1,			1	},
		{	L"R32_I",		4,			1,			1,			1	},
		{	L"R32_U",		4,			1,			1,			1	},

		{	L"RG8_I",		2,			1,			1,			2	},
		{	L"RG8_U",		2,			1,			1,			2	},
	
		{	L"RG16_F",		4,			1,			1,			2	},
		{	L"RG16_I",		4,			1,			1,			2	},
		{	L"RG16_U",		4,			1,			1,			2	},

		{	L"RG32_F",		8,			1,			1,			2	},
		{	L"RG32_I",		8,			1,			1,			2	},
		{	L"RG32_U",		8,			1,			1,			2	},

		{	L"RGB32_F",		12,			1,			1,			3	},
		{	L"RGB32_I",		12,			1,			1,			3	},
		{	L"RGB32_U",		12,			1,			1,			3	},

		{	L"RGBA8_I",		4,			1,			1,			4	},
		{	L"RGBA8_U",		4,			1,			1,			4	},
		{	L"RGBA8_UNORM",	4,			1,			1,			4	},

		{	L"RGBA32_F",	16,			1,			1,			4	},
		{	L"RGBA32_I",	16,			1,			1,			4	},
		{	L"RGBA32_U",	16,			1,			1,			4	},

		{	L"D24S8",		4,			1,			1,			2	},
	};

	const FormatInfo& getFormatInfo( EFormat format )
	{
		assert( format > EFormat::Unknown && format < EFormat::MAX );
		return g_formatsInfo[ static_cast<SizeT>( format ) ];
	}

	EFormat getFormatByName( String formatName )
	{
		for( SizeT i = 0; i < arraySize( g_formatsInfo ); ++i )
		{
			if( formatName == g_formatsInfo[i].name )
			{
				return static_cast<EFormat>( i );
			}
		}

		return EFormat::Unknown;
	}
}
}