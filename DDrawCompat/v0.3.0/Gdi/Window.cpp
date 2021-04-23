#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <algorithm>
#include <map>
#include <vector>

#include <dwmapi.h>

#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.0/D3dDdi/KernelModeThunks.h>
#include <DDrawCompat/v0.3.0/D3dDdi/ScopedCriticalSection.h>
#include <DDrawCompat/v0.3.0/DDraw/RealPrimarySurface.h>
#include <DDrawCompat/v0.3.0/Gdi/PresentationWindow.h>
#include <DDrawCompat/v0.3.0/Gdi/VirtualScreen.h>
#include <DDrawCompat/v0.3.0/Gdi/Window.h>

namespace
{
	struct UpdateWindowContext
	{
		Gdi::Region obscuredRegion;
		Gdi::Region invalidatedRegion;
		Gdi::Region virtualScreenRegion;
		DWORD processId;
	};

	struct Window
	{
		HWND hwnd;
		HWND presentationWindow;
		RECT windowRect;
		RECT clientRect;
		Gdi::Region windowRegion;
		Gdi::Region visibleRegion;
		Gdi::Region invalidatedRegion;
		COLORREF colorKey;
		BYTE alpha;
		bool isLayered;
		bool isVisibleRegionChanged;

		Window(HWND hwnd)
			: hwnd(hwnd)
			, presentationWindow(nullptr)
			, windowRect{}
			, clientRect{}
			, windowRegion(nullptr)
			, colorKey(CLR_INVALID)
			, alpha(255)
			, isLayered(true)
			, isVisibleRegionChanged(false)
		{
		}
	};

	const RECT REGION_OVERRIDE_MARKER_RECT = { 32000, 32000, 32001, 32001 };

	std::map<HWND, Window> g_windows;
	std::vector<Window*> g_windowZOrder;

	std::map<HWND, Window>::iterator addWindow(HWND hwnd)
	{
		DWMNCRENDERINGPOLICY ncRenderingPolicy = DWMNCRP_DISABLED;
		DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &ncRenderingPolicy, sizeof(ncRenderingPolicy));

