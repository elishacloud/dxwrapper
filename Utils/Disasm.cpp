/**
* Copyright (C) 2024 Elisha Riedlinger
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
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
extern "C"
{
#include "Disasm\disasm.h"
}
#include "Utils.h"
#include "External\Hooking\Hook.h"
#include "Logging\Logging.h"

typedef LPTOP_LEVEL_EXCEPTION_FILTER(WINAPI* PFN_SetUnhandledExceptionFilter)(LPTOP_LEVEL_EXCEPTION_FILTER);

namespace Utils
{
	LPTOP_LEVEL_EXCEPTION_FILTER pOriginalSetUnhandledExceptionFilter = SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)EXCEPTION_CONTINUE_EXECUTION);
	PFN_SetUnhandledExceptionFilter pSetUnhandledExceptionFilter = reinterpret_cast<PFN_SetUnhandledExceptionFilter>(SetUnhandledExceptionFilter);

	// Function declarations
	LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	LPTOP_LEVEL_EXCEPTION_FILTER WINAPI extSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER);
}

// Add filter for UnhandledExceptionFilter used by the exception handler to catch exceptions
LONG WINAPI Utils::myUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
	Logging::Log() << "UnhandledExceptionFilter: exception" << std::showbase << std::hex <<
		" code=" << ExceptionInfo->ExceptionRecord->ExceptionCode <<
		" flags=" << ExceptionInfo->ExceptionRecord->ExceptionFlags <<
		" addr=" << ExceptionInfo->ExceptionRecord->ExceptionAddress << std::dec << std::noshowbase;
	DWORD oldprot;
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
		cmdlen = Disasm((BYTE*)target, 10, 0, &da, 0, nullptr, nullptr);
		Logging::Log() << "UnhandledExceptionFilter: NOP opcode=" << std::showbase << std::hex << *(BYTE*)target << std::dec << std::noshowbase << " len=" << cmdlen;
		memset((BYTE*)target, 0x90, cmdlen);
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
	void* tmp;

	Logging::Log() << "Set exception handler";
	HMODULE dll = GetModuleHandleA("kernel32.dll");
	if (!dll)
	{
		Logging::Log() << "Failed to load kernel32.dll!";
		return;
	}
	// override default exception handler, if any....
	LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	tmp = Hook::HotPatch(UnhandledExceptionFilter, "UnhandledExceptionFilter", myUnhandledExceptionFilter);
	// so far, no need to save the previous handler, but anyway...
	tmp = Hook::HotPatch(SetUnhandledExceptionFilter, "SetUnhandledExceptionFilter", extSetUnhandledExceptionFilter);
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
