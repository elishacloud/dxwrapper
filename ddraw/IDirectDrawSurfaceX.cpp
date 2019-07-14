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
*
* Code taken from: https://github.com/strangebytes/diablo-ddrawwrapper
*/

#include "ddraw.h"
#include "d3d9ShaderPalette.h"

// Used to allow presenting non-primary surfaces in case the primary surface present fails
bool SceneReady = false;
bool IsPresentRunning = false;

/************************/
/*** IUnknown methods ***/
/************************/

HRESULT m_IDirectDrawSurfaceX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if ((riid == IID_IDirectDrawSurface || riid == IID_IDirectDrawSurface2 || riid == IID_IDirectDrawSurface3 || riid == IID_IDirectDrawSurface4 || riid == IID_IDirectDrawSurface7 || riid == IID_IUnknown) && ppvObj)
		{
			DWORD DxVersion = (riid == IID_IUnknown) ? DirectXVersion : GetIIDVersion(riid);

			*ppvObj = GetWrapperInterfaceX(DxVersion);

			::AddRef(*ppvObj);

			return DD_OK;
		}
		if ((riid == IID_IDirect3DHALDevice || riid == IID_IDirect3DRGBDevice || riid == IID_IDirect3DRampDevice || riid == IID_IDirect3DNullDevice) && ppvObj)
		{
			// Check for device interface
			if (FAILED(CheckInterface(__FUNCTION__, false, false)))
			{
				return DDERR_GENERIC;
			}

			m_IDirect3DDeviceX *D3DDeviceX = *ddrawParent->GetCurrentD3DDevice();

			if (D3DDeviceX)
			{
				*ppvObj = D3DDeviceX->GetWrapperInterfaceX(DirectXVersion);

				::AddRef(*ppvObj);

				return DD_OK;
			}

			m_IDirect3DX *D3DX = *ddrawParent->GetCurrentD3D();

			if (D3DX)
			{
				D3DX->CreateDevice(riid, (LPDIRECTDRAWSURFACE7)this, (LPDIRECT3DDEVICE7*)&D3DDeviceX, nullptr, DirectXVersion);

				*ppvObj = D3DDeviceX;

				return DD_OK;
			}

			LOG_LIMIT(100, __FUNCTION__ << " Query failed for " << riid << " from " << GetWrapperType(DirectXVersion));

			return E_NOINTERFACE;
		}
	}

	if (Config.ConvertToDirect3D7 && (riid == IID_IDirect3DTexture || riid == IID_IDirect3DTexture2) && ppvObj)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, false)))
		{
			return DDERR_GENERIC;
		}

		*ppvObj = new m_IDirect3DTextureX(ddrawParent->GetCurrentD3DDevice(), 7, ProxyInterface);

		return DD_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DirectXVersion), WrapperInterface);
}

void *m_IDirectDrawSurfaceX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 1:
		if (!UniqueProxyInterface.get())
		{
			UniqueProxyInterface = std::make_unique<m_IDirectDrawSurface>(this);
		}
		return UniqueProxyInterface.get();
	case 2:
		if (!UniqueProxyInterface2.get())
		{
			UniqueProxyInterface2 = std::make_unique<m_IDirectDrawSurface2>(this);
		}
		return UniqueProxyInterface2.get();
	case 3:
		if (!UniqueProxyInterface3.get())
		{
			UniqueProxyInterface3 = std::make_unique<m_IDirectDrawSurface3>(this);
		}
		return UniqueProxyInterface3.get();
	case 4:
		if (!UniqueProxyInterface4.get())
		{
			UniqueProxyInterface4 = std::make_unique<m_IDirectDrawSurface4>(this);
		}
		return UniqueProxyInterface4.get();
	case 7:
		if (!UniqueProxyInterface7.get())
		{
			UniqueProxyInterface7 = std::make_unique<m_IDirectDrawSurface7>(this);
		}
		return UniqueProxyInterface7.get();
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return nullptr;
	}
}

ULONG m_IDirectDrawSurfaceX::AddRef()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawSurfaceX::Release()
{
	Logging::LogDebug() << __FUNCTION__;

	ULONG ref;

	if (Config.Dd7to9)
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

/**********************************/
/*** IDirectDrawSurface methods ***/
/**********************************/

HRESULT m_IDirectDrawSurfaceX::AddAttachedSurface(LPDIRECTDRAWSURFACE7 lpDDSurface)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lpDDSurface)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, false)))
		{
			return DDERR_GENERIC;
		}

		m_IDirectDrawSurfaceX *lpAttachedSurface = ((m_IDirectDrawSurface*)lpDDSurface)->GetWrapperInterface();

		if (lpAttachedSurface == this)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: cannot attach self");
			return DDERR_CANNOTATTACHSURFACE;
		}

		if (!ddrawParent->DoesSurfaceExist(lpAttachedSurface))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid surface!");
			return DDERR_INVALIDPARAMS;
		}

		if (DoesAttachedSurfaceExist(lpAttachedSurface))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: attached surface already exists");
			return DDERR_SURFACEALREADYATTACHED;
		}

		DWORD AttachedSurfaceCaps = lpAttachedSurface->GetSurfaceCaps().dwCaps;
		if (!(((AttachedSurfaceCaps & DDSCAPS_BACKBUFFER) && (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER)) ||
			((AttachedSurfaceCaps & DDSCAPS_FRONTBUFFER) && (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_BACKBUFFER)) ||
			((AttachedSurfaceCaps & DDSCAPS_MIPMAP) && (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_MIPMAP))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: cannot attach surface with this method. dwCaps: " << lpAttachedSurface->GetSurfaceCaps().dwCaps);
			return DDERR_CANNOTATTACHSURFACE;
		}

		AddAttachedSurfaceToMap(lpAttachedSurface, true);

		lpAttachedSurface->AddRef();

		return DD_OK;
	}

	if (lpDDSurface)
	{
		lpDDSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDSurface)->GetProxyInterface();
	}

	return ProxyInterface->AddAttachedSurface(lpDDSurface);
}

HRESULT m_IDirectDrawSurfaceX::AddOverlayDirtyRect(LPRECT lpRect)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->AddOverlayDirtyRect(lpRect);
}

HRESULT m_IDirectDrawSurfaceX::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx, BOOL isSkipScene)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Check for required DDBLTFX structure
		if (!lpDDBltFx && (dwFlags & (DDBLT_DDFX | DDBLT_COLORFILL | DDBLT_DEPTHFILL | DDBLT_DDROPS | DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_ROP | DDBLT_ROTATIONANGLE)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: DDBLTFX structure not found");
			return DDERR_INVALIDPARAMS;
		}

		// Check for non-Win32 raster operations flag
		if (dwFlags & DDBLT_DDROPS)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Non-Win32 raster operations Not Implemented ");
			return DDERR_NODDROPSHW;
		}

		// Check for depth fill flag
		if (dwFlags & DDBLT_DEPTHFILL)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Depth Fill Not Implemented");
			return DDERR_NOZBUFFERHW;
		}

		// Check for ZBuffer flags
		if ((dwFlags & DDBLT_ZBUFFER) || ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & (DDBLTFX_ZBUFFERBASEDEST | DDBLTFX_ZBUFFERRANGE))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " ZBuffer Not Implemented");
			return DDERR_NOZBUFFERHW;
		}

		// Check for rotation flags
		if (((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & DDBLTFX_ROTATE180)))
		{
			// 180 degree rotate is the same as rotating both left to right and up to down
			dwFlags |= DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN;
		}
		else if ((dwFlags & DDBLT_ROTATIONANGLE) || ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & (DDBLTFX_ROTATE90 | DDBLTFX_ROTATE270))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Rotation operations Not Implemented");
			return DDERR_NOROTATIONHW;
		}

		// Unused flags (can be safely ignored?)
		// DDBLT_ALPHA
		// DDBLT_ASYNC
		// DDBLT_DONOTWAIT
		// DDBLT_WAIT
		// DDBLTFX_NOTEARING

		HRESULT hr = DD_OK;
		m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = nullptr;
		do {
			IsInBlt = true;

			// Check if the scene needs to be presented
			isSkipScene |= ((lpDestRect) ? (abs(lpDestRect->bottom - lpDestRect->top) < 2 || abs(lpDestRect->right - lpDestRect->left) < 2) : FALSE);

			// Do color fill
			if (dwFlags & DDBLT_COLORFILL)
			{
				hr = ColorFill(lpDestRect, lpDDBltFx->dwFillColor);
				break;
			}

			// Do supported raster operations
			if (dwFlags & DDBLT_ROP)
			{
				if (lpDDBltFx->dwROP == SRCCOPY)
				{
					// Do nothing
				}
				else if (lpDDBltFx->dwROP == BLACKNESS)
				{
					hr = ColorFill(lpDestRect, 0x00000000);
					break;
				}
				else if (lpDDBltFx->dwROP == WHITENESS)
				{
					hr = ColorFill(lpDestRect, 0xFFFFFFFF);
					break;
				}
				else
				{
					LOG_LIMIT(100, __FUNCTION__ << " Raster operation Not Implemented " << lpDDBltFx->dwROP);
					hr = DDERR_NORASTEROPHW;
					break;
				}
			}

			// Get source surface
			lpDDSrcSurfaceX = (m_IDirectDrawSurfaceX*)lpDDSrcSurface;
			if (!lpDDSrcSurfaceX)
			{
				lpDDSrcSurfaceX = this;
			}
			else
			{
				lpDDSrcSurfaceX = ((m_IDirectDrawSurface*)lpDDSrcSurfaceX)->GetWrapperInterface();

				// Check if source Surface exists
				if (!ddrawParent || !ddrawParent->DoesSurfaceExist(lpDDSrcSurfaceX))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not find source surface");
					hr = DD_OK;		// Return OK to allow some games to continue to work
					break;
				}
			}

			// Get surface copy flags
			DWORD Flags = ((dwFlags & (DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_KEYDEST | DDBLT_KEYSRC)) ? DDBLT_KEYDEST : 0) |		// Color key flags
				((lpDDBltFx && (dwFlags & DDBLT_DDFX)) ? (lpDDBltFx->dwDDFX & (DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN)) : 0);			// Mirror flags

			// Check if color key is set
			if (((dwFlags & DDBLT_KEYDEST) && !ColorKeys[0].IsSet) || ((dwFlags & DDBLT_KEYSRC) && !lpDDSrcSurfaceX->ColorKeys[2].IsSet))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: color key not set");
			}

			// Get color key
			DDCOLORKEY ColorKey = (dwFlags & DDBLT_KEYDESTOVERRIDE) ? lpDDBltFx->ddckDestColorkey :
				(dwFlags & DDBLT_KEYSRCOVERRIDE) ? lpDDBltFx->ddckSrcColorkey :
				(dwFlags & DDBLT_KEYDEST) ? ColorKeys[0].Key :
				(dwFlags & DDBLT_KEYSRC) ? lpDDSrcSurfaceX->ColorKeys[2].Key : ColorKeys[0].Key;

			hr = CopySurface(lpDDSrcSurfaceX, lpSrcRect, lpDestRect, D3DTEXF_NONE, ColorKey, Flags);

		} while (false);

		// If successful
		if (SUCCEEDED(hr))
		{
			// Set dirty flag
			dirtyFlag = true;

			// Present surface
			PresentSurface(isSkipScene);
		}

		IsInBlt = false;

		// Return
		return hr;
	}

	if (lpDDSrcSurface)
	{
		lpDDSrcSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDSrcSurface)->GetProxyInterface();
	}

	return ProxyInterface->Blt(lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
}

