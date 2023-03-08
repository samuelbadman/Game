#pragma once

namespace platformLayer
{
	namespace input
	{
		class keyCodes
		{
		public:

#if defined(PLATFORM_WIN32)

			static constexpr int16_t Backspace{ VK_BACK };
			static constexpr int16_t Tab{ VK_TAB };
			static constexpr int16_t Enter{ VK_RETURN };
			static constexpr int16_t Left_Shift{ VK_LSHIFT };
			static constexpr int16_t Right_Shift{ VK_RSHIFT };
			static constexpr int16_t Caps_Lock{ VK_CAPITAL };
			static constexpr int16_t Escape{ VK_ESCAPE };
			static constexpr int16_t Space_Bar{ VK_SPACE };
			static constexpr int16_t Page_Up{ VK_PRIOR };
			static constexpr int16_t Page_Down{ VK_NEXT };
			static constexpr int16_t End{ VK_END };
			static constexpr int16_t Home{ VK_HOME };
			static constexpr int16_t Insert{ VK_INSERT };
			static constexpr int16_t Delete{ VK_DELETE };
			static constexpr int16_t Left{ VK_LEFT };
			static constexpr int16_t Right{ VK_RIGHT };
			static constexpr int16_t Up{ VK_UP };
			static constexpr int16_t Down{ VK_DOWN };
			static constexpr int16_t Zero{ 0x30 };
			static constexpr int16_t One{ 0x31 };
			static constexpr int16_t Two{ 0x32 };
			static constexpr int16_t Three{ 0x33 };
			static constexpr int16_t Four{ 0x34 };
			static constexpr int16_t Five{ 0x35 };
			static constexpr int16_t Six{ 0x36 };
			static constexpr int16_t Seven{ 0x37 };
			static constexpr int16_t Eight{ 0x38 };
			static constexpr int16_t Nine{ 0x39 };
			static constexpr int16_t A{ 0x41 };
			static constexpr int16_t B{ 0x42 };
			static constexpr int16_t C{ 0x43 };
			static constexpr int16_t D{ 0x44 };
			static constexpr int16_t E{ 0x45 };
			static constexpr int16_t F{ 0x46 };
			static constexpr int16_t G{ 0x47 };
			static constexpr int16_t H{ 0x48 };
			static constexpr int16_t I{ 0x49 };
			static constexpr int16_t J{ 0x4A };
			static constexpr int16_t K{ 0x4B };
			static constexpr int16_t L{ 0x4C };
			static constexpr int16_t M{ 0x4D };
			static constexpr int16_t N{ 0x4E };
			static constexpr int16_t O{ 0x4F };
			static constexpr int16_t P{ 0x50 };
			static constexpr int16_t Q{ 0x51 };
			static constexpr int16_t R{ 0x52 };
			static constexpr int16_t S{ 0x53 };
			static constexpr int16_t T{ 0x54 };
			static constexpr int16_t U{ 0x55 };
			static constexpr int16_t V{ 0x56 };
			static constexpr int16_t W{ 0x57 };
			static constexpr int16_t X{ 0x58 };
			static constexpr int16_t Y{ 0x59 };
			static constexpr int16_t Z{ 0x5A };
			static constexpr int16_t Numpad_0{ VK_NUMPAD0 };
			static constexpr int16_t Numpad_1{ VK_NUMPAD1 };
			static constexpr int16_t Numpad_2{ VK_NUMPAD2 };
			static constexpr int16_t Numpad_3{ VK_NUMPAD3 };
			static constexpr int16_t Numpad_4{ VK_NUMPAD4 };
			static constexpr int16_t Numpad_5{ VK_NUMPAD5 };
			static constexpr int16_t Numpad_6{ VK_NUMPAD6 };
			static constexpr int16_t Numpad_7{ VK_NUMPAD7 };
			static constexpr int16_t Numpad_8{ VK_NUMPAD8 };
			static constexpr int16_t Numpad_9{ VK_NUMPAD9 };
			static constexpr int16_t F1{ VK_F1 };
			static constexpr int16_t F2{ VK_F2 };
			static constexpr int16_t F3{ VK_F3 };
			static constexpr int16_t F4{ VK_F4 };
			static constexpr int16_t F5{ VK_F5 };
			static constexpr int16_t F6{ VK_F6 };
			static constexpr int16_t F7{ VK_F7 };
			static constexpr int16_t F8{ VK_F8 };
			static constexpr int16_t F9{ VK_F9 };
			static constexpr int16_t F10{ VK_F10 };
			static constexpr int16_t F11{ VK_F11 };
			static constexpr int16_t F12{ VK_F12 };
			static constexpr int16_t Left_Ctrl{ VK_LCONTROL };
			static constexpr int16_t Right_Ctrl{ VK_RCONTROL };
			static constexpr int16_t Alt{ VK_MENU };
			static constexpr int16_t Tilde{ 223 };

