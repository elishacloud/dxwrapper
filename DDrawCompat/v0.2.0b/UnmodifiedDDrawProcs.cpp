#include "DDrawProcs.h"

namespace Compat20
{
#define CREATE_DDRAW_PROC_STUB(procName) \
	extern "C" __declspec(naked) void __stdcall DC20_ ## procName() \
	{ \
		__asm jmp Compat::origProcs.procName \
	}

	VISIT_UNMODIFIED_DDRAW_PROCS(CREATE_DDRAW_PROC_STUB)
}
