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

HRESULT m_IDirect3DViewportX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

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

	if (DirectXVersion != 1 && DirectXVersion != 2 && DirectXVersion != 3)
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

void *m_IDirect3DViewportX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 1:
		if (!WrapperInterface)
		{
			WrapperInterface = new m_IDirect3DViewport((LPDIRECT3DVIEWPORT)ProxyInterface, this);
		}
		return WrapperInterface;
	case 2:
		if (!WrapperInterface2)
		{
			WrapperInterface2 = new m_IDirect3DViewport2((LPDIRECT3DVIEWPORT2)ProxyInterface, this);
		}
		return WrapperInterface2;
	case 3:
		if (!WrapperInterface3)
		{
			WrapperInterface3 = new m_IDirect3DViewport3((LPDIRECT3DVIEWPORT3)ProxyInterface, this);
		}
		return WrapperInterface3;
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return nullptr;
	}
}

ULONG m_IDirect3DViewportX::AddRef(DWORD DirectXVersion)
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
		case 3:
			return InterlockedIncrement(&RefCount3);
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			return 0;
		}
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DViewportX::Release(DWORD DirectXVersion)
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
		case 3:
			ref = (InterlockedCompareExchange(&RefCount3, 0, 0)) ? InterlockedDecrement(&RefCount3) : 0;
			break;
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			ref = 0;
		}

		if (InterlockedCompareExchange(&RefCount1, 0, 0) + InterlockedCompareExchange(&RefCount2, 0, 0) +
			InterlockedCompareExchange(&RefCount3, 0, 0) == 0)
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

HRESULT m_IDirect3DViewportX::Initialize(LPDIRECT3D lpDirect3D)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		// The method returns DDERR_ALREADYINITIALIZED because the IDirect3DViewport object is initialized when it is created.
		return DDERR_ALREADYINITIALIZED;
	}

	if (lpDirect3D)
	{
		lpDirect3D->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3D);
	}

	return ProxyInterface->Initialize(lpDirect3D);
}

HRESULT m_IDirect3DViewportX::GetViewport(LPD3DVIEWPORT lpData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if (!lpData || lpData->dwSize != sizeof(D3DVIEWPORT))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ((lpData) ? lpData->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no D3DirectDevice interface!");
			return DDERR_GENERIC;
		}

		HRESULT hr = D3D_OK;

		if (IsViewPortSet)
		{
			*lpData = vData;
		}
		else if (IsViewPort2Set)
		{
			ConvertViewport(*lpData, vData2);
		}
		else
		{
			D3DVIEWPORT7 Viewport7 = {};

			(*D3DDeviceInterface)->GetDefaultViewport(*(D3DVIEWPORT9*)&Viewport7);

			ConvertViewport(*lpData, Viewport7);
		}

		return hr;
	}

	return ProxyInterface->GetViewport(lpData);
}

HRESULT m_IDirect3DViewportX::SetViewport(LPD3DVIEWPORT lpData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if (!lpData || lpData->dwSize != sizeof(D3DVIEWPORT))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ((lpData) ? lpData->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no D3DirectDevice interface!");
			return DDERR_GENERIC;
		}

		if (lpData->dvScaleX != 0 || lpData->dvScaleY != 0 || lpData->dvMaxX != 0 || lpData->dvMaxY != 0)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'Scale homogeneous' Not Implemented: " <<
				" ScaleX: " << lpData->dvScaleX << " ScaleY: " << lpData->dvScaleY << " MaxX: " << lpData->dvMaxX << " MaxY: " << lpData->dvMaxY);
		}

		// If current viewport is set then use new viewport
		if ((*D3DDeviceInterface)->CheckIfViewportSet(this))
		{
			D3DVIEWPORT7 Viewport7 = {};
			ConvertViewport(Viewport7, *lpData);
			if (FAILED((*D3DDeviceInterface)->SetViewport(&Viewport7)))
			{
				return DDERR_GENERIC;
			}
		}

		IsViewPortSet = true;

		vData = *lpData;

		return D3D_OK;
	}

	if (Config.DdrawUseNativeResolution && lpData)
	{
		lpData->dwX = (LONG)(lpData->dwX * ScaleDDWidthRatio) + ScaleDDPadX;
		lpData->dwY = (LONG)(lpData->dwY * ScaleDDHeightRatio) + ScaleDDPadY;
		lpData->dwWidth = (LONG)(lpData->dwWidth * ScaleDDWidthRatio);
		lpData->dwHeight = (LONG)(lpData->dwHeight * ScaleDDHeightRatio);
	}

	return ProxyInterface->SetViewport(lpData);
}

