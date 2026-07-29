/**
* Copyright (C) 2026 Elisha Riedlinger
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

#include "ddraw.h"
#include "ddrawex.h"

#define INITIALIZE_VAR_WRAPPED_PROC(procName, unused) \
	volatile FARPROC procName ## _var = (FARPROC)*ddex_ ## procName; \
	volatile FARPROC procName ## _funct = (FARPROC)*ddex_ ## procName;

namespace ddrawex
{
	VISIT_PROCS_DDRAWEX(INITIALIZE_OUT_WRAPPED_PROC);
	VISIT_PROCS_DDRAWEX(INITIALIZE_VAR_WRAPPED_PROC);
}

using namespace ddrawex;

HRESULT WINAPI ddex_DllCanUnloadNow()
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DllCanUnloadNowProc, DllCanUnloadNow, DllCanUnloadNow_out);

	if (!DllCanUnloadNow)
	{
		return DDERR_GENERIC;
	}

	return DllCanUnloadNow();
}

HRESULT WINAPI ddex_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (ppv == nullptr)
	{
		return E_POINTER;
	}

	HRESULT hr = E_OUTOFMEMORY;
	*ppv = nullptr;

	if (rclsid != CLSID_DirectDrawFactory)
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	ClassFactoryBase* wrapperFactory = nullptr;

	if (Config.Dd7to9)
	{
		wrapperFactory = new (std::nothrow) DDClassFactory(nullptr, rclsid);
	}
	else
	{
		DEFINE_STATIC_PROC_ADDRESS(DllGetClassObjectProc, DllGetClassObject, DllGetClassObject_out);

		if (!DllGetClassObject)
		{
			return DDERR_GENERIC;
		}

		IClassFactory* proxyFactory = nullptr;
		HRESULT proxyHr = DllGetClassObject(rclsid, IID_PPV_ARGS(&proxyFactory));
		if (FAILED(proxyHr))
		{
			return proxyHr;
		}

		wrapperFactory = new (std::nothrow) DDClassFactory(proxyFactory, rclsid);
		if (!wrapperFactory)
		{
			proxyFactory->Release();
		}
	}

	if (wrapperFactory != nullptr)
	{
		hr = wrapperFactory->QueryInterface(riid, ppv);
		wrapperFactory->Release();
	}

	return hr;
}

HRESULT WINAPI ddex_DllRegisterServer()
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DllRegisterServerProc, DllRegisterServer, DllRegisterServer_out);

	if (!DllRegisterServer)
	{
		return DDERR_GENERIC;
	}

	return DllRegisterServer();
}

HRESULT WINAPI ddex_DllUnregisterServer()
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DllUnregisterServerProc, DllUnregisterServer, DllUnregisterServer_out);

	if (!DllUnregisterServer)
	{
		return DDERR_GENERIC;
	}

	return DllUnregisterServer();
}
