//-----------------------------------------------------------------------------
//	Device.h: A base input device
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace in
{
	/**
	 *	An abstract input device
	 */
	class Device: public NonCopyable
	{
	public:
		static const SizeT MAX_GAMEPADS = 4;

		Device();
		~Device();

		// mouse events from application level
		void onMouseDown( EMouseButton button, Int32 x, Int32 y );
		void onMouseUp( EMouseButton button, Int32 x, Int32 y );
		void onMouseMove( EMouseButton button, Int32 x, Int32 y );
		void onMouseDblClick( EMouseButton button, Int32 x, Int32 y );
		void onMouseScroll( Int32 delta );

		// gamepad events from application level
		void onGamepadDown( GamepadId id, EGamepadButton button );
		void onGamepadUp( GamepadId id, EGamepadButton button );
		void onGamepadStick( GamepadId id, Int32 stick, const math::Vector& value );
		void onGamepadTrigger( GamepadId id, Int32 trigger, Float value );

		// keyboard events from application level
		void onKeyboardDown( EKeyboardButton button );
		void onKeyboardUp( EKeyboardButton button );
		void onKeyboardType( Char ch );

		// accessors
		Bool isKeyboardPressed( EKeyboardButton button ) const;
		
		Bool isMousePressed( EMouseButton button ) const;
		void getMousePosition( Int32& outX, Int32& outY ) const;

		Bool isGamepadPressed( GamepadId id, EGamepadButton button ) const;
		math::Vector getGamepadStick( GamepadId id, Int32 stick ) const;
		Float getGamepadTrigger( GamepadId id, Int32 trigger ) const;

		// force feedback
		virtual void setGamepadVibration( GamepadId id, Float leftSpeed, Float rightSpeed );

		// clients managment
		void addClient( InputClient* client );
		void removeClient( InputClient* client );

	private:
		// only one mouse supported
		MouseState m_mouse;

		// only one keyboard supported
		KeyboardState m_keyboard;

		// supported up to 4 gamepads
		GamepadState m_gamepads[MAX_GAMEPADS];

		Array<InputClient*> m_clients;
	};
}
}