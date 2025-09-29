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

#include "ddraw.h"

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
			SaveInterfaceAddress(this);
		}

		return ref;
	}

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		SaveInterfaceAddress(this);
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
		if (!lpLight)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Although this method's declaration specifies the lpLight parameter as being the address of a D3DLIGHT structure, that structure is not normally used.
		// Rather, the D3DLIGHT2 structure is recommended to achieve the best lighting effects.

		// Unlike its predecessors, the IDirect3DDevice7 interface does not use light objects. This method, and its use of the D3DLIGHT7 structure to describe a
		// set of lighting properties, replaces the lighting semantics used by previous versions of the device interface.

		if (lpLight->dwSize != sizeof(D3DLIGHT) && lpLight->dwSize != sizeof(D3DLIGHT2))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << lpLight->dwSize);
			return DDERR_INVALIDPARAMS;
		}

		// Only save D3DLIGHT struct without dwFlags, not D3DLIGHT2
		D3DLIGHT2 tmpLight2;
		memcpy(&tmpLight2, lpLight, sizeof(D3DLIGHT));
		tmpLight2.dwSize = sizeof(tmpLight2);
		tmpLight2.dwFlags = Light.dwFlags;

		// Handle dwFlags
		if (lpLight->dwSize == sizeof(D3DLIGHT2))
		{
			LPD3DLIGHT2 lpLight2 = reinterpret_cast<LPD3DLIGHT2>(lpLight);

			// Add active flag
			if (lpLight2->dwFlags & D3DLIGHT_ACTIVE)
			{
				tmpLight2.dwFlags |= D3DLIGHT_ACTIVE;
			}
			// Remove active flag
			else
			{
				tmpLight2.dwFlags &= ~D3DLIGHT_ACTIVE;
			}

			// Add no specular flag
			if (lpLight2->dwFlags & D3DLIGHT_NO_SPECULAR)
			{
				tmpLight2.dwFlags |= D3DLIGHT_NO_SPECULAR;
			}
			// Remove no specular flag
			else
			{
				tmpLight2.dwFlags &= ~D3DLIGHT_NO_SPECULAR;
			}
		}
		// Default to active after light has been set
		else if (!LightSet)
		{
			tmpLight2.dwFlags |= D3DLIGHT_ACTIVE;
		}

		// If current light is in use then update device
		{
			DWORD x = 0;
			while (D3DInterface)
			{
				m_IDirect3DDeviceX* D3DDeviceInterface = D3DInterface->GetNextD3DDevice(x++);
				if (!D3DDeviceInterface)
				{
					break;
				}
				if (D3DDeviceInterface->IsLightInUse(this))
				{
					if (FAILED(D3DDeviceInterface->SetLight(this, reinterpret_cast<LPD3DLIGHT>(&tmpLight2))))
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set light!");
						return D3DERR_LIGHT_SET_FAILED;
					}
				}
			}
		}

		LightSet = true;

		// Only copy D3DLIGHT struct without dwFlags, not D3DLIGHT2
		memcpy(&Light, &tmpLight2, sizeof(D3DLIGHT));
		Light.dwSize = sizeof(Light);
		Light.dwFlags = tmpLight2.dwFlags;		// Add flags manually

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
		if (!lpLight)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Although this method's declaration specifies the lpLight parameter as being the address of a D3DLIGHT structure, that structure is not normally used.
		// Rather, the D3DLIGHT2 structure is recommended to achieve the best lighting effects.

		// Unlike its predecessors, the IDirect3DDevice7 interface does not use light objects. This method, and its use of the D3DLIGHT7 structure to describe a
		// set of lighting properties, replaces the lighting semantics used by previous versions of the device interface.

		if (lpLight->dwSize != sizeof(D3DLIGHT) && lpLight->dwSize != sizeof(D3DLIGHT2))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << lpLight->dwSize);
			return DDERR_INVALIDPARAMS;
		}

		if (!LightSet)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: light has not yet been set.");
			return DDERR_INVALIDPARAMS;
		}

		// Copy light
		DWORD Size = lpLight->dwSize;
		memcpy(lpLight, &Light, Size);
		lpLight->dwSize = Size;

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
	Light = {};
	Light.dwSize = sizeof(Light);
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

	ClearD3D();
}

m_IDirect3DLight* m_IDirect3DLight::CreateDirect3DLight(IDirect3DLight* aOriginal, m_IDirect3DX* NewD3DInterface)
{
	m_IDirect3DLight* Interface = InterfaceAddressCache<m_IDirect3DLight>(nullptr);
	if (Interface)
	{
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
