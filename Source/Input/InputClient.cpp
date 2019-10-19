//-----------------------------------------------------------------------------
//	InputHandler.cpp: An abstract input handler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Input.h"

namespace flu
{
namespace in
{
	InputClient::InputClient()
		:	m_device( nullptr )
	{
	}

	InputClient::~InputClient()
	{
		assert( m_device == nullptr );
	}

	Bool InputClient::onMouseDown( EMouseButton button, Int32 x, Int32 y )
	{
		return false;
	}

	Bool InputClient::onMouseUp( EMouseButton button, Int32 x, Int32 y )
	{
		return false;
	}

	Bool InputClient::onMouseMove( EMouseButton button, Int32 x, Int32 y )
	{
		return false;
	}

	Bool InputClient::onMouseDblClick( EMouseButton button, Int32 x, Int32 y )
	{
		return false;
	}

	Bool InputClient::onMouseScroll( Int32 delta )
	{
		return false;
	}

	Bool InputClient::onGamepadDown( GamepadId id, EGamepadButton button )
	{
		return false;
	}

	Bool InputClient::onGamepadUp( GamepadId id, EGamepadButton button )
	{
		return false;
	}

	Bool InputClient::onGamepadStick( GamepadId id, Int32 stick, const math::Vector& value )
	{
		return false;
	}

	Bool InputClient::onGamepadTrigger( GamepadId id, Int32 trigger, Float value )
	{
		return false;
	}

	Bool InputClient::onKeyboardDown( EKeyboardButton button, Bool repeat )
	{
		return false;
	}

	Bool InputClient::onKeyboardUp( EKeyboardButton button )
	{
		return false;
	}

	Bool InputClient::onKeyboardType( Char ch )
	{
		return false;
	}

	Bool InputClient::isKeyboardPressed( EKeyboardButton button ) const
	{
		return m_device->isKeyboardPressed( button );
	}
		
	Bool InputClient::isMousePressed( EMouseButton button ) const
	{
		return m_device->isMousePressed( button );
	}

	void InputClient::getMousePosition( Int32& outX, Int32& outY ) const
	{
		m_device->getMousePosition( outX, outY );
	}

	Bool InputClient::isGamepadPressed( GamepadId id, EGamepadButton button ) const
	{
		return m_device->isGamepadPressed( id, button );
	}

	math::Vector InputClient::getGamepadStick( GamepadId id, Int32 stick ) const
	{
		return m_device->getGamepadStick( id, stick );
	}

	Float InputClient::getGamepadTrigger( GamepadId id, Int32 trigger ) const
	{
		return m_device->getGamepadTrigger( id, trigger );
	}

	void InputClient::setGamepadVibration( GamepadId id, Float leftSpeed, Float rightSpeed )
	{
		m_device->setGamepadVibration( id, leftSpeed, rightSpeed );
	}
}
}