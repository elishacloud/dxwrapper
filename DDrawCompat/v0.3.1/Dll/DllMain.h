#pragma once

#include "DDrawCompat\DDrawCompatExternal.h"

#undef DDrawCompatDefault
#define DDrawCompatDefault Compat31

#define DDRAWCOMPAT_31

namespace Compat31
{
	BOOL WINAPI DllMain_DDrawCompat(HINSTANCE, DWORD, LPVOID);

#define EXTERN_PROC_STUB(procName) extern "C" __declspec(dllexport) void DC30_ ## procName();
	VISIT_ALL_DDRAW_PROCS(EXTERN_PROC_STUB);
#undef EXTERN_PROC_STUB

#define INITIALIZE_WRAPPED_PROC(procName) \
	extern FARPROC procName ## _proc;

	VISIT_ALL_DDRAW_PROCS(INITIALIZE_WRAPPED_PROC);

#undef INITIALIZE_WRAPPED_PROC
}