			static constexpr int16_t Left_Mouse_Button{ MK_LBUTTON };
			static constexpr int16_t Right_Mouse_Button{ MK_RBUTTON };
			static constexpr int16_t Middle_Mouse_Button{ MK_MBUTTON };

			static constexpr int16_t Mouse_Wheel_Up{ 399 };
			static constexpr int16_t Mouse_Wheel_Down{ 400 };

			static constexpr int16_t Mouse_X{ 401 };
			static constexpr int16_t Mouse_Y{ 402 };

			static constexpr int16_t Gamepad_Face_Button_Bottom{ static_cast<int16_t>(XINPUT_GAMEPAD_A) };
			static constexpr int16_t Gamepad_Face_Button_Right{ static_cast<int16_t>(XINPUT_GAMEPAD_B) };
			static constexpr int16_t Gamepad_Face_Button_Left{ static_cast<int16_t>(XINPUT_GAMEPAD_X) };
			static constexpr int16_t Gamepad_Face_Button_Top{ static_cast<int16_t>(XINPUT_GAMEPAD_Y) };
			static constexpr int16_t Gamepad_D_Pad_Up{ static_cast<int16_t>(XINPUT_GAMEPAD_DPAD_UP) };
			static constexpr int16_t Gamepad_D_Pad_Down{ static_cast<int16_t>(XINPUT_GAMEPAD_DPAD_DOWN) };
			static constexpr int16_t Gamepad_D_Pad_Left{ static_cast<int16_t>(XINPUT_GAMEPAD_DPAD_LEFT) };
			static constexpr int16_t Gamepad_D_Pad_Right{ static_cast<int16_t>(XINPUT_GAMEPAD_DPAD_RIGHT) };
			static constexpr int16_t Gamepad_Left_Thumbstick_Button{ static_cast<int16_t>(XINPUT_GAMEPAD_LEFT_THUMB) };
			static constexpr int16_t Gamepad_Right_Thumbstick_Button{ static_cast<int16_t>(XINPUT_GAMEPAD_RIGHT_THUMB) };
			static constexpr int16_t Gamepad_Special_Left{ static_cast<int16_t>(XINPUT_GAMEPAD_BACK) };
			static constexpr int16_t Gamepad_Special_Right{ static_cast<int16_t>(XINPUT_GAMEPAD_START) };
			static constexpr int16_t Gamepad_Left_Shoulder{ static_cast<int16_t>(XINPUT_GAMEPAD_LEFT_SHOULDER) };
			static constexpr int16_t Gamepad_Right_Shoulder{ static_cast<int16_t>(XINPUT_GAMEPAD_RIGHT_SHOULDER) };

			static constexpr int16_t Gamepad_Left_Thumbstick_X_Axis{ 404 };
			static constexpr int16_t Gamepad_Left_Thumbstick_Y_Axis{ 405 };
			static constexpr int16_t Gamepad_Right_Thumbstick_X_Axis{ 406 };
			static constexpr int16_t Gamepad_Right_Thumbstick_Y_Axis{ 407 };
			static constexpr int16_t Gamepad_Left_Thumbstick_Up{ 408 };
			static constexpr int16_t Gamepad_Left_Thumbstick_Down{ 409 };
			static constexpr int16_t Gamepad_Left_Thumbstick_Left{ 410 };
			static constexpr int16_t Gamepad_Left_Thumbstick_Right{ 411 };
			static constexpr int16_t Gamepad_Right_Thumbstick_Up{ 412 };
			static constexpr int16_t Gamepad_Right_Thumbstick_Down{ 413 };
			static constexpr int16_t Gamepad_Right_Thumbstick_Left{ 414 };
			static constexpr int16_t Gamepad_Right_Thumbstick_Right{ 415 };

			static constexpr int16_t Gamepad_Left_Trigger{ 416 };
			static constexpr int16_t Gamepad_Right_Trigger{ 417 };
			static constexpr int16_t Gamepad_Left_Trigger_Axis{ 418 };
			static constexpr int16_t Gamepad_Right_Trigger_Axis{ 419 };

#elif defined(0)
#endif // PLATFORM_WIN32
		};
	}
}