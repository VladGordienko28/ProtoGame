/*=============================================================================
	Core.h: Core main include file.
	Copyright Jan.2019 Vlad Gordienko.
=============================================================================*/
#pragma once

#include "Build.h"

// platform specific include
#if FLU_PLATFORM_WINDOWS
#include <Windows.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef DrawText
#undef DrawText
#endif
#endif

// std includes
#include <string>
#include <stdarg.h>

// fluorine core includes
#include "Types.h"
#include "Utils.h"
#include "Heap.h"
#include "LogCallback.h"
#include "LogManager.h"
#include "Hash.h"

// legacy include
#include "FrSerial.h"

#include "SmartPointer.h"
#include "CString.h"
#include "Stream.h"
#include "Atomic.h"
#include "Concurrency.h"
#include "Threading.h"
#include "Stack.h"
#include "Array.h"
#include "Queue.h"
#include "Map.h"
#include "Profiler.h"
#include "StringManager.h"
#include "String.h"
#include "HandleArray.h"
#include "Namespace.h"
#include "Text.h"
#include "FileManager.h"
#include "TextWriter.h"
#include "Buffer.h"

#include "Lexer/Token.h"
#include "Lexer/Lexer.h"

#include "JSon/JSon.h"

#include "Evaluator.h"

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/