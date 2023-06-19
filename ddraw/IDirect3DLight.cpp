/**
* Copyright (C) 2022 Elisha Riedlinger
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

#include "ddraw.h"

HRESULT m_IDirect3DLight::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ppvObj)
	{
		return E_POINTER;
	}

	if (riid == IID_GetRealInterface)
	{
		*ppvObj = ProxyInterface;
		return DD_OK;
	}
	if (riid == IID_GetInterfaceX)
	{
		*ppvObj = this;
		return DD_OK;
	}

	if (riid == IID_IDirect3DLight || riid == IID_IUnknown)
	{
		AddRef();

		*ppvObj = this;

		return D3D_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID);
}

ULONG m_IDirect3DLight::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DLight::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LONG ref;

	if (!ProxyInterface)
	{
		ref = InterlockedDecrement(&RefCount);
	}
	else
	{
		ref = ProxyInterface->Release();
	}

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

HRESULT m_IDirect3DLight::Initialize(LPDIRECT3D lpDirect3D)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		// The method returns DDERR_ALREADYINITIALIZED because the Direct3DLight object is initialized when it is created.
		return DDERR_ALREADYINITIALIZED;
	}

	if (lpDirect3D)
	{
		lpDirect3D->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3D);
	}

	return ProxyInterface->Initialize(lpDirect3D);
}

HRESULT m_IDirect3DLight::SetLight(LPD3DLIGHT lpLight)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if (!lpLight || lpLight->dwSize != sizeof(D3DLIGHT))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ((lpLight) ? lpLight->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no D3DirectDevice interface!");
			return DDERR_GENERIC;
		}

		D3DLIGHT7 Light7;

		ConvertLight(Light7, *lpLight);

		HRESULT hr = (*D3DDeviceInterface)->SetLight(0, &Light7);

		if (FAILED(hr))
		{
			return D3DERR_LIGHT_SET_FAILED;
		}

		LightSet = true;

		Light.dwSize = sizeof(D3DLIGHT);
		ConvertLight(Light, *lpLight);

		return D3D_OK;
	}

	return ProxyInterface->SetLight(lpLight);
}

HRESULT m_IDirect3DLight::GetLight(LPD3DLIGHT lpLight)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if (!lpLight)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (LightSet)
		{
			lpLight->dwSize = sizeof(D3DLIGHT);

			ConvertLight(*lpLight, Light);

			return D3D_OK;
		}

		return DDERR_GENERIC;
	}

	return ProxyInterface->GetLight(lpLight);
}

/************************/
/*** Helper functions ***/
/************************/

void m_IDirect3DLight::InitLight()
{
	// To add later
}

void m_IDirect3DLight::ReleaseLight()
{
	// To add later
}
