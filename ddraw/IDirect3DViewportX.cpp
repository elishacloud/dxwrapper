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

	DWORD DxVersion = (CheckWrapperType(riid) && Config.Dd7to9) ? GetGUIDVersion(riid) : DirectXVersion;

	if (riid == GetWrapperType(DxVersion) || riid == IID_IUnknown)
	{
		*ppvObj = GetWrapperInterfaceX(DxVersion);

		AddRef(DxVersion);

		return D3D_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DirectXVersion));
}

ULONG m_IDirect3DViewportX::AddRef(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") v" << DirectXVersion;

	if (Config.Dd7to9)
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

	if (Config.Dd7to9)
	{
		ULONG ref;

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

		return ref;
	}

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

// ******************************
// IDirect3DViewport v1 functions
// ******************************

HRESULT m_IDirect3DViewportX::Initialize(LPDIRECT3D lpDirect3D)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
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

	if (Config.Dd7to9)
	{
		if (!lpData || lpData->dwSize != sizeof(D3DVIEWPORT))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ((lpData) ? lpData->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		if (IsViewPortSet)
		{
			*lpData = vData;
		}
		else if (IsViewPort2Set)
		{
			ConvertViewport(*lpData, vData2);
		}
		else if (!AttachedD3DDevices.empty())
		{
			D3DVIEWPORT7 Viewport7 = {};

			for (auto& entry : AttachedD3DDevices)
			{
				entry->GetDefaultViewport(*(D3DVIEWPORT9*)&Viewport7);

				ConvertViewport(*lpData, Viewport7);

				break;
			}
		}
		else if (D3DInterface)
		{
			DWORD Width = 0, Height = 0;

			D3DInterface->GetViewportResolution(Width, Height);

			D3DVIEWPORT7 Viewport7 = {
				0,           // X (starting X coordinate)
				0,           // Y (starting Y coordinate)
				Width,       // Width (usually set to the backbuffer width)
				Height,      // Height (usually set to the backbuffer height)
				0.0f,        // MinZ (near clipping plane, typically 0.0f)
				1.0f         // MaxZ (far clipping plane, typically 1.0f)
			};

			ConvertViewport(*lpData, Viewport7);
		}
		else
		{
			return D3DERR_VIEWPORTHASNODEVICE;
		}

		return DD_OK;
	}

	return ProxyInterface->GetViewport(lpData);
}

HRESULT m_IDirect3DViewportX::SetViewport(LPD3DVIEWPORT lpData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
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
		SetCurrentViewportActive(true, false, false);

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

	if (Config.Dd7to9)
	{
		if (dwVertexCount == 0)
		{
			return DD_OK;
		}

		if (!lpData || !lpData->lpIn || !lpData->lpOut)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check dwSize parameters
		if (lpData->dwSize != sizeof(D3DTRANSFORMDATA) ||
			lpData->dwInSize != sizeof(D3DLVERTEX) ||
			lpData->dwOutSize != sizeof(D3DTLVERTEX))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: dwSize doesn't match: " <<
				sizeof(D3DTRANSFORMDATA) << " -> " << lpData->dwSize <<
				" dwInSize: " << sizeof(D3DLVERTEX) << " -> " << lpData->dwInSize <<
				" dwOutSize: " << sizeof(D3DTLVERTEX) << " -> " << lpData->dwOutSize);
			return DDERR_INVALIDPARAMS;
		}

		if (dwFlags & D3DTRANSFORM_CLIPPED)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: D3DTRANSFORM_CLIPPED is ignored");
		}

		// D3DTRANSFORM_UNCLIPPED: flag can be safily ignored

		if (AttachedD3DDevices.empty())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no D3Ddevice attached!");
			return D3DERR_VIEWPORTHASNODEVICE;
		}

		m_IDirect3DDeviceX* pDirect3DDeviceX = AttachedD3DDevices.front();
		if (!pDirect3DDeviceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get Direct3DDeviceX interface!");
			return DDERR_GENERIC;
		}

		if (AttachedD3DDevices.size() > 1)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: More than one attached Direct3DDeviceX interface!");
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		D3DMATRIX matWorld, matView, matProj;
		if (FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld)) ||
			FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_VIEW, &matView)) ||
			FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get transform matrices");
			return DDERR_GENERIC;
		}

		D3DMATRIX matWorldView = {}, matWorldViewProj = {};
		D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);
		D3DXMatrixMultiply(&matWorldViewProj, &matWorldView, &matProj);

		LONG minX = LONG_MAX;
		LONG minY = LONG_MAX;
		LONG maxX = LONG_MIN;
		LONG maxY = LONG_MIN;

		D3DLVERTEX* pIn = reinterpret_cast<D3DLVERTEX*>(lpData->lpIn);
		D3DTLVERTEX* pOut = reinterpret_cast<D3DTLVERTEX*>(lpData->lpOut);
		D3DHVERTEX* pHOut = reinterpret_cast<D3DHVERTEX*>(lpData->lpHOut);

		for (DWORD i = 0; i < dwVertexCount; ++i)
		{
			D3DLVERTEX& src = pIn[i];
			D3DTLVERTEX& dst = pOut[i];

			D3DXVECTOR4 pos(src.x, src.y, src.z, 1.0f);
			D3DXVECTOR4 result;
			D3DXVec4Transform(&result, &pos, &matWorldViewProj);

			dst.sx = result.x / result.w;
			dst.sy = result.y / result.w;
			dst.sz = result.z / result.w;
			dst.rhw = 1.0f / result.w;

			dst.color = src.color;
			dst.specular = src.specular;
			dst.tu = src.tu;
			dst.tv = src.tv;

			if (pHOut)
			{
				D3DHVERTEX& hdst = pHOut[i];
				hdst.hx = result.x;
				hdst.hy = result.y;
				hdst.hz = result.z;
				hdst.dwFlags = 0;	// No clip flags computed
			}

			minX = min(minX, static_cast<LONG>(floor(dst.sx)));
			minY = min(minY, static_cast<LONG>(floor(dst.sy)));
			maxX = max(maxX, static_cast<LONG>(ceil(dst.sx)));
			maxY = max(maxY, static_cast<LONG>(ceil(dst.sy)));
		}

		lpData->drExtent.x1 = minX;
		lpData->drExtent.y1 = minY;
		lpData->drExtent.x2 = maxX;
		lpData->drExtent.y2 = maxY;

		//Address of a variable that is set to a nonzero value if the resulting vertices are all off-screen.
		if (lpOffscreen)
		{
			*lpOffscreen = 0;
		}

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)DD_OK << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

		return DD_OK;
	}

	return ProxyInterface->TransformVertices(dwVertexCount, lpData, dwFlags, lpOffscreen);
}