HRESULT m_IDirect3DViewportX::TransformVertices(DWORD dwVertexCount, LPD3DTRANSFORMDATA lpData, DWORD dwFlags, LPDWORD lpOffscreen)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->TransformVertices(dwVertexCount, lpData, dwFlags, lpOffscreen);
}

HRESULT m_IDirect3DViewportX::LightElements(DWORD dwElementCount, LPD3DLIGHTDATA lpData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->LightElements(dwElementCount, lpData);
}

HRESULT m_IDirect3DViewportX::SetBackground(D3DMATERIALHANDLE hMat)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		// ToDo: validate handle
		MaterialBackground.IsSet = TRUE;
		MaterialBackground.hMat = hMat;

		// If current viewport is set then use new viewport
		if ((*D3DDeviceInterface)->CheckIfViewportSet(this))
		{
			if (FAILED((*D3DDeviceInterface)->SetLightState(D3DLIGHTSTATE_MATERIAL, hMat)))
			{
				return DDERR_GENERIC;
			}
		}

		return D3D_OK;
	}

	return ProxyInterface->SetBackground(hMat);
}

HRESULT m_IDirect3DViewportX::GetBackground(LPD3DMATERIALHANDLE lphMat, LPBOOL lpValid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if (!lphMat || !lpValid)
		{
			return DDERR_INVALIDPARAMS;
		}

		*lphMat = MaterialBackground.IsSet;
		*lpValid = MaterialBackground.hMat;

		return D3D_OK;
	}

	return ProxyInterface->GetBackground(lphMat, lpValid);
}

HRESULT m_IDirect3DViewportX::SetBackgroundDepth(LPDIRECTDRAWSURFACE lpDDSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpDDSurface)
	{
		lpDDSurface->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDSurface);
	}

	return ProxyInterface->SetBackgroundDepth(lpDDSurface);
}

HRESULT m_IDirect3DViewportX::GetBackgroundDepth(LPDIRECTDRAWSURFACE * lplpDDSurface, LPBOOL lpValid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	HRESULT hr = ProxyInterface->GetBackgroundDepth(lplpDDSurface, lpValid);

	if (SUCCEEDED(hr) && lplpDDSurface)
	{
		*lplpDDSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface>(*lplpDDSurface);
	}

	return hr;
}

HRESULT m_IDirect3DViewportX::Clear(DWORD dwCount, LPD3DRECT lpRects, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		return Clear2(dwCount, lpRects, dwFlags, 0x00000000, 1.0f, 0);
	}

	return ProxyInterface->Clear(dwCount, lpRects, dwFlags);
}

HRESULT m_IDirect3DViewportX::AddLight(LPDIRECT3DLIGHT lpDirect3DLight)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpDirect3DLight)
	{
		lpDirect3DLight->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DLight);
	}

	return ProxyInterface->AddLight(lpDirect3DLight);
}

HRESULT m_IDirect3DViewportX::DeleteLight(LPDIRECT3DLIGHT lpDirect3DLight)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpDirect3DLight)
	{
		lpDirect3DLight->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DLight);
	}

	return ProxyInterface->DeleteLight(lpDirect3DLight);
}

HRESULT m_IDirect3DViewportX::NextLight(LPDIRECT3DLIGHT lpDirect3DLight, LPDIRECT3DLIGHT* lplpDirect3DLight, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpDirect3DLight)
	{
		lpDirect3DLight->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DLight);
	}

	HRESULT hr = ProxyInterface->NextLight(lpDirect3DLight, lplpDirect3DLight, dwFlags);

	if (SUCCEEDED(hr) && lplpDirect3DLight)
	{
		*lplpDirect3DLight = ProxyAddressLookupTable.FindAddress<m_IDirect3DLight>(*lplpDirect3DLight);
	}

	return hr;
}

