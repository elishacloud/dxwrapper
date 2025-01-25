/**
* Copyright (C) 2024 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "GDI.h"
#include "GDI\WndProc.h"
#include "Utils\Utils.h"
#include "ddraw\ddrawExternal.h"
#include "d3d9\d3d9External.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"

namespace GdiWrapper
{
	INITIALIZE_OUT_WRAPPED_PROC(CreateWindowExA, unused);
	INITIALIZE_OUT_WRAPPED_PROC(CreateWindowExW, unused);
	INITIALIZE_OUT_WRAPPED_PROC(DestroyWindow, unused);
	INITIALIZE_OUT_WRAPPED_PROC(GetSystemMetrics, unused);
	INITIALIZE_OUT_WRAPPED_PROC(GetWindowLongA, unused);
	INITIALIZE_OUT_WRAPPED_PROC(GetWindowLongW, unused);
	INITIALIZE_OUT_WRAPPED_PROC(SetWindowLongA, unused);
	INITIALIZE_OUT_WRAPPED_PROC(SetWindowLongW, unused);
}

using namespace GdiWrapper;

LPCSTR GetClassName(LPCSTR lpClassName) { return ((DWORD)lpClassName > 0xFFFF) ? lpClassName : "ClassName"; }

LPCWSTR GetClassName(LPCWSTR lpClassName) { return ((DWORD)lpClassName > 0xFFFF) ? lpClassName : L"ClassName"; }

template <class D, class T>
HWND WINAPI user_CreateWindowExT(D CreateWindowExT, DWORD dwExStyle, T lpClassName, T lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	// lpClassName: A null-terminated string or a class atom created by a previous call to the RegisterClass or RegisterClassEx function.

	Logging::LogDebug() << __FUNCTION__ << " " << GetClassName(lpClassName) << " " << lpWindowName << " " << Logging::hex(dwExStyle) << " " << Logging::hex(dwStyle) << " " << X << "x" << Y << " " << nWidth << "x" << nHeight << " " << Logging::hex((DWORD)hWndParent) << " " << hWndParent << " " << hMenu << " " << hInstance;

	if (!CreateWindowExT)
	{
		return nullptr;
	}

	// Check if border is missing from pop-up window (DXVK has issues with borderless windows)
	if (dwStyle == (WS_POPUPWINDOW & dwStyle) && (dwStyle && WS_POPUP) && !(dwStyle & WS_BORDER))
	{
		HMODULE hModule = nullptr;
		if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, "vulkan-1.dll", &hModule) && hModule)
		{
			dwStyle |= WS_BORDER;
		}
	}

	// Handle popup window type (some games forget to initialize the nWidth and nHeight values)
	if ((dwStyle & WS_POPUP) && !(nWidth & CW_USEDEFAULT) && (nWidth > 20000 || nHeight > 20000))
	{
		Utils::GetScreenSize(hWndParent, nWidth, nHeight);
	}

	return CreateWindowExT(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

HWND WINAPI user_CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	DEFINE_STATIC_PROC_ADDRESS(CreateWindowExAProc, CreateWindowExA, CreateWindowExA_out);

	return user_CreateWindowExT<CreateWindowExAProc, LPCSTR>(CreateWindowExA, dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

HWND WINAPI user_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	DEFINE_STATIC_PROC_ADDRESS(CreateWindowExWProc, CreateWindowExW, CreateWindowExW_out);

	return user_CreateWindowExT<CreateWindowExWProc, LPCWSTR>(CreateWindowExW, dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

BOOL WINAPI user_DestroyWindow(HWND hWnd)
{
	Logging::LogDebug() << __FUNCTION__ << " " << hWnd;

	DEFINE_STATIC_PROC_ADDRESS(DestroyWindowProc, DestroyWindow, DestroyWindow_out);

	if (!DestroyWindow)
	{
		return NULL;
	}

	HWND ownd = nullptr;

	if (IsWindow(hWnd))
	{
		ownd = GetWindow(hWnd, GW_OWNER);
	}

	BOOL result = DestroyWindow(hWnd);

	if (result && ownd)
	{
		RedrawWindow(ownd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}

	return result;
}

int WINAPI user_GetSystemMetrics(int nIndex)
{
	Logging::LogDebug() << __FUNCTION__ << " " << nIndex;

	DEFINE_STATIC_PROC_ADDRESS(GetSystemMetricsProc, GetSystemMetrics, GetSystemMetrics_out);

	switch (nIndex)
	{
	case SM_CXSCREEN:
	{
		int Width = GetDDrawWidth();
		if (Width)
		{
			return Width;
		}
		break;
	}
	case SM_CYSCREEN:
	{
		int Height = GetDDrawHeight();
		if (Height)
		{
			return Height;
		}
		break;
	}
	}

	if (!GetSystemMetrics)
	{
		return 0;
	}

	return GetSystemMetrics(nIndex);
}

LONG WINAPI GetWindowLongT(GetWindowLongProc GetWindowLongT, HWND hWnd, int nIndex)
{
	Logging::LogDebug() << __FUNCTION__ << " " << hWnd << " " << nIndex;

	if (!GetWindowLongT)
	{
		return NULL;
	}

	return GetWindowLongT(hWnd, nIndex);
}

LONG WINAPI user_GetWindowLongA(HWND hWnd, int nIndex)
{
	DEFINE_STATIC_PROC_ADDRESS(GetWindowLongProc, GetWindowLongA, GetWindowLongA_out);

	return GetWindowLongT(GetWindowLongA, hWnd, nIndex);
}

LONG WINAPI user_GetWindowLongW(HWND hWnd, int nIndex)
{
	DEFINE_STATIC_PROC_ADDRESS(GetWindowLongProc, GetWindowLongW, GetWindowLongW_out);

	return GetWindowLongT(GetWindowLongW, hWnd, nIndex);
}

LONG WINAPI SetWindowLongT(SetWindowLongProc SetWindowLongT, HWND hWnd, int nIndex, LONG dwNewLong)
{
	Logging::LogDebug() << __FUNCTION__ << " " << hWnd << " " << nIndex << " " << Logging::hex(dwNewLong);

	if (!SetWindowLongT)
	{
		return NULL;
	}

	return SetWindowLongT(hWnd, nIndex, dwNewLong);
}

LONG WINAPI user_SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong)
{
	DEFINE_STATIC_PROC_ADDRESS(SetWindowLongProc, SetWindowLongA, SetWindowLongA_out);

	return SetWindowLongT(SetWindowLongA, hWnd, nIndex, dwNewLong);
}

LONG WINAPI user_SetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong)
{
	DEFINE_STATIC_PROC_ADDRESS(SetWindowLongProc, SetWindowLongW, SetWindowLongW_out);

	return SetWindowLongT(SetWindowLongW, hWnd, nIndex, dwNewLong);
}
