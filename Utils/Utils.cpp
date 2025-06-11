/**
* Copyright (C) 2025 Elisha Riedlinger
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
*
* ASI plugin loader taken from source code found in Ultimate ASI Loader
* https://github.com/ThirteenAG/Ultimate-ASI-Loader
*
* DDrawResolutionHack taken from source code found in LegacyD3DResolutionHack
* https://github.com/UCyborg/LegacyD3DResolutionHack
*
* GetVideoRam taken from source code found in doom3.gpl
* https://github.com/TTimo/doom3.gpl
* 
* ReverseBits code taken from stanford.edu
* http://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iomanip>
#include <intrin.h>
#include <tlhelp32.h>
#include <atlbase.h>
#include <comdef.h>
#include <comutil.h>
#include <Wbemidl.h>
#include "Utils.h"
#include "Settings\Settings.h"
#include "Dllmain\Dllmain.h"
#include "Wrappers\wrapper.h"
#include "ddraw\ddrawExternal.h"
#include "d3d8\d3d8External.h"
#include "d3d9\d3d9External.h"
#include "External\Hooking\Hook.h"
#include "External\Hooking\Disasm.h"
#include "Logging\Logging.h"

#undef LoadLibrary

#include <initguid.h>

// Define the GUIDs
DEFINE_GUID(IID_IWbemLocator,
	0xDC12A687, 0x737F, 0x11CF, 0x88, 0x4D, 0x00, 0xAA, 0x00, 0x4B, 0x2E, 0x24);

DEFINE_GUID(CLSID_WbemLocator,
	0x4590F811, 0x1D3A, 0x11D0, 0x89, 0x1F, 0x00, 0xAA, 0x00, 0x4B, 0x2E, 0x24);

#define PAGE_READABLE_FLAGS (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)

#ifdef _DPI_AWARENESS_CONTEXTS_
typedef enum PROCESS_DPI_AWARENESS {
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
typedef HRESULT(WINAPI *SetProcessDpiAwarenessProc)(PROCESS_DPI_AWARENESS value);
typedef BOOL(WINAPI *SetProcessDPIAwareProc)();
typedef BOOL(WINAPI *SetProcessDpiAwarenessContextProc)(DPI_AWARENESS_CONTEXT value);
#endif // _DPI_AWARENESS_CONTEXTS_
typedef void(WINAPI* PFN_InitializeASI)(void);
typedef DWORD(WINAPI* GetThreadIdProc)(HANDLE Thread);
typedef FARPROC(WINAPI *GetProcAddressProc)(HMODULE, LPSTR);
typedef DWORD(WINAPI *GetModuleFileNameAProc)(HMODULE, LPSTR, DWORD);
typedef DWORD(WINAPI *GetModuleFileNameWProc)(HMODULE, LPWSTR, DWORD);
typedef BOOL(WINAPI* GetDiskFreeSpaceAProc)(LPCSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters);
typedef BOOL(WINAPI *CreateProcessAFunc)(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
typedef HANDLE(WINAPI* CreateThreadProc)(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
typedef HANDLE(WINAPI* CreateFileAProc)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
typedef LPVOID(WINAPI* VirtualAllocProc)(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
typedef LPVOID(WINAPI* HeapAllocProc)(HANDLE, DWORD, SIZE_T);
typedef SIZE_T(WINAPI* HeapSizeProc)(HANDLE, DWORD, LPCVOID);
typedef BOOL(WINAPI *CreateProcessWProc)(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
typedef BOOL(WINAPI* QueryPerformanceFrequencyProc)(LARGE_INTEGER* lpFrequency);
typedef BOOL(WINAPI* QueryPerformanceCounterProc)(LARGE_INTEGER* lpPerformanceCount);
typedef DWORD(WINAPI* GetTickCountProc)();
typedef LONGLONG(WINAPI* GetTickCount64Proc)();
typedef DWORD(WINAPI* timeGetTimeProc)();
typedef MMRESULT(WINAPI* timeGetSystemTimeProc)(LPMMTIME pmmt, UINT cbmmt);

namespace Utils
{
	// Strictures
	struct type_dll
	{
		HMODULE dll = nullptr;
		std::string name;
		std::string fullname;
	};

	// FontSmoothing
	struct SystemSettings
	{
		bool isSet = false;
		BOOL enabled = FALSE;
		UINT type = 0;
		UINT contrast = 0;
		UINT orientation = 0;
	} fontSystemSettings;

	// Screen settings
	HDC hDC = nullptr;
	WORD lpRamp[3 * 256] = {};

	// Declare variables
	INITIALIZE_OUT_WRAPPED_PROC(GetProcAddress, unused);
	INITIALIZE_OUT_WRAPPED_PROC(GetModuleFileNameA, unused);
	INITIALIZE_OUT_WRAPPED_PROC(GetModuleFileNameW, unused);
	INITIALIZE_OUT_WRAPPED_PROC(GetDiskFreeSpaceA, unused);
	INITIALIZE_OUT_WRAPPED_PROC(CreateThread, unused);
	INITIALIZE_OUT_WRAPPED_PROC(CreateFileA, unused);
	INITIALIZE_OUT_WRAPPED_PROC(VirtualAlloc, unused);
	INITIALIZE_OUT_WRAPPED_PROC(HeapAlloc, unused);
	INITIALIZE_OUT_WRAPPED_PROC(HeapSize, unused);
	INITIALIZE_OUT_WRAPPED_PROC(QueryPerformanceFrequency, unused);
	INITIALIZE_OUT_WRAPPED_PROC(QueryPerformanceCounter, unused);
	INITIALIZE_OUT_WRAPPED_PROC(GetTickCount, unused);
	INITIALIZE_OUT_WRAPPED_PROC(GetTickCount64, unused);
	INITIALIZE_OUT_WRAPPED_PROC(timeGetTime, unused);
	INITIALIZE_OUT_WRAPPED_PROC(timeGetSystemTime, unused);

	FARPROC p_CreateProcessA = nullptr;
	FARPROC p_CreateProcessW = nullptr;
	WNDPROC OriginalWndProc = nullptr;
	std::vector<type_dll> custom_dll;		// Used for custom dll's and asi plugins

	DWORD SubtractTimeInMS_gtc = 0;
	int64_t SubtractTimeInMS_gtc64 = 0;
	DWORD SubtractTimeInMS_tmt = 0;
	DWORD SubtractTimeInMS_mmt = 0;
	int64_t SubtractTimeInTicks_qpc = 0;
	bool IsPerformanceFrequencyCapped = false;
	uint64_t PerformanceFrequency_real = 0;
	constexpr uint64_t PerformanceFrequency_cap = 0xFFFFFFFF;

	// Function declarations
	ULONGLONG GetTickCount64_Emulated();
	void InitializeASI(HMODULE hModule);
	void FindFiles(WIN32_FIND_DATA*);
	void *memmem(const void *l, size_t l_len, const void *s, size_t s_len);
	LRESULT CALLBACK WndProcFilter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

// Execute a specified string
void Utils::Shell(const char* fileName)
{
	Logging::Log() << "Running process: " << fileName;

	// Get StartupInfo and ProcessInfo memory size and set process window to hidden
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process
	if (!CreateProcess(nullptr, const_cast<LPSTR>(fileName), nullptr, nullptr, true, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
	{
		// Failed to launch process!
		Logging::Log() << "Failed to launch process!";
	}
	else
	{
		// Wait for process to exit
		if (Config.WaitForProcess) WaitForSingleObjectEx(pi.hThread, INFINITE, true);

		// Close thread handle
		CloseHandle(pi.hThread);

		// Close process handle
		CloseHandle(pi.hProcess);
	}
	// Quit function
	return;
}

// Sets application DPI aware which disables DPI virtulization/High DPI scaling for this process
void Utils::DisableHighDPIScaling()
{
#ifdef _DPI_AWARENESS_CONTEXTS_
	Logging::Log() << "Disabling High DPI Scaling...";

	BOOL setDpiAware = FALSE;
	HMODULE hUser32 = LoadLibrary("user32.dll");
	HMODULE hShcore = LoadLibrary("shcore.dll");
	if (hUser32 && !setDpiAware)
	{
		SetProcessDpiAwarenessContextProc setProcessDpiAwarenessContext = (SetProcessDpiAwarenessContextProc)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");

		if (setProcessDpiAwarenessContext)
		{
			setDpiAware = setProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
		}
	}
	if (hShcore && !setDpiAware)
	{
		SetProcessDpiAwarenessProc setProcessDpiAwareness = (SetProcessDpiAwarenessProc)GetProcAddress(hShcore, "SetProcessDpiAwareness");

		if (setProcessDpiAwareness)
		{
			HRESULT result = setProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

			setDpiAware = (SUCCEEDED(result) || result == E_ACCESSDENIED);
		}
	}
	if (hUser32 && !setDpiAware)
	{
		SetProcessDPIAwareProc setProcessDPIAware = (SetProcessDPIAwareProc)GetProcAddress(hUser32, "SetProcessDPIAware");

		if (setProcessDPIAware)
		{
			setDpiAware = setProcessDPIAware();
		}
	}

	if (!setDpiAware)
	{
		Logging::Log() << "Failed to disable High DPI Scaling!";
	}
#endif // _DPI_AWARENESS_CONTEXTS_
}

FARPROC Utils::GetProcAddress(HMODULE hModule, LPCSTR lpProcName, FARPROC SetReturnValue)
{
	if (!lpProcName || !hModule)
	{
		return SetReturnValue;
	}

	FARPROC ProcAddress = GetProcAddress(hModule, lpProcName);

	if (!ProcAddress)
	{
		ProcAddress = SetReturnValue;
	}

	return ProcAddress;
}

// Update GetProcAddress to check for bad addresses
FARPROC WINAPI Utils::GetProcAddressHandler(HMODULE hModule, LPSTR lpProcName)
{
	DEFINE_STATIC_PROC_ADDRESS(GetProcAddressProc, GetProcAddress, GetProcAddress_out);

	FARPROC ProAddr = nullptr;

	if (GetProcAddress)
	{
		ProAddr = GetProcAddress(hModule, lpProcName);
	}
	if (!(Wrapper::ValidProcAddress(ProAddr)))
	{
		ProAddr = nullptr;
		SetLastError(127);
	}

	return ProAddr;
}

// Update GetModuleFileNameA to fix module name
DWORD WINAPI Utils::GetModuleFileNameAHandler(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	DEFINE_STATIC_PROC_ADDRESS(GetModuleFileNameAProc, GetModuleFileNameA, GetModuleFileNameA_out);

	if (GetModuleFileNameA)
	{
		DWORD ret = GetModuleFileNameA(hModule, lpFilename, nSize);

		if (lpFilename[0] != '\\' && lpFilename[1] != '\\' && lpFilename[2] != '\\' && lpFilename[3] != '\\')
		{
			DWORD lSize = GetModuleFileNameA(nullptr, lpFilename, nSize);
			char *pdest = strrchr(lpFilename, '\\');
			if (pdest && lSize > 0 && nSize - lSize + strlen(dtypename[dtype.dxwrapper]) > 0)
			{
				strcpy_s(pdest + 1, nSize - lSize, dtypename[dtype.dxwrapper]);
				return strlen(lpFilename);
			}
			return lSize;
		}

		return ret;
	}

	SetLastError(5);
	return 0;
}

// Update GetModuleFileNameW to fix module name
DWORD WINAPI Utils::GetModuleFileNameWHandler(HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
	DEFINE_STATIC_PROC_ADDRESS(GetModuleFileNameWProc, GetModuleFileNameW, GetModuleFileNameW_out);

	if (GetModuleFileNameW)
	{
		DWORD ret = GetModuleFileNameW(hModule, lpFilename, nSize);

		if (lpFilename[0] != '\\' && lpFilename[1] != '\\' && lpFilename[2] != '\\' && lpFilename[3] != '\\')
		{
			DWORD lSize = GetModuleFileNameW(nullptr, lpFilename, nSize);
			wchar_t *pdest = wcsrchr(lpFilename, '\\');
			std::string str(dtypename[dtype.dxwrapper]);
			std::wstring wrappername(str.begin(), str.end());
			if (pdest && lSize > 0 && nSize - lSize + strlen(dtypename[dtype.dxwrapper]) > 0)
			{
				wcscpy_s(pdest + 1, nSize - lSize, wrappername.c_str());
				return wcslen(lpFilename);
			}
			return lSize;
		}

		return ret;
	}

	SetLastError(5);
	return 0;
}

BOOL WINAPI Utils::kernel_GetDiskFreeSpaceA(LPCSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters)
{
	Logging::LogDebug() << __FUNCTION__;

	DEFINE_STATIC_PROC_ADDRESS(GetDiskFreeSpaceAProc, GetDiskFreeSpaceA, GetDiskFreeSpaceA_out);

	if (!GetDiskFreeSpaceA)
	{
		return FALSE;
	}

	BOOL result = GetDiskFreeSpaceA(lpRootPathName, lpSectorsPerCluster, lpBytesPerSector, lpNumberOfFreeClusters, lpTotalNumberOfClusters);

	// Limit the reported disk space
	if (lpSectorsPerCluster)
	{
		*lpSectorsPerCluster = min(0x00000040, *lpSectorsPerCluster);
	}
	if (lpBytesPerSector)
	{
		*lpBytesPerSector = min(0x00000200, *lpBytesPerSector);
	}
	if (lpNumberOfFreeClusters)
	{
		*lpNumberOfFreeClusters = min(0x0000F000, *lpNumberOfFreeClusters);
	}
	if (lpTotalNumberOfClusters)
	{
		*lpTotalNumberOfClusters = min(0x0000FFF6, *lpTotalNumberOfClusters);
	}

	return result;
}

HANDLE WINAPI Utils::kernel_CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
	Logging::LogDebug() << __FUNCTION__;

	DEFINE_STATIC_PROC_ADDRESS(CreateThreadProc, CreateThread, CreateThread_out);

	if (!CreateThread)
	{
		return nullptr;
	}

	// Check the current stack size, and if it's too small, increase it
	if (dwStackSize < 1024 * 64)  // Minimum 64 KB stack size
	{
		dwStackSize = 1024 * 64;
	}

	// Call the original CreateThread with modified parameters
	if (Config.SingleProcAffinity)
	{
		DWORD ThreadID = 0;
		if (!lpThreadId) lpThreadId = &ThreadID;

		HANDLE thread = CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags | CREATE_SUSPENDED, lpThreadId);

		if (thread)
		{
			SetThreadAffinity(*lpThreadId);
			if (!(dwCreationFlags & CREATE_SUSPENDED))
			{
				ResumeThread(thread);
			}
		}

		return thread;
	}

	return CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
}

HANDLE WINAPI Utils::kernel_CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	Logging::LogDebug() << __FUNCTION__ << " " << lpFileName;

	DEFINE_STATIC_PROC_ADDRESS(CreateFileAProc, CreateFileA, CreateFileA_out);

	if (!CreateFileA)
	{
		return nullptr;
	}

	auto IsPointerReadable = [](LPCVOID ptr) -> bool
		{
			MEMORY_BASIC_INFORMATION mbi = {};
			if (!ptr) return false;
			if (VirtualQuery(ptr, &mbi, sizeof(mbi)))
			{
				return (mbi.State == MEM_COMMIT) && (mbi.Protect & PAGE_READABLE_FLAGS);
			}
			return false;
		};

	auto IsPointerAligned = [](void* ptr, size_t alignment) -> bool
		{
			return (reinterpret_cast<uintptr_t>(ptr) % alignment) == 0;
		};

	auto SafeIsValidSecurityDescriptor = [](PSECURITY_DESCRIPTOR lpSD) -> bool
		{
			__try
			{
				return IsValidSecurityDescriptor(lpSD) != FALSE;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				return false;
			}
		};

	// Check for uninitialized Security Attributes
	SECURITY_ATTRIBUTES SecurityAttributes = {};
	if (lpSecurityAttributes &&
		lpSecurityAttributes->lpSecurityDescriptor &&
		(reinterpret_cast<uintptr_t>(lpSecurityAttributes->lpSecurityDescriptor) > 0x200000 ||	// Check for high-range garbage pointers
		!IsPointerAligned(lpSecurityAttributes->lpSecurityDescriptor, sizeof(void*)) ||			// Check if pointer is not byte aligned
		static_cast<uint32_t>(lpSecurityAttributes->bInheritHandle) > TRUE) &&					// Check if BOOL is not TRUE or FALSE
		(!IsPointerReadable(lpSecurityAttributes->lpSecurityDescriptor) || !SafeIsValidSecurityDescriptor(lpSecurityAttributes->lpSecurityDescriptor)))
	{
		LOG_LIMIT(3, __FUNCTION__ << " Fixing invalid SECURITY_ATTRIBUTES for CreateFileA");
		SecurityAttributes.nLength = lpSecurityAttributes->nLength;
		lpSecurityAttributes = &SecurityAttributes;
	}

	return CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

LPVOID WINAPI Utils::kernel_VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
{
	Logging::LogDebug() << __FUNCTION__ " " << lpAddress << " " << dwSize << " " << flAllocationType << " " << flProtect;

	DEFINE_STATIC_PROC_ADDRESS(VirtualAllocProc, VirtualAlloc, VirtualAlloc_out);

	if (!VirtualAlloc)
	{
		return FALSE;
	}

	// If this is a stack allocation (MEM_RESERVE | MEM_COMMIT)
	if ((flAllocationType & MEM_RESERVE) && (flAllocationType & MEM_COMMIT))
	{
		// Ensure the reserve size is at least 1MB
		if (dwSize < 1024 * 1024)  // 1MB minimum reserve
		{
			dwSize = 1024 * 1024;
		}
	}

	// Call the original VirtualAlloc function
	return VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
}

LPVOID WINAPI Utils::kernel_HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes)
{
	//Logging::LogDebug() << __FUNCTION__ " " << " hHeap: " << hHeap << " dwFlags: " << Logging::hex(dwFlags) << " lpMem: " << lpMem;

	DEFINE_STATIC_PROC_ADDRESS(HeapAllocProc, HeapAlloc, HeapAlloc_out);

	if (!HeapAlloc)
	{
		return nullptr;
	}

	SIZE_T NewBytes = dwBytes + dwBytes / 16;
	NewBytes += NewBytes % 16;

	if (dwBytes > 128 * 512 && NewBytes < 0x7FFF8)
	{
		LPVOID ret = HeapAlloc(hHeap, dwFlags, NewBytes);
		if (ret)
		{
			return ret;
		}
	}

	// Call the original HeapSize function
	return HeapAlloc(hHeap, dwFlags, dwBytes);
}

SIZE_T WINAPI Utils::kernel_HeapSize(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem)
{
	//Logging::LogDebug() << __FUNCTION__ " " << " hHeap: " << hHeap << " dwFlags: " << Logging::hex(dwFlags) << " lpMem: " << lpMem;

	DEFINE_STATIC_PROC_ADDRESS(HeapSizeProc, HeapSize, HeapSize_out);

	if (!HeapSize)
	{
		return (SIZE_T)-1;
	}

	// Validate hHeap
	if (!HeapValidate(hHeap, 0, lpMem))
	{
		Logging::Log() << __FUNCTION__ << " Error: Invalid heap handle!";
		return (SIZE_T)-1;
	}

	// Call the original HeapSize function
	return HeapSize(hHeap, dwFlags, lpMem);
}

bool Utils::InitUpTimeOffsets()
{
	const uint64_t MS_PER_DAY = 86400000ULL;

	// Gather system uptime values
	LARGE_INTEGER qpc, freq;
	if (!QueryPerformanceFrequency(&freq) || !QueryPerformanceCounter(&qpc))
	{
		Logging::Log() << __FUNCTION__ << " Error: Query Performance failed!";

		return false;
	}
	uint64_t ms_qpc = (qpc.QuadPart * 1000ULL) / freq.QuadPart;

	MMTIME mmt = {};
	mmt.wType = TIME_MS;
	if (timeGetSystemTime(&mmt, sizeof(mmt)) != MMSYSERR_NOERROR || mmt.wType != TIME_MS)
	{
		Logging::Log() << __FUNCTION__ << " Error: timeGetSystemTime failed!";

		return false;
	}

	DWORD tmt = timeGetTime();

	DWORD gtc = GetTickCount();
#if (_WIN32_WINNT >= 0x0502)
	ULONGLONG gtc64 = GetTickCount64();
#endif

	// Calculate full days from uptime in ms
	uint64_t timeInDays = ms_qpc / MS_PER_DAY;
	uint64_t timeInMS = timeInDays * MS_PER_DAY;
	uint64_t remainderTimeMS = ms_qpc - timeInMS;

	SubtractTimeInMS_gtc = static_cast<DWORD>(gtc - remainderTimeMS);
#if (_WIN32_WINNT >= 0x0502)
	SubtractTimeInMS_gtc64 = gtc64 - remainderTimeMS;
#endif
	SubtractTimeInMS_tmt = static_cast<DWORD>(tmt - remainderTimeMS);
	SubtractTimeInMS_mmt = static_cast<DWORD>(mmt.u.ms - remainderTimeMS);
	SubtractTimeInTicks_qpc = (timeInMS * freq.QuadPart) / 1000ULL;

	PerformanceFrequency_real = freq.QuadPart;

	if (Config.FixPerfCounterUptime == 1 && PerformanceFrequency_real > PerformanceFrequency_cap)
	{
		IsPerformanceFrequencyCapped = true;

		Logging::Log() << __FUNCTION__ << " Warning: Frequency is too high capping it. " <<
			std::fixed << std::setprecision(3) <<
			" Real: " << ((double)PerformanceFrequency_real / 1'000'000'000.0) << " GHz" <<
			" Capped: " << ((double)PerformanceFrequency_cap / 1'000'000'000.0) << " GHz";
	}

	Logging::Log() << __FUNCTION__ << " Found " << timeInDays << " day" << (timeInDays == 1 ? "" : "s") << " system up time!";

	return true;
}

BOOL WINAPI Utils::kernel_QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency)
{
	DEFINE_STATIC_PROC_ADDRESS(QueryPerformanceFrequencyProc, QueryPerformanceFrequency, QueryPerformanceFrequency_out);

	if (!QueryPerformanceFrequency)
	{
		return 0;
	}

	if (lpFrequency)
	{
		if (IsPerformanceFrequencyCapped)
		{
			lpFrequency->QuadPart = PerformanceFrequency_cap;
		}
		else
		{
			lpFrequency->QuadPart = PerformanceFrequency_real;
		}

		return TRUE;
	}

	return QueryPerformanceFrequency(lpFrequency);
}

BOOL WINAPI Utils::kernel_QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
	DEFINE_STATIC_PROC_ADDRESS(QueryPerformanceCounterProc, QueryPerformanceCounter, QueryPerformanceCounter_out);

	if (!QueryPerformanceCounter)
	{
		return FALSE;
	}

	BOOL ret = QueryPerformanceCounter(lpPerformanceCount);
	if (ret != FALSE)
	{
		if (IsPerformanceFrequencyCapped)
		{
			// Use integer math with carefully ordered operations to increase precision
			int64_t delta = lpPerformanceCount->QuadPart - SubtractTimeInTicks_qpc;
			lpPerformanceCount->QuadPart = (delta / PerformanceFrequency_real) * PerformanceFrequency_cap +
				(delta % PerformanceFrequency_real) * PerformanceFrequency_cap / PerformanceFrequency_real;
		}
		else
		{
			lpPerformanceCount->QuadPart -= SubtractTimeInTicks_qpc;
		}
	}
	return ret;
}

ULONGLONG Utils::GetTickCount64_Emulated()
{
	struct QPF {
		BOOL ret;
		LARGE_INTEGER freq;
		QPF()
		{
			ret = kernel_QueryPerformanceFrequency(&freq);
		}
	};

	static QPF qpf;

	LARGE_INTEGER qpc;
	if (!qpf.ret || !kernel_QueryPerformanceCounter(&qpc))
	{
		return 0;
	}

	return (qpc.QuadPart * 1000ULL) / qpf.freq.QuadPart;
}

DWORD WINAPI Utils::kernel_GetTickCount()
{
	DEFINE_STATIC_PROC_ADDRESS(GetTickCountProc, GetTickCount, GetTickCount_out);

	if (!GetTickCount)
	{
		return 0;
	}

	if (IsPerformanceFrequencyCapped)
	{
		return static_cast<DWORD>(GetTickCount64_Emulated());
	}
	else
	{
		return GetTickCount() - SubtractTimeInMS_gtc;
	}
}

ULONGLONG WINAPI Utils::kernel_GetTickCount64()
{
	DEFINE_STATIC_PROC_ADDRESS(GetTickCount64Proc, GetTickCount64, GetTickCount64_out);

	if (!GetTickCount64)
	{
		return 0;
	}

	if (IsPerformanceFrequencyCapped)
	{
		return GetTickCount64_Emulated();
	}
	else
	{
		return GetTickCount64() - SubtractTimeInMS_gtc64;
	}
}

DWORD WINAPI Utils::winmm_timeGetTime()
{
	DEFINE_STATIC_PROC_ADDRESS(timeGetTimeProc, timeGetTime, timeGetTime_out);

	if (!timeGetTime)
	{
		return 0;
	}

	return timeGetTime() - SubtractTimeInMS_tmt;
}

MMRESULT WINAPI Utils::winmm_timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt)
{
	DEFINE_STATIC_PROC_ADDRESS(timeGetSystemTimeProc, timeGetSystemTime, timeGetSystemTime_out);

	if (!timeGetSystemTime)
	{
		return TIMERR_NOCANDO;
	}

	MMRESULT result = timeGetSystemTime(pmmt, cbmmt);
	if (result == TIMERR_NOERROR)
	{
		if (cbmmt == sizeof(MMTIME) && pmmt->wType == TIME_MS)
		{
			pmmt->u.ms -= SubtractTimeInMS_mmt;
		}
	}
	return result;
}

// Your existing exception handler function
LONG WINAPI Utils::Vectored_Exception_Handler(EXCEPTION_POINTERS* ExceptionInfo)
{
	if (ExceptionInfo &&
		ExceptionInfo->ContextRecord &&
		ExceptionInfo->ExceptionRecord &&
		ExceptionInfo->ExceptionRecord->ExceptionAddress &&
		ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_PRIVILEGED_INSTRUCTION)
	{
		size_t size = Disasm::getInstructionLength(ExceptionInfo->ExceptionRecord->ExceptionAddress);

		if (size)
		{
			static DWORD count = 0;
			if (count++ < 10)
			{
				char moduleName[MAX_PATH];
				GetModuleFromAddress(ExceptionInfo->ExceptionRecord->ExceptionAddress, moduleName, MAX_PATH);

				Logging::Log() << "Skipping exception:" <<
					" code=" << Logging::hex(ExceptionInfo->ExceptionRecord->ExceptionCode) <<
					" flags=" << Logging::hex(ExceptionInfo->ExceptionRecord->ExceptionFlags) <<
					" addr=" << ExceptionInfo->ExceptionRecord->ExceptionAddress <<
					" module=" << moduleName;
			}

			ExceptionInfo->ContextRecord->Eip += size;
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

// Add HMODULE to vector
void Utils::AddHandleToVector(HMODULE dll, const char *name)
{
	if (dll)
	{
		type_dll newCustom_dll;
		newCustom_dll.dll = dll;
		newCustom_dll.name.assign((strrchr(name, '\\')) ? strrchr(name, '\\') + 1 : name);
		newCustom_dll.fullname.assign(name);
		custom_dll.push_back(newCustom_dll);
	}
}

// Load real dll file that is being wrapped
HMODULE Utils::LoadLibrary(const char *dllname, bool EnableLogging)
{
	// Declare vars
	HMODULE dll = nullptr;
	const char *loadpath;
	char path[MAX_PATH] = { 0 };

	// Check if dll is already loaded
	for (size_t x = 0; x < custom_dll.size(); x++)
	{
		if (_stricmp(custom_dll[x].name.c_str(), dllname) == 0 || _stricmp(custom_dll[x].fullname.c_str(), dllname) == 0)
		{
			return custom_dll[x].dll;
		}
	}

	// Logging
	if (EnableLogging)
	{
		Logging::Log() << "Loading " << dllname;
	}

	// Load default dll if not loading current dll
	if (_stricmp(Config.WrapperName.c_str(), dllname) != 0)
	{
		loadpath = dllname;
		dll = ::LoadLibraryA(loadpath);
	}

	// Load system dll
	if (!dll)
	{
		//Load library
		GetSystemDirectory(path, MAX_PATH);
		strcat_s(path, MAX_PATH, "\\");
		strcat_s(path, MAX_PATH, dllname);
		loadpath = path;
		dll = ::LoadLibraryA(loadpath);
	}

	// Store handle and dll name
	if (dll)
	{
		Logging::Log() << "Loaded library: " << loadpath;
		AddHandleToVector(dll, dllname);
	}

	// Return dll handle
	return dll;
}

// Load custom dll files
void Utils::LoadCustomDll()
{
	for (size_t x = 0; x < Config.LoadCustomDllPath.size(); ++x)
	{
		// Check if path is empty
		if (!Config.LoadCustomDllPath[x].empty())
		{
			// Load dll from ini
			auto h = LoadLibrary(Config.LoadCustomDllPath[x].c_str());

			// Cannot load dll
			if (h)
			{
				AddHandleToVector(h, Config.LoadCustomDllPath[x].c_str());
			}
			else
			{
				Logging::Log() << "Cannot load custom library: " << Config.LoadCustomDllPath[x];
			}
		}
	}
}

// Initialize ASI module
void Utils::InitializeASI(HMODULE hModule)
{
	if (!hModule)
	{
		return;
	}

	PFN_InitializeASI p_InitializeASI = (PFN_InitializeASI)GetProcAddress(hModule, "InitializeASI");

	if (!p_InitializeASI)
	{
		return;
	}

	p_InitializeASI();
}

// Find asi plugins to load
void Utils::FindFiles(WIN32_FIND_DATA* fd)
{
	char dir[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, dir);

	HANDLE asiFile = FindFirstFile("*.asi", fd);
	if (asiFile != INVALID_HANDLE_VALUE)
	{
		do {
			if (!(fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				auto pos = strlen(fd->cFileName);

				if (fd->cFileName[pos - 4] == '.' &&
					(fd->cFileName[pos - 3] == 'a' || fd->cFileName[pos - 3] == 'A') &&
					(fd->cFileName[pos - 2] == 's' || fd->cFileName[pos - 2] == 'S') &&
					(fd->cFileName[pos - 1] == 'i' || fd->cFileName[pos - 1] == 'I'))
				{
					char path[MAX_PATH] = { 0 };
					sprintf_s(path, "%s\\%s", dir, fd->cFileName);

					auto h = LoadLibrary(path);
					SetCurrentDirectory(dir); //in case asi switched it

					if (h)
					{
						AddHandleToVector(h, path);
						InitializeASI(h);
					}
					else
					{
						Logging::LogFormat("Unable to load '%s'. Error: %d", fd->cFileName, GetLastError());
					}
				}
			}
		} while (FindNextFile(asiFile, fd));
		FindClose(asiFile);
	}
}

// Load asi plugins
void Utils::LoadPlugins()
{
	Logging::Log() << "Loading ASI Plugins";

	char oldDir[MAX_PATH] = { 0 }; // store the current directory
	GetCurrentDirectory(MAX_PATH, oldDir);

	char selfPath[MAX_PATH] = { 0 };
	GetModuleFileName(hModule_dll, selfPath, MAX_PATH);
	*strrchr(selfPath, '\\') = '\0';
	SetCurrentDirectory(selfPath);

	WIN32_FIND_DATA fd;
	if (!Config.LoadFromScriptsOnly)
		FindFiles(&fd);

	SetCurrentDirectory(selfPath);

	if (SetCurrentDirectory("scripts\\"))
		FindFiles(&fd);

	SetCurrentDirectory(selfPath);

	if (SetCurrentDirectory("plugins\\"))
		FindFiles(&fd);

	SetCurrentDirectory(oldDir); // Reset the current directory
}

// Unload all dll files loaded by the wrapper
void Utils::UnloadAllDlls()
{
	// Logging
	Logging::Log() << "Unloading libraries...";

	// Unload custom libraries
	while (custom_dll.size() != 0)
	{
		// Unload dll
		FreeLibrary(custom_dll.back().dll);
		custom_dll.pop_back();
	}
}

HMEMORYMODULE Utils::LoadMemoryToDLL(LPVOID pMemory, DWORD Size)
{
	if (pMemory && Size)
	{
		return MemoryLoadLibrary(pMemory, Size);
	}
	return nullptr;
}

HMEMORYMODULE Utils::LoadResourceToMemory(DWORD ResID)
{
	HRSRC hResource = FindResource(hModule_dll, MAKEINTRESOURCE(ResID), RT_RCDATA);
	if (hResource)
	{
		HGLOBAL hLoadedResource = LoadResource(hModule_dll, hResource);
		if (hLoadedResource)
		{
			LPVOID pLockedResource = LockResource(hLoadedResource);
			if (pLockedResource)
			{
				DWORD dwResourceSize = SizeofResource(hModule_dll, hResource);
				if (dwResourceSize != 0)
				{
					return MemoryLoadLibrary(pLockedResource, dwResourceSize);
				}
			}
		}
	}
	return nullptr;
}

// Searches the memory
void *Utils::memmem(const void *l, size_t l_len, const void *s, size_t s_len)
{
	char *cur, *last;
	const char *cl = (const char *)l;
	const char *cs = (const char *)s;

	/* we need something to compare */
	if (!l_len || !s_len)
	{
		return nullptr;
	}

	/* "s" must be smaller or equal to "l" */
	if (l_len < s_len)
	{
		return nullptr;
	}

	/* special case where s_len == 1 */
	if (s_len == 1)
	{
		return (void*)memchr(l, (int)*cs, l_len);
	}

	/* the last position where it's possible to find "s" in "l" */
	last = (char *)cl + l_len - s_len;

	for (cur = (char *)cl; cur <= last; cur++)
	{
		if (cur[0] == cs[0] && !memcmp(cur, cs, s_len))
		{
			return cur;
		}
	}

	return nullptr;
}

