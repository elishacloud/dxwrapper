#pragma once

#include <atomic>

#define WM_APP_CREATE_D3D9_DEVICE (WM_APP + 0xFFF - 0x123)
#define WM_MAKE_KEY(Val1, Val2) \
	(LPARAM)WndProc::MakeKey((DWORD)Val1, (DWORD)Val2)

namespace WndProc
{
	struct DATASTRUCT {
		std::atomic<bool> IsDirectDraw = false;
		std::atomic<bool> IsDirect3D9 = false;
		std::atomic<bool> IsCreatingDevice = false;
		std::atomic<bool> IsExclusiveMode = false;
	};

	DATASTRUCT* AddWndProc(HWND hWnd);
	void RemoveWndProc(HWND hWnd);
	DATASTRUCT* GetWndProctStruct(HWND hWnd);
	DWORD MakeKey(DWORD Val1, DWORD Val2);
	WNDPROC CheckWndProc(HWND hWnd, LONG dwNewLong);
}
