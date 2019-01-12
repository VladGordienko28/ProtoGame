/*=============================================================================
    OpenGLRend.h: OpenGL general include file.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/
#ifndef _FLU_OPENGL_
#define _FLU_OPENGL_

// C++ includes.
#pragma comment (lib, "opengl32.lib")  
#include <Windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include "glext.h"

// Flu includes.
#include "..\Engine\Engine.h"

// Partial classes tree.
class COpenGLCanvas;
class COpenGLRender;
class COpenGLShader;


// Render includes.
#include "FrGLShader.h"
#include "FrGLRender.h"


#endif
/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/