#pragma once

// Returns non-zero if fails
extern int8_t platformInitConsole();
// Returns non-zero if fails
extern int8_t platformShutdownConsole();
extern void platformConsolePrint(const std::string& string);