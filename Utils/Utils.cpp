/**
* Copyright (C) 2023 Elisha Riedlinger
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
#include <tlhelp32.h>
#include <atlbase.h>
#include <comdef.h>
#include <comutil.h>
#include <Wbemidl.h>
#include "Utils.h"
#include "Settings\Settings.h"
#include "Dllmain\Dllmain.h"
#include "Wrappers\wrapper.h"
#include "d3d8\d3d8External.h"
#include "d3d9\d3d9External.h"
#include "External\Hooking\Hook.h"
#include "Logging\Logging.h"

#undef LoadLibrary

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
typedef BOOL(WINAPI *CreateProcessWFunc)(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);

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

	FARPROC p_CreateProcessA = nullptr;
	FARPROC p_CreateProcessW = nullptr;
	WNDPROC OriginalWndProc = nullptr;
	std::vector<type_dll> custom_dll;		// Used for custom dll's and asi plugins

	// Function declarations
	DWORD_PTR GetProcessMask();
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

// Check the number CPU cores is being used by the process
DWORD Utils::GetCoresUsedByProcess()
{
	int numCores = 0;
	DWORD_PTR ProcessAffinityMask, SystemAffinityMask;
	if (GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffinityMask, &SystemAffinityMask))
	{
		while (ProcessAffinityMask)
		{
			if (ProcessAffinityMask & 1)
			{
				++numCores;
			}
			ProcessAffinityMask >>= 1;
		}
	}
	return numCores;
}

// Get processor mask
DWORD_PTR Utils::GetProcessMask()
{
	static DWORD_PTR nMask = 0;
	if (nMask)
	{
		return nMask;
	}

	DWORD_PTR ProcessAffinityMask, SystemAffinityMask;
	if (GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffinityMask, &SystemAffinityMask))
	{
		DWORD_PTR AffinityLow = 1;
		while (AffinityLow && (AffinityLow & SystemAffinityMask) == 0)
		{
			AffinityLow <<= 1;
		}
		if (AffinityLow)
		{
			nMask = ((AffinityLow << (Config.SingleProcAffinity - 1)) & SystemAffinityMask) ? (AffinityLow << (Config.SingleProcAffinity - 1)) : AffinityLow;
		}
	}

	Logging::Log() << __FUNCTION__ << " Setting CPU mask: " << Logging::hex(nMask);
	return nMask;
}

// Set Single Core Affinity
void Utils::SetProcessAffinity()
{
	Logging::Log() << "Setting SingleCoreAffinity...";
	SetProcessAffinityMask(GetCurrentProcess(), GetProcessMask());
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
			HRESULT result = SUCCEEDED(setProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE));

			setDpiAware = (result == S_OK || result == E_ACCESSDENIED);
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

void Utils::BusyWaitYield()
{
#if (_WIN32_WINNT >= 0x0502)
	YieldProcessor();
#else
	Sleep(0);
#endif
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

void Utils::CheckMessageQueue(HWND hwnd)
{
	// Peek messages to help prevent a "Not Responding" window
	MSG msg = {};
	if (PeekMessage(&msg, hwnd, 0, 0, PM_NOREMOVE)) { BusyWaitYield(); };
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

BOOL WINAPI CreateProcessAHandler(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
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

BOOL WINAPI CreateProcessWHandler(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	DEFINE_STATIC_PROC_ADDRESS(CreateProcessWFunc, CreateProcessW, Utils::p_CreateProcessW);

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
	GetThreadIdProc pGetThreadId = (GetThreadIdProc)GetProcAddress(LoadLibrary("kernel32.dll"), "GetThreadId");

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
	InterlockedExchangePointer((PVOID*)&p_CreateProcessA, Hook::HotPatch(Hook::GetProcAddress(h_kernel32, "CreateProcessA"), "CreateProcessA", *CreateProcessAHandler));
	InterlockedExchangePointer((PVOID*)&p_CreateProcessW, Hook::HotPatch(Hook::GetProcAddress(h_kernel32, "CreateProcessW"), "CreateProcessW", *CreateProcessWHandler));
}

inline UINT GetValueFromString(wchar_t* str)
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

DWORD Utils::GetVideoRam(UINT AdapterNo)
{
	UNREFERENCED_PARAMETER(AdapterNo);

	DWORD retSize = 0;

#if (_WIN32_WINNT >= 0x0502)
	// Initialize COM
	HRESULT hr = CoInitialize(nullptr);
	if (FAILED(hr))
	{
		// Handle error
		Logging::Log() << __FUNCTION__ << " Error: Failed to CoInitialize.";
		return retSize;
	}

	do {
		// Create an instance of the IWbemLocator interface
		CComPtr<IWbemLocator> spLoc = nullptr;
		hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_SERVER, IID_IWbemLocator, (LPVOID*)&spLoc);
		if (FAILED(hr))
		{
			// Handle error
			Logging::Log() << __FUNCTION__ << " Error: Failed to CoCreateInstance.";
			break;
		}

		// Connect to the root\cimv2 namespace with the IWbemServices interface
		CComPtr<IWbemServices> spServices = nullptr;
		hr = spLoc->ConnectServer(CComBSTR(L"root\\cimv2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &spServices);
		if (FAILED(hr))
		{
			// Handle error
			Logging::Log() << __FUNCTION__ << " Error: Failed to connect to the root\\cimv2 namespace.";
			break;
		}

		// Set the security levels for the proxy
		hr = CoSetProxyBlanket(spServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
		if (FAILED(hr))
		{
			// Handle error
			Logging::Log() << __FUNCTION__ << " Error: Failed to set the security levels.";
			break;
		}

		// Create an enumerator for Win32_VideoController instances
		CComPtr<IEnumWbemClassObject> spEnumInst = nullptr;
		hr = spServices->CreateInstanceEnum(CComBSTR(L"Win32_VideoController"), WBEM_FLAG_SHALLOW, nullptr, &spEnumInst);
		if (FAILED(hr))
		{
			// Handle error
			Logging::Log() << __FUNCTION__ << " Error: Failed to create an enumerator for Win32_VideoController instances.";
			break;
		}

		// Loop through the instances and retrieve the video RAM
		ULONG uNumOfInstances = 0;
		do {
			CComPtr<IWbemClassObject> spInstance = nullptr;
			hr = spEnumInst->Next(WBEM_INFINITE, 1, &spInstance, &uNumOfInstances);
			if (SUCCEEDED(hr) && uNumOfInstances)
			{
				// Get the DeviceID property from the instance
				CComVariant varId;
				hr = spInstance->Get(CComBSTR(L"DeviceID"), 0, &varId, nullptr, nullptr);
				if (SUCCEEDED(hr))
				{
					UINT VideoAdapter = GetValueFromString(varId.bstrVal);

					// Check adapter number
					if (AdapterNo == VideoAdapter)
					{
						// Get the AdapterRAM property from the instance
						CComVariant varSize;
						if (SUCCEEDED(spInstance->Get(CComBSTR(L"AdapterRAM"), 0, &varSize, nullptr, nullptr)))
						{
							Logging::LogDebug() << __FUNCTION__ << " Found AdapterRAM on adapter: " << VideoAdapter << " Size: " << varSize.intVal;
							retSize = varSize.intVal;
							break;
						}
					}
				}
			}
		} while (SUCCEEDED(hr) && uNumOfInstances);

	} while (false);

	CoUninitialize();
#endif // _WIN32_WINNT >= 0x0502

	return retSize;
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
