/*=============================================================================
	Shell.h: Shell general include file.
	Copyright Apr.2018 Vlad Gordienko.
=============================================================================*/
#ifndef _FLU_SHELL_
#define _FLU_SHELL_

// C++ includes.
#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
   
#pragma pack( push, 8 )
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")
#pragma pack( pop ) 

#undef DrawText
#undef LoadBitmap
#undef FindText

// Flu includes.
#include "..\Engine\Engine.h"
#include "..\Compiler\Compiler.h"
#include "..\Window\Window.h"

// Shell includes.
#include "FrShell.h"


#endif
/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/