//-----------------------------------------------------------------------------
//	Controllers.h: Input controllers
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace in
{
	/**
	 *	A mouse state
	 */
	struct MouseState
	{
	public:
		Int32 posX;
		Int32 posY;
		Int32 scroll;

		Bool buttons[EMouseButton::MB_MAX];
	};

	/**
	 *	A keyboard state
	 */
	struct KeyboardState
	{
	public:
		Bool buttons[EKeyboardButton::KB_MAX];
	};

	/**
	 *	A gamepad id
	 */
	using GamepadId = UInt32;

	/**
	 *	A gamepad state
	 */
	struct GamepadState
	{
	public:
		static const GamepadId INVALID = -1;

		static const SizeT NUM_STICKS = 2;
		static const SizeT NUM_TRIGGERS = 2;

		math::Vector sticks[NUM_STICKS];
		Float triggers[NUM_TRIGGERS];

		Bool buttons[EGamepadButton::GB_MAX];
	};
}
}