/*=============================================================================
	Window.h: Window general include file.
	Created by Vlad Gordienko, Feb. 2018.
=============================================================================*/
#pragma once

// Flu includes.
#include "Core/Core.h"
#include "Engine/Engine.h"

// C++ includes.
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <windowsx.h>

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
extern Bool ExecuteColorDialog( HWND hWnd, math::Color& PickedColor, const math::Color Default = math::colors::BLACK );
extern Bool ExecuteOpenFileDialog( HWND hWnd, String& FileName, String Directory, const Char* Filters );
extern Bool ExecuteSaveFileDialog( HWND hWnd, String& FileName, String Directory, const Char* Filters );


struct SystemBitmap
{
public:
	Array<UInt8> data;
	Array<math::Color> palette;
	UInt32 width, height;
};


//
// GUI objects loaders.
//
extern SystemBitmap LoadBitmapFromResource( HINSTANCE hInstance, LPCTSTR ResID );

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/