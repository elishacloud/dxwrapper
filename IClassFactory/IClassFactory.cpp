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

#define INITGUID

#include <ddraw.h>
#include <ddrawex.h>
#include "IClassFactory.h"
#ifdef DDRAW
#include "ddraw\ddrawExternal.h"
#include "ddraw\ddraw.h"
#endif
#ifdef DINPUT
#include "dinput\dinputExternal.h"
#include "External\dinputto8\dinputto8.h"
#endif
#ifdef DINPUT8
#include "dinput8\dinput8External.h"
#include "dinput8\dinput8.h"
#endif
#ifdef DSOUND
#include "dsound\dsoundExternal.h"
#include "dsound\dsound.h"
#endif
#include "ScopeGuard.h"

INITIALIZE_OUT_WRAPPED_PROC(CoGetClassObject, unused);
INITIALIZE_OUT_WRAPPED_PROC(CoCreateInstance, unused);

#ifdef _DEBUG
#define LOG_GUID_LIMIT(iid, limit, msg) \
	Logging::Log() << msg;
#else
#define LOG_GUID_LIMIT(iid, limit, msg) \
	{ \
		if (GuidLogLimit(iid, limit, __LINE__)) \
		{ \
			Logging::Log() << msg; \
		} \
	}
#endif

namespace {

	struct GuidLogLimitEntry {
		GUID iid = {};
		DWORD Limit = 0;
		DWORD Line = 0;
	};

	// Global vector to store all logged GUIDs and their current limits
	std::vector<GuidLogLimitEntry> g_GuidLogVector;
	CriticalSectionInit g_GuidLogLock;

	bool GuidLogLimit(REFIID riid, DWORD limitThreshold, DWORD Line)
	{
		CriticalSectionInit::Lock Lock(g_GuidLogLock);

		for (auto& entry : g_GuidLogVector)
		{
			if (entry.Line == Line && IsEqualGUID(entry.iid, riid))
			{
				if (entry.Limit < limitThreshold)
				{
					entry.Limit++;
					return true;
				}
				return false;
			}
		}

		// No existing entry — create a new one
		GuidLogLimitEntry newEntry;
		newEntry.iid = riid;
		newEntry.Limit = 1; // First use counts as 1
		newEntry.Line = Line;
		g_GuidLogVector.push_back(newEntry);

		return true;
	}
}

HRESULT WINAPI CoGetClassObjectHandle(REFCLSID rclsid, DWORD dwClsContext, LPVOID pvReserved, REFIID riid, LPVOID* ppv)
{
	if (!ppv)
	{
		return E_POINTER;
	}
	*ppv = nullptr;

#ifdef DDRAW
	if (Config.Dd7to9)
	{
		if (rclsid == CLSID_DirectDraw || rclsid == CLSID_DirectDraw7 || rclsid == CLSID_DirectDrawClipper || rclsid == CLSID_DirectDrawFactory)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			// Dd7to9 is handled differently, just call DllGetClassObject
			return dd_DllGetClassObject(rclsid, riid, ppv);
		}
	}
#endif

	DEFINE_STATIC_PROC_ADDRESS(CoGetClassObjectProc, CoGetClassObject, CoGetClassObject_out);

	if (!CoGetClassObject)
	{
		return E_FAIL;
	}

#ifdef DINPUT
	if (Config.Dinputto8)
	{
		if (rclsid == m_IDirectInputX::wrapper_clsid)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			IClassFactory* proxyFactory = nullptr;
			HRESULT proxyHr = CoGetClassObject(m_IDirectInputX::proxy_clsid, dwClsContext, pvReserved, IID_PPV_ARGS(&proxyFactory));
			if (FAILED(proxyHr))
			{
				return proxyHr;
			}

			ClassFactoryBase* wrapperFactory = new (std::nothrow) ClassFactory<m_IDirectInputX>(proxyFactory);
			if (!wrapperFactory)
			{
				proxyFactory->Release();
				return E_OUTOFMEMORY;
			}

			HRESULT hr = wrapperFactory->QueryInterface(riid, ppv);
			wrapperFactory->Release();
			return hr;
		}
		else if (rclsid == m_IDirectInputDeviceX::wrapper_clsid)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			IClassFactory* proxyFactory = nullptr;
			HRESULT proxyHr = CoGetClassObject(m_IDirectInputDeviceX::proxy_clsid, dwClsContext, pvReserved, IID_PPV_ARGS(&proxyFactory));
			if (FAILED(proxyHr))
			{
				return proxyHr;
			}

			ClassFactoryBase* wrapperFactory = new (std::nothrow) ClassFactory<m_IDirectInputDeviceX>(proxyFactory);
			if (!wrapperFactory)
			{
				proxyFactory->Release();
				return E_OUTOFMEMORY;
			}

			HRESULT hr = wrapperFactory->QueryInterface(riid, ppv);
			wrapperFactory->Release();
			return hr;
		}
	}
