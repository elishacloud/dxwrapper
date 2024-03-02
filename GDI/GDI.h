#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Wrappers\wrapper.h"
#include "Gdi32.h"
#include "User32.h"

namespace WndProc
{
	WNDPROC CheckWndProc(HWND hWnd, LONG dwNewLong);
	bool AddWndProc(HWND hWnd);
	void RemoveWndProc(HWND hWnd);
}
