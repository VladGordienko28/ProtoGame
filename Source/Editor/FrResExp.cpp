/*=============================================================================
    FrResExp.cpp: Resource exporting.
    Copyright Dec.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    Bitmap exporters.
-----------------------------------------------------------------------------*/

//
// Export image to bmp or tga file.
//
Bool ExportBitmap( FBitmap* Bitmap, String Directory )
{
	assert(Bitmap && Bitmap->IsValidBlock());

	if( String::pos( L".bmp", String::lowerCase(Bitmap->FileName) ) != -1 )
	{
		//
		// Export bmp file.
		//
		fm::IBinaryFileWriter::Ptr saver = fm::writeBinaryFile( *( Directory + L"\\" + Bitmap->FileName ) );

		BITMAPFILEHEADER BmpHeader;
		BITMAPINFOHEADER BmpInfo;

		BmpHeader.bfType		= 0x4d42;
		BmpHeader.bfSize		= sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+Bitmap->VSize*Bitmap->USize;
		BmpHeader.bfReserved1	= 0;
		BmpHeader.bfReserved2	= 0;
		BmpHeader.bfOffBits		= sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);

		BmpInfo.biSize			= sizeof(BITMAPINFOHEADER);
		BmpInfo.biWidth			= Bitmap->USize;
		BmpInfo.biHeight		= Bitmap->VSize;
		BmpInfo.biPlanes		= 1;
		BmpInfo.biBitCount		= 24;
		BmpInfo.biCompression	= BI_RGB;
		BmpInfo.biSizeImage		= 0;
		BmpInfo.biXPelsPerMeter	= 0;
		BmpInfo.biYPelsPerMeter	= 0;
		BmpInfo.biClrUsed		= 0;
		BmpInfo.biClrImportant	= 0;

		saver->writeData( &BmpHeader, sizeof( BITMAPFILEHEADER ) );
		saver->writeData( &BmpInfo, sizeof( BITMAPINFOHEADER ) );

		if( Bitmap->Format == BF_Palette8 )
		{
			// Unpack to save as RGB.
			UInt8*	Data	= (UInt8*)Bitmap->GetData();
			for( Int32 V=0; V<Bitmap->VSize; V++ )
			{
				UInt8* Line	= &Data[(Bitmap->VSize-V-1) << Bitmap->UBits];

				for( Int32 U=0; U<Bitmap->USize; U++ )
				{
					math::Color	C		= Bitmap->Palette.Colors[Line[U]];
					UInt8	Buffer[3]	= { C.b, C.g, C.r };
					saver->writeData( Buffer, sizeof( Buffer ) );
				}
			}
		}
		else
		{
			// Save as RGB.
			math::Color* Data	= (math::Color*)Bitmap->GetData();
			for( Int32 V=0; V<Bitmap->VSize; V++ )
			{
				math::Color* Line	= &Data[(Bitmap->VSize-V-1) << Bitmap->UBits];

				for( Int32 U=0; U<Bitmap->USize; U++ )
				{
					UInt8 Buffer[3] = { Line[U].b, Line[U].g, Line[U].r };
					saver->writeData( Buffer, sizeof( Buffer ) );
				}
			}
		}
	}
	else if( String::pos( L".tga", String::lowerCase(Bitmap->FileName) ) != -1 )
	{
		//
		// Export tga file.
		//
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
		fm::IBinaryFileWriter::Ptr saver = fm::writeBinaryFile( *( Directory + L"\\" + Bitmap->FileName ) );

		TGAHeader	TgaHeader;

		TgaHeader.FileType			= 0;
		TgaHeader.ColorMapType		= 0;
		TgaHeader.ImageType			= 2;	// Uncompressed.
		TgaHeader.OrigX[0]			= 
		TgaHeader.OrigX[1]			= 
		TgaHeader.OrigY[0]			= 
		TgaHeader.OrigY[1]			= 0;
		TgaHeader.Width[0]			= Bitmap->USize;
		TgaHeader.Width[1]			= Bitmap->USize / 256;
		TgaHeader.Height[0]			= Bitmap->VSize;
		TgaHeader.Height[1]			= Bitmap->VSize / 256;
		TgaHeader.BPP				= 32;
		TgaHeader.ImageInfo			= 0;

		saver->writeData( &TgaHeader, sizeof( TGAHeader ) );

		// Save as RGBA.
		math::Color* Data	= (math::Color*)Bitmap->GetData();
		for( Int32 V=0; V<Bitmap->VSize; V++ )
		{
			math::Color* Line	= &Data[(Bitmap->VSize-V-1) << Bitmap->UBits];

			for( Int32 U=0; U<Bitmap->USize; U++ )
			{
				UInt8 Buffer[4] = { Line[U].b, Line[U].g, Line[U].r, Line[U].a };
				saver->writeData( Buffer, sizeof( Buffer ) );
			}
		}
	}
	if( String::pos( L".png", String::lowerCase(Bitmap->FileName) ) != -1 )
	{
		//
		// Export png file.
		//
		unsigned PngError;
		math::Color* Depalettized = nullptr;

		// Depalettize if required.
		if( Bitmap->Format == BF_Palette8 )
		{
			Depalettized = new math::Color[Bitmap->USize * Bitmap->VSize];
			UInt8* Pixels = (UInt8*)Bitmap->GetData();
			for( Int32 i=0; i<Bitmap->USize*Bitmap->VSize; i++ )
				Depalettized[i] = Bitmap->Palette.Colors[Pixels[i]];
		}

		// Save to file.
		AnsiChar file_name[1024];
		cstr::wideToMultiByte( file_name, arraySize( file_name ), *(Directory + L"\\" + Bitmap->FileName) );
		
		PngError = lodepng_encode32_file
		( 
			file_name,
			Bitmap->Format == BF_Palette8 ? (unsigned char*)Depalettized : (unsigned char*)Bitmap->GetData(),
			Bitmap->USize,
			Bitmap->VSize
		);
		if( PngError )
		{
			MessageBox( 0, *String::format( L"Png: Failed save png. %u: %hs", PngError, lodepng_error_text(PngError) ), 
				L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );
		}

		if( Depalettized )
			delete[] Depalettized;

		return PngError == 0;
	}
	else
		return false;
}


