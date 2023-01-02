#pragma once

#include <cstdint>
#include <string>

class win32Console
{
public:
	static bool init();
	static bool shutdown();
	static void print(const std::string& string);
};