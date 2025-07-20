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
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Utils.h"
#include "External\Hooking\Disasm.h"
#include "External\Hooking\Hook.h"
#include "Logging\Logging.h"

namespace Utils
{
	static LPTOP_LEVEL_EXCEPTION_FILTER g_previousFilter = nullptr;

	// Forward declarations
	LONG WINAPI CustomUnhandledExceptionFilter(LPEXCEPTION_POINTERS exceptionInfo);
	void SetCustomExceptionHandler();
	void RemoveCustomExceptionHandler();
}

LONG WINAPI Utils::CustomUnhandledExceptionFilter(LPEXCEPTION_POINTERS exceptionInfo)
{
	void* faultAddr = exceptionInfo->ExceptionRecord->ExceptionAddress;
	DWORD code = exceptionInfo->ExceptionRecord->ExceptionCode;

	Logging::Log() << __FUNCTION__ << " Exception caught at address: " << faultAddr << ", code: 0x" << Logging::hex(code);

	// Simulate instruction patching (NOP fill) for specific exceptions
	switch (code)
	{
	case 0xC0000094: // Divide by zero
	case 0xC0000095: // Integer overflow
	case 0xC0000096: // Privileged instruction
	case 0xC000001D: // Illegal instruction
	case 0xC0000005: // Access violation
	{
		// Use your custom disassembler to get the instruction length
		unsigned instrLen = Disasm::getInstructionLength(faultAddr);
		if (instrLen == 0 || instrLen > 15) // Sanity check
		{
			Logging::Log() << __FUNCTION__ << " Invalid instruction length, skipping patch.";
			return EXCEPTION_CONTINUE_SEARCH;
		}

		DWORD oldProtect = 0;
		if (VirtualProtect(faultAddr, instrLen, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			memset(faultAddr, 0x90, instrLen); // Patch with NOPs
			VirtualProtect(faultAddr, instrLen, oldProtect, &oldProtect);
			FlushInstructionCache(GetCurrentProcess(), faultAddr, instrLen);

			exceptionInfo->ContextRecord->Eip += instrLen;
			Logging::Log() << __FUNCTION__ << " Patched instruction with NOPs, continuing execution.";
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		else
		{
			Logging::Log() << __FUNCTION__ << " VirtualProtect failed, error=" << GetLastError();
		}
		break;
	}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

void Utils::SetCustomExceptionHandler()
{
	Logging::Log() << "Installing custom unhandled exception filter...";

	// Save previous filter
	g_previousFilter = SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);

	// Optionally suppress error popups
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
}

void Utils::RemoveCustomExceptionHandler()
{
	Logging::Log() << "Removing custom unhandled exception filter...";

	// Restore previous filter
	SetUnhandledExceptionFilter(g_previousFilter);

	// Restore default error mode
	SetErrorMode(0);
}
