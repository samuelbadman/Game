#pragma once

enum class eMessageLevel : uint8_t
{
	message = 0,
	warning = 1,
	error = 2
};

namespace platformLayer
{
	extern void messageBox(const eMessageLevel level, const std::string& message);
	// Shows a message box and immediately exits the application
	extern void messageBoxFatal(const std::string& message);
}