HRESULT m_IDirectDrawSurfaceX::BltBatch(LPDDBLTBATCH lpDDBltBatch, DWORD dwCount, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lpDDBltBatch)
		{
			return DDERR_INVALIDPARAMS;
		}

		HRESULT hr;

		for (DWORD x = 0; x < dwCount; x++)
		{
			hr = Blt(lpDDBltBatch[x].lprDest, (LPDIRECTDRAWSURFACE7)lpDDBltBatch[x].lpDDSSrc, lpDDBltBatch[x].lprSrc, lpDDBltBatch[x].dwFlags, lpDDBltBatch[x].lpDDBltFx, (x != dwCount - 1));
			if (FAILED(hr))
			{
				return hr;
			}
		}

		return DD_OK;
	}

	if (lpDDBltBatch && lpDDBltBatch->lpDDSSrc)
	{
		lpDDBltBatch->lpDDSSrc = static_cast<m_IDirectDrawSurface *>(lpDDBltBatch->lpDDSSrc)->GetProxyInterface();
	}

	return ProxyInterface->BltBatch(lpDDBltBatch, dwCount, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::BltFast(DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Convert BltFast flags into Blt flags
		DWORD Flags = 0;
		if (dwFlags & DDBLTFAST_SRCCOLORKEY)
		{
			Flags |= DDBLT_KEYSRC;
		}
		if (dwFlags & DDBLTFAST_DESTCOLORKEY)
		{
			Flags |= DDBLT_KEYDEST;
		}
		if (dwFlags & DDBLTFAST_WAIT)
		{
			Flags |= DDBLT_WAIT;
		}

		// Get SrcRect
		RECT SrcRect;
		m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = (!lpDDSrcSurface) ? this : ((m_IDirectDrawSurface*)lpDDSrcSurface)->GetWrapperInterface();

		// Check if source Surface exists
		if (!ddrawParent || !ddrawParent->DoesSurfaceExist(lpDDSrcSurfaceX))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find source surface");
			return DD_OK;		// Return OK to allow some games to continue to work
		}

		lpDDSrcSurfaceX->CheckCoordinates(&SrcRect, lpSrcRect);

		// Create DestRect
		RECT DestRect = { (LONG)dwX, (LONG)dwY, SrcRect.right - SrcRect.left + (LONG)dwX , SrcRect.bottom - SrcRect.top + (LONG)dwY };

		// Call Blt
		return Blt(&DestRect, lpDDSrcSurface, &SrcRect, Flags, nullptr);
	}

	if (lpDDSrcSurface)
	{
		lpDDSrcSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDSrcSurface)->GetProxyInterface();
	}
	
	return ProxyInterface->BltFast(dwX, dwY, lpDDSrcSurface, lpSrcRect, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::DeleteAttachedSurface(DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSAttachedSurface)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// dwFlags: Reserved. Must be zero.
		if (!lpDDSAttachedSurface || dwFlags)
		{
			return DDERR_INVALIDPARAMS;
		}

		m_IDirectDrawSurfaceX *lpAttachedSurface = ((m_IDirectDrawSurface*)lpDDSAttachedSurface)->GetWrapperInterface();

		if (!DoesAttachedSurfaceExist(lpAttachedSurface))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find attached surface");
			return DDERR_SURFACENOTATTACHED;
		}

		if (!WasAttachedSurfaceAdded(lpAttachedSurface))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: only surfaces added with AddAttachedSurface can be deleted with this method");
			return DDERR_CANNOTDETACHSURFACE;
		}

		RemoveAttachedSurfaceFromMap(lpAttachedSurface);

		lpAttachedSurface->Release();

		return DD_OK;
	}

	if (lpDDSAttachedSurface)
	{
		lpDDSAttachedSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDSAttachedSurface)->GetProxyInterface();
	}

	return ProxyInterface->DeleteAttachedSurface(dwFlags, lpDDSAttachedSurface);
}

HRESULT m_IDirectDrawSurfaceX::EnumAttachedSurfaces(LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!lpEnumSurfacesCallback)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (ProxyDirectXVersion > 3)
	{
		return EnumAttachedSurfaces2(lpContext, (LPDDENUMSURFACESCALLBACK7)lpEnumSurfacesCallback, DirectXVersion);
	}

	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = lpEnumSurfacesCallback;
	CallbackContext.DirectXVersion = DirectXVersion;

	return GetProxyInterfaceV3()->EnumAttachedSurfaces(&CallbackContext, m_IDirectDrawEnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawSurfaceX::EnumAttachedSurfaces2(LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback7, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!lpEnumSurfacesCallback7)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		for (auto it : AttachedSurfaceMap)
		{
			DDSURFACEDESC2 Desc2;
			Desc2.dwSize = sizeof(DDSURFACEDESC2);
			Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
			it.second.pSurface->GetSurfaceDesc2(&Desc2);
			if (lpEnumSurfacesCallback7((LPDIRECTDRAWSURFACE7)it.second.pSurface->GetWrapperInterfaceX(DirectXVersion), &Desc2, lpContext) == DDENUMRET_CANCEL)
			{
				return DD_OK;
			}
		}

		return DD_OK;
	}

	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback7 = lpEnumSurfacesCallback7;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ConvertSurfaceDescTo2 = (ProxyDirectXVersion > 3 && DirectXVersion < 4);

	return ProxyInterface->EnumAttachedSurfaces(&CallbackContext, m_IDirectDrawEnumSurface::ConvertCallback2);
}

HRESULT m_IDirectDrawSurfaceX::EnumOverlayZOrders(DWORD dwFlags, LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpfnCallback, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!lpfnCallback)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (ProxyDirectXVersion > 3)
	{
		return EnumOverlayZOrders2(dwFlags, lpContext, (LPDDENUMSURFACESCALLBACK7)lpfnCallback, DirectXVersion);
	}

	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = lpfnCallback;
	CallbackContext.DirectXVersion = DirectXVersion;

	return GetProxyInterfaceV3()->EnumOverlayZOrders(dwFlags, &CallbackContext, m_IDirectDrawEnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawSurfaceX::EnumOverlayZOrders2(DWORD dwFlags, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpfnCallback7, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!lpfnCallback7)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback7 = lpfnCallback7;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ConvertSurfaceDescTo2 = (ProxyDirectXVersion > 3 && DirectXVersion < 4);

	return ProxyInterface->EnumOverlayZOrders(dwFlags, &CallbackContext, m_IDirectDrawEnumSurface::ConvertCallback2);
}

HRESULT m_IDirectDrawSurfaceX::FlipBackBuffer()
{
	DWORD dwCaps = 0;
	m_IDirectDrawSurfaceX *lpTargetSurface = nullptr;

	// Loop through each surface
	for (auto it : AttachedSurfaceMap)
	{
		dwCaps = it.second.pSurface->GetSurfaceCaps().dwCaps;
		if (dwCaps & DDSCAPS_FLIP)
		{
			lpTargetSurface = it.second.pSurface;

			break;
		}
	}

	// Check if backbuffer was found
	if (!lpTargetSurface)
	{
		return DDERR_GENERIC;
	}

	// Check surface interface
	if (FAILED(lpTargetSurface->CheckInterface(__FUNCTION__, true, true)))
	{
		return DDERR_GENERIC;
	}

	// Check if surface is busy
	if (lpTargetSurface->WaitForLockState() || lpTargetSurface->IsSurfaceInDC())
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: backbuffer surface is busy!");
		return DDERR_SURFACEBUSY;
	}

	// Stop flipping when frontbuffer is found
	if (dwCaps & DDSCAPS_FRONTBUFFER)
	{
		return DD_OK;
	}

	// Swap surface
	SwapSurface(this, lpTargetSurface);

	// Flip next surface
	return lpTargetSurface->FlipBackBuffer();
}

HRESULT m_IDirectDrawSurfaceX::Flip(LPDIRECTDRAWSURFACE7 lpDDSurfaceTargetOverride, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		// Flip can be called only for a surface that has the DDSCAPS_FLIP and DDSCAPS_FRONTBUFFER capabilities
		if ((surfaceDesc2.ddsCaps.dwCaps & (DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER)) != (DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: This surface cannot be flipped");
			return DDERR_GENERIC;
		}

		// Check if surface is locked or has an open DC
		if (WaitForLockState() || IsSurfaceInDC())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: surface is busy!");
			return DDERR_SURFACEBUSY;
		}

		// Unneeded flags (can be safely ignored?)
		// Note: vsync handled by d3d9 PresentationInterval
		// - DDFLIP_DONOTWAIT
		// - DDFLIP_NOVSYNC
		// - DDFLIP_WAIT

		if ((dwFlags & (DDFLIP_INTERVAL2 | DDFLIP_INTERVAL3 | DDFLIP_INTERVAL4)) && (surfaceDesc2.ddsCaps.dwCaps2 & DDCAPS2_FLIPINTERVAL))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Interval flipping not fully implemented");
		}

		HRESULT hr = DD_OK;
		do {
			IsInFlip = true;

			// If SurfaceTargetOverride then use that surface
			if (lpDDSurfaceTargetOverride)
			{
				m_IDirectDrawSurfaceX *lpTargetSurface = ((m_IDirectDrawSurface*)lpDDSurfaceTargetOverride)->GetWrapperInterface();

				// Check if target surface exists
				if (!DoesFlipBackBufferExist(lpTargetSurface) || lpTargetSurface == this)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: invalid surface!");
					hr = DDERR_INVALIDPARAMS;
					break;
				}

				// Check for device interface
				if (FAILED(lpTargetSurface->CheckInterface(__FUNCTION__, true, true)))
				{
					hr = DDERR_GENERIC;
					break;
				}

				// Check if surface is locked or has an open DC
				if (lpTargetSurface->WaitForLockState() || lpTargetSurface->IsSurfaceInDC())
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: backbuffer surface is busy!");
					hr = DDERR_SURFACEBUSY;
					break;
				}

				// Swap surface
				SwapSurface(this, lpTargetSurface);
			}

			// Execute flip for all attached surfaces
			else
			{
				if ((dwFlags & (DDFLIP_EVEN | DDFLIP_ODD)) == (DDFLIP_EVEN | DDFLIP_ODD))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: invalid flags!");
					hr = DDERR_INVALIDPARAMS;
					break;
				}

				if (dwFlags & DDFLIP_STEREO)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Stereo flipping not implemented");
					hr = DDERR_NOSTEREOHARDWARE;
					break;
				}

				if (dwFlags & (DDFLIP_ODD | DDFLIP_EVEN))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Even and odd flipping not implemented");
					hr = DDERR_UNSUPPORTED;
					break;
				}

				// Flip surface
				hr = FlipBackBuffer();
			}

		} while (false);

		// Present surface
		if (SUCCEEDED(hr))
		{
			PresentSurface();
		}

		// Reset flip flag
		IsInFlip = false;

		return hr;
	}

	if (lpDDSurfaceTargetOverride)
	{
		lpDDSurfaceTargetOverride = static_cast<m_IDirectDrawSurface7 *>(lpDDSurfaceTargetOverride)->GetProxyInterface();
	}

	return ProxyInterface->Flip(lpDDSurfaceTargetOverride, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetAttachedSurface(LPDDSCAPS lpDDSCaps, LPDIRECTDRAWSURFACE7 FAR * lplpDDAttachedSurface, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	// Game using old DirectX, Convert DDSCAPS to DDSCAPS2
	if (ProxyDirectXVersion > 3)
	{
		if (!lplpDDAttachedSurface)
		{
			return DDERR_INVALIDPARAMS;
		}

		DDSCAPS2 Caps2;
		ConvertCaps(Caps2, *lpDDSCaps);

		return GetAttachedSurface2((lpDDSCaps) ? &Caps2 : nullptr, lplpDDAttachedSurface, DirectXVersion);
	}

	HRESULT hr = GetProxyInterfaceV3()->GetAttachedSurface(lpDDSCaps, (LPDIRECTDRAWSURFACE3*)lplpDDAttachedSurface);

	if (SUCCEEDED(hr) && lplpDDAttachedSurface)
	{
		*lplpDDAttachedSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpDDAttachedSurface, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetAttachedSurface2(LPDDSCAPS2 lpDDSCaps2, LPDIRECTDRAWSURFACE7 FAR * lplpDDAttachedSurface, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lplpDDAttachedSurface || !lpDDSCaps2)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, false)))
		{
			return DDERR_GENERIC;
		}

		m_IDirectDrawSurfaceX *lpFoundSurface = nullptr;

		// Check if attached surface exists
		for (auto it : AttachedSurfaceMap)
		{
			m_IDirectDrawSurfaceX *lpSurface = it.second.pSurface;

			if ((lpSurface->GetSurfaceCaps().dwCaps & lpDDSCaps2->dwCaps) == lpDDSCaps2->dwCaps)
			{
				if (lpFoundSurface)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: more than one surface is attached that matches the capabilities requested.");
					return DDERR_GENERIC;
				}

				lpFoundSurface = lpSurface;
			}
		}

		// No attached surface found
		if (!lpFoundSurface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to find attached surface that matches the capabilities requested: " << *lpDDSCaps2);

			return DDERR_NOTFOUND;
		}

		m_IDirectDrawSurfaceX *lpAttachedSurface = (m_IDirectDrawSurfaceX *)lpFoundSurface->GetWrapperInterfaceX(DirectXVersion);

		lpAttachedSurface->AddRef();

		*lplpDDAttachedSurface = (LPDIRECTDRAWSURFACE7)lpAttachedSurface;

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->GetAttachedSurface(lpDDSCaps2, lplpDDAttachedSurface);

	if (SUCCEEDED(hr) && lplpDDAttachedSurface)
	{
		*lplpDDAttachedSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpDDAttachedSurface, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetBltStatus(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Inquires whether a blit involving this surface can occur immediately, and returns DD_OK if the blit can be completed.
		if (dwFlags == DDGBS_CANBLT)
		{
			if (IsInBlt || IsLocked)
			{
				return DDERR_WASSTILLDRAWING;
			}
			return DD_OK;
		}
		// Inquires whether the blit is done, and returns DD_OK if the last blit on this surface has completed.
		else if (dwFlags == DDGBS_ISBLTDONE)
		{
			if (IsInBlt)
			{
				return DDERR_WASSTILLDRAWING;
			}
			return DD_OK;
		}

		return DDERR_INVALIDPARAMS;
	}

	return ProxyInterface->GetBltStatus(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetCaps(LPDDSCAPS lpDDSCaps)
{
	Logging::LogDebug() << __FUNCTION__;

	// Game using old DirectX, Convert DDSCAPS to DDSCAPS2
	if (ProxyDirectXVersion > 3)
	{
		if (!lpDDSCaps)
		{
			return DDERR_INVALIDPARAMS;
		}

		DDSCAPS2 Caps2;

		HRESULT hr = GetCaps2(&Caps2);

		// Convert back to DDSCAPS
		if (SUCCEEDED(hr))
		{
			ConvertCaps(*lpDDSCaps, Caps2);
		}

		return hr;
	}

	return GetProxyInterfaceV3()->GetCaps(lpDDSCaps);
}

HRESULT m_IDirectDrawSurfaceX::GetCaps2(LPDDSCAPS2 lpDDSCaps2)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lpDDSCaps2)
		{
			return DDERR_INVALIDPARAMS;
		}

		ConvertCaps(*lpDDSCaps2, surfaceDesc2.ddsCaps);

		return DD_OK;
	}

	return ProxyInterface->GetCaps(lpDDSCaps2);
}