		BOOL disableTransitions = TRUE;
		DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disableTransitions, sizeof(disableTransitions));

		const auto style = GetClassLong(hwnd, GCL_STYLE);
		if (style & CS_DROPSHADOW)
		{
			CALL_ORIG_FUNC(SetClassLongA)(hwnd, GCL_STYLE, style & ~CS_DROPSHADOW);
		}

		return g_windows.emplace(hwnd, Window(hwnd)).first;
	}

	bool bltWindow(const RECT& dst, const RECT& src, const Gdi::Region& clipRegion)
	{
		if (dst.left == src.left && dst.top == src.top || clipRegion.isEmpty())
		{
			return false;
		}

		HDC screenDc = GetDC(nullptr);
		SelectClipRgn(screenDc, clipRegion);
		BitBlt(screenDc, dst.left, dst.top, src.right - src.left, src.bottom - src.top, screenDc, src.left, src.top, SRCCOPY);
		SelectClipRgn(screenDc, nullptr);
		ReleaseDC(nullptr, screenDc);
		return true;
	}

	Gdi::Region getWindowRegion(HWND hwnd)
	{
		Gdi::Region rgn;
		if (ERROR == CALL_ORIG_FUNC(GetWindowRgn)(hwnd, rgn))
		{
			return nullptr;
		}
		return rgn;
	}

	void updatePosition(Window& window, const RECT& oldWindowRect, const RECT& oldClientRect,
		const Gdi::Region& oldVisibleRegion, Gdi::Region& invalidatedRegion)
	{
		const bool isClientOriginChanged =
			window.clientRect.left - window.windowRect.left != oldClientRect.left - oldWindowRect.left ||
			window.clientRect.top - window.windowRect.top != oldClientRect.top - oldWindowRect.top;
		const bool isClientWidthChanged =
			window.clientRect.right - window.clientRect.left != oldClientRect.right - oldClientRect.left;
		const bool isClientHeightChanged =
			window.clientRect.bottom - window.clientRect.top != oldClientRect.bottom - oldClientRect.top;
		const bool isClientInvalidated =
			isClientWidthChanged && (GetClassLong(window.hwnd, GCL_STYLE) & CS_HREDRAW) ||
			isClientHeightChanged && (GetClassLong(window.hwnd, GCL_STYLE) & CS_VREDRAW);
		const bool isFrameInvalidated = isClientOriginChanged || isClientWidthChanged || isClientHeightChanged ||
			window.windowRect.right - window.windowRect.left != oldWindowRect.right - oldWindowRect.left ||
			window.windowRect.bottom - window.windowRect.top != oldWindowRect.bottom - oldWindowRect.top;

		Gdi::Region preservedRegion;
		if (!isClientInvalidated || !isFrameInvalidated)
		{
			preservedRegion = oldVisibleRegion - invalidatedRegion;
			preservedRegion.offset(window.windowRect.left - oldWindowRect.left, window.windowRect.top - oldWindowRect.top);
			preservedRegion &= window.visibleRegion;

			if (isClientInvalidated)
			{
				preservedRegion -= window.clientRect;
			}
			else
			{
				if (isFrameInvalidated)
				{
					preservedRegion &= window.clientRect;
				}

				if (isClientWidthChanged)
				{
					RECT r = window.clientRect;
					r.left += oldClientRect.right - oldClientRect.left;
					if (r.left < r.right)
					{
						preservedRegion -= r;
					}
				}
				if (isClientHeightChanged)
				{
					RECT r = window.clientRect;
					r.top += oldClientRect.bottom - oldClientRect.top;
					if (r.top < r.bottom)
					{
						preservedRegion -= r;
					}
				}

				Gdi::Region updateRegion;
				GetUpdateRgn(window.hwnd, updateRegion, FALSE);
				updateRegion.offset(window.clientRect.left, window.clientRect.top);
				preservedRegion -= updateRegion;
			}

			bool isCopied = false;
			if (!isFrameInvalidated)
			{
				isCopied = bltWindow(window.windowRect, oldWindowRect, preservedRegion);
			}
			else
			{
				isCopied = bltWindow(window.clientRect, oldClientRect, preservedRegion);
			}

			if (isCopied)
			{
				invalidatedRegion |= preservedRegion;
			}
		}

		window.invalidatedRegion = window.visibleRegion - preservedRegion;
		if (!window.invalidatedRegion.isEmpty())
		{
			window.invalidatedRegion.offset(-window.clientRect.left, -window.clientRect.top);
		}
	}

	BOOL CALLBACK updateWindow(HWND hwnd, LPARAM lParam)
	{
		auto& context = *reinterpret_cast<UpdateWindowContext*>(lParam);

		DWORD processId = 0;
		GetWindowThreadProcessId(hwnd, &processId);
		if (processId != context.processId ||
			Gdi::PresentationWindow::isPresentationWindow(hwnd))
		{
			return TRUE;
		}

		auto it = g_windows.find(hwnd);
		if (it == g_windows.end())
		{
			it = addWindow(hwnd);
		}
		g_windowZOrder.push_back(&it->second);

		const LONG exStyle = CALL_ORIG_FUNC(GetWindowLongA)(hwnd, GWL_EXSTYLE);
		const bool isLayered = exStyle & WS_EX_LAYERED;
		const bool isVisible = IsWindowVisible(hwnd) && !IsIconic(hwnd);
		bool setPresentationWindowRgn = false;

		if (isLayered != it->second.isLayered)
		{
			it->second.isLayered = isLayered;
			it->second.isVisibleRegionChanged = isVisible;
			if (!isLayered)
			{
				it->second.presentationWindow = Gdi::PresentationWindow::create(hwnd);
				setPresentationWindowRgn = true;
			}
			else if (it->second.presentationWindow)
			{
				Gdi::PresentationWindow::destroy(it->second.presentationWindow);
				it->second.presentationWindow = nullptr;
			}
		}

		Gdi::Region windowRegion(getWindowRegion(hwnd));
		if (windowRegion && !PtInRegion(windowRegion, REGION_OVERRIDE_MARKER_RECT.left, REGION_OVERRIDE_MARKER_RECT.top) ||
			!windowRegion && it->second.windowRegion)
		{
			swap(it->second.windowRegion, windowRegion);
			setPresentationWindowRgn = true;
		}

		WINDOWINFO wi = {};
		Gdi::Region visibleRegion;

		if (isVisible)
		{
			wi.cbSize = sizeof(wi);
			GetWindowInfo(hwnd, &wi);
			if (!IsRectEmpty(&wi.rcWindow))
			{
				if (it->second.windowRegion)
				{
					visibleRegion = it->second.windowRegion;
					visibleRegion.offset(wi.rcWindow.left, wi.rcWindow.top);
				}
				else
				{
					visibleRegion = wi.rcWindow;
				}
				visibleRegion &= context.virtualScreenRegion;
				visibleRegion -= context.obscuredRegion;

				if (!isLayered && !(exStyle & WS_EX_TRANSPARENT))
				{
					context.obscuredRegion |= visibleRegion;
				}
			}
		}

		std::swap(it->second.windowRect, wi.rcWindow);
		std::swap(it->second.clientRect, wi.rcClient);
		swap(it->second.visibleRegion, visibleRegion);

		if (!isLayered)
		{
			if (!it->second.visibleRegion.isEmpty())
			{
				updatePosition(it->second, wi.rcWindow, wi.rcClient, visibleRegion, context.invalidatedRegion);
			}

			if (exStyle & WS_EX_TRANSPARENT)
			{
				context.invalidatedRegion |= visibleRegion - it->second.visibleRegion;
			}

			if (isVisible && !it->second.isVisibleRegionChanged)
			{
				visibleRegion.offset(it->second.windowRect.left - wi.rcWindow.left, it->second.windowRect.top - wi.rcWindow.top);

				if (it->second.visibleRegion != visibleRegion)
				{
					it->second.isVisibleRegionChanged = true;
				}
			}

			if (it->second.presentationWindow)
			{
				if (setPresentationWindowRgn)
				{
					Gdi::PresentationWindow::setWindowRgn(it->second.presentationWindow, it->second.windowRegion);
				}

				WINDOWPOS wp = {};
				wp.flags = SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_NOSENDCHANGING;
				if (isVisible)
				{
					wp.hwndInsertAfter = GetWindow(hwnd, GW_HWNDPREV);
					if (!wp.hwndInsertAfter)
					{
						wp.hwndInsertAfter = (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) ? HWND_TOPMOST : HWND_TOP;
					}
					else if (wp.hwndInsertAfter == it->second.presentationWindow)
					{
						wp.flags |= SWP_NOZORDER;
					}

					wp.x = it->second.windowRect.left;
					wp.y = it->second.windowRect.top;
					wp.cx = it->second.windowRect.right - it->second.windowRect.left;
					wp.cy = it->second.windowRect.bottom - it->second.windowRect.top;
					wp.flags |= SWP_SHOWWINDOW;
				}
				else
				{
					wp.flags |= SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER;
				}

				Gdi::PresentationWindow::setWindowPos(it->second.presentationWindow, wp);
			}
		}
		return TRUE;
	}
}

