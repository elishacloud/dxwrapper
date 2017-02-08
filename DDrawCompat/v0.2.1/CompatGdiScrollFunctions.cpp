#include "CompatGdi.h"
#include "CompatGdiScrollFunctions.h"
#include "DDrawLog.h"
#include "Hook.h"
#include "RealPrimarySurface.h"

namespace
{
	BOOL WINAPI scrollWindow(HWND hWnd, int XAmount, int YAmount,
		const RECT* lpRect, const RECT* lpClipRect)
	{
		Compat::LogEnter("scrollWindow", hWnd, XAmount, YAmount, lpRect, lpClipRect);
		BOOL result = CALL_ORIG_FUNC(ScrollWindow)(hWnd, XAmount, YAmount, lpRect, lpClipRect);
		if (result)
		{
			CompatGdiScrollFunctions::updateScrolledWindow(hWnd);
		}
		Compat::LogLeave("scrollWindow", hWnd, XAmount, YAmount, lpRect, lpClipRect) << result;
		return result;
	}

	int WINAPI scrollWindowEx(HWND hWnd, int dx, int dy, const RECT* prcScroll, const RECT* prcClip,
		HRGN hrgnUpdate, LPRECT prcUpdate, UINT flags)
	{
		Compat::LogEnter("scrollWindowEx",
			hWnd, dx, dy, prcScroll, prcClip, hrgnUpdate, prcUpdate, flags);

		if (flags & SW_SMOOTHSCROLL)
		{
			flags = LOWORD(flags ^ SW_SMOOTHSCROLL);
		}

		int result = CALL_ORIG_FUNC(ScrollWindowEx)(
			hWnd, dx, dy, prcScroll, prcClip, hrgnUpdate, prcUpdate, flags);
		if (ERROR != result)
		{
			CompatGdiScrollFunctions::updateScrolledWindow(hWnd);
		}

		Compat::LogLeave("scrollWindowEx",
			hWnd, dx, dy, prcScroll, prcClip, hrgnUpdate, prcUpdate, flags) << result;
		return result;
	}
}

namespace CompatGdiScrollFunctions
{
	void installHooks()
	{
		HOOK_FUNCTION(user32, ScrollWindow, scrollWindow);
		HOOK_FUNCTION(user32, ScrollWindowEx, scrollWindowEx);
	}

	void updateScrolledWindow(HWND hwnd)
	{
		if (CompatGdi::isEmulationEnabled())
		{
			RealPrimarySurface::disableUpdates();
			RedrawWindow(hwnd, nullptr, nullptr,
				RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_NOCHILDREN | RDW_UPDATENOW);
			RealPrimarySurface::enableUpdates();
		}
	}
}
