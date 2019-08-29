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

#include "dsound.h"

HRESULT m_IDirectSound8::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if ((riid == IID_IDirectSound || riid == IID_IDirectSound8 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return DS_OK;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectSound8::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirectSound8::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

// IDirectSound methods
HRESULT m_IDirectSound8::CreateSoundBuffer(LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER *ppDSBuffer, LPUNKNOWN pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pcDSBufferDesc)
	{
		DSBUFFERDESC dsdesc = *pcDSBufferDesc;

		if (Config.ForceSoftwareMixing)
		{
			dsdesc.dwFlags &= ~DSBCAPS_LOCHARDWARE;
			dsdesc.dwFlags |= DSBCAPS_LOCSOFTWARE;
		}

		if (Config.ForceHardwareMixing)
		{
			dsdesc.dwFlags &= ~DSBCAPS_LOCSOFTWARE;
			dsdesc.dwFlags |= DSBCAPS_LOCHARDWARE;
		}

		if ((Config.ForceHQ3DSoftMixing) && (dsdesc.dwFlags & DSBCAPS_LOCSOFTWARE) && (dsdesc.dwFlags & DSBCAPS_CTRL3D))
		{
			dsdesc.guid3DAlgorithm = DS3DALG_HRTF_FULL;
		}

		if ((Config.ForceNonStaticBuffers) && (dsdesc.dwFlags & DSBCAPS_STATIC))
		{
			dsdesc.dwFlags &= ~DSBCAPS_STATIC;
		}

		if ((Config.ForceVoiceManagement) && ((dsdesc.dwFlags & DSBCAPS_LOCDEFER) == 0))
		{
			dsdesc.dwFlags &= ~DSBCAPS_LOCSOFTWARE;
			dsdesc.dwFlags &= ~DSBCAPS_LOCHARDWARE;
			dsdesc.dwFlags |= DSBCAPS_LOCDEFER;
		}
	}

	HRESULT hr = ProxyInterface->CreateSoundBuffer(pcDSBufferDesc, ppDSBuffer, pUnkOuter);

	if (SUCCEEDED(hr) && ppDSBuffer)
	{
		*ppDSBuffer = ProxyAddressLookupTableDsound.FindAddress<m_IDirectSoundBuffer8>(*ppDSBuffer);

		if (pcDSBufferDesc && (pcDSBufferDesc->dwFlags & DSBCAPS_PRIMARYBUFFER) != 0)
		{
			((m_IDirectSoundBuffer8*)*ppDSBuffer)->SetPrimaryBuffer(true);

			if (Config.ForcePrimaryBufferFormat)
			{
				(*ppDSBuffer)->SetFormat(nullptr);
			}
		}
	}

	return hr;
}

HRESULT m_IDirectSound8::GetCaps(LPDSCAPS pDSCaps)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->GetCaps(pDSCaps);

	if (Config.Num2DBuffers && pDSCaps)
	{
		if (pDSCaps->dwMaxHwMixingAllBuffers >= Config.Num2DBuffers)
		{
			if ((Config.Num2DBuffers >= pDSCaps->dwMaxHwMixingAllBuffers - pDSCaps->dwFreeHwMixingAllBuffers) ||
				(Config.Num2DBuffers >= pDSCaps->dwMaxHwMixingStaticBuffers - pDSCaps->dwFreeHwMixingStaticBuffers) ||
				(Config.Num2DBuffers >= pDSCaps->dwFreeHwMixingStreamingBuffers - pDSCaps->dwMaxHwMixingStreamingBuffers))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect DSCaps data for 2D Buffers.");
			}

			pDSCaps->dwFreeHwMixingAllBuffers = Config.Num2DBuffers - (pDSCaps->dwMaxHwMixingAllBuffers - pDSCaps->dwFreeHwMixingAllBuffers);
			pDSCaps->dwFreeHwMixingStaticBuffers = Config.Num2DBuffers - (pDSCaps->dwMaxHwMixingStaticBuffers - pDSCaps->dwFreeHwMixingStaticBuffers);
			pDSCaps->dwFreeHwMixingStreamingBuffers = Config.Num2DBuffers - (pDSCaps->dwFreeHwMixingStreamingBuffers - pDSCaps->dwMaxHwMixingStreamingBuffers);
			pDSCaps->dwMaxHwMixingAllBuffers = Config.Num2DBuffers;
			pDSCaps->dwMaxHwMixingStaticBuffers = Config.Num2DBuffers;
			pDSCaps->dwMaxHwMixingStreamingBuffers = Config.Num2DBuffers;
		}
	}

	if (Config.Num3DBuffers && pDSCaps)
	{
		if (pDSCaps->dwMaxHw3DAllBuffers >= Config.Num3DBuffers)
		{
			if ((Config.Num3DBuffers >= pDSCaps->dwMaxHw3DAllBuffers - pDSCaps->dwFreeHw3DAllBuffers) ||
				(Config.Num3DBuffers >= pDSCaps->dwMaxHw3DStaticBuffers - pDSCaps->dwFreeHw3DStaticBuffers) ||
				(Config.Num3DBuffers >= pDSCaps->dwFreeHw3DStreamingBuffers - pDSCaps->dwMaxHw3DStreamingBuffers))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect DSCaps data for 3D Buffers.");
			}

			pDSCaps->dwFreeHw3DAllBuffers = Config.Num3DBuffers - (pDSCaps->dwMaxHw3DAllBuffers - pDSCaps->dwFreeHw3DAllBuffers);
			pDSCaps->dwFreeHw3DStaticBuffers = Config.Num3DBuffers - (pDSCaps->dwMaxHw3DStaticBuffers - pDSCaps->dwFreeHw3DStaticBuffers);
			pDSCaps->dwFreeHw3DStreamingBuffers = Config.Num3DBuffers - (pDSCaps->dwFreeHw3DStreamingBuffers - pDSCaps->dwMaxHw3DStreamingBuffers);
			pDSCaps->dwMaxHw3DAllBuffers = Config.Num3DBuffers;
			pDSCaps->dwMaxHw3DStaticBuffers = Config.Num3DBuffers;
			pDSCaps->dwMaxHw3DStreamingBuffers = Config.Num3DBuffers;
		}
	}

	return hr;
}