namespace Gdi
{
	namespace Window
	{
		void onStyleChanged(HWND hwnd, WPARAM wParam)
		{
			if (GWL_EXSTYLE == wParam)
			{
				D3dDdi::ScopedCriticalSection lock;
				auto it = g_windows.find(hwnd);
				if (it != g_windows.end())
				{
					const bool isLayered = GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED;
					if (isLayered != it->second.isLayered)
					{
						updateAll();
					}
				}
			}
		}

		void onSyncPaint(HWND hwnd)
		{
			LOG_FUNC("Window::onSyncPaint", hwnd);
			bool isInvalidated = false;

			{
				D3dDdi::ScopedCriticalSection lock;
				auto it = g_windows.find(hwnd);
				if (it == g_windows.end())
				{
					return;
				}

				if (it->second.isVisibleRegionChanged)
				{
					it->second.isVisibleRegionChanged = false;
					const LONG origWndProc = CALL_ORIG_FUNC(GetWindowLongA)(hwnd, GWL_WNDPROC);
					CALL_ORIG_FUNC(SetWindowLongA)(hwnd, GWL_WNDPROC, reinterpret_cast<LONG>(CALL_ORIG_FUNC(DefWindowProcA)));
					if (it->second.isLayered)
					{
						SetWindowRgn(hwnd, Gdi::Region(it->second.windowRegion).release(), FALSE);
					}
					else
					{
						Gdi::Region rgn(it->second.visibleRegion);
						rgn.offset(-it->second.windowRect.left, -it->second.windowRect.top);
						rgn |= REGION_OVERRIDE_MARKER_RECT;
						SetWindowRgn(hwnd, rgn, FALSE);
					}
					CALL_ORIG_FUNC(SetWindowLongA)(hwnd, GWL_WNDPROC, origWndProc);
				}

				isInvalidated = !it->second.invalidatedRegion.isEmpty();
				if (isInvalidated)
				{
					RedrawWindow(hwnd, nullptr, it->second.invalidatedRegion,
						RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
					it->second.invalidatedRegion = nullptr;
				}
			}

			if (isInvalidated)
			{
				RECT emptyRect = {};
				RedrawWindow(hwnd, &emptyRect, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ERASENOW);
			}
		}

		void present(CompatRef<IDirectDrawSurface7> dst, CompatRef<IDirectDrawSurface7> src,
			CompatRef<IDirectDrawClipper> clipper)
		{
			D3dDdi::ScopedCriticalSection lock;
			for (auto window : g_windowZOrder)
			{
				if (window->presentationWindow && !window->visibleRegion.isEmpty())
				{
					clipper->SetHWnd(&clipper, 0, window->presentationWindow);
					dst->Blt(&dst, nullptr, &src, nullptr, DDBLT_WAIT, nullptr);
				}
			}
		}

		void present(Gdi::Region excludeRegion)
		{
			D3dDdi::ScopedCriticalSection lock;
			std::unique_ptr<HDC__, void(*)(HDC)> virtualScreenDc(nullptr, &Gdi::VirtualScreen::deleteDc);
			RECT virtualScreenBounds = Gdi::VirtualScreen::getBounds();

			for (auto window : g_windowZOrder)
			{
				if (!window->presentationWindow)
				{
					continue;
				}

				Gdi::Region visibleRegion(window->visibleRegion);
				if (excludeRegion)
				{
					visibleRegion -= excludeRegion;
				}
				if (visibleRegion.isEmpty())
				{
					continue;
				}

				if (!virtualScreenDc)
				{
					const bool useDefaultPalette = false;
					virtualScreenDc.reset(Gdi::VirtualScreen::createDc(useDefaultPalette));
					if (!virtualScreenDc)
					{
						return;
					}
				}

				HDC dc = GetWindowDC(window->presentationWindow);
				RECT rect = window->windowRect;
				visibleRegion.offset(-rect.left, -rect.top);
				SelectClipRgn(dc, visibleRegion);
				CALL_ORIG_FUNC(BitBlt)(dc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, virtualScreenDc.get(),
					rect.left - virtualScreenBounds.left, rect.top - virtualScreenBounds.top, SRCCOPY);
				CALL_ORIG_FUNC(ReleaseDC)(window->presentationWindow, dc);
			}
		}

		void presentLayered(CompatRef<IDirectDrawSurface7> dst, POINT offset)
		{
			D3dDdi::ScopedCriticalSection lock;

			HDC dstDc = nullptr;
			for (auto it = g_windowZOrder.rbegin(); it != g_windowZOrder.rend(); ++it)
			{
				auto& window = **it;
				if (!window.isLayered)
				{
					continue;
				}

				if (!dstDc)
				{
					const UINT D3DDDIFMT_UNKNOWN = 0;
					const UINT D3DDDIFMT_X8R8G8B8 = 22;
					D3dDdi::KernelModeThunks::setDcFormatOverride(D3DDDIFMT_X8R8G8B8);
					dst->GetDC(&dst, &dstDc);
					D3dDdi::KernelModeThunks::setDcFormatOverride(D3DDDIFMT_UNKNOWN);
					if (!dstDc)
					{
						return;
					}
				}

				HDC windowDc = GetWindowDC(window.hwnd);
				Gdi::Region rgn(window.visibleRegion);
				RECT wr = window.windowRect;

				if (0 != offset.x || 0 != offset.y)
				{
					OffsetRect(&wr, offset.x, offset.y);
					rgn.offset(offset.x, offset.y);
				}

				SelectClipRgn(dstDc, rgn);

				auto colorKey = window.colorKey;
				if (CLR_INVALID != colorKey)
				{
					CALL_ORIG_FUNC(TransparentBlt)(dstDc, wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top,
						windowDc, 0, 0, wr.right - wr.left, wr.bottom - wr.top, colorKey);
				}
				else
				{
					BLENDFUNCTION blend = {};
					blend.SourceConstantAlpha = window.alpha;
					CALL_ORIG_FUNC(AlphaBlend)(dstDc, wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top,
						windowDc, 0, 0, wr.right - wr.left, wr.bottom - wr.top, blend);
				}

				CALL_ORIG_FUNC(ReleaseDC)(window.hwnd, windowDc);
			}

			if (dstDc)
			{
				SelectClipRgn(dstDc, nullptr);
				dst->ReleaseDC(&dst, dstDc);
			}
		}

		void updateAll()
		{
			LOG_FUNC("Window::updateAll");
			if (!Gdi::PresentationWindow::isThreadReady())
			{
				return;
			}

			UpdateWindowContext context;
			context.processId = GetCurrentProcessId();
			context.virtualScreenRegion = VirtualScreen::getRegion();
			std::vector<HWND> invalidatedWindows;

			{
				D3dDdi::ScopedCriticalSection lock;
				g_windowZOrder.clear();
				EnumWindows(updateWindow, reinterpret_cast<LPARAM>(&context));

				for (auto it = g_windows.begin(); it != g_windows.end();)
				{
					if (g_windowZOrder.end() == std::find(g_windowZOrder.begin(), g_windowZOrder.end(), &it->second))
					{
						if (it->second.presentationWindow)
						{
							Gdi::PresentationWindow::destroy(it->second.presentationWindow);
						}
						it = g_windows.erase(it);
					}
					else
					{
						++it;
					}
				}

				for (auto it = g_windowZOrder.rbegin(); it != g_windowZOrder.rend(); ++it)
				{
					auto& window = **it;
					if (window.isVisibleRegionChanged || !window.invalidatedRegion.isEmpty())
					{
						invalidatedWindows.push_back(window.hwnd);
					}
				}
			}

			for (auto hwnd : invalidatedWindows)
			{
				SendNotifyMessage(hwnd, WM_SYNCPAINT, 0, 0);
			}
		}

		void updateLayeredWindowInfo(HWND hwnd, COLORREF colorKey, BYTE alpha)
		{
			D3dDdi::ScopedCriticalSection lock;
			auto it = g_windows.find(hwnd);
			if (it != g_windows.end())
			{
				it->second.colorKey = colorKey;
				it->second.alpha = alpha;
				if (!it->second.visibleRegion.isEmpty())
				{
					DDraw::RealPrimarySurface::scheduleUpdate();
				}
			}
		}
	}
}
