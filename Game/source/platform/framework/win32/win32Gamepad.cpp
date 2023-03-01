#include "pch.h"

#include "platform/framework/platformGamepad.h"
#include "platform/framework/platformKeyCodes.h"
#include "platform/events/inputEvent.h"
#include "game/game.h"

static XINPUT_STATE prevStates[XUSER_MAX_COUNT];

static constexpr float gamepadLeftStickDeadzoneRadius = 0.24f;
static constexpr float gamepadRightStickDeadzoneRadius = 0.24f;
static constexpr int16_t gamepadMaxStickMagnitude = 32767;
static constexpr int16_t gamepadMaxTriggerMagnitude = 255;

static void applyCircularDeadzone(float& axisX, float& axisY, float deadzoneRadius)
{
	float normX = std::max(-1.0f, axisX / static_cast<float>(gamepadMaxStickMagnitude));
	float normY = std::max(-1.0f, axisY / static_cast<float>(gamepadMaxStickMagnitude));

	float absNormX = std::abs(normX);
	float absNormY = std::abs(normY);
	axisX = (absNormX < deadzoneRadius ? 0.0f : (absNormX - deadzoneRadius) * (normX / absNormX));
	axisY = (absNormY < deadzoneRadius ? 0.0f : (absNormY - deadzoneRadius) * (normY / absNormY));

	if (deadzoneRadius > 0.0f)
	{
		axisX /= 1.0f - deadzoneRadius;
		axisY /= 1.0f - deadzoneRadius;
	}
}

static void pollButton(const XINPUT_STATE& state, const XINPUT_STATE& prevState, uint32_t port, int16_t button)
{
	// Get the current and previous states of the button
	auto currentButtonState = state.Gamepad.wButtons & static_cast<WORD>(button);
	auto prevButtonState = prevState.Gamepad.wButtons & static_cast<WORD>(button);

	// Return if the current state of the button is the same as its previous state
	if (currentButtonState == prevButtonState)
	{
		return;
	}

	// Check button state
	if (currentButtonState != 0)
	{
		// Button pressed
		sInputEvent evt = {};
		evt.repeatedKey = false;
		evt.input = button;
		evt.port = port;
		evt.data = 1.0f;

		game::onInputEvent(nullptr, evt);
	}
	else
	{
		// Button released
		sInputEvent evt = {};
		evt.repeatedKey = false;
		evt.input = button;
		evt.port = port;
		evt.data = 0.0f;

		game::onInputEvent(nullptr, evt);
	}
}

static void pollButtons(const XINPUT_STATE& state, const XINPUT_STATE& prevState, uint32_t port)
{
	pollButton(state, prevState, port, platformLayer::keyCodes::Gamepad_Face_Button_Bottom);
	pollButton(state, prevState, port, platformLayer::keyCodes::Gamepad_Face_Button_Top);
	pollButton(state, prevState, port, platformLayer::keyCodes::Gamepad_Face_Button_Left);
	pollButton(state, prevState, port, platformLayer::keyCodes::Gamepad_Face_Button_Right);
	pollButton(state, prevState, port, platformLayer::keyCodes::Gamepad_D_Pad_Down);
	pollButton(state, prevState, port, platformLayer::keyCodes::Gamepad_D_Pad_Up);
	pollButton(state, prevState, port, platformLayer::keyCodes::Gamepad_D_Pad_Left);
	pollButton(state, prevState, port, platformLayer::keyCodes::Gamepad_D_Pad_Right);
	pollButton(state, prevState, port, platformLayer::keyCodes::Gamepad_Special_Left);
	pollButton(state, prevState, port, platformLayer::keyCodes::Gamepad_Special_Right);
	pollButton(state, prevState, port, platformLayer::keyCodes::Gamepad_Left_Shoulder);
	pollButton(state, prevState, port, platformLayer::keyCodes::Gamepad_Right_Shoulder);
	pollButton(state, prevState, port, platformLayer::keyCodes::Gamepad_Left_Thumbstick_Button);
	pollButton(state, prevState, port, platformLayer::keyCodes::Gamepad_Right_Thumbstick_Button);
}

