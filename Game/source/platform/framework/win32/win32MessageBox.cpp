#include "pch.h"

#include "platform/framework/abstract/platformMessageBox.h"

namespace platformLayer
{
	namespace messageBox
	{
		void showMessageBox(const eMessageLevel level, const std::string& message)
		{
			LPCSTR caption = "\0";
			UINT type = MB_OK;

			switch (level)
			{
			case eMessageLevel::message:
			{
				caption = "Message";
				type |= MB_ICONINFORMATION;
			}
			break;

			case eMessageLevel::warning:
			{
				caption = "Warning";
				type |= MB_ICONWARNING;
			}
			break;

			case eMessageLevel::error:
			{
				caption = "Error";
				type |= MB_ICONERROR;
			}
			break;
			}

			MessageBoxA(0, message.c_str(), caption, type);
		}

		void showMessageBoxFatal(const std::string& message)
		{
			LPCSTR caption = "Fatal Error";
			UINT type = MB_OK | MB_ICONERROR;
			MessageBoxA(0, message.c_str(), caption, type);
			exit(EXIT_FAILURE);
		}
	}
}