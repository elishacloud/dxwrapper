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

#include "ddraw.h"

HRESULT m_IDirect3DMaterialX::QueryInterface(REFIID riid, LPVOID * ppvObj, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 3)
	{
		if ((riid == IID_IDirect3DMaterial || riid == IID_IDirect3DMaterial2 || riid == IID_IDirect3DMaterial3 || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return D3D_OK;
		}
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DirectXVersion), WrapperInterface);
}

void *m_IDirect3DMaterialX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 1:
		if (!UniqueProxyInterface.get())
		{
			UniqueProxyInterface = std::make_unique<m_IDirect3DMaterial>(this);
		}
		return UniqueProxyInterface.get();
	case 2:
		if (!UniqueProxyInterface2.get())
		{
			UniqueProxyInterface2 = std::make_unique<m_IDirect3DMaterial2>(this);
		}
		return UniqueProxyInterface2.get();
	case 3:
		if (!UniqueProxyInterface3.get())
		{
			UniqueProxyInterface3 = std::make_unique<m_IDirect3DMaterial3>(this);
		}
		return UniqueProxyInterface3.get();
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return nullptr;
	}
}

ULONG m_IDirect3DMaterialX::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 3)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DMaterialX::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LONG ref;

	if (ProxyDirectXVersion > 3)
	{
		ref = InterlockedDecrement(&RefCount);
	}
	else
	{
		ref = ProxyInterface->Release();
	}

	if (ref == 0)
	{
		if (WrapperInterface)
		{
			WrapperInterface->DeleteMe();
		}
		else
		{
			delete this;
		}
	}

	return ref;
}

HRESULT m_IDirect3DMaterialX::Initialize(LPDIRECT3D lplpD3D)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		return D3D_OK;
	}

	if (lplpD3D)
	{
		lplpD3D = static_cast<m_IDirect3D *>(lplpD3D)->GetProxyInterface();
	}

	return GetProxyInterfaceV1()->Initialize(lplpD3D);
}

HRESULT m_IDirect3DMaterialX::SetMaterial(LPD3DMATERIAL lpMat)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->SetMaterial(lpMat);
	case 2:
		return GetProxyInterfaceV2()->SetMaterial(lpMat);
	case 3:
		return GetProxyInterfaceV3()->SetMaterial(lpMat);
	case 7:
	case 9:
	{
		if (!lpMat || lpMat->dwSize != sizeof(D3DMATERIAL))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ((lpMat) ? lpMat->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no D3DirectDevice interface!");
			return DDERR_GENERIC;
		}

		MaterialSet = true;

		Material.dwSize = sizeof(D3DMATERIAL);
		ConvertMaterial(Material, *lpMat);

		if (lpMat->hTexture || lpMat->dwRampSize)
		{
			LOG_LIMIT(100, __FUNCTION__ << " D3DMATERIALHANDLE and RampSize Not Implemented: " << lpMat->hTexture << " " << lpMat->dwRampSize);
		}

		D3DMATERIAL7 Material7;

		ConvertMaterial(Material7, *lpMat);

		return (*D3DDeviceInterface)->SetMaterial(&Material7);
	}
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DMaterialX::GetMaterial(LPD3DMATERIAL lpMat)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->GetMaterial(lpMat);
	case 2:
		return GetProxyInterfaceV2()->GetMaterial(lpMat);
	case 3:
		return GetProxyInterfaceV3()->GetMaterial(lpMat);
	case 7:
	case 9:
	{
		if (!lpMat)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (MaterialSet)
		{
			lpMat->dwSize = sizeof(D3DMATERIAL);

			ConvertMaterial(*lpMat, Material);

			return D3D_OK;
		}

		return D3DERR_MATERIAL_GETDATA_FAILED;
	}
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DMaterialX::GetHandle(LPDIRECT3DDEVICE3 lpDirect3DDevice, LPD3DMATERIALHANDLE lpHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (lpDirect3DDevice)
	{
		lpDirect3DDevice = static_cast<m_IDirect3DDevice3 *>(lpDirect3DDevice)->GetProxyInterface();
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->GetHandle((LPDIRECT3DDEVICE)lpDirect3DDevice, lpHandle);
	case 2:
		return GetProxyInterfaceV2()->GetHandle((LPDIRECT3DDEVICE2)lpDirect3DDevice, lpHandle);
	case 3:
		return GetProxyInterfaceV3()->GetHandle(lpDirect3DDevice, lpHandle);
	case 7:
	case 9:
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DMaterialX::Reserve()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return GetProxyInterfaceV1()->Reserve();
}

HRESULT m_IDirect3DMaterialX::Unreserve()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return GetProxyInterfaceV1()->Unreserve();
}
