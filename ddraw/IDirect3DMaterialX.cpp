/**
* Copyright (C) 2020 Elisha Riedlinger
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

HRESULT m_IDirect3DMaterialX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ppvObj && riid == IID_GetRealInterface)
	{
		*ppvObj = ProxyInterface;
		return DD_OK;
	}
	if (ppvObj && riid == IID_GetInterfaceX)
	{
		*ppvObj = this;
		return DD_OK;
	}

	DWORD DxVersion = (CheckWrapperType(riid) && (Config.Dd7to9 || Config.ConvertToDirect3D7)) ? GetGUIDVersion(riid) : DirectXVersion;

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DxVersion), GetWrapperInterfaceX(DxVersion));
}

void *m_IDirect3DMaterialX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 1:
		return WrapperInterface;
	case 2:
		return WrapperInterface2;
	case 3:
		return WrapperInterface3;
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return nullptr;
	}
}

ULONG m_IDirect3DMaterialX::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DMaterialX::Release()
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

HRESULT m_IDirect3DMaterialX::Initialize(LPDIRECT3D lplpD3D)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Former stub method. This method was never implemented and is not supported in any interface.
	if (ProxyDirectXVersion != 1)
	{
		return D3D_OK;
	}

	if (lplpD3D)
	{
		lplpD3D->QueryInterface(IID_GetRealInterface, (LPVOID*)&lplpD3D);
	}

	return GetProxyInterfaceV1()->Initialize(lplpD3D);
}

HRESULT m_IDirect3DMaterialX::SetMaterial(LPD3DMATERIAL lpMat)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
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

		if (lpMat->hTexture)
		{
			LOG_LIMIT(100, __FUNCTION__ << " D3DTEXTUREHANDLE Not Implemented: " << lpMat->hTexture);
		}

		if (lpMat->dwRampSize)
		{
			LOG_LIMIT(100, __FUNCTION__ << " RampSize Not Implemented: " << lpMat->dwRampSize);
		}

		D3DMATERIAL7 Material7;

		ConvertMaterial(Material7, *lpMat);

		return (*D3DDeviceInterface)->SetMaterial(&Material7);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->SetMaterial(lpMat);
	case 2:
		return GetProxyInterfaceV2()->SetMaterial(lpMat);
	case 3:
		return GetProxyInterfaceV3()->SetMaterial(lpMat);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DMaterialX::GetMaterial(LPD3DMATERIAL lpMat)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
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

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->GetMaterial(lpMat);
	case 2:
		return GetProxyInterfaceV2()->GetMaterial(lpMat);
	case 3:
		return GetProxyInterfaceV3()->GetMaterial(lpMat);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DMaterialX::GetHandle(LPDIRECT3DDEVICE3 lpDirect3DDevice, LPD3DMATERIALHANDLE lpHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if (lpHandle)
		{
			*lpHandle = mHandle;
		}
		return D3D_OK;
	}

	if (lpDirect3DDevice)
	{
		lpDirect3DDevice->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DDevice);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->GetHandle((LPDIRECT3DDEVICE)lpDirect3DDevice, lpHandle);
	case 2:
		return GetProxyInterfaceV2()->GetHandle((LPDIRECT3DDEVICE2)lpDirect3DDevice, lpHandle);
	case 3:
		return GetProxyInterfaceV3()->GetHandle(lpDirect3DDevice, lpHandle);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DMaterialX::Reserve()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Former stub method. This method was never implemented and is not supported in any interface.
	if (ProxyDirectXVersion != 1)
	{
		return DD_OK;
	}

	return GetProxyInterfaceV1()->Reserve();
}

HRESULT m_IDirect3DMaterialX::Unreserve()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Former stub method. This method was never implemented and is not supported in any interface.
	if (ProxyDirectXVersion != 1)
	{
		return DD_OK;
	}

	return GetProxyInterfaceV1()->Unreserve();
}

/************************/
/*** Helper functions ***/
/************************/

void m_IDirect3DMaterialX::InitMaterial()
{
	WrapperInterface = new m_IDirect3DMaterial((LPDIRECT3DMATERIAL)ProxyInterface, this);
	WrapperInterface2 = new m_IDirect3DMaterial2((LPDIRECT3DMATERIAL2)ProxyInterface, this);
	WrapperInterface3 = new m_IDirect3DMaterial3((LPDIRECT3DMATERIAL3)ProxyInterface, this);

	mHandle = (DWORD)this + 32;
}

void m_IDirect3DMaterialX::ReleaseMaterial()
{
	WrapperInterface->DeleteMe();
	WrapperInterface2->DeleteMe();
	WrapperInterface3->DeleteMe();
}
