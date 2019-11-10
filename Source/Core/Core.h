/*=============================================================================
	Core.h: Core main include file.
	Copyright Jan.2019 Vlad Gordienko.
=============================================================================*/
#pragma once

#include "Build.h"

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
#include "FrSerial.h" // todo: get rid of this file

#include "SmartPointer.h"
#include "CString.h"
#include "Stream.h"
#include "Atomic.h"
#include "Concurrency.h"
#include "Threading.h"
#include "Stack.h"
#include "Array.h"
#include "GrowOnlyArray.h"
#include "Queue.h"
#include "Map.h"
#include "Profiler.h"
#include "StringManager.h"
#include "String.h"
#include "HandleArray.h"
#include "Text.h"
#include "Time.h"
#include "FileManager.h"
#include "TextWriter.h"
#include "Buffer.h"
#include "ConfigManager.h"

#include "Lexer/Token.h"
#include "Lexer/Lexer.h"

#include "JSon/JSon.h"

#include "JobSystem/JobSystem.h"

#include "Evaluator.h"

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/