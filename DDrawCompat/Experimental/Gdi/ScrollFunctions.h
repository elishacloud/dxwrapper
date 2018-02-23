#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace Gdi
{
	namespace ScrollFunctions
	{
		void installHooks();
		void updateScrolledWindow(HWND hwnd);
	}
}
