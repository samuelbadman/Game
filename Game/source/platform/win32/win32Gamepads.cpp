#include "pch.h"
#include "win32Gamepads.h"
#include "win32InputKeyCode.h"

XINPUT_STATE win32Gamepads::prevStates[XUSER_MAX_COUNT];
callback<const inputEvent&> win32Gamepads::onInput;

void win32Gamepads::refresh()
{
	for (size_t i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		XINPUT_STATE state = {};
		if (!XInputGetState(static_cast<DWORD>(i), &state) == ERROR_SUCCESS)
		{
			continue;
		}

		refreshButtons(state, prevStates[i], static_cast<uint32_t>(i));
		refreshThumbsticks(state, prevStates[i], static_cast<uint32_t>(i));
		refreshTriggers(state, prevStates[i], static_cast<uint32_t>(i));

		prevStates[i] = state;
	}
}

void win32Gamepads::applyCircularDeadzone(float& axisX, float& axisY, 
	float deadzoneRadius)
{
	float normX = 
		std::max(-1.0f, axisX / static_cast<float>(gamepadMaxStickMagnitude));
	float normY = 
		std::max(-1.0f, axisY / static_cast<float>(gamepadMaxStickMagnitude));

	float absNormX = std::abs(normX);
	float absNormY = std::abs(normY);
	axisX = (absNormX < deadzoneRadius ?
		0.0f : (absNormX - deadzoneRadius) * (normX / absNormX));
	axisY = (absNormY < deadzoneRadius ?
		0.0f : (absNormY - deadzoneRadius) * (normY / absNormY));

	if (deadzoneRadius > 0.0f)
	{
		axisX /= 1.0f - deadzoneRadius;
		axisY /= 1.0f - deadzoneRadius;
	}
}

void win32Gamepads::refreshButton(const XINPUT_STATE& state, 
	const XINPUT_STATE& prevState, uint32_t port, int16_t button)
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
		inputEvent event = {};
		event.repeatedKey = false;
		event.input = button;
		event.port = port;
		event.data = 1.0f;
		onInput.broadcast(event);
	}
	else
	{
		// Button released
		inputEvent event = {};
		event.repeatedKey = false;
		event.input = button;
		event.port = port;
		event.data = 0.0f;
		onInput.broadcast(event);
	}
}

void win32Gamepads::refreshButtons(const XINPUT_STATE& state, 
	const XINPUT_STATE& prevState, uint32_t port)
{
	refreshButton(state, prevState, port, 
		win32InputKeyCode::Gamepad_Face_Button_Bottom);
	refreshButton(state, prevState, port, 
		win32InputKeyCode::Gamepad_Face_Button_Top);
	refreshButton(state, prevState, port, 
		win32InputKeyCode::Gamepad_Face_Button_Left);
	refreshButton(state, prevState, port, 
		win32InputKeyCode::Gamepad_Face_Button_Right);
	refreshButton(state, prevState, port, 
		win32InputKeyCode::Gamepad_D_Pad_Down);
	refreshButton(state, prevState, port, 
		win32InputKeyCode::Gamepad_D_Pad_Up);
	refreshButton(state, prevState, port, 
		win32InputKeyCode::Gamepad_D_Pad_Left);
	refreshButton(state, prevState, port, 
		win32InputKeyCode::Gamepad_D_Pad_Right);
	refreshButton(state, prevState, port, 
		win32InputKeyCode::Gamepad_Special_Left);
	refreshButton(state, prevState, port, 
		win32InputKeyCode::Gamepad_Special_Right);
	refreshButton(state, prevState, port, 
		win32InputKeyCode::Gamepad_Left_Shoulder);
	refreshButton(state, prevState, port, 
		win32InputKeyCode::Gamepad_Right_Shoulder);
	refreshButton(state, prevState, port, 
		win32InputKeyCode::Gamepad_Left_Thumbstick_Button);
	refreshButton(state, prevState, port, 
		win32InputKeyCode::Gamepad_Right_Thumbstick_Button);
}