HRESULT m_IDirect3DViewportX::LightElements(DWORD dwElementCount, LPD3DLIGHTDATA lpData, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (DirectXVersion != 1)
		{
			// This method is only implemented in DirectX2, in Viewport v1
			return DDERR_UNSUPPORTED;
		}

		if (dwElementCount == 0)
		{
			return DD_OK;
		}

		if (!lpData || !lpData->lpIn || !lpData->lpOut)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Validate sizes
		if (lpData->dwSize != sizeof(D3DLIGHTDATA) ||
			lpData->dwInSize != sizeof(D3DLIGHTINGELEMENT) ||
			lpData->dwOutSize != sizeof(D3DTLVERTEX))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: dwSize doesn't match: " <<
				sizeof(D3DLIGHTDATA) << " -> " << lpData->dwSize <<
				" dwInSize: " << sizeof(D3DLIGHTINGELEMENT) << " -> " << lpData->dwInSize <<
				" dwOutSize: " << sizeof(D3DTLVERTEX) << " -> " << lpData->dwOutSize);
			return DDERR_INVALIDPARAMS;
		}

		if (AttachedD3DDevices.empty())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no D3Ddevice attached!");
			return D3DERR_VIEWPORTHASNODEVICE;
		}

		m_IDirect3DDeviceX* pDirect3DDeviceX = AttachedD3DDevices.front();
		if (!pDirect3DDeviceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get Direct3DDeviceX interface!");
			return DDERR_GENERIC;
		}

		if (AttachedD3DDevices.size() > 1)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: More than one attached Direct3DDeviceX interface!");
		}

		LOG_LIMIT(100, __FUNCTION__ << " Warning: emulating LightElements()!");

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		D3DMATRIX matWorld, matView;
		if (FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld)) ||
			FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_VIEW, &matView)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get transform matrices");
			return DDERR_GENERIC;
		}

		D3DMATRIX matWorldView = {};
		D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);

		// Get materal for specular
		bool UseMaterial = false;
		D3DMATERIAL7 mat = {};
		if (SUCCEEDED(pDirect3DDeviceX->GetMaterial(&mat)))
		{
			UseMaterial = true;
		}

		// Cache light data once
		std::vector<D3DLIGHT> cachedLights;
		GetAttachedLights(cachedLights, pDirect3DDeviceX);

		if (cachedLights.empty())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: no attached lights found!");
		}

		for (DWORD i = 0; i < dwElementCount; ++i)
		{
			D3DLIGHTINGELEMENT& src = lpData->lpIn[i];
			D3DTLVERTEX& dst = lpData->lpOut[i];

			D3DXVECTOR3& srcPosition = *reinterpret_cast<D3DXVECTOR3*>(&src.dvPosition);
			D3DXVECTOR3& srcNormal = *reinterpret_cast<D3DXVECTOR3*>(&src.dvNormal);

			ComputeLightColor(dst.color, dst.specular, srcPosition, srcNormal, cachedLights, matWorldView, matWorld, matView, mat, UseMaterial);

			dst.sx = src.dvPosition.x;
			dst.sy = src.dvPosition.y;
			dst.sz = src.dvPosition.z;
			dst.rhw = 1.0f;		// Assumes pre-transformed
			dst.tu = 0;
			dst.tv = 0;
		}

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)DD_OK << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

		return DD_OK;
	}

	return ProxyInterface->LightElements(dwElementCount, lpData);
}

