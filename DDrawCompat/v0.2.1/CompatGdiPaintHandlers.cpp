#include "CompatGdi.h"
#include "CompatGdiDc.h"
#include "CompatGdiPaintHandlers.h"
#include "CompatGdiScrollBar.h"
#include "CompatGdiScrollFunctions.h"
#include "CompatGdiTitleBar.h"
#include "CompatPrimarySurface.h"
#include "CompatRegistry.h"
#include "DDrawCompat\DDrawLog.h"
#include "Hook.h"
#include "RealPrimarySurface.h"

namespace Compat21
{
	namespace
	{
		LRESULT WINAPI defPaintProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
			WNDPROC origWndProc, const char* origWndProcName);
		LRESULT onEraseBackground(HWND hwnd, HDC dc, WNDPROC origWndProc);
		LRESULT onMenuPaint(HWND hwnd, WNDPROC origWndProc);
		LRESULT onNcPaint(HWND hwnd, WPARAM wParam, WNDPROC origWndProc);
		LRESULT onPaint(HWND hwnd, WNDPROC origWndProc);
		LRESULT onPrint(HWND hwnd, UINT msg, HDC dc, LONG flags, WNDPROC origWndProc);

		WNDPROC g_origComboListBoxWndProc = nullptr;
		WNDPROC g_origEditWndProc = nullptr;
		WNDPROC g_origListBoxWndProc = nullptr;
		WNDPROC g_origMenuWndProc = nullptr;
		WNDPROC g_origScrollBarWndProc = nullptr;

