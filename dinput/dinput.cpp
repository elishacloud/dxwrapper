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

#include "Wrappers\dinput.h"
#include "External\dinputto8\resource.h"
#include "External\dinputto8\dinputto8.h"

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	FARPROC procName ## _out = nullptr;

namespace DinputWrapper
{
	VISIT_PROCS_DINPUT(INITIALIZE_WRAPPED_PROC);
	FARPROC DllCanUnloadNow_out = nullptr;
	FARPROC DllGetClassObject_out = nullptr;
	FARPROC DllRegisterServer_out = nullptr;
	FARPROC DllUnregisterServer_out = nullptr;
}

using namespace DinputWrapper;

bool InitFlag = false;
DWORD diVersion = 0;

AddressLookupTableDinput<void> ProxyAddressLookupTable = AddressLookupTableDinput<void>();

DirectInput8CreateProc m_pDirectInput8Create = nullptr;
DllCanUnloadNowProc m_pDllCanUnloadNow = nullptr;
DllGetClassObjectProc m_pDllGetClassObject = nullptr;
DllRegisterServerProc m_pDllRegisterServer = nullptr;
DllUnregisterServerProc m_pDllUnregisterServer = nullptr;

HRESULT WINAPI di_DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riid, LPVOID * lplpDD, LPUNKNOWN punkOuter);

void InitDinput8()
{
	// Check if already initialized
	if (InitFlag)
	{
		return;
	}
	InitFlag = true;

	Logging::Log() << "Starting dinputto8 v" << APP_VERSION;

	// Load dll
	HMODULE dinput8dll = LoadLibraryA("dinput8.dll");

	// Get function addresses
	m_pDirectInput8Create = (DirectInput8CreateProc)GetProcAddress(dinput8dll, "DirectInput8Create");
	m_pDllCanUnloadNow = (DllCanUnloadNowProc)GetProcAddress(dinput8dll, "DllCanUnloadNow");
	m_pDllGetClassObject = (DllGetClassObjectProc)GetProcAddress(dinput8dll, "DllGetClassObject");
	m_pDllRegisterServer = (DllRegisterServerProc)GetProcAddress(dinput8dll, "DllRegisterServer");
	m_pDllUnregisterServer = (DllUnregisterServerProc)GetProcAddress(dinput8dll, "DllUnregisterServer");
}

HRESULT WINAPI di_DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* lplpDirectInput, LPUNKNOWN punkOuter)
{
	return di_DirectInputCreateEx(hinst, dwVersion, IID_IDirectInputA, (LPVOID*)lplpDirectInput, punkOuter);
}

HRESULT WINAPI di_DirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* lplpDirectInput, LPUNKNOWN punkOuter)
{
	return di_DirectInputCreateEx(hinst, dwVersion, IID_IDirectInputW, (LPVOID*)lplpDirectInput, punkOuter);
}

HRESULT WINAPI di_DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riid, LPVOID * lplpDD, LPUNKNOWN punkOuter)
{
	InitDinput8();

	if (!m_pDirectInput8Create || !lplpDD)
	{
		return E_FAIL;
	}

	Logging::Log() << "Redirecting 'DirectInputCreate' " << riid << " version " << Logging::hex(dwVersion) << " to --> 'DirectInput8Create'";

	HRESULT hr = m_pDirectInput8Create(hinst, 0x0800, (GetStringType(riid) == DEFAULT_CHARSET) ? IID_IDirectInput8W : IID_IDirectInput8A, lplpDD, punkOuter);

	if (SUCCEEDED(hr))
	{
		diVersion = dwVersion;
		genericQueryInterface(riid, lplpDD);
	}

	return hr;
}

HRESULT WINAPI di_DllCanUnloadNow()
{
	InitDinput8();

	if (!m_pDllCanUnloadNow)
	{
		return E_FAIL;
	}

	return m_pDllCanUnloadNow();
}

HRESULT WINAPI di_DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	InitDinput8();

	if (!m_pDllGetClassObject || !ppv)
	{
		return E_FAIL;
	}

	DWORD StringType = GetStringType(riid);

	HRESULT hr = m_pDllGetClassObject(rclsid, (StringType == ANSI_CHARSET) ? IID_IDirectInput8A : (StringType == DEFAULT_CHARSET) ? IID_IDirectInput8W : riid, ppv);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppv);
	}

	return hr;
}

HRESULT WINAPI di_DllRegisterServer()
{
	InitDinput8();

	if (!m_pDllRegisterServer)
	{
		return E_FAIL;
	}

	return m_pDllRegisterServer();
}

HRESULT WINAPI di_DllUnregisterServer()
{
	InitDinput8();

	if (!m_pDllUnregisterServer)
	{
		return E_FAIL;
	}

	return m_pDllUnregisterServer();
}
