#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Wrappers\wrapper.h"
#include "External\MemoryModule\MemoryModule.h"

#undef LoadLibrary

namespace Utils
{
	EXPORT_OUT_WRAPPED_PROC(GetProcAddress, unused);
	EXPORT_OUT_WRAPPED_PROC(GetModuleFileNameA, unused);
	EXPORT_OUT_WRAPPED_PROC(GetModuleFileNameW, unused);
	EXPORT_OUT_WRAPPED_PROC(GetDiskFreeSpaceA, unused);
	EXPORT_OUT_WRAPPED_PROC(CreateThread, unused);
	EXPORT_OUT_WRAPPED_PROC(VirtualAlloc, unused);

	void Shell(const char*);
	void DisableHighDPIScaling();
	DWORD GetCoresUsedByProcess();
	void SetProcessAffinity();
	FARPROC GetProcAddress(HMODULE hModule, LPCSTR FunctionName, FARPROC SetReturnValue);
	FARPROC WINAPI GetProcAddressHandler(HMODULE hModule, LPSTR lpProcName);
	DWORD WINAPI GetModuleFileNameAHandler(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
	DWORD WINAPI GetModuleFileNameWHandler(HMODULE hModule, LPWSTR lpFilename, DWORD nSize);
	BOOL WINAPI kernel_GetDiskFreeSpaceA(LPCSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters);
	HANDLE WINAPI kernel_CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
	LPVOID WINAPI kernel_VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
	void HookExceptionHandler();
	void UnHookExceptionHandler();
	void AddHandleToVector(HMODULE dll, const char *name);
	HMODULE LoadLibrary(const char *dllname, bool EnableLogging = false);
	void LoadCustomDll();
	void LoadPlugins();
	void UnloadAllDlls();
	HMEMORYMODULE LoadMemoryToDLL(LPVOID pMemory, DWORD Size);
	HMEMORYMODULE LoadResourceToMemory(DWORD ResID);
	DWORD ReverseBits(DWORD v);
	void DDrawResolutionHack(HMODULE hD3DIm);
	void BusyWaitYield();
	void ResetInvalidFPUState();
	void CheckMessageQueue(HWND hwnd);
	bool IsWindowsVistaOrNewer();
	bool IsWindows7OrNewer();
	bool IsWindows8OrNewer();
	void GetScreenSettings();
	void ResetScreenSettings();
	void ResetGamma();
	bool IsWindowRectEqualOrLarger(HWND srchWnd, HWND desthWnd);
	HWND GetTopLevelWindowOfCurrentProcess();
	HMONITOR GetMonitorHandle(HWND hWnd);
	DWORD GetRefreshRate(HWND hWnd);
	DWORD GetBitCount(HWND hWnd);
	DWORD GetThreadIDByHandle(HANDLE hThread);
	void DisableGameUX();
	void WaitForWindowActions(HWND hWnd, DWORD Loops);
	bool SetWndProcFilter(HWND hWnd);
	bool RestoreWndProcFilter(HWND hWnd);
	void GetScreenSize(HWND hwnd, LONG &screenWidth, LONG &screenHeight);
	void GetScreenSize(HWND hwnd, int &screenWidth, int &screenHeight);
	void GetDesktopRect(HWND hWnd, RECT& screenRect);
	HRESULT GetVideoRam(UINT AdapterNo, DWORD& TotalMemory);	// Adapters start numbering from '1', based on "Win32_VideoController" WMI class and "DeviceID" property.
}

namespace WriteMemory
{
	bool CheckMemoryAddress(void* dataAddr, void* dataBytes, size_t dataSize);
	bool UpdateMemoryAddress(void* dataAddr, void* dataBytes, size_t dataSize);
	void WriteMemory();
	void StopThread();
}

namespace Fullscreen
{
	void StartThread();
	bool IsThreadRunning();
	void StopThread();
	void ResetScreen();
	HWND FindMainWindow(DWORD process_id, bool AutoDetect, bool Debug = false);
}

bool stristr(LPCSTR strCheck, LPCSTR str, size_t size);
bool wcsistr(LPCWSTR strCheck, LPCWSTR str, size_t size);

#define LoadLibrary Utils::LoadLibrary
