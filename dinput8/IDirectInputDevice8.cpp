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

HRESULT m_IDirectInputDevice8::QueryInterface(REFIID riid, LPVOID* ppvObj)
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

ULONG m_IDirectInputDevice8::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirectInputDevice8::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

HRESULT m_IDirectInputDevice8::GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetCapabilities(lpDIDevCaps);
}

template HRESULT m_IDirectInputDevice8::EnumObjectsT<IDirectInputDevice8A, LPDIENUMDEVICEOBJECTSCALLBACKA>(LPDIENUMDEVICEOBJECTSCALLBACKA, LPVOID, DWORD);
template HRESULT m_IDirectInputDevice8::EnumObjectsT<IDirectInputDevice8W, LPDIENUMDEVICEOBJECTSCALLBACKW>(LPDIENUMDEVICEOBJECTSCALLBACKW, LPVOID, DWORD);
template <class T, class V>
HRESULT m_IDirectInputDevice8::EnumObjectsT(V lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return GetProxyInterface<T>()->EnumObjects(lpCallback, pvRef, dwFlags);
}

HRESULT m_IDirectInputDevice8::GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetProperty(rguidProp, pdiph);
}

HRESULT m_IDirectInputDevice8::SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Handle mouse input
	if (IsMouse && Config.FixHighFrequencyMouse)
	{
		const DWORD dwMinBufferSize = 128;

		// Verify buffer is large enough
		if ((DWORD)&rguidProp == (DWORD)&DIPROP_BUFFERSIZE)
		{
			DIPROPDWORD dipdw = (pdiph && pdiph->dwSize == sizeof(DIPROPDWORD)) ? *(LPDIPROPDWORD)pdiph :
				DIPROPDWORD{ sizeof(DIPROPDWORD), sizeof(DIPROPHEADER), 0, DIPH_DEVICE, 0 };
			dipdw.dwData = max(dipdw.dwData, dwMinBufferSize);
			MouseBufferSize = dipdw.dwData;

			return ProxyInterface->SetProperty(rguidProp, &dipdw.diph);
		}

		// Override deadzone for mice
		if ((DWORD)&rguidProp == (DWORD)&DIPROP_DEADZONE)
		{
			return DI_OK;
		}
	}

	return ProxyInterface->SetProperty(rguidProp, pdiph);
}

HRESULT m_IDirectInputDevice8::Acquire()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Acquire();
}

HRESULT m_IDirectInputDevice8::Unacquire()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Unacquire();
}

HRESULT m_IDirectInputDevice8::GetDeviceState(DWORD cbData, LPVOID lpvData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetDeviceState(cbData, lpvData);
}

