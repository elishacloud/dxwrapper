#pragma once

#include "DDrawCompat\DDrawCompatExternal.h"

#define DDRAWCOMPAT_21

namespace Compat21
{
	void Prepair_DDrawCompat();
	BOOL WINAPI DllMain_DDrawCompat(HINSTANCE, DWORD, LPVOID);

	extern "C" HRESULT WINAPI DC21_DirectDrawCreate(GUID*, LPDIRECTDRAW*, IUnknown*);
	extern "C" HRESULT WINAPI DC21_DirectDrawCreateEx(GUID*, LPVOID*, REFIID, IUnknown*);
	extern "C" HRESULT WINAPI DC21_DllGetClassObject(REFCLSID, REFIID, LPVOID*);

#define INITIALIZE_WRAPPED_PROC(procName) \
	extern FARPROC procName ## _proc;

	VISIT_ALL_DDRAW_PROCS(INITIALIZE_WRAPPED_PROC);

#undef INITIALIZE_WRAPPED_PROC
}
