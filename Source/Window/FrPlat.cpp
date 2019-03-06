/*=============================================================================
	FrPlat.cpp: Window platfrom functions.
	Created by Vlad Gordienko, Feb. 2018.
=============================================================================*/

#include "Window.h"

/*-----------------------------------------------------------------------------
	CWinPlatform implementation.
-----------------------------------------------------------------------------*/

//
// Setup platform functions.
//
CWinPlatform::CWinPlatform()
{	
	// Timing.
	LARGE_INTEGER LInt;
	if( !QueryPerformanceFrequency(&LInt) )
		fatal( L"'QueryPerformanceFrequency' failed." );
	SecsPerCycle = 1.0 / (Double)LInt.QuadPart;
	NowTime = 0.0;
}


//
// Return current time.
//
Double CWinPlatform::TimeStamp()
{
	LARGE_INTEGER LInt;
	QueryPerformanceCounter( &LInt );
	return (Double)LInt.QuadPart * SecsPerCycle;
}


//
// Return time of the current frame.
// It's more optimized than TimeStamp().
//
Double CWinPlatform::Now()
{
	return NowTime;
}


//
// Set current now time.
//
void CWinPlatform::SetNow( Double InNow )
{
	NowTime = InNow;
}


//
// Return CPU cycles, used for benchmark.
//
UInt32 CWinPlatform::Cycles()
{
	LARGE_INTEGER Cyc;
	QueryPerformanceCounter(&Cyc);
	return Cyc.LowPart;
}


//
// Copy text to clipboard.
//
void CWinPlatform::ClipboardCopy( Char* Str ) 
{
	if( OpenClipboard(GetActiveWindow()) )
	{
		SizeT Size = (wcslen(Str)+1) * sizeof(Char);
		HGLOBAL hMem = GlobalAlloc( GMEM_MOVEABLE, Size );
		mem::copy( GlobalLock(hMem), Str, Size );
		GlobalUnlock( hMem );
		EmptyClipboard();
		SetClipboardData( CF_UNICODETEXT, hMem );
		CloseClipboard();
	}
}


//
// Paste text from the clipboard.
//
String CWinPlatform::ClipboardPaste()
{
	String Text;
	if( OpenClipboard(GetActiveWindow()) )
	{
		HGLOBAL hMem = GetClipboardData( CF_UNICODETEXT );

		if( hMem )
		{
			Char* Data = (Char*)GlobalLock(hMem);
			if( Data )
			{
				Text	= Data;
				GlobalUnlock(hMem);
			}
		}
		CloseClipboard();
	}
	return Text;
}


//
// Launch application or url in internet.
//
void CWinPlatform::Launch( const Char* Target, const Char* Parms )
{
	ShellExecute( nullptr, L"open", Target, Parms?Parms:L"", L"", SW_SHOWNORMAL );
}


//
// Return current time of the day.
//
envi::TimeOfDay CWinPlatform::GetTimeOfDay()
{
	SYSTEMTIME Time;
	GetLocalTime(&Time);
	return envi::TimeOfDay(Time.wHour, Time.wMinute, Time.wSecond);
};

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/