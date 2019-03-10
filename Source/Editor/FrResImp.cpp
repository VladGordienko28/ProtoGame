/*=============================================================================
    FrResImp.cpp: Resource importing.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    Image importers.
-----------------------------------------------------------------------------*/

//
// Import the bmp file.
//
FBitmap* ImportBMP( String Filename, String ResName )
{
	fm::IBinaryFileReader::Ptr loader = fm::readBinaryFile( *Filename );

	BITMAPFILEHEADER BmpHeader;
	BITMAPINFOHEADER BmpInfo;
	loader->readData( &BmpHeader, sizeof( BITMAPFILEHEADER ) );
	loader->readData( &BmpInfo, sizeof( BITMAPINFOHEADER ) );

	if( BmpHeader.bfType != 0x4d42 )
	{
		MessageBox( 0, *String::format( L"\"%s\" is not BMP file.", *Filename ), 
			L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );

		return nullptr;
	}

	if( !(((BmpInfo.biWidth)&(BmpInfo.biWidth-1)) == 0 && ((BmpInfo.biHeight)&(BmpInfo.biHeight-1)) == 0) )
	{
		MessageBox( 0, *String::format( L"Bitmap size should be power of two" ), 
			L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );

		return nullptr;
	}

	// Raw bmp palette.
	RGBQUAD BmpPalette[256];
	if( BmpInfo.biClrUsed )
	{
		BmpInfo.biClrUsed = min( BmpInfo.biClrUsed, (DWORD)256 );
		loader->readData( BmpPalette, sizeof( RGBQUAD ) * BmpInfo.biClrUsed );
	}

	math::Color* TempData = new math::Color[BmpInfo.biWidth * BmpInfo.biHeight];
	Array<math::Color> Palette;
	Int32 UMask = BmpInfo.biWidth-1;
	Int32 UBits = IntLog2( BmpInfo.biWidth );
	Int32 VMask = (BmpInfo.biHeight-1) << UBits;

	// Load entire bmp data.
	loader->seek( BmpHeader.bfOffBits );
	if( BmpInfo.biBitCount == 24 )
	{
		// 24 - bit.
		for( Int32 i=0; i<BmpInfo.biWidth*BmpInfo.biHeight; i++ )
		{
			UInt8 RawPix[4];
			loader->readData( RawPix, 3 * sizeof(UInt8) );
			math::Color Source( RawPix[2], RawPix[1], RawPix[0], 0xff );
			if( Source == MASK_COLOR )	Source.a = 0x00;
			TempData[(VMask&~i)|(i&UMask)] = Source;		// Flip image here.
		}
	}
	else if( BmpInfo.biBitCount == 32 )
	{
		// 32 - bit.
		for( Int32 i=0; i<BmpInfo.biWidth*BmpInfo.biHeight; i++ )
		{
			UInt8 RawPix[4];
			loader->readData( RawPix, 4 * sizeof( UInt8 ) );
			math::Color Source( RawPix[2], RawPix[1], RawPix[0], 0xff );
			if( Source == MASK_COLOR )	Source.a = 0x00;
			TempData[(VMask&~i)|(i&UMask)] = Source;		// Flip image here.
		}
	}
	else if( BmpInfo.biBitCount == 8 )
	{
		// 8 - bit.
		for( Int32 i=0; i<BmpInfo.biWidth*BmpInfo.biHeight; i++ )
		{
			UInt8 iColor;
			loader->readData( &iColor, sizeof( UInt8 ) );
			math::Color Source( BmpPalette[iColor].rgbRed, BmpPalette[iColor].rgbGreen, BmpPalette[iColor].rgbBlue, 0xff );
			if( Source == MASK_COLOR )	Source.a = 0x00;
			TempData[(VMask&~i)|(i&UMask)] = Source;		// Flip image here.
		}
	}
	else
	{
		// Bad bitmap format.
		delete[] TempData;

		MessageBox( 0, *String::format( L"Bad bit count %d", BmpInfo.biBitCount ), 
			L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );

		return nullptr;
	}

#if 0
	// Reduce bitmap color depth.
	enum{ BMP_REDUCE_DEPTH	= 1 };
	for( Integer i=0; i<BmpInfo.biWidth*BmpInfo.biHeight; i++ )
	{
		TempData[i].R	&= ~((Byte)BMP_REDUCE_DEPTH);
		TempData[i].G	&= ~((Byte)BMP_REDUCE_DEPTH);
		TempData[i].B	&= ~((Byte)BMP_REDUCE_DEPTH);
	}
#endif

	// Figure out it's a palette or not.
	for( Int32 i=0; i<BmpInfo.biWidth*BmpInfo.biHeight; i++ )
		if( Palette.addUnique(TempData[i]) > 255 )
			break;

	// Allocate bitmap and initialize it.
	FBitmap*	Bitmap	= NewObject<FBitmap>( ResName, nullptr );
	Bitmap->Init( BmpInfo.biWidth, BmpInfo.biHeight );
	Bitmap->FileName	= fm::getFileName( *Filename ) + L".bmp";

	// Load data into bitmap.
	if( Palette.size() <= 256 )
	{
		// Palette.
		Palette.sort( []( const math::Color& A, const math::Color& B )->Bool
		{
			return A.d < B.d;
		} );

		Bitmap->Format	= BF_Palette8;
		Bitmap->Palette.Allocate( Palette.size() );
		for( Int32 i=0; i<Palette.size(); i++ )
			Bitmap->Palette.Colors[i] = Palette[i];

		Bitmap->AllocateBlock( sizeof(UInt8)*Bitmap->USize*Bitmap->VSize );
		UInt8* BitDat = (UInt8*)Bitmap->GetData();
		for( Int32 i=0; i < Bitmap->USize*Bitmap->VSize; i++ )
			BitDat[i]	= Palette.find( TempData[i] );
	}
	else
	{
		// RGBA.
		Bitmap->Format	= BF_RGBA;
		Bitmap->AllocateBlock( sizeof(math::Color)*Bitmap->USize*Bitmap->VSize );
		mem::copy( Bitmap->GetData(), TempData, sizeof(math::Color)*Bitmap->USize*Bitmap->VSize );
	}

	delete[] TempData;
	return Bitmap;
}