#endif

	HRESULT hr = CoGetClassObject(rclsid, dwClsContext, pvReserved, riid, ppv);

	if (FAILED(hr))
	{
		return hr;
	}

#ifdef DDRAW
	if (Config.EnableDdrawWrapper)
	{
		if (rclsid == CLSID_DirectDraw || rclsid == CLSID_DirectDraw7 || rclsid == CLSID_DirectDrawClipper || rclsid == CLSID_DirectDrawFactory)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			ClassFactoryBase* wrapperFactory = new (std::nothrow) DDClassFactory(reinterpret_cast<IClassFactory*>(*ppv), rclsid);
			if (!wrapperFactory)
			{
				(reinterpret_cast<IClassFactory*>(*ppv))->Release();
				return E_OUTOFMEMORY;
			}

			hr = wrapperFactory->QueryInterface(riid, ppv);
			wrapperFactory->Release();
			return hr;
		}
	}
#endif

#ifdef DINPUT8
	if (Config.EnableDinput8Wrapper)
	{
		if (rclsid == m_IDirectInput8::wrapper_clsid)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			ClassFactoryBase* wrapperFactory = new (std::nothrow) ClassFactory<m_IDirectInput8>(reinterpret_cast<IClassFactory*>(*ppv));
			if (!wrapperFactory)
			{
				(reinterpret_cast<IClassFactory*>(*ppv))->Release();
				return E_OUTOFMEMORY;
			}

			hr = wrapperFactory->QueryInterface(riid, ppv);
			wrapperFactory->Release();
			return hr;
		}
		else if (rclsid == m_IDirectInputDevice8::wrapper_clsid)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			ClassFactoryBase* wrapperFactory = new (std::nothrow) ClassFactory<m_IDirectInputDevice8>(reinterpret_cast<IClassFactory*>(*ppv));
			if (!wrapperFactory)
			{
				(reinterpret_cast<IClassFactory*>(*ppv))->Release();
				return E_OUTOFMEMORY;
			}

			hr = wrapperFactory->QueryInterface(riid, ppv);
			wrapperFactory->Release();
			return hr;
		}
	}
#endif

#ifdef DSOUND
	if (Config.EnableDsoundWrapper)
	{
		if (rclsid == m_IDirectSound8::wrapper_clsid || rclsid == m_IDirectSound8::alternative_clsid)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			ClassFactoryBase* wrapperFactory = new (std::nothrow) ClassFactory<m_IDirectSound8>(reinterpret_cast<IClassFactory*>(*ppv));
			if (!wrapperFactory)
			{
				(reinterpret_cast<IClassFactory*>(*ppv))->Release();
				return E_OUTOFMEMORY;
			}

			hr = wrapperFactory->QueryInterface(riid, ppv);
			wrapperFactory->Release();
			return hr;
		}
	}
#endif

	LOG_GUID_LIMIT(rclsid, 3, __FUNCTION__ " Bypassing: " << rclsid << " -> " << riid);

	return hr;
}

