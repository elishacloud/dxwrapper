#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Wrappers\wrapper.h"

typedef struct IDirectInputA *LPDIRECTINPUTA;
typedef struct IDirectInputW *LPDIRECTINPUTW;

HRESULT WINAPI di_DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* lplpDirectInput, LPUNKNOWN punkOuter);
HRESULT WINAPI di_DirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* lplpDirectInput, LPUNKNOWN punkOuter);
HRESULT WINAPI di_DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riid, LPVOID * lplpDD, LPUNKNOWN punkOuter);
HRESULT WINAPI di_DllCanUnloadNow();
HRESULT WINAPI di_DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv);
HRESULT WINAPI di_DllRegisterServer();
HRESULT WINAPI di_DllUnregisterServer();

#define DECLARE_IN_WRAPPED_PROC(procName, unused) \
	const FARPROC procName ## _in = (FARPROC)*di_ ## procName;

namespace DinputWrapper
{
	VISIT_PROCS_DINPUT(DECLARE_IN_WRAPPED_PROC);
	VISIT_PROCS_DINPUT_SHARED(DECLARE_IN_WRAPPED_PROC);

	VISIT_PROCS_DINPUT_SHARED(EXPORT_OUT_WRAPPED_PROC);
	EXPORT_OUT_WRAPPED_PROC(DirectInput8Create, unused);
}

#undef DECLARE_IN_WRAPPED_PROC
