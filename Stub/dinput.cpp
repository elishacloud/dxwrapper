/**
* Copyright (C) 2019 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#define DIRECTINPUT_VERSION 0x0700

#include <dinput.h>
#include "..\Wrappers\wrapper.h"

// dinput proc typedefs
typedef HRESULT(WINAPI *DirectInputCreateAProc)(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* lplpDirectInput, LPUNKNOWN punkOuter);
typedef HRESULT(WINAPI *DirectInputCreateWProc)(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* lplpDirectInput, LPUNKNOWN punkOuter);
typedef HRESULT(WINAPI *DirectInputCreateExProc)(HINSTANCE hinst, DWORD dwVersion, REFIID riid, LPVOID* lplpDD, LPUNKNOWN punkOuter);
typedef HRESULT(WINAPI *DllCanUnloadNowProc)();
typedef HRESULT(WINAPI *DllGetClassObjectProc)(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv);
typedef HRESULT(WINAPI *DllRegisterServerProc)();
typedef HRESULT(WINAPI *DllUnregisterServerProc)();

namespace DinputWrapper
{
	char dllname[MAX_PATH];

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	procName ## Proc m_p ## procName = nullptr;

	VISIT_PROCS_DINPUT(INITIALIZE_WRAPPED_PROC);
	VISIT_PROCS_DINPUT_SHARED(INITIALIZE_WRAPPED_PROC);

#undef INITIALIZE_WRAPPED_PROC
}

using namespace DinputWrapper;

void InitDinput()
{
	static bool RunOnce = true;

	if (RunOnce)
	{
		// Load dll
		HMODULE dinputdll = LoadLibraryA(dllname);

		// Get function addresses
#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	m_p ## procName = (procName ## Proc)GetProcAddress(dinputdll, #procName);

		VISIT_PROCS_DINPUT(INITIALIZE_WRAPPED_PROC);
		VISIT_PROCS_DINPUT_SHARED(INITIALIZE_WRAPPED_PROC);

#undef INITIALIZE_WRAPPED_PROC

		RunOnce = false;
	}
}

HRESULT WINAPI di_DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* lplpDirectInput, LPUNKNOWN punkOuter)
{
	InitDinput();

	if (!m_pDirectInputCreateA)
	{
		return E_FAIL;
	}

	return m_pDirectInputCreateA(hinst, dwVersion, lplpDirectInput, punkOuter);
}

HRESULT WINAPI di_DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riid, LPVOID * lplpDD, LPUNKNOWN punkOuter)
{
	InitDinput();

	if (!m_pDirectInputCreateEx)
	{
		return E_FAIL;
	}

	return m_pDirectInputCreateEx(hinst, dwVersion, riid, lplpDD, punkOuter);
}

HRESULT WINAPI di_DirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* lplpDirectInput, LPUNKNOWN punkOuter)
{
	InitDinput();

	if (!m_pDirectInputCreateW)
	{
		return E_FAIL;
	}

	return m_pDirectInputCreateW(hinst, dwVersion, lplpDirectInput, punkOuter);
}

HRESULT WINAPI di_DllCanUnloadNow()
{
	InitDinput();

	if (!m_pDllCanUnloadNow)
	{
		return E_FAIL;
	}

	return m_pDllCanUnloadNow();
}

HRESULT WINAPI di_DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	InitDinput();

	if (!m_pDllGetClassObject)
	{
		return E_FAIL;
	}

	return m_pDllGetClassObject(rclsid, riid, ppv);
}

HRESULT WINAPI di_DllRegisterServer()
{
	InitDinput();

	if (!m_pDllRegisterServer)
	{
		return E_FAIL;
	}

	return m_pDllRegisterServer();
}

HRESULT WINAPI di_DllUnregisterServer()
{
	InitDinput();

	if (!m_pDllUnregisterServer)
	{
		return E_FAIL;
	}

	return m_pDllUnregisterServer();
}

void StartDinput(const char *name)
{
	if (name)
	{
		strcpy_s(dllname, MAX_PATH, name);
	}
	else
	{
		GetSystemDirectoryA(dllname, MAX_PATH);
		strcat_s(dllname, "\\dinput.dll");
	}

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	dinput::procName ## _var = (FARPROC)*di_ ## procName;

	VISIT_PROCS_DINPUT(INITIALIZE_WRAPPED_PROC);
	VISIT_PROCS_DINPUT_SHARED(INITIALIZE_WRAPPED_PROC);
}
