/**
* Copyright (C) 2017 Elisha Riedlinger
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
* http://www.blitzbasic.com/Community/post.php?topic=99477&post=1202996
*
* ASI plugin loader taken from source code found in Ultimate ASI Loader
* https://github.com/ThirteenAG/Ultimate-ASI-Loader
*/

#include "Settings\Settings.h"
#include "Dllmain\Dllmain.h"
#include "Wrappers\wrapper.h"
extern "C"
{
#include "Disasm\disasm.h"
}
#include "Hooking\Hook.h"
#include "Utils.h"
#include "Logging\Logging.h"

#undef LoadLibrary

typedef enum _PROCESS_DPI_AWARENESS {
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
typedef HRESULT(WINAPI* SetProcessDpiAwarenessProc)(PROCESS_DPI_AWARENESS value);
typedef FARPROC(WINAPI *GetProcAddressProc)(HMODULE, LPSTR);
typedef DWORD(WINAPI *GetModuleFileNameAProc)(HMODULE, LPSTR, DWORD);
typedef DWORD(WINAPI *GetModuleFileNameWProc)(HMODULE, LPWSTR, DWORD);
typedef HRESULT(__stdcall *SetAppCompatDataFunc)(DWORD, DWORD);
typedef LPTOP_LEVEL_EXCEPTION_FILTER(WINAPI *PFN_SetUnhandledExceptionFilter)(LPTOP_LEVEL_EXCEPTION_FILTER);

namespace Utils
{
	// Strictures
	struct type_dll
	{
		HMODULE dll;
		std::string name;
		std::string fullname;
	};

	// Declare varables
	FARPROC pGetProcAddress = nullptr;
	FARPROC pGetModuleFileNameA = nullptr;
	FARPROC pGetModuleFileNameW = nullptr;
	std::vector<type_dll> custom_dll;		// Used for custom dll's and asi plugins
	LPTOP_LEVEL_EXCEPTION_FILTER pOriginalSetUnhandledExceptionFilter = SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)EXCEPTION_CONTINUE_EXECUTION);
	PFN_SetUnhandledExceptionFilter pSetUnhandledExceptionFilter = reinterpret_cast<PFN_SetUnhandledExceptionFilter>(SetUnhandledExceptionFilter);

	// Function declarations
	void FindFiles(WIN32_FIND_DATA*);
	LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	LPTOP_LEVEL_EXCEPTION_FILTER WINAPI extSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER);
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

// Sets the proccess to single core affinity
void Utils::SetProcessAffinity()
{
	HANDLE hCurrentProcess = GetCurrentProcess();
	SetProcessAffinityMask(hCurrentProcess, 1);
	CloseHandle(hCurrentProcess);
}

// Sets application DPI aware which disables DPI virtulization/High DPI scaling for this process
void Utils::DisableHighDPIScaling()
{
	Logging::Log() << "Disabling High DPI Scaling...";
	HMODULE module = LoadLibrary("Shcore.dll");

	if (module)
	{
		SetProcessDpiAwarenessProc pSetProcessDpiAwareness = (SetProcessDpiAwarenessProc)GetProcAddress(module, "SetProcessDpiAwareness");
		if (pSetProcessDpiAwareness)
		{
			pSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
			return;
		}
	}
	Logging::Log() << "Failed to disable High DPI Scaling!";
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

	// SetAppCompatData
	if (appCompatFlag)
	{
		HMODULE module = LoadLibrary("ddraw.dll");
		FARPROC SetAppCompatDataPtr = (module != nullptr) ? GetProcAddress(module, "SetAppCompatData") : nullptr;
		if (module && SetAppCompatDataPtr)
		{
			SetAppCompatDataFunc SetAppCompatData = (SetAppCompatDataFunc)SetAppCompatDataPtr;
			for (DWORD x = 1; x <= 12; x++)
			{
				if (Config.DXPrimaryEmulation[x])
				{
					if (SetAppCompatData)
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
			Logging::Log() << "Cannnot open ddraw.dll to SetAppCompatData";
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
	if (InterlockedCompareExchangePointer((PVOID*)&pGetModuleFileNameA, nullptr, nullptr))
	{
		if (hModule == hModule_dll && Config.RealWrapperMode == dtype.dxwrapper)
		{
			hModule = nullptr;
			DWORD lSize = ((GetModuleFileNameAProc)InterlockedCompareExchangePointer((PVOID*)&pGetModuleFileNameA, nullptr, nullptr))(hModule, lpFilename, nSize);
			char *pdest = strrchr(lpFilename, '\\');
			if (pdest && lSize > 0 && nSize - lSize + strlen(dtypename[dtype.dxwrapper]) > 0)
			{
				strcpy_s(pdest + 1, nSize - lSize, dtypename[dtype.dxwrapper]);
				return nSize - lSize + strlen(dtypename[dtype.dxwrapper]);
			}
			return lSize;
		}
		else
		{
			return ((GetModuleFileNameAProc)InterlockedCompareExchangePointer((PVOID*)&pGetModuleFileNameA, nullptr, nullptr))(hModule, lpFilename, nSize);
		}
	}
	SetLastError(5);
	return 0;
}

// Update GetModuleFileNameW to fix module name
DWORD WINAPI Utils::GetModuleFileNameWHandler(HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
	if (InterlockedCompareExchangePointer((PVOID*)&pGetModuleFileNameW, nullptr, nullptr))
	{
		if (hModule == hModule_dll && Config.RealWrapperMode == dtype.dxwrapper)
		{
			hModule = nullptr;
			DWORD lSize = ((GetModuleFileNameWProc)InterlockedCompareExchangePointer((PVOID*)&pGetModuleFileNameW, nullptr, nullptr))(hModule, lpFilename, nSize);
			wchar_t *pdest = wcsrchr(lpFilename, '\\');
			std::string str(dtypename[dtype.dxwrapper]);
			std::wstring wrappername(str.begin(), str.end());
			if (pdest && lSize > 0 && nSize - lSize + strlen(dtypename[dtype.dxwrapper]) > 0)
			{
				wcscpy_s(pdest + 1, nSize - lSize, &wrappername[0]);
				return nSize - lSize + strlen(dtypename[dtype.dxwrapper]);
			}
			return lSize;
		}
		else
		{
			return ((GetModuleFileNameWProc)InterlockedCompareExchangePointer((PVOID*)&pGetModuleFileNameW, nullptr, nullptr))(hModule, lpFilename, nSize);
		}
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
		if (_strcmpi(custom_dll[x].name.c_str(), dllname) == 0 || _strcmpi(custom_dll[x].fullname.c_str(), dllname) == 0)
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
	if (_strcmpi(Config.WrapperName.c_str(), dllname) != 0 &&
		_strcmpi(dtypename[(Config.RealWrapperMode < sizeof(dtypename) / sizeof(dtypename[0])) ? Config.RealWrapperMode : 0], dllname) != 0)
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
