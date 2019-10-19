//-----------------------------------------------------------------------------
//	InputHandler.h: An abstract input handler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace in
{
	/**
	 *	An abstract input client
	 */
	class InputClient
	{
	public:
		InputClient();
		virtual ~InputClient();

		// mouse events
		virtual Bool onMouseDown( EMouseButton button, Int32 x, Int32 y );
		virtual Bool onMouseUp( EMouseButton button, Int32 x, Int32 y );
		virtual Bool onMouseMove( EMouseButton button, Int32 x, Int32 y );
		virtual Bool onMouseDblClick( EMouseButton button, Int32 x, Int32 y );
		virtual Bool onMouseScroll( Int32 delta );

		// gamepad events
		virtual Bool onGamepadDown( GamepadId id, EGamepadButton button );
		virtual Bool onGamepadUp( GamepadId id, EGamepadButton button );
		virtual Bool onGamepadStick( GamepadId id, Int32 stick, const math::Vector& value );
		virtual Bool onGamepadTrigger( GamepadId id, Int32 trigger, Float value );

		// keyboard events
		virtual Bool onKeyboardDown( EKeyboardButton button, Bool repeat );
		virtual Bool onKeyboardUp( EKeyboardButton button );
		virtual Bool onKeyboardType( Char ch );

		// accessors
		Bool isKeyboardPressed( EKeyboardButton button ) const;
		
		Bool isMousePressed( EMouseButton button ) const;
		void getMousePosition( Int32& outX, Int32& outY ) const;

		Bool isGamepadPressed( GamepadId id, EGamepadButton button ) const;
		math::Vector getGamepadStick( GamepadId id, Int32 stick ) const;
		Float getGamepadTrigger( GamepadId id, Int32 trigger ) const;

		// force feedback
		void setGamepadVibration( GamepadId id, Float leftSpeed, Float rightSpeed );

	private:
		class Device* m_device;

		friend class Device;
	};
}
}