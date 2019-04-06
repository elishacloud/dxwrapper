#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Wrappers\dinput.h"

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

#define EXPORT_OUT_WRAPPED_PROC(procName, unused) \
	extern FARPROC procName ## _out;

namespace DinputWrapper
{
	VISIT_PROCS_DINPUT(DECLARE_IN_WRAPPED_PROC);
	const FARPROC DllCanUnloadNow_in = (FARPROC)*di_DllCanUnloadNow;
	const FARPROC DllGetClassObject_in = (FARPROC)*di_DllGetClassObject;
	const FARPROC DllRegisterServer_in = (FARPROC)*di_DllRegisterServer;
	const FARPROC DllUnregisterServer_in = (FARPROC)*di_DllUnregisterServer;

	extern FARPROC DirectInput8Create_out;
	extern FARPROC DllCanUnloadNow_out;
	extern FARPROC DllGetClassObject_out;
	extern FARPROC DllRegisterServer_out;
	extern FARPROC DllUnregisterServer_out;
}

#undef DECLARE_IN_WRAPPED_PROC
#undef EXPORT_OUT_WRAPPED_PROC