HRESULT m_IDirectDrawSurfaceX::GetClipper(LPDIRECTDRAWCLIPPER FAR * lplpDDClipper)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// No clipper attached
		if (!attachedClipper)
		{
			return DDERR_NOCLIPPERATTACHED;
		}

		// Return attached clipper
		*lplpDDClipper = (LPDIRECTDRAWCLIPPER)attachedClipper;

		// Success
		return DD_OK;
	}

	HRESULT hr = ProxyInterface->GetClipper(lplpDDClipper);

	if (SUCCEEDED(hr) && lplpDDClipper)
	{
		*lplpDDClipper = ProxyAddressLookupTable.FindAddress<m_IDirectDrawClipper>(*lplpDDClipper);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Get color key index
		int x = (dwFlags == DDCKEY_DESTBLT) ? 0 :
			(dwFlags == DDCKEY_DESTOVERLAY) ? 1 :
			(dwFlags == DDCKEY_SRCBLT) ? 2 :
			(dwFlags == DDCKEY_SRCOVERLAY) ? 3 : -1;

		// Check index
		if (!lpDDColorKey || x == -1)
		{
			return DDERR_INVALIDPARAMS;
		}

		HRESULT hr = DD_OK;

		// Check if color key is set
		if (!ColorKeys[x].IsSet)
		{
			hr = DDERR_NOCOLORKEY;
		}

		// ToDo: Check if overlay exists
		if (dwFlags == DDCKEY_SRCOVERLAY)
		{
			hr = DDERR_INVALIDPARAMS;
		}

		// Copy color key
		memcpy(lpDDColorKey, &ColorKeys[x].Key, sizeof(DDCOLORKEY));

		// Return
		return hr;
	}

	return ProxyInterface->GetColorKey(dwFlags, lpDDColorKey);
}

HRESULT m_IDirectDrawSurfaceX::GetDC(HDC FAR * lphDC)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lphDC)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		// Create surface
		if (!offscreenSurface)
		{
			D3DFORMAT Format = (surfaceFormat == D3DFMT_A8R8G8B8 || surfaceFormat == D3DFMT_X8R8G8B8 || surfaceFormat == D3DFMT_R8G8B8 || surfaceFormat == D3DFMT_P8) ? D3DFMT_X8R8G8B8 :
				(surfaceFormat == D3DFMT_A1R5G5B5 || surfaceFormat == D3DFMT_X1R5G5B5) ? D3DFMT_X1R5G5B5 : 
				(surfaceFormat == D3DFMT_R5G6B5) ? D3DFMT_R5G6B5 : D3DFMT_UNKNOWN;

			if (FAILED((*d3d9Device)->CreateOffscreenPlainSurface(surfaceDesc2.dwWidth, surfaceDesc2.dwHeight, Format, D3DPOOL_SYSTEMMEM, &offscreenSurface, nullptr)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create managed surface size: " << surfaceDesc2.dwWidth << "x" << surfaceDesc2.dwHeight << " Format: " << surfaceFormat);
				return DDERR_GENERIC;
			}
		}

		HRESULT hr = DD_OK;
		bool UnlockSurface = false, UnlockTexture = false;

		do {
			D3DLOCKED_RECT SrcLockRect, DestLockRect;

			// Check if source surface is not locked then lock it
			if (FAILED(surfaceTexture->LockRect(0, &SrcLockRect, nullptr, D3DLOCK_READONLY)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock source surface");
				hr = DDERR_GENERIC;
				break;
			}
			UnlockTexture = true;

			// Check if destination surface is not locked then lock it
			if (FAILED(offscreenSurface->LockRect(&DestLockRect, nullptr, 0)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock destination surface");
				hr = DDERR_GENERIC;
				break;
			}
			UnlockSurface = true;

			// Get byte count
			DWORD SrcByteCount = SrcLockRect.Pitch / surfaceDesc2.dwWidth;
			DWORD DestByteCount = DestLockRect.Pitch / surfaceDesc2.dwWidth;
			if (!SrcByteCount || SrcByteCount > 4 || !DestByteCount || DestByteCount > 4)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: wrong bit count " << SrcByteCount << "-->" << DestByteCount);
				hr = DDERR_GENERIC;
				break;
			}

			// Create buffer variables
			BYTE *SrcBuffer = (BYTE*)SrcLockRect.pBits;
			BYTE *DestBuffer = (BYTE*)DestLockRect.pBits;

			// Copy data
			if (SrcByteCount == DestByteCount)
			{
				for (DWORD x = 0; x < surfaceDesc2.dwHeight; x++)
				{
					memcpy(DestBuffer, SrcBuffer, DestLockRect.Pitch);
					SrcBuffer += SrcLockRect.Pitch;
					DestBuffer += DestLockRect.Pitch;
				}
			}
			else if (SrcByteCount == 1 && DestByteCount == 4)
			{
				for (DWORD x = 0; x < surfaceDesc2.dwHeight; x++)
				{
					for (DWORD y = 0; y < surfaceDesc2.dwWidth; y++)
					{
						((DWORD*)DestBuffer)[y] = SrcBuffer[y];
					}
					SrcBuffer += SrcLockRect.Pitch;
					DestBuffer += DestLockRect.Pitch;
				}
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Could not copy to offscreen surface");
				hr = DDERR_GENERIC;
				break;
			}

		} while (false);

		// Unlock texure surfaces
		if (UnlockTexture)
		{
			surfaceTexture->UnlockRect(0);
		}
		if (UnlockSurface)
		{
			offscreenSurface->UnlockRect();
		}

		// GetDC from surface
		if (SUCCEEDED(hr))
		{
			hr = offscreenSurface->GetDC(lphDC);
		}

		if (SUCCEEDED(hr))
		{
			surfacehDC = *lphDC;
			IsInDC = true;
		}

		return hr;
	}

	return ProxyInterface->GetDC(lphDC);
}

HRESULT m_IDirectDrawSurfaceX::GetFlipStatus(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Get backbuffer
		m_IDirectDrawSurfaceX *lpBackBuffer = this;
		for (auto it : AttachedSurfaceMap)
		{
			if (!(it.second.pSurface->GetSurfaceCaps().dwCaps & DDSCAPS_BACKBUFFER))
			{
				lpBackBuffer = it.second.pSurface;

				break;
			}
		}

		// Queries whether the surface can flip now. The method returns DD_OK if the flip can be completed.
		if ((dwFlags == DDGFS_CANFLIP))
		{
			if (IsInFlip || IsLocked || IsInDC || lpBackBuffer->IsSurfaceLocked() || lpBackBuffer->IsSurfaceInDC())
			{
				return DDERR_WASSTILLDRAWING;
			}
			return DD_OK;
		}
		// Queries whether the flip is done. The method returns DD_OK if the last flip on this surface has completed.
		else if (dwFlags == DDGFS_ISFLIPDONE)
		{
			if (IsInFlip)
			{
				return DDERR_WASSTILLDRAWING;
			}
			return DD_OK;
		}

		return DDERR_INVALIDPARAMS;
	}

	return ProxyInterface->GetFlipStatus(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetOverlayPosition(LPLONG lplX, LPLONG lplY)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not fully Implemented.");

		if (!lplX || !lplY)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Set lplX and lplY to X, Y of this overlay surface
		*lplX = overlayX;
		*lplY = overlayY;

		return DD_OK;
	}

	return ProxyInterface->GetOverlayPosition(lplX, lplY);
}

HRESULT m_IDirectDrawSurfaceX::GetPalette(LPDIRECTDRAWPALETTE FAR * lplpDDPalette)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lplpDDPalette)
		{
			return DDERR_INVALIDPARAMS;
		}

		// No palette attached
		if (!attachedPalette)
		{
			return DDERR_NOPALETTEATTACHED;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, false)))
		{
			return DDERR_GENERIC;
		}

		// Check exclusive mode
		if (!ddrawParent->IsExclusiveMode())
		{
			return DDERR_NOEXCLUSIVEMODE;
		}

		// Return attached palette
		*lplpDDPalette = (LPDIRECTDRAWPALETTE)attachedPalette;

		// Success
		return DD_OK;
	}

	HRESULT hr = ProxyInterface->GetPalette(lplpDDPalette);

	if (SUCCEEDED(hr) && lplpDDPalette)
	{
		*lplpDDPalette = ProxyAddressLookupTable.FindAddress<m_IDirectDrawPalette>(*lplpDDPalette);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetPixelFormat(LPDDPIXELFORMAT lpDDPixelFormat)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lpDDPixelFormat)
		{
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

		// Update surface description
		GetSurfaceDesc2(&Desc2);

		// Copy pixel format to lpDDPixelFormat
		ConvertPixelFormat(*lpDDPixelFormat, Desc2.ddpfPixelFormat);

		return DD_OK;
	}

	return ProxyInterface->GetPixelFormat(lpDDPixelFormat);
}

