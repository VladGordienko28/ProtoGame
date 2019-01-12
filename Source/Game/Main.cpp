/*=============================================================================
    Main.cpp: Fluorine Engine main file.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

#include "Game.h"


//
// Application Entry Point.
//
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{	
	RegisterAll();

	CGame	Game;

	Game.Init( hInstance );
	Game.MainLoop();
	Game.Exit();

	return 0;
}
      

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/