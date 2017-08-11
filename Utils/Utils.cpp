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
* Code in GetVersionFile function taken from source code found on stackoverflow.com
* https://stackoverflow.com/questions/940707/how-do-i-programmatically-get-the-version-of-a-dll-or-exe-file
*
* Code in DisableHighDPIScaling function taken from source code found in Aqrit's ddwrapper
* http://bitpatch.com/ddwrapper.html
*/

#include "Settings\Settings.h"
#include "Wrappers\wrapper.h"
#include "Dllmain\Dllmain.h"
extern "C"
{
#include "Disasm\disasm.h"
}
#include "Hooking\Hook.h"
#include "Utils.h"
#include "Logging\Logging.h"

typedef HRESULT(__stdcall *SetAppCompatDataFunc)(DWORD, DWORD);
typedef LPTOP_LEVEL_EXCEPTION_FILTER(WINAPI *PFN_SetUnhandledExceptionFilter)(LPTOP_LEVEL_EXCEPTION_FILTER);

namespace Utils
{
	// Declare varables
	LPTOP_LEVEL_EXCEPTION_FILTER pOriginalSetUnhandledExceptionFilter = SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)EXCEPTION_CONTINUE_EXECUTION);
	PFN_SetUnhandledExceptionFilter pSetUnhandledExceptionFilter = reinterpret_cast<PFN_SetUnhandledExceptionFilter>(SetUnhandledExceptionFilter);
	Hook::HOOKVARS h_UnhandledExceptionFilter;
	Hook::HOOKVARS h_SetUnhandledExceptionFilter;

	// Function declarations
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

// DPI virtualization causes:
// Text Clipping, Blurring, or Inconsistent font sizes.
// "Rendering of full-screen DX applications partially off screen" - Mircosoft
// ...drawing(writting) to someplace that doesn't exist may cause crashes... 
// if your going to Disable DPI Scaling then do it as soon as possible
void Utils::DisableHighDPIScaling()
{
	Logging::Log() << "Disabling High DPI Scaling...";
	// use GetProcAddress because SetProcessDPIAware exists only on win6+
	// and "High" dpi scaling only exits on win6+?
	HMODULE hUser32 = GetModuleHandle("user32.dll");
	typedef bool(__stdcall* SetProcessDPIAwareFunc)();
	if (hUser32)
	{
		SetProcessDPIAwareFunc setDPIAware = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
		if (setDPIAware)
		{
			setDPIAware();
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

	// SetAppCompatData see: http://www.blitzbasic.com/Community/post.php?topic=99477&post=1202996
	if (appCompatFlag)
	{
		HMODULE module = Wrapper::LoadDll(dtype.ddraw);
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
	// override default exception handler, if any....
	LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	h_UnhandledExceptionFilter.apiproc = UnhandledExceptionFilter;
	h_UnhandledExceptionFilter.hookproc = myUnhandledExceptionFilter;
	tmp = Hook::HookAPI(hModule_dll, "KERNEL32.dll", h_UnhandledExceptionFilter.apiproc, "UnhandledExceptionFilter", h_UnhandledExceptionFilter.hookproc);
	// so far, no need to save the previous handler, but anyway...
	h_SetUnhandledExceptionFilter.apiproc = SetUnhandledExceptionFilter;
	h_SetUnhandledExceptionFilter.hookproc = extSetUnhandledExceptionFilter;
	tmp = Hook::HookAPI(hModule_dll, "KERNEL32.dll", h_SetUnhandledExceptionFilter.apiproc, "SetUnhandledExceptionFilter", h_SetUnhandledExceptionFilter.hookproc);
	if (tmp)
	{
		pSetUnhandledExceptionFilter = reinterpret_cast<PFN_SetUnhandledExceptionFilter>(tmp);
	}

	SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	pSetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)myUnhandledExceptionFilter);
}

// Unhooks the exception handler
void Utils::UnHookExceptionHandler(void)
{
	Compat::Log() << "Unloading exception handlers";
	Hook::UnhookAPI(hModule_dll, "KERNEL32.dll", h_UnhandledExceptionFilter.apiproc, "UnhandledExceptionFilter", h_UnhandledExceptionFilter.hookproc);
	Hook::UnhookAPI(hModule_dll, "KERNEL32.dll", h_SetUnhandledExceptionFilter.apiproc, "SetUnhandledExceptionFilter", h_SetUnhandledExceptionFilter.hookproc);
	SetErrorMode(0);
	SetUnhandledExceptionFilter(pOriginalSetUnhandledExceptionFilter);
	Finishdisasm();
}
