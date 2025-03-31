/**
* Copyright (C) 2025 Elisha Riedlinger
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

HRESULT m_IDirectSound3DBuffer8::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppvObj)
	{
		return E_POINTER;
	}

	if (riid == IID_IDirectSound3DBuffer8 || riid == IID_IUnknown)
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

ULONG m_IDirectSound3DBuffer8::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirectSound3DBuffer8::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

// IDirectSound3DBuffer methods
HRESULT m_IDirectSound3DBuffer8::GetAllParameters(_Out_ LPDS3DBUFFER pDs3dBuffer)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetAllParameters(pDs3dBuffer);
}

HRESULT m_IDirectSound3DBuffer8::GetConeAngles(_Out_ LPDWORD pdwInsideConeAngle, _Out_ LPDWORD pdwOutsideConeAngle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetConeAngles(pdwInsideConeAngle, pdwOutsideConeAngle);
}

HRESULT m_IDirectSound3DBuffer8::GetConeOrientation(_Out_ D3DVECTOR* pvOrientation)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetConeOrientation(pvOrientation);
}

HRESULT m_IDirectSound3DBuffer8::GetConeOutsideVolume(_Out_ LPLONG plConeOutsideVolume)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetConeOutsideVolume(plConeOutsideVolume);
}

HRESULT m_IDirectSound3DBuffer8::GetMaxDistance(_Out_ D3DVALUE* pflMaxDistance)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetMaxDistance(pflMaxDistance);
}

HRESULT m_IDirectSound3DBuffer8::GetMinDistance(_Out_ D3DVALUE* pflMinDistance)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetMinDistance(pflMinDistance);
}

HRESULT m_IDirectSound3DBuffer8::GetMode(_Out_ LPDWORD pdwMode)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetMode(pdwMode);
}

HRESULT m_IDirectSound3DBuffer8::GetPosition(_Out_ D3DVECTOR* pvPosition)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetPosition(pvPosition);
}

HRESULT m_IDirectSound3DBuffer8::GetVelocity(_Out_ D3DVECTOR* pvVelocity)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetVelocity(pvVelocity);
}

HRESULT m_IDirectSound3DBuffer8::SetAllParameters(_In_ LPCDS3DBUFFER pcDs3dBuffer, DWORD dwApply)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetAllParameters(pcDs3dBuffer, dwApply);
}

HRESULT m_IDirectSound3DBuffer8::SetConeAngles(DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, DWORD dwApply)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetConeAngles(dwInsideConeAngle, dwOutsideConeAngle, dwApply);
}

HRESULT m_IDirectSound3DBuffer8::SetConeOrientation(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetConeOrientation(x, y, z, dwApply);
}

HRESULT m_IDirectSound3DBuffer8::SetConeOutsideVolume(LONG lConeOutsideVolume, DWORD dwApply)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetConeOutsideVolume(lConeOutsideVolume, dwApply);
}

HRESULT m_IDirectSound3DBuffer8::SetMaxDistance(D3DVALUE flMaxDistance, DWORD dwApply)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetMaxDistance(flMaxDistance, dwApply);
}

HRESULT m_IDirectSound3DBuffer8::SetMinDistance(D3DVALUE flMinDistance, DWORD dwApply)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetMinDistance(flMinDistance, dwApply);
}

HRESULT m_IDirectSound3DBuffer8::SetMode(DWORD dwMode, DWORD dwApply)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetMode(dwMode, dwApply);
}

HRESULT m_IDirectSound3DBuffer8::SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetPosition(x, y, z, dwApply);
}

HRESULT m_IDirectSound3DBuffer8::SetVelocity(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetVelocity(x, y, z, dwApply);
}