HRESULT m_IDirectDrawSurfaceX::GetSurfaceDesc(LPDDSURFACEDESC lpDDSurfaceDesc)
{
	Logging::LogDebug() << __FUNCTION__;

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (ProxyDirectXVersion > 3)
	{
		if (!lpDDSurfaceDesc)
		{
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

		HRESULT hr = GetSurfaceDesc2(&Desc2);

		// Convert back to LPDDSURFACEDESC
		if (SUCCEEDED(hr))
		{
			ConvertSurfaceDesc(*lpDDSurfaceDesc, Desc2);
		}

		return hr;
	}

	return GetProxyInterfaceV3()->GetSurfaceDesc(lpDDSurfaceDesc);
}

HRESULT m_IDirectDrawSurfaceX::GetSurfaceDesc2(LPDDSURFACEDESC2 lpDDSurfaceDesc2)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lpDDSurfaceDesc2)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, false)))
		{
			return DDERR_GENERIC;
		}

		// Copy surfacedesc to lpDDSurfaceDesc2
		if (lpDDSurfaceDesc2 != &surfaceDesc2)
		{
			ConvertSurfaceDesc(*lpDDSurfaceDesc2, surfaceDesc2);
		}

		// Surface description
		DDSURFACEDESC2 Desc2 = { NULL };
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

		// Set Height and Width
		if ((lpDDSurfaceDesc2->dwFlags & (DDSD_HEIGHT | DDSD_WIDTH)) != (DDSD_HEIGHT | DDSD_WIDTH))
		{
			if ((Desc2.dwFlags & (DDSD_HEIGHT | DDSD_WIDTH)) != (DDSD_HEIGHT | DDSD_WIDTH))
			{
				ddrawParent->GetDisplayMode2(&Desc2);
			}
			lpDDSurfaceDesc2->dwFlags |= DDSD_HEIGHT | DDSD_WIDTH;
			lpDDSurfaceDesc2->dwWidth = Desc2.dwWidth;
			lpDDSurfaceDesc2->dwHeight = Desc2.dwHeight;
		}
		// Set Refresh Rate
		if (!(lpDDSurfaceDesc2->dwFlags & DDSD_REFRESHRATE))
		{
			if (!(Desc2.dwFlags & DDSD_REFRESHRATE))
			{
				ddrawParent->GetDisplayMode2(&Desc2);
			}
			if (Desc2.dwRefreshRate)
			{
				lpDDSurfaceDesc2->dwFlags |= DDSD_REFRESHRATE;
				lpDDSurfaceDesc2->dwRefreshRate = Desc2.dwRefreshRate;
			}
		}
		// Set PixelFormat
		if (!(lpDDSurfaceDesc2->dwFlags & DDSD_PIXELFORMAT))
		{
			if (!(Desc2.dwFlags & DDSD_PIXELFORMAT))
			{
				ddrawParent->GetDisplayMode2(&Desc2);
			}
			lpDDSurfaceDesc2->dwFlags |= DDSD_PIXELFORMAT;
			SetPixelDisplayFormat(GetDisplayFormat(Desc2.ddpfPixelFormat), lpDDSurfaceDesc2->ddpfPixelFormat);
		}
		// Set lPitch
		if (!(lpDDSurfaceDesc2->dwFlags & DDSD_PITCH))
		{
			lpDDSurfaceDesc2->dwFlags |= DDSD_PITCH;
			lpDDSurfaceDesc2->lPitch = lpDDSurfaceDesc2->dwWidth * (GetBitCount(lpDDSurfaceDesc2->ddpfPixelFormat) / 8);
		}

		// Return
		return DD_OK;
	}

	return ProxyInterface->GetSurfaceDesc(lpDDSurfaceDesc2);
}

HRESULT m_IDirectDrawSurfaceX::Initialize(LPDIRECTDRAW lpDD, LPDDSURFACEDESC lpDDSurfaceDesc)
{
	Logging::LogDebug() << __FUNCTION__;

	if (ProxyDirectXVersion > 3)
	{
		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		if (lpDDSurfaceDesc)
		{
			ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);
		}

		return Initialize2(lpDD, (lpDDSurfaceDesc) ? &Desc2 : nullptr);
	}

	if (lpDD)
	{
		lpDD = static_cast<m_IDirectDraw *>(lpDD)->GetProxyInterface();
	}

	return GetProxyInterfaceV3()->Initialize(lpDD, lpDDSurfaceDesc);
}

HRESULT m_IDirectDrawSurfaceX::Initialize2(LPDIRECTDRAW lpDD, LPDDSURFACEDESC2 lpDDSurfaceDesc2)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Not needed
		return DD_OK;
	}

	if (lpDD)
	{
		lpDD = static_cast<m_IDirectDraw *>(lpDD)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(lpDD, lpDDSurfaceDesc2);
}

HRESULT m_IDirectDrawSurfaceX::IsLost()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// You can use this method to determine when you need to reallocate surface memory. 
		// When a DirectDrawSurface object loses its surface memory, most methods return 
		// DDERR_SURFACELOST and perform no other action.

		// Check if surface is lost or not, if not return OK

		// Surface never lost
		return DD_OK;
	}

	return ProxyInterface->IsLost();
}

HRESULT m_IDirectDrawSurfaceX::Lock(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent)
{
	Logging::LogDebug() << __FUNCTION__;

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (ProxyDirectXVersion > 3)
	{
		DDSURFACEDESC2 Desc2 = { NULL };
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		if (lpDDSurfaceDesc)
		{
			ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);
		}

		HRESULT hr = Lock2(lpDestRect, &Desc2, dwFlags, hEvent);

		// Convert back to LPDDSURFACEDESC
		if (SUCCEEDED(hr) && lpDDSurfaceDesc)
		{
			ConvertSurfaceDesc(*lpDDSurfaceDesc, Desc2);
		}

		return hr;
	}

	return GetProxyInterfaceV3()->Lock(lpDestRect, lpDDSurfaceDesc, dwFlags, hEvent);
}

HRESULT m_IDirectDrawSurfaceX::Lock2(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD dwFlags, HANDLE hEvent)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		D3DLOCKED_RECT LockedRect;

		// Convert flags to d3d9
		DWORD Flags = dwFlags & (D3DLOCK_READONLY | (!IsPrimarySurface() ? DDLOCK_NOSYSLOCK : 0) | D3DLOCK_DISCARD);

		// Update rect
		RECT DestRect;
		if (!CheckCoordinates(&DestRect, lpDestRect))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid rect!");
			DDERR_INVALIDRECT;
		}

		// Lock surface
		HRESULT hr = SetLock(&LockedRect, (lpDestRect) ? &DestRect : nullptr, Flags);

		// Set desc and video memory
		if (SUCCEEDED(hr))
		{
			// Copy surfaceDesc into passed into local varable
			DDSURFACEDESC2 Desc2;
			Desc2.dwSize = sizeof(DDSURFACEDESC2);
			Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
			ConvertSurfaceDesc(Desc2, surfaceDesc2);

			// Set video memory and pitch
			if (LockedRect.pBits)
			{
				Desc2.dwFlags |= DDSD_LPSURFACE | DDSD_PITCH;
				Desc2.lpSurface = LockedRect.pBits;
				Desc2.lPitch = LockedRect.Pitch;
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to write to texture surface!");
				hr = DDERR_GENERIC;
			}

			if (lpDDSurfaceDesc2)
			{
				ConvertSurfaceDesc(*lpDDSurfaceDesc2, Desc2);
			}
		}
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock texture surface!");
		}

		return hr;
	}

	return ProxyInterface->Lock(lpDestRect, lpDDSurfaceDesc2, dwFlags, hEvent);
}

HRESULT m_IDirectDrawSurfaceX::ReleaseDC(HDC hDC)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)) || !offscreenSurface)
		{
			return DDERR_GENERIC;
		}

		HRESULT hr = offscreenSurface->ReleaseDC(hDC);

		if (SUCCEEDED(hr))
		{
			surfacehDC = nullptr;
			IsInDC = false;

			bool UnlockSurface = false, UnlockTexture = false;

			do {
				D3DLOCKED_RECT SrcLockRect, DestLockRect;

				// Check if source surface is not locked then lock it
				if (FAILED(offscreenSurface->LockRect(&SrcLockRect, nullptr, D3DLOCK_READONLY)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock source surface");
					break;
				}
				UnlockSurface = true;

				// Check if destination surface is not locked then lock it
				if (FAILED(surfaceTexture->LockRect(0, &DestLockRect, nullptr, 0)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock destination surface");
					break;
				}
				UnlockTexture = true;

				// Get byte count
				DWORD SrcByteCount = SrcLockRect.Pitch / surfaceDesc2.dwWidth;
				DWORD DestByteCount = DestLockRect.Pitch / surfaceDesc2.dwWidth;
				if (!SrcByteCount || SrcByteCount > 4 || !DestByteCount || DestByteCount > 4)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: wrong bit count " << SrcByteCount << "-->" << DestByteCount);
					hr = DDERR_GENERIC;
					break;
				}

				// Create buffer variables
				BYTE *SrcBuffer = (BYTE*)SrcLockRect.pBits;
				BYTE *DestBuffer = (BYTE*)DestLockRect.pBits;

				// Copy data
				if (SrcByteCount == DestByteCount)
				{
					for (DWORD x = 0; x < surfaceDesc2.dwHeight; x++)
					{
						memcpy(DestBuffer, SrcBuffer, DestLockRect.Pitch);
						SrcBuffer += SrcLockRect.Pitch;
						DestBuffer += DestLockRect.Pitch;
					}
				}
				else if (SrcByteCount == 4 && DestByteCount == 1)
				{
					for (DWORD x = 0; x < surfaceDesc2.dwHeight; x++)
					{
						for (DWORD y = 0; y < surfaceDesc2.dwWidth; y++)
						{
							DestBuffer[y] = (BYTE)(((DWORD*)SrcBuffer)[y]);		// ToDo: Fix this to show the correct color
						}
						SrcBuffer += SrcLockRect.Pitch;
						DestBuffer += DestLockRect.Pitch;
					}
				}
				else
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: Could not copy from offscreen surface");
					hr = DDERR_GENERIC;
					break;
				}

			} while (false);

			// Unlock texure surfaces
			if (UnlockTexture)
			{
				surfaceTexture->UnlockRect(0);
			}
			if (UnlockSurface)
			{
				offscreenSurface->UnlockRect();
			}

			// Present surface
			PresentSurface();
		}

		return hr;
	}

	return ProxyInterface->ReleaseDC(hDC);
}

HRESULT m_IDirectDrawSurfaceX::Restore()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// This method restores a surface that has been lost. The restore occurs when the surface memory associated with the DirectDrawSurface object has been freed.
		// No need to restore d3d9 surface
		return DD_OK;
	}

	return ProxyInterface->Restore();
}

HRESULT m_IDirectDrawSurfaceX::SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// If lpDDClipper is nullptr then detach the current clipper if it exists
		if (!lpDDClipper)
		{
			if (attachedClipper)
			{
				// Decrement ref count
				attachedClipper->Release();

				// Detach
				attachedClipper = nullptr;
			}

			// Reset FirstRun
			ClipperFirstRun = true;

			return DD_OK;
		}

		// Set clipper address
		attachedClipper = (m_IDirectDrawClipper *)lpDDClipper;

		// When you call SetClipper to set a clipper to a surface for the first time, 
		// SetClipper increments the clipper's reference count; subsequent calls to 
		// SetClipper do not affect the clipper's reference count.
		if (ClipperFirstRun)
		{
			attachedClipper->AddRef();
			ClipperFirstRun = false;
		}

		return DD_OK;
	}

	if (lpDDClipper)
	{
		lpDDClipper = static_cast<m_IDirectDrawClipper *>(lpDDClipper)->GetProxyInterface();
	}

	return ProxyInterface->SetClipper(lpDDClipper);
}

