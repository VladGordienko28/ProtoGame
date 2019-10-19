//-----------------------------------------------------------------------------
//	Converter.cpp: An images converter
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Image.h"

namespace flu
{
namespace img
{
	static const math::Color MASK_COLOR = { 0xff, 0x00, 0xff, 0xff };

	static Bool compileBmp( String relativePath, res::IDependencyProvider& dependencyProvider, 
		res::CompilationOutput& output )
	{
#pragma pack( push, 1 )
		struct BitmapFileHeader
		{
			UInt16	bfType;
			UInt32	bfSize;
			UInt16	bfReserved1;
			UInt16	bfReserved2;
			UInt32	bfOffBits;
		};

		struct BitmapInfoHeader
		{
			UInt32	biSize;
			Int32	biWidth;
			Int32	biHeight;
			UInt16	biPlanes;
			UInt16	biBitCount;
			UInt32	biCompression;
			UInt32	biSizeImage;
			Int32	biXPelsPerMeter;
			Int32	biYPelsPerMeter;
			UInt32	biClrUsed;
			UInt32	biClrImportant;
		};

		struct BitmapPaletteEntry
		{
			UInt8 rgbBlue;
			UInt8 rgbGreen;
			UInt8 rgbRed;
			UInt8 rgbReserved;
		};

#pragma pack( pop )

		auto loader = dependencyProvider.getBinaryFile( relativePath );
		assert( loader.hasObject() );

		BitmapFileHeader bmpHeader;
		BitmapInfoHeader bmpInfo;
		loader->readData( &bmpHeader, sizeof( BitmapFileHeader ) );
		loader->readData( &bmpInfo, sizeof( BitmapInfoHeader ) );

		if( bmpHeader.bfType != 0x4d42 )
		{
			output.errorMsg = String::format( TXT( "\"%s\" is not a bmp file" ), *relativePath );
			return false;
		}

		if( !isPowerOfTwo( bmpInfo.biWidth ) || !isPowerOfTwo( bmpInfo.biHeight ) )
		{
			output.errorMsg = String::format( TXT( "\"%s\" size is not power of two" ), *relativePath );
			return false;
		}

		// read palette is present
		BitmapPaletteEntry bmpPalette[256];
		if( bmpInfo.biClrUsed )
		{
			bmpInfo.biClrUsed = min<UInt32>( bmpInfo.biClrUsed, 256 );
			loader->readData( bmpPalette, sizeof( BitmapPaletteEntry ) * bmpInfo.biClrUsed );
		}

		Array<math::Color> tempData( bmpInfo.biWidth * bmpInfo.biHeight );
		UInt32 uMask = bmpInfo.biWidth - 1;
		UInt32 uBits = intLog2( bmpInfo.biWidth );
		UInt32 vMask = ( bmpInfo.biHeight - 1 ) << uBits;

		loader->seek( bmpHeader.bfOffBits );

		if( bmpInfo.biBitCount == 24 )
		{
			// 24 bits
			for( Int32 i = 0; i < bmpInfo.biWidth * bmpInfo.biHeight; i++ )
			{
				UInt8 pixData[3];
				loader->readData( pixData, 3 * sizeof( UInt8 ) );
				
				math::Color color( pixData[2], pixData[1], pixData[0], 0xff );
				if( color == MASK_COLOR )
				{
					color.a = 0x00;
				}

				tempData[( vMask & ~i ) | ( i & uMask )] = color;
			}
		}
		else if( bmpInfo.biBitCount == 32 )
		{
			// 32 bits
			for( Int32 i = 0; i < bmpInfo.biWidth * bmpInfo.biHeight; i++ )
			{
				UInt8 pixData[4];
				loader->readData( pixData, 4 * sizeof( UInt8 ) );

				math::Color color( pixData[2], pixData[1], pixData[0], 0xff );
				if( color == MASK_COLOR )
				{
					color.a = 0x00;
				}

				tempData[( vMask & ~i ) | ( i & uMask )] = color;
			}
		}
		else if( bmpInfo.biBitCount == 8 )
		{
			// 8 bits
			assert( bmpInfo.biClrUsed > 0 );

			for( Int32 i = 0; i < bmpInfo.biWidth * bmpInfo.biHeight; i++ )
			{
				UInt8 colorId;
				loader->readData( &colorId, sizeof( UInt8 ) );

				math::Color color( bmpPalette[colorId].rgbRed, bmpPalette[colorId].rgbGreen, bmpPalette[colorId].rgbBlue, 0xff );
				if( color == MASK_COLOR )
				{
					color.a = 0x00;
				}

				tempData[( vMask & ~i ) | ( i & uMask )] = color;
			}
		}
		else
		{
			output.errorMsg = String::format( TXT( "\"%s\" unsupported %d bits" ), *relativePath, bmpInfo.biBitCount );
			return false;
		}

		// make a palette image if possible
		Array<math::Color> palette;
		for( Int32 i = 0; i < bmpInfo.biWidth * bmpInfo.biHeight; ++i )
		{
			if( palette.addUnique( tempData[i] ) > 255 )
			{
				break;
			}
		}

		if( palette.size() <= 256 )
		{
			// save as palette
			palette.sort( []( const math::Color& a, const math::Color& b )->Bool
			{
				return a.d < b.d;
			} );

			Array<UInt8> palettedImage( tempData.size() );
			for( Int32 i = 0; i < tempData.size(); ++i )
			{
				palettedImage[i] = palette.find( tempData[i] );
			}

			LodePNGState state;
			lodepng_state_init( &state );

			state.info_png.color.colortype = LCT_PALETTE;
			state.info_png.color.bitdepth = 8;
			state.info_raw.colortype = LCT_PALETTE;
			state.info_raw.bitdepth = 8;
			state.encoder.auto_convert = false;

			palette.setSize( 256 );
			for( Int32 i = 0; i < 256; ++i )
			{
				UInt8 r = palette[i].r;
				UInt8 g = palette[i].g;
				UInt8 b = palette[i].b;
				UInt8 a = palette[i].a;

				lodepng_palette_add( &state.info_png.color, r, g, b, a );
				lodepng_palette_add( &state.info_raw, r, g, b, a );
			}

			UInt8* encodedData;
			SizeT encodedSize;
			auto pngError = lodepng_encode( &encodedData, &encodedSize, reinterpret_cast<unsigned char*>( &palettedImage[0] ), 
				bmpInfo.biWidth, bmpInfo.biHeight, &state );

			if( pngError )
			{
				output.errorMsg = String::format( TXT( "Unable to encode to png with error \"%s\"" ), 
					lodepng_error_text( pngError ) );

				lodepng_state_cleanup( &state );
				return false;
			}

			assert( encodedData && encodedSize > 0 );

			output.compiledResource.data.setSize( encodedSize );
			mem::copy( &output.compiledResource.data[0], encodedData, encodedSize );

			mem::free( encodedData );

			lodepng_state_cleanup( &state );

			return true;
		}
		else
		{
			// save as rgba
			LodePNGState state;
			lodepng_state_init( &state );

			state.info_png.color.colortype = LCT_RGBA;
			state.info_png.color.bitdepth = 8;
			state.info_raw.colortype = LCT_RGBA;
			state.info_raw.bitdepth = 8;
			state.encoder.auto_convert = false;

			UInt8* encodedData;
			SizeT encodedSize;
			auto pngError = lodepng_encode( &encodedData, &encodedSize, reinterpret_cast<unsigned char*>( &tempData[0] ), 
				bmpInfo.biWidth, bmpInfo.biHeight, &state );

			if( pngError )
			{
				output.errorMsg = String::format( TXT( "Unable to encode to png with error \"%s\"" ), 
					lodepng_error_text( pngError ) );

				lodepng_state_cleanup( &state );
				return false;
			}

			assert( encodedData && encodedSize > 0 );

			output.compiledResource.data.setSize( encodedSize );
			mem::copy( &output.compiledResource.data[0], encodedData, encodedSize );

			mem::free( encodedData );

			lodepng_state_cleanup( &state );

			return true;
		}
	}