HRESULT m_IDirect3DViewportX::SetBackground(D3DMATERIALHANDLE hMat)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		MaterialBackground.IsSet = TRUE;
		MaterialBackground.hMat = hMat;

		// If current viewport is set then use new viewport
		SetCurrentViewportActive(false, true, false);

		return D3D_OK;
	}

	return ProxyInterface->SetBackground(hMat);
}

HRESULT m_IDirect3DViewportX::GetBackground(LPD3DMATERIALHANDLE lphMat, LPBOOL lpValid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
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

	if (Config.Dd7to9)
	{
		return SetBackgroundDepth2(reinterpret_cast<LPDIRECTDRAWSURFACE4>(lpDDSurface));
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

	if (Config.Dd7to9)
	{
		return GetBackgroundDepth2(reinterpret_cast<LPDIRECTDRAWSURFACE4*>(lplpDDSurface), lpValid, 1);
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

	if (Config.Dd7to9)
	{
		// The requested operation could not be completed because the viewport has not yet been associated with a device.
		HRESULT hr = D3DERR_VIEWPORTHASNODEVICE;

		// ToDo: check on zbuffer and return error if does not exist:  D3DERR_ZBUFFER_NOTPRESENT

		for (auto& entry : AttachedD3DDevices)
		{
			hr = entry->Clear(dwCount, lpRects, dwFlags, 0x00000000, 1.0f, 0);

			if (FAILED(hr))
			{
				return hr;
			}
		}

		return hr;
	}

	return ProxyInterface->Clear(dwCount, lpRects, dwFlags);
}

HRESULT m_IDirect3DViewportX::AddLight(LPDIRECT3DLIGHT lpDirect3DLight)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// This method will fail, returning DDERR_INVALIDPARAMS, if you attempt to add a light that has already been assigned to the viewport.
		if (!lpDirect3DLight || IsLightAttached(lpDirect3DLight))
		{
			return DDERR_INVALIDPARAMS;
		}

		// If current viewport is set then use new light
		for (auto& entry : AttachedD3DDevices)
		{
			if (entry->CheckIfViewportSet(this))
			{
				D3DLIGHT2 Light2 = {};
				Light2.dwSize = sizeof(D3DLIGHT2);
				if (FAILED(lpDirect3DLight->GetLight((LPD3DLIGHT)&Light2)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not get light!");
					return DDERR_GENERIC;
				}
				Light2.dwFlags |= D3DLIGHT_ACTIVE;
				if (FAILED(entry->SetLight((m_IDirect3DLight*)lpDirect3DLight, (LPD3DLIGHT)&Light2)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not set light!");
					return DDERR_GENERIC;
				}
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

	if (Config.Dd7to9)
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
		for (auto& entry : AttachedD3DDevices)
		{
			if (entry->CheckIfViewportSet(this))
			{
				D3DLIGHT2 Light2 = {};
				Light2.dwSize = sizeof(D3DLIGHT2);
				if (FAILED(lpDirect3DLight->GetLight((LPD3DLIGHT)&Light2)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: could not get light!");
				}
				Light2.dwFlags &= ~D3DLIGHT_ACTIVE;
				if (FAILED(entry->SetLight((m_IDirect3DLight*)lpDirect3DLight, (LPD3DLIGHT)&Light2)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: could not set light!");
				}
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

	if (Config.Dd7to9)
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

// ******************************
// IDirect3DViewport v2 functions
// ******************************

HRESULT m_IDirect3DViewportX::GetViewport2(LPD3DVIEWPORT2 lpData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpData || lpData->dwSize != sizeof(D3DVIEWPORT2))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ((lpData) ? lpData->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		if (IsViewPort2Set)
		{
			*lpData = vData2;
		}
		else if (IsViewPortSet)
		{
			ConvertViewport(*lpData, vData);
		}
		else if (!AttachedD3DDevices.empty())
		{
			D3DVIEWPORT7 Viewport7 = {};

			for (auto& entry : AttachedD3DDevices)
			{
				entry->GetDefaultViewport(*(D3DVIEWPORT9*)&Viewport7);

				ConvertViewport(*lpData, Viewport7);

				break;
			}
		}
		else if (D3DInterface)
		{
			DWORD Width = 0, Height = 0;

			D3DInterface->GetViewportResolution(Width, Height);

			D3DVIEWPORT7 Viewport7 = {
				0,           // X (starting X coordinate)
				0,           // Y (starting Y coordinate)
				Width,       // Width (usually set to the backbuffer width)
				Height,      // Height (usually set to the backbuffer height)
				0.0f,        // MinZ (near clipping plane, typically 0.0f)
				1.0f         // MaxZ (far clipping plane, typically 1.0f)
			};

			ConvertViewport(*lpData, Viewport7);
		}
		else
		{
			return D3DERR_VIEWPORTHASNODEVICE;
		}

		return DD_OK;
	}

	return ProxyInterface->GetViewport2(lpData);
}

HRESULT m_IDirect3DViewportX::SetViewport2(LPD3DVIEWPORT2 lpData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
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
		SetCurrentViewportActive(true, false, false);

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

// ******************************
// IDirect3DViewport v3 functions
// ******************************

HRESULT m_IDirect3DViewportX::SetBackgroundDepth2(LPDIRECTDRAWSURFACE4 lpDDS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Sets the background-depth field (which is a depth buffer surface) for the viewport.
		// The depth-buffer is filled with the specified depth field when the IDirect3DViewport3::Clear or
		// IDirect3DViewport3::Clear2 methods are called with the D3DCLEAR_ZBUFFER flag is specified.

		if (!lpDDS)
		{
			return DDERR_INVALIDPARAMS;
		}

		m_IDirectDrawSurfaceX* lpSurfaceX = nullptr;
		if (FAILED(lpDDS->QueryInterface(IID_GetInterfaceX, reinterpret_cast<LPVOID*>(&lpSurfaceX))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get SurfaceX interface!");
			return DDERR_GENERIC;
		}

		if (!lpSurfaceX->IsDepthStencil())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not depth stencil!");
			return DDERR_INVALIDPARAMS;
		}

		LOG_LIMIT(100, __FUNCTION__ << " Warning: background depth stencil surface not fully implemented!");

		pBackgroundDepthSurfaceX = lpSurfaceX;

		return DD_OK;
	}

	if (lpDDS)
	{
		lpDDS->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDS);
	}

	return ProxyInterface->SetBackgroundDepth2(lpDDS);
}

HRESULT m_IDirect3DViewportX::GetBackgroundDepth2(LPDIRECTDRAWSURFACE4* lplpDDS, LPBOOL lpValid, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (lpValid)
		{
			*lpValid = FALSE;
		}

		if (!lplpDDS)
		{
			return DDERR_INVALIDPARAMS;
		}
		*lplpDDS = nullptr;

		if (!pBackgroundDepthSurfaceX)
		{
			return DD_OK;
		}

		*lplpDDS = reinterpret_cast<LPDIRECTDRAWSURFACE4>(pBackgroundDepthSurfaceX->GetWrapperInterfaceX(DirectXVersion));

		if (lpValid && *lplpDDS)
		{
			*lpValid = TRUE;
		}

		return DD_OK;
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

	if (Config.Dd7to9)
	{
		// The requested operation could not be completed because the viewport has not yet been associated with a device.
		HRESULT hr = D3DERR_VIEWPORTHASNODEVICE;

		// ToDo: check on zbuffer and stencil buffer and return error if does not exist:  D3DERR_ZBUFFER_NOTPRESENT and D3DERR_STENCILBUFFER_NOTPRESENT

		// For now just hard code this to 1.0f rather than copying the depth stencil buffer
		if ((dwFlags & D3DCLEAR_ZBUFFER) && pBackgroundDepthSurfaceX)
		{
			dvZ = 1.0f;
		}

		for (auto& entry : AttachedD3DDevices)
		{
			hr = entry->Clear(dwCount, lpRects, dwFlags, dwColor, dvZ, dwStencil);

			if (FAILED(hr))
			{
				return hr;
			}
		}

		return hr;
	}

	return ProxyInterface->Clear2(dwCount, lpRects, dwFlags, dwColor, dvZ, dwStencil);
}

// ******************************
// Helper functions
// ******************************

void m_IDirect3DViewportX::InitInterface(DWORD DirectXVersion)
{
	if (D3DInterface)
	{
		D3DInterface->AddViewport(this);
	}

	if (Config.Dd7to9)
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
	SaveInterfaceAddress(WrapperInterface);
	SaveInterfaceAddress(WrapperInterface2);
	SaveInterfaceAddress(WrapperInterface3);

	for (auto& entry : AttachedD3DDevices)
	{
		entry->ClearViewport(this);
	}
}

void* m_IDirect3DViewportX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 0:
		if (WrapperInterface3) return WrapperInterface3;
		if (WrapperInterface2) return WrapperInterface2;
		if (WrapperInterface) return WrapperInterface;
		break;
	case 1:
		return GetInterfaceAddress(WrapperInterface, (LPDIRECT3DVIEWPORT)ProxyInterface, this);
	case 2:
		return GetInterfaceAddress(WrapperInterface2, (LPDIRECT3DVIEWPORT2)ProxyInterface, this);
	case 3:
		return GetInterfaceAddress(WrapperInterface3, (LPDIRECT3DVIEWPORT3)ProxyInterface, this);
	}
	LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
	return nullptr;
}

void m_IDirect3DViewportX::SetCurrentViewportActive(bool SetViewPortData, bool SetBackgroundData, bool SetLightData)
{
	for (auto& D3DDevice : AttachedD3DDevices)
	{
		if (D3DDevice->CheckIfViewportSet(this))
		{
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
				if (FAILED(D3DDevice->SetViewport(&Viewport7)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: failed to set viewport data!");
				}
			}

			if (SetBackgroundData && MaterialBackground.IsSet)
			{
				if (FAILED(D3DDevice->SetLightState(D3DLIGHTSTATE_MATERIAL, MaterialBackground.hMat)))
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
					if (FAILED(D3DDevice->SetLight((m_IDirect3DLight*)entry, (LPD3DLIGHT)&Light2)))
					{
						LOG_LIMIT(100, __FUNCTION__ << " Warning: could not set light!");
					}
				}
			}
		}
	}
}

void m_IDirect3DViewportX::GetAttachedLights(std::vector<D3DLIGHT>& AttachedLightList, m_IDirect3DDeviceX* pDirect3DDeviceX)
{
	if (!pDirect3DDeviceX)
	{
		return;
	}

	for (auto& entry : AttachedLights)
	{
		// Get light data
		D3DLIGHT light = {};
		if (SUCCEEDED(entry->GetLight(&light)))
		{
			m_IDirect3DLight* pLightX = reinterpret_cast<m_IDirect3DLight*>(entry);

			// Check if light is enabled
			BOOL Enable = FALSE;
			if (SUCCEEDED(pDirect3DDeviceX->GetLightEnable(pLightX, &Enable)) && Enable)
			{
				AttachedLightList.push_back(light);
				continue;
			}
		}
	}
}

void m_IDirect3DViewportX::ComputeLightColor(D3DCOLOR& outColor, D3DCOLOR& outSpecular, const D3DXVECTOR3& Position, const D3DXVECTOR3& Normal, const std::vector<D3DLIGHT>& cachedLights, const D3DXMATRIX& matWorldView, const D3DMATRIX& matWorld, const D3DMATRIX& matView, const D3DMATERIAL7& mat, bool UseMaterial)
{
	// Transform position using full world-view matrix (this is fine)
	D3DXVECTOR3 worldPos;
	D3DXVec3TransformCoord(&worldPos, &Position, &matWorldView);

	// Now transform the normal using only the world matrix (like DX2 behavior)
	D3DXMATRIX matWorldRotOnly = matWorld;
	matWorldRotOnly._41 = 0.0f;
	matWorldRotOnly._42 = 0.0f;
	matWorldRotOnly._43 = 0.0f;

	// Clear perspective components
	matWorldRotOnly._14 = 0.0f;
	matWorldRotOnly._24 = 0.0f;
	matWorldRotOnly._34 = 0.0f;
	matWorldRotOnly._44 = 1.0f;

	D3DXVECTOR3 worldNormal;
	D3DXVec3TransformNormal(&worldNormal, &Normal, &matWorldRotOnly);
	D3DXVec3Normalize(&worldNormal, &worldNormal);

	D3DCOLORVALUE diffuse = { 0, 0, 0, 0 };
	D3DCOLORVALUE specular = { 0, 0, 0, 0 };

	for (const auto& light : cachedLights)
	{
		const D3DXVECTOR3& lightPos = *reinterpret_cast<const D3DXVECTOR3*>(&light.dvPosition);
		const D3DXVECTOR3& lightDir = *reinterpret_cast<const D3DXVECTOR3*>(&light.dvDirection);

		D3DXVECTOR3 toLight;
		float dist = 1.0f;
		float attenuation = 1.0f;
		float denom = 1.0f;

		switch (light.dltType)
		{
		case D3DLIGHT_DIRECTIONAL:
			toLight = -lightDir;
			D3DXVec3Normalize(&toLight, &toLight);
			break;

		case D3DLIGHT_POINT:
		case D3DLIGHT_SPOT:
			toLight = lightPos - worldPos;
			dist = D3DXVec3Length(&toLight);
			if (dist == 0.0f) continue;
			toLight /= dist;

			denom = light.dvAttenuation0 +
				light.dvAttenuation1 * dist +
				light.dvAttenuation2 * dist * dist;

			if (denom <= 0.0f) continue;  // Skip bad light

			attenuation = 1.0f / denom;

			// Handle range cutoff
			if (light.dvRange > 0.0f && dist > light.dvRange)
			{
				continue;
			}

			// Spotlight falloff
			if (light.dltType == D3DLIGHT_SPOT)
			{
				D3DXVECTOR3 toLightNeg = -toLight;
				float spotCos = D3DXVec3Dot(&toLightNeg, &lightDir);
				spotCos = max(-1.0f, min(1.0f, spotCos));	// Clamp spotCos to [-1, 1]
				float cosPhi = cosf(light.dvPhi * 0.5f);
				if (spotCos < cosPhi) continue;

				float cosTheta = cosf(light.dvTheta * 0.5f);
				if (spotCos >= cosTheta)
				{
					// full light
				}
				else
				{
					float falloff = powf((spotCos - cosPhi) / (cosTheta - cosPhi), max(light.dvFalloff, 1.0f));  // Clamp falloff to minimum 1.0
					attenuation *= falloff;
				}
			}
			break;

		default:
			continue; // unsupported light type
		}

		float NdotL = max(0.0f, D3DXVec3Dot(&worldNormal, &toLight));
		if (NdotL <= 0.0f) continue;

		// Clamp attenuation to [0,1]
		attenuation = min(max(attenuation, 0.0f), 1.0f);

		// Diffuse lighting
		diffuse.r += light.dcvColor.r * NdotL * attenuation;
		diffuse.g += light.dcvColor.g * NdotL * attenuation;
		diffuse.b += light.dcvColor.b * NdotL * attenuation;

		// Specular lighting
		if (UseMaterial)
		{
			// Transform position to view space
			D3DXVECTOR3 viewPos;
			D3DXVec3TransformCoord(&viewPos, &Position, &matView);

			// View direction from fragment to camera
			D3DXVECTOR3 viewDir = -viewPos;
			D3DXVec3Normalize(&viewDir, &viewDir);

			D3DXVec3Normalize(&toLight, &toLight);

			D3DXVECTOR3 reflectDir = worldNormal * 2.0f * NdotL - toLight;
			D3DXVec3Normalize(&reflectDir, &reflectDir);

			float RdotV = max(0.0f, D3DXVec3Dot(&reflectDir, &viewDir));
			float shininess = UseMaterial ? mat.power : 4.0f;
			float spec = powf(RdotV, shininess) * attenuation;

			specular.r += mat.specular.r * spec;
			specular.g += mat.specular.g * spec;
			specular.b += mat.specular.b * spec;
		}
	}

	// Add material ambient color
	if (UseMaterial)
	{
		diffuse.r += mat.diffuse.r * mat.ambient.r;
		diffuse.g += mat.diffuse.g * mat.ambient.g;
		diffuse.b += mat.diffuse.b * mat.ambient.b;
	}

	float alpha = UseMaterial ? mat.diffuse.a : 1.0f;

	// Clamp and convert to DWORD color
	outColor = D3DCOLOR_COLORVALUE(
		min(max(diffuse.r, 0.0f), 1.0f),
		min(max(diffuse.g, 0.0f), 1.0f),
		min(max(diffuse.b, 0.0f), 1.0f),
		alpha);

	// Clamp and convert to DWORD specular
	outSpecular = D3DCOLOR_COLORVALUE(
		min(max(specular.r, 0.0f), 1.0f),
		min(max(specular.g, 0.0f), 1.0f),
		min(max(specular.b, 0.0f), 1.0f),
		1.0f);
}

void m_IDirect3DViewportX::ClearSurface(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (lpSurfaceX == pBackgroundDepthSurfaceX)
	{
		pBackgroundDepthSurfaceX = nullptr;
	}
}

void m_IDirect3DViewportX::AddD3DDevice(m_IDirect3DDeviceX* lpD3DDevice)
{
	if (!lpD3DDevice)
	{
		return;
	}
	auto it = std::find(AttachedD3DDevices.begin(), AttachedD3DDevices.end(), lpD3DDevice);
	if (it != AttachedD3DDevices.end())
	{
		return;
	}

	AttachedD3DDevices.push_back(lpD3DDevice);
}

void m_IDirect3DViewportX::ClearD3DDevice(m_IDirect3DDeviceX* lpD3DDevice)
{
	// Find and remove the D3D device from the list
	auto it = std::find(AttachedD3DDevices.begin(), AttachedD3DDevices.end(), lpD3DDevice);
	if (it != AttachedD3DDevices.end())
	{
		AttachedD3DDevices.erase(it);
	}
}
