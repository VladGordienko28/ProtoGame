/*=============================================================================
    FrInput.h: Level input subsystem.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Input keys definition.
-----------------------------------------------------------------------------*/

//
// Here's list of all keys values. The order same as in WinApi, but you can
// easily change it if required. There is some available slots, and i'll use
// it in my puprposes, to handle combos and so on.
//
enum
{
	KEY_None,			KEY_LButton,			KEY_RButton,			KEY_DblClick,
	KEY_MButton,		KEY_WheelUp,			KEY_WheelDown,			KEY_AV0x07,
	KEY_Backspace,		KEY_Tab,				KEY_AV0x0a,				KEY_AV0x0b,
	KEY_AV0x0c,			KEY_Return,				KEY_AV0x0e,				KEY_AV0x0f,
	KEY_Shift,			KEY_Ctrl,				KEY_Alt,				KEY_Pause,
	KEY_CapsLock,		KEY_AV0x15,				KEY_AV0x16,				KEY_AV0x17,
	KEY_AV0x18,			KEY_AV0x19,				KEY_AV0x1a,				KEY_Escape,
	KEY_AV0x1c,			KEY_AV0x1d,				KEY_AV0x1e,				KEY_AV0x1f,
	KEY_Space,			KEY_PageUp,				KEY_PageDown,			KEY_End,
	KEY_Home,			KEY_Left,				KEY_Up,					KEY_Right,
	KEY_Down,			KEY_Select,				KEY_Print,				KEY_Execute,
	KEY_PrintScrn,		KEY_Insert,				KEY_Delete,				KEY_Help,
	KEY_0,				KEY_1,					KEY_2,					KEY_3,
	KEY_4,				KEY_5,					KEY_6,					KEY_7,
	KEY_8,				KEY_9,					KEY_AV0x3a,				KEY_AV0x3b,
	KEY_AV0x3c,			KEY_AV0x3d,				KEY_AV0x3e,				KEY_AV0x3f,
	KEY_AV0x40,			KEY_A,					KEY_B,					KEY_C,
	KEY_D,				KEY_E,					KEY_F,					KEY_G,
	KEY_H,				KEY_I,					KEY_J,					KEY_K,
	KEY_L,				KEY_M,					KEY_N,					KEY_O,
	KEY_P,				KEY_Q,					KEY_R,					KEY_S,
	KEY_T,				KEY_U,					KEY_V,					KEY_W,
	KEY_X,				KEY_Y,					KEY_Z,					KEY_AV0x5b,
	KEY_AV0x5c,			KEY_AV0x5d,				KEY_AV0x5e,				KEY_AV0x5f,
	KEY_NumPad0,		KEY_NumPad1,			KEY_NumPad2,			KEY_NumPad3,
	KEY_NumPad4,		KEY_NumPad5,			KEY_NumPad6,			KEY_NumPad7,
	KEY_NumPad8,		KEY_NumPad9,			KEY_Multiply,			KEY_Add,
	KEY_Separator,		KEY_Subtract,			KEY_Decimal,			KEY_Divide,
	KEY_F1,				KEY_F2,					KEY_F3,					KEY_F4,
	KEY_F5,				KEY_F6,					KEY_F7,					KEY_F8,
	KEY_F9,				KEY_F10,				KEY_F11,				KEY_F12,
	KEY_F13,			KEY_F14,				KEY_F15,				KEY_F16,
	KEY_F17,			KEY_F18,				KEY_F19,				KEY_F20,
	KEY_F21,			KEY_F22,				KEY_F23,				KEY_F24,
	KEY_AV0x88,			KEY_AV0x89,				KEY_AV0x8a,				KEY_AV0x8b,
	KEY_AV0x8c,			KEY_AV0x8d,				KEY_AV0x8e,				KEY_AV0x8f,
	KEY_NumLock,		KEY_ScrollLock,			KEY_AV0x92,				KEY_AV0x93,
	KEY_AV0x94,			KEY_AV0x95,				KEY_AV0x96,				KEY_AV0x97,
	KEY_AV0x98,			KEY_AV0x99,				KEY_AV0x9a,				KEY_AV0x9b,
	KEY_AV0x9c,			KEY_AV0x9d,				KEY_AV0x9e,				KEY_AV0x9,
	KEY_LShift,			KEY_RShift,				KEY_LControl,			KEY_RControl,
	KEY_JoyUp,			KEY_JoyDown,			KEY_JoyLeft,			KEY_JoyRight,
	KEY_JoySelect,		KEY_JoyStart,			KEY_JoyA,				KEY_JoyB,
	KEY_JoyC,			KEY_JoyX,				KEY_JoyY,				KEY_JoyZ,
	KEY_AV0xb0,			KEY_AV0xb1,				KEY_AV0xb2,				KEY_AV0xb3,
	KEY_AV0xb4,			KEY_AV0xb5,				KEY_AV0xb6,				KEY_AV0xb7,
	KEY_AV0xb8,			KEY_AV0xb9,				KEY_Semicolon,			KEY_Equals,
	KEY_Comma,			KEY_Minus,				KEY_Period,				KEY_Slash,
	KEY_Tilde,			KEY_AV0xc1,				KEY_AV0xc2,				KEY_AV0xc3,
	KEY_AV0xc4,			KEY_AV0xc5,				KEY_AV0xc6,				KEY_AV0xc7,
	KEY_AV0xc8,			KEY_AV0xc9,				KEY_AV0xca,				KEY_AV0xcb,
	KEY_AV0xcc,			KEY_AV0xcd,				KEY_AV0xce,				KEY_AV0xcf,
	KEY_AV0xd0,			KEY_AV0xd1,				KEY_AV0xd2,				KEY_AV0xd3,
	KEY_AV0xd4,			KEY_AV0xd5,				KEY_AV0xd6,				KEY_AV0xd7,
	KEY_AV0xd8,			KEY_AV0xd9,				KEY_AV0xda,				KEY_LeftBracket,
	KEY_Backslash,		KEY_RightBracket,		KEY_SingleQuote,		KEY_AV0xdf,
	KEY_AV0xe0,			KEY_AV0xe1,				KEY_AV0xe2,				KEY_AV0xe3,
	KEY_AV0xe4,			KEY_AV0xe5,				KEY_AV0xe6,				KEY_AV0xe7,
	KEY_AV0xe8,			KEY_AV0xe9,				KEY_AV0xea,				KEY_AV0xeb,
	KEY_AV0xec,			KEY_AV0xed,				KEY_AV0xee,				KEY_AV0xef,
	KEY_AV0xf0,			KEY_AV0xf1,				KEY_AV0xf2,				KEY_AV0xf3,
	KEY_AV0xf4,			KEY_AV0xf5,				KEY_Attn,				KEY_CrSel,
	KEY_ExSel,			KEY_ErEof,				KEY_Play,				KEY_Zoom,
	KEY_NoName,			KEY_PA1,				KEY_OEMClear,			KEY_MAX
};