HRESULT m_IDirectInputDevice8::GetMouseDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
	// Check arguments
	if (!pdwInOut || (rgdod && cbObjectData != sizeof(DIDEVICEOBJECTDATA) && cbObjectData != sizeof(DIDEVICEOBJECTDATA_DX3)))
	{
		if (pdwInOut)
		{
			*pdwInOut = 0;
		}
		return DIERR_INVALIDPARAM;
	}
	bool isPeek = (dwFlags == DIGDD_PEEK);

	// Lock for concurrency
	EnterCriticalSection(&dics);

	// Get latest mouse data from the DirectInput8 buffer
	if (*pdwInOut > dod.size())
	{
		// Get buffer
		DWORD dwItems = 0;
		LPDIDEVICEOBJECTDATA lpdod = (cbObjectData == sizeof(DIDEVICEOBJECTDATA_DX3)) ?
			GetObjectDataBuffer(dod_dx3, MouseBufferSize, dwItems) :
			GetObjectDataBuffer(dod_dx8, MouseBufferSize, dwItems);

		// Get device data from buffer
		HRESULT hr = ProxyInterface->GetDeviceData(cbObjectData, lpdod, &dwItems, 0);
		if (FAILED(hr))
		{
			// Unlock
			LeaveCriticalSection(&dics);

			return hr;
		}

		// Loop through buffer and merge like data
		bool isSet[3] = {};
		DWORD Loc[3] = {};
		for (UINT x = 0; x < dwItems; x++)
		{
			// Storing movement data
			if (lpdod->dwOfs == DIMOFS_X || lpdod->dwOfs == DIMOFS_Y || lpdod->dwOfs == DIMOFS_Z)
			{
				int v = lpdod->dwOfs == DIMOFS_X ? 0 :
					lpdod->dwOfs == DIMOFS_Y ? 1 :
					lpdod->dwOfs == DIMOFS_Z ? 2 : 0;

				// Check if record should be merged, if there is an existing record and the movement direction has not changed
				if (isSet[v] && !((dod[Loc[v]].lData < 0 && (LONG)lpdod->dwData > 0) || (dod[Loc[v]].lData > 0 && (LONG)lpdod->dwData < 0)))
				{
					// Updating movement data (merging records)
					dod[Loc[v]].lData += (LONG)lpdod->dwData;
					dod[Loc[v]].dwTimeStamp = lpdod->dwTimeStamp;
					dod[Loc[v]].dwSequence = lpdod->dwSequence;
					if (cbObjectData == sizeof(DIDEVICEOBJECTDATA))
					{
						dod[Loc[v]].uAppData = lpdod->uAppData;
					}
				}
				// Storing new movement data
				else
				{
					dod.push_back({ (LONG)lpdod->dwData, lpdod->dwOfs, lpdod->dwTimeStamp, lpdod->dwSequence, (cbObjectData == sizeof(DIDEVICEOBJECTDATA)) ? lpdod->uAppData : NULL });
					isSet[v] = true;
					Loc[v] = dod.size() - 1;
				}
			}
			// Storing button data
			else
			{
				dod.push_back({ (LONG)lpdod->dwData, lpdod->dwOfs, lpdod->dwTimeStamp, lpdod->dwSequence, (cbObjectData == sizeof(DIDEVICEOBJECTDATA)) ? lpdod->uAppData : NULL });

				// Reset movement data
				isSet[0] = false;
				isSet[1] = false;
				isSet[2] = false;
			}
			lpdod = (LPDIDEVICEOBJECTDATA)((DWORD)lpdod + cbObjectData);
		}
	}

	DWORD dwOut = 0;

	// Checking for overflow
	if (rgdod == nullptr && *pdwInOut == 0)
	{
		// Never return overflow
	}
	// Flush buffer
	else if (rgdod == nullptr && *pdwInOut == INFINITE && !isPeek)
	{
		dod.clear();
	}
	// Number of records in the buffer
	else if (rgdod == nullptr && *pdwInOut == INFINITE && isPeek)
	{
		dwOut = dod.size();
	}
	// Fill device object data
	else if (rgdod)
	{
		LPDIDEVICEOBJECTDATA p_rgdod = rgdod;

		for (DWORD i = 0; i < *pdwInOut; i++)
		{
			if (dwOut < dod.size())
			{
				p_rgdod->dwOfs = dod[dwOut].dwOfs;
				if (p_rgdod->dwOfs == DIMOFS_X)
				{
					LONG Sign = dod[dwOut].lData < 0 ? -1 : 1;
					p_rgdod->dwData = (LONG)round(dod[dwOut].lData * Config.MouseMovementFactor) + (Sign * Config.MouseMovementPadding);
				}
				else if (p_rgdod->dwOfs == DIMOFS_Y)
				{
					LONG Sign = dod[dwOut].lData < 0 ? -1 : 1;
					p_rgdod->dwData = (LONG)round(dod[dwOut].lData * abs(Config.MouseMovementFactor)) + (Sign * Config.MouseMovementPadding);
				}
				else
				{
					p_rgdod->dwData = dod[dwOut].lData;
				}
				p_rgdod->dwTimeStamp = dod[dwOut].dwTimeStamp;
				p_rgdod->dwSequence = dod[dwOut].dwSequence;
				if (cbObjectData == sizeof(DIDEVICEOBJECTDATA))
				{
					p_rgdod->uAppData = dod[dwOut].uAppData;
				}

				dwOut++;
			}
			// No more data to sent
			else
			{
				break;
			}
			p_rgdod = (LPDIDEVICEOBJECTDATA)((DWORD)p_rgdod + cbObjectData);
		}
	}

	// Remove used entries from buffer
	if (!isPeek && dwOut)
	{
		// Save unsent mouse data
		if (dwOut < dod.size())
		{
			std::vector<MOUSECACHEDATA> tmp_dod;
			for (size_t x = dwOut; x < dod.size(); x++)
			{
				tmp_dod.push_back(dod[x]);
			}
			dod = std::move(tmp_dod);
		}
		// Clear buffer if all entries were used
		else
		{
			dod.clear();
		}
	}

	// Unlock
	LeaveCriticalSection(&dics);

	*pdwInOut = dwOut;

	return DI_OK;
}

