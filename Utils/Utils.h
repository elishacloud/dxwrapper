#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "External\MemoryModule\MemoryModule.h"

#undef LoadLibrary

namespace Utils
{
	void Shell(const char*);
	void DisableHighDPIScaling();
	void SetProcessAffinity();
	void SetAppCompat();
	FARPROC GetProcAddress(HMODULE hModule, LPCSTR FunctionName, FARPROC SetReturnValue);
	extern FARPROC pGetProcAddress;
	FARPROC WINAPI GetProcAddressHandler(HMODULE hModule, LPSTR lpProcName);
	extern FARPROC pGetModuleFileNameA;
	extern FARPROC pGetModuleFileNameW;
	DWORD WINAPI GetModuleFileNameAHandler(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
	DWORD WINAPI GetModuleFileNameWHandler(HMODULE hModule, LPWSTR lpFilename, DWORD nSize);
	void HookExceptionHandler();
	void UnHookExceptionHandler();
	void AddHandleToVector(HMODULE dll, const char *name);
	HMODULE LoadLibrary(const char *dllname, bool EnableLogging = false);
	void LoadCustomDll();
	void LoadPlugins();
	void UnloadAllDlls();
	HMEMORYMODULE LoadResourceToMemory(DWORD ResID);
	void DDrawResolutionHack(HMODULE hD3DIm);
	void GetScreenSettings();
	void ResetScreenSettings();
	HMONITOR GetMonitorHandle(HWND hWnd);
	DWORD GetRefreshRate(HWND hWnd);
	DWORD GetBitCount(HWND hWnd);
	DWORD GetWindowHeight(HWND hWnd);
	void DisableGameUX();
	void GetScreenSize(HWND hwnd, LONG &screenWidth, LONG &screenHeight);
	void GetScreenSize(HWND hwnd, DWORD &screenWidth, DWORD &screenHeight);
	void GetDesktopRect(HWND hWnd, RECT& screenRect);
	DWORD GetVideoRam(DWORD DefaultSize);

	namespace WriteMemory
	{
		bool CheckMemoryAddress(void *dataAddr, void *dataBytes, size_t dataSize);
		bool UpdateMemoryAddress(void *dataAddr, void *dataBytes, size_t dataSize);
		void WriteMemory();
		void StopThread();
	}

	namespace Fullscreen
	{
		void StartThread();
		bool IsThreadRunning();
		void StopThread();
		void ResetScreen();
	}
}

bool stristr(LPCSTR strCheck, LPCSTR str, size_t size);
bool wcsistr(LPCWSTR strCheck, LPCWSTR str, size_t size);

#define LoadLibrary Utils::LoadLibrary
