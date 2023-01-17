#pragma once

class win32Console
{
public:
	static int8_t init();
	static int8_t shutdown();
	static void print(const std::string& string);
};