/**
* Copyright (C) 2020 Elisha Riedlinger
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

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	FARPROC procName ## _out = nullptr;

namespace DsoundWrapper
{
	VISIT_PROCS_DSOUND(INITIALIZE_WRAPPED_PROC);
	VISIT_PROCS_DSOUND_SHARED(INITIALIZE_WRAPPED_PROC);
}

using namespace DsoundWrapper;

HRESULT WINAPI ds_DirectSoundCreate(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	static DirectSoundCreateProc m_pDirectSoundCreate = (Wrapper::ValidProcAddress(DirectSoundCreate_out)) ? (DirectSoundCreateProc)DirectSoundCreate_out : nullptr;

	if (!m_pDirectSoundCreate)
	{
		return DSERR_GENERIC;
	}

	LOG_LIMIT(3, "Redirecting 'DirectSoundCreate' ...");

	HRESULT hr = m_pDirectSoundCreate(pcGuidDevice, ppDS, pUnkOuter);

	if (SUCCEEDED(hr) && ppDS)
	{
		*ppDS = ProxyAddressLookupTableDsound.FindAddress<m_IDirectSound8>(*ppDS);
	}

	return hr;
}

HRESULT WINAPI ds_DirectSoundEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
{
	LOG_LIMIT(1, __FUNCTION__);

	static DirectSoundEnumerateAProc m_pDirectSoundEnumerateA = (Wrapper::ValidProcAddress(DirectSoundEnumerateA_out)) ? (DirectSoundEnumerateAProc)DirectSoundEnumerateA_out : nullptr;

	if (!m_pDirectSoundEnumerateA)
	{
		return DSERR_GENERIC;
	}

	return m_pDirectSoundEnumerateA(pDSEnumCallback, pContext);
}

HRESULT WINAPI ds_DirectSoundEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext)
{
	LOG_LIMIT(1, __FUNCTION__);

	static DirectSoundEnumerateWProc m_pDirectSoundEnumerateW = (Wrapper::ValidProcAddress(DirectSoundEnumerateW_out)) ? (DirectSoundEnumerateWProc)DirectSoundEnumerateW_out : nullptr;

	if (!m_pDirectSoundEnumerateW)
	{
		return DSERR_GENERIC;
	}

	return m_pDirectSoundEnumerateW(pDSEnumCallback, pContext);
}

HRESULT WINAPI ds_DllCanUnloadNow()
{
	LOG_LIMIT(1, __FUNCTION__);

	static DllCanUnloadNowProc m_pDllCanUnloadNow = (Wrapper::ValidProcAddress(DllCanUnloadNow_out)) ? (DllCanUnloadNowProc)DllCanUnloadNow_out : nullptr;

	if (!m_pDllCanUnloadNow)
	{
		return DSERR_GENERIC;
	}

	return m_pDllCanUnloadNow();
}

HRESULT WINAPI ds_DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	LOG_LIMIT(1, __FUNCTION__);

	static DllGetClassObjectProc m_pDllGetClassObject = (Wrapper::ValidProcAddress(DllGetClassObject_out)) ? (DllGetClassObjectProc)DllGetClassObject_out : nullptr;

	if (!m_pDllGetClassObject)
	{
		return DSERR_GENERIC;
	}

	HRESULT hr = m_pDllGetClassObject(rclsid, riid, ppv);

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

	static DirectSoundCaptureCreateProc m_pDirectSoundCaptureCreate = (Wrapper::ValidProcAddress(DirectSoundCaptureCreate_out)) ? (DirectSoundCaptureCreateProc)DirectSoundCaptureCreate_out : nullptr;

	if (!m_pDirectSoundCaptureCreate)
	{
		return DSERR_GENERIC;
	}

	HRESULT hr = m_pDirectSoundCaptureCreate(pcGuidDevice, ppDSC, pUnkOuter);

	if (SUCCEEDED(hr) && ppDSC)
	{
		*ppDSC = ProxyAddressLookupTableDsound.FindAddress<m_IDirectSoundCapture8>(*ppDSC);
	}

	return hr;
}

HRESULT WINAPI ds_DirectSoundCaptureEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
{
	LOG_LIMIT(1, __FUNCTION__);

	static DirectSoundCaptureEnumerateAProc m_pDirectSoundCaptureEnumerateA = (Wrapper::ValidProcAddress(DirectSoundCaptureEnumerateA_out)) ? (DirectSoundCaptureEnumerateAProc)DirectSoundCaptureEnumerateA_out : nullptr;

	if (!m_pDirectSoundCaptureEnumerateA)
	{
		return DSERR_GENERIC;
	}

	return m_pDirectSoundCaptureEnumerateA(pDSEnumCallback, pContext);
}

HRESULT WINAPI ds_DirectSoundCaptureEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext)
{
	LOG_LIMIT(1, __FUNCTION__);

	static DirectSoundCaptureEnumerateWProc m_pDirectSoundCaptureEnumerateW = (Wrapper::ValidProcAddress(DirectSoundCaptureEnumerateW_out)) ? (DirectSoundCaptureEnumerateWProc)DirectSoundCaptureEnumerateW_out : nullptr;

	if (!m_pDirectSoundCaptureEnumerateW)
	{
		return DSERR_GENERIC;
	}

	return m_pDirectSoundCaptureEnumerateW(pDSEnumCallback, pContext);
}

HRESULT WINAPI ds_GetDeviceID(LPCGUID pGuidSrc, LPGUID pGuidDest)
{
	LOG_LIMIT(1, __FUNCTION__);

	static GetDeviceIDProc m_pGetDeviceID = (Wrapper::ValidProcAddress(GetDeviceID_out)) ? (GetDeviceIDProc)GetDeviceID_out : nullptr;

	if (!m_pGetDeviceID)
	{
		return DSERR_GENERIC;
	}

	return m_pGetDeviceID(pGuidSrc, pGuidDest);
}

HRESULT WINAPI ds_DirectSoundFullDuplexCreate(LPCGUID pcGuidCaptureDevice, LPCGUID pcGuidRenderDevice, LPCDSCBUFFERDESC pcDSCBufferDesc, LPCDSBUFFERDESC pcDSBufferDesc, HWND hWnd,
	DWORD dwLevel, LPDIRECTSOUNDFULLDUPLEX* ppDSFD, LPDIRECTSOUNDCAPTUREBUFFER8 *ppDSCBuffer8, LPDIRECTSOUNDBUFFER8 *ppDSBuffer8, LPUNKNOWN pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	static DirectSoundFullDuplexCreateProc m_pDirectSoundFullDuplexCreate = (Wrapper::ValidProcAddress(DirectSoundFullDuplexCreate_out)) ? (DirectSoundFullDuplexCreateProc)DirectSoundFullDuplexCreate_out : nullptr;

	if (!m_pDirectSoundFullDuplexCreate)
	{
		return DSERR_GENERIC;
	}

	HRESULT hr = m_pDirectSoundFullDuplexCreate(pcGuidCaptureDevice, pcGuidRenderDevice, pcDSCBufferDesc, pcDSBufferDesc, hWnd, dwLevel, ppDSFD, ppDSCBuffer8, ppDSBuffer8, pUnkOuter);

	if (SUCCEEDED(hr))
	{
		if (ppDSFD)
		{
			*ppDSFD = ProxyAddressLookupTableDsound.FindAddress<m_IDirectSoundFullDuplex8>(*ppDSFD);
		}
		if (ppDSCBuffer8)
		{
			*ppDSCBuffer8 = ProxyAddressLookupTableDsound.FindAddress<m_IDirectSoundCaptureBuffer8>(*ppDSCBuffer8);
		}
		if (ppDSBuffer8)
		{
			*ppDSBuffer8 = ProxyAddressLookupTableDsound.FindAddress<m_IDirectSoundBuffer8>(*ppDSBuffer8);
		}
	}

	return hr;
}

HRESULT WINAPI ds_DirectSoundCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	static DirectSoundCreate8Proc m_pDirectSoundCreate8 = (Wrapper::ValidProcAddress(DirectSoundCreate8_out)) ? (DirectSoundCreate8Proc)DirectSoundCreate8_out : nullptr;

	if (!m_pDirectSoundCreate8)
	{
		return DSERR_GENERIC;
	}

	LOG_LIMIT(3, "Redirecting 'DirectSoundCreate8' ...");

	HRESULT hr = m_pDirectSoundCreate8(pcGuidDevice, ppDS8, pUnkOuter);

	if (SUCCEEDED(hr) && ppDS8)
	{
		*ppDS8 = ProxyAddressLookupTableDsound.FindAddress<m_IDirectSound8>(*ppDS8);
	}

	return hr;
}

HRESULT WINAPI ds_DirectSoundCaptureCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE8 *ppDSC8, LPUNKNOWN pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	static DirectSoundCaptureCreate8Proc m_pDirectSoundCaptureCreate8 = (Wrapper::ValidProcAddress(DirectSoundCaptureCreate8_out)) ? (DirectSoundCaptureCreate8Proc)DirectSoundCaptureCreate8_out : nullptr;

	if (!m_pDirectSoundCaptureCreate8)
	{
		return DSERR_GENERIC;
	}

	HRESULT hr = m_pDirectSoundCaptureCreate8(pcGuidDevice, ppDSC8, pUnkOuter);

	if (SUCCEEDED(hr) && ppDSC8)
	{
		*ppDSC8 = ProxyAddressLookupTableDsound.FindAddress<m_IDirectSoundCapture8>(*ppDSC8);
	}

	return hr;
}
