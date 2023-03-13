#include "pch.h"
#include "platform/framework/abstract/platformGamepad.h"
#include "platform/framework/abstract/platformKeyCodes.h"
#include "platform/framework/events/sInputEvent.h"

static std::vector<std::function<void(platformLayer::input::sInputEvent&&)>> onInputEventCallbacks;

static XINPUT_STATE prevStates[XUSER_MAX_COUNT];

static constexpr float gamepadLeftStickDeadzoneRadius = 0.24f;
static constexpr float gamepadRightStickDeadzoneRadius = 0.24f;
static constexpr int16_t gamepadMaxStickMagnitude = 32767;
static constexpr int16_t gamepadMaxTriggerMagnitude = 255;

static void broadcastInputEvent(platformLayer::input::sInputEvent&& evt)
{
	for (const std::function<void(platformLayer::input::sInputEvent&&)>& callback : onInputEventCallbacks)
	{
		callback(std::move(evt));
	}
}

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
		platformLayer::input::sInputEvent evt = {};
		evt.repeatedKey = false;
		evt.input = button;
		evt.port = port;
		evt.data = 1.0f;

		broadcastInputEvent(std::move(evt));
	}
	else
	{
		// Button released
		platformLayer::input::sInputEvent evt = {};
		evt.repeatedKey = false;
		evt.input = button;
		evt.port = port;
		evt.data = 0.0f;

		broadcastInputEvent(std::move(evt));
	}
}