	static Bool compileTga( String relativePath, res::IDependencyProvider& dependencyProvider, 
		res::CompilationOutput& output )
	{
#pragma pack(push,1)
		struct TGAHeader
		{
			UInt8	fileType;
			UInt8	colorMapType;
			UInt8	imageType;
			UInt8	colorMapSpec[5];
			UInt8	origX[2];
			UInt8	origY[2];
			UInt8	width[2];
			UInt8	height[2];
			UInt8	bpp;
			UInt8	imageInfo;
		};
#pragma pack(pop)

		auto loader = dependencyProvider.getBinaryFile( relativePath );
		assert( loader.hasObject() );
	
		TGAHeader tgaHeader;
		loader->readData( &tgaHeader, sizeof( TGAHeader ) );

		if( tgaHeader.imageType != 2 && tgaHeader.imageType != 10 )
		{
			output.errorMsg = String::format( TXT( "\"%s\" only 24 and 32 bits TGA supported" ), *relativePath );
			return false;
		}

		if( tgaHeader.colorMapType != 0 )
		{
			output.errorMsg = String::format( TXT( "\"%s\" color-mapped TGA is not supported" ), *relativePath );
			return false;
		}

		const UInt32 width = tgaHeader.width[0] + tgaHeader.width[1] * 256;
		const UInt32 height = tgaHeader.height[0] + tgaHeader.height[1] * 256;
		const UInt32 colorDepth = tgaHeader.bpp;

		if( !isPowerOfTwo( width ) || !isPowerOfTwo( height ) )
		{
			output.errorMsg = String::format( TXT( "\"%s\" size is not power of two" ), *relativePath );
			return false;
		}

		if( colorDepth < 24 )
		{
			output.errorMsg = String::format( TXT( "\"%s\" only 24 and 32 bits TGA supported" ), *relativePath );
			return false;
		}

		const UInt32 uMask = width - 1;
		const UInt32 uBits = intLog2( width );
		const UInt32 vMask = ( height - 1 ) << uBits;

		Array<math::Color> image( width * height );

		if( tgaHeader.imageType == 2 )
		{
			// not compressed tga
			SizeT imageSize = width * height * ( colorDepth / 8 );
			Array<UInt8> tempImage( imageSize );

			loader->readData( &tempImage[0], imageSize );

			if( colorDepth == 24 )
			{
				// 24 bits, not compressed
				for( UInt32 i = 0; i < width * height; ++i )
				{
					math::Color pixel = { tempImage[i * 3 + 2], tempImage[i * 3 + 1], tempImage[i * 3 + 0], 0xff };
					
					if( pixel == MASK_COLOR )
					{
						pixel.a = 0x00;
					}

					image[( vMask & ~i ) | ( i & uMask )] = pixel;
				}
			}
			else
			{
				// 32 bits, not compressed
				for( UInt32 i = 0; i < width * height; ++i )
				{
					math::Color pixel = { tempImage[i * 4 + 2], tempImage[i * 4 + 1], tempImage[i * 4 + 0], tempImage[i * 4 + 3] };
					image[( vMask & ~i ) | ( i & uMask )] = pixel;
				}
			}		
		}
		else if( tgaHeader.imageType == 10 )
		{
			// 24 or 32 bits, rle compressed
			const UInt32 stride = colorDepth / 8;

			UInt32 imageWalker = 0;
			UInt32 bufferWalker = 0;
			UInt32 pixelsPassed = 0;

			const SizeT bytesRemain = loader->totalSize() - sizeof( TGAHeader );
			Array<UInt8> buffer( bytesRemain + 2 );

			const SizeT bytesRead = loader->readData( &buffer[0], bytesRemain );
			assert( bytesRead == bytesRemain );

			while( pixelsPassed < width * height )
			{
				UInt8 front = buffer[bufferWalker++];

				if( front < 128 )
				{
					for( Int32 i = 0; i <= front; ++i )
					{
						image[( vMask & ~imageWalker ) | ( imageWalker & uMask )].r = buffer[ bufferWalker + i * stride + 2 ];
						image[( vMask & ~imageWalker ) | ( imageWalker & uMask )].g = buffer[ bufferWalker + i * stride + 1 ];
						image[( vMask & ~imageWalker ) | ( imageWalker & uMask )].b = buffer[ bufferWalker + i * stride + 0 ];
						image[( vMask & ~imageWalker ) | ( imageWalker & uMask )].a = buffer[ bufferWalker + i * stride + 3 ];

						++imageWalker;
						++pixelsPassed;
					}

					bufferWalker += stride * ( static_cast<UInt32>( front ) + 1 );
				}
				else
				{
					for( Int32 i = 0; i <= front - 128; ++i )
					{
						image[( vMask & ~imageWalker ) | ( imageWalker & uMask )].r = buffer[ bufferWalker + 2 ];
						image[( vMask & ~imageWalker ) | ( imageWalker & uMask )].g = buffer[ bufferWalker + 1 ];
						image[( vMask & ~imageWalker ) | ( imageWalker & uMask )].b = buffer[ bufferWalker + 0 ];
						image[( vMask & ~imageWalker ) | ( imageWalker & uMask )].a = buffer[ bufferWalker + 3 ];

						++imageWalker;
						++pixelsPassed;
					}

					bufferWalker += stride;
				}
			}

			// fix alpha channel for 24 bits
			if( colorDepth == 24 )
			{
				for( auto& it : image )
				{
					it.a = 0xff;

					if( it == MASK_COLOR )
					{
						it.a = 0x00;
					}
				}
			}
		}
		else
		{
			output.errorMsg = String::format( TXT( "\"%s\" unknown TGA image type %d" ), *relativePath, tgaHeader.imageType );
			return false;
		}

		// save as rgba
		LodePNGState state;
		lodepng_state_init( &state );

		state.info_png.color.colortype = LCT_RGBA;
		state.info_png.color.bitdepth = 8;
		state.info_raw.colortype = LCT_RGBA;
		state.info_raw.bitdepth = 8;
		state.encoder.auto_convert = false;

		UInt8* encodedData;
		SizeT encodedSize;
		auto pngError = lodepng_encode( &encodedData, &encodedSize, reinterpret_cast<unsigned char*>( &image[0] ), 
			width, height, &state );

		if( pngError )
		{
			output.errorMsg = String::format( TXT("Unable to encode to png with error: \"%hs\""), 
				lodepng_error_text( pngError ) );

			lodepng_state_cleanup( &state );
			return false;
		}

		assert( encodedData && encodedSize > 0 );

		output.compiledResource.data.setSize( encodedSize );
		mem::copy( &output.compiledResource.data[0], encodedData, encodedSize );

		mem::free( encodedData );

		lodepng_state_cleanup( &state );

		return true;
	}

