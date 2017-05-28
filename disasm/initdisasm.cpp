/**
* Created from source code found in DxWnd v2.03.99
* https://sourceforge.net/projects/dxwnd/
*
* Updated 2017 by Elisha Riedlinger
*/

#include "cfg.h"
#include "dllmain.h"
#include "disasm.h"
#include "hook\inithook.h"

static HMODULE LoadDisasm();

typedef LPTOP_LEVEL_EXCEPTION_FILTER (WINAPI *SetUnhandledExceptionFilter_Type)(LPTOP_LEVEL_EXCEPTION_FILTER);
typedef char *(*Geterrwarnmessage_Type)(unsigned long, unsigned long);
typedef int(*Preparedisasm_Type)(void);
typedef void(*Finishdisasm_Type)(void);
typedef unsigned long(*Disasm_Type)(const unsigned char *, unsigned long, unsigned long, t_disasm *, int, t_config *, int(*)(tchar *, unsigned long));

SetUnhandledExceptionFilter_Type pSetUnhandledExceptionFilter;
Geterrwarnmessage_Type pGeterrwarnmessage;
Preparedisasm_Type pPreparedisasm;
Finishdisasm_Type pFinishdisasm;
Disasm_Type pDisasm;

#pragma warning (disable : 4706)
LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
//#ifdef _DEBUG
	Compat::Log() << "UnhandledExceptionFilter: exception code=" <<
		ExceptionInfo->ExceptionRecord->ExceptionCode << 
		" flags=" << ExceptionInfo->ExceptionRecord->ExceptionFlags << std::showbase << std::hex <<
		" addr=" << ExceptionInfo->ExceptionRecord->ExceptionAddress << std::dec << std::noshowbase;
//#endif
	DWORD oldprot;
	static HMODULE disasmlib = NULL;
	PVOID target = ExceptionInfo->ExceptionRecord->ExceptionAddress;
	switch (ExceptionInfo->ExceptionRecord->ExceptionCode) {
	case 0xc0000094: // IDIV reg (Ultim@te Race Pro)
	case 0xc0000095: // DIV by 0 (divide overflow) exception (SonicR)
	case 0xc0000096: // CLI Priviliged instruction exception (Resident Evil), FB (Asterix & Obelix)
	case 0xc000001d: // FEMMS (eXpendable)
	case 0xc0000005: // Memory exception (Tie Fighter)
		int cmdlen;
		t_disasm da;
		if (!disasmlib) {
			if (!(disasmlib = LoadDisasm())) return EXCEPTION_CONTINUE_SEARCH;
			(*pPreparedisasm)();
		}
		if (!VirtualProtect(target, 10, PAGE_READWRITE, &oldprot)) return EXCEPTION_CONTINUE_SEARCH; // error condition
		cmdlen = (*pDisasm)((BYTE *)target, 10, 0, &da, 0, NULL, NULL);
		Compat::Log() << "UnhandledExceptionFilter: NOP opcode=" << std::showbase << std::hex << *(BYTE *)target << std::dec << std::noshowbase << " len=" << cmdlen;
		memset((BYTE *)target, 0x90, cmdlen);
		VirtualProtect(target, 10, oldprot, &oldprot);
		if (!FlushInstructionCache(GetCurrentProcess(), target, cmdlen))
		{
			Compat::Log() << "UnhandledExceptionFilter: FlushInstructionCache ERROR target=" << std::showbase << std::hex << target << std::dec << std::noshowbase <<", err=" << GetLastError();
		}
		// skip replaced opcode
		ExceptionInfo->ContextRecord->Eip += cmdlen; // skip ahead op-code length
		return EXCEPTION_CONTINUE_EXECUTION;
		break;
	default:
		return EXCEPTION_CONTINUE_SEARCH;
	}
}
#pragma warning (default : 4706)

#pragma warning (disable : 4100)
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI extSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
#ifdef _DEBUG
	Compat::Log() << "SetUnhandledExceptionFilter: lpExceptionFilter=" << lpTopLevelExceptionFilter;
#endif
	extern LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	return (*pSetUnhandledExceptionFilter)(myUnhandledExceptionFilter);
}
#pragma warning (default : 4100)

static HMODULE LoadDisasm()
{
	HMODULE disasmlib;

	// Get dxwrapper path
	char buffer[MAX_PATH];
	GetModuleFileName(hModule_dll, buffer, MAX_PATH);

	disasmlib = LoadLibrary(buffer);
	if (!disasmlib) {
		Compat::Log() << "Load lib=" << buffer << " failed err=" << GetLastError();
		return NULL;
	}
	pGeterrwarnmessage = (Geterrwarnmessage_Type)(*GetProcAddress)(disasmlib, "Geterrwarnmessage");
	pPreparedisasm = (Preparedisasm_Type)(*GetProcAddress)(disasmlib, "Preparedisasm");
	pFinishdisasm = (Finishdisasm_Type)(*GetProcAddress)(disasmlib, "Finishdisasm");
	pDisasm = (Disasm_Type)(*GetProcAddress)(disasmlib, "Disasm");
#ifdef _DEBUG
	Compat::Log() << "Load " << buffer << " ptrs=" << pGeterrwarnmessage << std::showbase << std::hex << pPreparedisasm << pFinishdisasm << pDisasm << std::dec << std::noshowbase;
#endif
	return disasmlib;
}

void HookExceptionHandler(void)
{
	void *tmp;

	Compat::Log() << "Set exception handler";
	pSetUnhandledExceptionFilter = SetUnhandledExceptionFilter;
	// override default exception handler, if any....
	LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	tmp = HookAPI(hModule_dll, "KERNEL32.dll", UnhandledExceptionFilter, "UnhandledExceptionFilter", myUnhandledExceptionFilter);
	// so far, no need to save the previous handler, but anyway...
	tmp = HookAPI(hModule_dll, "KERNEL32.dll", SetUnhandledExceptionFilter, "SetUnhandledExceptionFilter", extSetUnhandledExceptionFilter);
	if (tmp) pSetUnhandledExceptionFilter = (SetUnhandledExceptionFilter_Type)tmp;

	SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	(*pSetUnhandledExceptionFilter)((LPTOP_LEVEL_EXCEPTION_FILTER)myUnhandledExceptionFilter);
}

void UnHookExceptionHandler(void)
{
	// Not sure how to unload exception handler
	// Unloading it sometimes crashes the program
	// Todo: look into this later
	//Compat::Log() << "Unload exception handlers";
	//(*pFinishdisasm)();
}