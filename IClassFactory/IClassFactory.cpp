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

#define INITGUID

#include <ddraw.h>
#include <ddrawex.h>
#include "IClassFactory.h"
#ifdef DDRAW
#include "ddraw\ddrawExternal.h"
#endif
#ifdef DINPUT
#include "dinput\dinputExternal.h"
#endif
#ifdef DINPUT8
#include "dinput8\dinput8External.h"
#endif

INITIALIZE_OUT_WRAPPED_PROC(CoCreateInstance, unused);

#ifdef DINPUT8
namespace dinputto8
{
	REFIID ConvertREFIID(REFIID riid);
}
#endif

REFIID ConvertAllREFIID(REFIID riid)
{
#ifdef DINPUT8
	if (Config.Dinputto8)
	{
		return dinputto8::ConvertREFIID(riid);
	}
#endif
	return riid;
}

/************************/
/*** IUnknown methods ***/
/************************/

HRESULT m_IClassFactory::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ppvObj)
	{
		return E_POINTER;
	}
	*ppvObj = nullptr;

	if (riid == IID_IClassFactory || riid == IID_IUnknown)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	Logging::LogDebug() << "Query for " << riid << " from " << WrapperID;

	if (!ProxyInterface)
	{
		*ppvObj = nullptr;

		IQueryInterface(riid, ppvObj);

		if (*ppvObj)
		{
			return S_OK;
		}

		Logging::Log() << __FUNCTION__ << " Query Not Implemented for " << riid << " from " << WrapperID;

		return E_NOINTERFACE;
	}

	HRESULT hr = ProxyInterface->QueryInterface(ConvertAllREFIID(riid), ppvObj);

	if (SUCCEEDED(hr))
	{
		IQueryInterface(riid, ppvObj);
	}
	else
	{
		*ppvObj = nullptr;

		IQueryInterface(riid, ppvObj);

		if (*ppvObj)
		{
			return S_OK;
		}

		Logging::LogDebug() << "Query failed for " << riid << " Error " << Logging::hex(hr);
	}

	return hr;
}

ULONG m_IClassFactory::AddRef()
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ProxyInterface)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IClassFactory::Release()
{
	Logging::LogDebug() << __FUNCTION__;

	ULONG ref;

	if (!ProxyInterface)
	{
		ref = InterlockedDecrement(&RefCount);
	}
	else
	{
		ref = ProxyInterface->Release();
	}

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

/*****************************/
/*** IClassFactory methods ***/
/*****************************/

HRESULT m_IClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
{
	Logging::LogDebug() << __FUNCTION__ << " " << ClassID << " --> " << riid;

	if (!ProxyInterface)
	{
		if (ppvObject)
		{
			*ppvObject = nullptr;

			IQueryInterface(riid, ppvObject);

			if (*ppvObject)
			{
				return S_OK;
			}
		}

		Logging::Log() << __FUNCTION__ << " Not Implemented for IID " << riid;

		return E_FAIL;
	}

	HRESULT hr = ProxyInterface->CreateInstance(pUnkOuter, ConvertAllREFIID(riid), ppvObject);

	if (SUCCEEDED(hr))
	{
		IQueryInterface(riid, ppvObject);
	}
	else
	{
		if (ppvObject)
		{
			*ppvObject = nullptr;

			IQueryInterface(riid, ppvObject);

			if (*ppvObject)
			{
				return S_OK;
			}
		}

		Logging::LogDebug() << "Query failed for " << riid << " Error " << Logging::hex(hr);
	}

	return hr;
}

HRESULT m_IClassFactory::LockServer(BOOL fLock)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ProxyInterface)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->LockServer(fLock);
}