HRESULT WINAPI CoCreateInstanceHandle(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
	if (!ppv)
	{
		return E_POINTER;
	}
	*ppv = nullptr;

#ifdef DDRAW
	if (Config.Dd7to9)
	{
		if (rclsid == CLSID_DirectDraw || rclsid == CLSID_DirectDraw7)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			IDirectDraw7* pDirectDraw = nullptr;

			if (rclsid == CLSID_DirectDraw7 || riid == IID_IDirectDraw7)
			{
				HRESULT proxyHr = dd_DirectDrawCreateEx(nullptr, reinterpret_cast<void**>(&pDirectDraw), IID_IDirectDraw7, pUnkOuter);
				if (FAILED(proxyHr))
				{
					return proxyHr;
				}
			}
			else
			{
				HRESULT proxyHr = dd_DirectDrawCreate(nullptr, reinterpret_cast<LPDIRECTDRAW*>(&pDirectDraw), pUnkOuter);
				if (FAILED(proxyHr))
				{
					return proxyHr;
				}
			}

			HRESULT hr = pDirectDraw->QueryInterface(riid, ppv);
			pDirectDraw->Release();
			return hr;
		}
		else if (rclsid == CLSID_DirectDrawClipper)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			IDirectDrawClipper* pDirectDrawClipper = nullptr;

			HRESULT proxyHr = dd_DirectDrawCreateClipper(0, reinterpret_cast<LPDIRECTDRAWCLIPPER*>(&pDirectDrawClipper), pUnkOuter);
			if (FAILED(proxyHr))
			{
				return proxyHr;
			}

			HRESULT hr = pDirectDrawClipper->QueryInterface(riid, ppv);
			pDirectDrawClipper->Release();
			return hr;
		}
		else if (rclsid == CLSID_DirectDrawFactory)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			m_IDirectDrawFactory* pDirectDrawFactory = new (std::nothrow) m_IDirectDrawFactory(nullptr);
			if (!pDirectDrawFactory)
			{
				return E_OUTOFMEMORY;
			}

			HRESULT hr = pDirectDrawFactory->QueryInterface(riid, ppv);
			pDirectDrawFactory->Release();
			return hr;
		}
	}
#endif

	DEFINE_STATIC_PROC_ADDRESS(CoCreateInstanceProc, CoCreateInstance, CoCreateInstance_out);

	if (!CoCreateInstance)
	{
		return E_FAIL;
	}

#ifdef DINPUT
	if (Config.Dinputto8)
	{
		if (rclsid == CLSID_DirectInput)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			HRESULT hr = CoCreateInstance(CLSID_DirectInput8, pUnkOuter, dwClsContext, IID_IDirectInput8W, ppv);

			if (FAILED(hr))
			{
				return hr;
			}

			m_IDirectInputX* pDirectInput = new (std::nothrow) m_IDirectInputX(reinterpret_cast<IDirectInput8W*>(*ppv));
			if (!pDirectInput)
			{
				(reinterpret_cast<IDirectInput8W*>(*ppv))->Release();
				return E_OUTOFMEMORY;
			}

			const DWORD dwVersion =
				riid == IID_IDirectInput7A || riid == IID_IDirectInput7W ? 0x0700 :
				riid == IID_IDirectInput2A || riid == IID_IDirectInput2W ? 0x0200 : 0x0100;

			pDirectInput->SetVersion(dwVersion);

			hr = pDirectInput->QueryInterface(riid, ppv);
			pDirectInput->Release();
			return hr;
		}
		else if (rclsid == CLSID_DirectInputDevice)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			HRESULT hr = CoCreateInstance(CLSID_DirectInputDevice8, pUnkOuter, dwClsContext, IID_IDirectInputDevice8W, ppv);

			if (FAILED(hr))
			{
				return hr;
			}

			m_IDirectInputDeviceX* pDirectInputDevice = new (std::nothrow) m_IDirectInputDeviceX(reinterpret_cast<IDirectInputDevice8W*>(*ppv));
			if (!pDirectInputDevice)
			{
				(reinterpret_cast<IDirectInputDevice8W*>(*ppv))->Release();
				return E_OUTOFMEMORY;
			}

			const DWORD dwVersion =
				riid == IID_IDirectInputDevice7A || riid == IID_IDirectInputDevice7W ? 0x0700 :
				riid == IID_IDirectInputDevice2A || riid == IID_IDirectInputDevice2W ? 0x0200 : 0x0100;

			pDirectInputDevice->SetVersion(dwVersion);

			hr = pDirectInputDevice->QueryInterface(riid, ppv);
			pDirectInputDevice->Release();
			return hr;
		}
	}
