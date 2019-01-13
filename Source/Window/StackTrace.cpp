//-----------------------------------------------------------------------------
//	StackTrace.cpp: A stack tracking helper
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Window.h"

#pragma pack( push, 8 )
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")
#pragma pack( pop ) 

namespace flu
{
namespace win
{
#pragma optimize ( "", off )

	Char* stackTrace( LPEXCEPTION_POINTERS exception )
	{
		static Char history[MAX_STACK_TRACE_LENGTH];
		mem::zero( history, sizeof( history ) );

		DWORD64 offset64 = 0;
		DWORD offset = 0;

		// prepare
		HANDLE currentProcess = GetCurrentProcess();
		HANDLE currentThread = GetCurrentThread();

		// symbol info
		static const SizeT MAX_SYMBOL_LENGTH = 1024;

		UInt8 symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYMBOL_LENGTH] = {};

		SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>( symbolBuffer );
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = MAX_SYMBOL_LENGTH;
		DWORD symOptions = SymGetOptions();
		symOptions |= SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_EXACT_SYMBOLS;
		SymSetOptions( symOptions );
		SymInitialize( currentProcess, ".", 1 );

		// line info
		IMAGEHLP_LINE64 line;
		mem::zero( &line, sizeof(IMAGEHLP_LINE64) );
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

		// setup frame info
		STACKFRAME64 stackFrame;
		mem::zero( &stackFrame, sizeof(STACKFRAME64) );

#if FLU_X32
		if( exception )
		{
			stackFrame.AddrStack.Offset = exception->ContextRecord->Esp;
			stackFrame.AddrFrame.Offset = exception->ContextRecord->Ebp;
			stackFrame.AddrPC.Offset = exception->ContextRecord->Eip;
		}
		else
		{
			__asm
			{ 
			Label: 
				mov dword ptr [stackFrame.AddrStack.Offset], esp
				mov dword ptr [stackFrame.AddrFrame.Offset], ebp
				mov eax, [Label]
				mov dword ptr [stackFrame.AddrPC.Offset], eax
			}
		}
#endif

		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrStack.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Mode = AddrModeFlat;
		stackFrame.AddrBStore.Mode = AddrModeFlat;
		stackFrame.AddrReturn.Mode = AddrModeFlat;

		// walk through the stack
		for( ; ; )
		{
			if( !StackWalk64( IMAGE_FILE_MACHINE_I386, currentProcess, currentThread, 
					&stackFrame, exception ? exception->ContextRecord : nullptr,
					nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr ) )
			{
				break;
			}

			if( SymFromAddr( currentProcess, stackFrame.AddrPC.Offset, &offset64, symbol ) && 
				SymGetLineFromAddr64( currentProcess, stackFrame.AddrPC.Offset, &offset, &line ) )
			{
				Char fileName[MAX_SYMBOL_LENGTH];
				Char funcName[MAX_SYMBOL_LENGTH];

				cstr::multiByteToWide( fileName, MAX_SYMBOL_LENGTH, line.FileName );
				cstr::multiByteToWide( funcName, MAX_SYMBOL_LENGTH, symbol->Name );

				cstr::cat( history, MAX_STACK_TRACE_LENGTH, funcName );
				cstr::cat( history, MAX_STACK_TRACE_LENGTH, L" <- " );
			}
		}

		return history;
	}
#pragma optimize ( "", on )

}
}