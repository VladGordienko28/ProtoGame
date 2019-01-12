/*=============================================================================
	Window.h: Window general include file.
	Created by Vlad Gordienko, Feb. 2018.
=============================================================================*/
#ifndef _FLU_WINDOW_
#define _FLU_WINDOW_

// C++ includes.
#include <windows.h>
#include <windowsx.h>

// Flu includes.
#include "../Engine/Engine.h"

// Window includes.
#include "FrPlat.h"


/*-----------------------------------------------------------------------------
	Utils.
-----------------------------------------------------------------------------*/

//
// Window dialogs functions.
//
extern Bool ExecuteColorDialog( HWND hWnd, TColor& PickedColor, const TColor Default = COLOR_Black );
extern Bool ExecuteOpenFileDialog( HWND hWnd, String& FileName, String Directory, const Char* Filters );
extern Bool ExecuteSaveFileDialog( HWND hWnd, String& FileName, String Directory, const Char* Filters );


//
// GUI objects loaders.
//
extern TStaticBitmap* LoadBitmapFromResource( HINSTANCE hInstance, LPCTSTR ResID );
extern TStaticFont* LoadFontFromResource( HINSTANCE hInstance, LPCTSTR FontID, LPCTSTR BitmapID );


#endif
/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/