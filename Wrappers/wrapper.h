#pragma once

#define ADD_FARPROC_MEMBER(memberName) FARPROC memberName = jmpaddr;

#define CREATE_PROC_STUB(procName) \
	extern "C" __declspec(naked) void __stdcall Fake ## procName() \
	{ \
		__asm jmp module.procName \
	}

#define	LOAD_ORIGINAL_PROC(procName) \
	module.procName = GetFunctionAddress(module.dll, #procName, jmpaddr);

HRESULT CallReturn();
static constexpr FARPROC jmpaddr = (FARPROC)*CallReturn;

void DllAttach();
HMODULE LoadDll(DWORD);
void DllDetach();