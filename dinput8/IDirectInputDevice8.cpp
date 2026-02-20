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
#include "ScopeGuard.h"
#include "GDI\WndProc.h"

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

	HRESULT hr = ProxyInterface->GetProperty(rguidProp, pdiph);

	if (SUCCEEDED(hr))
	{
		// Handle mouse input buffer size
		if (IsMouse && Config.FixHighFrequencyMouse)
		{
			if ((DWORD)&rguidProp == (DWORD)&DIPROP_BUFFERSIZE && RequestedMouseBufferSize)
			{
				if (pdiph && pdiph->dwSize == sizeof(DIPROPDWORD))
				{
					DIPROPDWORD* pOut = reinterpret_cast<DIPROPDWORD*>(pdiph);
					pOut->dwData = RequestedMouseBufferSize;
				}
			}
		}
	}

	return hr;
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
			if (pdiph && pdiph->dwSize == sizeof(DIPROPDWORD))
			{
				const DIPROPDWORD* pIn = reinterpret_cast<const DIPROPDWORD*>(pdiph);

				DIPROPDWORD dipdw = *pIn;

				DWORD requested = dipdw.dwData ? dipdw.dwData : RequestedMouseBufferSize;
				dipdw.dwData = max(dipdw.dwData, dwMinBufferSize);

				if (requested == 0)
				{
					DIPROPDWORD r_dipdw = {};
					r_dipdw.diph.dwSize = sizeof(DIPROPDWORD);
					r_dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
					r_dipdw.diph.dwObj = 0;
					r_dipdw.diph.dwHow = DIPH_DEVICE;

					if (SUCCEEDED(ProxyInterface->GetProperty(DIPROP_BUFFERSIZE, &r_dipdw.diph)))
					{
						requested = r_dipdw.dwData;
					}
				}

				HRESULT hr = ProxyInterface->SetProperty(rguidProp, &dipdw.diph);

				if (SUCCEEDED(hr))
				{
					if (dipdw.dwData > MouseBufferSize)
					{
						SentBufferOverflow = false;		// Send buffer overflow message again once buffer size is increased
					}

					RequestedMouseBufferSize = requested;
					MouseBufferSize = dipdw.dwData;
				}

				return hr;
			}
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

	HRESULT hr = ProxyInterface->GetDeviceState(cbData, lpvData);

	if (SUCCEEDED(hr) && lpvData)
	{
		if (IsMouse && Config.FixHighFrequencyMouse)
		{
			// Apply movement adjustment using stored offsets
			if (Ofs.x >= 0)
			{
				AdjustMouseAxis(*((LONG*)((BYTE*)lpvData + Ofs.x)), false);
			}
			if (Ofs.y >= 0)
			{
				AdjustMouseAxis(*((LONG*)((BYTE*)lpvData + Ofs.y)), true);
			}
		}
	}

	return hr;
}