// Reverse bit order
DWORD Utils::ReverseBits(DWORD v)
{
	v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);	// swap odd and even bits
	v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);	// swap consecutive pairs
	v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);	// swap nibbles
	v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);	// swap bytes
	return (v >> 16) | (v << 16);							// swap 2-byte long pairs
}

// Removes the artificial resolution limit from Direct3D7 and below
void Utils::DDrawResolutionHack(HMODULE hD3DIm)
{
	const BYTE wantedBytes[] = { 0xB8, 0x00, 0x08, 0x00, 0x00, 0x39 };

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hD3DIm;
	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((char *)pDosHeader + pDosHeader->e_lfanew);
	DWORD dwCodeBase = (DWORD)hD3DIm + pNtHeader->OptionalHeader.BaseOfCode;
	DWORD dwCodeSize = pNtHeader->OptionalHeader.SizeOfCode;
	DWORD dwOldProtect;

	DWORD dwPatchBase = (DWORD)memmem((void *)dwCodeBase, dwCodeSize, wantedBytes, sizeof(wantedBytes));
	if (dwPatchBase)
	{
		Logging::LogDebug() << __FUNCTION__ << " Found resolution check at: " << (void*)dwPatchBase;
		dwPatchBase++;
		VirtualProtect((LPVOID)dwPatchBase, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		*(DWORD *)dwPatchBase = (DWORD)-1;
		VirtualProtect((LPVOID)dwPatchBase, 4, dwOldProtect, &dwOldProtect);
	}
}

// Reset FPU if the _SW_INVALID flag is set
void Utils::ResetInvalidFPUState()
{
	if (_statusfp() & _SW_INVALID)
	{
		_clearfp();		// Clear invalid operation flag
		_statusfp();	// Re-read status after clearing
	}
}

void Utils::CheckMessageQueue(HWND hWnd)
{
	// Peek messages to help prevent a "Not Responding" window
	MSG msg = {};
	if (PeekMessage(&msg, hWnd, 0, 0, PM_NOREMOVE)) { BusyWaitYield((DWORD)-1); };
}

bool Utils::IsWindowsVistaOrNewer()
{
	HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");

	if (hKernel32)
	{
		// Check if any Windows Vista+ APIs exist
		if (GetProcAddress(hKernel32, "GetTickCount64") &&
			GetProcAddress(hKernel32, "InitializeCriticalSectionEx") &&
			GetProcAddress(hKernel32, "CancelSynchronousIo") &&
			GetProcAddress(hKernel32, "FlushProcessWriteBuffers"))
		{
			return true; // Windows Vista or newer
		}
	}

	return false; // Older than Windows Vista
}

bool Utils::IsWindows7OrNewer()
{
	HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");

	if (hKernel32)
	{
		// Check if any Windows 7+ APIs exist
		if (GetProcAddress(hKernel32, "TryAcquireSRWLockExclusive") &&
			GetProcAddress(hKernel32, "SetThreadGroupAffinity") &&
			GetProcAddress(hKernel32, "SetSearchPathMode") &&
			GetProcAddress(hKernel32, "GetLogicalProcessorInformationEx"))
		{
			return true; // Windows 7 or newer
		}
	}

	return false; // Older than Windows 7
}

bool Utils::IsWindows8OrNewer()
{
	HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");

	if (hKernel32)
	{
		// Check for several Windows 8+ APIs
		if (GetProcAddress(hKernel32, "GetSystemTimePreciseAsFileTime") &&
			GetProcAddress(hKernel32, "GetNumaNodeProcessorMaskEx") &&
			GetProcAddress(hKernel32, "GetNumaProximityNodeEx"))
		{
			return true; // Windows 8 or newer
		}
	}

	return false; // Older than Windows 8
}

void Utils::GetScreenSettings()
{
	// Store screen settings
	//hDC = GetDC(nullptr);
	//GetDeviceGammaRamp(hDC, lpRamp);  // <-- Hangs on this line starting in Windows 10 update 1903

#if (_WIN32_WINNT >= 0x0502)
	// Read values from the registry if they exist
	DWORD Size = sizeof(int);
	if (RegGetValue(HKEY_CURRENT_USER, "Volatile Environment", "DxWrapper_Font_Enabled", RRF_RT_REG_DWORD, nullptr, (PVOID)&fontSystemSettings.enabled, &Size) == ERROR_SUCCESS &&
		RegGetValue(HKEY_CURRENT_USER, "Volatile Environment", "DxWrapper_Font_Type", RRF_RT_REG_DWORD, nullptr, (PVOID)&fontSystemSettings.type, &Size) == ERROR_SUCCESS &&
		RegGetValue(HKEY_CURRENT_USER, "Volatile Environment", "DxWrapper_Font_Contrast", RRF_RT_REG_DWORD, nullptr, (PVOID)&fontSystemSettings.contrast, &Size) == ERROR_SUCCESS &&
		RegGetValue(HKEY_CURRENT_USER, "Volatile Environment", "DxWrapper_Font_Orientation", RRF_RT_REG_DWORD, nullptr, (PVOID)&fontSystemSettings.orientation, &Size) == ERROR_SUCCESS)
	{
		fontSystemSettings.isSet = true;
	}

	// Store font settings
	if (!fontSystemSettings.isSet &&
		SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &fontSystemSettings.enabled, 0) &&
		SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &fontSystemSettings.type, 0) &&
		SystemParametersInfo(SPI_GETFONTSMOOTHINGCONTRAST, 0, &fontSystemSettings.contrast, 0) &&
		SystemParametersInfo(SPI_GETFONTSMOOTHINGORIENTATION, 0, &fontSystemSettings.orientation, 0))
	{
		fontSystemSettings.isSet = true;
		RegSetKeyValue(HKEY_CURRENT_USER, "Volatile Environment", "DxWrapper_Font_Enabled", REG_DWORD, (PVOID)&fontSystemSettings.enabled, Size);
		RegSetKeyValue(HKEY_CURRENT_USER, "Volatile Environment", "DxWrapper_Font_Type", REG_DWORD, (PVOID)&fontSystemSettings.type, Size);
		RegSetKeyValue(HKEY_CURRENT_USER, "Volatile Environment", "DxWrapper_Font_Contrast", REG_DWORD, (PVOID)&fontSystemSettings.contrast, Size);
		RegSetKeyValue(HKEY_CURRENT_USER, "Volatile Environment", "DxWrapper_Font_Orientation", REG_DWORD, (PVOID)&fontSystemSettings.orientation, Size);
	}
