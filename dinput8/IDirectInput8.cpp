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

#include "dinput8.h"

HRESULT m_IDirectInput8::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppvObj)
	{
		return E_POINTER;
	}

	if (riid == IID_IUnknown || riid == IID_IDirectInput8W)
	{
		*ppvObj = static_cast<IDirectInput8W*>(this);
	}
	else if (riid == IID_IDirectInput8A)
	{
		*ppvObj = static_cast<IDirectInput8A*>(this);
	}
	else
	{
		return ProxyInterface->QueryInterface(riid, ppvObj);
	}

	AddRef();
	return S_OK;
}

ULONG m_IDirectInput8::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return _InterlockedIncrement(&RefCount);
}

ULONG m_IDirectInput8::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LONG ref = _InterlockedDecrement(&RefCount);

	if (ref == 0)
	{
		delete this;
		return 0;
	}

	return ref;
}

template HRESULT m_IDirectInput8::CreateDeviceT<IDirectInput8A, LPDIRECTINPUTDEVICE8A*>(IDirectInput8A*, REFGUID, LPDIRECTINPUTDEVICE8A*, LPUNKNOWN);
template HRESULT m_IDirectInput8::CreateDeviceT<IDirectInput8W, LPDIRECTINPUTDEVICE8W*>(IDirectInput8W*, REFGUID, LPDIRECTINPUTDEVICE8W*, LPUNKNOWN);
template <class T, class V>
HRESULT m_IDirectInput8::CreateDeviceT(T* ProxyInterfaceT, REFGUID rguid, V lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterfaceT->CreateDevice(rguid, lplpDirectInputDevice, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDirectInputDevice)
	{
		m_IDirectInputDevice8* pAddressX = new m_IDirectInputDevice8((IDirectInputDevice8W*)*lplpDirectInputDevice);

		*lplpDirectInputDevice = pAddressX;

		bool isMouse = false;

		if (IsEqualGUID(GUID_SysMouse, rguid) || IsEqualIID(GUID_SysMouseEm, rguid) || IsEqualIID(GUID_SysMouseEm2, rguid))
		{
			isMouse = true;
		}
		else
		{
			DIDEVCAPS caps = {};
			caps.dwSize = sizeof(DIDEVCAPS);

			if (SUCCEEDED(pAddressX->GetCapabilities(&caps)))
			{
				if (GET_DIDEVICE_TYPE(caps.dwDevType) == DI8DEVTYPE_MOUSE)
				{
					isMouse = true;
				}
			}
		}

		if (isMouse)
		{
			pAddressX->SetAsMouse();
		}
	}

	return hr;
}

template HRESULT m_IDirectInput8::EnumDevicesT<IDirectInput8A, LPDIENUMDEVICESCALLBACKA, DIDEVICEINSTANCEA>(IDirectInput8A*, DWORD, LPDIENUMDEVICESCALLBACKA, LPVOID, DWORD);
template HRESULT m_IDirectInput8::EnumDevicesT<IDirectInput8W, LPDIENUMDEVICESCALLBACKW, DIDEVICEINSTANCEW>(IDirectInput8W*, DWORD, LPDIENUMDEVICESCALLBACKW, LPVOID, DWORD);
template <class T, class V, class D>
HRESULT m_IDirectInput8::EnumDevicesT(T* ProxyInterfaceT, DWORD dwDevType, V lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.DeviceLookupCacheTime && lpCallback)
	{
		auto now = std::chrono::steady_clock::now();
		auto& cachedData = GetEnumCache(ProxyInterfaceT);

		// Check if cached data is valid
		if (dwDevType == cachedData.dwDevType && dwFlags == cachedData.dwFlags && (now - cachedData.lastUpdate) < cacheDuration)
		{
			// Use cached data
			for (const auto& entry : cachedData.devices)
			{
				if (lpCallback(&entry, pvRef) == DIENUM_STOP)
				{
					break;
				}
			}
			return DI_OK;
		}

		struct EnumDevices
		{
			LPVOID pvRef = nullptr;
			V lpCallback = nullptr;
			std::vector<D> CacheDevices;

			static BOOL CALLBACK DIEnumDevicesCallback(const D* lpddi, LPVOID pvRef)
			{
				EnumDevices* self = (EnumDevices*)pvRef;

				if (lpddi)
				{
					D ddi = {};
					const DWORD copySize = min(lpddi->dwSize, sizeof(ddi));
					memcpy(&ddi, lpddi, copySize);
					ddi.dwSize = copySize;
					self->CacheDevices.push_back(ddi);
				}
				return DIENUM_CONTINUE;
			}
		} CallbackContext;
		CallbackContext.pvRef = pvRef;
		CallbackContext.lpCallback = lpCallback;

		HRESULT hr = ProxyInterfaceT->EnumDevices(dwDevType, EnumDevices::DIEnumDevicesCallback, &CallbackContext, dwFlags);

		if (SUCCEEDED(hr))
		{
			// Update the cache
			cachedData.lastUpdate = now;
			cachedData.dwDevType = dwDevType;
			cachedData.dwFlags = dwFlags;
			cachedData.devices = std::move(CallbackContext.CacheDevices);

			// Use the new data
			for (const auto& entry : cachedData.devices)
			{
				if (lpCallback(&entry, pvRef) == DIENUM_STOP)
				{
					break;
				}
			}
		}

		return hr;
	}

	return ProxyInterfaceT->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
}

