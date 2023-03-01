#pragma once

namespace platformLayer
{
	// Returns non-zero if fails
	extern int8_t initConsole();
	// Returns non-zero if fails
	extern int8_t shutdownConsole();
	extern void consolePrint(const std::string& string);
}