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
#include "idirectsound3dBufferex.h"

//////////////////////////////////////////////////////////////////////////////////////////////

IDirectSound3DBuffer8Ex::IDirectSound3DBuffer8Ex(void)
{
	m_lpDirectSound3DBuffer8	= NULL;

#ifdef ENABLE_LOG
	m_cszClassName = IDIRECTSOUND3DBUFFER8EX_CLASS_NAME;

	if( g_bLogDirectSound3DBuffer == true )
		::LogMessage( m_cszClassName, this, "Constructor called....");
#endif // ENABLE_LOG

}

//////////////////////////////////////////////////////////////////////////////////////////////

IDirectSound3DBuffer8Ex::~IDirectSound3DBuffer8Ex(void)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		::LogMessage( m_cszClassName, this, "Destructor called....");
#endif // ENABLE_LOG
}

//////////////////////////////////////////////////////////////////////////////////////////////


HRESULT IDirectSound3DBuffer8Ex::QueryInterface( REFIID refIID, LPVOID * pVoid )
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		::LogMessage( m_cszClassName, this, "QueryInterface called....");
#endif // ENABLE_LOG

	*pVoid = (LPVOID) NULL;
	HRESULT hRes;

	if(( refIID == IID_IDirectSound3DBuffer ) || ( refIID == IID_IDirectSound3DBuffer8 ))
	{
		LPVOID pTemp;
		hRes =  m_lpDirectSound3DBuffer8->QueryInterface( refIID, &pTemp );

		if( hRes == S_OK )
		{
			m_lpDirectSound3DBuffer8 = (LPDIRECTSOUND3DBUFFER8) pTemp;
			*pVoid = (LPVOID) this;
		
			//m_lpDirectSoundBuffer8->AddRef();
		}

		return hRes;
	}
	
	// Unknown interface, let DX handle this...
	hRes =  m_lpDirectSound3DBuffer8->QueryInterface( refIID, pVoid );

#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		::LogMessage( m_cszClassName, this, "QueryInterface for unknown interface IID called....");
#endif // ENABLE_LOG

	return hRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG IDirectSound3DBuffer8Ex::AddRef()
{
	ULONG nRefCnt = m_lpDirectSound3DBuffer8->AddRef();

#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
	{
		CString sLogString;
		sLogString.Format("AddRef called....,nRefCnt=%u", nRefCnt ); 

		::LogMessage( m_cszClassName, this, sLogString.GetBuffer());
	}
#endif // ENABLE_LOG

	return nRefCnt;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG IDirectSound3DBuffer8Ex::Release()
{
	ULONG nRefCnt = m_lpDirectSound3DBuffer8->Release();

#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
	{
		CString sLogString;
		sLogString.Format("Release called....,nRefCnt=%u", nRefCnt ); 

		::LogMessage( m_cszClassName, this, sLogString.GetBuffer());
	}
#endif // ENABLE_LOG

	if ( nRefCnt == 0 )
	{
		delete this;
		return 0;
	}

	return nRefCnt;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// IDirectSound3DBuffer methods

HRESULT IDirectSound3DBuffer8Ex::GetAllParameters(LPDS3DBUFFER pDs3dBuffer)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "GetAllParameter called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->GetAllParameters( pDs3dBuffer);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::GetConeAngles(LPDWORD pdwInsideConeAngle, LPDWORD pdwOutsideConeAngle)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "GetConeAngles called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->GetConeAngles( pdwInsideConeAngle,pdwOutsideConeAngle);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::GetConeOrientation(D3DVECTOR* pvOrientation)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "GetConeOrientation called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->GetConeOrientation( pvOrientation);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::GetConeOutsideVolume(LPLONG plConeOutsideVolume)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "GetConeOutsideVolume called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->GetConeOutsideVolume( plConeOutsideVolume );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::GetMaxDistance(D3DVALUE* pflMaxDistance)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "GetMaxDistance called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->GetMaxDistance( pflMaxDistance);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::GetMinDistance(D3DVALUE* pflMinDistance)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "GetMinDistance called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->GetMinDistance( pflMinDistance );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::GetMode(LPDWORD pdwMode)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "GetMode called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->GetMode( pdwMode );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::GetPosition(D3DVECTOR* pvPosition)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "GetPosition called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->GetPosition( pvPosition );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::GetVelocity(D3DVECTOR* pvVelocity)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "GetVelocity called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->GetVelocity( pvVelocity );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::SetAllParameters(LPCDS3DBUFFER pcDs3dBuffer, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "SetAllParameters called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->SetAllParameters( pcDs3dBuffer, dwApply );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::SetConeAngles(DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "SetConeAngles called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->SetConeAngles( dwInsideConeAngle, dwOutsideConeAngle, dwApply );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::SetConeOrientation(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "SetConeOrientation called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->SetConeOrientation( x, y, z, dwApply );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::SetConeOutsideVolume(LONG lConeOutsideVolume, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "SetConeOutsideVolume called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->SetConeOutsideVolume( lConeOutsideVolume, dwApply );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::SetMaxDistance(D3DVALUE flMaxDistance, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "SetMaxDistance called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->SetMaxDistance( flMaxDistance, dwApply );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::SetMinDistance(D3DVALUE flMinDistance, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "SetMinDistance called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->SetMinDistance( flMinDistance, dwApply );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::SetMode(DWORD dwMode, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "SetMode called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->SetMode( dwMode, dwApply);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "SetPosition called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->SetPosition( x, y, z, dwApply);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DBuffer8Ex::SetVelocity(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DBuffer == true )
		LogMessage( m_cszClassName, this, "SetVelocity called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DBuffer8->SetVelocity( x, y, z, dwApply);
}

//////////////////////////////////////////////////////////////////////////////////////////////
