#pragma once

enum class eMessageLevel : uint8_t
{
	message = 0,
	warning = 1,
	error = 2
};

extern void platformMessageBox(const eMessageLevel level, const std::string& message);
// Shows a message box and immediately exits the application
extern void platformMessageBoxFatal(const std::string& message);