#endif // _WIN32_WINNT >= 0x0502
}

void Utils::ResetScreenSettings()
{
	Logging::Log() << "Reseting screen resolution";

	// Reset Gamma Ramp
	if (hDC)
	{
		SetDeviceGammaRamp(hDC, lpRamp);  // <-- Hangs on this line starting in Windows 10 update 1903
		ReleaseDC(nullptr, hDC);
	}

	// Reset screen settings
	ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, CDS_RESET, nullptr);

#if (_WIN32_WINNT >= 0x0502)
	// Reset font settings
	if (fontSystemSettings.isSet)
	{
		Logging::Log() << "Reseting font smoothing";
		if (SystemParametersInfo(SPI_SETFONTSMOOTHING, fontSystemSettings.enabled, nullptr, SPIF_SENDCHANGE) &&
			SystemParametersInfo(SPI_SETFONTSMOOTHINGTYPE, 0, (LPVOID)fontSystemSettings.type, SPIF_SENDCHANGE) &&
			SystemParametersInfo(SPI_SETFONTSMOOTHINGCONTRAST, 0, (LPVOID)fontSystemSettings.contrast, SPIF_SENDCHANGE) &&
			SystemParametersInfo(SPI_SETFONTSMOOTHINGORIENTATION, 0, (LPVOID)fontSystemSettings.orientation, SPIF_SENDCHANGE))
		{
			// Delete registry keys
			RegDeleteKeyValueA(HKEY_CURRENT_USER, "Volatile Environment", "DxWrapper_Font_Enabled");
			RegDeleteKeyValueA(HKEY_CURRENT_USER, "Volatile Environment", "DxWrapper_Font_Type");
			RegDeleteKeyValueA(HKEY_CURRENT_USER, "Volatile Environment", "DxWrapper_Font_Contrast");
			RegDeleteKeyValueA(HKEY_CURRENT_USER, "Volatile Environment", "DxWrapper_Font_Orientation");
		}
	}
