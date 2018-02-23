#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace Win32
{
	namespace Registry
	{
		void installHooks();
		void setValue(HKEY key, const char* subKey, const char* valueName, DWORD value);
		void unsetValue(HKEY key, const char* subKey, const char* valueName);
	}
}
