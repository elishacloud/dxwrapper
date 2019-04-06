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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "dinput8.h"
#include "dinput8External.h"

AddressLookupTableDinput8<void> ProxyAddressLookupTableDinput8 = AddressLookupTableDinput8<void>();

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	FARPROC procName ## _out = nullptr;

namespace Dinput8Wrapper
{
	VISIT_PROCS_DINPUT8(INITIALIZE_WRAPPED_PROC);
	FARPROC DllGetClassObject_out = nullptr;
	FARPROC DllCanUnloadNow_out = nullptr;
	FARPROC DllRegisterServer_out = nullptr;
	FARPROC DllUnregisterServer_out = nullptr;
}

using namespace Dinput8Wrapper;

HRESULT WINAPI di8_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)
{
	static DirectInput8CreateProc m_pDirectInput8Create = (Wrapper::ValidProcAddress(DirectInput8Create_out)) ? (DirectInput8CreateProc)DirectInput8Create_out : nullptr;

	if (!m_pDirectInput8Create)
	{
		return E_FAIL;
	}

	Logging::Log() << "Redirecting 'DirectInput8Create' ...";

	HRESULT hr = m_pDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);

	if (SUCCEEDED(hr))
	{
		dinput8QueryInterface(riidltf, ppvOut);
	}

	return hr;
}

HRESULT WINAPI di8_DllCanUnloadNow()
{
	static DllCanUnloadNowProc m_pDllCanUnloadNow = (Wrapper::ValidProcAddress(DllCanUnloadNow_out)) ? (DllCanUnloadNowProc)DllCanUnloadNow_out : nullptr;

	if (!m_pDllCanUnloadNow)
	{
		return E_FAIL;
	}

	return m_pDllCanUnloadNow();
}

HRESULT WINAPI di8_DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	static DllGetClassObjectProc m_pDllGetClassObject = (Wrapper::ValidProcAddress(DllGetClassObject_out)) ? (DllGetClassObjectProc)DllGetClassObject_out : nullptr;

	if (!m_pDllGetClassObject)
	{
		return E_FAIL;
	}

	HRESULT hr = m_pDllGetClassObject(rclsid, riid, ppv);

	if (SUCCEEDED(hr))
	{
		dinput8QueryInterface(riid, ppv);
	}

	return hr;
}

HRESULT WINAPI di8_DllRegisterServer()
{
	static DllRegisterServerProc m_pDllRegisterServer = (Wrapper::ValidProcAddress(DllRegisterServer_out)) ? (DllRegisterServerProc)DllRegisterServer_out : nullptr;

	if (!m_pDllRegisterServer)
	{
		return E_FAIL;
	}

	return m_pDllRegisterServer();
}

HRESULT WINAPI di8_DllUnregisterServer()
{
	static DllUnregisterServerProc m_pDllUnregisterServer = (Wrapper::ValidProcAddress(DllUnregisterServer_out)) ? (DllUnregisterServerProc)DllUnregisterServer_out : nullptr;

	if (!m_pDllUnregisterServer)
	{
		return E_FAIL;
	}

	return m_pDllUnregisterServer();
}

LPCDIDATAFORMAT WINAPI di8_GetdfDIJoystick()
{
	static GetdfDIJoystickProc m_pGetdfDIJoystick = (Wrapper::ValidProcAddress(GetdfDIJoystick_out)) ? (GetdfDIJoystickProc)GetdfDIJoystick_out : nullptr;

	if (!m_pGetdfDIJoystick)
	{
		return nullptr;
	}

	return m_pGetdfDIJoystick();
}