HRESULT m_IDirectInput8::GetDeviceStatus(REFGUID rguidInstance)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetDeviceStatus(rguidInstance);
}

HRESULT m_IDirectInput8::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->RunControlPanel(hwndOwner, dwFlags);
}

HRESULT m_IDirectInput8::Initialize(HINSTANCE hinst, DWORD dwVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Initialize(hinst, dwVersion);
}

template HRESULT m_IDirectInput8::FindDeviceT<IDirectInput8A, LPCSTR>(IDirectInput8A*, REFGUID, LPCSTR, LPGUID);
template HRESULT m_IDirectInput8::FindDeviceT<IDirectInput8W, LPCWSTR>(IDirectInput8W*, REFGUID, LPCWSTR, LPGUID);
template <class T, class V>
HRESULT m_IDirectInput8::FindDeviceT(T* ProxyInterfaceT, REFGUID rguidClass, V ptszName, LPGUID pguidInstance)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterfaceT->FindDevice(rguidClass, ptszName, pguidInstance);
}

template HRESULT m_IDirectInput8::EnumDevicesBySemanticsT<IDirectInput8A, LPCSTR, LPDIACTIONFORMATA, LPDIENUMDEVICESBYSEMANTICSCBA, LPCDIDEVICEINSTANCEA, LPDIRECTINPUTDEVICE8A>(IDirectInput8A*, LPCSTR, LPDIACTIONFORMATA, LPDIENUMDEVICESBYSEMANTICSCBA, LPVOID, DWORD);
template HRESULT m_IDirectInput8::EnumDevicesBySemanticsT<IDirectInput8W, LPCWSTR, LPDIACTIONFORMATW, LPDIENUMDEVICESBYSEMANTICSCBW, LPCDIDEVICEINSTANCEW, LPDIRECTINPUTDEVICE8W>(IDirectInput8W*, LPCWSTR, LPDIACTIONFORMATW, LPDIENUMDEVICESBYSEMANTICSCBW, LPVOID, DWORD);
template <class T, class V, class W, class X, class C, class D>
HRESULT m_IDirectInput8::EnumDevicesBySemanticsT(T* ProxyInterfaceT, V ptszUserName, W lpdiActionFormat, X lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (lpCallback)
	{
		struct DeviceEnumerator
		{
			LPVOID pvRef = nullptr;
			X lpCallback = nullptr;

			static BOOL CALLBACK EnumDeviceCallback(C lpddi, D lpdid, DWORD dwFlags, DWORD dwRemaining, LPVOID pvRef)
			{
				DeviceEnumerator* self = (DeviceEnumerator*)pvRef;

				if (lpdid)
				{
					m_IDirectInputDevice8* WrapperDevice = ProxyAddressLookupTableDinput8.FindAddress<m_IDirectInputDevice8>(lpdid);
					if (WrapperDevice == nullptr)
					{
						WrapperDevice = new m_IDirectInputDevice8(lpdid);
					}
					lpdid = WrapperDevice;
				}

				return self->lpCallback(lpddi, lpdid, dwFlags, dwRemaining, self->pvRef);
			}
		} CallbackContext;
		CallbackContext.pvRef = pvRef;
		CallbackContext.lpCallback = lpCallback;

		return ProxyInterfaceT->EnumDevicesBySemantics(ptszUserName, lpdiActionFormat, DeviceEnumerator::EnumDeviceCallback, &CallbackContext, dwFlags);
	}

	return ProxyInterfaceT->EnumDevicesBySemantics(ptszUserName, lpdiActionFormat, lpCallback, pvRef, dwFlags);
}

template HRESULT m_IDirectInput8::ConfigureDevicesT<IDirectInput8A, LPDICONFIGUREDEVICESPARAMSA>(IDirectInput8A*, LPDICONFIGUREDEVICESCALLBACK, LPDICONFIGUREDEVICESPARAMSA, DWORD, LPVOID);
template HRESULT m_IDirectInput8::ConfigureDevicesT<IDirectInput8W, LPDICONFIGUREDEVICESPARAMSW>(IDirectInput8W*, LPDICONFIGUREDEVICESCALLBACK, LPDICONFIGUREDEVICESPARAMSW, DWORD, LPVOID);
template <class T, class V>
HRESULT m_IDirectInput8::ConfigureDevicesT(T* ProxyInterfaceT, LPDICONFIGUREDEVICESCALLBACK lpdiCallback, V lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterfaceT->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
}
