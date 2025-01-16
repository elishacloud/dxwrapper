/**
* Copyright (C) 2024 Elisha Riedlinger
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

#include "dsound.h"
#include "dsoundExternal.h"
#include "IClassFactory\IClassFactory.h"

AddressLookupTableDsound<void> ProxyAddressLookupTableDsound = AddressLookupTableDsound<void>();

namespace DsoundWrapper
{
	VISIT_PROCS_DSOUND(INITIALIZE_OUT_WRAPPED_PROC);
	VISIT_PROCS_DSOUND_SHARED(INITIALIZE_OUT_WRAPPED_PROC);
}

using namespace DsoundWrapper;

HRESULT WINAPI ds_DirectSoundCreate(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DirectSoundCreateProc, DirectSoundCreate, DirectSoundCreate_out);

	if (!DirectSoundCreate)
	{
		return DSERR_GENERIC;
	}

	LOG_LIMIT(3, "Redirecting 'DirectSoundCreate' ...");

	HRESULT hr = DirectSoundCreate(pcGuidDevice, ppDS, pUnkOuter);

	if (SUCCEEDED(hr) && ppDS)
	{
		*ppDS = new m_IDirectSound8((IDirectSound8*)*ppDS);
	}

	return hr;
}

HRESULT WINAPI ds_DirectSoundEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DirectSoundEnumerateAProc, DirectSoundEnumerateA, DirectSoundEnumerateA_out);

	if (!DirectSoundEnumerateA)
	{
		return DSERR_GENERIC;
	}

	return DirectSoundEnumerateA(pDSEnumCallback, pContext);
}

HRESULT WINAPI ds_DirectSoundEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DirectSoundEnumerateWProc, DirectSoundEnumerateW, DirectSoundEnumerateW_out);

	if (!DirectSoundEnumerateW)
	{
		return DSERR_GENERIC;
	}

	return DirectSoundEnumerateW(pDSEnumCallback, pContext);
}

HRESULT WINAPI ds_DllCanUnloadNow()
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DllCanUnloadNowProc, DllCanUnloadNow, DllCanUnloadNow_out);

	if (!DllCanUnloadNow)
	{
		return DSERR_GENERIC;
	}

	return DllCanUnloadNow();
}

HRESULT WINAPI ds_DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DllGetClassObjectProc, DllGetClassObject, DllGetClassObject_out);

	if (!DllGetClassObject)
	{
		return DSERR_GENERIC;
	}

	HRESULT hr = DllGetClassObject(rclsid, riid, ppv);

	if (SUCCEEDED(hr) && ppv)
	{
		if (riid == IID_IClassFactory)
		{
			*ppv = new m_IClassFactory((IClassFactory*)*ppv, genericQueryInterface);

			((m_IClassFactory*)(*ppv))->SetCLSID(rclsid);

			return DS_OK;
		}

		genericQueryInterface(riid, ppv);
	}

	return hr;
}

HRESULT WINAPI ds_DirectSoundCaptureCreate(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE *ppDSC, LPUNKNOWN pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DirectSoundCaptureCreateProc, DirectSoundCaptureCreate, DirectSoundCaptureCreate_out);

	if (!DirectSoundCaptureCreate)
	{
		return DSERR_GENERIC;
	}

	HRESULT hr = DirectSoundCaptureCreate(pcGuidDevice, ppDSC, pUnkOuter);

	if (SUCCEEDED(hr) && ppDSC)
	{
		*ppDSC = new m_IDirectSoundCapture8(*ppDSC);
	}

	return hr;
}

HRESULT WINAPI ds_DirectSoundCaptureEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DirectSoundCaptureEnumerateAProc, DirectSoundCaptureEnumerateA, DirectSoundCaptureEnumerateA_out);

	if (!DirectSoundCaptureEnumerateA)
	{
		return DSERR_GENERIC;
	}

	return DirectSoundCaptureEnumerateA(pDSEnumCallback, pContext);
}

HRESULT WINAPI ds_DirectSoundCaptureEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DirectSoundCaptureEnumerateWProc, DirectSoundCaptureEnumerateW, DirectSoundCaptureEnumerateW_out);

	if (!DirectSoundCaptureEnumerateW)
	{
		return DSERR_GENERIC;
	}

	return DirectSoundCaptureEnumerateW(pDSEnumCallback, pContext);
}

HRESULT WINAPI ds_GetDeviceID(LPCGUID pGuidSrc, LPGUID pGuidDest)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(GetDeviceIDProc, GetDeviceID, GetDeviceID_out);

	if (!GetDeviceID)
	{
		return DSERR_GENERIC;
	}

	return GetDeviceID(pGuidSrc, pGuidDest);
}

HRESULT WINAPI ds_DirectSoundFullDuplexCreate(LPCGUID pcGuidCaptureDevice, LPCGUID pcGuidRenderDevice, LPCDSCBUFFERDESC pcDSCBufferDesc, LPCDSBUFFERDESC pcDSBufferDesc, HWND hWnd,
	DWORD dwLevel, LPDIRECTSOUNDFULLDUPLEX* ppDSFD, LPDIRECTSOUNDCAPTUREBUFFER8 *ppDSCBuffer8, LPDIRECTSOUNDBUFFER8 *ppDSBuffer8, LPUNKNOWN pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DirectSoundFullDuplexCreateProc, DirectSoundFullDuplexCreate, DirectSoundFullDuplexCreate_out);

	if (!DirectSoundFullDuplexCreate)
	{
		return DSERR_GENERIC;
	}

	HRESULT hr = DirectSoundFullDuplexCreate(pcGuidCaptureDevice, pcGuidRenderDevice, pcDSCBufferDesc, pcDSBufferDesc, hWnd, dwLevel, ppDSFD, ppDSCBuffer8, ppDSBuffer8, pUnkOuter);

	if (SUCCEEDED(hr))
	{
		if (ppDSFD)
		{
			*ppDSFD = new m_IDirectSoundFullDuplex8(*ppDSFD);
		}
		if (ppDSCBuffer8)
		{
			*ppDSCBuffer8 = new m_IDirectSoundCaptureBuffer8(*ppDSCBuffer8);
		}
		if (ppDSBuffer8)
		{
			*ppDSBuffer8 = new m_IDirectSoundBuffer8(*ppDSBuffer8);
		}
	}

	return hr;
}

HRESULT WINAPI ds_DirectSoundCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DirectSoundCreate8Proc, DirectSoundCreate8, DirectSoundCreate8_out);

	if (!DirectSoundCreate8)
	{
		return DSERR_GENERIC;
	}

	LOG_LIMIT(3, "Redirecting 'DirectSoundCreate8' ...");

	HRESULT hr = DirectSoundCreate8(pcGuidDevice, ppDS8, pUnkOuter);

	if (SUCCEEDED(hr) && ppDS8)
	{
		*ppDS8 = new m_IDirectSound8(*ppDS8);
	}

	return hr;
}

HRESULT WINAPI ds_DirectSoundCaptureCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE8 *ppDSC8, LPUNKNOWN pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(DirectSoundCaptureCreate8Proc, DirectSoundCaptureCreate8, DirectSoundCaptureCreate8_out);

	if (!DirectSoundCaptureCreate8)
	{
		return DSERR_GENERIC;
	}

	HRESULT hr = DirectSoundCaptureCreate8(pcGuidDevice, ppDSC8, pUnkOuter);

	if (SUCCEEDED(hr) && ppDSC8)
	{
		*ppDSC8 = new m_IDirectSoundCapture8(*ppDSC8);
	}

	return hr;
}
