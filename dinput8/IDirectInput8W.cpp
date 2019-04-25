/**
* Copyright (C) 2019 Elisha Riedlinger
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

HRESULT m_IDirectInput8W::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if ((riid == IID_IDirectInput8W || riid == IID_IUnknown) && ppvObj)
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

ULONG m_IDirectInput8W::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectInput8W::Release()
{
	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

HRESULT m_IDirectInput8W::CreateDevice(REFGUID rguid, LPDIRECTINPUTDEVICE8W *lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
{
	HRESULT hr = ProxyInterface->CreateDevice(rguid, lplpDirectInputDevice, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDirectInputDevice)
	{
		*lplpDirectInputDevice = ProxyAddressLookupTableDinput8.FindAddress<m_IDirectInputDevice8W>(*lplpDirectInputDevice);
	}

	return hr;
}

HRESULT m_IDirectInput8W::EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	return ProxyInterface->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
}

HRESULT m_IDirectInput8W::GetDeviceStatus(REFGUID rguidInstance)
{
	return ProxyInterface->GetDeviceStatus(rguidInstance);
}

HRESULT m_IDirectInput8W::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
	return ProxyInterface->RunControlPanel(hwndOwner, dwFlags);
}

HRESULT m_IDirectInput8W::Initialize(HINSTANCE hinst, DWORD dwVersion)
{
	return ProxyInterface->Initialize(hinst, dwVersion);
}

HRESULT m_IDirectInput8W::FindDevice(REFGUID rguidClass, LPCWSTR ptszName, LPGUID pguidInstance)
{
	return ProxyInterface->FindDevice(rguidClass, ptszName, pguidInstance);
}

HRESULT m_IDirectInput8W::EnumDevicesBySemantics(LPCWSTR ptszUserName, LPDIACTIONFORMATW lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCBW lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	if (!lpCallback)
	{
		return DIERR_INVALIDPARAM;
	}

	ENUMDEVICEW CallbackContext;
	CallbackContext.pvRef = pvRef;
	CallbackContext.lpCallback = lpCallback;

	return ProxyInterface->EnumDevicesBySemantics(ptszUserName, lpdiActionFormat, m_IDirectInputEnumDevice::EnumDeviceCallbackW, &CallbackContext, dwFlags);
}

HRESULT m_IDirectInput8W::ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMSW lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
{
	return ProxyInterface->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
}