	static Bool compilePng( String relativePath, res::IDependencyProvider& dependencyProvider, 
		res::CompilationOutput& output )
	{
		auto loader = dependencyProvider.getBinaryFile( relativePath );
		assert( loader.hasObject() );

		// no conversation needs
		Array<UInt8>& data = output.compiledResource.data;
		data.setSize( loader->totalSize() );
		loader->readData( &data[0], data.size() );

		return true;
	}

	Bool Converter::construct( String name, res::IConstructionEnvironment& environment, String& errorMsg,
		EImageType type, UInt32 width, UInt32 height, const void* data ) const
	{
		assert( type == EImageType::RGBA ); // todo: add support of other types
		assert( width > 0 && height > 0 );
		assert( data != nullptr );

		const String fileName = name + TXT(".png");
		IOutputStream::Ptr writer = environment.writeBinaryFile( fileName );

		if( !writer )
		{
			errorMsg = String::format( TXT("Unable to open file \"%s\""), *fileName );
			return false;
		}

		UInt8* pngData = nullptr;
		SizeT pngSize = 0;

		auto pngError = lodepng_encode_memory( &pngData, &pngSize, reinterpret_cast<const UInt8*>( data ), 
			width, height, LCT_RGBA, 8 );

		if( pngError )
		{
			errorMsg = String::format( TXT("Unable to encode to png with error: \"%hs\""), 
				lodepng_error_text( pngError ) );

			return false;
		}

		writer->writeData( pngData, pngSize );
		mem::free( pngData );

		return true;
	}

	Bool Converter::compile( String relativePath, res::IDependencyProvider& dependencyProvider, 
		res::CompilationOutput& output ) const
	{
		const String ext = fm::getFileExt( *relativePath );

		if( ext == TXT( "bmp" ) )
		{
			return compileBmp( relativePath, dependencyProvider, output );
		}
		else if( ext == TXT( "tga" ) )
		{
			return compileTga( relativePath, dependencyProvider, output );
		}
		else if( ext == TXT( "png" ) )
		{
			return compilePng( relativePath, dependencyProvider, output );
		}
		else
		{
			fatal( TXT( "Unknown image format in \"%s\"" ), *relativePath );
			return false;
		}
	}

	Converter::Converter()
	{
	}

	Converter::~Converter()
	{
	}
}
}