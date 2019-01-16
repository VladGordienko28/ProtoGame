/*=============================================================================
	Core.h: Core main include file.
	Copyright Jan.2019 Vlad Gordienko.
=============================================================================*/
#pragma once

#include <string>

#include "Build.h"
#include "Types.h"
#include "Utils.h"
#include "Heap.h"
#include "LogCallback.h"
#include "LogManager.h"

// legacy include
#include "FrSerial.h"

#include "Array.h"


// remove this!
namespace flu 
{
namespace cstr
{
	inline Char* cat( Char* dest, SizeT size, const Char* other )
	{
		wcscat_s( dest, size, other );
		return dest;
	}

	inline WideChar* multiByteToWide( WideChar* buffer, SizeT bufferSize, const AnsiChar* source )
	{
		mbstowcs( buffer, source, bufferSize );
		return buffer;
	}
}
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/