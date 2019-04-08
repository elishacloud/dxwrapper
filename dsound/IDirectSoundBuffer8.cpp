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

HRESULT m_IDirectSoundBuffer8::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if ((riid == IID_IDirectSoundBuffer || riid == IID_IDirectSoundBuffer8 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectSoundBuffer8::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectSoundBuffer8::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

HRESULT m_IDirectSoundBuffer8::GetCaps(LPDSBCAPS pDSBufferCaps)
{
	return ProxyInterface->GetCaps(pDSBufferCaps);
}

HRESULT m_IDirectSoundBuffer8::GetCurrentPosition(LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor)
{
	HRESULT hr = ProxyInterface->GetCurrentPosition(pdwCurrentPlayCursor, pdwCurrentWriteCursor);

	if (Config.StoppedDriverWorkaround && pdwCurrentWriteCursor)
	{
		DWORD dwStatus;

		ProxyInterface->GetStatus(&dwStatus);

		if (dwStatus & DSBSTATUS_PLAYING)
		{
			if (m_dwOldWriteCursorPos == *pdwCurrentWriteCursor)
			{
				if (++m_nWriteCursorIdent > 1)
				{
					ProxyInterface->Stop();
					ProxyInterface->Play(0, 0, dwStatus & DSBPLAY_LOOPING);
				}
			}
			else
			{
				m_nWriteCursorIdent = 0;
			}

			m_dwOldWriteCursorPos = *pdwCurrentWriteCursor;
		}
	}

	return hr;
}

HRESULT m_IDirectSoundBuffer8::GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten)
{
	return ProxyInterface->GetFormat(pwfxFormat, dwSizeAllocated, pdwSizeWritten);
}

HRESULT m_IDirectSoundBuffer8::GetVolume(LPLONG plVolume)
{
	return ProxyInterface->GetVolume(plVolume);
}

HRESULT m_IDirectSoundBuffer8::GetPan(LPLONG plPan)
{
	return ProxyInterface->GetPan(plPan);
}

HRESULT m_IDirectSoundBuffer8::GetFrequency(LPDWORD pdwFrequency)
{
	return ProxyInterface->GetFrequency(pdwFrequency);
}

HRESULT m_IDirectSoundBuffer8::GetStatus(LPDWORD pdwStatus)
{
	return ProxyInterface->GetStatus(pdwStatus);
}

HRESULT m_IDirectSoundBuffer8::Initialize(LPDIRECTSOUND pDirectSound, LPCDSBUFFERDESC pcDSBufferDesc)
{
	if (pDirectSound)
	{
		pDirectSound = static_cast<m_IDirectSound8 *>(pDirectSound)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(pDirectSound, pcDSBufferDesc);
}

HRESULT m_IDirectSoundBuffer8::Lock(DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
	return ProxyInterface->Lock(dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);
}

HRESULT m_IDirectSoundBuffer8::Play(DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags)
{

	return ProxyInterface->Play(dwReserved1, dwPriority, dwFlags);
}

HRESULT m_IDirectSoundBuffer8::SetCurrentPosition(DWORD dwNewPosition)
{
	return ProxyInterface->SetCurrentPosition(dwNewPosition);
}

HRESULT m_IDirectSoundBuffer8::SetFormat(LPCWAVEFORMATEX pcfxFormat)
{
	if (Config.ForcePrimaryBufferFormat && this->GetPrimaryBuffer())
	{
		WAVEFORMATEX fxFormat;
		fxFormat.wFormatTag = WAVE_FORMAT_PCM;
		fxFormat.cbSize = 0;
		fxFormat.nChannels = (WORD)Config.PrimaryBufferChannels;
		fxFormat.wBitsPerSample = (WORD)Config.PrimaryBufferBits;
		fxFormat.nSamplesPerSec = Config.PrimaryBufferSamples;
		fxFormat.nBlockAlign = (fxFormat.nChannels * fxFormat.wBitsPerSample) / 8;
		fxFormat.nAvgBytesPerSec = fxFormat.nBlockAlign * fxFormat.nSamplesPerSec;

		return ProxyInterface->SetFormat(&fxFormat);
	}

	return ProxyInterface->SetFormat(pcfxFormat);
}

HRESULT m_IDirectSoundBuffer8::SetVolume(LONG lVolume)
{
	return ProxyInterface->SetVolume(lVolume);
}

HRESULT m_IDirectSoundBuffer8::SetPan(LONG lPan)
{
	return ProxyInterface->SetPan(lPan);
}

HRESULT m_IDirectSoundBuffer8::SetFrequency(DWORD dwFrequency)
{
	return ProxyInterface->SetFrequency(dwFrequency);
}

HRESULT m_IDirectSoundBuffer8::Stop()
{
	return ProxyInterface->Stop();
}

HRESULT m_IDirectSoundBuffer8::Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
	return ProxyInterface->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
}

HRESULT m_IDirectSoundBuffer8::Restore()
{
	return ProxyInterface->Restore();
}

// IDirectSoundBuffer8 methods
HRESULT m_IDirectSoundBuffer8::SetFX(DWORD dwEffectsCount, LPDSEFFECTDESC pDSFXDesc, LPDWORD pdwResultCodes)
{
	return ProxyInterface->SetFX(dwEffectsCount, pDSFXDesc, pdwResultCodes);
}

HRESULT m_IDirectSoundBuffer8::AcquireResources(DWORD dwFlags, DWORD dwEffectsCount, LPDWORD pdwResultCodes)
{
	return ProxyInterface->AcquireResources(dwFlags, dwEffectsCount, pdwResultCodes);
}

HRESULT m_IDirectSoundBuffer8::GetObjectInPath(REFGUID rguidObject, DWORD dwIndex, REFGUID rguidInterface, LPVOID *ppObject)
{
	HRESULT hr = ProxyInterface->GetObjectInPath(rguidObject, dwIndex, rguidInterface, ppObject);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(rguidInterface, ppObject);
	}

	return hr;
}
