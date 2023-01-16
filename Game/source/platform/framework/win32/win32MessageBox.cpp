#include "pch.h"
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
