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

HRESULT m_IDirectSound3DListener8::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if ((riid == IID_IDirectSound3DListener || riid == IID_IDirectSound3DListener8 || riid == IID_IUnknown) && ppvObj)
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

ULONG m_IDirectSound3DListener8::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectSound3DListener8::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

// IDirectSound3DListener methods
HRESULT m_IDirectSound3DListener8::GetAllParameters(LPDS3DLISTENER pListener)
{
	return ProxyInterface->GetAllParameters(pListener);
}

HRESULT m_IDirectSound3DListener8::GetDistanceFactor(D3DVALUE* pflDistanceFactor)
{
	return ProxyInterface->GetDistanceFactor(pflDistanceFactor);
}

HRESULT m_IDirectSound3DListener8::GetDopplerFactor(D3DVALUE* pflDopplerFactor)
{
	return ProxyInterface->GetDopplerFactor(pflDopplerFactor);
}

HRESULT m_IDirectSound3DListener8::GetOrientation(D3DVECTOR* pvOrientFront, D3DVECTOR* pvOrientTop)
{
	return ProxyInterface->GetOrientation(pvOrientFront, pvOrientTop);
}

HRESULT m_IDirectSound3DListener8::GetPosition(D3DVECTOR* pvPosition)
{
	return ProxyInterface->GetPosition(pvPosition);
}

HRESULT m_IDirectSound3DListener8::GetRolloffFactor(D3DVALUE* pflRolloffFactor)
{
	return ProxyInterface->GetRolloffFactor(pflRolloffFactor);
}

HRESULT m_IDirectSound3DListener8::GetVelocity(D3DVECTOR* pvVelocity)
{
	return ProxyInterface->GetVelocity(pvVelocity);
}

HRESULT m_IDirectSound3DListener8::SetAllParameters(LPCDS3DLISTENER pcListener, DWORD dwApply)
{
	return ProxyInterface->SetAllParameters(pcListener, dwApply);
}

HRESULT m_IDirectSound3DListener8::SetDistanceFactor(D3DVALUE flDistanceFactor, DWORD dwApply)
{
	return ProxyInterface->SetDistanceFactor(flDistanceFactor, dwApply);
}

HRESULT m_IDirectSound3DListener8::SetDopplerFactor(D3DVALUE flDopplerFactor, DWORD dwApply)
{
	return ProxyInterface->SetDopplerFactor(flDopplerFactor, dwApply);
}

HRESULT m_IDirectSound3DListener8::SetOrientation(D3DVALUE xFront, D3DVALUE yFront, D3DVALUE zFront, D3DVALUE xTop, D3DVALUE yTop, D3DVALUE zTop, DWORD dwApply)
{
	return ProxyInterface->SetOrientation(xFront, yFront, zFront, xTop, yTop, zTop, dwApply);
}

HRESULT m_IDirectSound3DListener8::SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply)
{
	return ProxyInterface->SetPosition(x, y, z, dwApply);
}

HRESULT m_IDirectSound3DListener8::SetRolloffFactor(D3DVALUE flRolloffFactor, DWORD dwApply)
{
	return ProxyInterface->SetRolloffFactor(flRolloffFactor, dwApply);
}

HRESULT m_IDirectSound3DListener8::SetVelocity(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply)
{
	return ProxyInterface->SetVelocity(x, y, z, dwApply);
}

HRESULT m_IDirectSound3DListener8::CommitDeferredSettings()
{
	return ProxyInterface->CommitDeferredSettings();
}