HRESULT m_IDirectDrawSurfaceX::SetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Get color key index
		DWORD Flag = (dwFlags & ~DDCKEY_COLORSPACE);
		int x = (Flag == DDCKEY_DESTBLT) ? 0 :
			(Flag == DDCKEY_DESTOVERLAY) ? 1 :
			(Flag == DDCKEY_SRCBLT) ? 2 :
			(Flag == DDCKEY_SRCOVERLAY) ? 3 : -1;

		// Check index
		if (x == -1)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Set color key
		if (!lpDDColorKey)
		{
			ColorKeys[x].IsSet = false;
			ColorKeys[x].Key.dwColorSpaceHighValue = 0;
			ColorKeys[x].Key.dwColorSpaceLowValue = 0;
		}
		else
		{
			ColorKeys[x].IsSet = true;
			ColorKeys[x].IsColorSpace = ((dwFlags & DDCKEY_COLORSPACE) != 0);
			if (ColorKeys[x].IsColorSpace)
			{
				memcpy(&ColorKeys[x].Key, lpDDColorKey, sizeof(DDCOLORKEY));
			}
			else  // You must add the flag DDCKEY_COLORSPACE, otherwise DirectDraw will collapse the range to one value
			{
				ColorKeys[x].Key.dwColorSpaceHighValue = *(DWORD*)lpDDColorKey;
				ColorKeys[x].Key.dwColorSpaceLowValue = *(DWORD*)lpDDColorKey;
			}
		}

		// Return
		return DD_OK;
	}

	return ProxyInterface->SetColorKey(dwFlags, lpDDColorKey);
}

HRESULT m_IDirectDrawSurfaceX::SetOverlayPosition(LONG lX, LONG lY)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not fully Implemented.");

		// Store the new overlay position
		overlayX = lX;
		overlayY = lY;

		return DD_OK;
	}

	return ProxyInterface->SetOverlayPosition(lX, lY);
}

HRESULT m_IDirectDrawSurfaceX::SetPalette(LPDIRECTDRAWPALETTE lpDDPalette)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// If lpDDPalette is nullptr then detach the current palette if it exists
		if (!lpDDPalette)
		{
			// Check for device interface
			if (FAILED(CheckInterface(__FUNCTION__, true, false)))
			{
				return DDERR_GENERIC;
			}

			if (attachedPalette)
			{
				// Decrement ref count
				attachedPalette->Release();

				// Detach
				attachedPalette = nullptr;
			}

			// Reset FirstRun
			PaletteFirstRun = true;

			return DD_OK;
		}

		// Set palette address
		attachedPalette = (m_IDirectDrawPalette *)lpDDPalette;

		// When you call SetPalette to set a palette to a surface for the first time, 
		// SetPalette increments the palette's reference count; subsequent calls to 
		// SetPalette do not affect the palette's reference count.
		if (PaletteFirstRun)
		{
			attachedPalette->AddRef();
			PaletteFirstRun = false;
		}

		// Set new palette flag
		NewPalette = true;

		return DD_OK;
	}

	if (lpDDPalette)
	{
		lpDDPalette = static_cast<m_IDirectDrawPalette *>(lpDDPalette)->GetProxyInterface();
	}

	return ProxyInterface->SetPalette(lpDDPalette);
}

HRESULT m_IDirectDrawSurfaceX::Unlock(LPRECT lpRect)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Unlock surface
		return SetUnlock();
	}

	return ProxyInterface->Unlock(lpRect);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlay(LPRECT lpSrcRect, LPDIRECTDRAWSURFACE7 lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, LPDDOVERLAYFX lpDDOverlayFx)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpDDDestSurface)
	{
		lpDDDestSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDDestSurface)->GetProxyInterface();
	}

	return ProxyInterface->UpdateOverlay(lpSrcRect, lpDDDestSurface, lpDestRect, dwFlags, lpDDOverlayFx);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlayDisplay(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->UpdateOverlayDisplay(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlayZOrder(DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSReference)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpDDSReference)
	{
		lpDDSReference = static_cast<m_IDirectDrawSurface7 *>(lpDDSReference)->GetProxyInterface();
	}

	return ProxyInterface->UpdateOverlayZOrder(dwFlags, lpDDSReference);
}

/*********************************/
/*** Added in the v2 interface ***/
/*********************************/

HRESULT m_IDirectDrawSurfaceX::GetDDInterface(LPVOID FAR * lplpDD, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lplpDD)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, false)))
		{
			return DDERR_GENERIC;
		}

		*lplpDD = ddrawParent->GetWrapperInterfaceX(DirectXVersion);

		::AddRef(*lplpDD);

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->GetDDInterface(lplpDD);

	if (SUCCEEDED(hr) && lplpDD)
	{
		// Calling the GetDDInterface method from any surface created under DirectDrawEx will return a pointer to the 
		// IUnknown interface instead of a pointer to an IDirectDraw interface. Applications must use the
		// IUnknown::QueryInterface method to retrieve the IDirectDraw, IDirectDraw2, or IDirectDraw3 interfaces.
		IID tmpID = (DirectXVersion == 1) ? IID_IDirectDraw :
			(DirectXVersion == 2) ? IID_IDirectDraw2 :
			(DirectXVersion == 3) ? IID_IDirectDraw3 :
			(DirectXVersion == 4) ? IID_IDirectDraw4 :
			(DirectXVersion == 7) ? IID_IDirectDraw7 : IID_IDirectDraw7;

		IUnknown *lpDD = (IUnknown*)*lplpDD;

		hr = lpDD->QueryInterface(tmpID, lplpDD);
		if (SUCCEEDED(hr))
		{
			*lplpDD = ProxyAddressLookupTable.FindAddress<m_IDirectDraw7>(*lplpDD, DirectXVersion);
			lpDD->Release();
		}
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::PageLock(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Prevents a system-memory surface from being paged out while a bit block transfer (bitblt) operation that uses direct memory access (DMA) transfers to or from system memory is in progress.
		// Not needed for d3d9 surfaces
		return DD_OK;
	}

	return ProxyInterface->PageLock(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::PageUnlock(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Unlocks a system-memory surface, which then allows it to be paged out.
		// Not needed for d3d9 surfaces
		return DD_OK;
	}

	return ProxyInterface->PageUnlock(dwFlags);
}

/*********************************/
/*** Added in the v3 interface ***/
/*********************************/

HRESULT m_IDirectDrawSurfaceX::SetSurfaceDesc(LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (ProxyDirectXVersion > 3)
	{
		if (!lpDDSurfaceDesc)
		{
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);

		return SetSurfaceDesc2(&Desc2, dwFlags);
	}

	return GetProxyInterfaceV3()->SetSurfaceDesc(lpDDSurfaceDesc, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::SetSurfaceDesc2(LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lpDDSurfaceDesc2)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check flags
		DWORD SurfaceFlags = lpDDSurfaceDesc2->dwFlags;
		if (SurfaceFlags & DDSD_LPSURFACE)
		{
			LOG_LIMIT(100, __FUNCTION__ << " lpSurface not fully Implemented.");

			SurfaceFlags &= ~DDSD_LPSURFACE;
			surfaceDesc2.dwFlags |= DDSD_LPSURFACE;
			surfaceDesc2.lpSurface = lpDDSurfaceDesc2->lpSurface;
		}
		if (SurfaceFlags)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: flags not implemented " << SurfaceFlags);
			return DDERR_UNSUPPORTED;
		}
		return DD_OK;
	}

	return ProxyInterface->SetSurfaceDesc(lpDDSurfaceDesc2, dwFlags);
}

/*********************************/
/*** Added in the v4 interface ***/
/*********************************/

HRESULT m_IDirectDrawSurfaceX::SetPrivateData(REFGUID guidTag, LPVOID lpData, DWORD cbSize, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		return surfaceTexture->SetPrivateData(guidTag, lpData, cbSize, dwFlags);
	}

	return ProxyInterface->SetPrivateData(guidTag, lpData, cbSize, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetPrivateData(REFGUID guidTag, LPVOID lpBuffer, LPDWORD lpcbBufferSize)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		return surfaceTexture->GetPrivateData(guidTag, lpBuffer, lpcbBufferSize);
	}

	return ProxyInterface->GetPrivateData(guidTag, lpBuffer, lpcbBufferSize);
}

HRESULT m_IDirectDrawSurfaceX::FreePrivateData(REFGUID guidTag)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		return surfaceTexture->FreePrivateData(guidTag);
	}

	return ProxyInterface->FreePrivateData(guidTag);
}

HRESULT m_IDirectDrawSurfaceX::GetUniquenessValue(LPDWORD lpValue)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lpValue)
		{
			return DDERR_INVALIDPARAMS;
		}

		// The only defined uniqueness value is 0.
		*lpValue = UniquenessValue;
		return DD_OK;
	}

	return ProxyInterface->GetUniquenessValue(lpValue);
}

HRESULT m_IDirectDrawSurfaceX::ChangeUniquenessValue()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Manually updates the uniqueness value for this surface.
		UniquenessValue = (DWORD)this + UniquenessValue + 1;
		return DD_OK;
	}

	return ProxyInterface->ChangeUniquenessValue();
}

/***********************************/
/*** Moved Texture7 methods here ***/
/***********************************/

HRESULT m_IDirectDrawSurfaceX::SetPriority(DWORD dwPriority)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		return surfaceTexture->SetPriority(dwPriority);
	}

	return ProxyInterface->SetPriority(dwPriority);
}

HRESULT m_IDirectDrawSurfaceX::GetPriority(LPDWORD lpdwPriority)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lpdwPriority)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		*lpdwPriority = surfaceTexture->GetPriority();

		return DD_OK;
	}

	return ProxyInterface->GetPriority(lpdwPriority);
}

HRESULT m_IDirectDrawSurfaceX::SetLOD(DWORD dwMaxLOD)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		return surfaceTexture->SetLOD(dwMaxLOD);
	}

	return ProxyInterface->SetLOD(dwMaxLOD);
}

HRESULT m_IDirectDrawSurfaceX::GetLOD(LPDWORD lpdwMaxLOD)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lpdwMaxLOD)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		*lpdwMaxLOD = surfaceTexture->GetLOD();

		return DD_OK;
	}

	return ProxyInterface->GetLOD(lpdwMaxLOD);
}

/************************/
/*** Helper functions ***/
/************************/

HRESULT m_IDirectDrawSurfaceX::CheckInterface(char *FunctionName, bool CheckD3DDevice, bool CheckD3DSurface)
{
	// Check for device
	if (!ddrawParent)
	{
		LOG_LIMIT(100, FunctionName << " Error: no ddraw parent!");
		return DDERR_GENERIC;
	}

	// Check for device, if not then create it
	if (CheckD3DDevice)
	{
		if (!d3d9Device || !*d3d9Device)
		{
			ddrawParent->CreateD3D9Device();

			if (!d3d9Device || !*d3d9Device)
			{
				LOG_LIMIT(100, FunctionName << " Error: d3d9 device not setup!");
				return DDERR_GENERIC;
			}
		}
	}

	// Make sure surface exists, if not then create it
	if (CheckD3DSurface)
	{
		if (!surfaceTexture || (GetSurfaceFormat() == D3DFMT_P8 && (!paletteTexture || !pixelShader)))
		{
			if (FAILED(CreateD3d9Surface()))
			{
				LOG_LIMIT(100, FunctionName << " Error: d3d9 surface texture not setup!");
				return DDERR_GENERIC;
			}
		}
	}

	return DD_OK;
}

