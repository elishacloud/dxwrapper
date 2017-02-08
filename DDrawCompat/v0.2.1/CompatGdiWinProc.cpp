#define WIN32_LEAN_AND_MEAN

#include <unordered_map>

#include <dwmapi.h>
#include <Windows.h>

#include "CompatGdi.h"
#include "CompatGdiDc.h"
#include "CompatGdiScrollBar.h"
#include "CompatGdiScrollFunctions.h"
#include "CompatGdiTitleBar.h"
#include "CompatGdiWinProc.h"
#include "DDrawLog.h"
#include "ScopedCriticalSection.h"

namespace
{
	HHOOK g_callWndRetProcHook = nullptr;
	HWINEVENTHOOK g_objectStateChangeEventHook = nullptr;
	std::unordered_map<HWND, RECT> g_prevWindowRect;

	void disableDwmAttributes(HWND hwnd);
	void onActivate(HWND hwnd);
	void onMenuSelect();
	void onWindowPosChanged(HWND hwnd);
	void removeDropShadow(HWND hwnd);

	LRESULT CALLBACK callWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		auto ret = reinterpret_cast<CWPRETSTRUCT*>(lParam);
		Compat::LogEnter("callWndRetProc", nCode, wParam, ret);

		if (HC_ACTION == nCode)
		{
			if (WM_CREATE == ret->message)
			{
				disableDwmAttributes(ret->hwnd);
				removeDropShadow(ret->hwnd);
			}
			else if (WM_DESTROY == ret->message)
			{
				Compat::ScopedCriticalSection lock(CompatGdi::g_gdiCriticalSection);
				g_prevWindowRect.erase(ret->hwnd);
			}
			else if (WM_WINDOWPOSCHANGED == ret->message)
			{
				onWindowPosChanged(ret->hwnd);
			}
			else if (WM_ACTIVATE == ret->message)
			{
				onActivate(ret->hwnd);
			}
			else if (WM_COMMAND == ret->message)
			{
				auto notifCode = HIWORD(ret->wParam);
				if (ret->lParam && (EN_HSCROLL == notifCode || EN_VSCROLL == notifCode))
				{
					CompatGdiScrollFunctions::updateScrolledWindow(reinterpret_cast<HWND>(ret->lParam));
				}
			}
			else if (WM_MENUSELECT == ret->message)
			{
				onMenuSelect();
			}
		}

		LRESULT result = CallNextHookEx(nullptr, nCode, wParam, lParam);
		Compat::LogLeave("callWndRetProc", nCode, wParam, ret) << result;
		return result;
	}

	void disableDwmAttributes(HWND hwnd)
	{
		DWMNCRENDERINGPOLICY ncRenderingPolicy = DWMNCRP_DISABLED;
		DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY,
			&ncRenderingPolicy, sizeof(ncRenderingPolicy));

		BOOL disableTransitions = TRUE;
		DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED,
			&disableTransitions, sizeof(disableTransitions));
	}

	void CALLBACK objectStateChangeEvent(
		HWINEVENTHOOK /*hWinEventHook*/,
		DWORD /*event*/,
		HWND hwnd,
		LONG idObject,
		LONG /*idChild*/,
		DWORD /*dwEventThread*/,
		DWORD /*dwmsEventTime*/)
	{
		if (OBJID_TITLEBAR == idObject || OBJID_HSCROLL == idObject || OBJID_VSCROLL == idObject)
		{
			if (!hwnd || !CompatGdi::beginGdiRendering())
			{
				return;
			}

			HDC windowDc = GetWindowDC(hwnd);
			HDC compatDc = CompatGdiDc::getDc(windowDc);
			if (compatDc)
			{
				if (OBJID_TITLEBAR == idObject)
				{
					CompatGdi::TitleBar(hwnd, compatDc).drawButtons();
				}
				else if (OBJID_HSCROLL == idObject)
				{
					CompatGdi::ScrollBar(hwnd, compatDc).drawHorizArrows();
				}
				else if (OBJID_VSCROLL == idObject)
				{
					CompatGdi::ScrollBar(hwnd, compatDc).drawVertArrows();
				}
				CompatGdiDc::releaseDc(windowDc);
			}

			ReleaseDC(hwnd, windowDc);
			CompatGdi::endGdiRendering();
		}
	}

	void onActivate(HWND hwnd)
	{
		if (!CompatGdi::isEmulationEnabled())
		{
			return;
		}

		RECT windowRect = {};
		GetWindowRect(hwnd, &windowRect);
		RECT clientRect = {};
		GetClientRect(hwnd, &clientRect);
		POINT clientOrigin = {};
		ClientToScreen(hwnd, &clientOrigin);
		OffsetRect(&windowRect, -clientOrigin.x, -clientOrigin.y);

		HRGN ncRgn = CreateRectRgnIndirect(&windowRect);
		HRGN clientRgn = CreateRectRgnIndirect(&clientRect);
		CombineRgn(ncRgn, ncRgn, clientRgn, RGN_DIFF);
		RedrawWindow(hwnd, nullptr, ncRgn, RDW_FRAME | RDW_INVALIDATE);
		DeleteObject(clientRgn);
		DeleteObject(ncRgn);
	}

	void onMenuSelect()
	{
		if (!CompatGdi::isEmulationEnabled())
		{
			return;
		}

		HWND menuWindow = FindWindow(reinterpret_cast<LPCSTR>(0x8000), nullptr);
		while (menuWindow)
		{
			RedrawWindow(menuWindow, nullptr, nullptr, RDW_INVALIDATE);
			menuWindow = FindWindowEx(nullptr, menuWindow, reinterpret_cast<LPCSTR>(0x8000), nullptr);
		}
	}

	void onWindowPosChanged(HWND hwnd)
	{
		Compat::ScopedCriticalSection lock(CompatGdi::g_gdiCriticalSection);

		const auto it = g_prevWindowRect.find(hwnd);
		if (it != g_prevWindowRect.end())
		{
			CompatGdi::invalidate(&it->second);
		}

		if (IsWindowVisible(hwnd))
		{
			if (CompatGdi::isEmulationEnabled())
			{
				GetWindowRect(hwnd, it != g_prevWindowRect.end() ? &it->second : &g_prevWindowRect[hwnd]);
				RedrawWindow(hwnd, nullptr, nullptr, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
			}
		}
		else if (it != g_prevWindowRect.end())
		{
			g_prevWindowRect.erase(it);
		}
	}

	void removeDropShadow(HWND hwnd)
	{
		const auto style = GetClassLongPtr(hwnd, GCL_STYLE);
		if (style & CS_DROPSHADOW)
		{
			SetClassLongPtr(hwnd, GCL_STYLE, style ^ CS_DROPSHADOW);
		}
	}
}

namespace CompatGdiWinProc
{
	void installHooks()
	{
		const DWORD threadId = GetCurrentThreadId();
		g_callWndRetProcHook = SetWindowsHookEx(WH_CALLWNDPROCRET, callWndRetProc, nullptr, threadId);
		g_objectStateChangeEventHook = SetWinEventHook(EVENT_OBJECT_STATECHANGE, EVENT_OBJECT_STATECHANGE,
			nullptr, &objectStateChangeEvent, 0, threadId, WINEVENT_OUTOFCONTEXT);
	}

	void uninstallHooks()
	{
		UnhookWinEvent(g_objectStateChangeEventHook);
		UnhookWindowsHookEx(g_callWndRetProcHook);
	}
}
