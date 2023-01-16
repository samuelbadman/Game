#pragma once

#include "events/core/inputEvent.h"
#include "callback.h"

class xInputGamepad
{
public:
	static constexpr uint32_t maxGamepadsSupported = XUSER_MAX_COUNT;
	static callback<const inputEvent&> onInput;

public:
	static bool refresh(const uint32_t port);
	static bool setVibration(const uint32_t port, const uint16_t leftMotorSpeed, const uint16_t rightMotorSpeed);

private:
	static void applyCircularDeadzone(float& axisX, float& axisY, float deadzoneRadius);
	static void refreshButton(const XINPUT_STATE& state, const XINPUT_STATE& prevState, uint32_t port, int16_t button);
	static void refreshButtons(const XINPUT_STATE& state, const XINPUT_STATE& prevState, uint32_t port);
	static void refreshThumbsticks(const XINPUT_STATE& state, const XINPUT_STATE& prevState, uint32_t port);
	static void refreshTriggers(const XINPUT_STATE& state, const XINPUT_STATE& prevState, uint32_t port);
};