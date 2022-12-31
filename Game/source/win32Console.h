#pragma once

#include <cstdint>
#include <string>

class win32Console
{
public:
	static bool init(uint32_t lines);
	static bool shutdown();
	static void print(const std::string& string);
};