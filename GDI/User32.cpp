/**
* Copyright (C) 2022 Elisha Riedlinger
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
#include "Wrappers\wrapper.h"
#include "GDI.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"

namespace GdiWrapper
{
	FARPROC CreateWindowExA_out = nullptr;
	FARPROC CreateWindowExW_out = nullptr;
	FARPROC DestroyWindow_out = nullptr;
}

using namespace GdiWrapper;

LPCSTR GetClassName(LPCSTR lpClassName) { return ((DWORD)lpClassName > 0xFFFF) ? lpClassName : "ClassName"; }

LPCWSTR GetClassName(LPCWSTR lpClassName) { return ((DWORD)lpClassName > 0xFFFF) ? lpClassName : L"ClassName"; }

template <class D, class T>
HWND WINAPI user_CreateWindowExT(D m_pCreateWindowEx, DWORD dwExStyle, T lpClassName, T lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	// lpClassName: A null-terminated string or a class atom created by a previous call to the RegisterClass or RegisterClassEx function.

	Logging::LogDebug() << __FUNCTION__ << " " << GetClassName(lpClassName) << " " << lpWindowName << " " << Logging::hex(dwExStyle) << " " << Logging::hex(dwStyle) << " " << X << "x" << Y << " " << nWidth << "x" << nHeight << " " << Logging::hex((DWORD)hWndParent) << " " << hWndParent << " " << hMenu << " " << hInstance;

	if (!m_pCreateWindowEx)
	{
		return nullptr;
	}

	// Handle popup window type
	if ((dwStyle & WS_POPUP) && (dwStyle & WS_VISIBLE) && !(dwStyle & WS_CLIPSIBLINGS) && !hWndParent)
	{
		DWORD dwNewStyle = dwStyle;

		// Remove popup style
		dwNewStyle = dwStyle & ~WS_POPUP;

		HWND hwnd = m_pCreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwNewStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

		if (hwnd)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Removed WS_POPUP window style! " << hwnd);

			SetWindowLong(hwnd, GWL_STYLE, dwNewStyle);

			return hwnd;
		}
	}

	return m_pCreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

HWND WINAPI user_CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	static CreateWindowExAProc m_pCreateWindowExA = (Wrapper::ValidProcAddress(CreateWindowExA_out)) ? (CreateWindowExAProc)CreateWindowExA_out : nullptr;

	return user_CreateWindowExT<CreateWindowExAProc, LPCSTR>(m_pCreateWindowExA, dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

HWND WINAPI user_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	static CreateWindowExWProc m_pCreateWindowExW = (Wrapper::ValidProcAddress(CreateWindowExW_out)) ? (CreateWindowExWProc)CreateWindowExW_out : nullptr;

	return user_CreateWindowExT<CreateWindowExWProc, LPCWSTR>(m_pCreateWindowExW, dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

BOOL WINAPI user_DestroyWindow(HWND hWnd)
{
	Logging::LogDebug() << __FUNCTION__ << " " << hWnd;

	static DestroyWindowProc m_pDestroyWindow = (Wrapper::ValidProcAddress(DestroyWindow_out)) ? (DestroyWindowProc)DestroyWindow_out : nullptr;

	if (!m_pDestroyWindow)
	{
		return NULL;
	}

	HWND ownd = nullptr;

	if (IsWindow(hWnd))
	{
		ownd = GetWindow(hWnd, GW_OWNER);
	}

	BOOL result = m_pDestroyWindow(hWnd);

	if (result && ownd)
	{
		RedrawWindow(ownd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);

		// Peek messages to help prevent a "Not Responding" window
		MSG msg;
		PeekMessage(&msg, ownd, 0, 0, PM_NOREMOVE);
	}

	return result;
}
