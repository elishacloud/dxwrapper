/**
* Copyright (c) 2003-2007, Arne Bockholdt, github@bockholdt.info
*
* This file is part of Direct Sound Control.
*
* Direct Sound Control is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Direct Sound Control is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with Direct Sound Control.  If not, see <http://www.gnu.org/licenses/>.
*
* Updated 2017 by Elisha Riedlinger
*/

#pragma once

#define IDIRECTSOUNDBUFFER8EX_CLASS_NAME	"IDirectSoundBuffer8Ex"

#ifdef _DEBUG
extern void LogMessage(const char* szClassName, void* pInstance, char* szMessage);
extern bool	g_bLogDirectSoundBuffer;
#endif // _DEBUG

extern bool	g_bForcePrimaryBufferFormat;
extern DWORD g_nPrimaryBufferBits;
extern DWORD g_nPrimaryBufferSamples;
extern DWORD g_nPrimaryBufferChannels;
extern bool g_bStoppedDriverWorkaround;

class IDirectSoundBuffer8Ex : public IDirectSoundBuffer8
{
public:
	IDirectSoundBuffer8Ex(void);
	virtual ~IDirectSoundBuffer8Ex(void);

	virtual HRESULT __stdcall QueryInterface(REFIID, LPVOID *);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	// IDirectSoundBuffer methods
	virtual HRESULT __stdcall GetCaps(LPDSBCAPS pDSBufferCaps);
	virtual HRESULT __stdcall GetCurrentPosition(LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor);
	virtual HRESULT __stdcall GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
	virtual HRESULT __stdcall GetVolume(LPLONG plVolume);
	virtual HRESULT __stdcall GetPan(LPLONG plPan);
	virtual HRESULT __stdcall GetFrequency(LPDWORD pdwFrequency);
	virtual HRESULT __stdcall GetStatus(LPDWORD pdwStatus);
	virtual HRESULT __stdcall Initialize(LPDIRECTSOUND pDirectSound, LPCDSBUFFERDESC pcDSBufferDesc);
	virtual HRESULT __stdcall Lock(DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags);
	virtual HRESULT __stdcall Play(DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags);
	virtual HRESULT __stdcall SetCurrentPosition(DWORD dwNewPosition);
	virtual HRESULT __stdcall SetFormat(LPCWAVEFORMATEX pcfxFormat);
	virtual HRESULT __stdcall SetVolume(LONG lVolume);
	virtual HRESULT __stdcall SetPan(LONG lPan);
	virtual HRESULT __stdcall SetFrequency(DWORD dwFrequency);
	virtual HRESULT __stdcall Stop();
	virtual HRESULT __stdcall Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2);
	virtual HRESULT __stdcall Restore();

	// IDirectSoundBuffer8 methods
	virtual HRESULT __stdcall SetFX(DWORD dwEffectsCount, LPDSEFFECTDESC pDSFXDesc, LPDWORD pdwResultCodes);
	virtual HRESULT __stdcall AcquireResources(DWORD dwFlags, DWORD dwEffectsCount, LPDWORD pdwResultCodes);
	virtual HRESULT __stdcall GetObjectInPath(REFGUID rguidObject, DWORD dwIndex, REFGUID rguidInterface, LPVOID *ppObject);

	bool GetPrimaryBuffer()
	{
		return m_bIsPrimary;
	};
	void SetPrimaryBuffer(bool bIsPrimary)
	{
		m_bIsPrimary = bIsPrimary;
	};

	LPDIRECTSOUNDBUFFER8 m_lpDirectSoundBuffer8;


protected:
#ifdef _DEBUG
	const char* m_cszClassName;
	char m_acLogBuffer[1024];
#endif // _DEBUG

	DWORD m_dwOldWriteCursorPos;
	BYTE m_nWriteCursorIdent;

	bool m_bIsPrimary;
};