static void pollButtons(const XINPUT_STATE& state, const XINPUT_STATE& prevState, uint32_t port)
{
	pollButton(state, prevState, port, platformLayer::input::keyCodes::Gamepad_Face_Button_Bottom);
	pollButton(state, prevState, port, platformLayer::input::keyCodes::Gamepad_Face_Button_Top);
	pollButton(state, prevState, port, platformLayer::input::keyCodes::Gamepad_Face_Button_Left);
	pollButton(state, prevState, port, platformLayer::input::keyCodes::Gamepad_Face_Button_Right);
	pollButton(state, prevState, port, platformLayer::input::keyCodes::Gamepad_D_Pad_Down);
	pollButton(state, prevState, port, platformLayer::input::keyCodes::Gamepad_D_Pad_Up);
	pollButton(state, prevState, port, platformLayer::input::keyCodes::Gamepad_D_Pad_Left);
	pollButton(state, prevState, port, platformLayer::input::keyCodes::Gamepad_D_Pad_Right);
	pollButton(state, prevState, port, platformLayer::input::keyCodes::Gamepad_Special_Left);
	pollButton(state, prevState, port, platformLayer::input::keyCodes::Gamepad_Special_Right);
	pollButton(state, prevState, port, platformLayer::input::keyCodes::Gamepad_Left_Shoulder);
	pollButton(state, prevState, port, platformLayer::input::keyCodes::Gamepad_Right_Shoulder);
	pollButton(state, prevState, port, platformLayer::input::keyCodes::Gamepad_Left_Thumbstick_Button);
	pollButton(state, prevState, port, platformLayer::input::keyCodes::Gamepad_Right_Thumbstick_Button);
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
	platformLayer::input::sInputEvent leftXAxisEvent = {};
	leftXAxisEvent.repeatedKey = false;
	leftXAxisEvent.input = platformLayer::input::keyCodes::Gamepad_Left_Thumbstick_X_Axis;
	leftXAxisEvent.port = port;
	leftXAxisEvent.data = thumbLX;

	broadcastInputEvent(std::move(leftXAxisEvent));

	platformLayer::input::sInputEvent leftYAxisEvent = {};
	leftYAxisEvent.repeatedKey = false;
	leftYAxisEvent.input = platformLayer::input::keyCodes::Gamepad_Left_Thumbstick_Y_Axis;
	leftYAxisEvent.port = port;
	leftYAxisEvent.data = thumbLY;

	broadcastInputEvent(std::move(leftYAxisEvent));

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
			platformLayer::input::sInputEvent leftThumbstickRightEvent = {};
			leftThumbstickRightEvent.repeatedKey = false;
			leftThumbstickRightEvent.input = platformLayer::input::keyCodes::Gamepad_Left_Thumbstick_Right;
			leftThumbstickRightEvent.port = port;
			leftThumbstickRightEvent.data = 1.0f;

			broadcastInputEvent(std::move(leftThumbstickRightEvent));
		}
		// Check if the stick is pushed left
		else if (currLX < 0)
		{
			// Submit input
			platformLayer::input::sInputEvent leftThumbstickLeftEvent = {};
			leftThumbstickLeftEvent.repeatedKey = false;
			leftThumbstickLeftEvent.input = platformLayer::input::keyCodes::Gamepad_Left_Thumbstick_Left;
			leftThumbstickLeftEvent.port = port;
			leftThumbstickLeftEvent.data = 1.0f;

			broadcastInputEvent(std::move(leftThumbstickLeftEvent));
		}
	}

	// Check if the state of the stick has changed
	if (currLY != prevLY)
	{
		// Check if the stick is pushed up
		if (currLY > 0)
		{
			// Submit input
			platformLayer::input::sInputEvent leftThumbstickUpEvent = {};
			leftThumbstickUpEvent.repeatedKey = false;
			leftThumbstickUpEvent.input = platformLayer::input::keyCodes::Gamepad_Left_Thumbstick_Up;
			leftThumbstickUpEvent.port = port;
			leftThumbstickUpEvent.data = 1.0f;

			broadcastInputEvent(std::move(leftThumbstickUpEvent));
		}
		// Check if the stick is pushed down
		else if (currLY < 0)
		{
			// Submit input
			platformLayer::input::sInputEvent leftThumbstickDownEvent = {};
			leftThumbstickDownEvent.repeatedKey = false;
			leftThumbstickDownEvent.input = platformLayer::input::keyCodes::Gamepad_Left_Thumbstick_Down;
			leftThumbstickDownEvent.port = port;
			leftThumbstickDownEvent.data = 1.0f;

			broadcastInputEvent(std::move(leftThumbstickDownEvent));
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
	platformLayer::input::sInputEvent rightXAxisEvent = {};
	rightXAxisEvent.repeatedKey = false;
	rightXAxisEvent.input = platformLayer::input::keyCodes::Gamepad_Right_Thumbstick_X_Axis;
	rightXAxisEvent.port = port;
	rightXAxisEvent.data = thumbRX;

	broadcastInputEvent(std::move(rightXAxisEvent));

	platformLayer::input::sInputEvent rightYAxisEvent = {};
	rightYAxisEvent.repeatedKey = false;
	rightYAxisEvent.input = platformLayer::input::keyCodes::Gamepad_Right_Thumbstick_Y_Axis;
	rightYAxisEvent.port = port;
	rightYAxisEvent.data = thumbRY;

	broadcastInputEvent(std::move(rightYAxisEvent));

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
			platformLayer::input::sInputEvent rightThumbstickRightEvent = {};
			rightThumbstickRightEvent.repeatedKey = false;
			rightThumbstickRightEvent.input = platformLayer::input::keyCodes::Gamepad_Right_Thumbstick_Right;
			rightThumbstickRightEvent.port = port;
			rightThumbstickRightEvent.data = 1.0f;

			broadcastInputEvent(std::move(rightThumbstickRightEvent));
		}
		// Check if the stick is pushed left
		else if (currRX < 0)
		{
			// Submit input
			platformLayer::input::sInputEvent rightThumbstickLeftEvent = {};
			rightThumbstickLeftEvent.repeatedKey = false;
			rightThumbstickLeftEvent.input = platformLayer::input::keyCodes::Gamepad_Right_Thumbstick_Left;
			rightThumbstickLeftEvent.port = port;
			rightThumbstickLeftEvent.data = 1.0f;

			broadcastInputEvent(std::move(rightThumbstickLeftEvent));
		}
	}

	// Check if the state of the stick has changed
	if (currRY != prevRY)
	{
		// Check if the stick is pushed up
		if (currRY > 0)
		{
			// Submit input
			platformLayer::input::sInputEvent rightThumbstickUpEvent = {};
			rightThumbstickUpEvent.repeatedKey = false;
			rightThumbstickUpEvent.input = platformLayer::input::keyCodes::Gamepad_Right_Thumbstick_Up;
			rightThumbstickUpEvent.port = port;
			rightThumbstickUpEvent.data = 1.0f;

			broadcastInputEvent(std::move(rightThumbstickUpEvent));
		}
		// Check if the stick is pushed down
		else if (currRY < 0)
		{
			// Submit input
			platformLayer::input::sInputEvent rightThumbstickDownEvent = {};
			rightThumbstickDownEvent.repeatedKey = false;
			rightThumbstickDownEvent.input = platformLayer::input::keyCodes::Gamepad_Right_Thumbstick_Down;
			rightThumbstickDownEvent.port = port;
			rightThumbstickDownEvent.data = 1.0f;

			broadcastInputEvent(std::move(rightThumbstickDownEvent));
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
		platformLayer::input::sInputEvent leftActionEvent = {};
		leftActionEvent.repeatedKey = false;
		leftActionEvent.input = platformLayer::input::keyCodes::Gamepad_Left_Trigger;
		leftActionEvent.port = port;
		leftActionEvent.data = static_cast<float>(currL);

		broadcastInputEvent(std::move(leftActionEvent));
	}

	// Right
	auto currR = state.Gamepad.bRightTrigger / gamepadMaxTriggerMagnitude;
	auto prevR = prevState.Gamepad.bRightTrigger / gamepadMaxTriggerMagnitude;
	if (prevR != currR)
	{
		platformLayer::input::sInputEvent rightActionEvent = {};
		rightActionEvent.repeatedKey = false;
		rightActionEvent.input = platformLayer::input::keyCodes::Gamepad_Right_Trigger;
		rightActionEvent.port = port;
		rightActionEvent.data = static_cast<float>(currR);

		broadcastInputEvent(std::move(rightActionEvent));
	}

	// Axis
	// Left
	platformLayer::input::sInputEvent leftAxisEvent = {};
	leftAxisEvent.repeatedKey = false;
	leftAxisEvent.input = platformLayer::input::keyCodes::Gamepad_Left_Trigger_Axis;
	leftAxisEvent.port = port;
	leftAxisEvent.data = static_cast<float>(state.Gamepad.bLeftTrigger) / static_cast<float>(gamepadMaxTriggerMagnitude);

	broadcastInputEvent(std::move(leftAxisEvent));

	// Right
	platformLayer::input::sInputEvent rightAxisEvent = {};
	rightAxisEvent.repeatedKey = false;
	rightAxisEvent.input = platformLayer::input::keyCodes::Gamepad_Right_Trigger_Axis;
	rightAxisEvent.port = port;
	rightAxisEvent.data = static_cast<float>(state.Gamepad.bRightTrigger) / static_cast<float>(gamepadMaxTriggerMagnitude);

	broadcastInputEvent(std::move(rightAxisEvent));
}

namespace platformLayer
{
	namespace gamepad
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

		void addOnInputEventDelegate(const std::function<void(platformLayer::input::sInputEvent&&)>& inDelegate)
		{
			onInputEventCallbacks.push_back(inDelegate);
		}
	}
}
