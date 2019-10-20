//-----------------------------------------------------------------------------
//	XInput.h: A XBox input device
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace xb
{
	/**
	 *	A XBox input device
	 */
	class XInputDevice: public in::Device
	{
	public:
		using UPtr = UniquePtr<XInputDevice>;

		XInputDevice();
		~XInputDevice();

		void update( Float deltaTime );

		void setGamepadVibration( in::GamepadId id, Float leftSpeed, Float rightSpeed ) override;

	private:
		static const constexpr Float STICK_DEAD_ZONE_RADIUS = 0.1f;

		Array<Windows::Gaming::Input::Gamepad^> m_gamepads;
		concurrency::CriticalSection::UPtr m_criticalSection;

		void onGamepadAdded( Platform::Object^ sender, Windows::Gaming::Input::Gamepad^ args );
		void onGamepadRemoved( Platform::Object^ sender, Windows::Gaming::Input::Gamepad^ args );
	};
}
}