#endif // _WIN32_WINNT >= 0x0502

	// Redraw desktop window
	RedrawWindow(nullptr, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}

void Utils::ResetGamma()
{
	// Get the device context for the screen
	HDC hdc = ::GetDC(NULL);

	if (hdc)
	{
		// Create a default gamma ramp (linear ramp)
		WORD defaultRamp[3][256] = {};
		for (int i = 0; i < 256; i++)
		{
			WORD value = WORD(i * 257);  // Linear mapping (0 maps to 0, 255 maps to 65535)
			defaultRamp[0][i] = value;  // Red
			defaultRamp[1][i] = value;  // Green
			defaultRamp[2][i] = value;  // Blue
		}

		// Set the default gamma ramp
		if (!::SetDeviceGammaRamp(hdc, defaultRamp))
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to reset gamma ramp!";
		}

		// Release the device context
		::ReleaseDC(NULL, hdc);
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to get device context!";
	}
}

HWND Utils::GetTopLevelWindowOfCurrentProcess()
{
	HWND foregroundWindow = GetForegroundWindow();

	if (foregroundWindow)
	{
		DWORD foregroundProcessId;
		GetWindowThreadProcessId(foregroundWindow, &foregroundProcessId);

		DWORD currentProcessId = GetCurrentProcessId();

		if (foregroundProcessId == currentProcessId)
		{
			return foregroundWindow;
		}
	}

	return nullptr; // No top-level window found for the current process.
}

