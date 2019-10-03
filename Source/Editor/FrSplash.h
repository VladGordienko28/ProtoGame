/*=============================================================================
    FrSplash.h: Application splash dialog.
    Copyright Dec.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    CSplash.
-----------------------------------------------------------------------------*/

//
// A splash.
//
class CSplash
{
public:
	// CSplash interface.
	CSplash( LPCTSTR BitmapID );
	~CSplash();

private:
	// Splash internal.
	HWND		hWnd;
	HBITMAP		hBitmap;
	Int32		XSize;
	Int32		YSize;
};


/*-----------------------------------------------------------------------------
    CSplash implementation.
-----------------------------------------------------------------------------*/

//
// Splash dialog WinProc.
//
LRESULT CALLBACK SplashWndProc( HWND, UINT, WPARAM, LPARAM )
{ 
	return 0; 
}


//
// Splash constructor.
//
CSplash::CSplash( LPCTSTR BitmapID )
{
	SystemBitmap Bitmap	= LoadBitmapFromResource(GEditor->hInstance, BitmapID);
	{
		BITMAPINFO*	Info	= (BITMAPINFO*)mem::alloc(sizeof(BITMAPINFO)+sizeof(RGBQUAD)*Bitmap.palette.size());
		HDC			hDc		= GetDC(nullptr);

		Info->bmiHeader.biBitCount		= 8;
		Info->bmiHeader.biClrImportant	= 
		Info->bmiHeader.biClrUsed		= Bitmap.palette.size();
		Info->bmiHeader.biCompression	= 0;
		Info->bmiHeader.biHeight		= Bitmap.height;
		Info->bmiHeader.biPlanes		= 1;
		Info->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
		Info->bmiHeader.biSizeImage		= Bitmap.width * Bitmap.height;
		Info->bmiHeader.biWidth			= Bitmap.width;
		Info->bmiHeader.biXPelsPerMeter	= 
		Info->bmiHeader.biYPelsPerMeter	= 2834;

		// Flip palette RGBA -> BGR.
		for( Int32 i=0; i<Bitmap.palette.size(); i++ )
		{
			math::Color Col = Bitmap.palette[i];
			Info->bmiColors[i].rgbBlue		= Col.b;
			Info->bmiColors[i].rgbGreen		= Col.g;
			Info->bmiColors[i].rgbRed		= Col.r;
			Info->bmiColors[i].rgbReserved	= 0;
		}

		// Store size.
		XSize		= Bitmap.width;
		YSize		= Bitmap.height;

		// Flip image.
		UInt8*	Data	= (UInt8*)&Bitmap.data[0];
		UInt8	Buffer[1024];
		for( Int32 V=0; V<Bitmap.height/2; V++ )
		{
			mem::copy( Buffer, &Data[V*Bitmap.width], Bitmap.width );
			mem::copy( &Data[V*Bitmap.width], &Data[(Bitmap.height-V-1)*Bitmap.width], Bitmap.width );
			mem::copy( &Data[(Bitmap.height-V-1)*Bitmap.width], Buffer, Bitmap.width );
		}

		// To hBitmap;
		hBitmap		= CreateDIBitmap
		(
			hDc,
			&Info->bmiHeader,
			CBM_INIT,
			&Bitmap.data[0],
			Info,
			DIB_RGB_COLORS
		);
		ReleaseDC( nullptr, hDc );
		mem::free( Info );
	}
	
	// Allocate dialog.
	hWnd	= CreateDialog
	(
		GEditor->hInstance,
		MAKEINTRESOURCE(IDD_SPLASH),
		nullptr,
		(DLGPROC)SplashWndProc
	);
		
	if( hWnd )
	{
		HWND hWndLogo = GetDlgItem( hWnd, IDC_LOGO );
		if( hWndLogo )
		{
			SetWindowPos
			( 
				hWnd, 
				HWND_TOPMOST, 
				(GetSystemMetrics(SM_CXSCREEN)-XSize)/2, 
				(GetSystemMetrics(SM_CYSCREEN)-YSize)/2, 
				XSize, YSize, 
				SWP_SHOWWINDOW 
			);
			SetWindowPos( hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE );
			SendMessage( hWndLogo, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap );
			UpdateWindow( hWnd );
		}
	}
}


//
// Splash destructor.
//
CSplash::~CSplash()
{
	DestroyWindow(hWnd);
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/