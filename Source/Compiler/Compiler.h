/*=============================================================================
	Compiler.h: Compiler general include file.
	Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/
#pragma once

// Flu includes.
#include "Engine\Engine.h"

/*-----------------------------------------------------------------------------
	Top level compiler functions.
-----------------------------------------------------------------------------*/

namespace Compiler
{
	//
	// An information about compiler
	// fatal error.
	//
	struct TError
	{
	public:
		FScript*	Script;
		String		Message;
		Int32		ErrorPos;
		Int32		ErrorLine;
	};

	//
	// Compiler functions.
	//
	extern Bool CompileAllScripts
	( 
		CObjectDatabase* InDatabase, 
		TArray<String>& OutWarnings, 
		TError& OutFatalError 
	);

	extern Bool DropAllScripts( CObjectDatabase* InDatabase );
};

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/