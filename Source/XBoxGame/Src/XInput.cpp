//-----------------------------------------------------------------------------
//	XInput.cpp: A XBox input device implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "pch.h"

using namespace Windows::Foundation;
using namespace Windows::Gaming::Input;

namespace flu
{
namespace xb
{
	XInputDevice::XInputDevice()
		:	in::Device(),
			m_gamepads(),
			m_criticalSection( concurrency::CriticalSection::create() )
	{
		Gamepad::GamepadAdded += ref new EventHandler<Gamepad^>( [this]( Platform::Object^ sender, Gamepad^ args )
		{
			onGamepadAdded( sender, args );
		} );

		Gamepad::GamepadRemoved += ref new EventHandler<Gamepad^>( [this]( Platform::Object^ sender, Gamepad^ args )
		{
			onGamepadRemoved( sender, args );
		} );		
		
		// add initial gamepads list
		for( UInt32 i = 0; i < Gamepad::Gamepads->Size; ++i )
		{
			onGamepadAdded( nullptr, Gamepad::Gamepads->GetAt(i) );
		}

		info( L"XInputDevice successfully created with %d gamepads", Gamepad::Gamepads->Size );
	}

	XInputDevice::~XInputDevice()
	{
		m_gamepads.empty();
		m_criticalSection = nullptr;
	}

	void XInputDevice::update( Float deltaTime )
	{
		concurrency::CriticalSection::Guard csg( m_criticalSection );

		for( Int32 id = 0; id < m_gamepads.size(); ++id )
		{
			Gamepad^ gamepad = m_gamepads[id];

			GamepadReading reading = gamepad->GetCurrentReading();

			// Handle buttons
			static GamepadButtons GAMEPAD_BUTTONS[in::GB_MAX] = 
			{
				GamepadButtons::DPadUp,
				GamepadButtons::DPadDown,
				GamepadButtons::DPadLeft,
				GamepadButtons::DPadRight,
				GamepadButtons::View, // todo: clarify button
				GamepadButtons::Menu, // todo: clarify button
				GamepadButtons::LeftThumbstick,
				GamepadButtons::RightThumbstick,
				GamepadButtons::LeftShoulder,
				GamepadButtons::RightShoulder,
				GamepadButtons::A,
				GamepadButtons::B,
				GamepadButtons::X,
				GamepadButtons::Y
			};

			for( UInt32 button = 0; button < arraySize( GAMEPAD_BUTTONS ); ++button )
			{
				Bool pressed = GAMEPAD_BUTTONS[button] == ( reading.Buttons & GAMEPAD_BUTTONS[button] );
				Bool wasPressed = isGamepadPressed( id, in::EGamepadButton(button) );

				if( pressed != wasPressed )
				{
					if( pressed )
					{
						onGamepadDown( id, in::EGamepadButton(button) );
					}
					else
					{
						onGamepadUp( id, in::EGamepadButton(button) );
					}
				}
			}

			// Handle sticks
			for( UInt32 stick = 0; stick < in::GamepadState::NUM_STICKS; ++stick )
			{
				math::Vector stickDir = { 0.f, 0.f };

				if( stick == 0 )
				{
					stickDir.x = reading.LeftThumbstickX;
					stickDir.y = reading.LeftThumbstickY;
				}
				else if( stick == 1 )
				{
					stickDir.x = reading.RightThumbstickX;
					stickDir.y = reading.RightThumbstickY;
				}

				const Float deadZoneRadiusSq = sqr( STICK_DEAD_ZONE_RADIUS );

				if( ( sqr( stickDir.x ) + sqr( stickDir.y ) ) < deadZoneRadiusSq )
				{
					// in the dead zone
					stickDir.x = 0.f;
					stickDir.y = 0.f;
				}

				if( stickDir != getGamepadStick( id, stick ) )
				{
					onGamepadStick( id, stick, stickDir );
				}
			}

			// Handle triggers
			for( UInt32 trigger = 0; trigger < in::GamepadState::NUM_TRIGGERS; ++trigger )
			{
				Float value = 0.f;

				if( trigger == 0 )
				{
					value = reading.LeftTrigger;
				}
				else if( trigger == 1 )
				{
					value = reading.RightTrigger;
				}

				if( value != getGamepadTrigger( id, trigger ) )
				{
					onGamepadTrigger( id, trigger, value );
				}
			}
		}
	}

	void XInputDevice::setGamepadVibration( in::GamepadId id, Float leftSpeed, Float rightSpeed )
	{
		assert( false && "Not implemented yet" );
	}

	void XInputDevice::onGamepadAdded( Platform::Object^ sender, Gamepad^ args )
	{
		concurrency::CriticalSection::Guard csg( m_criticalSection );

		m_gamepads.addUnique( args );
		info( L"New Gamepad %d added", m_gamepads.size() - 1 );
	}

	void XInputDevice::onGamepadRemoved( Platform::Object^ sender, Gamepad^ args )
	{
		concurrency::CriticalSection::Guard csg( m_criticalSection );

		m_gamepads.removeUnique( args, true );
		info( L"Gamepad removed" );
	}
}
}