//
// Import the tga file.
//
FBitmap* ImportTGA( String Filename, String ResName )
{
#pragma pack(push,1)
	struct TGAHeader
	{
		UInt8		FileType;
		UInt8		ColorMapType;
		UInt8		ImageType;
		UInt8		ColorMapSpec[5];
		UInt8		OrigX[2];
		UInt8		OrigY[2];
		UInt8		Width[2];
		UInt8		Height[2];
		UInt8		BPP;
		UInt8		ImageInfo;
	};
#pragma pack(pop)

	fm::IBinaryFileReader::Ptr loader = fm::readBinaryFile( *Filename );
	TGAHeader	TgaHeader;
	loader->readData( &TgaHeader, sizeof( TGAHeader ) );

	// Only 24 and 32 bits supported.
	if( TgaHeader.ImageType != 2 && TgaHeader.ImageType != 10 )
	{
		MessageBox( 0, *String::format( L"Only 24 and 32 bit TGA supported" ), 
			L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );

		return nullptr;
	}

	// Doesn't allow color map.
	if( TgaHeader.ColorMapType != 0 )
	{
		MessageBox( 0, *String::format( L"Color-mapped TGA doesn't supported" ), 
			L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );

		return nullptr;
	}

	// Figure out general properties.
	Int32	Width		= TgaHeader.Width[0] + TgaHeader.Width[1]*256;
	Int32	Height		= TgaHeader.Height[0] + TgaHeader.Height[1]*256;
	Int32	ColorDepth	= TgaHeader.BPP;
	Int32	ImageSize	= Width*Height*(ColorDepth/8);

	if( !(((Width)&(Width-1)) == 0 && ((Height)&(Height-1)) == 0) )
	{
		MessageBox( 0, *String::format( L"Image size should be power of two" ), 
			L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );

		return nullptr;
	}
	if( ColorDepth < 24 )
	{
		MessageBox( 0, *String::format( L"Only 24 and 32 bit TGA supported" ), 
			L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );

		return nullptr;
	}

	FBitmap*	Bitmap	= nullptr;

	if( TgaHeader.ImageType == 2 )
	{
		// Standard uncompressed tga.
		UInt8* TmpImage	= new UInt8[ImageSize];
		loader->readData( TmpImage, ImageSize );

		// Allocate bitmap and initialize it.
		Bitmap				= NewObject<FBitmap>( ResName, nullptr );
		Bitmap->Init( Width, Height );
		Bitmap->FileName	= fm::getFileName( *Filename ) + L".tga";
		Bitmap->Format		= BF_RGBA;
		Bitmap->AllocateBlock( sizeof(math::Color)*Bitmap->USize*Bitmap->VSize );
		math::Color* Dest		= (math::Color*)Bitmap->GetData();

		Int32 UMask = Width-1;
		Int32 UBits = IntLog2( Width );
		Int32 VMask = (Height-1) << UBits;

		if( TgaHeader.BPP == 24 )
		{
			// 24-bit uncompressed.
			for( Int32 i=0; i<Width*Height; i++ )
			{
				math::Color Source( TmpImage[i*3+2], TmpImage[i*3+1], TmpImage[i*3+0], 0xff );
				if( Source == MASK_COLOR )	Source.a = 0x00;
				Dest[(VMask&~i)|(i&UMask)]	= Source;
			}
		}
		else
		{
			// 32-bit uncompressed.
			for( Int32 i=0; i<Width*Height; i++ )
			{
				math::Color Source( TmpImage[i*4+2], TmpImage[i*4+1], TmpImage[i*4+0], TmpImage[i*4+3] );
				Dest[(VMask&~i)|(i&UMask)]	= Source;
			}
		}

		delete[] TmpImage;
	}
	else if( TgaHeader.ImageType == 10 )
	{
		// 24 or 32 compressed.
		Int32	Stride	= ColorDepth / 8;
		Int32	WalkColor	= 0,
				WalkPixel	= 0,
				WalkBuffer	= 0;

		// Load remain of file to buffer.
		UInt8*	Compressed	= new UInt8[ loader->totalSize() - sizeof( TGAHeader ) ];
		loader->readData( Compressed, loader->totalSize() - sizeof( TGAHeader ) );

		// Allocate bitmap and initialize it.
		Bitmap				= NewObject<FBitmap>( ResName, nullptr );
		Bitmap->Init( Width, Height );
		Bitmap->FileName	= fm::getFileName( *Filename ) + L".tga";
		Bitmap->Format		= BF_RGBA;
		Bitmap->AllocateBlock( sizeof(math::Color)*Bitmap->USize*Bitmap->VSize );
		math::Color* Image		= (math::Color*)Bitmap->GetData();

		Int32 UMask = Width-1;
		Int32 UBits = IntLog2( Width );
		Int32 VMask = (Height-1) << UBits;

		// Extract pixel by pixel from compressed data.
		do 
		{
			UInt8 Front	= Compressed[WalkBuffer++];
			if( Front < 128 )
			{
				for( Int32 i=0; i<=Front; i++ )
				{
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].r	= Compressed[WalkBuffer+i*Stride+2];
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].g	= Compressed[WalkBuffer+i*Stride+1];
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].b	= Compressed[WalkBuffer+i*Stride+0];
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].a	= Compressed[WalkBuffer+i*Stride+3];

					WalkColor++;
					WalkPixel++;
				}

				WalkBuffer	+= (Front+1)*Stride;
			}
			else
			{
				for( Int32 i=0; i<=Front-128; i++ )
				{
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].r	= Compressed[WalkBuffer+2];
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].g	= Compressed[WalkBuffer+1];
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].b	= Compressed[WalkBuffer+0];
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].a	= Compressed[WalkBuffer+3];

					WalkColor++;
					WalkPixel++;
				}

				WalkBuffer	+= Stride;
			}
		} while( WalkPixel < Width*Height );

		assert(WalkPixel==Width*Height);

		// Fix alpha-channel in 24-bits mode.
		if( TgaHeader.BPP == 24 )
			for( Int32 i=0; i<Width*Height; i++ )
			{
				Image[i].a	= 0xff;
				if( Image[i] == MASK_COLOR )
					Image[i].a	= 0x00;
			}

		delete[] Compressed;
	}

	return Bitmap;
}


