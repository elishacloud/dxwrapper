/**
* Copyright (C) 2024 Elisha Riedlinger
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

namespace Dinput8Wrapper
{
	VISIT_PROCS_DINPUT8(INITIALIZE_OUT_WRAPPED_PROC);
	VISIT_PROCS_DINPUT8_SHARED(INITIALIZE_OUT_WRAPPED_PROC);
}

using namespace Dinput8Wrapper;

HRESULT WINAPI di8_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DirectInput8CreateProc, DirectInput8Create, DirectInput8Create_out);

	if (!DirectInput8Create)
	{
		return DIERR_GENERIC;
	}

	LOG_LIMIT(3, "Redirecting 'DirectInput8Create' ...");

	HRESULT hr = DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);

	if (SUCCEEDED(hr) && ppvOut)
	{
		if (riidltf == IID_IDirectInput8A || riidltf == IID_IDirectInput8W)
		{
			*ppvOut = new m_IDirectInput8((IDirectInput8W*)*ppvOut, riidltf);
		}
		else
		{
			genericQueryInterface(riidltf, ppvOut);
		}
	}

	return hr;
}

HRESULT WINAPI di8_DllCanUnloadNow()
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DllCanUnloadNowProc, DllCanUnloadNow, DllCanUnloadNow_out);

	if (!DllCanUnloadNow)
	{
		return DIERR_GENERIC;
	}

	return DllCanUnloadNow();
}

HRESULT WINAPI di8_DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DllGetClassObjectProc, DllGetClassObject, DllGetClassObject_out);

	if (!DllGetClassObject)
	{
		return DIERR_GENERIC;
	}

	HRESULT hr = DllGetClassObject(rclsid, riid, ppv);

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

	DEFINE_STATIC_PROC_ADDRESS(DllRegisterServerProc, DllRegisterServer, DllRegisterServer_out);

	if (!DllRegisterServer)
	{
		return DIERR_GENERIC;
	}

	return DllRegisterServer();
}

HRESULT WINAPI di8_DllUnregisterServer()
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DllUnregisterServerProc, DllUnregisterServer, DllUnregisterServer_out);

	if (!DllUnregisterServer)
	{
		return DIERR_GENERIC;
	}

	return DllUnregisterServer();
}

LPCDIDATAFORMAT WINAPI di8_GetdfDIJoystick()
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(GetdfDIJoystickProc, GetdfDIJoystick, GetdfDIJoystick_out);

	if (!GetdfDIJoystick)
	{
		return nullptr;
	}

	return GetdfDIJoystick();
}
