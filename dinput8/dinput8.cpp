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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "dinput8.h"
#include "dinput8External.h"

AddressLookupTableDinput8 ProxyAddressLookupTableDinput8 = AddressLookupTableDinput8();

namespace Dinput8Wrapper
{
	VISIT_PROCS_DINPUT8(INITIALIZE_OUT_WRAPPED_PROC);
	VISIT_PROCS_DINPUT8_SHARED(INITIALIZE_OUT_WRAPPED_PROC);
}

using namespace Dinput8Wrapper;

HRESULT WINAPI di8_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riid, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DirectInput8CreateProc, DirectInput8Create, DirectInput8Create_out);

	if (!DirectInput8Create)
	{
		return DIERR_GENERIC;
	}

	// DirectInputCreateEx can only be called with IDirectInput8 interfaces, not with IUnknown!
	if (riid != IID_IDirectInput8A && riid != IID_IDirectInput8W)
	{
		return DIERR_NOINTERFACE;
	}

	LOG_LIMIT(3, "Redirecting 'DirectInput8Create' ...");

	HRESULT hr = DirectInput8Create(hinst, dwVersion, m_IDirectInput8::proxy_iid, ppvOut, punkOuter);

	if (SUCCEEDED(hr) && ppvOut)
	{
		*ppvOut = new m_IDirectInput8((IDirectInput8W*)*ppvOut);
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

	if (ppv == nullptr)
	{
		return E_POINTER;
	}

	HRESULT hr = E_OUTOFMEMORY;
	*ppv = nullptr;

	ClassFactoryBase* wrapperFactory = nullptr;
	if (rclsid == m_IDirectInput8::wrapper_clsid)
	{
		IClassFactory* proxyFactory;
		HRESULT proxyHr = DllGetClassObject(rclsid, IID_PPV_ARGS(&proxyFactory));
		if (FAILED(proxyHr))
		{
			return proxyHr;
		}

		wrapperFactory = new(std::nothrow) ClassFactory<m_IDirectInput8>(proxyFactory);
	}
	else if (rclsid == m_IDirectInputDevice8::wrapper_clsid)
	{
		IClassFactory* proxyFactory;
		HRESULT proxyHr = DllGetClassObject(rclsid, IID_PPV_ARGS(&proxyFactory));
		if (FAILED(proxyHr))
		{
			return proxyHr;
		}

		wrapperFactory = new(std::nothrow) ClassFactory<m_IDirectInputDevice8>(proxyFactory);
	}
	else
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	if (wrapperFactory != nullptr)
	{
		hr = wrapperFactory->QueryInterface(riid, ppv);
		wrapperFactory->Release();
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
