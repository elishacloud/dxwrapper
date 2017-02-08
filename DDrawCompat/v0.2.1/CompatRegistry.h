#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace CompatRegistry
{
	void installHooks();
	void setValue(HKEY key, const char* subKey, const char* valueName, DWORD value);
}
