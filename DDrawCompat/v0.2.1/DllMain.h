#pragma once

#include "DDrawCompat\DDrawCompatExternal.h"

#undef DDrawCompatDefault
#define DDrawCompatDefault Compat21

#define DDRAWCOMPAT_21

namespace Compat21
{
	BOOL WINAPI DllMain_DDrawCompat(HINSTANCE, DWORD, LPVOID);

	extern "C" HRESULT WINAPI DC21_DirectDrawCreate(GUID*, LPDIRECTDRAW*, IUnknown*);
	extern "C" HRESULT WINAPI DC21_DirectDrawCreateEx(GUID*, LPVOID*, REFIID, IUnknown*);
	extern "C" HRESULT WINAPI DC21_DllGetClassObject(REFCLSID, REFIID, LPVOID*);

#define EXTERN_PROC_STUB(procName) extern "C" void __stdcall DC21_ ## procName();
	VISIT_UNMODIFIED_DDRAW_PROCS(EXTERN_PROC_STUB);
#undef EXTERN_PROC_STUB

#define INITUALIZE_WRAPPED_PROC(procName) \
	const FARPROC procName ## _proc = (FARPROC)*DC21_ ## procName;

	VISIT_ALL_DDRAW_PROCS(INITUALIZE_WRAPPED_PROC);

#undef INITUALIZE_WRAPPED_PROC
}
