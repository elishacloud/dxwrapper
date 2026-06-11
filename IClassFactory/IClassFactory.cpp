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
#ifdef DSOUND
#include "dsound\dsoundExternal.h"
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
				bool Ret = false;

				if (entry.Limit < limitThreshold)
				{
					entry.Limit++;
					Ret = true;
				}

				return Ret;
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

static inline HRESULT CreateClassFactory(REFCLSID rclsid, LPVOID* ppv)
{
	if (!ppv)
	{
		return E_POINTER;
	}

	*ppv = new (std::nothrow) m_IClassFactory(rclsid);

	if (!*ppv)
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

static HRESULT CreateWrapperInterface(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv, bool& InterfaceCreated)
{
	UNREFERENCED_PARAMETER(dwClsContext);

	InterfaceCreated = true;

#ifdef DDRAW
	// IDirectDraw wrapper
	if ((Config.EnableDdrawWrapper || Config.Dd7to9) && ddraw::DirectDrawCreateEx_var && ddraw::DirectDrawCreate_var)
	{
		// Create DirectDraw interface
		if (rclsid == CLSID_DirectDraw)
		{
			if (riid == IID_IClassFactory)
			{
				return CreateClassFactory(rclsid, ppv);
			}

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

			IDirectDraw* pDirectDraw = nullptr;

			HRESULT hr = ((DirectDrawCreateProc)ddraw::DirectDrawCreate_var)(nullptr, &pDirectDraw, pUnkOuter);

			if (SUCCEEDED(hr) && pDirectDraw)
			{
				hr = pDirectDraw->QueryInterface(riid, ppv);
				pDirectDraw->Release();
			}

			return hr;
		}

		// Create DirectDraw7 interface
		if (rclsid == CLSID_DirectDraw7)
		{
			if (riid == IID_IClassFactory)
			{
				return CreateClassFactory(rclsid, ppv);
			}

			if (riid != IID_IUnknown && riid != IID_IDirectDraw7)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			IDirectDraw7* pDirectDraw = nullptr;

			HRESULT hr = ((DirectDrawCreateExProc)ddraw::DirectDrawCreateEx_var)(nullptr, reinterpret_cast<void**>(&pDirectDraw), IID_IDirectDraw7, pUnkOuter);

			if (SUCCEEDED(hr) && pDirectDraw)
			{
				hr = pDirectDraw->QueryInterface(riid, ppv);
				pDirectDraw->Release();
			}

			return hr;
		}

		// Create DirectDrawClipper interface
		if (rclsid == CLSID_DirectDrawClipper)
		{
			if (riid == IID_IClassFactory)
			{
				return CreateClassFactory(rclsid, ppv);
			}

			if (riid != IID_IUnknown && riid != IID_IDirectDrawClipper)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			IDirectDrawClipper* pDirectDrawClipper = nullptr;

			HRESULT hr = ((DirectDrawCreateClipperProc)ddraw::DirectDrawCreateClipper_var)(0, &pDirectDrawClipper, pUnkOuter);

			if (SUCCEEDED(hr) && pDirectDrawClipper)
			{
				hr = pDirectDrawClipper->QueryInterface(riid, ppv);
				pDirectDrawClipper->Release();
			}

			return hr;
		}
	}
#endif

#ifdef DINPUT
	// DirectInput wrapper
	if (Config.Dinputto8 && dinput::DirectInputCreateEx_var)
	{
		// Create DirectInput interface
		if (rclsid == CLSID_DirectInput)
		{
			if (riid == IID_IClassFactory)
			{
				return CreateClassFactory(rclsid, ppv);
			}

			if (riid != IID_IUnknown &&
				riid != IID_IDirectInputA && riid != IID_IDirectInputW &&
				riid != IID_IDirectInput2A && riid != IID_IDirectInput2W &&
				riid != IID_IDirectInput7A && riid != IID_IDirectInput7W)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			IDirectInput7W* pDirectInput = nullptr;

			HRESULT hr = ((DirectInputCreateExProc)dinput::DirectInputCreateEx_var)(GetModuleHandle(nullptr), 0x0700, riid == IID_IUnknown ? IID_IDirectInput7W : riid, reinterpret_cast<void**>(&pDirectInput), pUnkOuter);

			if (SUCCEEDED(hr) && pDirectInput)
			{
				hr = pDirectInput->QueryInterface(riid, ppv);
				pDirectInput->Release();
			}

			return hr;
		}

		// Create DirectInputDevice interface
		if (rclsid == CLSID_DirectInputDevice)
		{
			if (riid == IID_IClassFactory)
			{
				return CreateClassFactory(rclsid, ppv);
			}

			if (riid != IID_IUnknown &&
				riid != IID_IDirectInputDeviceA && riid != IID_IDirectInputDeviceW &&
				riid != IID_IDirectInputDevice2A && riid != IID_IDirectInputDevice2W &&
				riid != IID_IDirectInputDevice7A && riid != IID_IDirectInputDevice7W)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			REFIID riidltf = (riid == IID_IDirectInputDeviceA) ? IID_IDirectInputA :
				(riid == IID_IDirectInputDeviceW) ? IID_IDirectInputW :
				(riid == IID_IDirectInputDevice2A) ? IID_IDirectInput2A :
				(riid == IID_IDirectInputDevice2W) ? IID_IDirectInput2W :
				(riid == IID_IDirectInputDevice7A) ? IID_IDirectInput7A : IID_IDirectInput7A;

			if (riid == IID_IUnknown ||
				riid == IID_IDirectInputDeviceW ||
				riid == IID_IDirectInputDevice2W ||
				riid == IID_IDirectInputDevice7W)
			{
				IDirectInputW* pIDirectInput = nullptr;

				HRESULT hr = ((DirectInputCreateExProc)dinput::DirectInputCreateEx_var)(GetModuleHandle(nullptr), 0x0700, riidltf, reinterpret_cast<void**>(&pIDirectInput), pUnkOuter);

				if (SUCCEEDED(hr) && pIDirectInput)
				{
					IDirectInputDeviceW* pDirectInputDevice = nullptr;

					hr = pIDirectInput->CreateDevice(riid == IID_IUnknown ? IID_IDirectInputDevice7W : riid, &pDirectInputDevice, pUnkOuter);

					if (SUCCEEDED(hr) && pDirectInputDevice)
					{
						hr = pDirectInputDevice->QueryInterface(riid, ppv);
						pDirectInputDevice->Release();
					}

					pIDirectInput->Release();
				}

				return hr;
			}
			else
			{
				IDirectInputA* pIDirectInput = nullptr;

				HRESULT hr = ((DirectInputCreateExProc)dinput::DirectInputCreateEx_var)(GetModuleHandle(nullptr), 0x0700, riidltf, reinterpret_cast<void**>(&pIDirectInput), pUnkOuter);

				if (SUCCEEDED(hr) && pIDirectInput)
				{
					IDirectInputDeviceA* pDirectInputDevice = nullptr;

					hr = pIDirectInput->CreateDevice(IID_IDirectInputDevice7A, &pDirectInputDevice, pUnkOuter);

					if (SUCCEEDED(hr) && pDirectInputDevice)
					{
						hr = pDirectInputDevice->QueryInterface(riid, ppv);
						pDirectInputDevice->Release();
					}

					pIDirectInput->Release();
				}

				return hr;
			}
		}
	}
#endif

#ifdef DINPUT8
	// DirectInput8 wrapper
	if (Config.EnableDinput8Wrapper && dinput8::DirectInput8Create_var)
	{
		// Create DirectInput8 interface
		if (rclsid == CLSID_DirectInput8)
		{
			if (riid == IID_IClassFactory)
			{
				return CreateClassFactory(rclsid, ppv);
			}

			if (riid != IID_IUnknown && riid != IID_IDirectInput8A && riid != IID_IDirectInput8W)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			IDirectInput8W* pDirectInput = nullptr;

			HRESULT hr = ((DirectInput8CreateProc)dinput8::DirectInput8Create_var)(GetModuleHandle(nullptr), 0x0800, riid == IID_IUnknown ? IID_IDirectInput8W : riid, reinterpret_cast<void**>(&pDirectInput), pUnkOuter);

			if (SUCCEEDED(hr) && pDirectInput)
			{
				hr = pDirectInput->QueryInterface(riid, ppv);
				pDirectInput->Release();
			}

			return hr;
		}

		// Create DirectInputDevice8 interface
		if (rclsid == CLSID_DirectInputDevice8)
		{
			if (riid == IID_IClassFactory)
			{
				return CreateClassFactory(rclsid, ppv);
			}

			if (riid != IID_IUnknown && riid != IID_IDirectInputDevice8A && riid != IID_IDirectInputDevice8W)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			if (riid == IID_IUnknown || riid == IID_IDirectInputDevice8W)
			{
				IDirectInput8W* pIDirectInput = nullptr;

				HRESULT hr = ((DirectInput8CreateProc)dinput8::DirectInput8Create_var)(GetModuleHandle(nullptr), 0x0800, IID_IDirectInput8W, reinterpret_cast<void**>(&pIDirectInput), pUnkOuter);

				if (SUCCEEDED(hr) && pIDirectInput)
				{
					IDirectInputDevice8W* pDirectInputDevice = nullptr;

					hr = pIDirectInput->CreateDevice(IID_IDirectInputDevice8W, &pDirectInputDevice, pUnkOuter);

					if (SUCCEEDED(hr) && pDirectInputDevice)
					{
						hr = pDirectInputDevice->QueryInterface(riid, ppv);
						pDirectInputDevice->Release();
					}

					pIDirectInput->Release();
				}

				return hr;
			}
			else
			{
				IDirectInput8A* pIDirectInput = nullptr;

				HRESULT hr = ((DirectInput8CreateProc)dinput8::DirectInput8Create_var)(GetModuleHandle(nullptr), 0x0800, IID_IDirectInput8A, reinterpret_cast<void**>(&pIDirectInput), pUnkOuter);

				if (SUCCEEDED(hr) && pIDirectInput)
				{
					IDirectInputDevice8A* pDirectInputDevice = nullptr;

					hr = pIDirectInput->CreateDevice(IID_IDirectInputDevice8A, &pDirectInputDevice, pUnkOuter);

					if (SUCCEEDED(hr) && pDirectInputDevice)
					{
						hr = pDirectInputDevice->QueryInterface(riid, ppv);
						pDirectInputDevice->Release();
					}

					pIDirectInput->Release();
				}

				return hr;
			}
		}
	}
#endif

#ifdef DSOUND
	// DirectSound wrapper
	if (Config.EnableDsoundWrapper && dsound::DirectSoundCreate_var && dsound::DirectSoundCreate8_var)
	{
		// Create DirectSound interface
		if (rclsid == CLSID_DirectSound)
		{
			if (riid == IID_IClassFactory)
			{
				return CreateClassFactory(rclsid, ppv);
			}

			if (riid != IID_IUnknown && riid != IID_IDirectSound)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			IDirectSound* pIDirectSound = nullptr;

			HRESULT hr = ((DirectSoundCreateProc)dsound::DirectSoundCreate_var)(nullptr, &pIDirectSound, pUnkOuter);

			if (SUCCEEDED(hr) && pIDirectSound)
			{
				hr = pIDirectSound->QueryInterface(riid, ppv);
				pIDirectSound->Release();
			}

			return hr;
		}

		// Create DirectSound8 interface
		if (rclsid == CLSID_DirectSound8)
		{
			if (riid == IID_IClassFactory)
			{
				return CreateClassFactory(rclsid, ppv);
			}

			if (riid != IID_IUnknown && riid != IID_IDirectSound8)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID: " << riid << " from: " << rclsid);
				return E_NOINTERFACE;
			}

			IDirectSound8* pIDirectSound = nullptr;

			HRESULT hr = ((DirectSoundCreate8Proc)dsound::DirectSoundCreate8_var)(nullptr, &pIDirectSound, pUnkOuter);

			if (SUCCEEDED(hr) && pIDirectSound)
			{
				hr = pIDirectSound->QueryInterface(riid, ppv);
				pIDirectSound->Release();
			}

			return hr;
		}
	}
#endif

	InterfaceCreated = false;
	return S_OK;
}

