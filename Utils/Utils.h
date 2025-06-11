#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>
#include "Wrappers\wrapper.h"
#include "External\MemoryModule\MemoryModule.h"
#include "Logging\Logging.h"
#ifndef _TIMERAPI_H_
#include "winmm.h"
#endif

#undef LoadLibrary

namespace Utils
{
	EXPORT_OUT_WRAPPED_PROC(GetProcAddress, unused);
	EXPORT_OUT_WRAPPED_PROC(GetModuleFileNameA, unused);
	EXPORT_OUT_WRAPPED_PROC(GetModuleFileNameW, unused);
	EXPORT_OUT_WRAPPED_PROC(GetDiskFreeSpaceA, unused);
	EXPORT_OUT_WRAPPED_PROC(CreateThread, unused);
	EXPORT_OUT_WRAPPED_PROC(CreateFileA, unused);
	EXPORT_OUT_WRAPPED_PROC(VirtualAlloc, unused);
	EXPORT_OUT_WRAPPED_PROC(HeapAlloc, unused);
	EXPORT_OUT_WRAPPED_PROC(HeapSize, unused);
	EXPORT_OUT_WRAPPED_PROC(QueryPerformanceFrequency, unused);
	EXPORT_OUT_WRAPPED_PROC(QueryPerformanceCounter, unused);
	EXPORT_OUT_WRAPPED_PROC(GetTickCount, unused);
	EXPORT_OUT_WRAPPED_PROC(GetTickCount64, unused);
	EXPORT_OUT_WRAPPED_PROC(timeGetTime, unused);
	EXPORT_OUT_WRAPPED_PROC(timeGetSystemTime, unused);

	void Shell(const char*);
	void DisableHighDPIScaling();
	FARPROC GetProcAddress(HMODULE hModule, LPCSTR FunctionName, FARPROC SetReturnValue);
	FARPROC WINAPI GetProcAddressHandler(HMODULE hModule, LPSTR lpProcName);
	DWORD WINAPI GetModuleFileNameAHandler(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
	DWORD WINAPI GetModuleFileNameWHandler(HMODULE hModule, LPWSTR lpFilename, DWORD nSize);
	BOOL WINAPI kernel_GetDiskFreeSpaceA(LPCSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters);
	HANDLE WINAPI kernel_CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
	HANDLE WINAPI kernel_CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
	LPVOID WINAPI kernel_VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
	LPVOID WINAPI kernel_HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
	SIZE_T WINAPI kernel_HeapSize(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem);
	bool InitUpTimeOffsets();
	BOOL WINAPI kernel_QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);
	BOOL WINAPI kernel_QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);
	DWORD WINAPI kernel_GetTickCount();
	ULONGLONG WINAPI kernel_GetTickCount64();
	DWORD WINAPI winmm_timeGetTime();
	MMRESULT WINAPI winmm_timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt);
	void HookExceptionHandler();
	void UnHookExceptionHandler();
	LONG WINAPI Vectored_Exception_Handler(EXCEPTION_POINTERS* ExceptionInfo);
	void AddHandleToVector(HMODULE dll, const char *name);
	HMODULE LoadLibrary(const char *dllname, bool EnableLogging = false);
	void LoadCustomDll();
	void LoadPlugins();
	void UnloadAllDlls();
	HMEMORYMODULE LoadMemoryToDLL(LPVOID pMemory, DWORD Size);
	HMEMORYMODULE LoadResourceToMemory(DWORD ResID);
	DWORD ReverseBits(DWORD v);
	void DDrawResolutionHack(HMODULE hD3DIm);
	void ResetInvalidFPUState();
	void CheckMessageQueue(HWND hWnd);
	bool IsWindowsVistaOrNewer();
	bool IsWindows7OrNewer();
	bool IsWindows8OrNewer();
	void GetScreenSettings();
	void ResetScreenSettings();
	void ResetGamma();
	bool IsMainWindow(HWND hWnd);
	HWND GetMainWindowForProcess(DWORD processId);
	bool IsWindowRectEqualOrLarger(HWND srchWnd, HWND desthWnd);
	HWND GetTopLevelWindowOfCurrentProcess();
	bool IsMonitorValid(HMONITOR hMonitor);
	HMONITOR GetMonitorFromWindow(HWND hWnd);
	HMONITOR GetMonitorFromDeviceName(char* DeviceName);
	bool MoveWindowToMonitor(HMONITOR hMonitor, HWND hWnd);
	BOOL SetWindowPosToMonitor(HMONITOR hMonitor, HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
	BOOL SetWindowPlacementToMonitor(HMONITOR hMonitor, HWND hWnd, const WINDOWPLACEMENT* lpwndpl);
	void SetDisplaySettings(HMONITOR hMonitor, DWORD Width, DWORD Height);
	void ResetDisplaySettings(HMONITOR hMonitor);
	DWORD GetRefreshRate(HMONITOR hMonitor);
	DWORD GetBitCount(HMONITOR hMonitor);
	DWORD GetThreadIDByHandle(HANDLE hThread);
	void DisableGameUX();
	void WaitForWindowActions(HWND hWnd, DWORD Loops);
	void ApplyFPUSetup();
	void GetModuleFromAddress(void* address, char* module, const size_t size);
	bool SetWndProcFilter(HWND hWnd);
	bool RestoreWndProcFilter(HWND hWnd);
	void GetScreenSize(HMONITOR hMonitor, volatile LONG &screenWidth, volatile LONG &screenHeight);
	void GetScreenSize(HMONITOR hMonitor, int& screenWidth, int& screenHeight);
	void GetScreenClientRect(HMONITOR hMonitor, RECT& workAreaOut);
	HRESULT GetVideoRam(UINT AdapterNo, DWORD& TotalMemory);	// Adapters start numbering from '1', based on "Win32_VideoController" WMI class and "DeviceID" property.

	// CPU Affinity
	void SetProcessAffinity();
	void SetThreadAffinity(DWORD threadId);
	void ApplyThreadAffinity();
	void StartPriorityMonitor();
	void StopPriorityMonitor();

	inline void BusyWaitYield(DWORD RemainingMS)
	{
		static bool supports_pause = []() {
			int cpu_info[4] = { 0 };
			__cpuid(cpu_info, 1); // Query CPU features
			bool SSE2 = (cpu_info[3] & (1 << 26)) != 0; // Check for SSE2 support
			LOG_ONCE(__FUNCTION__ << " SSE2 CPU support: " << SSE2);
			return SSE2;
			}();

		// If remaining time is very small (e.g., 3 ms or less), use busy-wait with no operations
		if (RemainingMS < 4 && supports_pause)
		{
			// Use _mm_pause or __asm { nop } to prevent unnecessary CPU cycles
#ifdef YieldProcessor
			YieldProcessor();
#else
			_mm_pause();
#endif
		}
		else
		{
			// For larger remaining times, we can relax by yielding to the OS
			// Sleep(0) yields without consuming CPU excessively
			Sleep(0); // Let the OS schedule other tasks if there's significant time left
		}
	}
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
}

bool stristr(LPCSTR strCheck, LPCSTR str, size_t size);
bool wcsistr(LPCWSTR strCheck, LPCWSTR str, size_t size);

#define LoadLibrary Utils::LoadLibrary