bool Utils::IsMainWindow(HWND hWnd)
{
	// A main window is a top-level window without a parent
	return GetWindow(hWnd, GW_OWNER) == nullptr && IsWindowVisible(hWnd);
}

HWND Utils::GetMainWindowForProcess(DWORD processId)
{
	struct {
		DWORD processID = 0;
		HWND mainWindow = nullptr;
	} lparam;
	lparam.processID = processId;

	EnumWindows([](HWND hWnd, LPARAM lParam) -> BOOL {
		DWORD windowProcessId = 0;
		GetWindowThreadProcessId(hWnd, &windowProcessId);

		if (windowProcessId == reinterpret_cast<DWORD*>(lParam)[0] && IsMainWindow(hWnd))
		{
			*reinterpret_cast<HWND*>(reinterpret_cast<DWORD*>(lParam) + 1) = hWnd;
			return FALSE; // Found the main window, stop enumeration
		}
		return TRUE; // Continue searching
		}, reinterpret_cast<LPARAM>(&lparam));

	return lparam.mainWindow;
}

bool Utils::IsWindowRectEqualOrLarger(HWND srchWnd, HWND desthWnd)
{
	RECT rect1, rect2;

	if (GetWindowRect(srchWnd, &rect1) && GetWindowRect(desthWnd, &rect2))
	{
		int width1 = rect1.right - rect1.left;
		int height1 = rect1.bottom - rect1.top;

		int width2 = rect2.right - rect2.left;
		int height2 = rect2.bottom - rect2.top;

		return (width1 >= width2) && (height1 >= height2);
	}

	// If GetWindowRect fails for either window, return false.
	return false;
}