//
// Import the png file.
//
FBitmap* ImportPNG( String Filename, String ResName )
{
	unsigned		PngError, PngWidth, PngHeight;
	unsigned char*	PngImage = nullptr;

	// Load from file.
	char file_name[1024];
	wcstombs( file_name, *Filename, arraySize(file_name) );	// It's not good.
	PngError = lodepng_decode32_file( &PngImage, &PngWidth, &PngHeight, file_name );
	if( PngError )
	{
		MessageBox( 0, *String::format( L"Png: Failed error png. %u: %hs", PngError, lodepng_error_text(PngError) ), 
			L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );

		free(PngImage);
		return nullptr;
	}
	if( !(isPowerOfTwo(PngWidth) && isPowerOfTwo(PngHeight)) )
	{
		MessageBox( 0, *String::format( L"Png size should be power of two" ), 
			L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );

		free(PngImage);
		return nullptr;
	}

	Int32 UMask = PngWidth-1;
	Int32 UBits = IntLog2(PngWidth);
	Int32 VMask = (PngHeight-1) << UBits;
	math::Color*	SourceData = (math::Color*)PngImage;

	// Allocate bitmap and initialize it.
	FBitmap* Bitmap = NewObject<FBitmap>( ResName, nullptr );
	Bitmap->Init( PngWidth, PngHeight );
	Bitmap->FileName = fm::getFileName( *Filename ) + L".png";

	// Figure out it's a palette or not.
	Array<math::Color> Palette;
	for( Int32 i=0; i<PngWidth*PngHeight; i++ )
		if( Palette.addUnique(SourceData[i]) > 255 )
			break;

	// Load data into bitmap.
	if( Palette.size() <= 256 )
	{
		// Palette.
		Palette.sort( []( const math::Color& A, const math::Color& B )->Bool
		{
			return A.d < B.d;
		} );
		Bitmap->Format	= BF_Palette8;
		Bitmap->Palette.Allocate( Palette.size() );
		for( Int32 i=0; i<Palette.size(); i++ )
			Bitmap->Palette.Colors[i] = Palette[i];

		Bitmap->AllocateBlock( sizeof(UInt8)*Bitmap->USize*Bitmap->VSize );
		UInt8* Dest = (UInt8*)Bitmap->GetData();
		for( Int32 i=0; i < Bitmap->USize*Bitmap->VSize; i++ )
			Dest[i]	= Palette.find( SourceData[i] );
	}
	else
	{
		// RGBA.
		Bitmap->Format	= BF_RGBA;
		Bitmap->AllocateBlock( sizeof(math::Color)*Bitmap->USize*Bitmap->VSize );
		mem::copy( Bitmap->GetData(), SourceData, sizeof(math::Color)*Bitmap->USize*Bitmap->VSize );
	}

	// Turn on alpha or not?
	Bool bMaskOnly = true;
	Bitmap->BlendMode = BLEND_Regular;
	for( Int32 i=0; i<PngWidth*PngHeight; i++ )
		if( SourceData[i].a != 255 )
			if( SourceData[i].a != 0 )
			{
				Bitmap->BlendMode = BLEND_Alpha;
				break;
			}
			else
				bMaskOnly = false;

	if( Bitmap->BlendMode != BLEND_Alpha )
		Bitmap->BlendMode = bMaskOnly ? BLEND_Regular : BLEND_Masked;

	free(PngImage);
	return Bitmap;
}


