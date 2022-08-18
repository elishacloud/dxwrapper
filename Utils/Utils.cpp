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
*
* Exception handling code taken from source code found in DxWnd v2.03.99
* https://sourceforge.net/projects/dxwnd/
*
* SetAppCompatData code created based on information from here:
* http://www.mojolabs.nz/posts.php?topic=99477
*
* ASI plugin loader taken from source code found in Ultimate ASI Loader
* https://github.com/ThirteenAG/Ultimate-ASI-Loader
*
* DDrawResolutionHack taken from source code found in LegacyD3DResolutionHack
* https://github.com/UCyborg/LegacyD3DResolutionHack
*
* GetVideoRam taken from source code found in doom3.gpl
* https://github.com/TTimo/doom3.gpl
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
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
extern "C"
{
#include "Disasm\disasm.h"
}
#include "External\Hooking\Hook.h"
#include "Utils.h"
#include "Logging\Logging.h"

#undef LoadLibrary

typedef enum PROCESS_DPI_AWARENESS {
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
typedef void(WINAPI *PFN_InitializeASI)(void);
typedef HRESULT(WINAPI *SetProcessDpiAwarenessProc)(PROCESS_DPI_AWARENESS value);
typedef BOOL(WINAPI *SetProcessDPIAwareProc)();
typedef BOOL(WINAPI *SetProcessDpiAwarenessContextProc)(DPI_AWARENESS_CONTEXT value);
typedef FARPROC(WINAPI *GetProcAddressProc)(HMODULE, LPSTR);
typedef DWORD(WINAPI *GetModuleFileNameAProc)(HMODULE, LPSTR, DWORD);
typedef DWORD(WINAPI *GetModuleFileNameWProc)(HMODULE, LPWSTR, DWORD);
typedef HRESULT(WINAPI *SetAppCompatDataFunc)(DWORD, DWORD);
typedef int(WINAPI *Direct3DEnableMaximizedWindowedModeShimProc)(BOOL);
typedef LPTOP_LEVEL_EXCEPTION_FILTER(WINAPI *PFN_SetUnhandledExceptionFilter)(LPTOP_LEVEL_EXCEPTION_FILTER);
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
	FARPROC pGetProcAddress = nullptr;
	FARPROC pGetModuleFileNameA = nullptr;
	FARPROC pGetModuleFileNameW = nullptr;
	FARPROC p_CreateProcessA = nullptr;
	FARPROC p_CreateProcessW = nullptr;
	Direct3DEnableMaximizedWindowedModeShimProc m_pDirect3D9WindowedModeShim = nullptr;
	std::vector<type_dll> custom_dll;		// Used for custom dll's and asi plugins
	LPTOP_LEVEL_EXCEPTION_FILTER pOriginalSetUnhandledExceptionFilter = SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)EXCEPTION_CONTINUE_EXECUTION);
	PFN_SetUnhandledExceptionFilter pSetUnhandledExceptionFilter = reinterpret_cast<PFN_SetUnhandledExceptionFilter>(SetUnhandledExceptionFilter);

	// Function declarations
	DWORD_PTR GetProcessMask();
	int WINAPI DisableMaximizedWindowedMode(BOOL mEnable);
	void InitializeASI(HMODULE hModule);
	void FindFiles(WIN32_FIND_DATA*);
	LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	LPTOP_LEVEL_EXCEPTION_FILTER WINAPI extSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER);
	void *memmem(const void *l, size_t l_len, const void *s, size_t s_len);
}

int WINAPI Utils::DisableMaximizedWindowedMode(BOOL)
{
	if (!m_pDirect3D9WindowedModeShim)
	{
		return NULL;
	}

	return m_pDirect3D9WindowedModeShim(FALSE);
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
}