static BOOL WINAPI kernel_CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	DEFINE_STATIC_PROC_ADDRESS(CreateProcessAFunc, CreateProcessA, Utils::p_CreateProcessA);

	if (!CreateProcessA)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";

		if (lpProcessInformation)
		{
			lpProcessInformation->dwProcessId = 0;
			lpProcessInformation->dwThreadId = 0;
			lpProcessInformation->hProcess = nullptr;
			lpProcessInformation->hThread = nullptr;
		}
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	if (stristr(lpCommandLine, "gameux.dll,GameUXShim", MAX_PATH))
	{
		Logging::Log() << __FUNCTION__ << " " << lpCommandLine;

		char CommandLine[MAX_PATH] = { '\0' };

		for (int x = 0; x < MAX_PATH && lpCommandLine && lpCommandLine[x] != ',' && lpCommandLine[x] != '\0'; x++)
		{
			CommandLine[x] = lpCommandLine[x];
		}

		return CreateProcessA(lpApplicationName, CommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
			lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}

	return CreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
		lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

static BOOL WINAPI kernel_CreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	DEFINE_STATIC_PROC_ADDRESS(CreateProcessWProc, CreateProcessW, Utils::p_CreateProcessW);

	if (!CreateProcessW)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";

		if (lpProcessInformation)
		{
			lpProcessInformation->dwProcessId = 0;
			lpProcessInformation->dwThreadId = 0;
			lpProcessInformation->hProcess = nullptr;
			lpProcessInformation->hThread = nullptr;
		}
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	if (wcsistr(lpCommandLine, L"gameux.dll,GameUXShim", MAX_PATH))
	{
		Logging::Log() << __FUNCTION__ << " " << lpCommandLine;

		wchar_t CommandLine[MAX_PATH] = { '\0' };

		for (int x = 0; x < MAX_PATH && lpCommandLine && lpCommandLine[x] != ',' && lpCommandLine[x] != '\0'; x++)
		{
			CommandLine[x] = lpCommandLine[x];
		}

		return CreateProcessW(lpApplicationName, CommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
			lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}

	return CreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
		lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

DWORD Utils::GetThreadIDByHandle(HANDLE hThread)
{
	HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
	GetThreadIdProc pGetThreadId = nullptr;
	if (kernel32)
	{
		pGetThreadId = (GetThreadIdProc)GetProcAddress(kernel32, "GetThreadId");
	}

	if (pGetThreadId)
	{
		return pGetThreadId(hThread);
	}
	else
	{
		// Create a snapshot of the system's current processes and threads.
		HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

		if (hThreadSnapshot != INVALID_HANDLE_VALUE)
		{
			THREADENTRY32 te32 = {};
			te32.dwSize = sizeof(THREADENTRY32);

			// Iterate through the thread list to find the thread with the specified handle.
			if (Thread32First(hThreadSnapshot, &te32))
			{
				do
				{
					HANDLE hThreadSnapshotHandle = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
					if (hThreadSnapshotHandle != NULL)
					{
						if (hThreadSnapshotHandle == hThread)
						{
							CloseHandle(hThreadSnapshot);
							CloseHandle(hThreadSnapshotHandle);
							return te32.th32ThreadID;
						}
						CloseHandle(hThreadSnapshotHandle);
					}
				} while (Thread32Next(hThreadSnapshot, &te32));
			}
			CloseHandle(hThreadSnapshot);
		}

		// Thread not found.
		return 0;
	}
}

void Utils::DisableGameUX()
{
	// Logging
	Logging::Log() << "Disabling Microsoft Game Explorer...";

	// Hook CreateProcess APIs
	Logging::Log() << "Hooking 'CreateProcess' API...";
	HMODULE h_kernel32 = GetModuleHandle("kernel32");
	InterlockedExchangePointer((PVOID*)&p_CreateProcessA, Hook::HotPatch(Hook::GetProcAddress(h_kernel32, "CreateProcessA"), "CreateProcessA", *kernel_CreateProcessA));
	InterlockedExchangePointer((PVOID*)&p_CreateProcessW, Hook::HotPatch(Hook::GetProcAddress(h_kernel32, "CreateProcessW"), "CreateProcessW", *kernel_CreateProcessW));
}

inline static UINT GetValueFromString(wchar_t* str)
{
	int num = 0;
	for (UINT i = 0; i < wcslen(str); i++)
	{
		if (iswdigit(str[i]))
		{
			num = (num * 10) + (str[i] - '0');
		}
		else if (num)
		{
			break;
		}
	}
	return num;
}

HRESULT Utils::GetVideoRam(UINT AdapterNo, DWORD& TotalMemory)
{
	struct ADLIST {
		DWORD DeviceID = 0;
		DWORD AdapterRAM = 0;
	};
	static std::vector<ADLIST> AdapterList;

	if (AdapterList.size())
	{
		for (auto& entry : AdapterList)
		{
			if (entry.DeviceID == AdapterNo)
			{
				TotalMemory = entry.AdapterRAM;
				return S_OK;
			}
		}
		return E_FAIL;
	}

	HRESULT hr = E_FAIL;
	HRESULT t_hr = E_FAIL;
	IWbemLocator* pLoc = NULL;
	IWbemServices* pSvc = NULL;
	IEnumWbemClassObject* pEnumerator = NULL;
	IWbemClassObject* pclsObj = NULL;
	ULONG uReturn = 0;

	HRESULT c_hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	bool AlreadyInitialized = (c_hr == S_FALSE || c_hr == RPC_E_CHANGED_MODE);
	if ((SUCCEEDED(c_hr) || AlreadyInitialized) &&
		SUCCEEDED(t_hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL)) &&
		SUCCEEDED(t_hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc)))
	{
		if (SUCCEEDED(t_hr = pLoc->ConnectServer(_bstr_t(L"root\\cimv2"), NULL, NULL, 0, NULL, 0, 0, &pSvc)))
		{
			if (SUCCEEDED(t_hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE)) &&
				SUCCEEDED(t_hr = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_VideoController"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator)))
			{
				while (pEnumerator)
				{
					t_hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
					if (FAILED(t_hr) || 0 == uReturn)
					{
						break;
					}

					VARIANT varId = {};

					t_hr = pclsObj->Get(L"DeviceID", 0, &varId, 0, 0);
					if (SUCCEEDED(t_hr))
					{
						UINT DeviceID = GetValueFromString(varId.bstrVal);
						VariantClear(&varId);

						VARIANT vtTotalMemory = {};

						t_hr = pclsObj->Get(L"AdapterRAM", 0, &vtTotalMemory, 0, 0);
						if (SUCCEEDED(t_hr))
						{
							Logging::Log() << __FUNCTION__ << " Found Video Memory on adapter: " << vtTotalMemory.ulVal / (1024 * 1024) << "MBs on DeviceID: " << DeviceID;

							ADLIST tmpItem = { DeviceID, vtTotalMemory.ulVal };
							AdapterList.push_back(tmpItem);

							// Check adapter number
							if (AdapterNo == DeviceID)
							{
								TotalMemory = vtTotalMemory.ulVal;
								hr = S_OK;
							}
							VariantClear(&vtTotalMemory);
						}
					}
					pclsObj->Release();
				}
				pEnumerator->Release();
			}
			pSvc->Release();
		}
		pLoc->Release();
	}
	if (c_hr == S_OK)
	{
		CoUninitialize();
	}
	else if (AlreadyInitialized)
	{
		Logging::Log() << __FUNCTION__ << " Warning: CoInitializeEx() was already initialized: " << Logging::hex(hr);
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: CoInitializeEx() returned an error: " << Logging::hex(hr);
	}

	return hr;
}

// Wait for window actions
void Utils::WaitForWindowActions(HWND hWnd, DWORD Loops)
{
	UINT x = 0;
	while (true)
	{
		// Peek at the messages without removing them
		MSG msg;
		if (PeekMessage(&msg, hWnd, 0, 0, PM_NOREMOVE))
		{
			// A message is available, but we won't process it here
			Sleep(0); // Allow other threads to run
		}
		else
		{
			// No messages in the queue, exit the loop
			break;
		}

		// Only loop limited times
		if (++x > Loops)
		{
			break;
		}
	}
}

// Single precision and exceptions disabled
void Utils::ApplyFPUSetup()
{
	unsigned int currentControl = 0;
	unsigned int newControl = _EM_INEXACT | _EM_UNDERFLOW | _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL;

	_controlfp_s(&currentControl, newControl, _MCW_EM);
}

void Utils::GetModuleFromAddress(void* address, char* module, const size_t size)
{
	if (!module || size == 0)
	{
		return;
	}

	module[0] = '\0'; // Ensure null-termination in case of failure

	HMODULE hModule = NULL;
	HANDLE hProcess = GetCurrentProcess();
	MEMORY_BASIC_INFORMATION mbi;

	// Query the memory region of the given address
	if (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)))
	{
		hModule = (HMODULE)mbi.AllocationBase;

		if (hModule)
		{
			if (GetModuleFileNameA(hModule, module, static_cast<DWORD>(size)) == 0)
			{
				module[0] = '\0'; // Ensure null-termination if GetModuleFileNameA() fails
			}
		}
	}
}

