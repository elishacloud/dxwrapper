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

#include "dinput8.h"

HRESULT m_IDirectInput8::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if ((riid == WrapperID || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return DI_OK;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectInput8::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirectInput8::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

template HRESULT m_IDirectInput8::CreateDeviceT<IDirectInput8A, LPDIRECTINPUTDEVICE8A*>(REFGUID, LPDIRECTINPUTDEVICE8A*, LPUNKNOWN);
template HRESULT m_IDirectInput8::CreateDeviceT<IDirectInput8W, LPDIRECTINPUTDEVICE8W*>(REFGUID, LPDIRECTINPUTDEVICE8W*, LPUNKNOWN);
template <class T, class V>
HRESULT m_IDirectInput8::CreateDeviceT(REFGUID rguid, V lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = GetProxyInterface<T>()->CreateDevice(rguid, lplpDirectInputDevice, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDirectInputDevice)
	{
		m_IDirectInputDevice8* pAddressX = new m_IDirectInputDevice8((IDirectInputDevice8W*)*lplpDirectInputDevice, WrapperDeviceID);

		*lplpDirectInputDevice = pAddressX;

		if (IsEqualIID(GUID_SysMouse, rguid) || IsEqualIID(GUID_SysMouseEm, rguid) || IsEqualIID(GUID_SysMouseEm2, rguid))
		{
			pAddressX->SetAsMouse();

			if (Config.UseRawInputForMouse)
			{
				CDirectInputDeviceMouse8* pMouseInterface = new CDirectInputDeviceMouse8();
				if (pMouseInterface && !pMouseInterface->CheckInterface())
				{
					delete pMouseInterface;
					pMouseInterface = nullptr;
				}

				pAddressX->SetCDirectInputDeviceMouse8(pMouseInterface);
			}
		}
	}

	return hr;
}

template HRESULT m_IDirectInput8::EnumDevicesT<IDirectInput8A, LPDIENUMDEVICESCALLBACKA, DIDEVICEINSTANCEA>(DWORD, LPDIENUMDEVICESCALLBACKA, LPVOID, DWORD);
template HRESULT m_IDirectInput8::EnumDevicesT<IDirectInput8W, LPDIENUMDEVICESCALLBACKW, DIDEVICEINSTANCEW>(DWORD, LPDIENUMDEVICESCALLBACKW, LPVOID, DWORD);
template <class T, class V, class D>
HRESULT m_IDirectInput8::EnumDevicesT(DWORD dwDevType, V lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	auto now = std::chrono::steady_clock::now();
	auto& cachedData = GetEnumCache(GetProxyInterface<T>());

	// Check if cached data is valid
	if (dwDevType == cachedData.dwDevType && dwFlags == cachedData.dwFlags && (now - cachedData.lastUpdate) < cacheDuration && lpCallback)
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
		std::vector<D> CacheDevices;

		static BOOL CALLBACK DIEnumDevicesCallback(const D* lpddi, LPVOID pvRef)
		{
			EnumDevices* self = (EnumDevices*)pvRef;

			if (lpddi)
			{
				D ddi = {};
				ddi.dwSize = sizeof(ddi);
				memcpy(&ddi, lpddi, min(lpddi->dwSize, sizeof(ddi)));
				self->CacheDevices.push_back(ddi);
			}

			return DIENUM_CONTINUE;
		}
	} CallbackContext;

	HRESULT hr = GetProxyInterface<T>()->EnumDevices(dwDevType, (V)EnumDevices::DIEnumDevicesCallback, &CallbackContext, dwFlags);

	if (SUCCEEDED(hr) && lpCallback)
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

template HRESULT m_IDirectInput8::FindDeviceT<IDirectInput8A, LPCSTR>(REFGUID, LPCSTR, LPGUID);
template HRESULT m_IDirectInput8::FindDeviceT<IDirectInput8W, LPCWSTR>(REFGUID, LPCWSTR, LPGUID);
template <class T, class V>
HRESULT m_IDirectInput8::FindDeviceT(REFGUID rguidClass, V ptszName, LPGUID pguidInstance)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return GetProxyInterface<T>()->FindDevice(rguidClass, ptszName, pguidInstance);
}

template HRESULT m_IDirectInput8::EnumDevicesBySemanticsT<IDirectInput8A, LPCSTR, LPDIACTIONFORMATA, LPDIENUMDEVICESBYSEMANTICSCBA, LPCDIDEVICEINSTANCEA, LPDIRECTINPUTDEVICE8A>(LPCSTR, LPDIACTIONFORMATA, LPDIENUMDEVICESBYSEMANTICSCBA, LPVOID, DWORD);
template HRESULT m_IDirectInput8::EnumDevicesBySemanticsT<IDirectInput8W, LPCWSTR, LPDIACTIONFORMATW, LPDIENUMDEVICESBYSEMANTICSCBW, LPCDIDEVICEINSTANCEW, LPDIRECTINPUTDEVICE8W>(LPCWSTR, LPDIACTIONFORMATW, LPDIENUMDEVICESBYSEMANTICSCBW, LPVOID, DWORD);
template <class T, class V, class W, class X, class C, class D>
HRESULT m_IDirectInput8::EnumDevicesBySemanticsT(V ptszUserName, W lpdiActionFormat, X lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lpCallback)
	{
		return DIERR_INVALIDPARAM;
	}

	struct EnumDevice
	{
		LPVOID pvRef;
		X lpCallback;
		GUID WrapperDeviceID;

		static BOOL CALLBACK EnumDeviceCallback(C lpddi, D lpdid, DWORD dwFlags, DWORD dwRemaining, LPVOID pvRef)
		{
			EnumDevice *self = (EnumDevice*)pvRef;

			if (lpdid)
			{
				lpdid = ProxyAddressLookupTableDinput8.FindAddress<m_IDirectInputDevice8>(lpdid, self->WrapperDeviceID);
			}

			return self->lpCallback(lpddi, lpdid, dwFlags, dwRemaining, self->pvRef);
		}
	} CallbackContext;
	CallbackContext.pvRef = pvRef;
	CallbackContext.lpCallback = lpCallback;
	CallbackContext.WrapperDeviceID = WrapperDeviceID;

	return GetProxyInterface<T>()->EnumDevicesBySemantics(ptszUserName, lpdiActionFormat, EnumDevice::EnumDeviceCallback, &CallbackContext, dwFlags);
}

template HRESULT m_IDirectInput8::ConfigureDevicesT<IDirectInput8A, LPDICONFIGUREDEVICESPARAMSA>(LPDICONFIGUREDEVICESCALLBACK, LPDICONFIGUREDEVICESPARAMSA, DWORD, LPVOID);
template HRESULT m_IDirectInput8::ConfigureDevicesT<IDirectInput8W, LPDICONFIGUREDEVICESPARAMSW>(LPDICONFIGUREDEVICESCALLBACK, LPDICONFIGUREDEVICESPARAMSW, DWORD, LPVOID);
template <class T, class V>
HRESULT m_IDirectInput8::ConfigureDevicesT(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, V lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return GetProxyInterface<T>()->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
}
