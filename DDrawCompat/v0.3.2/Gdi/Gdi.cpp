#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.2/DDraw/Surfaces/PrimarySurface.h>
#include <DDrawCompat/v0.3.2/Gdi/Caret.h>
#include <DDrawCompat/v0.3.2/Gdi/Dc.h>
#include <DDrawCompat/v0.3.2/Gdi/DcFunctions.h>
#include <DDrawCompat/v0.3.2/Gdi/Font.h>
#include <DDrawCompat/v0.3.2/Gdi/Gdi.h>
#include <DDrawCompat/v0.3.2/Gdi/Icon.h>
#include <DDrawCompat/v0.3.2/Gdi/Metrics.h>
#include <DDrawCompat/v0.3.2/Gdi/Palette.h>
#include <DDrawCompat/v0.3.2/Gdi/PresentationWindow.h>
#include <DDrawCompat/v0.3.2/Gdi/ScrollFunctions.h>
#include <DDrawCompat/v0.3.2/Gdi/User32WndProcs.h>
#include <DDrawCompat/v0.3.2/Gdi/WinProc.h>

namespace
{
	BOOL CALLBACK redrawWindowCallback(HWND hwnd, LPARAM lParam)
	{
		DWORD windowPid = 0;
		GetWindowThreadProcessId(hwnd, &windowPid);
		if (GetCurrentProcessId() == windowPid)
		{
			Gdi::redrawWindow(hwnd, reinterpret_cast<HRGN>(lParam));
		}
		return TRUE;
	}
}

namespace Gdi
{
	void dllThreadDetach()
	{
		WinProc::dllThreadDetach();
		Dc::dllThreadDetach();
	}

	void installHooks()
	{
		DisableProcessWindowsGhosting();

		DcFunctions::installHooks();
		Icon::installHooks();
		Metrics::installHooks();
		Palette::installHooks();
		PresentationWindow::installHooks();
		ScrollFunctions::installHooks();
		User32WndProcs::installHooks();
		Caret::installHooks();
		Font::installHooks();
	}

	bool isDisplayDc(HDC dc)
	{
		return dc && OBJ_DC == GetObjectType(dc) && DT_RASDISPLAY == CALL_ORIG_FUNC(GetDeviceCaps)(dc, TECHNOLOGY) &&
			!(CALL_ORIG_FUNC(GetWindowLongA)(CALL_ORIG_FUNC(WindowFromDC)(dc), GWL_EXSTYLE) & WS_EX_LAYERED);
	}

	void redraw(HRGN rgn)
	{
		EnumWindows(&redrawWindowCallback, reinterpret_cast<LPARAM>(rgn));
	}

	void redrawWindow(HWND hwnd, HRGN rgn)
	{
		if (!IsWindowVisible(hwnd) || IsIconic(hwnd) || PresentationWindow::isPresentationWindow(hwnd))
		{
			return;
		}

		POINT origin = {};
		if (rgn)
		{
			ClientToScreen(hwnd, &origin);
			OffsetRgn(rgn, -origin.x, -origin.y);
		}

		RedrawWindow(hwnd, nullptr, rgn, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

		if (rgn)
		{
			OffsetRgn(rgn, origin.x, origin.y);
		}
	}

	void unhookWndProc(LPCSTR className, WNDPROC oldWndProc)
	{
		HWND hwnd = CreateWindow(className, nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, 0);
		SetClassLongPtr(hwnd, GCLP_WNDPROC, reinterpret_cast<LONG>(oldWndProc));
		DestroyWindow(hwnd);
	}

	void watchWindowPosChanges(WindowPosChangeNotifyFunc notifyFunc)
	{
		WinProc::watchWindowPosChanges(notifyFunc);
	}
}