bool Utils::SetWndProcFilter(HWND hWnd)
{
	// Check window handle
	if (!IsWindow(hWnd))
	{
		Logging::Log() << __FUNCTION__ << " Error: hWnd invalid!";
		return false;
	}

	// Check if WndProc is already overloaded
	if (OriginalWndProc)
	{
		Logging::Log() << __FUNCTION__ << " Error: WndProc already overloaded!";
		return false;
	}

	LOG_LIMIT(3, __FUNCTION__ << " Setting new WndProc " << hWnd);

	// Store existing WndProc
	OriginalWndProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);

	// Set new WndProc
	if (!OriginalWndProc || !SetWindowLong(hWnd, GWL_WNDPROC, (LONG)WndProcFilter))
	{
		Logging::Log() << __FUNCTION__ << " Failed to overload WndProc!";
		OriginalWndProc = nullptr;
		return false;
	}

	return true;
}

bool Utils::RestoreWndProcFilter(HWND hWnd)
{
	// Check window handle
	if (!IsWindow(hWnd))
	{
		Logging::Log() << __FUNCTION__ << " Error: hWnd invalid!";
		return false;
	}

	// Check if WndProc is overloaded
	if (!OriginalWndProc)
	{
		Logging::Log() << __FUNCTION__ << " Error: WndProc is not yet overloaded!";
		return false;
	}

	// Get current WndProc
	WNDPROC CurrentWndProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);

	// Check if WndProc is overloaded
	if (CurrentWndProc != WndProcFilter)
	{
		Logging::Log() << __FUNCTION__ << " Error: WndProc does not match!";
		return false;
	}

	// Resetting WndProc
	if (!SetWindowLong(hWnd, GWL_WNDPROC, (LONG)OriginalWndProc))
	{
		Logging::Log() << __FUNCTION__ << " Failed to reset WndProc";
		return false;
	}

	OriginalWndProc = nullptr;
	return true;
}

LRESULT CALLBACK Utils::WndProcFilter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Logging::LogDebug() << __FUNCTION__ << " " << Logging::hex(uMsg);

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
