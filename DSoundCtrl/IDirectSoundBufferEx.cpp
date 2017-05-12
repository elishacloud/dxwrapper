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

#include <atlstr.h>
#include "mmsystem.h"
#include "dsound.h"
#include "idirectsoundBufferex.h"
#include "idirectsound3dBufferex.h"
#include "idirectsound3dlistenerex.h"
#include "idirectsoundex.h"

//////////////////////////////////////////////////////////////////////////////////////////////

IDirectSoundBuffer8Ex::IDirectSoundBuffer8Ex(void)
{
	m_lpDirectSoundBuffer8	= NULL;
	m_bIsPrimary			= false;
	m_dwOldWriteCursorPos	= 0;
	m_nWriteCursorIdent		= 0;

#ifdef _DEBUG
	m_cszClassName = IDIRECTSOUNDBUFFER8EX_CLASS_NAME;

	if( g_bLogDirectSoundBuffer == true )
		::LogMessage( m_cszClassName, this, "Constructor called....");
#endif // _DEBUG

}

//////////////////////////////////////////////////////////////////////////////////////////////

IDirectSoundBuffer8Ex::~IDirectSoundBuffer8Ex(void)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		::LogMessage( m_cszClassName, this, "Destructor called....");
#endif // _DEBUG
}

//////////////////////////////////////////////////////////////////////////////////////////////


