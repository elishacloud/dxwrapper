#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

	namespace WriteMemory
	{
		void WriteMemory();
		void StopThread();
	}

	namespace Fullscreen
	{
		void StartThread();
		bool IsThreadRunning();
		void StopThread();
		void ResetScreen();
		HWND FindMainWindow(DWORD, bool, bool = false);
	}
}

#define LoadLibrary Utils::LoadLibrary