#endif

	HRESULT hr = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);

	if (FAILED(hr))
	{
		return hr;
	}

#ifdef DDRAW
	if (Config.EnableDdrawWrapper)
	{
		if (rclsid == CLSID_DirectDraw || rclsid == CLSID_DirectDraw7)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			const DWORD DxVersion = rclsid == CLSID_DirectDraw ? 1 : 7;

			m_IDirectDrawX* pDirectDraw = new (std::nothrow) m_IDirectDrawX(reinterpret_cast<IDirectDraw7*>(*ppv), DxVersion);
			if (!pDirectDraw)
			{
				(reinterpret_cast<IDirectDraw7*>(*ppv))->Release();
				return E_OUTOFMEMORY;
			}

			hr = pDirectDraw->QueryInterface(riid, ppv, DxVersion);
			pDirectDraw->Release(DxVersion);
			return hr;
		}
		else if (rclsid == CLSID_DirectDrawClipper)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			m_IDirectDrawClipper* pDirectDrawClipper = new (std::nothrow) m_IDirectDrawClipper(reinterpret_cast<IDirectDrawClipper*>(*ppv));
			if (!pDirectDrawClipper)
			{
				(reinterpret_cast<IDirectDrawClipper*>(*ppv))->Release();
				return E_OUTOFMEMORY;
			}

			hr = pDirectDrawClipper->QueryInterface(riid, ppv);
			pDirectDrawClipper->Release();
			return hr;
		}
		else if (rclsid == CLSID_DirectDrawFactory)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			m_IDirectDrawFactory* pDirectDrawFactory = new (std::nothrow) m_IDirectDrawFactory(reinterpret_cast<IDirectDrawFactory*>(*ppv));
			if (!pDirectDrawFactory)
			{
				(reinterpret_cast<IDirectDrawFactory*>(*ppv))->Release();
				return E_OUTOFMEMORY;
			}

			hr = pDirectDrawFactory->QueryInterface(riid, ppv);
			pDirectDrawFactory->Release();
			return hr;
		}
	}
#endif

#ifdef DINPUT8
	if (Config.EnableDinput8Wrapper)
	{
		if (rclsid == CLSID_DirectInput8)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			m_IDirectInput8* pDirectInput = new (std::nothrow) m_IDirectInput8(reinterpret_cast<IDirectInput8W*>(*ppv));
			if (!pDirectInput)
			{
				(reinterpret_cast<IDirectInput8W*>(*ppv))->Release();
				return E_OUTOFMEMORY;
			}

			hr = pDirectInput->QueryInterface(riid, ppv);
			pDirectInput->Release();
			return hr;
		}
		else if (rclsid == CLSID_DirectInputDevice8)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			m_IDirectInputDevice8* pDirectInputDevice = new (std::nothrow) m_IDirectInputDevice8(reinterpret_cast<IDirectInputDevice8W*>(*ppv));
			if (!pDirectInputDevice)
			{
				(reinterpret_cast<IDirectInputDevice8W*>(*ppv))->Release();
				return E_OUTOFMEMORY;
			}

			hr = pDirectInputDevice->QueryInterface(riid, ppv);
			pDirectInputDevice->Release();
			return hr;
		}
	}
#endif

#ifdef DSOUND
	if (Config.EnableDsoundWrapper)
	{
		if (rclsid == CLSID_DirectSound || rclsid == CLSID_DirectSound8)
		{
			LOG_LIMIT(3, __FUNCTION__ " Wrapping: " << rclsid << " -> " << riid);

			m_IDirectSound8* pDirectSound = new (std::nothrow) m_IDirectSound8(reinterpret_cast<IDirectSound8*>(*ppv));
			if (!pDirectSound)
			{
				(reinterpret_cast<IDirectSound8*>(*ppv))->Release();
				return E_OUTOFMEMORY;
			}

			hr = pDirectSound->QueryInterface(riid, ppv);
			pDirectSound->Release();
			return hr;
		}
	}
#endif

	LOG_GUID_LIMIT(rclsid, 3, __FUNCTION__ " Bypassing: " << rclsid << " -> " << riid);

	return hr;
}
