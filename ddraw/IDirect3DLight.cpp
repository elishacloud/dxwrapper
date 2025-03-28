/**
* Copyright (C) 2024 Elisha Riedlinger
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

namespace {
	m_IDirect3DLight* WrapperInterfaceBackup = nullptr;
}

// ******************************
// IUnknown functions
// ******************************

HRESULT m_IDirect3DLight::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ProxyInterface && !D3DInterface)
	{
		if (ppvObj)
		{
			*ppvObj = nullptr;
		}
		return E_NOINTERFACE;
	}

	if (!ppvObj)
	{
		return E_POINTER;
	}
	*ppvObj = nullptr;

	if (riid == IID_GetRealInterface)
	{
		*ppvObj = ProxyInterface;
		return D3D_OK;
	}
	if (riid == IID_GetInterfaceX)
	{
		*ppvObj = this;
		return D3D_OK;
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

	if (!ProxyInterface && !D3DInterface)
	{
		return 0;
	}

	if (Config.Dd7to9)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DLight::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DInterface)
	{
		return 0;
	}

	if (Config.Dd7to9)
	{
		ULONG ref = (InterlockedCompareExchange(&RefCount, 0, 0)) ? InterlockedDecrement(&RefCount) : 0;

		if (ref == 0)
		{
			SaveInterfaceAddress(this, WrapperInterfaceBackup);
		}

		return ref;
	}

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		SaveInterfaceAddress(this, WrapperInterfaceBackup);
	}

	return ref;
}

// ******************************
// IDirect3DLight functions
// ******************************

HRESULT m_IDirect3DLight::Initialize(LPDIRECT3D lpDirect3D)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (Config.Dd7to9)
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

	if (!ProxyInterface && !D3DInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (Config.Dd7to9)
	{
		// Although this method's declaration specifies the lpLight parameter as being the address of a D3DLIGHT structure, that structure is not normally used.
		// Rather, the D3DLIGHT2 structure is recommended to achieve the best lighting effects.

		if (!lpLight || (lpLight->dwSize != sizeof(D3DLIGHT) && lpLight->dwSize != sizeof(D3DLIGHT2)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ((lpLight) ? lpLight->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		// If current light is in use then update device
		BOOL Enable = FALSE;
		if (SUCCEEDED(CheckInterface(__FUNCTION__)) && SUCCEEDED((*D3DDeviceInterface)->GetLightEnable(this, &Enable)) && Enable)
		{
			D3DLIGHT2 Light2 = {};
			memcpy(&Light2, lpLight, lpLight->dwSize);
			Light2.dwSize = sizeof(D3DLIGHT2);
			Light2.dwFlags |= D3DLIGHT_ACTIVE;

			HRESULT hr = (*D3DDeviceInterface)->SetLight(this, (LPD3DLIGHT)&Light2);

			if (FAILED(hr))
			{
				return D3DERR_LIGHT_SET_FAILED;
			}
		}

		LightSet = true;

		memcpy(&Light, lpLight, lpLight->dwSize);

		return D3D_OK;
	}

	return ProxyInterface->SetLight(lpLight);
}

HRESULT m_IDirect3DLight::GetLight(LPD3DLIGHT lpLight)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (Config.Dd7to9)
	{
		// Although this method's declaration specifies the lpLight parameter as being the address of a D3DLIGHT structure, that structure is not normally used.
		// Rather, the D3DLIGHT2 structure is recommended to achieve the best lighting effects.

		if (!lpLight || (lpLight->dwSize != sizeof(D3DLIGHT) && lpLight->dwSize != sizeof(D3DLIGHT2)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ((lpLight) ? lpLight->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		if (!LightSet)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: light has not yet been set.");
			return DDERR_GENERIC;
		}

		// Copy light
		DWORD Size = lpLight->dwSize;
		memcpy(lpLight, &Light, Size);
		lpLight->dwSize = Size;

		// D3DLIGHT2
		if (lpLight->dwSize == sizeof(D3DLIGHT2))
		{
			// Reset flags if Light struct does not have them because it is using the old structure
			if (Light.dwSize == sizeof(D3DLIGHT))
			{
				((LPD3DLIGHT2)lpLight)->dwFlags = NULL;
			}

			// Check for active
			BOOL Enable = FALSE;
			if (SUCCEEDED(CheckInterface(__FUNCTION__)) && SUCCEEDED((*D3DDeviceInterface)->GetLightEnable(this, &Enable)) && Enable)
			{
				((LPD3DLIGHT2)lpLight)->dwFlags |= D3DLIGHT_ACTIVE;
			}
			else
			{
				((LPD3DLIGHT2)lpLight)->dwFlags &= ~D3DLIGHT_ACTIVE;
			}
		}

		return D3D_OK;
	}

	return ProxyInterface->GetLight(lpLight);
}

// ******************************
// Helper functions
// ******************************

void m_IDirect3DLight::InitInterface()
{
	if (D3DInterface)
	{
		D3DInterface->AddLight(this);
	}

	LightSet = false;
}

void m_IDirect3DLight::ReleaseInterface()
{
	if (Config.Exiting)
	{
		return;
	}

	if (D3DInterface)
	{
		D3DInterface->ClearLight(this);
	}

	if (D3DDeviceInterface && *D3DDeviceInterface)
	{
		(*D3DDeviceInterface)->ClearLight(this);
	}

	ClearD3D();
}

HRESULT m_IDirect3DLight::CheckInterface(char* FunctionName)
{
	// Check D3DInterface device
	if (!D3DInterface)
	{
		LOG_LIMIT(100, FunctionName << " Error: no D3D parent!");
		return DDERR_INVALIDOBJECT;
	}

	// Check d3d9 device
	if (!D3DDeviceInterface || !*D3DDeviceInterface)
	{
		D3DDeviceInterface = D3DInterface->GetD3DDevice();
		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			return DDERR_INVALIDOBJECT;
		}
	}

	return D3D_OK;
}

m_IDirect3DDeviceX* m_IDirect3DLight::GetD3DDevice()
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__)))
	{
		return nullptr;
	}

	return *D3DDeviceInterface;
}

m_IDirect3DLight* m_IDirect3DLight::CreateDirect3DLight(IDirect3DLight* aOriginal, m_IDirect3DX* NewD3DInterface)
{
	m_IDirect3DLight* Interface = nullptr;
	if (WrapperInterfaceBackup)
	{
		Interface = WrapperInterfaceBackup;
		WrapperInterfaceBackup = nullptr;
		Interface->SetProxy(aOriginal, NewD3DInterface);
	}
	else
	{
		if (aOriginal)
		{
			Interface = new m_IDirect3DLight(aOriginal);
		}
		else
		{
			Interface = new m_IDirect3DLight(NewD3DInterface);
		}
	}
	return Interface;
}