// Create surface
HRESULT m_IDirectDrawSurfaceX::CreateD3d9Surface()
{
	// Don't recreate surface while it is locked
	if (surfaceTexture && IsLocked)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface is locked!");
		return DDERR_GENERIC;
	}

	// Release existing surface
	ReleaseD9Surface();

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, false)))
	{
		return DDERR_GENERIC;
	}

	// Update surface description
	GetSurfaceDesc2(&surfaceDesc2);

	// Get resource type
	// DDSCAPS_BACKBUFFER
	// DDSCAPS_FRONTBUFFER
	// DDSCAPS_OFFSCREENPLAIN
	// DDSCAPS_PRIMARYSURFACE
	// DDSCAPS_TEXTURE
	// DDSCAPS_MIPMAP
	// DDSCAPS2_CUBEMAP

	// Resource pool
	// DDSCAPS_SYSTEMMEMORY
	// DDSCAPS2_TEXTUREMANAGE
	// DDSCAPS2_D3DTEXTUREMANAGE
	// DDSCAPS2_DONOTPERSIST

	// Resource usage
	// DDSCAPS2_HINTSTATIC
	// DDSCAPS2_HINTDYNAMIC

	// Other flags
	// DDSCAPS_HWCODEC
	// DDSCAPS_LIVEVIDEO
	// DDSCAPS_MODEX
	// DDSCAPS_OVERLAY
	// DDSCAPS_VIDEOPORT
	// DDSCAPS_ZBUFFER
	// DDSCAPS2_CUBEMAP_POSITIVEX
	// DDSCAPS2_CUBEMAP_NEGATIVEX
	// DDSCAPS2_CUBEMAP_POSITIVEY
	// DDSCAPS2_CUBEMAP_NEGATIVEY
	// DDSCAPS2_CUBEMAP_POSITIVEZ
	// DDSCAPS2_CUBEMAP_NEGATIVEZ
	// DDSCAPS2_CUBEMAP_ALLFACES
	// DDSCAPS2_OPAQUE
	// DDSCAPS2_STEREOSURFACELEFT

	// Unused flags (can be safely ignored?)
	// DDSCAPS_3D
	// DDSCAPS_3DDEVICE
	// DDSCAPS_ALLOCONLOAD
	// DDSCAPS_OPTIMIZED
	// DDSCAPS_STANDARDVGAMODE
	// DDSCAPS2_HINTANTIALIASING
	// DDSCAPS2_MIPMAPSUBLEVEL

	// Get texture data
	surfaceFormat = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);
	surfaceBitCount = GetBitCount(surfaceFormat);
	DWORD Usage = (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_WRITEONLY) ? D3DUSAGE_WRITEONLY : 0;
	D3DFORMAT Format = (surfaceFormat == D3DFMT_P8) ? D3DFMT_L8 : (surfaceFormat == D3DFMT_R8G8B8) ? D3DFMT_X8R8G8B8 : surfaceFormat;

	// Create surface
	if (FAILED((*d3d9Device)->CreateTexture(surfaceDesc2.dwWidth, surfaceDesc2.dwHeight, 1, Usage, Format, D3DPOOL_MANAGED, &surfaceTexture, nullptr)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create surface size: " << surfaceDesc2.dwWidth << "x" << surfaceDesc2.dwHeight << " Format: " << surfaceFormat);
		return DDERR_GENERIC;
	}

	if (surfaceFormat == D3DFMT_P8)
	{
		// Create palette surface
		if (FAILED((*d3d9Device)->CreateTexture(256, 256, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &paletteTexture, nullptr)) ||
			FAILED((*d3d9Device)->CreatePixelShader((DWORD*)PalettePixelShaderSrc, &pixelShader)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create palette surface");
			return DDERR_GENERIC;
		}
		NewPalette = true;
	}

	// Reset d3d9 surface texture data
	Logging::LogDebug() << __FUNCTION__ << " Resetting Direct3D9 texture surface data";
	if (surfaceTexture)
	{
		do {
			D3DLOCKED_RECT LockRect;
			if (FAILED(surfaceTexture->LockRect(0, &LockRect, nullptr, 0)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock texture surface!");
				break;
			}

			size_t size = surfaceDesc2.dwHeight * LockRect.Pitch;

			if (size == surfaceArray.size())
			{
				memcpy(LockRect.pBits, &surfaceArray[0], size);
			}

			surfaceTexture->UnlockRect(0);

			// ToDo: reset other surface data, like PrivateData, Priority and LOD

		} while (false);
	}

	// Reset Locked flag
	IsLocked = false;

	// Only display surface if it is primary for now...
	if (!IsPrimarySurface())
	{
		return DD_OK;
	}

	// Set vertex shader
	if (FAILED((*d3d9Device)->SetVertexShader(nullptr)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set vertex shader");
		return DDERR_GENERIC;
	}

	// Set fv format
	if (FAILED((*d3d9Device)->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set the current vertex stream format");
		return DDERR_GENERIC;
	}

	// Create vertex buffer
	if (FAILED((*d3d9Device)->CreateVertexBuffer(sizeof(TLVERTEX) * 4, D3DUSAGE_DYNAMIC, (D3DFVF_XYZRHW | D3DFVF_TEX1), D3DPOOL_DEFAULT, &vertexBuffer, nullptr)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create vertex buffer");
		return DDERR_GENERIC;
	}

	// Set stream source
	if (FAILED((*d3d9Device)->SetStreamSource(0, vertexBuffer, 0, sizeof(TLVERTEX))))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set vertex buffer stream source");
		return DDERR_GENERIC;
	}

	// Set render states(no lighting)
	if (FAILED((*d3d9Device)->SetRenderState(D3DRS_LIGHTING, FALSE)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set device render state(no lighting)");
		return DDERR_GENERIC;
	}

	// Set scale mode to linear
	if (FAILED((*d3d9Device)->SetSamplerState(0, D3DSAMP_MAGFILTER, (surfaceFormat == D3DFMT_P8) ? D3DTEXF_POINT : D3DTEXF_LINEAR)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set D3D device to LINEAR sampling");
	}

	// Setup verticies (0,0,currentWidth,currentHeight)
	TLVERTEX* vertices;

	// Lock vertex buffer
	if (FAILED(vertexBuffer->Lock(0, 0, (void**)&vertices, 0)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock vertex buffer");
		return DDERR_GENERIC;
	}

	// Get width and height
	DWORD BackBufferWidth = displayWidth;
	DWORD BackBufferHeight = displayHeight;
	if (!BackBufferWidth || !BackBufferHeight)
	{
		BackBufferWidth = GetSystemMetrics(SM_CXSCREEN);
		BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
	}

	// Calculate width and height with original aspect ratio
	DWORD xpad = 0;
	DWORD ypad = 0;
	if (Config.DdrawMaintainAspectRatio)
	{
		if (surfaceDesc2.dwWidth * displayHeight < surfaceDesc2.dwHeight * displayWidth)
		{
			// 4:3 displayed on 16:9
			BackBufferWidth = displayHeight * surfaceDesc2.dwWidth / surfaceDesc2.dwHeight;
		}
		else
		{
			// 16:9 displayed on 4:3
			BackBufferHeight = displayWidth * surfaceDesc2.dwHeight / surfaceDesc2.dwWidth;
		}
		xpad = (displayWidth - BackBufferWidth) / 2;
		ypad = (displayHeight - BackBufferHeight) / 2;
	}

	Logging::LogDebug() << __FUNCTION__ << " D3d9 Vertex size: " << BackBufferWidth << "x" << BackBufferHeight;
	// Set vertex points
	// 0, 0
	vertices[0].x = -0.5f + xpad;
	vertices[0].y = -0.5f + ypad;
	vertices[0].z = 0.0f;
	vertices[0].rhw = 1.0f;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;

	// scaledWidth, 0
	vertices[1].x = -0.5f + xpad + BackBufferWidth;
	vertices[1].y = vertices[0].y;
	vertices[1].z = 0.0f;
	vertices[1].rhw = 1.0f;
	vertices[1].u = 1.0f;
	vertices[1].v = 0.0f;

	// scaledWidth, scaledHeight
	vertices[2].x = vertices[1].x;
	vertices[2].y = -0.5f + ypad + BackBufferHeight;
	vertices[2].z = 0.0f;
	vertices[2].rhw = 1.0f;
	vertices[2].u = 1.0f;
	vertices[2].v = 1.0f;

	// 0, scaledHeight
	vertices[3].x = vertices[0].x;
	vertices[3].y = vertices[2].y;
	vertices[3].z = 0.0f;
	vertices[3].rhw = 1.0f;
	vertices[3].u = 0.0f;
	vertices[3].v = 1.0f;

	// Unlock vertex buffer
	if (FAILED(vertexBuffer->Unlock()))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to unlock vertex buffer");
		return DDERR_GENERIC;
	}

	return DD_OK;
}

template <typename T>
void m_IDirectDrawSurfaceX::ReleaseD9Interface(T **ppInterface)
{
	if (ppInterface && *ppInterface)
	{
		DWORD x = 0, z = 0;
		do
		{
			z = (*ppInterface)->Release();
		} while (z != 0 && ++x < 100);

		// Add Error: checking
		if (z != 0)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to release Direct3D9 interface");
		}

		(*ppInterface) = nullptr;
	}
}

// Release surface and vertext buffer
void m_IDirectDrawSurfaceX::ReleaseD9Surface(bool BackupData)
{
	// Store d3d9 surface texture
	if (surfaceTexture && BackupData)
	{
		Logging::LogDebug() << __FUNCTION__ << " Storing Direct3D9 texture surface data";
		do {
			surfaceTexture->UnlockRect(0);

			D3DLOCKED_RECT LockRect;
			if (FAILED(surfaceTexture->LockRect(0, &LockRect, nullptr, 0)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock texture surface!");
				break;
			}

			size_t size = surfaceDesc2.dwHeight * LockRect.Pitch;

			if (size != surfaceArray.size())
			{
				surfaceArray.resize(size);
			}

			memcpy(&surfaceArray[0], LockRect.pBits, size);

			surfaceTexture->UnlockRect(0);

			// ToDo: store other surface data, like PrivateData, Priority and LOD

		} while (false);
	}

	// Release d3d9 surface texture
	if (surfaceTexture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 texture surface";
		surfaceTexture->UnlockRect(0);
		ReleaseD9Interface(&surfaceTexture);
	}

	// Release d3d9 offscreen surface
	if (offscreenSurface)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 offscreen surface";
		offscreenSurface->UnlockRect();
		ReleaseD9Interface(&offscreenSurface);
	}

	// Release d3d9 palette surface texture
	if (paletteTexture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 palette texture surface";
		paletteTexture->UnlockRect(0);
		ReleaseD9Interface(&paletteTexture);
	}

	// Release d3d9 pixel shader
	if (pixelShader)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 pixel shader";
		ReleaseD9Interface(&pixelShader);
	}

	// Release d3d9 vertex buffer
	if (vertexBuffer)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 vertext buffer";
		ReleaseD9Interface(&vertexBuffer);
	}

	// Set unlock
	IsLocked = false;
}

// Present surface
HRESULT m_IDirectDrawSurfaceX::PresentSurface(BOOL isSkipScene)
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, true)))
	{
		return DDERR_GENERIC;
	}

	// Check if is not primary surface or if scene should be skipped
	if (!IsPrimarySurface() && SceneReady && !IsPresentRunning)
	{
		m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = ddrawParent->GetPrimarySurface();
		if (lpDDSrcSurfaceX)
		{
			lpDDSrcSurfaceX->PresentSurface(isSkipScene);
		}
		return DDERR_GENERIC;
	}
	else if (!IsPrimarySurface())
	{
		return DDERR_GENERIC;
	}
	else if (isSkipScene && !SceneReady)
	{
		Logging::LogDebug() << __FUNCTION__ << " Skipping scene!";
		return DDERR_GENERIC;
	}

	// Set scene ready
	SceneReady = true;

	// Check if surface is locked or has an open DC
	if (WaitForLockState() || IsInDC || IsPresentRunning)
	{
		Logging::LogDebug() << __FUNCTION__ << " Surface is busy!";
		return DDERR_SURFACEBUSY;
	}
	IsPresentRunning = true;

	// Preset surface
	HRESULT hr = DD_OK;
	do {
		// If new palette data then write it to texture
		if ((paletteTexture && GetSurfaceFormat() == D3DFMT_P8) &&
			(NewPalette || (attachedPalette && attachedPalette->NewPaletteData)))
		{
			do {
				D3DLOCKED_RECT LockRect;
				RECT Rect = { 0, 0, 256, 1 };
				const DWORD *memPalette = (attachedPalette && attachedPalette->rgbPalette) ? attachedPalette->rgbPalette : rgbPalette;

				if (FAILED(paletteTexture->LockRect(0, &LockRect, &Rect, 0)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock palette texture!");
					break;
				}

				memcpy(LockRect.pBits, memPalette, 256 * sizeof(int));

				paletteTexture->UnlockRect(0);

				NewPalette = false;
				if (attachedPalette)
				{
					attachedPalette->NewPaletteData = false;
				}

			} while (false);
		}

		// Set texture
		if (surfaceTexture)
		{
			if (FAILED((*d3d9Device)->SetTexture(0, surfaceTexture)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set texture");
				hr = DDERR_GENERIC;
				break;
			}
		}

		// Set palette texture
		if (paletteTexture)
		{
			if (FAILED((*d3d9Device)->SetTexture(1, paletteTexture)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock palette texture!");
				hr = DDERR_GENERIC;
				break;
			}
		}

		// Set pixel shader
		if (pixelShader)
		{
			if (FAILED((*d3d9Device)->SetPixelShader(pixelShader)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set pixel shader");
				hr = DDERR_GENERIC;
				break;
			}
		}

		// EndScene
		if (FAILED(ddrawParent->EndScene()))
		{
			hr = DDERR_GENERIC;
			break;
		}

		// Reset dirty flag
		dirtyFlag = false;

		// Reset scene ready
		SceneReady = false;

	} while (false);

	// Reset present flag
	IsPresentRunning = false;

	return hr;
}

// Swap surface addresses for Flip
void m_IDirectDrawSurfaceX::SwapSurface(m_IDirectDrawSurfaceX *lpTargetSurface1, m_IDirectDrawSurfaceX *lpTargetSurface2)
{
	if (!lpTargetSurface1 || !lpTargetSurface2 || lpTargetSurface1 == lpTargetSurface2)
	{
		return;
	}

	// Swap surface textures
	SwapAddresses(lpTargetSurface1->GetSurfaceTexture(), lpTargetSurface2->GetSurfaceTexture());
}

// Check surface reck dimensions and copy rect to new rect
bool m_IDirectDrawSurfaceX::CheckCoordinates(LPRECT lpOutRect, LPRECT lpInRect)
{
	if (!lpOutRect)
	{
		return false;
	}

	// Check device coordinates
	if (!surfaceDesc2.dwWidth || !surfaceDesc2.dwHeight)
	{
		return false;
	}

	if (lpInRect)
	{
		memcpy(lpOutRect, lpInRect, sizeof(RECT));
	}
	else
	{
		lpOutRect->top = 0;
		lpOutRect->left = 0;
		lpOutRect->right = surfaceDesc2.dwWidth;
		lpOutRect->bottom = surfaceDesc2.dwHeight;
	}

	if (lpOutRect->left < 0)
	{
		lpOutRect->left = 0;
	}

	if (lpOutRect->top < 0)
	{
		lpOutRect->top = 0;
	}

	if (lpOutRect->left > (LONG)surfaceDesc2.dwWidth)
	{
		lpOutRect->left = surfaceDesc2.dwWidth;
	}

	if (lpOutRect->right > (LONG)surfaceDesc2.dwWidth)
	{
		lpOutRect->right = surfaceDesc2.dwWidth;
	}

	if (lpOutRect->top > (LONG)surfaceDesc2.dwHeight)
	{
		lpOutRect->top = surfaceDesc2.dwHeight;
	}

	if (lpOutRect->bottom > (LONG)surfaceDesc2.dwHeight)
	{
		lpOutRect->bottom = surfaceDesc2.dwHeight;
	}

	return lpOutRect->left < lpOutRect->right && lpOutRect->top < lpOutRect->bottom;
}

// Wait for other thread to unlock surface
bool m_IDirectDrawSurfaceX::WaitForLockState()
{
	if (IsLocked && LockThreadID != GetCurrentThreadId())
	{
		do {
			Sleep(0);
		} while (IsLocked);
	}

	return IsLocked;
}

// Lock the d3d9 surface
HRESULT m_IDirectDrawSurfaceX::SetLock(D3DLOCKED_RECT* pLockedRect, LPRECT lpDestRect, DWORD dwFlags, BOOL isSkipScene)
{
	if (!pLockedRect)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, true)))
	{
		return DDERR_GENERIC;
	}

	// Check if the scene needs to be presented
	isSkipScene |= ((lpDestRect) ? (abs(lpDestRect->bottom - lpDestRect->top) < 2 || abs(lpDestRect->right - lpDestRect->left) < 2) : FALSE);

	// Check if locked
	if (IsLocked)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Locking surface twice not fully implemented");
	}

	// Run EndScene before locking if dirty flag is set
	if (dirtyFlag)
	{
		if (SUCCEEDED(PresentSurface(isSkipScene)))
		{
			EndSceneLock = true;
		}
	}

	// Lock surface
	HRESULT hr = surfaceTexture->LockRect(0, pLockedRect, lpDestRect, dwFlags);
	if (FAILED(hr))
	{
		// If failed then wait for other threads, unlock and try again
		WaitForLockState();
		surfaceTexture->UnlockRect(0);
		hr = surfaceTexture->LockRect(0, pLockedRect, lpDestRect, dwFlags);

		// If lock fails
		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock surface");
			return (hr == D3DERR_INVALIDCALL) ? DDERR_GENERIC :
				(hr == D3DERR_WASSTILLDRAWING) ? DDERR_WASSTILLDRAWING :
				DDERR_SURFACELOST;
		}
	}

	// Set lock flag
	InterlockedExchange(&LockThreadID, GetCurrentThreadId());
	IsLocked = true;

	// Set pitch
	surfaceDesc2.dwFlags |= DDSD_PITCH;
	surfaceDesc2.lPitch = pLockedRect->Pitch;

	// Set dirty flag
	dirtyFlag = true;

	// Success
	return DD_OK;
}

