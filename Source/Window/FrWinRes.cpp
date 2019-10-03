/*=============================================================================
	FrGUIRes.cpp: GUI resources loading.
	Copyright Jan.2017 Vlad Gordienko.
=============================================================================*/

#include "Window.h"

/*-----------------------------------------------------------------------------
	CResourceStream.
-----------------------------------------------------------------------------*/

//
// Stream to load resource.
//
class CResourceStream: public CSerializer
{
public:
	// CResourceStream interface.
	CResourceStream( HINSTANCE hInstance, LPCTSTR ResID, LPCTSTR ResType )
	{
		hResInfo	= FindResource( hInstance, ResID, ResType );
		assert(hResInfo != 0);
		hGlobal		= LoadResource( hInstance, hResInfo );
		assert(hGlobal != 0);

		Size		= SizeofResource( hInstance, hResInfo );
		Memory		= LockResource(hGlobal);
		Mode		= SM_Load;
		Pos			= 0;
	}
	~CResourceStream()
	{
		UnlockResource(hGlobal);
		FreeResource(hGlobal);
	}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count )
	{
		mem::copy( Mem, (UInt8*)Memory + Pos, Count );
		Pos	+= Count;
	}
	void SerializeRef( FObject*& Obj )
	{
		fatal(L"CResourceStream::SerializeRef");
	}
	SizeT TotalSize()
	{
		return Size;
	}
	void Seek( SizeT NewPos )
	{
		Pos	= NewPos;
	}
	SizeT Tell()
	{
		return Pos;
	}

private:
	// Internal.
	HRSRC				hResInfo;
	HGLOBAL				hGlobal;
	void*				Memory;
	SizeT				Size, Pos;
};


/*-----------------------------------------------------------------------------
    Bitmap loading.
-----------------------------------------------------------------------------*/

// 
// Load bitmap.
//
SystemBitmap LoadBitmapFromResource( HINSTANCE hInstance, LPCTSTR ResID )
{
	CResourceStream		Stream( hInstance, ResID, RT_BITMAP );

	BITMAPINFOHEADER	Info;
	Stream.SerializeData( &Info, sizeof(BITMAPINFOHEADER) );
	assert(Info.biBitCount==8);

	SystemBitmap bitmap;


	bitmap.width		= Info.biWidth;
	bitmap.height		= Info.biHeight;
	bitmap.data.setSize( bitmap.width * bitmap.height * sizeof(UInt8) );

	// Load palette.
	RGBQUAD RawPalette[256];
	Stream.SerializeData( RawPalette, sizeof(RGBQUAD)*Info.biClrUsed );

	bitmap.palette.setSize(Info.biClrUsed);
	for( Int32 i=0; i<Info.biClrUsed; i++ )
	{
		math::Color Col( RawPalette[i].rgbRed, RawPalette[i].rgbGreen, RawPalette[i].rgbBlue, 0xff );
		//if( Col == img::MASK_COLOR )	Col.a	= 0x00;
		bitmap.palette[i]	= Col;
	}

	// Load data, don't forget V-flip.
	UInt8* Data = (UInt8*)&bitmap.data[0];
	for( Int32 V=0; V<bitmap.height; V++ )
		Stream.SerializeData
		(
			&Data[(bitmap.height-1-V) * bitmap.width],
			bitmap.width * sizeof(UInt8)
		);

	// Return it.
	return bitmap;
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/