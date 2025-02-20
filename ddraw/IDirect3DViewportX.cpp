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

// Cached wrapper interface
namespace {
	m_IDirect3DViewport* WrapperInterfaceBackup = nullptr;
	m_IDirect3DViewport2* WrapperInterfaceBackup2 = nullptr;
	m_IDirect3DViewport3* WrapperInterfaceBackup3 = nullptr;
}

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

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DirectXVersion));
}

void *m_IDirect3DViewportX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 0:
		if (WrapperInterface3) return WrapperInterface3;
		if (WrapperInterface2) return WrapperInterface2;
		if (WrapperInterface) return WrapperInterface;
		break;
	case 1:
		return GetInterfaceAddress(WrapperInterface, WrapperInterfaceBackup, (LPDIRECT3DVIEWPORT)ProxyInterface, this);
	case 2:
		return GetInterfaceAddress(WrapperInterface2, WrapperInterfaceBackup2, (LPDIRECT3DVIEWPORT2)ProxyInterface, this);
	case 3:
		return GetInterfaceAddress(WrapperInterface3, WrapperInterfaceBackup3, (LPDIRECT3DVIEWPORT3)ProxyInterface, this);
	}
	LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
	return nullptr;
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
			if (AttachedLights.size())
			{
				Logging::Log() << __FUNCTION__ << " Warning: releasing Viewport while lights are still attached! " << AttachedLights.size();
			}
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
			// Check for device interface
			if (FAILED(CheckInterface(__FUNCTION__)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get the D3DDevice!");
				return DDERR_GENERIC;
			}

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

		if (lpData->dvScaleX != 0 || lpData->dvScaleY != 0 || lpData->dvMaxX != 0 || lpData->dvMaxY != 0)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'Scale homogeneous' Not Implemented: " <<
				" ScaleX: " << lpData->dvScaleX << " ScaleY: " << lpData->dvScaleY << " MaxX: " << lpData->dvMaxX << " MaxY: " << lpData->dvMaxY);
		}

		IsViewPortSet = true;

		vData = *lpData;

		// If current viewport is set then use new viewport
		if (SUCCEEDED(CheckInterface(__FUNCTION__)) && (*D3DDeviceInterface)->CheckIfViewportSet(this))
		{
			SetCurrentViewportActive(true, false, false);
		}

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
		MaterialBackground.IsSet = TRUE;
		MaterialBackground.hMat = hMat;

		// If current viewport is set then use new viewport
		if (SUCCEEDED(CheckInterface(__FUNCTION__)) && (*D3DDeviceInterface)->CheckIfViewportSet(this))
		{
			SetCurrentViewportActive(false, true, false);
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
		// Sets the background-depth field for the viewport.
		// The depth-buffer is filled with the specified depth field when the IDirect3DViewport3::Clear method is called
		// and the D3DCLEAR_ZBUFFER flag is specified. The bit depth must be 16 bits.
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
		// This method will fail, returning DDERR_INVALIDPARAMS, if you attempt to add a light that has already been assigned to the viewport.
		if (!lpDirect3DLight || IsLightAttached(lpDirect3DLight))
		{
			return DDERR_INVALIDPARAMS;
		}

		// If current viewport is set then use new light
		if (SUCCEEDED(CheckInterface(__FUNCTION__)) && (*D3DDeviceInterface)->CheckIfViewportSet(this))
		{
			D3DLIGHT2 Light2 = {};
			Light2.dwSize = sizeof(D3DLIGHT2);
			if (FAILED(lpDirect3DLight->GetLight((LPD3DLIGHT)&Light2)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get light!");
				return DDERR_GENERIC;
			}
			Light2.dwFlags |= D3DLIGHT_ACTIVE;
			if (FAILED((*D3DDeviceInterface)->SetLight((m_IDirect3DLight*)lpDirect3DLight, (LPD3DLIGHT)&Light2)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not set light!");
				return DDERR_GENERIC;
			}
		}

		AttachedLights.push_back(lpDirect3DLight);

		lpDirect3DLight->AddRef();

		return D3D_OK;
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
		if (!lpDirect3DLight)
		{
			return DDERR_INVALIDPARAMS;
		}

		bool ret = DeleteAttachedLight(lpDirect3DLight);

		if (!ret)
		{
			return DDERR_INVALIDPARAMS;
		}

		lpDirect3DLight->Release();

		// If current viewport is then deactivate the light
		if (SUCCEEDED(CheckInterface(__FUNCTION__)) && (*D3DDeviceInterface)->CheckIfViewportSet(this))
		{
			D3DLIGHT2 Light2 = {};
			Light2.dwSize = sizeof(D3DLIGHT2);
			if (FAILED(lpDirect3DLight->GetLight((LPD3DLIGHT)&Light2)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: could not get light!");
			}
			Light2.dwFlags &= ~D3DLIGHT_ACTIVE;
			if (FAILED((*D3DDeviceInterface)->SetLight((m_IDirect3DLight*)lpDirect3DLight, (LPD3DLIGHT)&Light2)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: could not set light!");
			}
		}

		return D3D_OK;
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
		if (!lplpDirect3DLight || (dwFlags == D3DNEXT_NEXT && !lpDirect3DLight))
		{
			return DDERR_INVALIDPARAMS;
		}

		*lplpDirect3DLight = nullptr;

		if (AttachedLights.size() == 0)
		{
			return D3DERR_NOVIEWPORTS;
		}

		switch (dwFlags)
		{
		case D3DNEXT_HEAD:
			// Retrieve the item at the beginning of the list.
			*lplpDirect3DLight = AttachedLights.front();
			break;
		case D3DNEXT_TAIL:
			// Retrieve the item at the end of the list.
			*lplpDirect3DLight = AttachedLights.back();
			break;
		case D3DNEXT_NEXT:
			// Retrieve the next item in the list.
			// If you attempt to retrieve the next viewport in the list when you are at the end of the list, this method returns D3D_OK but lplpDirect3DLight is NULL.
			for (UINT x = 1; x < AttachedLights.size(); x++)
			{
				if (AttachedLights[x - 1] == lpDirect3DLight)
				{
					*lplpDirect3DLight = AttachedLights[x];
					break;
				}
			}
			break;
		default:
			return DDERR_INVALIDPARAMS;
			break;
		}

		return D3D_OK;
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
			// Check for device interface
			if (FAILED(CheckInterface(__FUNCTION__)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get the D3DDevice!");
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

		if (lpData->dvClipWidth != 0 || lpData->dvClipHeight != 0 || lpData->dvClipX != 0 || lpData->dvClipY != 0)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'clip volume' Not Implemented: " <<
				lpData->dvClipWidth << "x" << lpData->dvClipHeight << " X: " << lpData->dvClipX << " Y: " << lpData->dvClipY);
		}

		IsViewPort2Set = true;

		vData2 = *lpData;

		// If current viewport is set then use new viewport
		if (SUCCEEDED(CheckInterface(__FUNCTION__)) && (*D3DDeviceInterface)->CheckIfViewportSet(this))
		{
			SetCurrentViewportActive(true, false, false);
		}

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

void m_IDirect3DViewportX::SetCurrentViewportActive(bool SetViewPortData, bool SetBackgroundData, bool SetLightData)
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__)))
	{
		return;
	}

	if (SetViewPortData && (IsViewPortSet || IsViewPort2Set))
	{
		D3DVIEWPORT7 Viewport7 = {};
		if (IsViewPort2Set)
		{
			ConvertViewport(Viewport7, vData2);
		}
		else
		{
			ConvertViewport(Viewport7, vData);
		}
		if (FAILED((*D3DDeviceInterface)->SetViewport(&Viewport7)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: failed to set viewport data!");
		}
	}

	if (SetBackgroundData && MaterialBackground.IsSet)
	{
		if (FAILED((*D3DDeviceInterface)->SetLightState(D3DLIGHTSTATE_MATERIAL, MaterialBackground.hMat)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: failed to set material background!");
		}
	}

	if (SetLightData)
	{
		for (auto& entry : AttachedLights)
		{
			D3DLIGHT2 Light2 = {};
			Light2.dwSize = sizeof(D3DLIGHT2);
			if (FAILED(entry->GetLight((LPD3DLIGHT)&Light2)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: could not get light!");
			}
			Light2.dwFlags |= D3DLIGHT_ACTIVE;
			if (FAILED((*D3DDeviceInterface)->SetLight((m_IDirect3DLight*)entry, (LPD3DLIGHT)&Light2)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: could not set light!");
			}
		}
	}
}

HRESULT m_IDirect3DViewportX::SetBackgroundDepth2(LPDIRECTDRAWSURFACE4 lpDDS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		// Sets the background-depth field for the viewport.
		// The depth-buffer is filled with the specified depth field when the IDirect3DViewport3::Clear or
		// IDirect3DViewport3::Clear2 methods are called with the D3DCLEAR_ZBUFFER flag is specified.
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
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get the D3DDevice!");
			return DDERR_GENERIC;
		}

		return (*D3DDeviceInterface)->Clear(dwCount, lpRects, dwFlags, dwColor, dvZ, dwStencil);
	}

	return ProxyInterface->Clear2(dwCount, lpRects, dwFlags, dwColor, dvZ, dwStencil);
}

/************************/
/*** Helper functions ***/
/************************/

HRESULT m_IDirect3DViewportX::CheckInterface(char* FunctionName)
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

void m_IDirect3DViewportX::InitInterface(DWORD DirectXVersion)
{
	if (D3DInterface)
	{
		D3DInterface->AddViewport(this);
	}

	if (!ProxyInterface)
	{
		AddRef(DirectXVersion);
	}
}

void m_IDirect3DViewportX::ReleaseInterface()
{
	if (Config.Exiting)
	{
		return;
	}

	if (D3DInterface)
	{
		D3DInterface->ClearViewport(this);
	}

	// Don't delete wrapper interface
	SaveInterfaceAddress(WrapperInterface, WrapperInterfaceBackup);
	SaveInterfaceAddress(WrapperInterface2, WrapperInterfaceBackup2);
	SaveInterfaceAddress(WrapperInterface3, WrapperInterfaceBackup3);

	if (D3DDeviceInterface && *D3DDeviceInterface)
	{
		(*D3DDeviceInterface)->ClearViewport(this);
	}
}