// Unlock the d3d9 surface
HRESULT m_IDirectDrawSurfaceX::SetUnlock(BOOL isSkipScene)
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, true)))
	{
		return DDERR_GENERIC;
	}

	// Lock surface
	HRESULT hr = surfaceTexture->UnlockRect(0);
	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to unlock surface");
		return (hr == D3DERR_INVALIDCALL) ? DDERR_GENERIC :
			(hr == D3DERR_WASSTILLDRAWING) ? DDERR_WASSTILLDRAWING :
			DDERR_SURFACELOST;
	}
	IsLocked = false;

	// Present surface
	if (!EndSceneLock)
	{
		PresentSurface(isSkipScene);
	}

	// Reset endscene lock
	EndSceneLock = false;

	return DD_OK;
}

// Update surface description and create backbuffers
void m_IDirectDrawSurfaceX::InitSurfaceDesc(LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD DirectXVersion)
{
	if (!lpDDSurfaceDesc2)
	{
		return;
	}

	// Copy surface description
	surfaceDesc2.dwSize = sizeof(DDSURFACEDESC2);
	surfaceDesc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	ConvertSurfaceDesc(surfaceDesc2, *lpDDSurfaceDesc2);
	surfaceDesc2.dwFlags |= DDSD_CAPS;
	if (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
	{
		surfaceDesc2.ddsCaps.dwCaps |= DDSCAPS_VISIBLE;
	}
	if (!(surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) && !(surfaceDesc2.ddsCaps.dwCaps2 & DDSCAPS2_DONOTPERSIST))
	{
		surfaceDesc2.ddsCaps.dwCaps |= DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY;
		surfaceDesc2.ddsCaps.dwCaps &= ~DDSCAPS_NONLOCALVIDMEM;
	}

	// Create backbuffers
	if (surfaceDesc2.dwBackBufferCount)
	{
		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		ConvertSurfaceDesc(Desc2, surfaceDesc2);
		Desc2.dwBackBufferCount--;
		if (Desc2.ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER)
		{
			Desc2.ddsCaps.dwCaps &= ~(DDSCAPS_VISIBLE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FRONTBUFFER);
			Desc2.ddsCaps.dwCaps |= DDSCAPS_BACKBUFFER;
			Desc2.dwReserved = (DWORD)this;
		}
		else if (Desc2.ddsCaps.dwCaps & DDSCAPS_BACKBUFFER)
		{
			Desc2.ddsCaps.dwCaps &= ~DDSCAPS_BACKBUFFER;
		}

		// Create complex surfaces
		if (Desc2.ddsCaps.dwCaps & DDSCAPS_COMPLEX)
		{
			BackBufferInterface = std::make_unique<m_IDirectDrawSurfaceX>(d3d9Device, ddrawParent, DirectXVersion, &Desc2, displayWidth, displayHeight);

			m_IDirectDrawSurfaceX *attachedSurface = BackBufferInterface.get();

			AddAttachedSurfaceToMap(attachedSurface);

			attachedSurface->AddRef();
		}
		else
		{
			m_IDirectDrawSurfaceX *attachedSurface = new m_IDirectDrawSurfaceX(d3d9Device, ddrawParent, DirectXVersion, &Desc2, displayWidth, displayHeight);

			AddAttachedSurfaceToMap(attachedSurface);
		}
	}
	else if (surfaceDesc2.dwReserved)
	{
		m_IDirectDrawSurfaceX *attachedSurface = (m_IDirectDrawSurfaceX *)surfaceDesc2.dwReserved;

		// Check if source Surface exists and add to surface map
		if (ddrawParent->DoesSurfaceExist(attachedSurface))
		{
			AddAttachedSurfaceToMap(attachedSurface);
		}
	}
	if (!(surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER))
	{
		surfaceDesc2.dwFlags &= ~DDSD_BACKBUFFERCOUNT;
		surfaceDesc2.dwBackBufferCount = 0;
	}
	surfaceDesc2.dwReserved = 0;
}

// Add attached surface to map
void m_IDirectDrawSurfaceX::AddAttachedSurfaceToMap(m_IDirectDrawSurfaceX* lpSurfaceX, bool MarkAttached)
{
	if (!lpSurfaceX)
	{
		return;
	}

	// Store surface
	AttachedSurfaceMap[++MapKey].pSurface = lpSurfaceX;
	if (MarkAttached)
	{
		AttachedSurfaceMap[MapKey].isAttachedSurfaceAdded = true;
	}
}

// Remove attached surface from map
void m_IDirectDrawSurfaceX::RemoveAttachedSurfaceFromMap(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return;
	}

	auto it = std::find_if(AttachedSurfaceMap.begin(), AttachedSurfaceMap.end(),
		[=](auto Map) -> bool { return Map.second.pSurface == lpSurfaceX; });

	if (it != std::end(AttachedSurfaceMap))
	{
		AttachedSurfaceMap.erase(it);
	}
}

// Check if attached surface exists
bool m_IDirectDrawSurfaceX::DoesAttachedSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return false;
	}

	auto it = std::find_if(AttachedSurfaceMap.begin(), AttachedSurfaceMap.end(),
		[=](auto Map) -> bool { return Map.second.pSurface == lpSurfaceX; });

	if (it == std::end(AttachedSurfaceMap))
	{
		return false;
	}

	return true;
}

bool m_IDirectDrawSurfaceX::WasAttachedSurfaceAdded(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return false;
	}

	auto it = std::find_if(AttachedSurfaceMap.begin(), AttachedSurfaceMap.end(),
		[=](auto Map) -> bool { return (Map.second.pSurface == lpSurfaceX) && Map.second.isAttachedSurfaceAdded; });

	if (it == std::end(AttachedSurfaceMap))
	{
		return false;
	}

	return true;
}

