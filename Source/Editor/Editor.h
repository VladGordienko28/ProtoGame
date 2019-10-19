/*=============================================================================
    Editor.h: Editor general include file.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/
#pragma once

// Flu includes.
#include "Core/Core.h"
#include "Engine/Engine.h"
#include "GUI/GUI.h"
#include "Window/Window.h"
#include "OpenGL/OpenGLRend.h"
#include "OpenAL/OpenALAud.h"
#include "Compiler/Compiler.h"
#include "Network/Network.h"
#include "Resource/Resource.h"
#include "Image/Image.h"

// experimental:
#include "DirectX11/DirectX11.h"
#include "FFX/FFX.h"
#include "Experimental/DxAdapter.h"

#define WIN32_LEAN_AND_MEAN

// C++ includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>


#undef DrawText
#undef LoadBitmap
#undef FindText
#undef EVENT_MAX




// Partial classes tree.
class CEditor;
class WResourceBrowser;
class WObjectInspector;
class WCodeEditor;
class WCompilerOutput;
class WEntitySearchDialog;
class WResourceBrowser;
class TMouseDragInfo;
class WWatchListDialog;


// Editor includes.
#include "FrEdUtil.h"
#include "FrGizmo.h"
#include "FrPage.h"
#include "FrHello.h"
#include "FrTileEd.h"
#include "FrKeyEdit.h"
#include "FrUndoRedo.h"
#include "FrLevPage.h"
#include "FrBitPage.h"
#include "FrPlayPage.h"
#include "FrScriptPage.h"
#include "FrAnimPage.h"
#include "FrSkelPage.h"
#include "FrEdToolBar.h"
#include "FrFields.h"
#include "FrResBrow.h"
#include "FrEdMenu.h"
#include "FrTask.h"
#include "FrGamBld.h"
#include "FrEditor.h"
#include "FrAbout.h"

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/