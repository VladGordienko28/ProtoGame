/*=============================================================================
    FrBuild.h: Predefined compiling directives.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

// Whether use assembler instead C++ code?
#define FLU_ASM			1

// Whether allow to use cheats console?
#define FLU_CONSOLE		1

// Whether use console & log file for debug.
#define FDEBUG_LOG		1

// An engine info.
#define FLU_VER			L"0.2 Alpha"
#define FLU_NAME		L"Fluorine"

// Hello page copyright string.
#define	FLU_COPYRIGHT	L"Copyright 2016-2018 Vlad Gordienko."

// Fluorine client name.
#if _DEBUG
#define CLIENT_EXE_NAME L"flu_client.exe"
#else
#define CLIENT_EXE_NAME L"Flu.exe"
#endif


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/