static void pollThumbsticks(const XINPUT_STATE& state, const XINPUT_STATE& prevState, uint32_t port)
{
	// Left
	// Axis
	// Calculate normalized axis value after a deadzone has been applied
	auto thumbLX{ static_cast<float>(state.Gamepad.sThumbLX) };
	auto thumbLY{ static_cast<float>(state.Gamepad.sThumbLY) };
	applyCircularDeadzone(
		thumbLX,
		thumbLY,
		gamepadLeftStickDeadzoneRadius
	);

	// Submit inputs for the stick
	sInputEvent leftXAxisEvent = {};
	leftXAxisEvent.repeatedKey = false;
	leftXAxisEvent.input = platformLayer::keyCodes::Gamepad_Left_Thumbstick_X_Axis;
	leftXAxisEvent.port = port;
	leftXAxisEvent.data = thumbLX;

	game::onInputEvent(nullptr, leftXAxisEvent);

	sInputEvent leftYAxisEvent = {};
	leftYAxisEvent.repeatedKey = false;
	leftYAxisEvent.input = platformLayer::keyCodes::Gamepad_Left_Thumbstick_Y_Axis;
	leftYAxisEvent.port = port;
	leftYAxisEvent.data = thumbLY;

	game::onInputEvent(nullptr, leftYAxisEvent);

	// Action
	// Get the current and previous states of the stick
	auto currLX = state.Gamepad.sThumbLX / gamepadMaxStickMagnitude;
	auto prevLX = prevState.Gamepad.sThumbLX / gamepadMaxStickMagnitude;

	auto currLY = state.Gamepad.sThumbLY / gamepadMaxStickMagnitude;
	auto prevLY = prevState.Gamepad.sThumbLY / gamepadMaxStickMagnitude;

	// Check if the state of the stick has changed
	if (currLX != prevLX)
	{
		// Check if the stick is pushed right
		if (currLX > 0)
		{
			// Submit input
			sInputEvent leftThumbstickRightEvent = {};
			leftThumbstickRightEvent.repeatedKey = false;
			leftThumbstickRightEvent.input = platformLayer::keyCodes::Gamepad_Left_Thumbstick_Right;
			leftThumbstickRightEvent.port = port;
			leftThumbstickRightEvent.data = 1.0f;

			game::onInputEvent(nullptr, leftThumbstickRightEvent);
		}
		// Check if the stick is pushed left
		else if (currLX < 0)
		{
			// Submit input
			sInputEvent leftThumbstickLeftEvent = {};
			leftThumbstickLeftEvent.repeatedKey = false;
			leftThumbstickLeftEvent.input = platformLayer::keyCodes::Gamepad_Left_Thumbstick_Left;
			leftThumbstickLeftEvent.port = port;
			leftThumbstickLeftEvent.data = 1.0f;

			game::onInputEvent(nullptr, leftThumbstickLeftEvent);
		}
	}

	// Check if the state of the stick has changed
	if (currLY != prevLY)
	{
		// Check if the stick is pushed up
		if (currLY > 0)
		{
			// Submit input
			sInputEvent leftThumbstickUpEvent = {};
			leftThumbstickUpEvent.repeatedKey = false;
			leftThumbstickUpEvent.input = platformLayer::keyCodes::Gamepad_Left_Thumbstick_Up;
			leftThumbstickUpEvent.port = port;
			leftThumbstickUpEvent.data = 1.0f;

			game::onInputEvent(nullptr, leftThumbstickUpEvent);
		}
		// Check if the stick is pushed down
		else if (currLY < 0)
		{
			// Submit input
			sInputEvent leftThumbstickDownEvent = {};
			leftThumbstickDownEvent.repeatedKey = false;
			leftThumbstickDownEvent.input = platformLayer::keyCodes::Gamepad_Left_Thumbstick_Down;
			leftThumbstickDownEvent.port = port;
			leftThumbstickDownEvent.data = 1.0f;

			game::onInputEvent(nullptr, leftThumbstickDownEvent);
		}
	}

	// Right
	// Axis
	// Calculate normalized axis value after a deadzone has been applied
	auto thumbRX = static_cast<float>(state.Gamepad.sThumbRX);
	auto thumbRY = static_cast<float>(state.Gamepad.sThumbRY);
	applyCircularDeadzone(
		thumbRX,
		thumbRY,
		gamepadRightStickDeadzoneRadius
	);

	// Submit inputs for the stick
	sInputEvent rightXAxisEvent = {};
	rightXAxisEvent.repeatedKey = false;
	rightXAxisEvent.input = platformLayer::keyCodes::Gamepad_Right_Thumbstick_X_Axis;
	rightXAxisEvent.port = port;
	rightXAxisEvent.data = thumbRX;

	game::onInputEvent(nullptr, rightXAxisEvent);

	sInputEvent rightYAxisEvent = {};
	rightYAxisEvent.repeatedKey = false;
	rightYAxisEvent.input = platformLayer::keyCodes::Gamepad_Right_Thumbstick_Y_Axis;
	rightYAxisEvent.port = port;
	rightYAxisEvent.data = thumbRY;

	game::onInputEvent(nullptr, rightYAxisEvent);

	// Action
	// Get the current and previous states of the stick
	auto currRX = state.Gamepad.sThumbRX / gamepadMaxStickMagnitude;
	auto prevRX = prevState.Gamepad.sThumbRX / gamepadMaxStickMagnitude;

	auto currRY = state.Gamepad.sThumbRY / gamepadMaxStickMagnitude;
	auto prevRY = prevState.Gamepad.sThumbRY / gamepadMaxStickMagnitude;

	// Check if the state of the stick has changed
	if (currRX != prevRX)
	{
		// Check if the stick is pushed right
		if (currRX > 0)
		{
			// Submit input
			sInputEvent rightThumbstickRightEvent = {};
			rightThumbstickRightEvent.repeatedKey = false;
			rightThumbstickRightEvent.input = platformLayer::keyCodes::Gamepad_Right_Thumbstick_Right;
			rightThumbstickRightEvent.port = port;
			rightThumbstickRightEvent.data = 1.0f;

			game::onInputEvent(nullptr, rightThumbstickRightEvent);
		}
		// Check if the stick is pushed left
		else if (currRX < 0)
		{
			// Submit input
			sInputEvent rightThumbstickLeftEvent = {};
			rightThumbstickLeftEvent.repeatedKey = false;
			rightThumbstickLeftEvent.input = platformLayer::keyCodes::Gamepad_Right_Thumbstick_Left;
			rightThumbstickLeftEvent.port = port;
			rightThumbstickLeftEvent.data = 1.0f;

			game::onInputEvent(nullptr, rightThumbstickLeftEvent);
		}
	}

	// Check if the state of the stick has changed
	if (currRY != prevRY)
	{
		// Check if the stick is pushed up
		if (currRY > 0)
		{
			// Submit input
			sInputEvent rightThumbstickUpEvent = {};
			rightThumbstickUpEvent.repeatedKey = false;
			rightThumbstickUpEvent.input = platformLayer::keyCodes::Gamepad_Right_Thumbstick_Up;
			rightThumbstickUpEvent.port = port;
			rightThumbstickUpEvent.data = 1.0f;

			game::onInputEvent(nullptr, rightThumbstickUpEvent);
		}
		// Check if the stick is pushed down
		else if (currRY < 0)
		{
			// Submit input
			sInputEvent rightThumbstickDownEvent = {};
			rightThumbstickDownEvent.repeatedKey = false;
			rightThumbstickDownEvent.input = platformLayer::keyCodes::Gamepad_Right_Thumbstick_Down;
			rightThumbstickDownEvent.port = port;
			rightThumbstickDownEvent.data = 1.0f;

			game::onInputEvent(nullptr, rightThumbstickDownEvent);
		}
	}
}

