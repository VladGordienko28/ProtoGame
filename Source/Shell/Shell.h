//-----------------------------------------------------------------------------
//	Shell.h: A shell main include file
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------
#pragma once

// Fluorine includes
#include "Core/Core.h"
#include "Resource/Resource.h"
#include "Engine/Engine.h"
#include "Window/Window.h"
#include "Network/Network.h"
#include "DirectX11/DirectX11.h"

// Std includes
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

// Shell includes
#include "IApp.h"
#include "ResourceServerApp.h"