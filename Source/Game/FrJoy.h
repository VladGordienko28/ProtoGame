/*=============================================================================
    FrJoy.cpp: Joystick input processing.
    Copyright Dec.2016 Vlad Gordienko.
=============================================================================*/
#ifndef _INC_JOY_
#define _INC_JOY_

#pragma comment( lib, "winmm.lib" )

/*-----------------------------------------------------------------------------
    Joystick tick.
-----------------------------------------------------------------------------*/

// First joystick key.
#define KEY_JOYSTICK_FIRST	KEY_JoyUp


//
// Capture joystick input.
//
static void JoystickTick()
{
	Bool		Keys[16];
	static Bool	OldKeys[16] = {};

	// Read current info.
	JOYINFO		Info;
	if( joyGetPos( JOYSTICKID1, &Info ) == JOYERR_NOERROR )
	{
		// Axis status, threshold are 5000.
		Keys[KEY_JoyLeft-KEY_JOYSTICK_FIRST]	= Info.wXpos < 5000;
		Keys[KEY_JoyRight-KEY_JOYSTICK_FIRST]	= Info.wXpos > 60535;	
		Keys[KEY_JoyUp-KEY_JOYSTICK_FIRST]		= Info.wYpos < 5000;
		Keys[KEY_JoyDown-KEY_JOYSTICK_FIRST]	= Info.wYpos > 60535;

		// Buttons.
		Keys[KEY_JoySelect-KEY_JOYSTICK_FIRST]	= Info.wButtons & JOY_BUTTON1;
		Keys[KEY_JoyStart-KEY_JOYSTICK_FIRST]	= Info.wButtons & JOY_BUTTON2;
		Keys[KEY_JoyA-KEY_JOYSTICK_FIRST]		= Info.wButtons & JOY_BUTTON3;
		Keys[KEY_JoyB-KEY_JOYSTICK_FIRST]		= Info.wButtons & JOY_BUTTON4;
		Keys[KEY_JoyC-KEY_JOYSTICK_FIRST]		= Info.wButtons & JOY_BUTTON5;
		Keys[KEY_JoyX-KEY_JOYSTICK_FIRST]		= Info.wButtons & JOY_BUTTON6;
		Keys[KEY_JoyY-KEY_JOYSTICK_FIRST]		= Info.wButtons & JOY_BUTTON7;
		Keys[KEY_JoyZ-KEY_JOYSTICK_FIRST]		= Info.wButtons & JOY_BUTTON8;

		// Notify input system.
		for( Int32 i=0; i<16; i++ )
		{
			if( Keys[i] && !OldKeys[i] )
				GApp->GInput->OnKeyDown(KEY_JOYSTICK_FIRST+i);

			if( !Keys[i] && OldKeys[i] )
				GApp->GInput->OnKeyUp(KEY_JOYSTICK_FIRST+i);
		}

		// Store it.
		mem::copy( OldKeys, Keys, sizeof(Keys) );
	}
}


#endif
/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/