HRESULT m_IDirectInputDevice8::GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.FilterNonActiveInput && pdwInOut)
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

	if (IsMouse && Config.FixHighFrequencyMouse)
	{
		return GetMouseDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
	}

	return ProxyInterface->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
}

HRESULT m_IDirectInputDevice8::SetDataFormat(LPCDIDATAFORMAT lpdf)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetDataFormat(lpdf);
}

HRESULT m_IDirectInputDevice8::SetEventNotification(HANDLE hEvent)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetEventNotification(hEvent);
}

HRESULT m_IDirectInputDevice8::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetCooperativeLevel(hwnd, dwFlags);
}

template HRESULT m_IDirectInputDevice8::GetObjectInfoT<IDirectInputDevice8A, LPDIDEVICEOBJECTINSTANCEA>(LPDIDEVICEOBJECTINSTANCEA, DWORD, DWORD);
template HRESULT m_IDirectInputDevice8::GetObjectInfoT<IDirectInputDevice8W, LPDIDEVICEOBJECTINSTANCEW>(LPDIDEVICEOBJECTINSTANCEW, DWORD, DWORD);
template <class T, class V>
HRESULT m_IDirectInputDevice8::GetObjectInfoT(V pdidoi, DWORD dwObj, DWORD dwHow)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return GetProxyInterface<T>()->GetObjectInfo(pdidoi, dwObj, dwHow);
}

template HRESULT m_IDirectInputDevice8::GetDeviceInfoT<IDirectInputDevice8A, LPDIDEVICEINSTANCEA>(LPDIDEVICEINSTANCEA);
template HRESULT m_IDirectInputDevice8::GetDeviceInfoT<IDirectInputDevice8W, LPDIDEVICEINSTANCEW>(LPDIDEVICEINSTANCEW);
template <class T, class V>
HRESULT m_IDirectInputDevice8::GetDeviceInfoT(V pdidi)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return GetProxyInterface<T>()->GetDeviceInfo(pdidi);
}

HRESULT m_IDirectInputDevice8::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->RunControlPanel(hwndOwner, dwFlags);
}

HRESULT m_IDirectInputDevice8::Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Initialize(hinst, dwVersion, rguid);
}

HRESULT m_IDirectInputDevice8::CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT * ppdeff, LPUNKNOWN punkOuter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->CreateEffect(rguid, lpeff, ppdeff, punkOuter);

	if (SUCCEEDED(hr) && ppdeff)
	{
		*ppdeff = new m_IDirectInputEffect8(*ppdeff, IID_IDirectInputEffect);
	}

	return hr;
}

template HRESULT m_IDirectInputDevice8::EnumEffectsT<IDirectInputDevice8A, LPDIENUMEFFECTSCALLBACKA>(LPDIENUMEFFECTSCALLBACKA, LPVOID, DWORD);
template HRESULT m_IDirectInputDevice8::EnumEffectsT<IDirectInputDevice8W, LPDIENUMEFFECTSCALLBACKW>(LPDIENUMEFFECTSCALLBACKW, LPVOID, DWORD);
template <class T, class V>
HRESULT m_IDirectInputDevice8::EnumEffectsT(V lpCallback, LPVOID pvRef, DWORD dwEffType)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return GetProxyInterface<T>()->EnumEffects(lpCallback, pvRef, dwEffType);
}

template HRESULT m_IDirectInputDevice8::GetEffectInfoT<IDirectInputDevice8A, LPDIEFFECTINFOA>(LPDIEFFECTINFOA, REFGUID);
template HRESULT m_IDirectInputDevice8::GetEffectInfoT<IDirectInputDevice8W, LPDIEFFECTINFOW>(LPDIEFFECTINFOW, REFGUID);
template <class T, class V>
HRESULT m_IDirectInputDevice8::GetEffectInfoT(V pdei, REFGUID rguid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return GetProxyInterface<T>()->GetEffectInfo(pdei, rguid);
}

HRESULT m_IDirectInputDevice8::GetForceFeedbackState(LPDWORD pdwOut)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetForceFeedbackState(pdwOut);
}

HRESULT m_IDirectInputDevice8::SendForceFeedbackCommand(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SendForceFeedbackCommand(dwFlags);
}

