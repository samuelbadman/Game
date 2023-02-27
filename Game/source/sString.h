#pragma once

// Thin wrapper struct around std::string to provide helper functions
struct sString
{
public:
	static std::string printf(const char* format, ...);

public:
	sString() = default;
	sString(const std::string& string);
	sString(const char* string);
	sString(const sString& string);

	void operator=(const std::string& rhs) { internalString = rhs; }
	void operator=(const char* rhs) { internalString = rhs; }
	void operator=(const sString& rhs) { internalString = rhs.internalString; }

public:
	const std::string& getString() const { return internalString; }
	constexpr const char* getCString() const { return internalString.c_str(); }

private:
	std::string internalString;
};