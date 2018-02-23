#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace Compat21
{
	namespace CompatActivateAppHandler
	{
		void installHooks();
		void setFullScreenCooperativeLevel(HWND hwnd, DWORD flags);
		void uninstallHooks();
	}
}
