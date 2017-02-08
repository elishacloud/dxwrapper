/**
* Created from source code found in DxWnd v 2.03.60
* https://sourceforge.net/projects/dxwnd/
*
* Updated 2017 by Elisha Riedlinger
*
*/

#include "dgame.h"
#include "disasm\disasm.h"

typedef LPTOP_LEVEL_EXCEPTION_FILTER
(WINAPI *SetUnhandledExceptionFilter_Type)(LPTOP_LEVEL_EXCEPTION_FILTER);

SetUnhandledExceptionFilter_Type pSetUnhandledExceptionFilter;

typedef char *(*Geterrwarnmessage_Type)(unsigned long, unsigned long);
typedef int(*Preparedisasm_Type)(void);
typedef void(*Finishdisasm_Type)(void);
typedef unsigned long(*Disasm_Type)(const unsigned char *, unsigned long, unsigned long, t_disasm *, int, t_config *, int(*)(tchar *, unsigned long));

Geterrwarnmessage_Type pGeterrwarnmessage;
Preparedisasm_Type pPreparedisasm;
Finishdisasm_Type pFinishdisasm;
Disasm_Type pDisasm;

void *HookAPI(HMODULE module, char *dll, void *apiproc, const char *apiname, void *hookproc)
{
	/*if (dxw.dwTFlags & OUTIMPORTTABLE) OutTrace("HookAPI: module=%x dll=%s apiproc=%x apiname=%s hookproc=%x\n",
		module, dll, apiproc, apiname, hookproc);*/

	if (!*apiname) { // check
		//char *sMsg = "HookAPI: NULL api name\n";
		//OutTraceE(sMsg);
		//if (IsAssertEnabled) MessageBox(0, sMsg, "HookAPI", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	//Hopatch
	//if (dxw.dwFlags4 & HOTPATCHALWAYS) {
	if (false) {
		void *orig;
		orig = HotPatch(apiproc, apiname, hookproc);
		if (orig) return orig;
	}

	return IATPatch(module, 0, dll, apiproc, apiname, hookproc);
}

static HMODULE LoadDisasm()
{
	HMODULE disasmlib;

	// Get dxwrapper path
	char buffer[MAX_PATH];
	GetModuleFileName(hModule_dll, buffer, MAX_PATH);

	disasmlib = LoadLibrary(buffer);
	if (!disasmlib) {
		//OutTraceDW("DXWND: Load lib=\"%s\" failed err=%d\n", "disasm.dll", GetLastError());
		return NULL;
	}
	pGeterrwarnmessage = (Geterrwarnmessage_Type)(*GetProcAddress)(disasmlib, "Geterrwarnmessage");
	pPreparedisasm = (Preparedisasm_Type)(*GetProcAddress)(disasmlib, "Preparedisasm");
	pFinishdisasm = (Finishdisasm_Type)(*GetProcAddress)(disasmlib, "Finishdisasm");
	pDisasm = (Disasm_Type)(*GetProcAddress)(disasmlib, "Disasm");
	//OutTraceDW("DXWND: Load disasm.dll ptrs=%x,%x,%x,%x\n", pGeterrwarnmessage, pPreparedisasm, pFinishdisasm, pDisasm);
	return disasmlib;
}

#pragma warning (disable : 4706)
LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
	/*OutTrace("UnhandledExceptionFilter: exception code=%x flags=%x addr=%x\n",
		ExceptionInfo->ExceptionRecord->ExceptionCode,
		ExceptionInfo->ExceptionRecord->ExceptionFlags,
		ExceptionInfo->ExceptionRecord->ExceptionAddress);*/
	if (Config.Debug) Compat::Log() << "UnhandledExceptionFilter: exception code=" <<
		ExceptionInfo->ExceptionRecord->ExceptionCode << 
		" flags=" << ExceptionInfo->ExceptionRecord->ExceptionFlags << 
		" addr=" << ExceptionInfo->ExceptionRecord->ExceptionAddress;
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
		//OutTrace("UnhandledExceptionFilter: NOP opcode=%x len=%d\n", *(BYTE *)target, cmdlen);
		if (Config.Debug) Compat::Log() << "UnhandledExceptionFilter: NOP opcode=" << *(BYTE *)target << " len=" << cmdlen;
		memset((BYTE *)target, 0x90, cmdlen);
		VirtualProtect(target, 10, oldprot, &oldprot);
		if (!FlushInstructionCache(GetCurrentProcess(), target, cmdlen))
		{
			//OutTrace("UnhandledExceptionFilter: FlushInstructionCache ERROR target=%x, err=%x\n", target, GetLastError());
		}
		// v2.03.10 skip replaced opcode
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
	//OutTraceDW("SetUnhandledExceptionFilter: lpExceptionFilter=%x\n", lpTopLevelExceptionFilter);
	extern LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	return (*pSetUnhandledExceptionFilter)(myUnhandledExceptionFilter);
}
#pragma warning (default : 4100)

void HookExceptionHandler(void)
{
	void *tmp;
	HMODULE base;

	Compat::Log() << "Set exception handlers";
	base = GetModuleHandle(NULL);
	pSetUnhandledExceptionFilter = SetUnhandledExceptionFilter;
	//v2.1.75 override default exception handler, if any....
	LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	tmp = HookAPI(base, "KERNEL32.dll", UnhandledExceptionFilter, "UnhandledExceptionFilter", myUnhandledExceptionFilter);
	// so far, no need to save the previous handler, but anyway...
	tmp = HookAPI(base, "KERNEL32.dll", SetUnhandledExceptionFilter, "SetUnhandledExceptionFilter", extSetUnhandledExceptionFilter);
	if (tmp) pSetUnhandledExceptionFilter = (SetUnhandledExceptionFilter_Type)tmp;

	SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	(*pSetUnhandledExceptionFilter)((LPTOP_LEVEL_EXCEPTION_FILTER)myUnhandledExceptionFilter);
}

void UnHookExceptionHandler(void)
{
	// Not sure how to unload exception handler
	//Compat::Log() << "Unload exception handlers";
	//(*pFinishdisasm)();
}
