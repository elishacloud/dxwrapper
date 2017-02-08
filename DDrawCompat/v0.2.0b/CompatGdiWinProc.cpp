#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include "CompatGdi.h"
#include "CompatGdiDc.h"
#include "CompatGdiWinProc.h"
#include "DDrawLog.h"

namespace
{
	void eraseBackground(HWND hwnd, HDC dc);

	LRESULT CALLBACK callWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (HC_ACTION == nCode)
		{
			auto ret = reinterpret_cast<CWPRETSTRUCT*>(lParam);
			if (WM_ERASEBKGND == ret->message)
			{
				if (0 != ret->lResult)
				{
					eraseBackground(ret->hwnd, reinterpret_cast<HDC>(ret->wParam));
				}
			}
			else if (WM_WINDOWPOSCHANGED == ret->message)
			{
				CompatGdi::invalidate();
			}
			else if (WM_VSCROLL == ret->message || WM_HSCROLL == ret->message)
			{
				InvalidateRect(ret->hwnd, nullptr, TRUE);
			}
		}

		return CallNextHookEx(nullptr, nCode, wParam, lParam);
	}

	void eraseBackground(HWND hwnd, HDC dc)
	{
		if (CompatGdi::beginGdiRendering())
		{
			HDC compatDc = CompatGdiDc::getDc(dc);
			if (compatDc)
			{
				SendMessage(hwnd, WM_ERASEBKGND, reinterpret_cast<WPARAM>(compatDc), 0);
				CompatGdiDc::releaseDc(dc);
			}
			CompatGdi::endGdiRendering();
		}
	}
}

namespace CompatGdiWinProc
{
	void installHooks()
	{
		const DWORD threadId = GetCurrentThreadId();
		SetWindowsHookEx(WH_CALLWNDPROCRET, callWndRetProc, nullptr, threadId);
	}
}
