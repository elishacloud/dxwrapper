#pragma once

#include <Windows.h>

const LONG DCX_USESTYLE = 0x10000;

namespace Gdi
{
	const ATOM MENU_ATOM = 0x8000;

	typedef void(*WindowPosChangeNotifyFunc)();

	void dllThreadDetach();
	void installHooks();
	bool isDisplayDc(HDC dc);
	void redraw(HRGN rgn);
	void redrawWindow(HWND hwnd, HRGN rgn);
	void unhookWndProc(LPCSTR className, WNDPROC oldWndProc);
	void watchWindowPosChanges(WindowPosChangeNotifyFunc notifyFunc);
};