static void pollTriggers(const XINPUT_STATE& state, const XINPUT_STATE& prevState, uint32_t port)
{
	// Action
	// Left
	auto currL = state.Gamepad.bLeftTrigger / gamepadMaxTriggerMagnitude;
	auto prevL = prevState.Gamepad.bLeftTrigger / gamepadMaxTriggerMagnitude;
	if (prevL != currL)
	{
		sInputEvent leftActionEvent = {};
		leftActionEvent.repeatedKey = false;
		leftActionEvent.input = platformLayer::keyCodes::Gamepad_Left_Trigger;
		leftActionEvent.port = port;
		leftActionEvent.data = static_cast<float>(currL);

		game::onInputEvent(nullptr, leftActionEvent);
	}

	// Right
	auto currR = state.Gamepad.bRightTrigger / gamepadMaxTriggerMagnitude;
	auto prevR = prevState.Gamepad.bRightTrigger / gamepadMaxTriggerMagnitude;
	if (prevR != currR)
	{
		sInputEvent rightActionEvent = {};
		rightActionEvent.repeatedKey = false;
		rightActionEvent.input = platformLayer::keyCodes::Gamepad_Right_Trigger;
		rightActionEvent.port = port;
		rightActionEvent.data = static_cast<float>(currR);

		game::onInputEvent(nullptr, rightActionEvent);
	}

	// Axis
	// Left
	sInputEvent leftAxisEvent = {};
	leftAxisEvent.repeatedKey = false;
	leftAxisEvent.input = platformLayer::keyCodes::Gamepad_Left_Trigger_Axis;
	leftAxisEvent.port = port;
	leftAxisEvent.data = static_cast<float>(state.Gamepad.bLeftTrigger) / static_cast<float>(gamepadMaxTriggerMagnitude);

	game::onInputEvent(nullptr, leftAxisEvent);

	// Right
	sInputEvent rightAxisEvent = {};
	rightAxisEvent.repeatedKey = false;
	rightAxisEvent.input = platformLayer::keyCodes::Gamepad_Right_Trigger_Axis;
	rightAxisEvent.port = port;
	rightAxisEvent.data = static_cast<float>(state.Gamepad.bRightTrigger) / static_cast<float>(gamepadMaxTriggerMagnitude);

	game::onInputEvent(nullptr, rightAxisEvent);
}

namespace platformLayer
{
	void pollGamepads()
	{
		for (uint32_t i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			XINPUT_STATE state;
			if (XInputGetState(static_cast<DWORD>(i), &state) != ERROR_SUCCESS)
			{
				continue;
			}

			pollButtons(state, prevStates[i], i);
			pollThumbsticks(state, prevStates[i], i);
			pollTriggers(state, prevStates[i], i);

			prevStates[i] = state;
		}
	}

	int8_t setGamepadVibration(const uint32_t port, const uint16_t leftMotorSpeed, const uint16_t rightMotorSpeed)
	{
		XINPUT_VIBRATION vibration = {};
		vibration.wLeftMotorSpeed = leftMotorSpeed;
		vibration.wRightMotorSpeed = rightMotorSpeed;
		const DWORD setStateResult = XInputSetState(static_cast<DWORD>(port), &vibration);

		if (setStateResult == ERROR_DEVICE_NOT_CONNECTED)
		{
			return 1;
		}

		return (setStateResult == ERROR_SUCCESS) ? 0 : 2;
	}
}
