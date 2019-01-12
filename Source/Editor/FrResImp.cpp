/*=============================================================================
    FrResImp.cpp: Resource importing.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    Utility.
-----------------------------------------------------------------------------*/

//
// Retrieve the name of the file.
//
String GetFileName( String FileName )
{
	Int32 i, j;

	for( i=FileName.Len()-1; i>=0; i-- )
		if( FileName[i] == L'\\' )
			break;

	j = String::Pos( L".", FileName );
	return String::Copy( FileName, i+1, j-i-1 );
}


//
// Retrieve the directory of the file.
//
String GetFileDir( String FileName )
{ 
	Int32 i;
	for( i=FileName.Len()-1; i>=0; i-- )
		if( FileName[i] == L'\\' )
			break;

	return String::Copy( FileName, 0, i );
}


/*-----------------------------------------------------------------------------
    Image importers.
-----------------------------------------------------------------------------*/

//
// Import the bmp file.
//
FBitmap* ImportBMP( String Filename, String ResName )
{
	CFileLoader Loader( Filename );

	BITMAPFILEHEADER BmpHeader;
	BITMAPINFOHEADER BmpInfo;
	Loader.SerializeData( &BmpHeader, sizeof(BITMAPFILEHEADER) );
	Loader.SerializeData( &BmpInfo, sizeof(BITMAPINFOHEADER) );

	if( BmpHeader.bfType != 0x4d42 )
	{
		warn( L"\"%s\" is not BMP file.", *Filename );
		return nullptr;
	}

	if( !(((BmpInfo.biWidth)&(BmpInfo.biWidth-1)) == 0 && ((BmpInfo.biHeight)&(BmpInfo.biHeight-1)) == 0) )
	{
		warn( L"Bitmap size should be power of two" );
		return nullptr;
	}

	// Raw bmp palette.
	RGBQUAD BmpPalette[256];
	if( BmpInfo.biClrUsed )
	{
		BmpInfo.biClrUsed = Min( BmpInfo.biClrUsed, (DWORD)256 );
		Loader.SerializeData( BmpPalette, sizeof(RGBQUAD)*BmpInfo.biClrUsed );
	}

	TColor* TempData = new TColor[BmpInfo.biWidth * BmpInfo.biHeight];
	TArray<TColor> Palette;
	Int32 UMask = BmpInfo.biWidth-1;
	Int32 UBits = IntLog2( BmpInfo.biWidth );
	Int32 VMask = (BmpInfo.biHeight-1) << UBits;

	// Load entire bmp data.
	Loader.Seek( BmpHeader.bfOffBits );	
	if( BmpInfo.biBitCount == 24 )
	{
		// 24 - bit.
		for( Int32 i=0; i<BmpInfo.biWidth*BmpInfo.biHeight; i++ )
		{
			UInt8 RawPix[4];
			Loader.SerializeData( RawPix, 3*sizeof(UInt8) );
			TColor Source( RawPix[2], RawPix[1], RawPix[0], 0xff );
			if( Source == MASK_COLOR )	Source.A = 0x00;
			TempData[(VMask&~i)|(i&UMask)] = Source;		// Flip image here.
		}
	}
	else if( BmpInfo.biBitCount == 32 )
	{
		// 32 - bit.
		for( Int32 i=0; i<BmpInfo.biWidth*BmpInfo.biHeight; i++ )
		{
			UInt8 RawPix[4];
			Loader.SerializeData( RawPix, 4*sizeof(UInt8) );
			TColor Source( RawPix[2], RawPix[1], RawPix[0], 0xff );
			if( Source == MASK_COLOR )	Source.A = 0x00;
			TempData[(VMask&~i)|(i&UMask)] = Source;		// Flip image here.
		}
	}
	else if( BmpInfo.biBitCount == 8 )
	{
		// 8 - bit.
		for( Int32 i=0; i<BmpInfo.biWidth*BmpInfo.biHeight; i++ )
		{
			UInt8 iColor;
			Serialize( Loader, iColor );
			TColor Source( BmpPalette[iColor].rgbRed, BmpPalette[iColor].rgbGreen, BmpPalette[iColor].rgbBlue, 0xff );
			if( Source == MASK_COLOR )	Source.A = 0x00;
			TempData[(VMask&~i)|(i&UMask)] = Source;		// Flip image here.
		}
	}
	else
	{
		// Bad bitmap format.
		delete[] TempData;
		warn( L"Bad bit count %d", BmpInfo.biBitCount );
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
		if( Palette.AddUnique(TempData[i]) > 255 )
			break;

	// Allocate bitmap and initialize it.
	FBitmap*	Bitmap	= NewObject<FBitmap>( ResName, nullptr );
	Bitmap->Init( BmpInfo.biWidth, BmpInfo.biHeight );
	Bitmap->FileName	= GetFileName(Filename) + L".bmp";

	// Load data into bitmap.
	if( Palette.Num() <= 256 )
	{
		// Palette.
		Palette.Sort( []( const TColor& A, const TColor& B )->Bool
		{
			return A.D < B.D;
		} );

		Bitmap->Format	= BF_Palette8;
		Bitmap->Palette.Allocate( Palette.Num() );
		for( Int32 i=0; i<Palette.Num(); i++ )
			Bitmap->Palette.Colors[i] = Palette[i];

		Bitmap->AllocateBlock( sizeof(UInt8)*Bitmap->USize*Bitmap->VSize );
		UInt8* BitDat = (UInt8*)Bitmap->GetData();
		for( Int32 i=0; i < Bitmap->USize*Bitmap->VSize; i++ )
			BitDat[i]	= Palette.FindItem( TempData[i] );
	}
	else
	{
		// RGBA.
		Bitmap->Format	= BF_RGBA;
		Bitmap->AllocateBlock( sizeof(TColor)*Bitmap->USize*Bitmap->VSize );
		mem::copy( Bitmap->GetData(), TempData, sizeof(TColor)*Bitmap->USize*Bitmap->VSize );
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

	CFileLoader Loader( Filename );
	TGAHeader	TgaHeader;
	Loader.SerializeData( &TgaHeader, sizeof(TGAHeader) );

	// Only 24 and 32 bits supported.
	if( TgaHeader.ImageType!=2 && TgaHeader.ImageType!=10 )
	{
		warn( L"Only 24 and 32 bit TGA supported" );
		return nullptr;
	}

	// Doesn't allow color map.
	if( TgaHeader.ColorMapType != 0 )
	{
		warn( L"Color-mapped TGA doesn't supported" );
		return nullptr;
	}

	// Figure out general properties.
	Int32	Width		= TgaHeader.Width[0] + TgaHeader.Width[1]*256;
	Int32	Height		= TgaHeader.Height[0] + TgaHeader.Height[1]*256;
	Int32	ColorDepth	= TgaHeader.BPP;
	Int32	ImageSize	= Width*Height*(ColorDepth/8);

	if( !(((Width)&(Width-1)) == 0 && ((Height)&(Height-1)) == 0) )
	{
		warn( L"Image size should be power of two" );
		return nullptr;
	}
	if( ColorDepth < 24 )
	{
		warn( L"Only 24 and 32 bit TGA supported" );
		return nullptr;
	}

	FBitmap*	Bitmap	= nullptr;

	if( TgaHeader.ImageType == 2 )
	{
		// Standard uncompressed tga.
		UInt8* TmpImage	= new UInt8[ImageSize];
		Loader.SerializeData( TmpImage, ImageSize );

		// Allocate bitmap and initialize it.
		Bitmap				= NewObject<FBitmap>( ResName, nullptr );
		Bitmap->Init( Width, Height );
		Bitmap->FileName	= GetFileName(Filename) + L".tga";
		Bitmap->Format		= BF_RGBA;
		Bitmap->AllocateBlock( sizeof(TColor)*Bitmap->USize*Bitmap->VSize );
		TColor* Dest		= (TColor*)Bitmap->GetData();

		Int32 UMask = Width-1;
		Int32 UBits = IntLog2( Width );
		Int32 VMask = (Height-1) << UBits;

		if( TgaHeader.BPP == 24 )
		{
			// 24-bit uncompressed.
			for( Int32 i=0; i<Width*Height; i++ )
			{
				TColor Source( TmpImage[i*3+2], TmpImage[i*3+1], TmpImage[i*3+0], 0xff );
				if( Source == MASK_COLOR )	Source.A = 0x00;
				Dest[(VMask&~i)|(i&UMask)]	= Source;
			}
		}
		else
		{
			// 32-bit uncompressed.
			for( Int32 i=0; i<Width*Height; i++ )
			{
				TColor Source( TmpImage[i*4+2], TmpImage[i*4+1], TmpImage[i*4+0], TmpImage[i*4+3] );
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
		UInt8*	Compressed	= new UInt8[Loader.TotalSize()-sizeof(TGAHeader)];
		Loader.SerializeData( Compressed, Loader.TotalSize()-sizeof(TGAHeader) );

		// Allocate bitmap and initialize it.
		Bitmap				= NewObject<FBitmap>( ResName, nullptr );
		Bitmap->Init( Width, Height );
		Bitmap->FileName	= GetFileName(Filename) + L".tga";
		Bitmap->Format		= BF_RGBA;
		Bitmap->AllocateBlock( sizeof(TColor)*Bitmap->USize*Bitmap->VSize );
		TColor* Image		= (TColor*)Bitmap->GetData();

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
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].R	= Compressed[WalkBuffer+i*Stride+2];
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].G	= Compressed[WalkBuffer+i*Stride+1];
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].B	= Compressed[WalkBuffer+i*Stride+0];
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].A	= Compressed[WalkBuffer+i*Stride+3];

					WalkColor++;
					WalkPixel++;
				}

				WalkBuffer	+= (Front+1)*Stride;
			}
			else
			{
				for( Int32 i=0; i<=Front-128; i++ )
				{
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].R	= Compressed[WalkBuffer+2];
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].G	= Compressed[WalkBuffer+1];
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].B	= Compressed[WalkBuffer+0];
					Image[(VMask&~WalkColor)|(WalkColor&UMask)].A	= Compressed[WalkBuffer+3];

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
				Image[i].A	= 0xff;
				if( Image[i] == MASK_COLOR )
					Image[i].A	= 0x00;
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
	wcstombs( file_name, *Filename, arr_len(file_name) );	// It's not good.
	PngError = lodepng_decode32_file( &PngImage, &PngWidth, &PngHeight, file_name );
	if( PngError )
	{
		warn( L"Png: Failed error png. %u: %hs", PngError, lodepng_error_text(PngError) );
		free(PngImage);
		return nullptr;
	}
	if( !(IsPowerOfTwo(PngWidth) && IsPowerOfTwo(PngHeight)) )
	{
		warn( L"Png size should be power of two" );
		free(PngImage);
		return nullptr;
	}

	Int32 UMask = PngWidth-1;
	Int32 UBits = IntLog2(PngWidth);
	Int32 VMask = (PngHeight-1) << UBits;
	TColor*	SourceData = (TColor*)PngImage;

	// Allocate bitmap and initialize it.
	FBitmap* Bitmap = NewObject<FBitmap>( ResName, nullptr );
	Bitmap->Init( PngWidth, PngHeight );
	Bitmap->FileName = GetFileName(Filename) + L".png";

	// Figure out it's a palette or not.
	TArray<TColor> Palette;
	for( Int32 i=0; i<PngWidth*PngHeight; i++ )
		if( Palette.AddUnique(SourceData[i]) > 255 )
			break;

	// Load data into bitmap.
	if( Palette.Num() <= 256 )
	{
		// Palette.
		Palette.Sort( []( const TColor& A, const TColor& B )->Bool
		{
			return A.D < B.D;
		} );
		Bitmap->Format	= BF_Palette8;
		Bitmap->Palette.Allocate( Palette.Num() );
		for( Int32 i=0; i<Palette.Num(); i++ )
			Bitmap->Palette.Colors[i] = Palette[i];

		Bitmap->AllocateBlock( sizeof(UInt8)*Bitmap->USize*Bitmap->VSize );
		UInt8* Dest = (UInt8*)Bitmap->GetData();
		for( Int32 i=0; i < Bitmap->USize*Bitmap->VSize; i++ )
			Dest[i]	= Palette.FindItem( SourceData[i] );
	}
	else
	{
		// RGBA.
		Bitmap->Format	= BF_RGBA;
		Bitmap->AllocateBlock( sizeof(TColor)*Bitmap->USize*Bitmap->VSize );
		mem::copy( Bitmap->GetData(), SourceData, sizeof(TColor)*Bitmap->USize*Bitmap->VSize );
	}

	// Turn on alpha or not?
	Bool bMaskOnly = true;
	Bitmap->BlendMode = BLEND_Regular;
	for( Int32 i=0; i<PngWidth*PngHeight; i++ )
		if( SourceData[i].A != 255 )
			if( SourceData[i].A != 0 )
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
	CFileLoader Loader( Filename );
	FSound*	Sound = NewObject<FSound>( ResName, nullptr );
	UInt32 FileSize = Loader.TotalSize();

	Sound->AllocateBlock( FileSize );
	Sound->FileName = GetFileName(Filename) + L".wav";
	Loader.SerializeData( Sound->GetData(), FileSize );

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
	if( String::Pos( String::UpperCase(GDirectory), String::UpperCase(Filename) ) != -1 )
	{
		// In directory of exe.
		Music->FileName	=	String::Copy
							( 
								Filename, 
								GDirectory.Len()+1, 
								Filename.Len()-GDirectory.Len()-1 
							);
	}
	else
	{
		// External directory.
		Music->FileName	=	GetFileName(Filename) + L".ogg";
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
		TArray<UInt8>	Remap(65536);
		TArray<TGlyph>	Glyphs;
		Int32			iMaxChar = 0;
		mem::set( &Remap[0], Remap.Num()*sizeof(UInt8), 0xff );

		// Read info about each glyph.
		while( !feof(File) )
		{
			Char C[2];
			Int32 X, Y, W, H, iBitmap;
			C[0] = *fgetws( C, 2, File );

			fwscanf( File, L"%d %d %d %d %d\n", &iBitmap, &X, &Y, &W, &H );	
			NumPages = Max( NumPages, iBitmap+1 );

			TGlyph Glyph;
			Glyph.iBitmap	= iBitmap;
			Glyph.X			= X;
			Glyph.Y			= Y;
			Glyph.W			= W;
			Glyph.H			= H;	

			RealHeight		= Max( RealHeight, H );
			iMaxChar		= Max<Int32>( C[0], iMaxChar );
			Remap[(UInt16)C[0]] = Glyphs.Push(Glyph);
		}

		// Test all pages, they are exists?
		String Dir = GetFileDir( Filename );
		for( Int32 i=0; i<NumPages; i++ )
			if( !GPlat->FileExists(Dir+String::Format(L"\\%s%d_%d.bmp", Name, Height, i)) )
			{
				warn( L"Font page %i not found", i );
				fclose(File);
				return nullptr;
			}

		// Allocate font and it bitmaps.
		Font			= NewObject<FFont>( ResName, nullptr );
		Font->Glyphs	= Glyphs;
		Font->Remap		= Remap;
		Font->Remap.SetNum( iMaxChar+1 );
		Font->FileName	= GetFileName(Filename) + L".flf";

		for( Int32 i=0; i<NumPages; i++ )
		{
			String		BitFile = Dir  + String::Format(L"\\%s%d_%d.bmp", Name, Height, i);
			FBitmap*	Page	= ImportBMP( BitFile, GetFileName(BitFile) );
			Page->BlendMode		= BLEND_Translucent;
			Page->Group			= L"";

			Page->SetOwner( Font );
			Font->Bitmaps.Push( Page );
		}
	}
	fclose(File);

	// Convert palette from RGB to RGBA format with solid white color and alpha mask.
	for( Int32 iPage=0; iPage<Font->Bitmaps.Num(); iPage++ )
	{
		FBitmap* Page = Font->Bitmaps[iPage];
		assert(Page->Format == BF_Palette8);

		for( Int32 i=0; i<Page->Palette.Colors.Num(); i++ )
		{
			TColor Ent = Page->Palette.Colors[i];
			Page->Palette.Colors[i] = TColor( 0xff, 0xff, 0xff, Int32(Ent.R + Ent.G + Ent.B)/3 );
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
	assert(GPlat->FileExists(Filename));

	// Try all formats.
	if( String::Pos( L".bmp", String::LowerCase(Filename) ) != -1 )
	{
		// Simple bmp file.
		return ImportBMP( Filename, ResName ? ResName : GetFileName(Filename) );
	}
	if( String::Pos( L".tga", String::LowerCase(Filename) ) != -1 )
	{
		// Simple tga file.
		return ImportTGA( Filename, ResName ? ResName : GetFileName(Filename) );
	}
	if( String::Pos( L".png", String::LowerCase(Filename) ) != -1 )
	{
		// Advanced png file.
		return ImportPNG( Filename, ResName ? ResName : GetFileName(Filename) );
	}
	else if( String::Pos( L".wav", String::LowerCase(Filename) ) != -1 )
	{
		// wav file.
		return ImportWAV( Filename, ResName ? ResName : GetFileName(Filename) );
	}
	else if( String::Pos( L".ogg", String::LowerCase(Filename) ) != -1 )
	{
		// Ogg file.
		return ImportOGG( Filename, ResName ? ResName : GetFileName(Filename) );
	}
	else if( String::Pos( L".flf", String::LowerCase(Filename) ) != -1 )
	{
		// Flf file.
		return ImportFLF( Filename, ResName ? ResName : GetFileName(Filename) );
	}
	else
	{
		// Bad format.
		warn( L"Failed load '%s'. Unsupported file format", *Filename );
		return nullptr;
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/