		LRESULT WINAPI comboListBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			return defPaintProc(hwnd, msg, wParam, lParam, g_origComboListBoxWndProc, "comboListBoxWndProc");
		}

		LRESULT WINAPI defDlgProcA(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			return defPaintProc(hdlg, msg, wParam, lParam, CALL_ORIG_FUNC(DefDlgProcA), "defDlgProcA");
		}

		LRESULT WINAPI defDlgProcW(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			return defPaintProc(hdlg, msg, wParam, lParam, CALL_ORIG_FUNC(DefDlgProcW), "defDlgProcW");
		}

		LRESULT WINAPI defPaintProc(
			HWND hwnd,
			UINT msg,
			WPARAM wParam,
			LPARAM lParam,
			WNDPROC origWndProc,
			const char* origWndProcName)
		{
			Logging::LogEnter(origWndProcName, hwnd, msg, wParam, lParam);
			LRESULT result = 0;

			switch (msg)
			{
			case WM_ERASEBKGND:
				result = onEraseBackground(hwnd, reinterpret_cast<HDC>(wParam), origWndProc);
				break;

			case WM_NCPAINT:
				result = onNcPaint(hwnd, wParam, origWndProc);
				break;

			case WM_PRINT:
			case WM_PRINTCLIENT:
				result = onPrint(hwnd, msg, reinterpret_cast<HDC>(wParam), lParam, origWndProc);
				break;

			default:
				result = origWndProc(hwnd, msg, wParam, lParam);
				break;
			}

			Logging::LogLeave(origWndProcName, hwnd, msg, wParam, lParam) << result;
			return result;
		}

		LRESULT WINAPI defWindowProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			return defPaintProc(hwnd, msg, wParam, lParam, CALL_ORIG_FUNC(DefWindowProcA), "defWindowProcA");
		}

		LRESULT WINAPI defWindowProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			return defPaintProc(hwnd, msg, wParam, lParam, CALL_ORIG_FUNC(DefWindowProcW), "defWindowProcW");
		}

		void disableImmersiveContextMenus()
		{
			// Immersive context menus don't display properly (empty items) when theming is disabled
			CompatRegistry::setValue(
				HKEY_LOCAL_MACHINE,
				"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FlightedFeatures",
				"ImmersiveContextMenu",
				0);

			// An update in Windows 10 seems to have moved the key from the above location
			CompatRegistry::setValue(
				HKEY_LOCAL_MACHINE,
				"Software\\Microsoft\\Windows\\CurrentVersion\\FlightedFeatures",
				"ImmersiveContextMenu",
				0);
		}

		LRESULT WINAPI editWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			LRESULT result = defPaintProc(hwnd, msg, wParam, lParam, g_origEditWndProc, "editWndProc");
			if (0 == result && (WM_HSCROLL == msg || WM_VSCROLL == msg))
			{
				CompatGdiScrollFunctions::updateScrolledWindow(hwnd);
			}
			return result;
		}

		LRESULT WINAPI listBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			return defPaintProc(hwnd, msg, wParam, lParam, g_origListBoxWndProc, "listBoxWndProc");
		}

		LRESULT WINAPI menuWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			Logging::LogEnter("menuWndProc", hwnd, msg, wParam, lParam);
			LRESULT result = 0;

			switch (msg)
			{
			case WM_PAINT:
				result = onMenuPaint(hwnd, g_origMenuWndProc);
				break;

			case 0x1e5:
				if (-1 == wParam)
				{
					// Clearing of selection is not caught by WM_MENUSELECT when mouse leaves menu window
					RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE);
				}
				// fall through to default

			default:
				result = g_origMenuWndProc(hwnd, msg, wParam, lParam);
				break;
			}

			Logging::LogLeave("menuWndProc", hwnd, msg, wParam, lParam) << result;
			return result;
		}

		LRESULT onEraseBackground(HWND hwnd, HDC dc, WNDPROC origWndProc)
		{
			if (!hwnd || !CompatGdi::beginGdiRendering())
			{
				return origWndProc(hwnd, WM_ERASEBKGND, reinterpret_cast<WPARAM>(dc), 0);
			}

			LRESULT result = 0;
			HDC compatDc = CompatGdiDc::getDc(dc);
			if (compatDc)
			{
				result = origWndProc(hwnd, WM_ERASEBKGND, reinterpret_cast<WPARAM>(compatDc), 0);
				CompatGdiDc::releaseDc(dc);
				if (result)
				{
					RealPrimarySurface::disableUpdates();
				}
			}
			else
			{
				result = origWndProc(hwnd, WM_ERASEBKGND, reinterpret_cast<WPARAM>(dc), 0);
			}

			CompatGdi::endGdiRendering();

			if (result && compatDc)
			{
				UpdateWindow(hwnd);
				RealPrimarySurface::enableUpdates();
			}

			return result;
		}

		LRESULT onMenuPaint(HWND hwnd, WNDPROC origWndProc)
		{
			if (!hwnd || !CompatGdi::beginGdiRendering())
			{
				return origWndProc(hwnd, WM_PAINT, 0, 0);
			}

			HDC dc = GetWindowDC(hwnd);
			const bool isMenuPaintDc = true;
			HDC compatDc = CompatGdiDc::getDc(dc, isMenuPaintDc);
			if (compatDc)
			{
				origWndProc(hwnd, WM_PRINT, reinterpret_cast<WPARAM>(compatDc),
					PRF_NONCLIENT | PRF_ERASEBKGND | PRF_CLIENT);
				ValidateRect(hwnd, nullptr);
				CompatGdiDc::releaseDc(dc);
			}
			else
			{
				origWndProc(hwnd, WM_PAINT, 0, 0);
			}

			ReleaseDC(hwnd, dc);
			CompatGdi::endGdiRendering();
			return 0;
		}

		LRESULT onNcPaint(HWND hwnd, WPARAM wParam, WNDPROC origWndProc)
		{
			if (!hwnd || !CompatGdi::beginGdiRendering())
			{
				return origWndProc(hwnd, WM_NCPAINT, wParam, 0);
			}

			HDC windowDc = GetWindowDC(hwnd);
			HDC compatDc = CompatGdiDc::getDc(windowDc);

			if (compatDc)
			{
				CompatGdi::TitleBar titleBar(hwnd, compatDc);
				titleBar.drawAll();
				titleBar.excludeFromClipRegion();

				CompatGdi::ScrollBar scrollBar(hwnd, compatDc);
				scrollBar.drawAll();
				scrollBar.excludeFromClipRegion();

				SendMessage(hwnd, WM_PRINT, reinterpret_cast<WPARAM>(compatDc), PRF_NONCLIENT);

				CompatGdiDc::releaseDc(windowDc);
			}

			ReleaseDC(hwnd, windowDc);
			CompatGdi::endGdiRendering();
			return 0;
		}

		LRESULT onPaint(HWND hwnd, WNDPROC origWndProc)
		{
			if (!hwnd || !CompatGdi::beginGdiRendering())
			{
				return origWndProc(hwnd, WM_PAINT, 0, 0);
			}

			PAINTSTRUCT paint = {};
			HDC dc = BeginPaint(hwnd, &paint);
			HDC compatDc = CompatGdiDc::getDc(dc);

			if (compatDc)
			{
				origWndProc(hwnd, WM_PRINTCLIENT, reinterpret_cast<WPARAM>(compatDc), PRF_CLIENT);
				CompatGdiDc::releaseDc(dc);
			}
			else
			{
				origWndProc(hwnd, WM_PRINTCLIENT, reinterpret_cast<WPARAM>(dc), PRF_CLIENT);
			}

			EndPaint(hwnd, &paint);

			CompatGdi::endGdiRendering();
			return 0;
		}

		LRESULT onPrint(HWND hwnd, UINT msg, HDC dc, LONG flags, WNDPROC origWndProc)
		{
			if (!CompatGdi::beginGdiRendering())
			{
				return origWndProc(hwnd, msg, reinterpret_cast<WPARAM>(dc), flags);
			}

			LRESULT result = 0;
			HDC compatDc = CompatGdiDc::getDc(dc);
			if (compatDc)
			{
				result = origWndProc(hwnd, msg, reinterpret_cast<WPARAM>(compatDc), flags);
				CompatGdiDc::releaseDc(dc);
			}
			else
			{
				result = origWndProc(hwnd, msg, reinterpret_cast<WPARAM>(dc), flags);
			}

			CompatGdi::endGdiRendering();
			return result;
		}

		LRESULT WINAPI scrollBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			Logging::LogEnter("scrollBarWndProc", hwnd, msg, wParam, lParam);
			LRESULT result = 0;

			switch (msg)
			{
			case WM_PAINT:
				result = onPaint(hwnd, g_origScrollBarWndProc);
				break;

			case WM_SETCURSOR:
				if (GetWindowLong(hwnd, GWL_STYLE) & (SBS_SIZEBOX | SBS_SIZEGRIP))
				{
					SetCursor(LoadCursor(nullptr, IDC_SIZENWSE));
				}
				result = TRUE;
				break;

			default:
				result = g_origScrollBarWndProc(hwnd, msg, wParam, lParam);
				break;
			}

			Logging::LogLeave("scrollBarWndProc", hwnd, msg, wParam, lParam) << result;
			return result;
		}
	}

	namespace CompatGdiPaintHandlers
	{
		void installHooks()
		{
			disableImmersiveContextMenus();

			CompatGdi::hookWndProc("ComboLBox", g_origComboListBoxWndProc, &comboListBoxWndProc);
			CompatGdi::hookWndProc("Edit", g_origEditWndProc, &editWndProc);
			CompatGdi::hookWndProc("ListBox", g_origListBoxWndProc, &listBoxWndProc);
			CompatGdi::hookWndProc("#32768", g_origMenuWndProc, &menuWndProc);
			CompatGdi::hookWndProc("ScrollBar", g_origScrollBarWndProc, &scrollBarWndProc);

			HOOK_FUNCTION(user32, DefWindowProcA, defWindowProcA);
			HOOK_FUNCTION(user32, DefWindowProcW, defWindowProcW);
			HOOK_FUNCTION(user32, DefDlgProcA, defDlgProcA);
			HOOK_FUNCTION(user32, DefDlgProcW, defDlgProcW);
		}

		void uninstallHooks()
		{
			CompatGdi::unhookWndProc("ComboLBox", g_origComboListBoxWndProc);
			CompatGdi::unhookWndProc("Edit", g_origEditWndProc);
			CompatGdi::unhookWndProc("ListBox", g_origListBoxWndProc);
			CompatGdi::unhookWndProc("#32768", g_origMenuWndProc);
			CompatGdi::unhookWndProc("ScrollBar", g_origScrollBarWndProc);
		}
	}
}
