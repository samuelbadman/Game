#pragma once

class win32Console
{
public:
	// Returns non-zero if fails
	static int8_t init();
	// Returns non-zero if fails
	static int8_t shutdown();
	static void print(const std::string& string);
};