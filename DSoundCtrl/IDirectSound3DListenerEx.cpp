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

#include <atlstr.h>
#include "mmsystem.h"
#include "dsound.h"
#include "idirectsound3dListenerEx.h"

IDirectSound3DListener8Ex::IDirectSound3DListener8Ex(void)
{
	m_lpDirectSound3DListener8 = nullptr;

#ifdef _DEBUG
	m_cszClassName = IDIRECTSOUND3DLISTENER8EX_CLASS_NAME;

	if (g_bLogDirectSound3DListener)
	{
		::LogMessage(m_cszClassName, this, "Constructor called....");
	}
#endif // _DEBUG
}

IDirectSound3DListener8Ex::~IDirectSound3DListener8Ex(void)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		::LogMessage(m_cszClassName, this, "Destructor called....");
	}
#endif // _DEBUG
}

HRESULT IDirectSound3DListener8Ex::QueryInterface(REFIID refIID, LPVOID * pVoid)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		::LogMessage(m_cszClassName, this, "QueryInterface called....");
	}
#endif // _DEBUG

	*pVoid = (LPVOID) nullptr;
	HRESULT hRes;

	if ((refIID == IID_IDirectSound3DListener) || (refIID == IID_IDirectSound3DListener8))
	{
		LPVOID pTemp;
		hRes = m_lpDirectSound3DListener8->QueryInterface(refIID, &pTemp);

		if (hRes == S_OK)
		{
			m_lpDirectSound3DListener8 = (LPDIRECTSOUND3DLISTENER8)pTemp;
			*pVoid = (LPVOID) this;

			//m_lpDirectSoundListener8->AddRef();
		}

		return hRes;
	}

	// Unknown interface, let DX handle this...
	hRes = m_lpDirectSound3DListener8->QueryInterface(refIID, pVoid);

#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		::LogMessage(m_cszClassName, this, "QueryInterface for unknown interface IID called....");
	}
#endif // _DEBUG

	return hRes;
}

ULONG IDirectSound3DListener8Ex::AddRef()
{
	ULONG nRefCnt = m_lpDirectSound3DListener8->AddRef();

#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		CString sLogString;
		sLogString.Format("AddRef called....,nRefCnt=%u", nRefCnt);

		::LogMessage(m_cszClassName, this, sLogString.GetBuffer());
	}
#endif // _DEBUG

	return nRefCnt;
}

ULONG IDirectSound3DListener8Ex::Release()
{
	ULONG nRefCnt = m_lpDirectSound3DListener8->Release();

#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		CString sLogString;
		sLogString.Format("Release called....,nRefCnt=%u", nRefCnt);

		::LogMessage(m_cszClassName, this, sLogString.GetBuffer());
	}
#endif // _DEBUG

	if (nRefCnt == 0)
	{
		delete this;
		return 0;
	}

	return nRefCnt;
}

// IDirectSound3DListener methods
HRESULT IDirectSound3DListener8Ex::GetAllParameters(LPDS3DLISTENER pListener)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "GetAllParameter called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->GetAllParameters(pListener);
}

HRESULT IDirectSound3DListener8Ex::GetDistanceFactor(D3DVALUE* pflDistanceFactor)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "GetDistanceFactor called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->GetDistanceFactor(pflDistanceFactor);
}

HRESULT IDirectSound3DListener8Ex::GetDopplerFactor(D3DVALUE* pflDopplerFactor)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "GetDopplerFactor called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->GetDopplerFactor(pflDopplerFactor);
}

HRESULT IDirectSound3DListener8Ex::GetOrientation(D3DVECTOR* pvOrientFront, D3DVECTOR* pvOrientTop)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "GetOrientation called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->GetOrientation(pvOrientFront, pvOrientTop);
}

HRESULT IDirectSound3DListener8Ex::GetPosition(D3DVECTOR* pvPosition)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "GetPosition called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->GetPosition(pvPosition);
}

HRESULT IDirectSound3DListener8Ex::GetRolloffFactor(D3DVALUE* pflRolloffFactor)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "GetRolloffFactor called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->GetRolloffFactor(pflRolloffFactor);
}

HRESULT IDirectSound3DListener8Ex::GetVelocity(D3DVECTOR* pvVelocity)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "GetVelocity called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->GetVelocity(pvVelocity);
}

HRESULT IDirectSound3DListener8Ex::SetAllParameters(LPCDS3DLISTENER pcListener, DWORD dwApply)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "SetAllParameters called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->SetAllParameters(pcListener, dwApply);
}

HRESULT IDirectSound3DListener8Ex::SetDistanceFactor(D3DVALUE flDistanceFactor, DWORD dwApply)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "SetDistanceFactor called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->SetDistanceFactor(flDistanceFactor, dwApply);
}

HRESULT IDirectSound3DListener8Ex::SetDopplerFactor(D3DVALUE flDopplerFactor, DWORD dwApply)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "SetDopplerFactor called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->SetDopplerFactor(flDopplerFactor, dwApply);
}

HRESULT IDirectSound3DListener8Ex::SetOrientation(D3DVALUE xFront, D3DVALUE yFront, D3DVALUE zFront, D3DVALUE xTop, D3DVALUE yTop, D3DVALUE zTop, DWORD dwApply)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "SetOrientation called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->SetOrientation(xFront, yFront, zFront, xTop, yTop, zTop, dwApply);
}

HRESULT IDirectSound3DListener8Ex::SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "SetPosition called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->SetPosition(x, y, z, dwApply);
}

HRESULT IDirectSound3DListener8Ex::SetRolloffFactor(D3DVALUE flRolloffFactor, DWORD dwApply)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "SetRolloffFactor called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->SetRolloffFactor(flRolloffFactor, dwApply);
}

HRESULT IDirectSound3DListener8Ex::SetVelocity(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply)
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "SetVelocity called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->SetVelocity(x, y, z, dwApply);
}

HRESULT IDirectSound3DListener8Ex::CommitDeferredSettings()
{
#ifdef _DEBUG
	if (g_bLogDirectSound3DListener)
	{
		LogMessage(m_cszClassName, this, "CommitDeferredSettings called....");
	}
#endif // _DEBUG

	return m_lpDirectSound3DListener8->CommitDeferredSettings();
}
