#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace Compat21
{
	namespace CompatRegistry
	{
		void installHooks();
		void setValue(HKEY key, const char* subKey, const char* valueName, DWORD value);
	}
}
