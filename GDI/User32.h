#pragma once

typedef HWND(WINAPI* CreateWindowExAProc)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
typedef HWND(WINAPI* CreateWindowExWProc)(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
typedef BOOL(WINAPI* DestroyWindowProc)(HWND hWnd);
typedef int(WINAPI* GetSystemMetricsProc)(int nIndex);
typedef LONG(WINAPI* GetWindowLongProc)(HWND hWnd, int nIndex);
typedef LONG(WINAPI* SetWindowLongProc)(HWND hWnd, int nIndex, LONG dwNewLong);

HWND WINAPI user_CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
HWND WINAPI user_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
BOOL WINAPI user_DestroyWindow(HWND hWnd);
int WINAPI user_GetSystemMetrics(int nIndex);
LONG WINAPI user_GetWindowLongA(HWND hWnd, int nIndex);
LONG WINAPI user_GetWindowLongW(HWND hWnd, int nIndex);
LONG WINAPI user_SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong);
LONG WINAPI user_SetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong);

namespace GdiWrapper
{
	EXPORT_OUT_WRAPPED_PROC(CreateWindowExA, unused);
	EXPORT_OUT_WRAPPED_PROC(CreateWindowExW, unused);
	EXPORT_OUT_WRAPPED_PROC(DestroyWindow, unused);
	EXPORT_OUT_WRAPPED_PROC(GetSystemMetrics, unused);
	EXPORT_OUT_WRAPPED_PROC(GetWindowLongA, unused);
	EXPORT_OUT_WRAPPED_PROC(GetWindowLongW, unused);
	EXPORT_OUT_WRAPPED_PROC(SetWindowLongA, unused);
	EXPORT_OUT_WRAPPED_PROC(SetWindowLongW, unused);
}
