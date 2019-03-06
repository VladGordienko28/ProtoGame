//-----------------------------------------------------------------------------
//	Tests.h: Unit Tests main include file
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------
#pragma once

// Fluorine includes
#include "Core/Core.h"
#include "Engine/Engine.h"
#include "Window/Window.h"

// Unit test validator
#define check( expr )	\
	if( !(expr) )\
	{\
		error( L"Unit '%hs' failed at %hs(%i) with check '%s'", g_currentTest.unitName, __FILE__, __LINE__, L#expr );\
		throw nullptr;\
	}\
	else\
	{\
		g_currentTest.checksCount++;\
	}

#define enter_unit( name )	\
	g_currentTest.checksCount = 0;\
	g_currentTest.unitName = #name;\
	g_currentTest.startTime = GPlat->TimeStamp();\
	info( L"Executing unit '%hs'...", g_currentTest.unitName );

#define leave_unit	\
	g_passedUnitsCount++;\
	info( L"Unit '%hs' passed with %d checks in %.4f ms", g_currentTest.unitName, \
		g_currentTest.checksCount, ( GPlat->TimeStamp() - g_currentTest.startTime ) * 1000.0 );

namespace flu
{
namespace tests
{
	struct UnitTestInfo
	{
		Int32 checksCount = 0;
		Double startTime = 0.0;
		const AnsiChar* unitName = "";
	};

	typedef void (*TestFunction)( void );

	extern UnitTestInfo g_currentTest;
	extern Double g_startTime;
	extern Int32 g_passedUnitsCount;
	extern Int32 g_failedUnitsCount;
	extern Int32 g_skippedUnitsCount;

	// all units, all units...
	extern void test_Array();
	//extern void test_Set();
	//extern void test_String();
	extern void test_File();

	static const TestFunction g_tests[] = 
	{
		test_Array,
		//test_Set,
		//test_String,
		test_File
	};
} // namespace tests
} // namespace flu