void win32Gamepads::refreshThumbsticks(const XINPUT_STATE& state,
	const XINPUT_STATE& prevState, uint32_t port)
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
	inputEvent leftXAxisEvent = {};
	leftXAxisEvent.repeatedKey = false;
	leftXAxisEvent.input = win32InputKeyCode::Gamepad_Left_Thumbstick_X_Axis;
	leftXAxisEvent.port = port;
	leftXAxisEvent.data = thumbLX;
	onInput.broadcast(leftXAxisEvent);

	inputEvent leftYAxisEvent = {};
	leftYAxisEvent.repeatedKey = false;
	leftYAxisEvent.input = win32InputKeyCode::Gamepad_Left_Thumbstick_Y_Axis;
	leftYAxisEvent.port = port;
	leftYAxisEvent.data = thumbLY;
	onInput.broadcast(leftYAxisEvent);

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
			inputEvent leftThumbstickRightEvent = {};
			leftThumbstickRightEvent.repeatedKey = false;
			leftThumbstickRightEvent.input =
				win32InputKeyCode::Gamepad_Left_Thumbstick_Right;
			leftThumbstickRightEvent.port = port;
			leftThumbstickRightEvent.data = 1.0f;
			onInput.broadcast(leftThumbstickRightEvent);
		}
		// Check if the stick is pushed left
		else if (currLX < 0)
		{
			// Submit input
			inputEvent leftThumbstickLeftEvent = {};
			leftThumbstickLeftEvent.repeatedKey = false;
			leftThumbstickLeftEvent.input =
				win32InputKeyCode::Gamepad_Left_Thumbstick_Left;
			leftThumbstickLeftEvent.port = port;
			leftThumbstickLeftEvent.data = 1.0f;
			onInput.broadcast(leftThumbstickLeftEvent);
		}
	}

	// Check if the state of the stick has changed
	if (currLY != prevLY)
	{
		// Check if the stick is pushed up
		if (currLY > 0)
		{
			// Submit input
			inputEvent leftThumbstickUpEvent = {};
			leftThumbstickUpEvent.repeatedKey = false;
			leftThumbstickUpEvent.input =
				win32InputKeyCode::Gamepad_Left_Thumbstick_Up;
			leftThumbstickUpEvent.port = port;
			leftThumbstickUpEvent.data = 1.0f;
			onInput.broadcast(leftThumbstickUpEvent);
		}
		// Check if the stick is pushed down
		else if (currLY < 0)
		{
			// Submit input
			inputEvent leftThumbstickDownEvent = {};
			leftThumbstickDownEvent.repeatedKey = false;
			leftThumbstickDownEvent.input =
				win32InputKeyCode::Gamepad_Left_Thumbstick_Down;
			leftThumbstickDownEvent.port = port;
			leftThumbstickDownEvent.data = 1.0f;
			onInput.broadcast(leftThumbstickDownEvent);
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
	inputEvent rightXAxisEvent = {};
	rightXAxisEvent.repeatedKey = false;
	rightXAxisEvent.input =
		win32InputKeyCode::Gamepad_Right_Thumbstick_X_Axis;
	rightXAxisEvent.port = port;
	rightXAxisEvent.data = thumbRX;
	onInput.broadcast(rightXAxisEvent);

	inputEvent rightYAxisEvent = {};
	rightYAxisEvent.repeatedKey = false;
	rightYAxisEvent.input =
		win32InputKeyCode::Gamepad_Right_Thumbstick_Y_Axis;
	rightYAxisEvent.port = port;
	rightYAxisEvent.data = thumbRY;
	onInput.broadcast(rightYAxisEvent);

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
			inputEvent rightThumbstickRightEvent = {};
			rightThumbstickRightEvent.repeatedKey = false;
			rightThumbstickRightEvent.input =
				win32InputKeyCode::Gamepad_Right_Thumbstick_Right;
			rightThumbstickRightEvent.port = port;
			rightThumbstickRightEvent.data = 1.0f;
			onInput.broadcast(rightThumbstickRightEvent);
		}
		// Check if the stick is pushed left
		else if (currRX < 0)
		{
			// Submit input
			inputEvent rightThumbstickLeftEvent = {};
			rightThumbstickLeftEvent.repeatedKey = false;
			rightThumbstickLeftEvent.input =
				win32InputKeyCode::Gamepad_Right_Thumbstick_Left;
			rightThumbstickLeftEvent.port = port;
			rightThumbstickLeftEvent.data = 1.0f;
			onInput.broadcast(rightThumbstickLeftEvent);
		}
	}

	// Check if the state of the stick has changed
	if (currRY != prevRY)
	{
		// Check if the stick is pushed up
		if (currRY > 0)
		{
			// Submit input
			inputEvent rightThumbstickUpEvent = {};
			rightThumbstickUpEvent.repeatedKey = false;
			rightThumbstickUpEvent.input =
				win32InputKeyCode::Gamepad_Right_Thumbstick_Up;
			rightThumbstickUpEvent.port = port;
			rightThumbstickUpEvent.data = 1.0f;
			onInput.broadcast(rightThumbstickUpEvent);
		}
		// Check if the stick is pushed down
		else if (currRY < 0)
		{
			// Submit input
			inputEvent rightThumbstickDownEvent = {};
			rightThumbstickDownEvent.repeatedKey = false;
			rightThumbstickDownEvent.input =
				win32InputKeyCode::Gamepad_Right_Thumbstick_Down;
			rightThumbstickDownEvent.port = port;
			rightThumbstickDownEvent.data = 1.0f;
			onInput.broadcast(rightThumbstickDownEvent);
		}
	}
}