/************************/
/*** IUnknown methods ***/
/************************/

HRESULT m_IClassFactory::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	LOG_GUID_LIMIT(riid, 3, __FUNCTION__ << " Query for " << riid << " from IID_IClassFactory");

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

	return E_NOINTERFACE;
}

ULONG m_IClassFactory::AddRef()
{
	Logging::LogDebug() << __FUNCTION__;

	return InterlockedIncrement(&RefCount);
}

ULONG m_IClassFactory::Release()
{
	Logging::LogDebug() << __FUNCTION__;

	ULONG ref = InterlockedDecrement(&RefCount);

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

/*****************************/
/*** IClassFactory methods ***/
/*****************************/

HRESULT m_IClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
	LOG_GUID_LIMIT(riid, 3, __FUNCTION__ << " " << ClassID << " --> " << riid);

	if (!ppvObject)
	{
		return E_INVALIDARG;
	}
	*ppvObject = nullptr;

	if (pUnkOuter)
	{
		return CLASS_E_NOAGGREGATION;
	}

	// Prevent recursive loop
	if (riid == IID_IClassFactory)
	{
		return E_NOINTERFACE;
	}

	bool InterfaceCreated = false;

	HRESULT hr = CreateWrapperInterface(ClassID, pUnkOuter, 0, riid, ppvObject, InterfaceCreated);

	if (InterfaceCreated)
	{
		return hr;
	}

	Logging::LogDebug() << "Query failed for " << riid << " Error " << Logging::hex(hr);

	return E_NOINTERFACE;
}

