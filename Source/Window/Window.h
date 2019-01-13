/*=============================================================================
	Window.h: Window general include file.
	Created by Vlad Gordienko, Feb. 2018.
=============================================================================*/
#pragma once

// C++ includes.
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <windowsx.h>

// Flu includes.
#include "Core/Core.h"
#include "Engine/Engine.h"

// Window includes.
#include "FrPlat.h"
#include "StackTrace.h"
#include "LogCallbackConsole.h"
#include "LogCallbackDebug.h"
#include "LogCallbackFile.h"

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

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/