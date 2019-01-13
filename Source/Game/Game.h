/*=============================================================================
    Game.h: Game general include file.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/
#pragma once

#define WIN32_LEAN_AND_MEAN

// C++ includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <mmsystem.h>

#pragma pack( push, 8 )
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")
#pragma pack( pop ) 

#undef DrawText
#undef LoadBitmap
#undef FindText

// Flu includes.
#include "Core/Core.h"
#include "Engine\Engine.h"
#include "OpenGL\OpenGLRend.h"
#include "OpenAL\OpenALAud.h"
#include "Window\Window.h"
#include "Network\Network.h"

// Game includes.
#include "FrConsole.h"
#include "FrGame.h"

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/