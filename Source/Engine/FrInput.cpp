/*=============================================================================
    FrInput.cpp: Level input processing.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    CInput implementation.
-----------------------------------------------------------------------------*/

//
// Input subsystem constructor.
//
CInput::CInput()
{
	// Nothing pressed.
	mem::zero( Keys, sizeof(Keys) );

	// Default system remap. In case of
	// some weird platform, platform should
	// change this table itself.
	for( Int32 iKey=0; iKey<KEY_MAX; iKey++ )
		SystemRemap[iKey]	= iKey;

	// Default user's remap table. It should be
	// changed via config file.
	for( Int32 iKey=0; iKey<KEY_MAX; iKey++ )
		ConfigRemap[iKey]	= iKey;

	// Mouse variables.
	MouseX		= 0;
	MouseY		= 0;
	WheelScroll	= 0;
	WorldCursor	= math::Vector( 0.f, 0.f );

	// Nothing to process.
	Level	= nullptr;

	// Notify.
	info( L"In: Input subsystem initialized" );
}


//
// Reset subsystem.
//
void CInput::Reset()
{
	// Unpress everything.
	mem::zero( Keys, sizeof(Keys) );
	mem::zero( KeysHistory, sizeof(KeysHistory) );
	MouseX			= 0;
	MouseY			= 0;
	WheelScroll		= 0;
	WorldCursor		= math::Vector( 0.f, 0.f );

	// Set no level.
	Level	= nullptr;
}


//
// Initialize input system for a given level, all
// input events will be redirected to script system
// via FInputComponent.
//
void CInput::SetLevel( FLevel* InLevel )
{
	assert(InLevel);

	Reset();
	Level	= InLevel;
}


//
// Some key has been pressed.
//
void CInput::OnKeyDown( Int32 iKey )
{
	iKey	= KEY_MAX & iKey;

	// Notify inputs.
	if( Level )
	{
		Int32 Pressed = ConfigRemap[SystemRemap[iKey]];

		// Add to history only Letters.
		if( Pressed >= 'A' && Pressed <= 'Z' )
		{
			for( Int32 i=1; i<MAX_COMBO_LENGTH; i++ )
				KeysHistory[i-1] = KeysHistory[i];
			KeysHistory[MAX_COMBO_LENGTH-1] = Pressed;
		}

		for( FInputComponent* I = Level->FirstInput; I; I = I->NextInput )
		{
			// Event OnKeyDown triggered only once, without key-repeating signals.
			if( !Keys[iKey] )
				I->Entity->OnKeyDown(Pressed);

			// Event OnKeyPressed triggered on key-repeating.
			I->Entity->OnKeyPressed(Pressed);
		}
	}

	// Store it.
	Keys[iKey]	= true;
}


//
// Some key has been unpressed.
//
void CInput::OnKeyUp( Int32 iKey )
{
	iKey	= KEY_MAX & iKey;

	// Notify inputs.
	if( Level )
	{
		Int32 Pressed = ConfigRemap[SystemRemap[iKey]];
		for( FInputComponent* I = Level->FirstInput; I; I = I->NextInput )
			I->Entity->OnKeyUp(Pressed);
	}

	// Store it.
	Keys[iKey]	= false;
}


//
// When user type some character, bring it to
// script.
//
void CInput::OnCharType( Char TypedChar )
{
	if( !Level )
		return;

	// Avoid really crazy characters such as
	// <return> or <esc>... Process only typeable
	// letters, digits, symbols.
	static const Char Symbols[] = L"~!@#$%^& *()_+{}[]/-;.,:";
	if	(
			!cstr::isLetter( TypedChar ) &&
			!cstr::isDigit( TypedChar ) &&
			!cstr::findChar( Symbols, TypedChar )
		)
		return;

	// Pass to script as string.
	Char	Ch[2]	= { TypedChar, 0 };
	String	S		= Ch;

	for( FInputComponent* I = Level->FirstInput; I; I = I->NextInput )
		I->Entity->OnCharType(S);
}


//
// Read ConfigRemap from the ini file to apply user
// configuration preferences.
//
void CInput::RemapFromConfig()
{
	// Get list of names.
	CEnum* InputKeys	= CClassDatabase::StaticFindEnum(L"EInputKeys");
	assert(InputKeys);
	assert(InputKeys->Elements.size()>=KEY_MAX);

	// Let's torment ini.
	for( Int32 i=0; i<KEY_MAX; i++ )
		ConfigRemap[i]	= ConfigManager::readInt( EConfigFile::User, L"Input", *InputKeys->GetAliasOf(i), i );

}


//
// Return true, if given key are pressed.
//
Bool CInput::KeyIsPressed( Int32 iKey )
{
	iKey	= KEY_MAX & iKey;
	return Keys[ConfigRemap[SystemRemap[iKey]]];
}


//
// Count references in system.
//
void CInput::CountRefs( CSerializer& S )
{
	FLevel* OldLevel = Level;
	Serialize( S, Level );

	// Reset system, if level gone far away.
	if( OldLevel && !Level )
		Reset();
}


//
// Return true, if cheat-code matched.
//
Bool CInput::MatchKeyCombo( String TestCombo ) const
{
	if( TestCombo.len() >= MAX_COMBO_LENGTH )
		return false;

	for( Int32 i=TestCombo.len()-1, j=MAX_COMBO_LENGTH-1; i>=0; i--, j-- )
		if( TestCombo[i] != KeysHistory[j] )
			return false;

	return true;
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/