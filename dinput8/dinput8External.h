#pragma once

#define INITGUID

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "Wrappers\wrapper.h"

HRESULT WINAPI di8_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter);
HRESULT WINAPI di8_DllCanUnloadNow();
HRESULT WINAPI di8_DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv);
HRESULT WINAPI di8_DllRegisterServer();
HRESULT WINAPI di8_DllUnregisterServer();
LPCDIDATAFORMAT WINAPI di8_GetdfDIJoystick();

#define DECLARE_IN_WRAPPED_PROC(procName, unused) \
	const FARPROC procName ## _in = (FARPROC)*di8_ ## procName;

#define EXPORT_OUT_WRAPPED_PROC(procName, unused) \
	extern FARPROC procName ## _out;

namespace Dinput8Wrapper
{
	VISIT_PROCS_DINPUT8(DECLARE_IN_WRAPPED_PROC);
	const FARPROC DllGetClassObject_in = (FARPROC)*di8_DllGetClassObject;
	const FARPROC DllCanUnloadNow_in = (FARPROC)*di8_DllCanUnloadNow;
	const FARPROC DllRegisterServer_in = (FARPROC)*di8_DllRegisterServer;
	const FARPROC DllUnregisterServer_in = (FARPROC)*di8_DllUnregisterServer;

	VISIT_PROCS_DINPUT8(EXPORT_OUT_WRAPPED_PROC);
	extern FARPROC DllGetClassObject_out;
	extern FARPROC DllCanUnloadNow_out;
	extern FARPROC DllRegisterServer_out;
	extern FARPROC DllUnregisterServer_out;
}

#undef DECLARE_IN_WRAPPED_PROC
#undef EXPORT_OUT_WRAPPED_PROC
