/*=============================================================================
    FrConsole.cpp: Game console implementation.
    Copyright Dec.2016 Vlad Gordienko.
=============================================================================*/

#include "Game.h"

/*-----------------------------------------------------------------------------
	CConsole implementation.
-----------------------------------------------------------------------------*/

//
// Console font.
//
TStaticFont*	CConsole::Font	= nullptr;


//
// Console constructor.
//
CConsole::CConsole()
	:	bActive( false ),
		HistTop( 0 ) 
{
}


//
// Console destruction.
//
CConsole::~CConsole()
{
}


//
// Count references.
//
void CConsole::CountRefs( CSerializer& S )
{
}


//
// On <Enter> click.
//
void CConsole::Accept()
{
	// Add to history.
	AddToHistory( String(L"Console: ") + Command, ETextColor::TCR_Cyan );

	// Execute.
	if( Command )
		GGame->ConsoleExecute( Command );
	Command	= L"";
}


//
// Process the C++ log mechanism.
// Just add a 'log' message directly to
// the console.
//
void CConsole::LogCallback( String Msg, ETextColor Color )
{
	AddToHistory( Msg, Color );
}


//
// Process a char type.
//
void CConsole::CharType( Char C )
{
	assert(IsActive());

	if( C == CON_EXEC_BUTTON )
	{
		// <Enter> click - execute cmd.
		Accept();
		return;
	}
	else if( C == 8 )
	{
		// <Backspace> - erase last char.
		if( Command )
			Command	= String::Delete( Command, Command.Len()-1, 1 );
		return;
	}

	// Avoid really crazy characters such as
	// <return> or <esc>... Process only typeable
	// letters, digits, symbols.
	static const Char Symbols[] = L"~!@#$ %^&*()_+{}[]/-;.,:=";
	if	(
			!IsLetter(C) &&
			!IsDigit(C) &&
			!wcschr( Symbols, C )
		)
		return;

	// Concat string and new character.
	Char New[2] = { C, '\0' };
	Command	+= New;
}


//
// Render the console.
//
void CConsole::Render( CCanvas* Canvas )
{
	if( !IsActive() )
		return;

	// Set window coords.
	Canvas->PushTransform
	(
		TViewInfo
		(
			0.f, 0.f,
			Canvas->ScreenWidth, Canvas->ScreenHeight
		)
	);
	{
		// Backdrop.
		TRenderRect R;
		R.Texture		= nullptr;
		R.Bounds.Min	= TVector( 0.f, 0.f );
		R.Bounds.Max	= TVector( Canvas->ScreenWidth, 16.f*(MAX_CON_HISTORY+1)+16 );
		R.Color			= COLOR_Black;
		R.Flags			= POLY_FlatShade;
		R.Rotation		= 0;
		Canvas->DrawRect( R );

		// Console colors.
		static const TColor ConsoleClrs[TCR_MAX] =
		{
			TColor( 0xff, 0xff, 0xff, 0xff ),	// TCR_White.
			TColor( 0xc0, 0xc0, 0xc0, 0xff ),	// TCR_Gray.
			TColor( 0x80, 0x00, 0x00, 0xff ),	// TCR_Red.
			TColor( 0x00, 0x80, 0x00, 0xff ),	// TCR_Green.
			TColor( 0x00, 0x00, 0x80, 0xff ),	// TCR_Blue.
			TColor( 0x80, 0x00, 0x80, 0xff ),	// TCR_Magenta.
			TColor( 0x00, 0x80, 0x80, 0xff ),	// TCR_Cyan.
			TColor( 0x80, 0x80, 0x00, 0xff )	// TCR_Yellow.
		};

		// History.
		for( Int32 i=0; i<HistTop; i++ )
			Canvas->DrawText
			(
				History[i].Text,
				CConsole::Font,
				ConsoleClrs[History[i].Color],
				TVector( 8.f, 8.f+16.f*i)
			);

		// Typing command.
		Canvas->DrawText
		(
			String(L"<> ") + Command + String(L"_"),
			CConsole::Font,
			TColor( 0xc0, 0xc0, 0xc0, 0xc0 ),
			TVector( 8.f, 8.f+16.f*MAX_CON_HISTORY)
		);
	}
	Canvas->PopTransform();
}


//
// Add a new record to the console history.
//
void CConsole::AddToHistory( String S, ETextColor Color )
{
	if( HistTop < MAX_CON_HISTORY )
	{
		// There is available history slot.
		History[HistTop].Text	= S;
		History[HistTop].Color	= Color;
		HistTop++;
	}
	else
	{
		// No available slot, pop last one.
		for( Int32 i=1; i<MAX_CON_HISTORY; i++ )
			History[i-1]	=	History[i];

		History[HistTop-1].Text		= S;
		History[HistTop-1].Color	= Color;
	}
}


//
// Toggle the console activity - show or
// hide it.
//
Bool CConsole::ShowToggle()
{
	bActive	=	!bActive;
	return bActive;
}


//
// Clear console.
//
void CConsole::Clear()
{
	HistTop	= 0;
	for( Int32 i=0; i<MAX_CON_HISTORY; i++ )
		History[i].Text	= L"";
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/