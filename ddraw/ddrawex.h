#pragma once

#include <ddraw.h>
#include "Wrappers\wrapper.h"

#define VISIT_PROCS_DDRAWEX(visit) \
	visit(DllCanUnloadNow, jmpaddr) \
	visit(DllGetClassObject, jmpaddr) \
	visit(DllRegisterServer, jmpaddr) \
	visit(DllUnregisterServer, jmpaddr)

typedef HRESULT(WINAPI* DllRegisterServerProc)();
typedef HRESULT(WINAPI* DllUnregisterServerProc)();

HRESULT WINAPI ddex_DllCanUnloadNow();
HRESULT WINAPI ddex_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
HRESULT WINAPI ddex_DllRegisterServer();
HRESULT WINAPI ddex_DllUnregisterServer();

#define DECLARE_IN_WRAPPED_PROC(procName, unused) \
	const FARPROC procName ## _in = (FARPROC)*ddex_ ## procName;

namespace ddrawex
{
	VISIT_PROCS_DDRAWEX(DECLARE_IN_WRAPPED_PROC);
	VISIT_PROCS_DDRAWEX(EXPORT_OUT_WRAPPED_PROC);
	VISIT_PROCS_DDRAWEX(DECLARE_PROC_VARIABLES);
}

#undef DECLARE_IN_WRAPPED_PROC
