#pragma once

namespace platformLayer
{
	namespace messageBox
	{
		enum class eMessageLevel : uint8_t
		{
			message = 0,
			warning = 1,
			error = 2
		};

		extern void showMessageBox(const eMessageLevel level, const std::string& message);
		// Shows a message box and immediately exits the application
		extern void showMessageBoxFatal(const std::string& message);
	}
}
