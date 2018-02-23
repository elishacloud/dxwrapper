#define WIN32_LEAN_AND_MEAN

#include <memory>
#include <set>
#include <unordered_map>

#include <dwmapi.h>
#include <Windows.h>

#include "DDrawLog.h"
#include "Common/ScopedCriticalSection.h"
#include "Gdi/Dc.h"
#include "Gdi/ScrollBar.h"
#include "Gdi/ScrollFunctions.h"
#include "Gdi/TitleBar.h"
#include "Gdi/WinProc.h"

namespace
{
	struct WindowData
	{
		RECT wndRect;
		std::shared_ptr<HRGN__> sysClipRgn;
	};

	HHOOK g_callWndRetProcHook = nullptr;
	HWINEVENTHOOK g_objectStateChangeEventHook = nullptr;
	std::unordered_map<HWND, WindowData> g_windowData;
	std::set<Gdi::WindowPosChangeNotifyFunc> g_windowPosChangeNotifyFuncs;

	void disableDwmAttributes(HWND hwnd);
	void onActivate(HWND hwnd);
	void onMenuSelect();
	void onWindowPosChanged(HWND hwnd);
	void redrawChangedWindowRegion(HWND hwnd, const WindowData& prevData, const WindowData& data);
	void redrawUncoveredRegion(const WindowData& prevData, const WindowData& data);
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
				Compat::ScopedCriticalSection lock(Gdi::g_gdiCriticalSection);
				g_windowData.erase(ret->hwnd);
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
					Gdi::ScrollFunctions::updateScrolledWindow(reinterpret_cast<HWND>(ret->lParam));
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

	WindowData getWindowData(HWND hwnd)
	{
		WindowData data;
		if (IsWindowVisible(hwnd))
		{
			GetWindowRect(hwnd, &data.wndRect);
			data.sysClipRgn.reset(CreateRectRgnIndirect(&data.wndRect), DeleteObject);

			HDC dc = GetWindowDC(hwnd);
			GetRandomRgn(dc, data.sysClipRgn.get(), SYSRGN);
			ReleaseDC(hwnd, dc);
		}
		return data;
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
			if (!hwnd || !Gdi::beginGdiRendering())
			{
				return;
			}

			HDC windowDc = GetWindowDC(hwnd);
			HDC compatDc = Gdi::Dc::getDc(windowDc);
			if (compatDc)
			{
				if (OBJID_TITLEBAR == idObject)
				{
					Gdi::TitleBar(hwnd, compatDc).drawButtons();
				}
				else if (OBJID_HSCROLL == idObject)
				{
					Gdi::ScrollBar(hwnd, compatDc).drawHorizArrows();
				}
				else if (OBJID_VSCROLL == idObject)
				{
					Gdi::ScrollBar(hwnd, compatDc).drawVertArrows();
				}
				Gdi::Dc::releaseDc(windowDc);
			}

			ReleaseDC(hwnd, windowDc);
			Gdi::endGdiRendering();
		}
	}

	void onActivate(HWND hwnd)
	{
		if (!Gdi::isEmulationEnabled())
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
		if (!Gdi::isEmulationEnabled())
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
		if (GetAncestor(hwnd, GA_ROOT) != hwnd)
		{
			return;
		}

		Compat::ScopedCriticalSection lock(Gdi::g_gdiCriticalSection);

		WindowData prevData = g_windowData[hwnd];
		WindowData data = getWindowData(hwnd);
		g_windowData[hwnd] = data;

		for (auto notifyFunc : g_windowPosChangeNotifyFuncs)
		{
			notifyFunc(hwnd, prevData.wndRect, data.wndRect);
		}

		if (!prevData.sysClipRgn && !data.sysClipRgn || !Gdi::isEmulationEnabled())
		{
			return;
		}

		redrawUncoveredRegion(prevData, data);
		redrawChangedWindowRegion(hwnd, prevData, data);
	}

	void redrawChangedWindowRegion(HWND hwnd, const WindowData& prevData, const WindowData& data)
	{
		if (!data.sysClipRgn)
		{
			return;
		}

		if (!prevData.sysClipRgn)
		{
			Gdi::redrawWindow(hwnd, data.sysClipRgn.get());
			return;
		}

		if (EqualRect(&prevData.wndRect, &data.wndRect))
		{
			HRGN rgn = CreateRectRgn(0, 0, 0, 0);
			CombineRgn(rgn, data.sysClipRgn.get(), prevData.sysClipRgn.get(), RGN_DIFF);
			Gdi::redrawWindow(hwnd, rgn);
			DeleteObject(rgn);
		}
		else
		{
			Gdi::redrawWindow(hwnd, data.sysClipRgn.get());
		}
	}

	void redrawUncoveredRegion(const WindowData& prevData, const WindowData& data)
	{
		if (!prevData.sysClipRgn)
		{
			return;
		}

		if (!data.sysClipRgn)
		{
			Gdi::redraw(prevData.sysClipRgn.get());
			return;
		}

		HRGN rgn = CreateRectRgn(0, 0, 0, 0);
		CombineRgn(rgn, prevData.sysClipRgn.get(), data.sysClipRgn.get(), RGN_DIFF);
		Gdi::redraw(rgn);
		DeleteObject(rgn);
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

namespace Gdi
{
	namespace WinProc
	{
		void installHooks()
		{
			const DWORD threadId = GetCurrentThreadId();
			g_callWndRetProcHook = SetWindowsHookEx(WH_CALLWNDPROCRET, callWndRetProc, nullptr, threadId);
			g_objectStateChangeEventHook = SetWinEventHook(EVENT_OBJECT_STATECHANGE, EVENT_OBJECT_STATECHANGE,
				nullptr, &objectStateChangeEvent, 0, threadId, WINEVENT_OUTOFCONTEXT);
		}

		void watchWindowPosChanges(WindowPosChangeNotifyFunc notifyFunc)
		{
			g_windowPosChangeNotifyFuncs.insert(notifyFunc);
		}

		void uninstallHooks()
		{
			UnhookWinEvent(g_objectStateChangeEventHook);
			UnhookWindowsHookEx(g_callWndRetProcHook);
		}
	}
}
