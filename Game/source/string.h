#pragma once

// Thin wrapper struct around std::string to provide helper functions
struct sString
{
public:
	static std::string printf(const char* format, ...);

public:
	sString() = default;
	sString(const char* string);

private:
	std::string internalString;
};