/*-----------------------------------------------------------------------------
    Sound importers.
-----------------------------------------------------------------------------*/

//
// Import the wav file.
//
FSound* ImportWAV( String Filename, String ResName )
{
	fm::IBinaryFileReader::Ptr loader = fm::readBinaryFile( *Filename );

	FSound*	Sound = NewObject<FSound>( ResName, nullptr );
	SizeT FileSize = loader->totalSize();

	Sound->AllocateBlock( FileSize );
	Sound->FileName = fm::getFileName( *Filename ) + L".wav";
	loader->readData( Sound->GetData(), FileSize );

	return Sound;
}


/*-----------------------------------------------------------------------------
    Music importers.
-----------------------------------------------------------------------------*/

//
// Import the ogg file.
//
FMusic* ImportOGG( String Filename, String ResName )
{
	FMusic*	Music	= NewObject<FMusic>( ResName, nullptr );

	// Figure out Music directory.
	if( String::pos( String::upperCase(fm::getCurrentDirectory()), String::upperCase(Filename) ) != -1 )
	{
		// In directory of exe.
		Music->FileName	=	String::copy
							( 
								Filename, 
								fm::getCurrentDirectory().len()+1, 
								Filename.len()-fm::getCurrentDirectory().len()-1 
							);
	}
	else
	{
		// External directory.
		Music->FileName	=	fm::getFileName( *Filename ) + L".ogg";
	}

	return Music;
}


/*-----------------------------------------------------------------------------
    Font importers.
-----------------------------------------------------------------------------*/

