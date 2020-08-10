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

HRESULT m_IDirect3DViewportX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
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

void *m_IDirect3DViewportX::GetWrapperInterfaceX(DWORD DirectXVersion)
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

ULONG m_IDirect3DViewportX::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DViewportX::Release()
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

HRESULT m_IDirect3DViewportX::Initialize(LPDIRECT3D lpDirect3D)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		return D3D_OK;
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
		if (!lpData)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no D3DirectDevice interface!");
			return DDERR_GENERIC;
		}

		D3DVIEWPORT7 ViewPort7;

		HRESULT hr = (*D3DDeviceInterface)->GetViewport(&ViewPort7);

		ConvertViewport(*lpData, ViewPort7);

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
			LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ((lpData) ? lpData->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no D3DirectDevice interface!");
			return DDERR_GENERIC;
		}

		if (lpData->dwSize > 44 && (lpData->dvScaleX != 0 || lpData->dvScaleY != 0))
		{
			LOG_LIMIT(100, __FUNCTION__ << " 'Scale homogeneous' Not Implemented");
		}

		D3DVIEWPORT7 ViewPort7;

		ConvertViewport(ViewPort7, *lpData);		

		return (*D3DDeviceInterface)->SetViewport(&ViewPort7);
	}

	return ProxyInterface->SetViewport(lpData);
}

HRESULT m_IDirect3DViewportX::TransformVertices(DWORD dwVertexCount, LPD3DTRANSFORMDATA lpData, DWORD dwFlags, LPDWORD lpOffscreen)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->TransformVertices(dwVertexCount, lpData, dwFlags, lpOffscreen);
}

HRESULT m_IDirect3DViewportX::LightElements(DWORD dwElementCount, LPD3DLIGHTDATA lpData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->LightElements(dwElementCount, lpData);
}

HRESULT m_IDirect3DViewportX::SetBackground(D3DMATERIALHANDLE hMat)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->SetBackground(hMat);
}

HRESULT m_IDirect3DViewportX::GetBackground(LPD3DMATERIALHANDLE lphMat, LPBOOL lpValid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->GetBackground(lphMat, lpValid);
}

HRESULT m_IDirect3DViewportX::SetBackgroundDepth(LPDIRECTDRAWSURFACE lpDDSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
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
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
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
		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no D3DirectDevice interface!");
			return DDERR_GENERIC;
		}

		return (*D3DDeviceInterface)->Clear(dwCount, lpRects, dwFlags, 0, 0, 0);
	}

	return ProxyInterface->Clear(dwCount, lpRects, dwFlags);
}

HRESULT m_IDirect3DViewportX::AddLight(LPDIRECT3DLIGHT lpDirect3DLight)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
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
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpDirect3DLight)
	{
		lpDirect3DLight->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DLight);
	}

	return ProxyInterface->DeleteLight(lpDirect3DLight);
}

HRESULT m_IDirect3DViewportX::NextLight(LPDIRECT3DLIGHT lpDirect3DLight, LPDIRECT3DLIGHT * lplpDirect3DLight, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
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
		if (!lpData)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no D3DirectDevice interface!");
			return DDERR_GENERIC;
		}

		D3DVIEWPORT7 ViewPort7;

		HRESULT hr = (*D3DDeviceInterface)->GetViewport(&ViewPort7);

		ConvertViewport(*lpData, ViewPort7);

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
			LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ((lpData) ? lpData->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no D3DirectDevice interface!");
			return DDERR_GENERIC;
		}

		if (lpData->dwSize > 44 && (lpData->dvClipX != 0 || lpData->dvClipY != 0))
		{
			LOG_LIMIT(100, __FUNCTION__ << " 'clip volume' Not Implemented");
		}

		D3DVIEWPORT7 ViewPort7;

		ConvertViewport(ViewPort7, *lpData);

		return (*D3DDeviceInterface)->SetViewport(&ViewPort7);
	}

	return ProxyInterface->SetViewport2(lpData);
}

HRESULT m_IDirect3DViewportX::SetBackgroundDepth2(LPDIRECTDRAWSURFACE4 lpDDS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpDDS)
	{
		lpDDS->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDS);
	}

	return ProxyInterface->SetBackgroundDepth2(lpDDS);
}

HRESULT m_IDirect3DViewportX::GetBackgroundDepth2(LPDIRECTDRAWSURFACE4 * lplpDDS, LPBOOL lpValid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
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

void m_IDirect3DViewportX::InitViewport()
{
	WrapperInterface = new m_IDirect3DViewport((LPDIRECT3DVIEWPORT)ProxyInterface, this);
	WrapperInterface2 = new m_IDirect3DViewport2((LPDIRECT3DVIEWPORT2)ProxyInterface, this);
	WrapperInterface3 = new m_IDirect3DViewport3((LPDIRECT3DVIEWPORT3)ProxyInterface, this);
}

void m_IDirect3DViewportX::ReleaseViewport()
{
	WrapperInterface->DeleteMe();
	WrapperInterface2->DeleteMe();
	WrapperInterface3->DeleteMe();
}
