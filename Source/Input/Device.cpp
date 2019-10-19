//-----------------------------------------------------------------------------
//	Device.cpp: A base input device implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Input.h"

namespace flu
{
namespace in
{
	Device::Device()
	{
		// reset all states
		mem::zero( &m_mouse, sizeof( MouseState ) );
		mem::zero( &m_keyboard, sizeof( KeyboardState ) );
		mem::zero( &m_gamepads[0], sizeof( GamepadState ) * MAX_GAMEPADS );
	}

	Device::~Device()
	{
		assert( m_clients.size() == 0 );
	}

	void Device::onMouseDown( EMouseButton button, Int32 x, Int32 y )
	{
		m_mouse.buttons[button] = true;
		m_mouse.posX = x;
		m_mouse.posY = y;

		for( auto& it : m_clients )
		{
			if( it->onMouseDown( button, x, y ) )
			{
				break;
			}
		}
	}

	void Device::onMouseUp( EMouseButton button, Int32 x, Int32 y )
	{
		m_mouse.buttons[button] = false;
		m_mouse.posX = x;
		m_mouse.posY = y;

		for( auto& it : m_clients )
		{
			if( it->onMouseUp( button, x, y ) )
			{
				break;
			}
		}
	}

	void Device::onMouseMove( EMouseButton button, Int32 x, Int32 y )
	{
		m_mouse.buttons[button] = true;
		m_mouse.posX = x;
		m_mouse.posY = y;

		for( auto& it : m_clients )
		{
			if( it->onMouseMove( button, x, y ) )
			{
				break;
			}
		}
	}

	void Device::onMouseDblClick( EMouseButton button, Int32 x, Int32 y )
	{
		m_mouse.buttons[button] = false;
		m_mouse.posX = x;
		m_mouse.posY = y;

		for( auto& it : m_clients )
		{
			if( it->onMouseDblClick( button, x, y ) )
			{
				break;
			}
		}
	}

	void Device::onMouseScroll( Int32 delta )
	{
		m_mouse.scroll += delta;

		for( auto& it : m_clients )
		{
			if( it->onMouseScroll( delta ) )
			{
				break;
			}
		}
	}

	void Device::onGamepadDown( GamepadId id, EGamepadButton button )
	{
		assert( id >= 0 && id < MAX_GAMEPADS );
		m_gamepads[id].buttons[button] = true;

		for( auto& it : m_clients )
		{
			if( it->onGamepadDown( id, button ) )
			{
				break;
			}
		}
	}

	void Device::onGamepadUp( GamepadId id, EGamepadButton button )
	{
		assert( id >= 0 && id < MAX_GAMEPADS );
		m_gamepads[id].buttons[button] = false;

		for( auto& it : m_clients )
		{
			if( it->onGamepadUp( id, button ) )
			{
				break;
			}
		}
	}

	void Device::onGamepadStick( GamepadId id, Int32 stick, const math::Vector& value )
	{
		assert( id >= 0 && id < MAX_GAMEPADS );
		assert( stick >= 0 && stick < GamepadState::NUM_STICKS );
		m_gamepads[id].sticks[stick] = value;

		for( auto& it : m_clients )
		{
			if( it->onGamepadStick( id, stick, value ) )
			{
				break;
			}
		}
	}

	void Device::onGamepadTrigger( GamepadId id, Int32 trigger, Float value )
	{
		assert( id >= 0 && id < MAX_GAMEPADS );
		assert( trigger >= 0 && trigger < GamepadState::NUM_TRIGGERS );
		m_gamepads[id].triggers[trigger] = value;

		for( auto& it : m_clients )
		{
			if( it->onGamepadTrigger( id, trigger, value ) )
			{
				break;
			}
		}
	}

	void Device::onKeyboardDown( EKeyboardButton button )
	{
		Bool wasDown = m_keyboard.buttons[button];
		m_keyboard.buttons[button] = true;

		for( auto& it : m_clients )
		{
			if( it->onKeyboardDown( button, wasDown ) )
			{
				break;
			}
		}
	}

	void Device::onKeyboardUp( EKeyboardButton button )
	{
		m_keyboard.buttons[button] = false;

		for( auto& it : m_clients )
		{
			if( it->onKeyboardUp( button ) )
			{
				break;
			}
		}
	}

	void Device::onKeyboardType( Char ch )
	{
		for( auto& it : m_clients )
		{
			if( it->onKeyboardType( ch ) )
			{
				break;
			}
		}
	}

	Bool Device::isKeyboardPressed( EKeyboardButton button ) const
	{
		return m_keyboard.buttons[button];
	}
		
	Bool Device::isMousePressed( EMouseButton button ) const
	{
		return m_mouse.buttons[button];
	}

	void Device::getMousePosition( Int32& outX, Int32& outY ) const
	{
		outX = m_mouse.posX;
		outY = m_mouse.posY;
	}

	Bool Device::isGamepadPressed( GamepadId id, EGamepadButton button ) const
	{
		assert( id >= 0 && id < MAX_GAMEPADS );
		return m_gamepads[id].buttons[button];
	}

	math::Vector Device::getGamepadStick( GamepadId id, Int32 stick ) const
	{
		assert( id >= 0 && id < MAX_GAMEPADS );
		assert( stick >= 0 && stick < GamepadState::NUM_STICKS );
		return m_gamepads[id].sticks[stick];
	}

	Float Device::getGamepadTrigger( GamepadId id, Int32 trigger ) const
	{
		assert( id >= 0 && id < MAX_GAMEPADS );
		assert( trigger >= 0 && trigger < GamepadState::NUM_TRIGGERS );
		return m_gamepads[id].triggers[trigger];
	}

	void Device::setGamepadVibration( GamepadId id, Float leftSpeed, Float rightSpeed )
	{
	}

	void Device::addClient( InputClient* client )
	{
		assert( m_clients.find( client ) == -1 );
		m_clients.push( client );
	}

	void Device::removeClient( InputClient* client )
	{
		assert( m_clients.find( client ) != -1 );
		m_clients.removeUnique( client, true );
	}
}
}