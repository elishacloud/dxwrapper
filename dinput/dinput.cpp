/**
* Copyright (C) 2025 Elisha Riedlinger
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

namespace DinputWrapper
{
	VISIT_PROCS_DINPUT_SHARED(INITIALIZE_OUT_WRAPPED_PROC);
	INITIALIZE_OUT_WRAPPED_PROC(DirectInput8Create, unused);
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

	DEFINE_STATIC_PROC_ADDRESS(DirectInput8CreateProc, DirectInput8Create, DirectInput8Create_out);

	if (!DirectInput8Create)
	{
		return DIERR_GENERIC;
	}

	LOG_ONCE("Starting dinputto8 v" << APP_VERSION);

	LOG_LIMIT(3, "Redirecting 'DirectInputCreate' " << riid << " version " << Logging::hex(dwVersion) << " to --> 'DirectInput8Create'");

	HRESULT hr = hresValidInstanceAndVersion(hinst, dwVersion);
	if (SUCCEEDED(hr))
	{
		hr = DirectInput8Create(hinst, 0x0800, ConvertREFIID(riid), lplpDD, punkOuter);

		if (SUCCEEDED(hr) && lplpDD)
		{
			m_IDirectInputX* Interface = new m_IDirectInputX((IDirectInput8W*)*lplpDD, riid);
			Interface->SetVersion(dwVersion);

			*lplpDD = Interface->GetWrapperInterfaceX(GetGUIDVersion(riid));
		}
	}

	return hr;
}

HRESULT WINAPI di_DllCanUnloadNow()
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DllCanUnloadNowProc, DllCanUnloadNow, DllCanUnloadNow_out);

	if (!DllCanUnloadNow)
	{
		return DIERR_GENERIC;
	}

	return DllCanUnloadNow();
}

HRESULT WINAPI di_DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DllGetClassObjectProc, DllGetClassObject, DllGetClassObject_out);

	if (!DllGetClassObject)
	{
		return DIERR_GENERIC;
	}

	HRESULT hr = DllGetClassObject(dinputto8::ConvertREFCLSID(rclsid), dinputto8::ConvertREFIID(riid), ppv);

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

	DEFINE_STATIC_PROC_ADDRESS(DllRegisterServerProc, DllRegisterServer, DllRegisterServer_out);

	if (!DllRegisterServer)
	{
		return DIERR_GENERIC;
	}

	return DllRegisterServer();
}

HRESULT WINAPI di_DllUnregisterServer()
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DllUnregisterServerProc, DllUnregisterServer, DllUnregisterServer_out);

	if (!DllUnregisterServer)
	{
		return DIERR_GENERIC;
	}

	return DllUnregisterServer();
}
