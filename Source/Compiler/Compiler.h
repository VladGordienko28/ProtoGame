/*=============================================================================
	Compiler.h: Compiler general include file.
	Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/
#ifndef _FLU_COMPILER_
#define _FLU_COMPILER_

// Flu includes.
#include "..\Engine\Engine.h"

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
		Integer		ErrorPos;
		Integer		ErrorLine;
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


#endif
/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/