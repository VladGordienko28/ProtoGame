//-----------------------------------------------------------------------------
//	Keys.h: All supported keys for all controllers
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace in
{
	/**
	 *	A mouse buttons
	 */
	enum EMouseButton
	{
		MB_None,
		MB_Left,
		MB_Right,
		MB_Middle,
		MB_MAX
	};

	/**
	 *	A keyboard buttons, inspired by WinApi table
	 */
	enum EKeyboardButton
	{
		KB_None,
		KB_0x01,
		KB_0x02,
		KB_0x03,
		KB_0x04,		
		KB_0x05,
		KB_0x06,
		KB_0x07,
		KB_Backspace,
		KB_Tab,
		KB_0x0a,
		KB_0x0b,
		KB_0x0c,
		KB_Return,
		KB_0x0e,
		KB_0x0f,
		KB_Shift,
		KB_Ctrl,
		KB_Alt,
		KB_Pause,
		KB_CapsLock,
		KB_0x15,
		KB_0x16,
		KB_0x17,
		KB_0x18,
		KB_0x19,
		KB_0x1a,
		KB_Escape,
		KB_0x1c,
		KB_0x1d,
		KB_0x1e,
		KB_0x1f,
		KB_Space,
		KB_PageUp,
		KB_PageDown,
		KB_End,
		KB_Home,
		KB_Left,
		KB_Up,
		KB_Right,
		KB_Down,
		KB_Select,
		KB_Print,
		KB_Execute,
		KB_PrintScrn,
		KB_Insert,
		KB_Delete,
		KB_Help,
		KB_0,
		KB_1,
		KB_2,
		KB_3,
		KB_4,
		KB_5,
		KB_6,
		KB_7,
		KB_8,
		KB_9,
		KB_0x3a,
		KB_0x3b,
		KB_0x3c,
		KB_0x3d,
		KB_0x3e,
		KB_0x3f,
		KB_0x40,
		KB_A,
		KB_B,
		KB_C,
		KB_D,
		KB_E,
		KB_F,
		KB_G,
		KB_H,
		KB_I,
		KB_J,
		KB_K,
		KB_L,
		KB_M,
		KB_N,
		KB_O,
		KB_P,
		KB_Q,
		KB_R,
		KB_S,
		KB_T,
		KB_U,
		KB_V,
		KB_W,
		KB_X,
		KB_Y,
		KB_Z,
		KB_0x5b,
		KB_0x5c,
		KB_0x5d,
		KB_0x5e,
		KB_0x5f,
		KB_NumPad0,
		KB_NumPad1,
		KB_NumPad2,
		KB_NumPad3,
		KB_NumPad4,
		KB_NumPad5,
		KB_NumPad6,
		KB_NumPad7,
		KB_NumPad8,
		KB_NumPad9,
		KB_Multiply,
		KB_Add,
		KB_Separator,
		KB_Subtract,
		KB_Decimal,
		KB_Divide,
		KB_F1,
		KB_F2,
		KB_F3,
		KB_F4,
		KB_F5,	
		KB_F6,
		KB_F7,
		KB_F8,
		KB_F9,
		KB_F10,
		KB_F11,
		KB_F12,
		KB_F13,
		KB_F14,
		KB_F15,
		KB_F16,
		KB_F17,
		KB_F18,
		KB_F19,
		KB_F20,
		KB_F21,
		KB_F22,
		KB_F23,
		KB_F24,
		KB_0x88,
		KB_0x89,
		KB_0x8a,
		KB_0x8b,
		KB_0x8c,
		KB_0x8d,
		KB_0x8e,
		KB_0x8f,
		KB_NumLock,
		KB_ScrollLock,
		KB_0x92,
		KB_0x93,
		KB_0x94,
		KB_0x95,
		KB_0x96,
		KB_0x97,
		KB_0x98,
		KB_0x99,
		KB_0x9a,
		KB_0x9b,
		KB_0x9c,
		KB_0x9d,
		KB_0x9e,
		KB_0x9f,
		KB_LShift,
		KB_RShift,
		KB_LControl,
		KB_RControl,
		KB_0xa4,
		KB_0xa5,
		KB_0xa6,
		KB_0xa7,
		KB_0xa8,
		KB_0xa9,
		KB_0xaa,
		KB_0xab,
		KB_0xac,
		KB_0xad,
		KB_0xae,
		KB_0xaf,
		KB_0xb0,
		KB_0xb1,
		KB_0xb2,
		KB_0xb3,
		KB_0xb4,
		KB_0xb5,
		KB_0xb6,
		KB_0xb7,
		KB_0xb8,
		KB_0xb9,	
		KB_Semicolon,
		KB_Equals,
		KB_Comma,
		KB_Minus,
		KB_Period,
		KB_Slash,
		KB_Tilde,
		KB_0xc1,
		KB_0xc2,
		KB_0xc3,
		KB_0xc4,
		KB_0xc5,
		KB_0xc6,
		KB_0xc7,
		KB_0xc8,
		KB_0xc9,
		KB_0xca,
		KB_0xcb,
		KB_0xcc,
		KB_0xcd,
		KB_0xce,
		KB_0xcf,
		KB_0xd0,
		KB_0xd1,
		KB_0xd2,
		KB_0xd3,
		KB_0xd4,
		KB_0xd5,
		KB_0xd6,
		KB_0xd7,
		KB_0xd8,
		KB_0xd9,
		KB_0xda,
		KB_LeftBracket,
		KB_Backslash,
		KB_RightBracket,
		KB_SingleQuote,
		KB_0xdf,
		KB_0xe0,
		KB_0xe1,
		KB_0xe2,
		KB_0xe3,
		KB_0xe4,
		KB_0xe5,
		KB_0xe6,
		KB_0xe7,
		KB_0xe8,
		KB_0xe9,
		KB_0xea,
		KB_0xeb,
		KB_0xec,
		KB_0xed,
		KB_0xee,
		KB_0xef,
		KB_0xf0,
		KB_0xf1,
		KB_0xf2,
		KB_0xf3,
		KB_0xf4,
		KB_0xf5,	
		KB_Attn,
		KB_CrSel,
		KB_ExSel,
		KB_ErEof,
		KB_Play,
		KB_Zoom,
		KB_NoName,
		KB_PA1,
		KB_OEMClear,
		KB_MAX
	};

	/**
	 *	A gamepad buttons, inspired by XBox One controller
	 */
	enum EGamepadButton
	{
		GB_Up,
		GB_Down,
		GB_Left,
		GB_Right,
		GB_Start,
		GB_Back,
		GB_LeftThumb,
		GB_RightThumb,
		GB_LeftShoulder,
		GB_RightShoulder,
		GB_A,
		GB_B,
		GB_X,
		GB_Y,
		GB_MAX
	};
}
}