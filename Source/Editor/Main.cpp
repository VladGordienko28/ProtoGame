/*=============================================================================
    Main.cpp: Fluorine Engine main file.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"


//
// Application Entry Point.
//
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	RegisterAll();

	CEditor	Editor;

	Editor.Init( hInstance );
	Editor.MainLoop();
	Editor.Exit();

	UnRegisterAll();

	return 0;
}
         

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/