// Check if backbuffer surface exists
bool m_IDirectDrawSurfaceX::DoesFlipBackBufferExist(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return false;
	}

	DWORD dwCaps = 0;
	m_IDirectDrawSurfaceX *lpTargetSurface = nullptr;

	// Loop through each surface
	for (auto it : AttachedSurfaceMap)
	{
		dwCaps = it.second.pSurface->GetSurfaceCaps().dwCaps;
		if (dwCaps & DDSCAPS_FLIP)
		{
			lpTargetSurface = it.second.pSurface;

			break;
		}
	}

	// Check if attached surface was not found
	if (!lpTargetSurface || (dwCaps & DDSCAPS_FRONTBUFFER))
	{
		return false;
	}

	// Check if attached surface was found
	if (lpTargetSurface == lpSurfaceX)
	{
		return true;
	}

	// Check next surface
	return lpTargetSurface->DoesFlipBackBufferExist(lpSurfaceX);
}

HRESULT m_IDirectDrawSurfaceX::ColorFill(RECT* pRect, D3DCOLOR dwFillColor)
{
	Logging::LogDebug() << __FUNCTION__;

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, true)))
	{
		return DDERR_GENERIC;
	}

	HRESULT hr = DD_OK;
	bool UnlockDest = false;
	do {
		// Check and copy rect
		RECT DestRect;
		if (!CheckCoordinates(&DestRect, pRect))
		{
			hr = DDERR_INVALIDRECT;
			break;
		}

		// Check if surface is not locked then lock it
		D3DLOCKED_RECT DestLockRect;
		if (FAILED(SetLock(&DestLockRect, &DestRect, 0, true)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock dest surface " << DestRect);
			hr = (IsLocked) ? DDERR_SURFACEBUSY : DDERR_GENERIC;
			break;
		}
		UnlockDest = true;

		// Get byte count
		DWORD ByteCount = DestLockRect.Pitch / surfaceDesc2.dwWidth;
		if (!ByteCount || ByteCount > 4)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find correct fill color for ByteCount " << ByteCount);
			hr = DDERR_GENERIC;
			break;
		}

		// Get width and height of rect
		LONG FillWidth = DestRect.right - DestRect.left;
		LONG FillHeight = DestRect.bottom - DestRect.top;

		// Set memory address
		BYTE *surfaceBuffer = (BYTE*)DestLockRect.pBits;

		// Fill temporary memory
		if (ByteCount != 4)
		{
			size_t size = FillWidth * ByteCount;
			if (size > surfaceArray.size())
			{
				surfaceArray.resize(size);
			}
			for (LONG x = 0; x < FillWidth; x++)
			{
				memcpy(&surfaceArray[0] + (x * ByteCount),		// Video memory address
					&dwFillColor,								// Fill color
					ByteCount);									// Size of bytes to write
			}
		}

		// Fill surface rect
		for (LONG y = 0; y < FillHeight; y++)
		{
			if (ByteCount == 4)
			{
				memset(surfaceBuffer,							// Video memory address
					dwFillColor,								// Fill color
					FillWidth);									// Size of bytes to write
			}
			else
			{
				memcpy(surfaceBuffer,							// Video memory address
					&surfaceArray[0],							// Fill color array
					FillWidth * ByteCount);						// Size of bytes to write
			}
			surfaceBuffer += DestLockRect.Pitch;
		}

	} while (false);

	// Unlock surfaces if needed
	if (UnlockDest)
	{
		SetUnlock(true);
	}

	return hr;
}

// Copy surface
HRESULT m_IDirectDrawSurfaceX::CopySurface(m_IDirectDrawSurfaceX* pSourceSurface, RECT* pSourceRect, RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter, DDCOLORKEY ColorKey, DWORD dwFlags)
{
	UNREFERENCED_PARAMETER(Filter);

	Logging::LogDebug() << __FUNCTION__;

	// Check parameters
	if (!pSourceSurface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid parameters!");
		return DDERR_INVALIDPARAMS;
	}

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, true)) || FAILED(pSourceSurface->CheckInterface(__FUNCTION__, true, true)))
	{
		return DDERR_GENERIC;
	}

	HRESULT hr = DD_OK;
	bool UnlockSrc = false, UnlockDest = false;
	do {
		D3DLOCKED_RECT SrcLockRect, DestLockRect;
		DWORD DestBitCount = GetSurfaceBitCount();
		D3DFORMAT SrcFormat = pSourceSurface->GetSurfaceFormat();
		D3DFORMAT DestFormat = GetSurfaceFormat();

		// Get byte count
		DWORD ByteCount = DestBitCount / 8;
		if (!ByteCount || ByteCount > 4)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrong bit count " << DestBitCount);
			hr = DDERR_GENERIC;
			break;
		}

		// Check source and destination format
		if (!(SrcFormat == DestFormat ||
			((SrcFormat == D3DFMT_A1R5G5B5 || SrcFormat == D3DFMT_X1R5G5B5) && (DestFormat == D3DFMT_A1R5G5B5 || DestFormat == D3DFMT_X1R5G5B5)) ||
			((SrcFormat == D3DFMT_A8R8G8B8 || SrcFormat == D3DFMT_X8R8G8B8) && (DestFormat == D3DFMT_A8R8G8B8 || DestFormat == D3DFMT_X8R8G8B8)) ||
			((SrcFormat == D3DFMT_A8B8G8R8 || SrcFormat == D3DFMT_X8B8G8R8) && (DestFormat == D3DFMT_A8B8G8R8 || DestFormat == D3DFMT_X8B8G8R8))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: not supported for different source and destination formats! " << SrcFormat << "-->" << DestFormat);
			hr = DDERR_GENERIC;
			break;
		}

		// Get source and dest rect
		RECT SrcRect = { 0, 0, (LONG)pSourceSurface->GetWidth(), (LONG)pSourceSurface->GetHeight() };
		RECT DestRect = { 0, 0, (LONG)surfaceDesc2.dwWidth, (LONG)surfaceDesc2.dwHeight };

		if (pSourceRect)
		{
			memcpy(&SrcRect, pSourceRect, sizeof(RECT));
		}

		if (pDestRect)
		{
			memcpy(&DestRect, pDestRect, sizeof(RECT));
		}

		// Get copy flags
		bool IsStretchRect = (abs((DestRect.right - DestRect.left) - (SrcRect.right - SrcRect.left)) > 1 || abs((DestRect.bottom - DestRect.top) - (SrcRect.bottom - SrcRect.top)) > 1);
		bool IsColorKey = ((dwFlags & DDBLT_KEYDEST) != 0);
		bool IsMirrorLeftRight = ((dwFlags & DDBLTFX_MIRRORLEFTRIGHT) != 0);
		bool IsMirrorUpDown = ((dwFlags & DDBLTFX_MIRRORUPDOWN) != 0);

		// Check and copy rect and do clipping
		if (!pSourceSurface->CheckCoordinates(&SrcRect, pSourceRect) || !CheckCoordinates(&DestRect, pDestRect))
		{
			hr = DDERR_INVALIDRECT;
			break;
		}

		// Get width and height of rect
		LONG SrcRectWidth = SrcRect.right - SrcRect.left;
		LONG SrcRectHeight = SrcRect.bottom - SrcRect.top;
		LONG DestRectWidth = DestRect.right - DestRect.left;
		LONG DestRectHeight = DestRect.bottom - DestRect.top;

		if (!IsStretchRect)
		{
			SrcRectWidth = min(SrcRectWidth, DestRectWidth);
			SrcRectHeight = min(SrcRectHeight, DestRectHeight);
			DestRectWidth = min(SrcRectWidth, DestRectWidth);
			DestRectHeight = min(SrcRectHeight, DestRectHeight);
		}

		// Check if source surface is not locked then lock it
		if (FAILED(pSourceSurface->SetLock(&SrcLockRect, &SrcRect, D3DLOCK_READONLY, true)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock source surface " << SrcRect);
			hr = (pSourceSurface->IsSurfaceLocked()) ? DDERR_SURFACEBUSY : DDERR_GENERIC;
			break;
		}
		UnlockSrc = true;

		// Check if source and destination memory addresses are overlapping
		if (this == pSourceSurface)
		{
			size_t size = surfaceDesc2.dwHeight * SrcLockRect.Pitch;
			if (size > surfaceArray.size())
			{
				surfaceArray.resize(size);
			}
			BYTE *SrcBuffer = (BYTE*)SrcLockRect.pBits;
			BYTE *DestBuffer = (BYTE*)&surfaceArray[0];
			for (LONG y = 0; y < SrcRectHeight; y++)
			{
				memcpy(DestBuffer, SrcBuffer, SrcRectWidth * ByteCount);
				SrcBuffer += SrcLockRect.Pitch;
				DestBuffer += SrcLockRect.Pitch;
			}
			SrcLockRect.pBits = &surfaceArray[0];
			SetUnlock(true);
			UnlockSrc = false;
		}

		// Check if destination surface is not locked then lock it
		if (FAILED(SetLock(&DestLockRect, &DestRect, 0, true)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock destination surface " << DestRect);
			hr = (IsLocked) ? DDERR_SURFACEBUSY : DDERR_GENERIC;
			break;
		}
		UnlockDest = true;

		// Create buffer variables
		BYTE *SrcBuffer = (BYTE*)SrcLockRect.pBits;
		BYTE *DestBuffer = (BYTE*)DestLockRect.pBits;

		// For mirror copy up/down
		LONG DestPitch = DestLockRect.Pitch;
		if (IsMirrorUpDown)
		{
			DestPitch = -DestLockRect.Pitch;
			DestBuffer += DestLockRect.Pitch * (DestRectHeight - 1);
		}

		// Copy memory (simple)
		if (!IsStretchRect && !IsColorKey & !IsMirrorLeftRight)
		{
			for (LONG y = 0; y < DestRectHeight; y++)
			{
				memcpy(DestBuffer, SrcBuffer, DestRectWidth * ByteCount);
				SrcBuffer += SrcLockRect.Pitch;
				DestBuffer += DestPitch;
			}
			break;
		}

		// Get ratio
		float WidthRatio = (float)SrcRectWidth / (float)DestRectWidth;
		float HeightRatio = (float)SrcRectHeight / (float)DestRectHeight;

		// Set color varables
		DWORD ByteMask = (ByteCount == 1) ? 0x000000FF : (ByteCount == 2) ? 0x0000FFFF : (ByteCount == 3) ? 0x00FFFFFF : 0xFFFFFFFF;
		DWORD ColorKeyLow = ColorKey.dwColorSpaceLowValue & ByteMask;
		DWORD ColorKeyHigh = ColorKey.dwColorSpaceHighValue & ByteMask;

		// Copy memory (complex)
		for (LONG y = 0; y < DestRectHeight; y++)
		{
			for (LONG x = 0; x < DestRectWidth; x++)
			{
				DWORD r = (IsStretchRect) ? (DWORD)((float)x * WidthRatio) : x;
				BYTE *NewPixel = (IsMirrorLeftRight) ? SrcBuffer + ((SrcRectWidth - r - 1) * ByteCount) : SrcBuffer + (r * ByteCount);
				DWORD PixelColor = (IsColorKey) ? (DWORD)(*(DWORD*)NewPixel) & ByteMask : 0;

				if (!IsColorKey || PixelColor < ColorKeyLow || PixelColor > ColorKeyHigh)
				{
					memcpy(DestBuffer + (x * ByteCount), NewPixel, ByteCount);
				}
			}
			SrcBuffer = (IsStretchRect) ? (BYTE*)SrcLockRect.pBits + (DWORD)((float)y * HeightRatio) * SrcLockRect.Pitch : SrcBuffer + SrcLockRect.Pitch;
			DestBuffer += DestPitch;
		}

	} while (false);

	// Unlock surfaces if needed
	if (UnlockDest)
	{
		SetUnlock(true);
	}
	if (UnlockSrc)
	{
		pSourceSurface->SetUnlock(true);
	}

	// Return
	return hr;
}

void m_IDirectDrawSurfaceX::ReleaseInterface()
{
	SetCriticalSection();
	if (ddrawParent)
	{
		ddrawParent->RemoveSurfaceFromVector(this);
	}
	ReleaseCriticalSection();
}
