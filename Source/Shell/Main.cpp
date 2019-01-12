/*=============================================================================
	Main.cpp: Fluorine Engine main file.
	Created by Vlad Gordienko, Apr. 2018.
=============================================================================*/

#include "Shell.h"


//
// Application Entry Point.
//
Integer wmain( Integer ArgC, Char *ArgV[] )
{
	RegisterAll();

	CShell	Shell;
	return Shell.Run( ArgC, ArgV );
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/