#include "Common/Hook.h"
#include "DDrawCompat\DDrawLog.h"
#include "DDraw/RealPrimarySurface.h"
#include "Gdi/Dc.h"
#include "Gdi/Gdi.h"
#include "Gdi/PaintHandlers.h"
#include "Gdi/ScrollBar.h"
#include "Gdi/ScrollFunctions.h"
#include "Gdi/TitleBar.h"
#include "Win32/Registry.h"

namespace
{
	LRESULT WINAPI defPaintProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
		WNDPROC origWndProc, const char* origWndProcName);
	LRESULT onEraseBackground(HWND hwnd, HDC dc, WNDPROC origWndProc);
	LRESULT onMenuPaint(HWND hwnd, WNDPROC origWndProc);
	LRESULT onNcPaint(HWND hwnd, WPARAM wParam, WNDPROC origWndProc);
	LRESULT onPaint(HWND hwnd, WNDPROC origWndProc);
	LRESULT onPrint(HWND hwnd, UINT msg, HDC dc, LONG flags, WNDPROC origWndProc);

	WNDPROC g_origButtonWndProc = nullptr;
	WNDPROC g_origComboListBoxWndProc = nullptr;
	WNDPROC g_origEditWndProc = nullptr;
	WNDPROC g_origListBoxWndProc = nullptr;
	WNDPROC g_origMenuWndProc = nullptr;
	WNDPROC g_origScrollBarWndProc = nullptr;

	LRESULT WINAPI buttonWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		Compat::LogEnter("buttonWndProc", hwnd, msg, wParam, lParam);
		LRESULT result = 0;

		switch (msg)
		{
		case WM_PAINT:
			result = onPaint(hwnd, g_origButtonWndProc);
			break;

		case WM_ENABLE:
		case WM_SETTEXT:
		case BM_SETCHECK:
		case BM_SETSTATE:
			result = CallWindowProc(g_origButtonWndProc, hwnd, msg, wParam, lParam);
			RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
			break;

		default:
			result = CallWindowProc(g_origButtonWndProc, hwnd, msg, wParam, lParam);
			break;
		}

		Compat::LogLeave("buttonWndProc", hwnd, msg, wParam, lParam) << result;
		return result;
	}

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
		Compat::LogEnter(origWndProcName, hwnd, msg, wParam, lParam);
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
			result = CallWindowProc(origWndProc, hwnd, msg, wParam, lParam);
			break;
		}

		Compat::LogLeave(origWndProcName, hwnd, msg, wParam, lParam) << result;
		return result;
	}

	LRESULT WINAPI defWindowProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return defPaintProc(hwnd, msg, wParam, lParam, CALL_ORIG_FUNC(DefWindowProcA), "defWindowProcA");
	}

	LRESULT WINAPI defWindowProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (WM_CREATE == msg)
		{
			auto className = reinterpret_cast<CREATESTRUCTW*>(lParam)->lpszClass;
			if (reinterpret_cast<DWORD>(className) > 0xFFFF &&
				0 == lstrcmpW(className, L"CompatWindowDesktopReplacement"))
			{
				return -1;
			}
		}
		return defPaintProc(hwnd, msg, wParam, lParam, CALL_ORIG_FUNC(DefWindowProcW), "defWindowProcW");
	}

	void disableImmersiveContextMenus()
	{
		// Immersive context menus don't display properly (empty items) when theming is disabled
		Win32::Registry::setValue(
			HKEY_LOCAL_MACHINE,
			"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FlightedFeatures",
			"ImmersiveContextMenu",
			0);

		// An update in Windows 10 seems to have moved the key from the above location
		Win32::Registry::setValue(
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
			Gdi::ScrollFunctions::updateScrolledWindow(hwnd);
		}
		return result;
	}

	LRESULT WINAPI listBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return defPaintProc(hwnd, msg, wParam, lParam, g_origListBoxWndProc, "listBoxWndProc");
	}

	LRESULT WINAPI menuWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		Compat::LogEnter("menuWndProc", hwnd, msg, wParam, lParam);
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
			result = CallWindowProc(g_origMenuWndProc, hwnd, msg, wParam, lParam);
			break;
		}

		Compat::LogLeave("menuWndProc", hwnd, msg, wParam, lParam) << result;
		return result;
	}

	LRESULT onEraseBackground(HWND hwnd, HDC dc, WNDPROC origWndProc)
	{
		if (hwnd && Gdi::beginGdiRendering())
		{
			LRESULT result = 0;
			HDC compatDc = Gdi::Dc::getDc(dc);
			if (compatDc)
			{
				result = CallWindowProc(origWndProc, hwnd, WM_ERASEBKGND, reinterpret_cast<WPARAM>(compatDc), 0);
				Gdi::Dc::releaseDc(dc);
			}

			Gdi::endGdiRendering();
			if (compatDc)
			{
				return result;
			}
		}

		return CallWindowProc(origWndProc, hwnd, WM_ERASEBKGND, reinterpret_cast<WPARAM>(dc), 0);
	}

	LRESULT onMenuPaint(HWND hwnd, WNDPROC origWndProc)
	{
		if (!hwnd || !Gdi::beginGdiRendering())
		{
			return CallWindowProc(origWndProc, hwnd, WM_PAINT, 0, 0);
		}

		HDC dc = GetWindowDC(hwnd);
		HDC compatDc = Gdi::Dc::getDc(dc);
		if (compatDc)
		{
			CallWindowProc(origWndProc, hwnd, WM_PRINT, reinterpret_cast<WPARAM>(compatDc),
				PRF_NONCLIENT | PRF_ERASEBKGND | PRF_CLIENT);
			ValidateRect(hwnd, nullptr);
			Gdi::Dc::releaseDc(dc);
		}
		else
		{
			CallWindowProc(origWndProc, hwnd, WM_PAINT, 0, 0);
		}

		ReleaseDC(hwnd, dc);
		Gdi::endGdiRendering();
		return 0;
	}

	LRESULT onNcPaint(HWND hwnd, WPARAM wParam, WNDPROC origWndProc)
	{
		if (!hwnd || !Gdi::beginGdiRendering())
		{
			return CallWindowProc(origWndProc, hwnd, WM_NCPAINT, wParam, 0);
		}

		HDC windowDc = GetWindowDC(hwnd);
		HDC compatDc = Gdi::Dc::getDc(windowDc);

		if (compatDc)
		{
			Gdi::TitleBar titleBar(hwnd, compatDc);
			titleBar.drawAll();
			titleBar.excludeFromClipRegion();

			Gdi::ScrollBar scrollBar(hwnd, compatDc);
			scrollBar.drawAll();
			scrollBar.excludeFromClipRegion();

			SendMessage(hwnd, WM_PRINT, reinterpret_cast<WPARAM>(compatDc), PRF_NONCLIENT);

			Gdi::Dc::releaseDc(windowDc);
		}

		ReleaseDC(hwnd, windowDc);
		Gdi::endGdiRendering();
		return 0;
	}

	LRESULT onPaint(HWND hwnd, WNDPROC origWndProc)
	{
		if (!hwnd || !Gdi::beginGdiRendering())
		{
			return CallWindowProc(origWndProc, hwnd, WM_PAINT, 0, 0);
		}

		PAINTSTRUCT paint = {};
		HDC dc = BeginPaint(hwnd, &paint);
		HDC compatDc = Gdi::Dc::getDc(dc);

		if (compatDc)
		{
			CallWindowProc(origWndProc, hwnd, WM_PRINTCLIENT,
				reinterpret_cast<WPARAM>(compatDc), PRF_CLIENT);
			Gdi::Dc::releaseDc(dc);
		}
		else
		{
			CallWindowProc(origWndProc, hwnd, WM_PRINTCLIENT, reinterpret_cast<WPARAM>(dc), PRF_CLIENT);
		}

		EndPaint(hwnd, &paint);

		Gdi::endGdiRendering();
		return 0;
	}

	LRESULT onPrint(HWND hwnd, UINT msg, HDC dc, LONG flags, WNDPROC origWndProc)
	{
		if (!Gdi::beginGdiRendering())
		{
			return CallWindowProc(origWndProc, hwnd, msg, reinterpret_cast<WPARAM>(dc), flags);
		}

		LRESULT result = 0;
		HDC compatDc = Gdi::Dc::getDc(dc);
		if (compatDc)
		{
			result = CallWindowProc(origWndProc, hwnd, msg, reinterpret_cast<WPARAM>(compatDc), flags);
			Gdi::Dc::releaseDc(dc);
		}
		else
		{
			result = CallWindowProc(origWndProc, hwnd, msg, reinterpret_cast<WPARAM>(dc), flags);
		}

		Gdi::endGdiRendering();
		return result;
	}

	LRESULT WINAPI scrollBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		Compat::LogEnter("scrollBarWndProc", hwnd, msg, wParam, lParam);
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
			result = CallWindowProc(g_origScrollBarWndProc, hwnd, msg, wParam, lParam);
			break;
		}

		Compat::LogLeave("scrollBarWndProc", hwnd, msg, wParam, lParam) << result;
		return result;
	}
}

