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

HRESULT m_IDirectInputDevice8W::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if ((riid == IID_IDirectInputDevice8W || riid == IID_IUnknown) && ppvObj)
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

ULONG m_IDirectInputDevice8W::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirectInputDevice8W::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

HRESULT m_IDirectInputDevice8W::GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetCapabilities(lpDIDevCaps);
}

HRESULT m_IDirectInputDevice8W::EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->EnumObjects(lpCallback, pvRef, dwFlags);
}

HRESULT m_IDirectInputDevice8W::GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetProperty(rguidProp, pdiph);
}

HRESULT m_IDirectInputDevice8W::SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetProperty(rguidProp, pdiph);
}

HRESULT m_IDirectInputDevice8W::Acquire()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Acquire();
}

HRESULT m_IDirectInputDevice8W::Unacquire()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Unacquire();
}

HRESULT m_IDirectInputDevice8W::GetDeviceState(DWORD cbData, LPVOID lpvData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetDeviceState(cbData, lpvData);
}

HRESULT m_IDirectInputDevice8W::GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.FilterNonActiveInput)
	{
		// Check foreground window's process and don't copy device data if process is not active
		HWND hfgwnd = GetForegroundWindow();
		if (hfgwnd)
		{
			DWORD fgwndprocid = 0;
			GetWindowThreadProcessId(hfgwnd, &fgwndprocid);

			if (ProcessID != fgwndprocid)
			{
				// Foreground window belongs to another process, don't copy the device data
				*pdwInOut = 0;
			}
		}
	}

	HRESULT hr = ProxyInterface->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);

	// Several games handle DI_BUFFEROVERFLOW as failure
	return IsMouse && hr == DI_BUFFEROVERFLOW && rgdod && pdwInOut && *pdwInOut > 0 ? DI_OK : hr;
}

HRESULT m_IDirectInputDevice8W::SetDataFormat(LPCDIDATAFORMAT lpdf)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetDataFormat(lpdf);
}

HRESULT m_IDirectInputDevice8W::SetEventNotification(HANDLE hEvent)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetEventNotification(hEvent);
}

HRESULT m_IDirectInputDevice8W::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetCooperativeLevel(hwnd, dwFlags);
}

HRESULT m_IDirectInputDevice8W::GetObjectInfo(LPDIDEVICEOBJECTINSTANCEW pdidoi, DWORD dwObj, DWORD dwHow)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetObjectInfo(pdidoi, dwObj, dwHow);
}

HRESULT m_IDirectInputDevice8W::GetDeviceInfo(LPDIDEVICEINSTANCEW pdidi)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetDeviceInfo(pdidi);
}

HRESULT m_IDirectInputDevice8W::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->RunControlPanel(hwndOwner, dwFlags);
}

HRESULT m_IDirectInputDevice8W::Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Initialize(hinst, dwVersion, rguid);
}

HRESULT m_IDirectInputDevice8W::CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT * ppdeff, LPUNKNOWN punkOuter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->CreateEffect(rguid, lpeff, ppdeff, punkOuter);

	if (SUCCEEDED(hr) && ppdeff)
	{
		*ppdeff = new m_IDirectInputEffect8(*ppdeff);
	}

	return hr;
}

HRESULT m_IDirectInputDevice8W::EnumEffects(LPDIENUMEFFECTSCALLBACKW lpCallback, LPVOID pvRef, DWORD dwEffType)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->EnumEffects(lpCallback, pvRef, dwEffType);
}

HRESULT m_IDirectInputDevice8W::GetEffectInfo(LPDIEFFECTINFOW pdei, REFGUID rguid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetEffectInfo(pdei, rguid);
}

HRESULT m_IDirectInputDevice8W::GetForceFeedbackState(LPDWORD pdwOut)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetForceFeedbackState(pdwOut);
}

HRESULT m_IDirectInputDevice8W::SendForceFeedbackCommand(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SendForceFeedbackCommand(dwFlags);
}

HRESULT m_IDirectInputDevice8W::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lpCallback)
	{
		return DIERR_INVALIDPARAM;
	}

	struct EnumEffect
	{
		LPVOID pvRef;
		LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback;

		static BOOL CALLBACK EnumEffectCallback(LPDIRECTINPUTEFFECT a, LPVOID pvRef)
		{
			EnumEffect *self = (EnumEffect*)pvRef;

			if (a)
			{
				a = ProxyAddressLookupTableDinput8.FindAddress<m_IDirectInputEffect8>(a);
			}

			return self->lpCallback(a, self->pvRef);
		}
	} CallbackContext;
	CallbackContext.pvRef = pvRef;
	CallbackContext.lpCallback = lpCallback;

	return ProxyInterface->EnumCreatedEffectObjects(EnumEffect::EnumEffectCallback, &CallbackContext, fl);
}

HRESULT m_IDirectInputDevice8W::Escape(LPDIEFFESCAPE pesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Escape(pesc);
}

HRESULT m_IDirectInputDevice8W::Poll()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Poll();
}

HRESULT m_IDirectInputDevice8W::SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SendDeviceData(cbObjectData, rgdod, pdwInOut, fl);
}

HRESULT m_IDirectInputDevice8W::EnumEffectsInFile(LPCWSTR lpszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->EnumEffectsInFile(lpszFileName, pec, pvRef, dwFlags);
}

HRESULT m_IDirectInputDevice8W::WriteEffectToFile(LPCWSTR lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->WriteEffectToFile(lpszFileName, dwEntries, rgDiFileEft, dwFlags);
}

HRESULT m_IDirectInputDevice8W::BuildActionMap(LPDIACTIONFORMATW lpdiaf, LPCWSTR lpszUserName, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->BuildActionMap(lpdiaf, lpszUserName, dwFlags);
}

HRESULT m_IDirectInputDevice8W::SetActionMap(LPDIACTIONFORMATW lpdiActionFormat, LPCWSTR lptszUserName, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetActionMap(lpdiActionFormat, lptszUserName, dwFlags);
}

HRESULT m_IDirectInputDevice8W::GetImageInfo(LPDIDEVICEIMAGEINFOHEADERW lpdiDevImageInfoHeader)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetImageInfo(lpdiDevImageInfoHeader);
}
