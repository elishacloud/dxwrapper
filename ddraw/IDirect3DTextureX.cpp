/**
* Copyright (C) 2023 Elisha Riedlinger
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

HRESULT m_IDirect3DTextureX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ppvObj)
	{
		return E_POINTER;
	}

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

	if (DirectXVersion != 1 && DirectXVersion != 2)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return E_NOINTERFACE;
	}

	DWORD DxVersion = (CheckWrapperType(riid) && (Config.Dd7to9 || Config.ConvertToDirect3D7)) ? GetGUIDVersion(riid) : DirectXVersion;

	if (riid == GetWrapperType(DxVersion) || riid == IID_IUnknown)
	{
		*ppvObj = GetWrapperInterfaceX(DxVersion);

		AddRef(DxVersion);

		return D3D_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DxVersion));
}

void *m_IDirect3DTextureX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 1:
		return WrapperInterface;
	case 2:
		return WrapperInterface2;
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return nullptr;
	}
}

ULONG m_IDirect3DTextureX::AddRef(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") v" << DirectXVersion;

	if (!ProxyInterface)
	{
		switch (DirectXVersion)
		{
		case 1:
			return InterlockedIncrement(&RefCount1);
		case 2:
			return InterlockedIncrement(&RefCount2);
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			return 0;
		}
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DTextureX::Release(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") v" << DirectXVersion;

	ULONG ref;

	if (!ProxyInterface)
	{
		switch (DirectXVersion)
		{
		case 1:
			ref = (InterlockedCompareExchange(&RefCount1, 0, 0)) ? InterlockedDecrement(&RefCount1) : 0;
			break;
		case 2:
			ref = (InterlockedCompareExchange(&RefCount2, 0, 0)) ? InterlockedDecrement(&RefCount2) : 0;
			break;
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			ref = 0;
		}

		if (InterlockedCompareExchange(&RefCount1, 0, 0) + InterlockedCompareExchange(&RefCount2, 0, 0) == 0)
		{
			delete this;
		}
	}
	else
	{
		ref = ProxyInterface->Release();

		if (ref == 0)
		{
			delete this;
		}
	}

	return ref;
}

HRESULT m_IDirect3DTextureX::Initialize(LPDIRECT3DDEVICE lpDirect3DDevice, LPDIRECTDRAWSURFACE lplpDDSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		// Former stub method. This method was never implemented and is not supported in any interface.
		return D3D_OK;
	}

	if (lpDirect3DDevice)
	{
		lpDirect3DDevice->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DDevice);
	}
	if (lplpDDSurface)
	{
		lplpDDSurface->QueryInterface(IID_GetRealInterface, (LPVOID*)&lplpDDSurface);
	}

	return GetProxyInterfaceV1()->Initialize(lpDirect3DDevice, lplpDDSurface);
}

HRESULT m_IDirect3DTextureX::GetHandle(LPDIRECT3DDEVICE2 lpDirect3DDevice2, LPD3DTEXTUREHANDLE lpHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if (!lpDirect3DDevice2 || !lpHandle)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: D3DDevice does not exist!");
			return DDERR_GENERIC;
		}

		// ToDo: Validate Direct3D Device
		m_IDirect3DDeviceX* pDirect3DDeviceX = nullptr;
		lpDirect3DDevice2->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pDirect3DDeviceX);
		if (!pDirect3DDeviceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get Direct3D Device wrapper!");
			return DDERR_INVALIDPARAMS;
		}

		if (*D3DDeviceInterface != pDirect3DDeviceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Direct3D Device wrapper does not match! " << *D3DDeviceInterface << "->" << pDirect3DDeviceX);
		}

		(*D3DDeviceInterface)->SetTextureHandle(tHandle, this);

		*lpHandle = tHandle;

		return D3D_OK;
	}

	if (lpDirect3DDevice2)
	{
		lpDirect3DDevice2->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DDevice2);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->GetHandle((LPDIRECT3DDEVICE)lpDirect3DDevice2, lpHandle);
	case 2:
		return GetProxyInterfaceV2()->GetHandle(lpDirect3DDevice2, lpHandle);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DTextureX::SetHandle(DWORD dwHandle)
{
	if (!dwHandle)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: NULL pointer found!");
		return DDERR_GENERIC;
	}

	tHandle = dwHandle;

	return D3D_OK;
}

HRESULT m_IDirect3DTextureX::PaletteChanged(DWORD dwStart, DWORD dwCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->PaletteChanged(dwStart, dwCount);
	case 2:
		return GetProxyInterfaceV2()->PaletteChanged(dwStart, dwCount);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DTextureX::Load(LPDIRECT3DTEXTURE2 lpD3DTexture2)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if (lpD3DTexture2)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: D3DDevice does not exist!");
			return DDERR_GENERIC;
		}

		if (!DDrawSurface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not attached!");
			return DDERR_GENERIC;
		}

		m_IDirect3DTextureX* pSrcTextureX = nullptr;
		lpD3DTexture2->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pSrcTextureX);

		if (!pSrcTextureX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get texture wrapper!");
			return DDERR_GENERIC;
		}

		m_IDirectDrawSurfaceX* pSrcSurfaceX = pSrcTextureX->GetSurface();
		if (!pSrcSurfaceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surface!");
			return DDERR_GENERIC;
		}

		IDirectDrawSurface7* pSrcSurface7 = (IDirectDrawSurface7*)pSrcSurfaceX->GetWrapperInterfaceX(7);
		IDirectDrawSurface7* pDestSurface7 = (IDirectDrawSurface7*)DDrawSurface->GetWrapperInterfaceX(7);

		if (!pDestSurface7 || !pSrcSurface7)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get v7 surface interface!");
			return DDERR_GENERIC;
		}

		POINT Point = {};
		return (*D3DDeviceInterface)->Load(pDestSurface7, &Point, pSrcSurface7, nullptr, 0);
	}

	if (lpD3DTexture2)
	{
		lpD3DTexture2->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpD3DTexture2);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->Load((LPDIRECT3DTEXTURE)lpD3DTexture2);
	case 2:
		return GetProxyInterfaceV2()->Load(lpD3DTexture2);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DTextureX::Unload()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		// Former stub method. This method was never implemented and is not supported in any interface.
		return D3D_OK;
	}

	return GetProxyInterfaceV1()->Unload();
}

/************************/
/*** Helper functions ***/
/************************/

void m_IDirect3DTextureX::InitTexture(DWORD DirectXVersion)
{
	WrapperInterface = new m_IDirect3DTexture((LPDIRECT3DTEXTURE)ProxyInterface, this);
	WrapperInterface2 = new m_IDirect3DTexture2((LPDIRECT3DTEXTURE2)ProxyInterface, this);

	if (ProxyInterface)
	{
		return;
	}

	AddRef(DirectXVersion);

	tHandle = (DWORD)this + 32;
}

void m_IDirect3DTextureX::ReleaseTexture()
{
	WrapperInterface->DeleteMe();
	WrapperInterface2->DeleteMe();

	if (D3DDeviceInterface && *D3DDeviceInterface)
	{
		(*D3DDeviceInterface)->ReleaseTextureHandle(this);
	}

	if (DDrawSurface)
	{
		DDrawSurface->ClearAttachedTexture();
	}
}
