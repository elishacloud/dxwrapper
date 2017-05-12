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
#include "idirectsound3dListenerEx.h"

//////////////////////////////////////////////////////////////////////////////////////////////

IDirectSound3DListener8Ex::IDirectSound3DListener8Ex(void)
{
	m_lpDirectSound3DListener8	= NULL;

#ifdef ENABLE_LOG
	m_cszClassName = IDIRECTSOUND3DLISTENER8EX_CLASS_NAME;

	if( g_bLogDirectSound3DListener == true )
		::LogMessage( m_cszClassName, this, "Constructor called....");
#endif // ENABLE_LOG

}

//////////////////////////////////////////////////////////////////////////////////////////////

IDirectSound3DListener8Ex::~IDirectSound3DListener8Ex(void)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		::LogMessage( m_cszClassName, this, "Destructor called....");
#endif // ENABLE_LOG
}

//////////////////////////////////////////////////////////////////////////////////////////////


HRESULT IDirectSound3DListener8Ex::QueryInterface( REFIID refIID, LPVOID * pVoid )
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		::LogMessage( m_cszClassName, this, "QueryInterface called....");
#endif // ENABLE_LOG

	*pVoid = (LPVOID) NULL;
	HRESULT hRes;

	if(( refIID == IID_IDirectSound3DListener ) || ( refIID == IID_IDirectSound3DListener8 ))
	{
		LPVOID pTemp;
		hRes =  m_lpDirectSound3DListener8->QueryInterface( refIID, &pTemp );

		if( hRes == S_OK )
		{
			m_lpDirectSound3DListener8 = (LPDIRECTSOUND3DLISTENER8) pTemp;
			*pVoid = (LPVOID) this;
		
			//m_lpDirectSoundListener8->AddRef();
		}

		return hRes;
	}
	
	// Unknown interface, let DX handle this...
	hRes =  m_lpDirectSound3DListener8->QueryInterface( refIID, pVoid );

#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		::LogMessage( m_cszClassName, this, "QueryInterface for unknown interface IID called....");
#endif // ENABLE_LOG

	return hRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG IDirectSound3DListener8Ex::AddRef()
{
	ULONG nRefCnt = m_lpDirectSound3DListener8->AddRef();

#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
	{
		CString sLogString;
		sLogString.Format("AddRef called....,nRefCnt=%u", nRefCnt ); 

		::LogMessage( m_cszClassName, this, sLogString.GetBuffer());
	}
#endif // ENABLE_LOG

	return nRefCnt; 
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG IDirectSound3DListener8Ex::Release()
{
	ULONG nRefCnt = m_lpDirectSound3DListener8->Release();

#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
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
// IDirectSound3DListener methods

// IDirectSound3DListener methods
HRESULT IDirectSound3DListener8Ex::GetAllParameters( LPDS3DLISTENER pListener )
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "GetAllParameter called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->GetAllParameters( pListener );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DListener8Ex::GetDistanceFactor( D3DVALUE* pflDistanceFactor )
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "GetDistanceFactor called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->GetDistanceFactor( pflDistanceFactor );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DListener8Ex::GetDopplerFactor( D3DVALUE* pflDopplerFactor )
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "GetDopplerFactor called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->GetDopplerFactor( pflDopplerFactor );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DListener8Ex::GetOrientation( D3DVECTOR* pvOrientFront, D3DVECTOR* pvOrientTop)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "GetOrientation called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->GetOrientation( pvOrientFront, pvOrientTop );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DListener8Ex::GetPosition( D3DVECTOR* pvPosition)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "GetPosition called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->GetPosition( pvPosition );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DListener8Ex::GetRolloffFactor( D3DVALUE* pflRolloffFactor)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "GetRolloffFactor called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->GetRolloffFactor( pflRolloffFactor );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DListener8Ex::GetVelocity( D3DVECTOR* pvVelocity)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "GetVelocity called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->GetVelocity( pvVelocity );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DListener8Ex::SetAllParameters( LPCDS3DLISTENER pcListener, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "SetAllParameters called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->SetAllParameters( pcListener, dwApply );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DListener8Ex::SetDistanceFactor( D3DVALUE flDistanceFactor, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "SetDistanceFactor called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->SetDistanceFactor( flDistanceFactor, dwApply );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DListener8Ex::SetDopplerFactor( D3DVALUE flDopplerFactor, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "SetDopplerFactor called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->SetDopplerFactor( flDopplerFactor, dwApply );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DListener8Ex::SetOrientation( D3DVALUE xFront, D3DVALUE yFront, D3DVALUE zFront, D3DVALUE xTop, D3DVALUE yTop, D3DVALUE zTop, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "SetOrientation called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->SetOrientation( xFront, yFront, zFront, xTop, yTop, zTop, dwApply );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DListener8Ex::SetPosition( D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "SetPosition called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->SetPosition( x, y, z, dwApply );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DListener8Ex::SetRolloffFactor( D3DVALUE flRolloffFactor, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "SetRolloffFactor called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->SetRolloffFactor( flRolloffFactor, dwApply );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DListener8Ex::SetVelocity( D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply)
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "SetVelocity called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->SetVelocity( x, y, z, dwApply );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT IDirectSound3DListener8Ex::CommitDeferredSettings()
{
#ifdef ENABLE_LOG
	if( g_bLogDirectSound3DListener == true )
		LogMessage( m_cszClassName, this, "CommitDeferredSettings called....");
#endif // ENABLE_LOG

	return m_lpDirectSound3DListener8->CommitDeferredSettings();
}

//////////////////////////////////////////////////////////////////////////////////////////////
