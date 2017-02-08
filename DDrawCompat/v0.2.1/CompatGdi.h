#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace CompatGdi
{
	bool beginGdiRendering();
	void endGdiRendering();

	void disableEmulation();
	void enableEmulation();

	void hookWndProc(LPCSTR className, WNDPROC &oldWndProc, WNDPROC newWndProc);
	void installHooks();
	void invalidate(const RECT* rect);
	bool isEmulationEnabled();
	void unhookWndProc(LPCSTR className, WNDPROC oldWndProc);
	void uninstallHooks();
	void updatePalette(DWORD startingEntry, DWORD count);

	extern CRITICAL_SECTION g_gdiCriticalSection;
};
