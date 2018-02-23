#include "Dll/Procs.h"

#define CREATE_PROC_STUB(procName) \
	extern "C" __declspec(naked) void __stdcall _ ## procName() \
	{ \
		__asm jmp Dll::g_origProcs.procName \
	}

VISIT_UNMODIFIED_PROCS(CREATE_PROC_STUB)