HRESULT m_IDirect3DViewportX::GetViewport2(LPD3DVIEWPORT2 lpData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if (!lpData || lpData->dwSize != sizeof(D3DVIEWPORT2))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ((lpData) ? lpData->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		HRESULT hr = D3D_OK;

		if (IsViewPort2Set)
		{
			*lpData = vData2;
		}
		else if (IsViewPortSet)
		{
			ConvertViewport(*lpData, vData);
		}
		else
		{
			if (!D3DDeviceInterface || !*D3DDeviceInterface)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: no D3DirectDevice interface!");
				return DDERR_GENERIC;
			}

			D3DVIEWPORT7 Viewport7 = {};

			(*D3DDeviceInterface)->GetDefaultViewport(*(D3DVIEWPORT9*)&Viewport7);

			ConvertViewport(*lpData, Viewport7);
		}

		return hr;
	}

	return ProxyInterface->GetViewport2(lpData);
}

HRESULT m_IDirect3DViewportX::SetViewport2(LPD3DVIEWPORT2 lpData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if (!lpData || lpData->dwSize != sizeof(D3DVIEWPORT2))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ((lpData) ? lpData->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no D3DirectDevice interface!");
			return DDERR_GENERIC;
		}

		if (lpData->dvClipWidth != 0 || lpData->dvClipHeight != 0 || lpData->dvClipX != 0 || lpData->dvClipY != 0)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'clip volume' Not Implemented: " <<
				lpData->dvClipWidth << "x" << lpData->dvClipHeight << " X: " << lpData->dvClipX << " Y: " << lpData->dvClipY);
		}

		// If current viewport is set then use new viewport
		if ((*D3DDeviceInterface)->CheckIfViewportSet(this))
		{
			D3DVIEWPORT7 Viewport7 = {};
			ConvertViewport(Viewport7, *lpData);
			if (FAILED((*D3DDeviceInterface)->SetViewport(&Viewport7)))
			{
				return DDERR_GENERIC;
			}
		}
		IsViewPort2Set = true;

		vData2 = *lpData;

		return D3D_OK;
	}

	if (Config.DdrawUseNativeResolution && lpData)
	{
		lpData->dwX = (LONG)(lpData->dwX * ScaleDDWidthRatio) + ScaleDDPadX;
		lpData->dwY = (LONG)(lpData->dwY * ScaleDDHeightRatio) + ScaleDDPadY;
		lpData->dwWidth = (LONG)(lpData->dwWidth * ScaleDDWidthRatio);
		lpData->dwHeight = (LONG)(lpData->dwHeight * ScaleDDHeightRatio);
	}

	return ProxyInterface->SetViewport2(lpData);
}

HRESULT m_IDirect3DViewportX::SetBackgroundDepth2(LPDIRECTDRAWSURFACE4 lpDDS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpDDS)
	{
		lpDDS->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDS);
	}

	return ProxyInterface->SetBackgroundDepth2(lpDDS);
}

HRESULT m_IDirect3DViewportX::GetBackgroundDepth2(LPDIRECTDRAWSURFACE4* lplpDDS, LPBOOL lpValid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	HRESULT hr = ProxyInterface->GetBackgroundDepth2(lplpDDS, lpValid);

	if (SUCCEEDED(hr) && lplpDDS)
	{
		*lplpDDS = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface4>(*lplpDDS);
	}

	return hr;
}

HRESULT m_IDirect3DViewportX::Clear2(DWORD dwCount, LPD3DRECT lpRects, DWORD dwFlags, D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no D3DirectDevice interface!");
			return DDERR_GENERIC;
		}

		return (*D3DDeviceInterface)->Clear(dwCount, lpRects, dwFlags, dwColor, dvZ, dwStencil);
	}

	return ProxyInterface->Clear2(dwCount, lpRects, dwFlags, dwColor, dvZ, dwStencil);
}

/************************/
/*** Helper functions ***/
/************************/

void m_IDirect3DViewportX::InitViewport(DWORD DirectXVersion)
{
	if (ProxyInterface)
	{
		return;
	}

	AddRef(DirectXVersion);
}

void m_IDirect3DViewportX::ReleaseViewport()
{
	if (WrapperInterface)
	{
		WrapperInterface->DeleteMe();
	}
	if (WrapperInterface2)
	{
		WrapperInterface2->DeleteMe();
	}
	if (WrapperInterface3)
	{
		WrapperInterface3->DeleteMe();
	}
}