HRESULT m_IClassFactory::LockServer(BOOL fLock)
{
	UNREFERENCED_PARAMETER(fLock);

	Logging::LogDebug() << __FUNCTION__;

	LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
	return E_NOTIMPL;
}

HRESULT WINAPI CoGetClassObjectHandle(REFCLSID rclsid, DWORD dwClsContext, LPVOID pvReserved, REFIID riid, LPVOID* ppv)
{
	LOG_GUID_LIMIT(rclsid, 3, __FUNCTION__ " " << rclsid << " -> " << riid);

	if (!ppv)
	{
		return E_POINTER;
	}
	*ppv = nullptr;

	DEFINE_STATIC_PROC_ADDRESS(CoGetClassObjectProc, CoGetClassObject, CoGetClassObject_out);

	if (!CoGetClassObject)
	{
		return E_FAIL;
	}

	bool InterfaceCreated = false;

	HRESULT hr = CreateWrapperInterface(rclsid, nullptr, dwClsContext, riid, ppv, InterfaceCreated);

	if (InterfaceCreated)
	{
		return hr;
	}

	return CoGetClassObject(rclsid, dwClsContext, pvReserved, riid, ppv);
}

HRESULT WINAPI CoCreateInstanceHandle(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
	LOG_GUID_LIMIT(rclsid, 3, __FUNCTION__ " " << rclsid << " -> " << riid);

	if (!ppv)
	{
		return E_POINTER;
	}
	*ppv = nullptr;

	DEFINE_STATIC_PROC_ADDRESS(CoCreateInstanceProc, CoCreateInstance, CoCreateInstance_out);

	if (!CoCreateInstance)
	{
		return E_FAIL;
	}

	bool InterfaceCreated = false;

	HRESULT hr = CreateWrapperInterface(rclsid, pUnkOuter, dwClsContext, riid, ppv, InterfaceCreated);

	if (InterfaceCreated)
	{
		return hr;
	}

	return CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}
