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
	g_currentTest.startTimeStamp = time::cycles64();\
	info( L"Executing unit '%hs'...", g_currentTest.unitName );

#define leave_unit	\
	g_passedUnitsCount++;\
	info( L"Unit '%hs' passed with %d checks in %.4f ms", g_currentTest.unitName, \
		g_currentTest.checksCount, time::elapsedMsFrom( g_currentTest.startTimeStamp ) );

namespace flu
{
namespace tests
{
	struct UnitTestInfo
	{
		Int32 checksCount = 0;
		UInt64 startTimeStamp = 0;
		const AnsiChar* unitName = "";
	};

	typedef void (*TestFunction)( void );

	extern UnitTestInfo g_currentTest;
	extern UInt64 g_startTimeStamp;
	extern Int32 g_passedUnitsCount;
	extern Int32 g_failedUnitsCount;
	extern Int32 g_skippedUnitsCount;

	// all units, all units...
	extern void test_Array();
	//extern void test_Set();
	//extern void test_String();
	extern void test_File();
	extern void test_Map();

	static const TestFunction g_tests[] = 
	{
		test_Array,
		//test_Set,
		//test_String,
		test_File,
		test_Map
		//test_JSon,
		//test_Lexer,
		//test_HandleArray,
		//test_RingQueue
		//test_Text
	};
} // namespace tests
} // namespace flu