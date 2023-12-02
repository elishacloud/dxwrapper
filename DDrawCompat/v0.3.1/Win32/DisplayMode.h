#pragma once

#include <Windows.h>

namespace Win32
{
	namespace DisplayMode
	{
		DWORD getBpp();
		ULONG queryDisplaySettingsUniqueness();

		void installHooks();
	}
}
