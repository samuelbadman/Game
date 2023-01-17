#pragma once

enum class eMessageLevel : uint8_t
{
	message = 0,
	warning = 1,
	error   = 2,
	fatal   = 3
};

class win32MessageBox
{
public:
	static void messageBox(const eMessageLevel level, const std::string& message);
};