HRESULT m_IDirectInputDevice8::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lpCallback)
	{
		return DIERR_INVALIDPARAM;
	}

	struct EnumEffect
	{
		LPVOID pvRef = nullptr;
		LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback = nullptr;

		static BOOL CALLBACK EnumEffectCallback(LPDIRECTINPUTEFFECT a, LPVOID pvRef)
		{
			EnumEffect *self = (EnumEffect*)pvRef;

			if (a)
			{
				a = ProxyAddressLookupTableDinput8.FindAddress<m_IDirectInputEffect8>(a, IID_IDirectInputEffect);
			}

			return self->lpCallback(a, self->pvRef);
		}
	} CallbackContext;
	CallbackContext.pvRef = pvRef;
	CallbackContext.lpCallback = lpCallback;

	return ProxyInterface->EnumCreatedEffectObjects(EnumEffect::EnumEffectCallback, &CallbackContext, fl);
}

HRESULT m_IDirectInputDevice8::Escape(LPDIEFFESCAPE pesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Escape(pesc);
}

HRESULT m_IDirectInputDevice8::Poll()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Poll();
}

HRESULT m_IDirectInputDevice8::SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SendDeviceData(cbObjectData, rgdod, pdwInOut, fl);
}

template HRESULT m_IDirectInputDevice8::EnumEffectsInFileT<IDirectInputDevice8A, LPCSTR>(LPCSTR, LPDIENUMEFFECTSINFILECALLBACK, LPVOID, DWORD);
template HRESULT m_IDirectInputDevice8::EnumEffectsInFileT<IDirectInputDevice8W, LPCWSTR>(LPCWSTR, LPDIENUMEFFECTSINFILECALLBACK, LPVOID, DWORD);
template <class T, class V>
HRESULT m_IDirectInputDevice8::EnumEffectsInFileT(V lpszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return GetProxyInterface<T>()->EnumEffectsInFile(lpszFileName, pec, pvRef, dwFlags);
}

template HRESULT m_IDirectInputDevice8::WriteEffectToFileT<IDirectInputDevice8A, LPCSTR>(LPCSTR, DWORD, LPDIFILEEFFECT, DWORD);
template HRESULT m_IDirectInputDevice8::WriteEffectToFileT<IDirectInputDevice8W, LPCWSTR>(LPCWSTR, DWORD, LPDIFILEEFFECT, DWORD);
template <class T, class V>
HRESULT m_IDirectInputDevice8::WriteEffectToFileT(V lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return GetProxyInterface<T>()->WriteEffectToFile(lpszFileName, dwEntries, rgDiFileEft, dwFlags);
}

template HRESULT m_IDirectInputDevice8::BuildActionMapT<IDirectInputDevice8A, LPDIACTIONFORMATA, LPCSTR>(LPDIACTIONFORMATA, LPCSTR, DWORD);
template HRESULT m_IDirectInputDevice8::BuildActionMapT<IDirectInputDevice8W, LPDIACTIONFORMATW, LPCWSTR>(LPDIACTIONFORMATW, LPCWSTR, DWORD);
template <class T, class V, class W>
HRESULT m_IDirectInputDevice8::BuildActionMapT(V lpdiaf, W lpszUserName, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return GetProxyInterface<T>()->BuildActionMap(lpdiaf, lpszUserName, dwFlags);
}

template HRESULT m_IDirectInputDevice8::SetActionMapT<IDirectInputDevice8A, LPDIACTIONFORMATA, LPCSTR>(LPDIACTIONFORMATA, LPCSTR, DWORD);
template HRESULT m_IDirectInputDevice8::SetActionMapT<IDirectInputDevice8W, LPDIACTIONFORMATW, LPCWSTR>(LPDIACTIONFORMATW, LPCWSTR, DWORD);
template <class T, class V, class W>
HRESULT m_IDirectInputDevice8::SetActionMapT(V lpdiActionFormat, W lptszUserName, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return GetProxyInterface<T>()->SetActionMap(lpdiActionFormat, lptszUserName, dwFlags);
}

template HRESULT m_IDirectInputDevice8::GetImageInfoT<IDirectInputDevice8A, LPDIDEVICEIMAGEINFOHEADERA>(LPDIDEVICEIMAGEINFOHEADERA);
template HRESULT m_IDirectInputDevice8::GetImageInfoT<IDirectInputDevice8W, LPDIDEVICEIMAGEINFOHEADERW>(LPDIDEVICEIMAGEINFOHEADERW);
template <class T, class V>
HRESULT m_IDirectInputDevice8::GetImageInfoT(V lpdiDevImageInfoHeader)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return GetProxyInterface<T>()->GetImageInfo(lpdiDevImageInfoHeader);
}