template <class T>
HRESULT m_IDirectInputDevice8::GetMouseDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags, std::vector<T>& dod)
{
	// Check arguments
	if (!pdwInOut || (rgdod && *pdwInOut == 0))
	{
		return DIERR_INVALIDPARAM;
	}
	if (cbObjectData != sizeof(T))
	{
		*pdwInOut = 0;
		return DIERR_INVALIDPARAM;
	}
	const bool isPeek = (dwFlags == DIGDD_PEEK);
	const bool isFlushingData = (rgdod == nullptr && *pdwInOut == INFINITE && !isPeek);
	const bool isRequestingOverflow = (rgdod == nullptr && *pdwInOut == 0);

	// Lock for concurrency
	ScopedCriticalSection ThreadLock(&dics);

	bool isBufferOverflow = false;
	DWORD dwItems = INFINITE;

	// Determine number of records to store
	HRESULT hr = ProxyInterface->GetDeviceData(cbObjectData, nullptr, &dwItems, DIGDD_PEEK);
	if (hr == DI_BUFFEROVERFLOW)
	{
		isBufferOverflow = true;
	}
	else if (FAILED(hr))
	{
		*pdwInOut = 0;
		return hr;
	}
	if (dwItems == INFINITE)
	{
		dwItems = MouseBufferSize ? MouseBufferSize : 32;	// Resonable default
	}

	// Ensure buffer is large enough
	const DWORD requiredBytes = dwItems * cbObjectData;
	if (tmp_dod.size() < requiredBytes)
	{
		tmp_dod.resize(requiredBytes);
	}

	// Pointer to first record
	LPDIDEVICEOBJECTDATA lpdod = reinterpret_cast<LPDIDEVICEOBJECTDATA>(tmp_dod.data());

	// Get device data from buffer
	hr = ProxyInterface->GetDeviceData(cbObjectData, lpdod, &dwItems, 0);
	if (hr == DI_BUFFEROVERFLOW)
	{
		isBufferOverflow = true;
	}
	else if (FAILED(hr))
	{
		*pdwInOut = 0;
		return hr;
	}

	// Merge and store device object data
	if (!isFlushingData && dwItems)
	{
		bool isSet[3] = { false };
		DWORD Loc[3] = { 0 };

		if (SequenceCounter == 0)
		{
			SequenceCounter = lpdod->dwSequence;
		}
		if (LastSentSequenceCounter && std::abs((LONG)LastSentSequenceCounter - (LONG)lpdod->dwSequence) > 1)
		{
			isBufferOverflow = true;
		}

		// Loop through buffer and merge like data
		for (UINT x = 0; x < dwItems; x++)
		{
			// Handle movement data
			if (lpdod->dwOfs == Ofs.x || lpdod->dwOfs == Ofs.y || lpdod->dwOfs == Ofs.z)
			{
				const int v = lpdod->dwOfs == Ofs.x ? 0 : lpdod->dwOfs == Ofs.y ? 1 : 2;

				// Merge data only
				if (isSet[v])
				{
					dod[Loc[v]].lData += (LONG)lpdod->dwData;

					lpdod = (LPDIDEVICEOBJECTDATA)((BYTE*)lpdod + cbObjectData);
					continue;
				}
				// Storing new movement data
				else
				{
					isSet[v] = true;
					Loc[v] = dod.size();
				}
			}

			// Store data
			if constexpr (std::is_same_v<T, MOUSECACHEDATA_DX3>)
			{
				dod.push_back({ lpdod->dwOfs, (LONG)lpdod->dwData, lpdod->dwTimeStamp, SequenceCounter++ });
			}
			else
			{
				dod.push_back({ lpdod->dwOfs, (LONG)lpdod->dwData, lpdod->dwTimeStamp, SequenceCounter++, lpdod->uAppData });
			}
			LastSentSequenceCounter = lpdod->dwSequence;

			lpdod = (LPDIDEVICEOBJECTDATA)((BYTE*)lpdod + cbObjectData);
		}

		// Update data for mouse factor and padding
		for (DWORD item : { Ofs.x, Ofs.y })
		{
			const int v = item == Ofs.x ? 0 : item == Ofs.y ? 1 : 2;

			if (isSet[v])
			{
				AdjustMouseAxis(dod[Loc[v]].lData, (item == Ofs.y));
			}
		}
	}

	// Check buffer size
	if (MouseBufferSize == 0)
	{
		DIPROPDWORD dipdw = {};
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;

		SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	}

	// Check for buffer overflow
	if (RequestedMouseBufferSize && RequestedMouseBufferSize < dod.size())
	{
		isBufferOverflow = true;
	}

	DWORD dwOut = 0;

	// Checking for device object data
	if (rgdod == nullptr)
	{
		// Flush buffer
		if (isFlushingData)
		{
			dod.clear();
		}
		// Get number of records in the buffer
		else
		{
			dwOut = dod.size();
		}
	}
	// Fill device object data
	else
	{
		DWORD requested = *pdwInOut;
		if (requested == INFINITE && RequestedMouseBufferSize)
		{
			requested = RequestedMouseBufferSize;
		}
		dwOut = min(dod.size(), requested);

		memcpy(rgdod, dod.data(), sizeof(T) * dwOut);

		// Remove used entries from buffer
		if (dwOut && !isPeek)
		{
			if (dwOut < dod.size())
			{
				dod.erase(dod.begin(), dod.begin() + dwOut);
			}
			else
			{
				dod.clear();
			}
		}
	}

	*pdwInOut = dwOut;

	if (isBufferOverflow)
	{
		SequenceCounter += 5;	// Simulate overflow

		if (isRequestingOverflow || !SentBufferOverflow)
		{
			SentBufferOverflow = true;
			return DI_BUFFEROVERFLOW;
		}
	}

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
		if (rgdod && cbObjectData != sizeof(DIDEVICEOBJECTDATA_DX3) && cbObjectData != sizeof(DIDEVICEOBJECTDATA))
		{
			if (pdwInOut)
			{
				*pdwInOut = 0;
			}
			return DIERR_INVALIDPARAM;
		}
		if (cbObjectData == sizeof(DIDEVICEOBJECTDATA_DX3) || cbObjectData == sizeof(DIDEVICEOBJECTDATA))
		{
			LastObjectSize = cbObjectData;
		}
		else
		{
			cbObjectData = LastObjectSize ? LastObjectSize : sizeof(DIDEVICEOBJECTDATA);
		}
		if (cbObjectData == sizeof(DIDEVICEOBJECTDATA_DX3))
		{
			if (dod_dx8.size())
			{
				// Lock for concurrency
				ScopedCriticalSection ThreadLock(&dics);

				for (auto& item : dod_dx8)
				{
					dod_dx3.push_back({ item.dwOfs, item.lData, item.dwTimeStamp, item.dwSequence });
				}
				dod_dx8.clear();
			}

			return GetMouseDeviceData<MOUSECACHEDATA_DX3>(sizeof(DIDEVICEOBJECTDATA_DX3), rgdod, pdwInOut, dwFlags, dod_dx3);
		}
		else
		{
			return GetMouseDeviceData<MOUSECACHEDATA>(sizeof(DIDEVICEOBJECTDATA), rgdod, pdwInOut, dwFlags, dod_dx8);
		}
	}

	return ProxyInterface->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
}

