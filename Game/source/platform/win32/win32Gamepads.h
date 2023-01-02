#pragma once

#include "events/core/inputEvent.h"
#include "callback.h"

#include <cstdint>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>

#pragma comment(lib, "xinput.lib")

class win32Gamepads
{
private:
	static XINPUT_STATE prevStates[XUSER_MAX_COUNT];

public:
	static constexpr float gamepadLeftStickDeadzoneRadius = 0.24f;
	static constexpr float gamepadRightStickDeadzoneRadius = 0.24f;
	static constexpr int16_t gamepadMaxStickMagnitude = 32767;
	static constexpr int16_t gamepadMaxTriggerMagnitude = 255;

	static callback<const inputEvent&> onInput;

public:
	static void refresh();

private:
	static void applyCircularDeadzone(float& axisX, float& axisY, 
		float deadzoneRadius);
	static void refreshButton(const XINPUT_STATE& state, 
		const XINPUT_STATE& prevState, uint32_t port, int16_t button);
	static void refreshButtons(const XINPUT_STATE& state,
		const XINPUT_STATE& prevState, uint32_t port);
	static void refreshThumbsticks(const XINPUT_STATE& state,
		const XINPUT_STATE& prevState, uint32_t port);
	static void refreshTriggers(const XINPUT_STATE& state, 
		const XINPUT_STATE& prevState, uint32_t port);
};