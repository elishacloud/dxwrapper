#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace Gdi
{
	typedef void(*WindowPosChangeNotifyFunc)(HWND, const RECT&, const RECT&);

	bool beginGdiRendering(DWORD lockFlags = 0);
	void endGdiRendering();

	void disableEmulation();
	void enableEmulation();

	void hookWndProc(LPCSTR className, WNDPROC &oldWndProc, WNDPROC newWndProc);
	void installHooks();
	bool isEmulationEnabled();
	void redraw(HRGN rgn);
	void redrawWindow(HWND hwnd, HRGN rgn);
	void unhookWndProc(LPCSTR className, WNDPROC oldWndProc);
	void uninstallHooks();
	void updatePalette(DWORD startingEntry, DWORD count);
	void watchWindowPosChanges(WindowPosChangeNotifyFunc notifyFunc);

	extern CRITICAL_SECTION g_gdiCriticalSection;
};