HRESULT m_IDirectInputDevice8::SetDataFormat(LPCDIDATAFORMAT lpdf)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->SetDataFormat(lpdf);

	if (SUCCEEDED(hr))
	{
		if (IsMouse && lpdf && lpdf->rgodf && lpdf->dwNumObjs)
		{
			for (DWORD i = 0; i < lpdf->dwNumObjs; i++)
			{
				const DIOBJECTDATAFORMAT& obj = lpdf->rgodf[i];

				// Check if this object is an axis (relative)
				if (DIDFT_GETTYPE(obj.dwType) & DIDFT_RELAXIS)
				{
					if (obj.pguid && IsEqualGUID(GUID_XAxis, *obj.pguid))
					{
						Ofs.x = obj.dwOfs;
					}
					else if (obj.pguid && IsEqualGUID(GUID_YAxis, *obj.pguid))
					{
						Ofs.y = obj.dwOfs;
					}
					else if (obj.pguid && IsEqualGUID(GUID_ZAxis, *obj.pguid))
					{
						Ofs.z = obj.dwOfs;
					}
				}
			}
		}
	}

	return hr;
}

HRESULT m_IDirectInputDevice8::SetEventNotification(HANDLE hEvent)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetEventNotification(hEvent);
}

HRESULT m_IDirectInputDevice8::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	WndProc::AddWndProc(hwnd);

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

// Helper functions
void m_IDirectInputDevice8::AdjustMouseAxis(LONG& value, bool isY)
{
	if (value != 0)
	{
		double factor = isY ? Config.MouseMovementFactor : std::abs(Config.MouseMovementFactor);
		LONG baseMovement = (LONG)round(value * factor);

		if (std::abs(baseMovement) < (LONG)Config.MouseMovementPadding)
		{
			value = (value < 0) ? -(LONG)Config.MouseMovementPadding : (LONG)Config.MouseMovementPadding;
		}
		else
		{
			value = baseMovement;
		}
	}
}
