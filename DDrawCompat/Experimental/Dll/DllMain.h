#pragma once

#include "DDrawCompat\DDrawCompatExternal.h"

#undef DDrawCompatDefault
#define DDrawCompatDefault CompatExperimental

#define DDRAWCOMPAT_EXP

namespace CompatExperimental
{
	BOOL WINAPI DllMain_DDrawCompat(HINSTANCE, DWORD, LPVOID);

	extern "C" HRESULT WINAPI _DirectDrawCreate(GUID*, LPDIRECTDRAW*, IUnknown*);
	extern "C" HRESULT WINAPI _DirectDrawCreateEx(GUID*, LPVOID*, REFIID, IUnknown*);
	extern "C" HRESULT WINAPI _DllGetClassObject(REFCLSID, REFIID, LPVOID*);

#define EXTERN_PROC_STUB(procName) extern "C" void __stdcall _ ## procName();
	VISIT_UNMODIFIED_DDRAW_PROCS(EXTERN_PROC_STUB);
#undef EXTERN_PROC_STUB

#define INITUALIZE_WRAPPED_PROC(procName) \
	const FARPROC procName ## _proc = (FARPROC)*_ ## procName;

	VISIT_ALL_DDRAW_PROCS(INITUALIZE_WRAPPED_PROC);

#undef INITUALIZE_WRAPPED_PROC
}