void win32Gamepads::refreshTriggers(const XINPUT_STATE& state, 
	const XINPUT_STATE& prevState, uint32_t port)
{
	// Action
	// Left
	auto currL = state.Gamepad.bLeftTrigger / gamepadMaxTriggerMagnitude;
	auto prevL = prevState.Gamepad.bLeftTrigger / gamepadMaxTriggerMagnitude;
	if (prevL != currL)
	{
		inputEvent leftActionEvent = {};
		leftActionEvent.repeatedKey = false;
		leftActionEvent.input =
			win32InputKeyCode::Gamepad_Left_Trigger;
		leftActionEvent.port = port;
		leftActionEvent.data = static_cast<float>(currL);
		onInput.broadcast(leftActionEvent);
	}

	// Right
	auto currR = state.Gamepad.bRightTrigger / gamepadMaxTriggerMagnitude;
	auto prevR = prevState.Gamepad.bRightTrigger / gamepadMaxTriggerMagnitude;
	if (prevR != currR)
	{
		inputEvent rightActionEvent = {};
		rightActionEvent.repeatedKey = false;
		rightActionEvent.input =
			win32InputKeyCode::Gamepad_Right_Trigger;
		rightActionEvent.port = port;
		rightActionEvent.data = static_cast<float>(currR);
		onInput.broadcast(rightActionEvent);
	}

	// Axis
	// Left
	inputEvent leftAxisEvent = {};
	leftAxisEvent.repeatedKey = false;
	leftAxisEvent.input = win32InputKeyCode::Gamepad_Left_Trigger_Axis;
	leftAxisEvent.port = port;
	leftAxisEvent.data = static_cast<float>(state.Gamepad.bLeftTrigger) / 
		static_cast<float>(gamepadMaxTriggerMagnitude);
	onInput.broadcast(leftAxisEvent);

	// Right
	inputEvent rightAxisEvent = {};
	rightAxisEvent.repeatedKey = false;
	rightAxisEvent.input = win32InputKeyCode::Gamepad_Right_Trigger_Axis;
	rightAxisEvent.port = port;
	rightAxisEvent.data = static_cast<float>(state.Gamepad.bRightTrigger) /
		static_cast<float>(gamepadMaxTriggerMagnitude);
	onInput.broadcast(rightAxisEvent);
}
