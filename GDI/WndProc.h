#pragma once

#include "d3d9\d3d9External.h"
#include <atomic>

#define WM_USER_CREATE_D3D9_DEVICE (WM_APP + 0x1234)

namespace WndProc
{
	struct DATASTRUCT {
		std::atomic<bool> IsCreatingD3d9 = false;
	};

	DATASTRUCT* GetWndProctStruct(HWND hWnd);
}

struct D9_DEVICE_CREATION {
	HWND hWnd = nullptr;
	DWORD BehaviorFlags = 0;
	LPDIRECT3D9 d3d9Object = nullptr;
	LPDIRECT3DDEVICE9* d3d9Device = nullptr;
	D3DPRESENT_PARAMETERS* presParams = nullptr;
	HANDLE hEvent = nullptr;
	HRESULT hr = E_FAIL;
};