namespace Gdi
{
	namespace PaintHandlers
	{
		void installHooks()
		{
			disableImmersiveContextMenus();

			Gdi::hookWndProc("Button", g_origButtonWndProc, &buttonWndProc);
			Gdi::hookWndProc("ComboLBox", g_origComboListBoxWndProc, &comboListBoxWndProc);
			Gdi::hookWndProc("Edit", g_origEditWndProc, &editWndProc);
			Gdi::hookWndProc("ListBox", g_origListBoxWndProc, &listBoxWndProc);
			Gdi::hookWndProc("#32768", g_origMenuWndProc, &menuWndProc);
			Gdi::hookWndProc("ScrollBar", g_origScrollBarWndProc, &scrollBarWndProc);

			HOOK_FUNCTION(user32, DefWindowProcA, defWindowProcA);
			HOOK_FUNCTION(user32, DefWindowProcW, defWindowProcW);
			HOOK_FUNCTION(user32, DefDlgProcA, defDlgProcA);
			HOOK_FUNCTION(user32, DefDlgProcW, defDlgProcW);
		}

		void uninstallHooks()
		{
			Gdi::unhookWndProc("Button", g_origButtonWndProc);
			Gdi::unhookWndProc("ComboLBox", g_origComboListBoxWndProc);
			Gdi::unhookWndProc("Edit", g_origEditWndProc);
			Gdi::unhookWndProc("ListBox", g_origListBoxWndProc);
			Gdi::unhookWndProc("#32768", g_origMenuWndProc);
			Gdi::unhookWndProc("ScrollBar", g_origScrollBarWndProc);
		}
	}
}
