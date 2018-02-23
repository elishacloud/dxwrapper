#pragma once

namespace Compat21
{
	namespace CompatGdiScrollFunctions
	{
		void installHooks();
		void updateScrolledWindow(HWND hwnd);
	};
}
