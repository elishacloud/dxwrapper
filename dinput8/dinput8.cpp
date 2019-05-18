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
#include "IClassFactory\IClassFactory.h"

AddressLookupTableDinput8<void> ProxyAddressLookupTableDinput8 = AddressLookupTableDinput8<void>();

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	FARPROC procName ## _out = nullptr;

namespace Dinput8Wrapper
{
	VISIT_PROCS_DINPUT8(INITIALIZE_WRAPPED_PROC);
	VISIT_PROCS_DINPUT8_SHARED(INITIALIZE_WRAPPED_PROC);
}

using namespace Dinput8Wrapper;

HRESULT WINAPI di8_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)
{
	static DirectInput8CreateProc m_pDirectInput8Create = (Wrapper::ValidProcAddress(DirectInput8Create_out)) ? (DirectInput8CreateProc)DirectInput8Create_out : nullptr;

	if (!m_pDirectInput8Create)
	{
		return DIERR_GENERIC;
	}

	LOG_LIMIT(3, "Redirecting 'DirectInput8Create' ...");

	HRESULT hr = m_pDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);

	if (SUCCEEDED(hr) && ppvOut)
	{
		genericQueryInterface(riidltf, ppvOut);
	}

	return hr;
}

HRESULT WINAPI di8_DllCanUnloadNow()
{
	LOG_LIMIT(1, __FUNCTION__);

	static DllCanUnloadNowProc m_pDllCanUnloadNow = (Wrapper::ValidProcAddress(DllCanUnloadNow_out)) ? (DllCanUnloadNowProc)DllCanUnloadNow_out : nullptr;

	if (!m_pDllCanUnloadNow)
	{
		return DIERR_GENERIC;
	}

	return m_pDllCanUnloadNow();
}

HRESULT WINAPI di8_DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	LOG_LIMIT(1, __FUNCTION__);

	static DllGetClassObjectProc m_pDllGetClassObject = (Wrapper::ValidProcAddress(DllGetClassObject_out)) ? (DllGetClassObjectProc)DllGetClassObject_out : nullptr;

	if (!m_pDllGetClassObject)
	{
		return DIERR_GENERIC;
	}

	HRESULT hr = m_pDllGetClassObject(rclsid, riid, ppv);

	if (SUCCEEDED(hr) && ppv)
	{
		if (riid == IID_IClassFactory)
		{
			*ppv = new m_IClassFactory((IClassFactory*)*ppv, genericQueryInterface);

			((m_IClassFactory*)(*ppv))->SetCLSID(rclsid);

			return DI_OK;
		}

		genericQueryInterface(riid, ppv);
	}

	return hr;
}

HRESULT WINAPI di8_DllRegisterServer()
{
	LOG_LIMIT(1, __FUNCTION__);

	static DllRegisterServerProc m_pDllRegisterServer = (Wrapper::ValidProcAddress(DllRegisterServer_out)) ? (DllRegisterServerProc)DllRegisterServer_out : nullptr;

	if (!m_pDllRegisterServer)
	{
		return DIERR_GENERIC;
	}

	return m_pDllRegisterServer();
}

HRESULT WINAPI di8_DllUnregisterServer()
{
	LOG_LIMIT(1, __FUNCTION__);

	static DllUnregisterServerProc m_pDllUnregisterServer = (Wrapper::ValidProcAddress(DllUnregisterServer_out)) ? (DllUnregisterServerProc)DllUnregisterServer_out : nullptr;

	if (!m_pDllUnregisterServer)
	{
		return DIERR_GENERIC;
	}

	return m_pDllUnregisterServer();
}

LPCDIDATAFORMAT WINAPI di8_GetdfDIJoystick()
{
	LOG_LIMIT(1, __FUNCTION__);

	static GetdfDIJoystickProc m_pGetdfDIJoystick = (Wrapper::ValidProcAddress(GetdfDIJoystick_out)) ? (GetdfDIJoystickProc)GetdfDIJoystick_out : nullptr;

	if (!m_pGetdfDIJoystick)
	{
		return nullptr;
	}

	return m_pGetdfDIJoystick();
}
