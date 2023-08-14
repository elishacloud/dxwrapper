/**
* Copyright (C) 2023 Elisha Riedlinger
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

#include "dinput8\dinput8External.h"
#include "External\dinputto8\resource.h"
#include "External\dinputto8\dinputto8.h"
#include "IClassFactory\IClassFactory.h"

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	FARPROC procName ## _out = nullptr;

namespace DinputWrapper
{
	FARPROC DirectInput8Create_out = nullptr;
	VISIT_PROCS_DINPUT_SHARED(INITIALIZE_WRAPPED_PROC);
}

using namespace DinputWrapper;

DWORD diVersion = 0;

AddressLookupTableDinput<void> ProxyAddressLookupTable = AddressLookupTableDinput<void>();

HRESULT WINAPI di_DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riid, LPVOID * lplpDD, LPUNKNOWN punkOuter);

HRESULT WINAPI di_DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* lplpDirectInput, LPUNKNOWN punkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	return di_DirectInputCreateEx(hinst, dwVersion, IID_IDirectInputA, (LPVOID*)lplpDirectInput, punkOuter);
}

HRESULT WINAPI di_DirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* lplpDirectInput, LPUNKNOWN punkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	return di_DirectInputCreateEx(hinst, dwVersion, IID_IDirectInputW, (LPVOID*)lplpDirectInput, punkOuter);
}

HRESULT WINAPI di_DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riid, LPVOID * lplpDD, LPUNKNOWN punkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	static DirectInput8CreateProc m_pDirectInput8Create = (Wrapper::ValidProcAddress(DirectInput8Create_out)) ? (DirectInput8CreateProc)DirectInput8Create_out : nullptr;

	if (!m_pDirectInput8Create)
	{
		return DIERR_GENERIC;
	}

	LOG_ONCE("Starting dinputto8 v" << APP_VERSION);

	LOG_LIMIT(3, "Redirecting 'DirectInputCreate' " << riid << " version " << Logging::hex(dwVersion) << " to --> 'DirectInput8Create'");

	HRESULT hr = m_pDirectInput8Create(hinst, 0x0800, dinputto8::ConvertREFIID(riid), lplpDD, punkOuter);

	if (SUCCEEDED(hr) && lplpDD)
	{
		diVersion = dwVersion;

		m_IDirectInputX *Interface = new m_IDirectInputX((IDirectInput8W*)*lplpDD, riid);

		*lplpDD = Interface->GetWrapperInterfaceX(GetGUIDVersion(riid));
	}

	return hr;
}

HRESULT WINAPI di_DllCanUnloadNow()
{
	LOG_LIMIT(1, __FUNCTION__);

	static DllCanUnloadNowProc m_pDllCanUnloadNow = (Wrapper::ValidProcAddress(DllCanUnloadNow_out)) ? (DllCanUnloadNowProc)DllCanUnloadNow_out : nullptr;

	if (!m_pDllCanUnloadNow)
	{
		return DIERR_GENERIC;
	}

	return m_pDllCanUnloadNow();
}

HRESULT WINAPI di_DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	LOG_LIMIT(1, __FUNCTION__);

	static DllGetClassObjectProc m_pDllGetClassObject = (Wrapper::ValidProcAddress(DllGetClassObject_out)) ? (DllGetClassObjectProc)DllGetClassObject_out : nullptr;

	if (!m_pDllGetClassObject)
	{
		return DIERR_GENERIC;
	}

	HRESULT hr = m_pDllGetClassObject(dinputto8::ConvertREFCLSID(rclsid), dinputto8::ConvertREFIID(riid), ppv);

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

HRESULT WINAPI di_DllRegisterServer()
{
	LOG_LIMIT(1, __FUNCTION__);

	static DllRegisterServerProc m_pDllRegisterServer = (Wrapper::ValidProcAddress(DllRegisterServer_out)) ? (DllRegisterServerProc)DllRegisterServer_out : nullptr;

	if (!m_pDllRegisterServer)
	{
		return DIERR_GENERIC;
	}

	return m_pDllRegisterServer();
}

HRESULT WINAPI di_DllUnregisterServer()
{
	LOG_LIMIT(1, __FUNCTION__);

	static DllUnregisterServerProc m_pDllUnregisterServer = (Wrapper::ValidProcAddress(DllUnregisterServer_out)) ? (DllUnregisterServerProc)DllUnregisterServer_out : nullptr;

	if (!m_pDllUnregisterServer)
	{
		return DIERR_GENERIC;
	}

	return m_pDllUnregisterServer();
}