/*-----------------------------------------------------------------------------
    CInput.
-----------------------------------------------------------------------------*/

//
// An input subsystem.
//
class CInput: public CRefsHolder
{
public:
	// Keys variables.
	Bool		Keys[KEY_MAX];
	UInt8		SystemRemap[KEY_MAX];
	UInt8		ConfigRemap[KEY_MAX];

	// Cursor, wheel and/or touch variables.
	Int32			MouseX;
	Int32			MouseY;
	Int32			WheelScroll;
	math::Vector	WorldCursor;

	// Current level.
	FLevel*		Level;

	// CInput interface.
	CInput();
	void RemapFromIni( CConfigManager* Config );
	void SetLevel( FLevel* InLevel );
	void Reset();
	Bool KeyIsPressed( Int32 iKey );

	// Events from the platform.
	void OnKeyDown( Int32 iKey );
	void OnKeyUp( Int32 iKey );
	void OnCharType( Char TypedChar );

	// Cursor variables, should be updated by platform
	// directly, without any restrictions.

	// CRefsHolder interface.
	void CountRefs( CSerializer& S );

	// Utils.
	Bool MatchKeyCombo( String TestCombo ) const;

private:
	// Combo-handles.
	enum{ MAX_COMBO_LENGTH = 8 };
	Char	KeysHistory[MAX_COMBO_LENGTH];
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/