//////////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2003-2007, Arne Bockholdt, github@bockholdt.info
//
// This file is part of Direct Sound Control.
//
// Direct Sound Control is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Direct Sound Control is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Direct Sound Control.  If not, see <http://www.gnu.org/licenses/>.
//
//////////////////////////////////////////////////////////////////////////////////////////////
//
// Updated by Elisha Riedlinger 2017
//
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

#define IDIRECTSOUND8EX_CLASS_NAME	"IDirectSound8Ex"

//////////////////////////////////////////////////////////////////////////////////////////////

extern void LogMessage(const char* szClassName, void* pInstance, char* szMessage );
extern bool	g_bLogDirectSound;

extern bool g_bForceExclusiveMode;
extern bool g_bForceCertification;
extern bool g_bForceSoftwareMixing;
extern bool g_bForceHardwareMixing;
extern bool g_bPreventSpeakerSetup;
extern DWORD g_nNum2DBuffers;
extern DWORD g_nNum3DBuffers;
extern bool	g_bForceHQSoftware3D;
extern bool	g_bForceNonStatic;
extern bool g_bForceVoiceManagement;
extern bool	g_bForcePrimaryBufferFormat;
extern DWORD g_nPrimaryBufferBits;
extern DWORD g_nPrimaryBufferSamples;
extern DWORD g_nPrimaryBufferChannels;
extern DWORD g_nSpeakerConfig;
extern bool g_bForceSpeakerConfig;

//////////////////////////////////////////////////////////////////////////////////////////////

class IDirectSound8Ex : public IDirectSound8
{
public:
	IDirectSound8Ex(void);
	virtual ~IDirectSound8Ex(void);

    virtual HRESULT __stdcall QueryInterface	   (REFIID, LPVOID *);
    virtual ULONG   __stdcall AddRef();
    virtual ULONG   __stdcall Release();
    virtual HRESULT __stdcall CreateSoundBuffer    (LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER *ppDSBuffer, LPUNKNOWN pUnkOuter) ;
    virtual HRESULT __stdcall GetCaps              (LPDSCAPS pDSCaps) ;
    virtual HRESULT __stdcall DuplicateSoundBuffer (LPDIRECTSOUNDBUFFER pDSBufferOriginal, LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate);
    virtual HRESULT __stdcall SetCooperativeLevel  (HWND hwnd, DWORD dwLevel);
    virtual HRESULT __stdcall Compact              ();
    virtual HRESULT __stdcall GetSpeakerConfig     (LPDWORD pdwSpeakerConfig);
    virtual HRESULT __stdcall SetSpeakerConfig     (DWORD dwSpeakerConfig);
    virtual HRESULT __stdcall Initialize           (LPCGUID pcGuidDevice) ;
    virtual HRESULT __stdcall VerifyCertification	(LPDWORD pdwCertified);

	LPDIRECTSOUND8	m_lpDirectSound8;

protected : 
	const char* m_cszClassName;
};

//////////////////////////////////////////////////////////////////////////////////////////////