/*-----------------------------------------------------------------------------
    Script exporters.
-----------------------------------------------------------------------------*/

//
// Export to flu file.
//
Bool ExportScript( FScript* Script, String Directory )
{
	assert(Script);
	
	if( Script->IsScriptable() )
	{
		CTextWriter	TextFile(Directory+L"\\"+Script->FileName);
		for( Int32 iLine=0; iLine<Script->Text.size(); iLine++ )
			TextFile.WriteString( Script->Text[iLine] );
		return true;
	}
	else
		return false;
}


/*-----------------------------------------------------------------------------
    Sound exporters.
-----------------------------------------------------------------------------*/

//
// Export to wav file.
//
Bool ExportSound( FSound* Sound, String Directory )
{
	assert(Sound && Sound->IsValidBlock());

	fm::IBinaryFileWriter::Ptr saver = fm::writeBinaryFile( *(Directory + L"\\" + Sound->FileName) );
	saver->writeData( Sound->GetData(), Sound->GetBlockSize() );

	return true;
}


/*-----------------------------------------------------------------------------
    Editor functions.
-----------------------------------------------------------------------------*/

//
// Export resource to file, such as FBitmap -> .bmp, Sound -> .wav.
// Return true, if successfully saved. If some error occurs or
// resource type are unsupported return false.
// bOverride - should we override file if it exists? It's important
// for large resources.
//
Bool CEditor::ExportResource( FResource* Res, String Directory, Bool bOverride )
{
	assert(Res && Directory);

	// File already exists.
	if( !bOverride && fm::fileExists( *(Directory+L"\\"+Res->FileName) ) )
		return false;

	if( Res->IsA(FSound::MetaClass) )
	{
		// Export sound file.
		return ExportSound( As<FSound>(Res), Directory );
	}
	else if( Res->IsA(FScript::MetaClass) )
	{
		// Export script to file.
		return ExportScript( As<FScript>(Res), Directory );
	}
	else if( Res->IsA(FBitmap::MetaClass) )
	{
		// Export bitmap to file.
		return ExportBitmap( As<FBitmap>(Res), Directory );
	}
	else
	{
		// Unsupported type.
		MessageBox( 0, *String::format( L"Failed save '%s'. Unsupported resource type '%s'", *Res->GetName(), *Res->GetClass()->Name ), 
			L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );

		return false;
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/