//
// Import the ogg file.
//
FFont* ImportFLF( String Filename, String ResName )
{
	FFont* Font = nullptr;
	FILE* File	= _wfopen( *Filename, L"r" );
	{
		// First header line.
		Char Name[64];
		Int32 Height, RealHeight=-1, NumPages=0;
		fwscanf( File, L"%d %s\n", &Height, Name );

		// Temporary tables.
		Array<UInt8>	Remap(65536);
		Array<TGlyph>	Glyphs;
		Int32			iMaxChar = 0;
		mem::set( &Remap[0], Remap.size()*sizeof(UInt8), 0xff );

		// Read info about each glyph.
		while( !feof(File) )
		{
			Char C[2];
			Int32 X, Y, W, H, iBitmap;
			C[0] = *fgetws( C, 2, File );

			fwscanf( File, L"%d %d %d %d %d\n", &iBitmap, &X, &Y, &W, &H );	
			NumPages = max( NumPages, iBitmap+1 );

			TGlyph Glyph;
			Glyph.iBitmap	= iBitmap;
			Glyph.X			= X;
			Glyph.Y			= Y;
			Glyph.W			= W;
			Glyph.H			= H;	

			RealHeight		= max( RealHeight, H );
			iMaxChar		= max<Int32>( C[0], iMaxChar );
			Remap[(UInt16)C[0]] = Glyphs.push(Glyph);
		}

		// Test all pages, they are exists?
		String Dir = fm::getFilePath( *Filename );
		for( Int32 i=0; i<NumPages; i++ )
			if( !fm::fileExists(*(Dir+String::format(L"\\%s%d_%d.bmp", Name, Height, i))) )
			{
				MessageBox( 0, *String::format( L"Font page %i not found", i ), 
					L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );

				fclose(File);
				return nullptr;
			}

		// Allocate font and it bitmaps.
		Font			= NewObject<FFont>( ResName, nullptr );
		Font->Glyphs	= Glyphs;
		Font->Remap		= Remap;
		Font->Remap.setSize( iMaxChar+1 );
		Font->FileName	= fm::getFileName( *Filename ) + L".flf";

		for( Int32 i=0; i<NumPages; i++ )
		{
			String		BitFile = Dir  + String::format(L"\\%s%d_%d.bmp", Name, Height, i);
			FBitmap*	Page	= ImportBMP( BitFile, fm::getFileName( *Filename ));
			Page->BlendMode		= BLEND_Translucent;
			Page->Group			= L"";

			Page->SetOwner( Font );
			Font->Bitmaps.push( Page );
		}
	}
	fclose(File);

	// Convert palette from RGB to RGBA format with solid white color and alpha mask.
	for( Int32 iPage=0; iPage<Font->Bitmaps.size(); iPage++ )
	{
		FBitmap* Page = Font->Bitmaps[iPage];
		assert(Page->Format == BF_Palette8);

		for( Int32 i=0; i<Page->Palette.Colors.size(); i++ )
		{
			math::Color Ent = Page->Palette.Colors[i];
			Page->Palette.Colors[i] = math::Color( 0xff, 0xff, 0xff, Int32(Ent.r + Ent.g + Ent.b)/3 );
		}
	
		Page->BlendMode = BLEND_Alpha;
	}

	return Font;
}


/*-----------------------------------------------------------------------------
    Editor functions.
-----------------------------------------------------------------------------*/

//
// Import any resource. Creates and initializes  the resource.
// If some problem occurred return nullptr instead resource.
//
FResource* CEditor::ImportResource( String Filename, String ResName )
{
	assert( fm::getFileName( *Filename ) );

	// Try all formats.
	if( String::pos( L".bmp", String::lowerCase(Filename) ) != -1 )
	{
		// Simple bmp file.
		return ImportBMP( Filename, ResName ? ResName : fm::getFileName( *Filename ) );
	}
	if( String::pos( L".tga", String::lowerCase(Filename) ) != -1 )
	{
		// Simple tga file.
		return ImportTGA( Filename, ResName ? ResName : fm::getFileName( *Filename ) );
	}
	if( String::pos( L".png", String::lowerCase(Filename) ) != -1 )
	{
		// Advanced png file.
		return ImportPNG( Filename, ResName ? ResName : fm::getFileName( *Filename ) );
	}
	else if( String::pos( L".wav", String::lowerCase(Filename) ) != -1 )
	{
		// wav file.
		return ImportWAV( Filename, ResName ? ResName : fm::getFileName( *Filename ) );
	}
	else if( String::pos( L".ogg", String::lowerCase(Filename) ) != -1 )
	{
		// Ogg file.
		return ImportOGG( Filename, ResName ? ResName : fm::getFileName( *Filename ) );
	}
	else if( String::pos( L".flf", String::lowerCase(Filename) ) != -1 )
	{
		// Flf file.
		return ImportFLF( Filename, ResName ? ResName : fm::getFileName( *Filename ) );
	}
	else
	{
		// Bad format.
		MessageBox( 0, *String::format( L"Failed load '%s'. Unsupported file format", *Filename ), 
			L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );

		return nullptr;
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/