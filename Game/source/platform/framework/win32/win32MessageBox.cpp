#include "pch.h"

#if defined(PLATFORM_WIN32)

#include "win32MessageBox.h"

void win32MessageBox::messageBox(const eMessageLevel level, const std::string& message)
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

void win32MessageBox::messageBoxFatal(const std::string& message)
{
	LPCSTR caption = "Fatal";
	UINT type = MB_OK | MB_ICONERROR;
	MessageBoxA(0, message.c_str(), caption, type);
	exit(EXIT_FAILURE);
}

#endif // PLATFORM_WIN32