HRESULT m_IDirectSound8::DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER pDSBufferOriginal, LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pDSBufferOriginal)
	{
		pDSBufferOriginal = static_cast<m_IDirectSoundBuffer8 *>(pDSBufferOriginal)->GetProxyInterface();
	}

	HRESULT hr = ProxyInterface->DuplicateSoundBuffer(pDSBufferOriginal, ppDSBufferDuplicate);

	if (SUCCEEDED(hr) && ppDSBufferDuplicate)
	{
		*ppDSBufferDuplicate = ProxyAddressLookupTableDsound.FindAddress<m_IDirectSoundBuffer8>(*ppDSBufferDuplicate);
	}

	return hr;
}

HRESULT m_IDirectSound8::SetCooperativeLevel(HWND hwnd, DWORD dwLevel)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.ForceExclusiveMode && dwLevel == DSSCL_NORMAL)
	{
		dwLevel = DSSCL_EXCLUSIVE;
	}

	return ProxyInterface->SetCooperativeLevel(hwnd, dwLevel);
}

HRESULT m_IDirectSound8::Compact()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Compact();
}

HRESULT m_IDirectSound8::GetSpeakerConfig(LPDWORD pdwSpeakerConfig)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.ForceSpeakerConfig && pdwSpeakerConfig)
	{
		*pdwSpeakerConfig = Config.SpeakerConfig;

		return DS_OK;
	}

	return ProxyInterface->GetSpeakerConfig(pdwSpeakerConfig);
}

HRESULT m_IDirectSound8::SetSpeakerConfig(DWORD dwSpeakerConfig)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.PreventSpeakerSetup)
	{
		return DS_OK;
	}

	return ProxyInterface->SetSpeakerConfig(dwSpeakerConfig);
}

HRESULT m_IDirectSound8::Initialize(LPCGUID pcGuidDevice)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Initialize(pcGuidDevice);
}

// IDirectSound8 methods
HRESULT  m_IDirectSound8::VerifyCertification(LPDWORD pdwCertified)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->VerifyCertification(pdwCertified);

	if (Config.ForceCertification && pdwCertified)
	{
		*pdwCertified = DS_CERTIFIED;

		return DS_OK;
	}

	return hr;
}
