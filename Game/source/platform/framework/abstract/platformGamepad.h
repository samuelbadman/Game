#pragma once

namespace platformLayer
{
	namespace input
	{
		struct sInputEvent;
	}

	namespace gamepad
	{
		extern void pollGamepads();
		extern int8_t setGamepadVibration(const uint32_t port, const uint16_t leftMotorSpeed, const uint16_t rightMotorSpeed);
		extern void addOnInputEventDelegate(const std::function<void(platformLayer::input::sInputEvent&&)>& inDelegate);
	}
}