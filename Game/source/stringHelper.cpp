#include "pch.h"
#include "stringHelper.h"

std::string stringHelper::printf(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	// Convert to formatted string to a string. Requires heap allocation
	size_t size = vsnprintf(nullptr, 0, format, args) + 1;
	std::string buf;
	buf.resize(size);
	vsnprintf(buf.data(), size, format, args);
	buf[size - 1] = '\0';

	// Print string created from formatted string
	//std::cout << buf << '\n';

	// Just print formatted string
	//vprintf(format, args);

	va_end(args);

	return buf;
}