HRESULT WINAPI CoCreateInstanceHandle(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
	Logging::LogDebug() << __FUNCTION__ " " << rclsid << " -> " << riid;

	DEFINE_STATIC_PROC_ADDRESS(CoCreateInstanceHandleProc, CoCreateInstance, CoCreateInstance_out);

	if (!CoCreateInstance)
	{
		return E_FAIL;
	}

	if (!ppv)
	{
		return E_POINTER;
	}

	*ppv = nullptr;

#ifdef DDRAW
	// IDirectDraw wrapper
	if (Config.EnableDdrawWrapper || Config.DDrawCompat)
	{
		// Create DirectDraw interface
		if (rclsid == CLSID_DirectDraw)
		{
			if (riid != IID_IUnknown &&
				riid != IID_IDirectDraw &&
				riid != IID_IDirectDraw2 &&
				riid != IID_IDirectDraw3 &&
				riid != IID_IDirectDraw4 &&
				riid != IID_IDirectDraw7)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			IDirectDraw *pDirectDraw = nullptr;
			HRESULT hr;
			if (riid == IID_IDirectDraw7)
			{
				hr = ((DirectDrawCreateExProc)ddraw::DirectDrawCreateEx_var)(nullptr, (LPVOID*)&pDirectDraw, riid, pUnkOuter);
			}
			else
			{
				hr = ((DirectDrawCreateProc)ddraw::DirectDrawCreate_var)(nullptr, &pDirectDraw, pUnkOuter);
			}

			if (SUCCEEDED(hr) && pDirectDraw)
			{
				if (riid == IID_IUnknown || riid == IID_IDirectDraw || riid == IID_IDirectDraw7)
				{
					*ppv = pDirectDraw;
				}
				else
				{
					void *pvObj = nullptr;
					hr = pDirectDraw->QueryInterface(riid, &pvObj);

					if (SUCCEEDED(hr) && pvObj)
					{
						*ppv = pvObj;
					}
					pDirectDraw->Release();
				}
			}

			return hr;
		}

		// Create DirectDraw7 interface
		if (rclsid == CLSID_DirectDraw7)
		{
			if (riid != IID_IUnknown && riid != IID_IDirectDraw7)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			return ((DirectDrawCreateExProc)ddraw::DirectDrawCreateEx_var)(nullptr, ppv, IID_IDirectDraw7, pUnkOuter);
		}

		// Create DirectDrawClipper interface
		if (rclsid == CLSID_DirectDrawClipper)
		{
			if (riid != IID_IDirectDrawClipper)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			return ((DirectDrawCreateClipperProc)ddraw::DirectDrawCreateClipper_var)(0, (LPDIRECTDRAWCLIPPER*)ppv, pUnkOuter);
		}
	}
#endif

#ifdef DINPUT
	// DirectInput wrapper
	if (Config.Dinputto8)
	{
		// Create DirectInput interface
		if (rclsid == CLSID_DirectInput)
		{
			if (riid != IID_IUnknown &&
				riid != IID_IDirectInputA && riid != IID_IDirectInputW &&
				riid != IID_IDirectInput2A && riid != IID_IDirectInput2W && 
				riid != IID_IDirectInput7A && riid != IID_IDirectInput7W)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			return di_DirectInputCreateEx(GetModuleHandle(nullptr), 0x0700, (riid != IID_IUnknown) ? riid : IID_IDirectInputA, ppv, pUnkOuter);
		}

		// Create DirectInputDevice interface
		if (rclsid == CLSID_DirectInputDevice)
		{
			if (riid != IID_IDirectInputDeviceA && riid != IID_IDirectInputDeviceW &&
				riid != IID_IDirectInputDevice2A && riid != IID_IDirectInputDevice2W &&
				riid != IID_IDirectInputDevice7A && riid != IID_IDirectInputDevice7W)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			REFIID riidltf = (riid != IID_IDirectInputDeviceA) ? IID_IDirectInputA :
				(riid != IID_IDirectInputDeviceW) ? IID_IDirectInputW :
				(riid != IID_IDirectInputDevice2A) ? IID_IDirectInput2A :
				(riid != IID_IDirectInputDevice2W) ? IID_IDirectInput2W :
				(riid != IID_IDirectInputDevice7A) ? IID_IDirectInput7A : IID_IDirectInput7W;
			IDirectInput *pIDirectInput = nullptr;
			HRESULT hr = di_DirectInputCreateEx(GetModuleHandle(nullptr), 0x0700, riidltf, (LPVOID*)&pIDirectInput, pUnkOuter);

			if (SUCCEEDED(hr) && pIDirectInput)
			{
				hr = pIDirectInput->CreateDevice(riid, (LPDIRECTINPUTDEVICE*)ppv, pUnkOuter);
			}

			return hr;
		}
	}
#endif

#ifdef DINPUT8
	// DirectInput8 wrapper
	if (Config.EnableDinput8Wrapper)
	{
		// Create DirectInput8 interface
		if (rclsid == CLSID_DirectInput8)
		{
			if (riid != IID_IUnknown && riid != IID_IDirectInput8A && riid != IID_IDirectInput8W)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			return di8_DirectInput8Create(GetModuleHandle(nullptr), 0x0800, (riid != IID_IUnknown) ? riid : IID_IDirectInput8A, ppv, pUnkOuter);
		}

		// Create DirectInputDevice8 interface
		if (rclsid == CLSID_DirectInputDevice8)
		{
			if (riid != IID_IDirectInputDevice8A && riid != IID_IDirectInputDevice8W)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			REFIID riidltf = (riid != IID_IDirectInputDevice8A) ? IID_IDirectInput8A : IID_IDirectInput8W;
			IDirectInput8 *pIDirectInput8 = nullptr;
			HRESULT hr = di8_DirectInput8Create(GetModuleHandle(nullptr), 0x0800, riidltf, (LPVOID*)&pIDirectInput8, pUnkOuter);

			if (SUCCEEDED(hr) && pIDirectInput8)
			{
				hr = pIDirectInput8->CreateDevice(riid, (LPDIRECTINPUTDEVICE8*)ppv, pUnkOuter);
			}

			return hr;
		}
	}
#endif

	return CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}
