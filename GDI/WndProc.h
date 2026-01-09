#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define WM_APP_CREATE_D3D9_DEVICE (WM_APP + 0xFFF - 0x123)
#define WM_APP_SET_KEYBOARD_LAYOUT (WM_APP + 0xFFF - 0x124)

#define WM_MAKE_KEY(Val1, Val2) \
	(LPARAM)WndProc::MakeKey((DWORD)Val1, (DWORD)Val2)

namespace WndProc
{
	struct DATASTRUCT {
		bool IsDirectDraw = false;
		bool IsDirect3D9 = false;
		bool IsCreatingDevice = false;
		bool IsExclusiveMode = false;
		bool NoWindowChanges = false;
		WORD isActive = WA_INACTIVE;
		DWORD DirectXVersion = 0;
	};

	extern bool SwitchingResolution;

	bool ShouldHook(HWND hWnd);
	DATASTRUCT* AddWndProc(HWND hWnd);
	void RemoveWndProc(HWND hWnd);
	DATASTRUCT* GetWndProctStruct(HWND hWnd);
	DWORD MakeKey(DWORD Val1, DWORD Val2);
	WNDPROC CheckWndProc(HWND hWnd, LONG dwNewLong);
}
