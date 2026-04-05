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
#include "Utils\Utils.h"

namespace DinputWrapper
{
	VISIT_PROCS_DINPUT_SHARED(INITIALIZE_OUT_WRAPPED_PROC);
	INITIALIZE_OUT_WRAPPED_PROC(DirectInput8Create, unused);

	static void CheckSystemModule()
	{
		static bool RunOnce = true;
		if (RunOnce && Utils::CheckIfSystemModuleLoaded("dinput.dll"))
		{
			Logging::Log() << "Warning: System 'dinput.dll' is already loaded before dxwrapper!";
		}
		RunOnce = false;
	}
}

using namespace DinputWrapper;

DWORD diVersion = 0;

AddressLookupTableDinput ProxyAddressLookupTable;

HRESULT WINAPI di_DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riid, LPVOID * lplpDD, LPUNKNOWN pUnkOuter);

HRESULT WINAPI di_DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* lplpDirectInput, LPUNKNOWN pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	return di_DirectInputCreateEx(hinst, dwVersion, IID_IDirectInputA, (LPVOID*)lplpDirectInput, pUnkOuter);
}

HRESULT WINAPI di_DirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* lplpDirectInput, LPUNKNOWN pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	return di_DirectInputCreateEx(hinst, dwVersion, IID_IDirectInputW, (LPVOID*)lplpDirectInput, pUnkOuter);
}

HRESULT WINAPI di_DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riid, LPVOID * lplpDD, LPUNKNOWN pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	CheckSystemModule();

	DEFINE_STATIC_PROC_ADDRESS(DirectInput8CreateProc, DirectInput8Create, DirectInput8Create_out);

	if (!DirectInput8Create)
	{
		return DIERR_GENERIC;
	}

	// DirectInputCreateEx can only be called with IDirectInput interfaces, not with IUnknown!
	if (riid != IID_IDirectInputA && riid != IID_IDirectInput2A && riid != IID_IDirectInput7A &&
		riid != IID_IDirectInputW && riid != IID_IDirectInput2W && riid != IID_IDirectInput7W)
	{
		return DIERR_NOINTERFACE;
	}

	LOG_ONCE("Starting dinputto8 v" << APP_VERSION);

	LOG_LIMIT(3, "Redirecting 'DirectInputCreate' " << riid << " version " << Logging::hex(dwVersion) << " to --> 'DirectInput8Create'");

	if (pUnkOuter)
	{
		LOG_LIMIT(3, __FUNCTION__ << " Warning: 'pUnkOuter' is not null: " << pUnkOuter);
	}

	HRESULT hr = hresValidInstanceAndVersion(hinst, dwVersion);
	if (SUCCEEDED(hr))
	{
		typename m_IDirectInputX::proxy_type* Proxy;
		hr = DirectInput8Create(hinst, 0x0800, m_IDirectInputX::proxy_iid, reinterpret_cast<LPVOID*>(&Proxy), nullptr);

		if (SUCCEEDED(hr))
		{
			m_IDirectInputX* Interface = new m_IDirectInputX(Proxy);
			Interface->SetVersion(dwVersion);

			hr = Interface->QueryInterface(riid, lplpDD);
			Interface->Release();
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

	if (ModuleObjectCount::AnyObjectsInUse())
	{
		return S_FALSE;
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

	if (ppv == nullptr)
	{
		return E_POINTER;
	}

	HRESULT hr = E_OUTOFMEMORY;
	*ppv = nullptr;

	ClassFactoryBase* wrapperFactory = nullptr;
	if (rclsid == m_IDirectInputX::wrapper_clsid)
	{
		IClassFactory* proxyFactory;
		HRESULT proxyHr = DllGetClassObject(m_IDirectInputX::proxy_clsid, IID_PPV_ARGS(&proxyFactory));
		if (FAILED(proxyHr))
		{
			return proxyHr;
		}

		wrapperFactory = new(std::nothrow) ClassFactory<m_IDirectInputX>(proxyFactory);
	}
	else if (rclsid == m_IDirectInputDeviceX::wrapper_clsid)
	{
		IClassFactory* proxyFactory;
		HRESULT proxyHr = DllGetClassObject(m_IDirectInputDeviceX::proxy_clsid, IID_PPV_ARGS(&proxyFactory));
		if (FAILED(proxyHr))
		{
			return proxyHr;
		}

		wrapperFactory = new(std::nothrow) ClassFactory<m_IDirectInputDeviceX>(proxyFactory);
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
