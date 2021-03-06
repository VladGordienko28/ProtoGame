/*=============================================================================
	FrBuild.h: Predefined compiling directives.
	Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/
#pragma once

// Platform bitness
#if !_WIN64
	#define FLU_X32 1
	#define FLU_X64 0
#else
	#define FLU_X32 0
	#define FLU_X64 1
#endif

// Platform
#if _WIN32
	#define FLU_PLATFORM_WINDOWS 1
#else
	#define FLU_PLATFORM_WINDOWS 0
#endif

// XBox support
#if _SCORPIO
	#define FLU_PLATFORM_XBOX 1
#else
	#define FLU_PLATFORM_XBOX 0
#endif

// Configuration
#if _DEBUG
	#define FLU_DEBUG 1
	#define FLU_RELEASE 0
#else
	#define FLU_DEBUG 0
	#define FLU_RELEASE 1
#endif

// Profile
#define FLU_ENABLE_PROFILER 1 || FLU_DEBUG
#define FLU_PROFILE_MEMORY FLU_DEBUG
#define FLU_PROFILE_GPU FLU_DEBUG

// Memory leaks
#define FLU_ENABLE_MEM_TRACKING FLU_DEBUG && FLU_PLATFORM_WINDOWS && !FLU_PLATFORM_XBOX

// Assertions
#define FLU_ENABLE_ASSERT 1 || FLU_DEBUG

// Wide char
#define FLU_USE_WIDECHAR UNICODE

// Whether use assembler instead C++ code?
#define FLU_ASM		0

// Whether allow to use cheats console?
#define FLU_CONSOLE		1

// An engine info.
#define FLU_VERSION		L"0.3 Alpha"
#define FLU_NAME		L"Fluorine"

// Hello page copyright string.
#define	FLU_COPYRIGHT	L"Copyright 2016-2019 Vlad Gordienko"

// Fluorine client name.
#if FLU_DEBUG
#define CLIENT_EXE_NAME L"flu_client.exe"
#else
#define CLIENT_EXE_NAME L"Flu.exe"
#endif

// Fluorine engine configuration
#if FLU_DEBUG
	#if FLU_X32
		#define FLUORINE_INFO L"Fluorine Engine x86 Debug"
	#else
		#define FLUORINE_INFO L"Fluorine Engine x64 Debug"
	#endif
#elif FLU_RELEASE
	#if FLU_X32
		#define FLUORINE_INFO L"Fluorine Engine x86"
	#else
		#define FLUORINE_INFO L"Fluorine Engine x64"
	#endif
#endif

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/