HRESULT IDirectSoundBuffer8Ex::QueryInterface( REFIID refIID, LPVOID * pVoid )
{
	*pVoid = (LPVOID) NULL;
	HRESULT hRes;

	if(( refIID == IID_IDirectSoundBuffer ) || ( refIID == IID_IDirectSoundBuffer8 ))
	{
		LPVOID pTemp;
		hRes =  m_lpDirectSoundBuffer8->QueryInterface( refIID, &pTemp );

		if( hRes == S_OK )
		{
			m_lpDirectSoundBuffer8 = (LPDIRECTSOUNDBUFFER8) pTemp;
			*pVoid = (LPVOID) this;
		
			//m_lpDirectSoundBuffer8->AddRef();
		}

#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		::LogMessage( m_cszClassName, this, "QueryInterface called,Interface=IID_IDirectSoundBuffer8");
#endif // _DEBUG

		return hRes;
	}

	if(( refIID == IID_IDirectSound3DBuffer ) || ( refIID == IID_IDirectSound3DBuffer8 ))
	{
		IDirectSound3DBuffer8Ex* pDS3DBX = new IDirectSound3DBuffer8Ex;

		hRes = m_lpDirectSoundBuffer8->QueryInterface( refIID, (LPVOID*) &(pDS3DBX->m_lpDirectSound3DBuffer8) );

		if( hRes == S_OK )
			*pVoid = (LPVOID) pDS3DBX;

#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		::LogMessage( m_cszClassName, this, "QueryInterface called,Interface=IID_IDirectSound3DBuffer8");
#endif // _DEBUG

		return hRes;
	}

	if(( refIID == IID_IDirectSound3DListener8 ) || ( refIID == IID_IDirectSound3DListener8 ))
	{
		IDirectSound3DListener8Ex* pDS3DLX = new IDirectSound3DListener8Ex;

		hRes = m_lpDirectSoundBuffer8->QueryInterface( refIID, (LPVOID*) &(pDS3DLX->m_lpDirectSound3DListener8) );

		if( hRes == S_OK )
			*pVoid = (LPVOID) pDS3DLX;

#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		::LogMessage( m_cszClassName, this, "QueryInterface called,Interface=IID_IDirectSound3DListener8");
#endif // _DEBUG

		return hRes;
	}
	
	// Unknown interface, let DX handle this...
	hRes =  m_lpDirectSoundBuffer8->QueryInterface( refIID, pVoid );

#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		::LogMessage( m_cszClassName, this, "QueryInterface for unknown interface IID called....");
#endif // _DEBUG

	return hRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG IDirectSoundBuffer8Ex::AddRef()
{
	ULONG nRefCnt = m_lpDirectSoundBuffer8->AddRef();

#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
	{
		CString sLogString;
		sLogString.Format("AddRef called....,nRefCnt=%u", nRefCnt ); 

		::LogMessage( m_cszClassName, this, sLogString.GetBuffer());
	}
#endif // _DEBUG

	return nRefCnt;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG IDirectSoundBuffer8Ex::Release()
{
	ULONG nRefCnt = m_lpDirectSoundBuffer8->Release();

#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
	{
		CString sLogString;
		sLogString.Format("Release called....,nRefCnt=%u", nRefCnt ); 

		::LogMessage( m_cszClassName, this, sLogString.GetBuffer());
	}
#endif // _DEBUG

	if ( nRefCnt == 0 )
	{
		delete this;
		return 0;
	}

	return nRefCnt;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// IDirectSoundBuffer methods

HRESULT IDirectSoundBuffer8Ex::GetCaps				(LPDSBCAPS pDSBufferCaps)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "GetCaps called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->GetCaps( pDSBufferCaps );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::GetCurrentPosition	(LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor)
{
	HRESULT hRes = m_lpDirectSoundBuffer8->GetCurrentPosition( pdwCurrentPlayCursor,pdwCurrentWriteCursor );

#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
	{
		sprintf_s( m_acLogBuffer, "GetCurrentPosition called,HRES=0x%x,PlayCursor=%u,WriteCursor=%u", hRes,(pdwCurrentPlayCursor==NULL?0:*pdwCurrentPlayCursor), (pdwCurrentWriteCursor==NULL?0:*pdwCurrentWriteCursor) );
		LogMessage( m_cszClassName, this, m_acLogBuffer );
	}
#endif // _DEBUG

	if( g_bStoppedDriverWorkaround == true )
	{
		if( pdwCurrentWriteCursor != NULL )
		{
			DWORD dwStatus;

			m_lpDirectSoundBuffer8->GetStatus( &dwStatus );

			if( dwStatus & DSBSTATUS_PLAYING )
			{
				if( m_dwOldWriteCursorPos == *pdwCurrentWriteCursor )
				{
					if( ++m_nWriteCursorIdent > 1 )
					{
#ifdef _DEBUG
						LogMessage( m_cszClassName, this, "Driver has stopped playing...enabling workaround" );
#endif // _DEBUG
						m_lpDirectSoundBuffer8->Stop();
						m_lpDirectSoundBuffer8->Play(0, 0, dwStatus & DSBPLAY_LOOPING );
					}
				}
				else
					m_nWriteCursorIdent = 0;

				m_dwOldWriteCursorPos = *pdwCurrentWriteCursor;
			}
		}
	}

	return hRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::GetFormat (LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "GetFormat called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->GetFormat( pwfxFormat, dwSizeAllocated, pdwSizeWritten );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::GetVolume				(LPLONG plVolume)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "GetVolume called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->GetVolume( plVolume );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::GetPan				(LPLONG plPan)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "GetPan called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->GetPan( plPan );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::GetFrequency	( LPDWORD pdwFrequency )
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "GetFrequency called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->GetFrequency	( pdwFrequency );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::GetStatus				(LPDWORD pdwStatus)
{
	HRESULT hRes = m_lpDirectSoundBuffer8->GetStatus	( pdwStatus );

#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
	{
		sprintf_s( m_acLogBuffer, "GetStatus called,HRES=0x%x,Status=%u", hRes, (pdwStatus==NULL?0:*pdwStatus) );
		LogMessage( m_cszClassName, this, m_acLogBuffer );
	}
#endif // _DEBUG


	return hRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::Initialize(LPDIRECTSOUND pDirectSound, LPCDSBUFFERDESC pcDSBufferDesc)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "Initialize called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->Initialize	( ((IDirectSound8Ex*)pDirectSound)->m_lpDirectSound8, pcDSBufferDesc );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::Lock	(DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1,LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
	HRESULT hRes = m_lpDirectSoundBuffer8->Lock	( dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1,ppvAudioPtr2, pdwAudioBytes2, dwFlags);
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
	{
		sprintf_s( m_acLogBuffer, "Lock,HRES=0x%x,Offset=%u,Bytes=%u,Ptr1=0x%x,BytesPtr1=0x%x,Ptr2=0x%x,BytesPtr2=0x%x,Flags=0x%x",hRes,dwOffset,dwBytes,(DWORD) ppvAudioPtr1, (DWORD) pdwAudioBytes1, (DWORD) ppvAudioPtr2, (DWORD) pdwAudioBytes2, dwFlags );
		LogMessage( m_cszClassName, this, m_acLogBuffer );
	}
#endif // _DEBUG

	return hRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::Play					(DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "Play called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->Play	( dwReserved1, dwPriority, dwFlags );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::SetCurrentPosition	(DWORD dwNewPosition)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "SetCurrentPosition called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->SetCurrentPosition	( dwNewPosition );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::SetFormat(LPCWAVEFORMATEX pcfxFormat)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "SetFormat called....");
#endif // _DEBUG

	if(( g_bForcePrimaryBufferFormat == true ) && this->GetPrimaryBuffer() )
	{
#ifdef _DEBUG
		if( g_bLogDirectSoundBuffer == true )
		{
			CString sMessage;
			sMessage.Format("FORCING PRIMARY BUFFER FORMAT,Format=%ub %uhz %uch", g_nPrimaryBufferBits,g_nPrimaryBufferSamples,g_nPrimaryBufferChannels );

			::LogMessage(m_cszClassName, this, sMessage.GetBuffer() );
		}
#endif // _DEBUG

		WAVEFORMATEX fxFormat;
		fxFormat.wFormatTag		=	WAVE_FORMAT_PCM;
		fxFormat.cbSize			=	0;
		fxFormat.nChannels		=	(WORD) g_nPrimaryBufferChannels;
		fxFormat.wBitsPerSample	=	(WORD) g_nPrimaryBufferBits;
		fxFormat.nSamplesPerSec	=	g_nPrimaryBufferSamples;
		fxFormat.nBlockAlign	=	(fxFormat.nChannels * fxFormat.wBitsPerSample) / 8;
		fxFormat.nAvgBytesPerSec=	fxFormat.nBlockAlign * fxFormat.nSamplesPerSec;
		
		return m_lpDirectSoundBuffer8->SetFormat	( &fxFormat );
	}

	return m_lpDirectSoundBuffer8->SetFormat	( pcfxFormat );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::SetVolume				(LONG lVolume)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "SetVolume called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->SetVolume( lVolume );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::SetPan				(LONG lPan)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "SetPan called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->SetPan( lPan );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::SetFrequency	(DWORD dwFrequency)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "SetFrequency called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->SetFrequency	( dwFrequency );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::Stop					()
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "Stop called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->Stop	();
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::Unlock				(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
	HRESULT hRes = m_lpDirectSoundBuffer8->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2 );

#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
	{
		sprintf_s( m_acLogBuffer, "Unlock,HRES=0x%x,Ptr1=0x%x,Bytes1=%u,Ptr2=0x%x,Bytes2=%u,", hRes, (DWORD) pvAudioPtr1, dwAudioBytes1, (DWORD) pvAudioPtr2, dwAudioBytes2 );
		LogMessage( m_cszClassName, this, m_acLogBuffer );
	}

#endif // _DEBUG

	return hRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::Restore()
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "Restore called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->Restore();
}

//////////////////////////////////////////////////////////////////////////////////////////////

	// IDirectSoundBuffer8 methods
HRESULT IDirectSoundBuffer8Ex::SetFX(DWORD dwEffectsCount, LPDSEFFECTDESC pDSFXDesc, LPDWORD pdwResultCodes)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "SetFX called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->SetFX(dwEffectsCount, pDSFXDesc, pdwResultCodes);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::AcquireResources	(DWORD dwFlags, DWORD dwEffectsCount, LPDWORD pdwResultCodes)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "AcquireResources called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->AcquireResources( dwFlags, dwEffectsCount, pdwResultCodes);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSoundBuffer8Ex::GetObjectInPath(REFGUID rguidObject, DWORD dwIndex, REFGUID rguidInterface, LPVOID *ppObject)
{
#ifdef _DEBUG
	if( g_bLogDirectSoundBuffer == true )
		LogMessage( m_cszClassName, this, "GetObjectInPath called....");
#endif // _DEBUG

	return m_lpDirectSoundBuffer8->GetObjectInPath(rguidObject, dwIndex, rguidInterface, ppObject);
}