// Sets Application Compatibility Toolkit options for DXPrimaryEmulation using SetAppCompatData API
void Utils::SetAppCompat()
{
	// Check if any DXPrimaryEmulation flags is set
	bool appCompatFlag = false;
	for (UINT x = 1; x <= 12; x++)
	{
		if (Config.DXPrimaryEmulation[x])
		{
			appCompatFlag = true;
		}
	}

	// SetAppCompatData for DirectDraw
	if (appCompatFlag)
	{
		// Load ddraw.dll from System32
		char path[MAX_PATH];
		GetSystemDirectoryA(path, MAX_PATH);
		strcat_s(path, MAX_PATH, "\\ddraw.dll");
		HMODULE dll = LoadLibrary(path);

		if (dll)
		{
			SetAppCompatDataFunc SetAppCompatData = (SetAppCompatDataFunc)GetProcAddress(dll, "SetAppCompatData");
			if (SetAppCompatData)
			{
				for (DWORD x = 1; x <= 12; x++)
				{
					if (Config.DXPrimaryEmulation[x])
					{
						Logging::Log() << "SetAppCompatData: " << x;
						// For LockColorkey, this one uses the second parameter
						if (x == AppCompatDataType.LockColorkey)
						{
							(SetAppCompatData)(x, Config.LockColorkey);
						}
						// For all the other items
						else
						{
							(SetAppCompatData)(x, 0);
						}
					}
				}
			}
		}
		else
		{
			Logging::Log() << "Cannnot open ddraw.dll to SetAppCompatData!";
		}
	}

	// Disable MaxWindowedMode
	if (Config.DXPrimaryEmulation[AppCompatDataType.DisableMaxWindowedMode])
	{
		// Load d3d8.dll from System32
		char path[MAX_PATH];
		GetSystemDirectoryA(path, MAX_PATH);
		strcat_s(path, MAX_PATH, "\\d3d8.dll");
		HMODULE dll = LoadLibrary(path);

		if (dll)
		{
			// Get function address
			BYTE *addr = (BYTE*)Hook::GetProcAddress(dll, "Direct3D8EnableMaximizedWindowedModeShim");
			if (addr)
			{
				if (addr[0] == 0xC7 && addr[1] == 0x05)
				{
					Logging::Log() << "Disabling MaximizedWindowedMode for Direct3D8!";

					// Update function to disable Maximized Windowed Mode
					DWORD Protect;
					VirtualProtect((LPVOID)(addr + 6), 4, PAGE_EXECUTE_READWRITE, &Protect);
					*(DWORD*)(addr + 6) = 0;
					VirtualProtect((LPVOID)(addr + 6), 4, Protect, &Protect);

					// Launch function to disable Maximized Windowed Mode
					Direct3DEnableMaximizedWindowedModeShimProc Direct3D8EnableMaximizedWindowedMode = (Direct3DEnableMaximizedWindowedModeShimProc)addr;
					Direct3D8EnableMaximizedWindowedMode(0);
				}
			}
		}
		else
		{
			Logging::Log() << "Cannnot open d3d8.dll to disable MaximizedWindowedMode!";
		}

		// Load d3d9.dll from System32
		GetSystemDirectoryA(path, MAX_PATH);
		strcat_s(path, MAX_PATH, "\\d3d9.dll");
		dll = LoadLibrary(path);

		if (dll)
		{
			// Get function address
			BYTE *addr = (BYTE*)Hook::GetProcAddress(dll, "Direct3D9EnableMaximizedWindowedModeShim");
			if (addr)
			{
				Logging::Log() << "Disabling MaximizedWindowedMode for Direct3D9!";

				// Launch function to disable Maximized Windowed Mode
				Direct3DEnableMaximizedWindowedModeShimProc Direct3D9EnableMaximizedWindowedMode = (Direct3DEnableMaximizedWindowedModeShimProc)addr;
				Direct3D9EnableMaximizedWindowedMode(0);

				// Hook function to keep Maximized Windowed Mode disabled
				m_pDirect3D9WindowedModeShim = (Direct3DEnableMaximizedWindowedModeShimProc)Hook::HookAPI(dll, "d3d9.dll", Direct3D9EnableMaximizedWindowedMode, "Direct3D9EnableMaximizedWindowedModeShim", DisableMaximizedWindowedMode);
			}
		}
		else
		{
			Logging::Log() << "Cannnot open d3d9.dll to disable MaximizedWindowedMode!";
		}
	}
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
	FARPROC ProAddr = nullptr;

	if (InterlockedCompareExchangePointer((PVOID*)&pGetProcAddress, nullptr, nullptr))
	{
		ProAddr = ((GetProcAddressProc)InterlockedCompareExchangePointer((PVOID*)&pGetProcAddress, nullptr, nullptr))(hModule, lpProcName);
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
	GetModuleFileNameAProc org_GetModuleFileName = (GetModuleFileNameAProc)InterlockedCompareExchangePointer((PVOID*)&pGetModuleFileNameA, nullptr, nullptr);

	if (org_GetModuleFileName)
	{
		DWORD ret = org_GetModuleFileName(hModule, lpFilename, nSize);

		if (lpFilename[0] != '\\' && lpFilename[1] != '\\' && lpFilename[2] != '\\' && lpFilename[3] != '\\')
		{
			DWORD lSize = org_GetModuleFileName(nullptr, lpFilename, nSize);
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
	GetModuleFileNameWProc org_GetModuleFileName = (GetModuleFileNameWProc)InterlockedCompareExchangePointer((PVOID*)&pGetModuleFileNameW, nullptr, nullptr);

	if (org_GetModuleFileName)
	{
		DWORD ret = org_GetModuleFileName(hModule, lpFilename, nSize);

		if (lpFilename[0] != '\\' && lpFilename[1] != '\\' && lpFilename[2] != '\\' && lpFilename[3] != '\\')
		{
			DWORD lSize = org_GetModuleFileName(nullptr, lpFilename, nSize);
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

// Add filter for UnhandledExceptionFilter used by the exception handler to catch exceptions
LONG WINAPI Utils::myUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
	Logging::Log() << "UnhandledExceptionFilter: exception code=" << ExceptionInfo->ExceptionRecord->ExceptionCode <<
		" flags=" << ExceptionInfo->ExceptionRecord->ExceptionFlags << std::showbase << std::hex <<
		" addr=" << ExceptionInfo->ExceptionRecord->ExceptionAddress << std::dec << std::noshowbase;
	DWORD oldprot;
	static HMODULE disasmlib = nullptr;
	PVOID target = ExceptionInfo->ExceptionRecord->ExceptionAddress;
	switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
	{
	case 0xc0000094: // IDIV reg (Ultim@te Race Pro)
	case 0xc0000095: // DIV by 0 (divide overflow) exception (SonicR)
	case 0xc0000096: // CLI Priviliged instruction exception (Resident Evil), FB (Asterix & Obelix)
	case 0xc000001d: // FEMMS (eXpendable)
	case 0xc0000005: // Memory exception (Tie Fighter)
	{
		int cmdlen;
		t_disasm da;
		Preparedisasm();
		if (!VirtualProtect(target, 10, PAGE_READWRITE, &oldprot))
		{
			return EXCEPTION_CONTINUE_SEARCH; // error condition
		}
		cmdlen = Disasm((BYTE *)target, 10, 0, &da, 0, nullptr, nullptr);
		Logging::Log() << "UnhandledExceptionFilter: NOP opcode=" << std::showbase << std::hex << *(BYTE *)target << std::dec << std::noshowbase << " len=" << cmdlen;
		memset((BYTE *)target, 0x90, cmdlen);
		VirtualProtect(target, 10, oldprot, &oldprot);
		HANDLE hCurrentProcess = GetCurrentProcess();
		if (!FlushInstructionCache(hCurrentProcess, target, cmdlen))
		{
			Logging::Log() << "UnhandledExceptionFilter: FlushInstructionCache ERROR target=" << std::showbase << std::hex << target << std::dec << std::noshowbase << ", err=" << GetLastError();
		}
		CloseHandle(hCurrentProcess);
		// skip replaced opcode
		ExceptionInfo->ContextRecord->Eip += cmdlen; // skip ahead op-code length
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	break;
	default:
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

// Add filter for SetUnhandledExceptionFilter used by the exception handler to catch exceptions
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI Utils::extSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
#ifdef _DEBUG
	Logging::Log() << "SetUnhandledExceptionFilter: lpExceptionFilter=" << lpTopLevelExceptionFilter;
#else
	UNREFERENCED_PARAMETER(lpTopLevelExceptionFilter);
#endif
	extern LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	return pSetUnhandledExceptionFilter(myUnhandledExceptionFilter);
}

// Sets the exception handler by hooking UnhandledExceptionFilter
void Utils::HookExceptionHandler(void)
{
	void *tmp;

	Logging::Log() << "Set exception handler";
	HMODULE dll = LoadLibrary("kernel32.dll");
	if (!dll)
	{
		Logging::Log() << "Failed to load kernel32.dll!";
		return;
	}
	// override default exception handler, if any....
	LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	tmp = Hook::HookAPI(dll, "kernel32.dll", UnhandledExceptionFilter, "UnhandledExceptionFilter", myUnhandledExceptionFilter);
	// so far, no need to save the previous handler, but anyway...
	tmp = Hook::HookAPI(dll, "kernel32.dll", SetUnhandledExceptionFilter, "SetUnhandledExceptionFilter", extSetUnhandledExceptionFilter);
	if (tmp)
	{
		pSetUnhandledExceptionFilter = reinterpret_cast<PFN_SetUnhandledExceptionFilter>(tmp);
	}

	SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	pSetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)myUnhandledExceptionFilter);
	Logging::Log() << "Finished setting exception handler";
}

// Unhooks the exception handler
void Utils::UnHookExceptionHandler(void)
{
	Logging::Log() << "Unloading exception handlers";
	SetErrorMode(0);
	SetUnhandledExceptionFilter(pOriginalSetUnhandledExceptionFilter);
	Finishdisasm();
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
	HMODULE hModule = NULL;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)Utils::LoadPlugins, &hModule);
	GetModuleFileName(hModule, selfPath, MAX_PATH);
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

void Utils::GetScreenSettings()
{
	// Store screen settings
	//hDC = GetDC(nullptr);
	//GetDeviceGammaRamp(hDC, lpRamp);  // <-- Hangs on this line starting in Windows 10 update 1903

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

	// Redraw desktop window
	RedrawWindow(nullptr, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}

BOOL WINAPI CreateProcessAHandler(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	static CreateProcessAFunc org_CreateProcess = (CreateProcessAFunc)InterlockedCompareExchangePointer((PVOID*)&Utils::p_CreateProcessA, nullptr, nullptr);

	if (!org_CreateProcess)
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

		return org_CreateProcess(lpApplicationName, CommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
			lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}

	return org_CreateProcess(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
		lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

BOOL WINAPI CreateProcessWHandler(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	static CreateProcessWFunc org_CreateProcess = (CreateProcessWFunc)InterlockedCompareExchangePointer((PVOID*)&Utils::p_CreateProcessW, nullptr, nullptr);

	if (!org_CreateProcess)
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

		return org_CreateProcess(lpApplicationName, CommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
			lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}

	return org_CreateProcess(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
		lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
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

DWORD Utils::GetVideoRam(DWORD DefaultSize)
{
	DWORD retSize = DefaultSize;

	CComPtr<IWbemLocator> spLoc = NULL;
	HRESULT hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_SERVER, IID_IWbemLocator, (LPVOID *)&spLoc);
	if (hr != S_OK || spLoc == NULL)
	{
		return retSize;
	}

	CComBSTR bstrNamespace(_T("\\\\.\\root\\CIMV2"));
	CComPtr<IWbemServices> spServices;

	// Connect to CIM
	hr = spLoc->ConnectServer(bstrNamespace, NULL, NULL, 0, NULL, 0, 0, &spServices);
	if (hr != WBEM_S_NO_ERROR)
	{
		return retSize;
	}

	// Switch the security level to IMPERSONATE so that provider will grant access to system-level objects.  
	hr = CoSetProxyBlanket(spServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	if (hr != S_OK)
	{
		return retSize;
	}

	// Get the vid controller
	CComPtr<IEnumWbemClassObject> spEnumInst = NULL;
	hr = spServices->CreateInstanceEnum(CComBSTR("Win32_VideoController"), WBEM_FLAG_SHALLOW, NULL, &spEnumInst);
	if (hr != WBEM_S_NO_ERROR || spEnumInst == NULL)
	{
		return retSize;
	}

	ULONG uNumOfInstances = 0;
	CComPtr<IWbemClassObject> spInstance = NULL;
	hr = spEnumInst->Next(10000, 1, &spInstance, &uNumOfInstances);

	if (hr == S_OK && spInstance)
	{
		// Get properties from the object
		CComVariant varSize;
		hr = spInstance->Get(CComBSTR(_T("AdapterRAM")), 0, &varSize, 0, 0);
		if (hr == S_OK)
		{
			retSize = (varSize.intVal) ? (DWORD)varSize.intVal : retSize;
		}
	}

	return retSize;
}
