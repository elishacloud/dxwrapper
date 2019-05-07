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

/************************/
/*** IUnknown methods ***/
/************************/

HRESULT m_IDirectDrawSurfaceX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9 && (riid == IID_IDirectDrawSurface || riid == IID_IDirectDrawSurface2 || riid == IID_IDirectDrawSurface3 || riid == IID_IDirectDrawSurface4 || riid == IID_IDirectDrawSurface7 || riid == IID_IUnknown) && ppvObj)
	{
		DWORD DxVersion = (riid == IID_IUnknown) ? DirectXVersion : GetIIDVersion(riid);

		*ppvObj = GetWrapperInterfaceX(DxVersion);

		::AddRef(*ppvObj);

		return DD_OK;
	}

	if (Config.ConvertToDirect3D7 && (riid == IID_IDirect3DTexture || riid == IID_IDirect3DTexture2) && ppvObj)
	{
		// Check for device
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
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
		Logging::Log() << __FUNCTION__ << " Error, wrapper interface version not found: " << DirectXVersion;
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

		// Check for device
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_GENERIC;
		}

		m_IDirectDrawSurfaceX *lpAttachedSurface = ((m_IDirectDrawSurface*)lpDDSurface)->GetWrapperInterface();

		if (!ddrawParent->DoesSurfaceExist(lpAttachedSurface))
		{
			return DDERR_INVALIDPARAMS;
		}

		if (DoesAttachedSurfaceExist(lpAttachedSurface))
		{
			return DDERR_SURFACEALREADYATTACHED;
		}

		AddAttachedSurfaceToMap(lpAttachedSurface);

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
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->AddOverlayDirtyRect(lpRect);
}

HRESULT m_IDirectDrawSurfaceX::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Check for required DDBLTFX structure
		if (!lpDDBltFx && (dwFlags & (DDBLT_DDFX | DDBLT_COLORFILL | DDBLT_DEPTHFILL | DDBLT_DDROPS | DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_ROP | DDBLT_ROTATIONANGLE)))
		{
			Logging::Log() << __FUNCTION__ << " DDBLTFX structure not found";
			return DDERR_INVALIDPARAMS;
		}

		// Check for DDROP flag
		if (dwFlags & DDBLT_DDROPS)
		{
			Logging::Log() << __FUNCTION__ << " DDROP Not Implemented";
			return DDERR_NODDROPSHW;
		}

		// Check for raster operations flag
		if (dwFlags & DDBLT_ROP)
		{
			Logging::Log() << __FUNCTION__ << " Raster operations Not Implemented";
			return DDERR_NORASTEROPHW;
		}

		// Check for ZBuffer flags
		if (dwFlags & DDBLT_DEPTHFILL)
		{
			Logging::Log() << __FUNCTION__ << " Depth Fill Not Implemented";
			return DDERR_NOZBUFFERHW;
		}

		// Check for ZBuffer flags
		if ((dwFlags & DDBLT_ZBUFFER) || ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & (DDBLTFX_ZBUFFERBASEDEST | DDBLTFX_ZBUFFERRANGE))))
		{
			Logging::Log() << __FUNCTION__ << " ZBuffer Not Implemented";
			return DDERR_NOZBUFFERHW;
		}

		// Check for rotation flags
		if ((dwFlags & DDBLT_ROTATIONANGLE) || ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & (DDBLTFX_ROTATE180 | DDBLTFX_ROTATE270 | DDBLTFX_ROTATE90))))
		{
			Logging::Log() << __FUNCTION__ << " Rotation operations Not Implemented";
			return DDERR_NOROTATIONHW;
		}

		// Check for FX mirror flags
		if ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & (DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN)))
		{
			Logging::Log() << __FUNCTION__ << " Mirror operations Not Implemented";
			return DDERR_NOMIRRORHW;
		}

		// Check for FX flag
		if ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & DDBLTFX_ARITHSTRETCHY))
		{
			Logging::Log() << __FUNCTION__ << " Stretch operations Not Implemented";
			return DDERR_NOSTRETCHHW;
		}

		// Unused flags (can be safely ignored?)
		// DDBLT_ALPHA
		// DDBLT_ASYNC
		// DDBLT_DONOTWAIT
		// DDBLT_WAIT
		// DDBLTFX_NOTEARING

		// Do color fill
		if (dwFlags & DDBLT_COLORFILL)
		{
			return ColorFill(lpDestRect, lpDDBltFx->dwFillColor);
		}

		// Check for device
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_GENERIC;
		}

		// Make sure surface exists, if not then create it
		if (!CheckD3d9Surface())
		{
			Logging::Log() << __FUNCTION__ << " Error surface does not exist!";
			return DDERR_GENERIC;
		}

		// Check if source Surface exists
		m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = (m_IDirectDrawSurfaceX*)lpDDSrcSurface;
		if (!lpDDSrcSurfaceX)
		{
			lpDDSrcSurfaceX = this;
		}
		else
		{
			lpDDSrcSurfaceX = ((m_IDirectDrawSurface*)lpDDSrcSurfaceX)->GetWrapperInterface();

			if (!ddrawParent->DoesSurfaceExist(lpDDSrcSurfaceX))
			{
				Logging::Log() << __FUNCTION__ << " Error, could not find source surface";
				return DDERR_INVALIDPARAMS;
			}
		}

		RECT SrcRect = { 0, 0, (LONG)lpDDSrcSurfaceX->GetWidth(), (LONG)lpDDSrcSurfaceX->GetHeight() };
		RECT DestRect = { 0, 0, (LONG)GetWidth(), (LONG)GetHeight() };

		if (lpSrcRect)
		{
			memcpy(&SrcRect, lpSrcRect, sizeof(RECT));
		}

		if (lpDestRect)
		{
			memcpy(&DestRect, lpDestRect, sizeof(RECT));
		}

		// Check if the rect should be stretched before clipping
		bool isStretchRect = (abs((DestRect.right - DestRect.left) - (SrcRect.right - SrcRect.left)) > 1 || abs((DestRect.bottom - DestRect.top) - (SrcRect.bottom - SrcRect.top)) > 1);

		// Check if the scene needs to be presented
		bool isSkipScene = (DestRect.bottom - DestRect.top < 2 || DestRect.right - DestRect.left < 2 || SrcRect.bottom - SrcRect.top < 2 || SrcRect.right - SrcRect.left < 2);

		// Check and copy source and destination rect and do clipping
		if (!lpDDSrcSurfaceX->UpdateRect(&SrcRect, lpSrcRect) || !UpdateRect(&DestRect, lpDestRect))
		{
			Logging::Log() << __FUNCTION__ << " Error, invalid rect size";
			return DDERR_INVALIDRECT;
		}

		// Check if destination surface is not locked then lock it
		bool UnlockDest = false;
		if (!IsSurfaceLocked())
		{
			HRESULT hr = SetLock(nullptr, 0, true);
			if (FAILED(hr))
			{
				Logging::Log() << __FUNCTION__ << " Error, could not lock dest surface";
				return DDERR_GENERIC;
			}
			UnlockDest = true;
		}

		// Destination and source variables
		D3DLOCKED_RECT DestLockRect, SrcLockRect;
		D3DFORMAT DestFormat, SrcFormat;
		DWORD DestBitCount, SrcBitCount;

		// Get destination surface information
		HRESULT hr = GetSurfaceInfo(&DestLockRect, &DestBitCount, &DestFormat);

		// Check source surface
		bool UnlockSrc = false;
		if (SUCCEEDED(hr) && lpDDSrcSurfaceX != this)
		{
			// Check if source surface is not locked then lock it
			if (!lpDDSrcSurfaceX->IsSurfaceLocked())
			{
				hr = lpDDSrcSurfaceX->SetLock(nullptr, 0, true);
				if (FAILED(hr))
				{
					Logging::Log() << __FUNCTION__ << " Error, could not lock src surface";
					hr = DDERR_GENERIC;
				}
				else
				{
					UnlockSrc = true;
				}
			}

			// Get source surface information
			hr = lpDDSrcSurfaceX->GetSurfaceInfo(&SrcLockRect, &SrcBitCount, &SrcFormat);
		}
		else
		{
			SrcLockRect.pBits = DestLockRect.pBits;
			SrcLockRect.Pitch = DestLockRect.Pitch;
			SrcBitCount = DestBitCount;
			SrcFormat = DestFormat;
		}

		// Copy rect
		if (SUCCEEDED(hr))
		{
			// Strect rect and color key
			if (isStretchRect && (dwFlags & (DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_KEYDEST | DDBLT_KEYSRC)))
			{
				Logging::Log() << __FUNCTION__ << " stretch rect plus color key not imlpemented";
				hr = DDERR_GENERIC;
			}
			// Check for color key flags
			else if (dwFlags & (DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_KEYDEST | DDBLT_KEYSRC))
			{
				// Check if color key is set
				if (((dwFlags & DDBLT_KEYDEST) && !ColorKeys[0].IsSet) || ((dwFlags & DDBLT_KEYSRC) && !lpDDSrcSurfaceX->ColorKeys[2].IsSet))
				{
					Logging::Log() << __FUNCTION__ << " Error color key not set";
				}

				// Get color key
				DDCOLORKEY ColorKey = { 0, 0 };
				ColorKey = (dwFlags & DDBLT_KEYDESTOVERRIDE) ? lpDDBltFx->ddckDestColorkey :
					(dwFlags & DDBLT_KEYSRCOVERRIDE) ? lpDDBltFx->ddckSrcColorkey :
					(dwFlags & DDBLT_KEYDEST) ? ColorKeys[0].Key :
					(dwFlags & DDBLT_KEYSRC) ? lpDDSrcSurfaceX->ColorKeys[2].Key : ColorKey;

				// copy rect using color key
				hr = CopyRectColorKey(&DestLockRect, &DestRect, DestBitCount, DestFormat, &SrcLockRect, &SrcRect, SrcBitCount, SrcFormat, ColorKey);
				if (FAILED(hr))
				{
					Logging::Log() << __FUNCTION__ << " failed to copy rect";
				}
			}
			// Strect rect
			else if (isStretchRect)
			{
				hr = StretchRect(&DestLockRect, &DestRect, DestBitCount, DestFormat, &SrcLockRect, &SrcRect, SrcBitCount, SrcFormat);
				if (FAILED(hr))
				{
					Logging::Log() << __FUNCTION__ << " failed to strect rect";
				}
			}
			// Normal copy rect
			else
			{
				hr = CopyRect(&DestLockRect, &DestRect, DestBitCount, DestFormat, &SrcLockRect, &SrcRect, SrcBitCount, SrcFormat);
				if (FAILED(hr))
				{
					Logging::Log() << __FUNCTION__ << " failed to copy rect";
				}
			}
		}

		// Unlock surfaces if needed
		if (UnlockDest)
		{
			SetUnLock(true);
		}
		if (UnlockSrc)
		{
			lpDDSrcSurfaceX->SetUnLock(true);
		}

		// EndScene
		if (IsPrimarySurface() && !isSkipScene)
		{
			ddrawParent->EndScene();
		}

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
			hr = Blt(lpDDBltBatch[x].lprDest, (LPDIRECTDRAWSURFACE7)lpDDBltBatch[x].lpDDSSrc, lpDDBltBatch[x].lprSrc, lpDDBltBatch[x].dwFlags, lpDDBltBatch[x].lpDDBltFx);
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
		lpDDSrcSurfaceX->UpdateRect(&SrcRect, lpSrcRect);

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

		if (DoesAttachedSurfaceExist(lpAttachedSurface))
		{
			RemoveAttachedSurfaceFromMap(lpAttachedSurface);

			return DD_OK;
		}
		else
		{
			return DDERR_SURFACENOTATTACHED;
		}
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
			it.second->GetSurfaceDesc2(&Desc2);
			if (lpEnumSurfacesCallback7((LPDIRECTDRAWSURFACE7)it.second->GetWrapperInterfaceX(DirectXVersion), &Desc2, lpContext) == DDENUMRET_CANCEL)
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
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return DDERR_UNSUPPORTED;
	}

	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback7 = lpfnCallback7;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ConvertSurfaceDescTo2 = (ProxyDirectXVersion > 3 && DirectXVersion < 4);

	return ProxyInterface->EnumOverlayZOrders(dwFlags, &CallbackContext, m_IDirectDrawEnumSurface::ConvertCallback2);
}

void m_IDirectDrawSurfaceX::SwapSurface(m_IDirectDrawSurfaceX *lpTargetSurface1, m_IDirectDrawSurfaceX *lpTargetSurface2)
{
	if (!lpTargetSurface1 || !lpTargetSurface2 || lpTargetSurface1 == lpTargetSurface2)
	{
		return;
	}

	// Swap textures
	SwapAddresses(lpTargetSurface1->GetSurfaceTexture(), lpTargetSurface2->GetSurfaceTexture());
}

HRESULT m_IDirectDrawSurfaceX::Flip(LPDIRECTDRAWSURFACE7 lpDDSurfaceTargetOverride, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Check for device
		if (!d3d9Device || !*d3d9Device || !ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " D3d9 Device not setup.";
			return DDERR_GENERIC;
		}

		// Make sure surface exists, if not then create it
		if (!CheckD3d9Surface())
		{
			Logging::Log() << __FUNCTION__ << " Error surface does not exist!";
			return DDERR_GENERIC;
		}

		// Flip only supported from primary surface
		if (!IsPrimarySurface())
		{
			Logging::Log() << __FUNCTION__ << " Attempting to Flip non-primary surface";
			return DDERR_GENERIC;
		}

		// Unneeded flags (can be safely ignored?)
		// Note: vsync handled by d3d9 PresentationInterval
		// - DDFLIP_DONOTWAIT
		// - DDFLIP_NOVSYNC
		// - DDFLIP_WAIT

		if ((dwFlags & (DDFLIP_INTERVAL2 | DDFLIP_INTERVAL3 | DDFLIP_INTERVAL4)) && (surfaceDesc2.ddsCaps.dwCaps2 & DDCAPS2_FLIPINTERVAL))
		{
			Logging::Log() << __FUNCTION__ << " Interval flipping not implemented";
		}

		// Check if attached surface is found
		bool FoundAttachedSurface = false;

		// If SurfaceTargetOverride then use that surface
		if (lpDDSurfaceTargetOverride)
		{
			m_IDirectDrawSurfaceX *lpTargetSurface = ((m_IDirectDrawSurface*)lpDDSurfaceTargetOverride)->GetWrapperInterface();

			// Check if target surface exists
			if (!DoesAttachedSurfaceExist(lpTargetSurface) || lpTargetSurface == this)
			{
				Logging::Log() << __FUNCTION__ << " Invalid SurfaceTarget";
				return DDERR_INVALIDPARAMS;
			}

			// Found surface
			FoundAttachedSurface = true;

			// Swap surface
			SwapSurface(this, lpTargetSurface);
		}

		// Execute flip for all attached surfaces
		else
		{
			if ((dwFlags & DDFLIP_ODD) && (dwFlags & DDFLIP_EVEN))
			{
				return DDERR_INVALIDPARAMS;
			}

			if (dwFlags & DDFLIP_STEREO)
			{
				Logging::Log() << __FUNCTION__ << " Stereo flipping not implemented";
				return DDERR_NOSTEREOHARDWARE;
			}

			if (dwFlags & (DDFLIP_ODD | DDFLIP_EVEN))
			{
				Logging::Log() << __FUNCTION__ << " Even and odd flipping not implemented";
				return DDERR_UNSUPPORTED;
			}

			// Loop through each surface and swap them
			for (auto it : AttachedSurfaceMap)
			{
				m_IDirectDrawSurfaceX *lpTargetSurface = (m_IDirectDrawSurfaceX*)it.second;

				// Found surface
				FoundAttachedSurface = true;

				// Swap surface
				SwapSurface(this, lpTargetSurface);
			}
		}

		// Return error if no attached surfaces found
		if (!FoundAttachedSurface)
		{
			Logging::Log() << __FUNCTION__ << " No attached surfaces found";
			return DDERR_GENERIC;
		}

		// Set new texture
		HRESULT hr = (*d3d9Device)->SetTexture(0, (paletteTexture) ? paletteTexture : surfaceTexture);
		if (FAILED(hr))
		{
			Logging::Log() << __FUNCTION__ << " Failed to set texture";
		}

		if (attachedPalette)
		{
			WritePaletteToSurface();
		}

		ddrawParent->EndScene();

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
		if (!lplpDDAttachedSurface)
		{
			return DDERR_INVALIDPARAMS;
		}

		LOG_LIMIT(1, __FUNCTION__<< " Not fully Implemented.");

		// Check for device
		if (!d3d9Device || !*d3d9Device || !ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " D3d9 Device not setup.";
			return DDERR_GENERIC;
		}

		/*ToDo: GetAttachedSurface fails if more than one surface is attached that matches the capabilities requested. 
		In this case, the application must use the IDirectDrawSurface7::EnumAttachedSurfaces method to obtain the attached surfaces.*/

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		ConvertSurfaceDesc(Desc2, surfaceDesc2);
		if (lpDDSCaps2)
		{
			ConvertCaps(Desc2.ddsCaps, *lpDDSCaps2);
		}
		Desc2.ddsCaps.dwCaps &= ~DDSCAPS_PRIMARYSURFACE;		// Remove Primary surface flag

		m_IDirectDrawSurfaceX *attachedSurface = new m_IDirectDrawSurfaceX(d3d9Device, ddrawParent, DirectXVersion, &Desc2, displayWidth, displayHeight);

		*lplpDDAttachedSurface = (LPDIRECTDRAWSURFACE7)attachedSurface->GetWrapperInterfaceX(DirectXVersion);

		AddAttachedSurfaceToMap(attachedSurface);

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
		LOG_LIMIT(1, __FUNCTION__ << " Not fully Implemented.");

		// Inquires whether a blit involving this surface can occur immediately, and returns DD_OK if the blit can be completed.
		if (dwFlags & DDGBS_CANBLT)
		{
			if (!IsSurfaceLocked())
			{
				//return DDERR_WASSTILLDRAWING;
			}
			return DD_OK;
		}
		// Inquires whether the blit is done, and returns DD_OK if the last blit on this surface has completed.
		else if (dwFlags & DDGBS_ISBLTDONE)
		{
			if (!IsSurfaceLocked())
			{
				//return DDERR_WASSTILLDRAWING;
			}
			return DD_OK;
		}

		return DDERR_UNSUPPORTED;
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
		int x = (dwFlags & DDCKEY_DESTBLT) ? 0 :
			(dwFlags & DDCKEY_DESTOVERLAY) ? 1 :
			(dwFlags & DDCKEY_SRCBLT) ? 2 :
			(dwFlags & DDCKEY_SRCOVERLAY) ? 3 : -1;

		// Check index
		if (!lpDDColorKey || x == -1)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check if color key is set
		if (!ColorKeys[x].IsSet)
		{
			return DDERR_NOCOLORKEY;
		}

		// Copy color key
		memcpy(lpDDColorKey, &ColorKeys[x].Key, sizeof(DDCOLORKEY));

		// Return
		return DD_OK;
	}

	return ProxyInterface->GetColorKey(dwFlags, lpDDColorKey);
}

HRESULT m_IDirectDrawSurfaceX::GetDC(HDC FAR * lphDC)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		LOG_LIMIT(1, __FUNCTION__ << " Not fully Implemented.");

		if (!lphDC)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_GENERIC;
		}

		*lphDC = ::GetDC(ddrawParent->GetHwnd());

		if (!*lphDC)
		{
			return DDERR_GENERIC;
		}

		return DD_OK;
	}

	return ProxyInterface->GetDC(lphDC);
}

HRESULT m_IDirectDrawSurfaceX::GetFlipStatus(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		LOG_LIMIT(1, __FUNCTION__ << " Not fully Implemented.");

		// Queries whether the surface can flip now. The method returns DD_OK if the flip can be completed.
		if (dwFlags & DDGFS_CANFLIP)
		{
			return DD_OK;
		}
		// Queries whether the flip is done. The method returns DD_OK if the last flip on this surface has completed.
		else if (dwFlags & DDGFS_ISFLIPDONE)
		{
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
		LOG_LIMIT(1, __FUNCTION__ << " Not fully Implemented.");

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

		// Check for device
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
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

		// Check for device
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
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
		if ((lpDDSurfaceDesc2->dwFlags & DDSD_REFRESHRATE) == 0)
		{
			if ((Desc2.dwFlags & DDSD_REFRESHRATE) == 0)
			{
				ddrawParent->GetDisplayMode2(&Desc2);
			}
			lpDDSurfaceDesc2->dwFlags |= DDSD_REFRESHRATE;
			lpDDSurfaceDesc2->dwRefreshRate = Desc2.dwRefreshRate;
		}
		// Set PixelFormat
		if ((lpDDSurfaceDesc2->dwFlags & DDSD_PIXELFORMAT) == 0)
		{
			if ((Desc2.dwFlags & DDSD_PIXELFORMAT) == 0)
			{
				ddrawParent->GetDisplayMode2(&Desc2);
			}

			// Set PixelFormat flags
			lpDDSurfaceDesc2->dwFlags |= DDSD_PIXELFORMAT;
			lpDDSurfaceDesc2->ddpfPixelFormat.dwFlags = DDPF_RGB;

			// Set BitCount
			lpDDSurfaceDesc2->ddpfPixelFormat.dwRGBBitCount = Desc2.ddpfPixelFormat.dwRGBBitCount;

			// Set BitMask
			switch (lpDDSurfaceDesc2->ddpfPixelFormat.dwRGBBitCount)
			{
			case 8:
				lpDDSurfaceDesc2->ddpfPixelFormat.dwRBitMask = 0;
				lpDDSurfaceDesc2->ddpfPixelFormat.dwGBitMask = 0;
				lpDDSurfaceDesc2->ddpfPixelFormat.dwBBitMask = 0;
				break;
			case 16:
				GetPixelDisplayFormat(D3DFMT_R5G6B5, lpDDSurfaceDesc2->ddpfPixelFormat);
				break;
			case 24:
			case 32:
				GetPixelDisplayFormat(D3DFMT_X8R8G8B8, lpDDSurfaceDesc2->ddpfPixelFormat);
				break;
			default:
				Logging::Log() << __FUNCTION__ << " Not implemented bit count " << lpDDSurfaceDesc2->ddpfPixelFormat.dwRGBBitCount;
				return DDERR_UNSUPPORTED;
			}
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
		// Check if locked
		if (IsSurfaceLocked() && d3dlrect.pBits)
		{
			LOG_LIMIT(1, __FUNCTION__ << " Locking surface twice not fully implemented");
		}

		// Save Lock Rect
		if (lpDestRect)
		{
			memcpy(&lkDestRect, lpDestRect, sizeof(RECT));
		}
		else
		{
			lkDestRect.top = 0;
			lkDestRect.left = 0;
			lkDestRect.right = surfaceDesc2.dwWidth;
			lkDestRect.bottom = surfaceDesc2.dwHeight;
		}

		// Convert flags to d3d9
		DWORD Flags = dwFlags & (D3DLOCK_READONLY | (!IsPrimarySurface() ? DDLOCK_NOSYSLOCK : 0) | D3DLOCK_DISCARD);

		// Lock surface
		HRESULT hr = SetLock(lpDestRect, Flags);

		// Set desc and video memory
		if (SUCCEEDED(hr))
		{
			// Copy surfaceDesc into passed into local varable
			DDSURFACEDESC2 Desc2;
			Desc2.dwSize = sizeof(DDSURFACEDESC2);
			Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
			ConvertSurfaceDesc(Desc2, surfaceDesc2);

			// Set video memory and pitch
			if (d3dlrect.pBits)
			{
				Desc2.dwFlags |= DDSD_LPSURFACE | DDSD_PITCH;
				Desc2.lpSurface = d3dlrect.pBits;
				Desc2.lPitch = d3dlrect.Pitch;
			}
			else
			{
				LOG_LIMIT(1, __FUNCTION__ << " Failed to write to texture surface!");
				hr = DDERR_GENERIC;
			}

			if (lpDDSurfaceDesc2)
			{
				ConvertSurfaceDesc(*lpDDSurfaceDesc2, Desc2);
			}
		}
		else
		{
			LOG_LIMIT(1, __FUNCTION__ << " Failed to lock texture surface!");
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
		LOG_LIMIT(1, __FUNCTION__ << " Not fully Implemented.");

		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_GENERIC;
		}

		if (::ReleaseDC(ddrawParent->GetHwnd(), hDC) == 0)
		{
			return DDERR_GENERIC;
		}

		return DD_OK;
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
		if (!lpDDColorKey)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Get color key index
		int x = (dwFlags & DDCKEY_DESTBLT) ? 0 :
			(dwFlags & DDCKEY_DESTOVERLAY) ? 1 :
			(dwFlags & DDCKEY_SRCBLT) ? 2 :
			(dwFlags & DDCKEY_SRCOVERLAY) ? 3 : -1;

		// Check index
		if (x == -1)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Set color key
		if (!lpDDColorKey)
		{
			ColorKeys[x].IsSet = false;
		}
		else
		{
			ColorKeys[x].IsSet = true;
			ColorKeys[x].IsColorSpace = ((dwFlags & DDCKEY_COLORSPACE) != 0);
			memcpy(&ColorKeys[x].Key, lpDDColorKey, sizeof(DDCOLORKEY));
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
		LOG_LIMIT(1, __FUNCTION__ << " Not fully Implemented.");

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

		// Don't write to surface when using palettes
		if (!paletteTexture)
		{
			CreateD3d9Surface();
		}

		// When you call SetPalette to set a palette to a surface for the first time, 
		// SetPalette increments the palette's reference count; subsequent calls to 
		// SetPalette do not affect the palette's reference count.
		if (PaletteFirstRun)
		{
			attachedPalette->AddRef();
			PaletteFirstRun = false;
		}

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
		// If no video memory than nothing to do...
		if (!d3dlrect.pBits)
		{
			Logging::Log() << __FUNCTION__ << " No video memory!";
			return DD_OK;
		}

		// Unlock surface
		return SetUnLock();
	}

	return ProxyInterface->Unlock(lpRect);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlay(LPRECT lpSrcRect, LPDIRECTDRAWSURFACE7 lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, LPDDOVERLAYFX lpDDOverlayFx)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
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
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->UpdateOverlayDisplay(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlayZOrder(DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSReference)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
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

		// Check for device
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
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

		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return DDERR_UNSUPPORTED;
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
		// Make sure surface exists, if not then create it
		if (!CheckD3d9Surface())
		{
			Logging::Log() << __FUNCTION__ << " Error surface does not exist!";
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
		// Make sure surface exists, if not then create it
		if (!CheckD3d9Surface())
		{
			Logging::Log() << __FUNCTION__ << " Error surface does not exist!";
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
		// Make sure surface exists, if not then create it
		if (!CheckD3d9Surface())
		{
			Logging::Log() << __FUNCTION__ << " Error surface does not exist!";
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
		// Make sure surface exists, if not then create it
		if (!CheckD3d9Surface())
		{
			Logging::Log() << __FUNCTION__ << " Error surface does not exist!";
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

		// Make sure surface exists, if not then create it
		if (!CheckD3d9Surface())
		{
			Logging::Log() << __FUNCTION__ << " Error surface does not exist!";
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
		// Make sure surface exists, if not then create it
		if (!CheckD3d9Surface())
		{
			Logging::Log() << __FUNCTION__ << " Error surface does not exist!";
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

		// Make sure surface exists, if not then create it
		if (!CheckD3d9Surface())
		{
			Logging::Log() << __FUNCTION__ << " Error surface does not exist!";
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

bool m_IDirectDrawSurfaceX::CheckD3d9Surface()
{
	// Make sure surface exists, if not then create it
	if (!surfaceTexture || (attachedPalette && !paletteTexture))
	{
		if (FAILED(CreateD3d9Surface()))
		{
			return false;
		}
	}
	return true;
}

// Create surface
HRESULT m_IDirectDrawSurfaceX::CreateD3d9Surface()
{
	// Don't recreate surface while it is locked
	if (surfaceTexture && IsSurfaceLocked())
	{
		Logging::Log() << __FUNCTION__ << " Error: surface is locked!";
		return DDERR_GENERIC;
	}

	// Release existing surface
	ReleaseD9Surface();

	// Check for device
	if (!ddrawParent)
	{
		Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
		return DDERR_GENERIC;
	}

	// Check for device, if not then create it
	if (!d3d9Device || !*d3d9Device)
	{
		ddrawParent->CreateD3D9Device();
	}

	// Check for device
	if (!d3d9Device || !*d3d9Device)
	{
		Logging::Log() << __FUNCTION__ << " D3d9 Device not setup.";
		return DDERR_GENERIC;
	}

	// Update surface description
	GetSurfaceDesc2(&surfaceDesc2);

	// Get usage
	DWORD Usage = 0;
	if ((surfaceDesc2.ddsCaps.dwCaps2 & DDSCAPS2_HINTSTATIC) == 0 || (surfaceDesc2.ddsCaps.dwCaps2 & DDSCAPS2_HINTDYNAMIC))
	{
		Usage |= D3DUSAGE_DYNAMIC;
	}
	if (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_MIPMAP)
	{
		Usage |= D3DUSAGE_AUTOGENMIPMAP;
	}
	if (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_WRITEONLY)
	{
		Usage |= D3DUSAGE_WRITEONLY;
	}

	// Get pool type
	D3DPOOL Pool = D3DPOOL_DEFAULT;
	if (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
	{
		Pool = D3DPOOL_SYSTEMMEM;
	}
	else if (((surfaceDesc2.ddsCaps.dwCaps & (DDSCAPS_VIDEOMEMORY | DDSCAPS_NONLOCALVIDMEM)) || (surfaceDesc2.ddsCaps.dwCaps2 & (DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE))) &&
		((Usage & D3DUSAGE_DYNAMIC) == 0))		// D3DPOOL_MANAGED cannot be used with D3DUSAGE_DYNAMIC
	{
		Pool = D3DPOOL_MANAGED;
	}
	else if (surfaceDesc2.ddsCaps.dwCaps2 & DDSCAPS2_DONOTPERSIST)
	{
		Pool = D3DPOOL_SCRATCH;
	}

	// Get resource type
	// DDSCAPS_BACKBUFFER
	// DDSCAPS_FRONTBUFFER
	// DDSCAPS_OFFSCREENPLAIN
	// DDSCAPS_PRIMARYSURFACE
	// DDSCAPS_TEXTURE
	// DDSCAPS2_CUBEMAP

	// Other flags
	// DDSCAPS_ALPHA
	// DDSCAPS_COMPLEX
	// DDSCAPS_FLIP
	// DDSCAPS_HWCODEC
	// DDSCAPS_LIVEVIDEO
	// DDSCAPS_MODEX
	// DDSCAPS_OVERLAY
	// DDSCAPS_PALETTE
	// DDSCAPS_VIDEOPORT
	// DDSCAPS_VISIBLE
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

	// Get format
	D3DFORMAT Format = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);

	Logging::LogDebug() << __FUNCTION__ << " D3d9 Surface size: " << surfaceDesc2.dwWidth << "x" << surfaceDesc2.dwHeight << " Usage: " << Usage << " Format: " << Format << " Pool: " << Pool;

	// Create surface
	if (FAILED((*d3d9Device)->CreateTexture(surfaceDesc2.dwWidth, surfaceDesc2.dwHeight, 1, Usage, Format, Pool, &surfaceTexture, nullptr)))
	{
		Logging::Log() << __FUNCTION__ << " Error: Unable to create surface";
		return DDERR_GENERIC;
	}

	if (attachedPalette)
	{
		// Create palette surface
		if (FAILED((*d3d9Device)->CreateTexture(surfaceDesc2.dwWidth, surfaceDesc2.dwHeight, 1, Usage, D3DFMT_A8R8G8B8, Pool, &paletteTexture, nullptr)))
		{
			Logging::Log() << __FUNCTION__ << " Error: Unable to create palette surface";
			return DDERR_GENERIC;
		}
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
		Logging::Log() << __FUNCTION__ << " Unable to set vertex shader";
		return DDERR_GENERIC;
	}

	// Set fv format
	if (FAILED((*d3d9Device)->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1)))
	{
		Logging::Log() << __FUNCTION__ << " Unable to set the current vertex stream format";
		return DDERR_GENERIC;
	}

	// Create vertex buffer
	if (FAILED((*d3d9Device)->CreateVertexBuffer(sizeof(TLVERTEX) * 4, D3DUSAGE_DYNAMIC, (D3DFVF_XYZRHW | D3DFVF_TEX1), D3DPOOL_DEFAULT, &vertexBuffer, nullptr)))
	{
		Logging::Log() << __FUNCTION__ << " Unable to create vertex buffer";
		return DDERR_GENERIC;
	}

	// Set stream source
	if (FAILED((*d3d9Device)->SetStreamSource(0, vertexBuffer, 0, sizeof(TLVERTEX))))
	{
		Logging::Log() << __FUNCTION__ << " Unable to set vertex buffer stream source";
		return DDERR_GENERIC;
	}

	// Set render states(no lighting)
	if (FAILED((*d3d9Device)->SetRenderState(D3DRS_LIGHTING, FALSE)))
	{
		Logging::Log() << __FUNCTION__ << " Unable to set device render state(no lighting)";
		return DDERR_GENERIC;
	}

	// Set scale mode to linear
	if (FAILED((*d3d9Device)->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR)))
	{
		Logging::Log() << __FUNCTION__ << " Failed to set D3D device to LINEAR sampling";
	}

	// Setup verticies (0,0,currentWidth,currentHeight)
	TLVERTEX* vertices;

	// Lock vertex buffer
	if (FAILED(vertexBuffer->Lock(0, 0, (void**)&vertices, 0)))
	{
		Logging::Log() << __FUNCTION__ << " Unable to lock vertex buffer";
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

	Logging::LogDebug() << __FUNCTION__ << " D3d9 Vertex size: " << BackBufferWidth << "x" << BackBufferHeight;
	// Set vertex points
	// 0, 0
	vertices[0].x = -0.5f;
	vertices[0].y = -0.5f;
	vertices[0].z = 0.0f;
	vertices[0].rhw = 1.0f;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;

	// currentWidth, 0
	vertices[1].x = (float)BackBufferWidth - 0.5f;
	vertices[1].y = -0.5f;
	vertices[1].z = 0.0f;
	vertices[1].rhw = 1.0f;
	vertices[1].u = 1.0f;
	vertices[1].v = 0.0f;

	// currentWidth, scaledHeight
	vertices[2].x = (float)BackBufferWidth - 0.5f;
	vertices[2].y = (float)BackBufferHeight - 0.5f;
	vertices[2].z = 0.0f;
	vertices[2].rhw = 1.0f;
	vertices[2].u = 1.0f;
	vertices[2].v = 1.0f;

	// 0, currentHeight
	vertices[3].x = -0.5f;
	vertices[3].y = (float)BackBufferHeight - 0.5f;
	vertices[3].z = 0.0f;
	vertices[3].rhw = 1.0f;
	vertices[3].u = 0.0f;
	vertices[3].v = 1.0f;

	// Unlcok vertex buffer
	if (FAILED(vertexBuffer->Unlock()))
	{
		Logging::Log() << __FUNCTION__ << " Unable to unlock vertex buffer";
		return DDERR_GENERIC;
	}

	// Set texture
	if (FAILED((*d3d9Device)->SetTexture(0, (paletteTexture) ? paletteTexture : surfaceTexture)))
	{
		Logging::Log() << __FUNCTION__ << " Failed to set texture";
		return DDERR_GENERIC;
	}

	return DD_OK;
}

template <typename T>
void m_IDirectDrawSurfaceX::ReleaseD9Interface(T *ppInterface)
{
	if (ppInterface && *ppInterface)
	{
		DWORD x = 0, z = 0;
		do
		{
			z = (*ppInterface)->Release();
		} while (z != 0 && ++x < 100);

		// Add error checking
		if (z != 0)
		{
			Logging::Log() << __FUNCTION__ << " Unable to release Direct3D9 interface";
		}

		(*ppInterface) = nullptr;
	}
}

// Release surface and vertext buffer
void m_IDirectDrawSurfaceX::ReleaseD9Surface()
{
	// Release d3d9 surface
	Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 texture surface";
	if (surfaceTexture)
	{
		surfaceTexture->UnlockRect(0);
	}
	ReleaseD9Interface(&surfaceTexture);

	// Release d3d9 palette surface
	Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 palette texture surface";
	if (paletteTexture)
	{
		paletteTexture->UnlockRect(0);
	}
	ReleaseD9Interface(&paletteTexture);

	// Release d3d9 vertex buffer
	Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 vertext buffer";
	ReleaseD9Interface(&vertexBuffer);

	// Set surface video buffer to nullptr
	d3dlrect.pBits = nullptr;

	// Set unlock
	IsLocked = false;
}

// Check surface reck dimensions and copy rect to new rect
bool m_IDirectDrawSurfaceX::UpdateRect(LPRECT lpOutRect, LPRECT lpInRect)
{
	if (!lpOutRect)
	{
		return false;
	}

	// Make sure surface exists, if not then create it
	if (!CheckD3d9Surface())
	{
		Logging::Log() << __FUNCTION__ << " Error surface does not exist!";
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

	return lpOutRect->left <= lpOutRect->right && lpOutRect->top <= lpOutRect->bottom;
}

// Lock the d3d9 surface
HRESULT m_IDirectDrawSurfaceX::SetLock(LPRECT lpDestRect, DWORD dwFlags, bool isSkipScene)
{
	// Check for device
	if (!d3d9Device || !*d3d9Device || !ddrawParent)
	{
		Logging::Log() << __FUNCTION__ << " D3d9 Device not setup.";
		return DDERR_GENERIC;
	}

	// Make sure surface exists, if not then create it
	if (!CheckD3d9Surface())
	{
		Logging::Log() << __FUNCTION__ << " Error surface does not exist!";
		return DDERR_GENERIC;
	}

	// Run BeginScene (ignore results)
	if (!isSkipScene)
	{
		ddrawParent->BeginScene();
	}

	// Lock surface
	if (FAILED(surfaceTexture->LockRect(0, &d3dlrect, lpDestRect, dwFlags)))
	{
		d3dlrect.pBits = nullptr;
		Logging::Log() << __FUNCTION__ << " Failed to lock surface";
		return DDERR_GENERIC;
	}

	// Set lock flag
	IsLocked = true;

	// Success
	return DD_OK;
}

// Unlock the d3d9 surface
HRESULT m_IDirectDrawSurfaceX::SetUnLock(bool isSkipScene)
{
	// Check for device
	if (!ddrawParent)
	{
		Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
		return DDERR_GENERIC;
	}

	// Make sure surface exists, if not then create it
	if (!CheckD3d9Surface())
	{
		Logging::Log() << __FUNCTION__ << " Error surface does not exist!";
		return DDERR_GENERIC;
	}

	// Write to palette surface using a palette
	if (IsPrimarySurface() && !isSkipScene && attachedPalette)
	{
		if (FAILED(WritePaletteToSurface()))
		{
			Logging::Log() << __FUNCTION__ << " Failed to write palette data to surface!";
		}
	}

	// Lock surface
	if (FAILED(surfaceTexture->UnlockRect(0)))
	{
		Logging::Log() << __FUNCTION__ << " Failed to unlock surface";
		return DDERR_GENERIC;
	}
	IsLocked = false;
	d3dlrect.pBits = nullptr;

	// Keep running EndScene until it succeeds
	if (IsPrimarySurface() && !isSkipScene)
	{
		ddrawParent->EndScene();
	}

	return DD_OK;
}

// Get LOCKED_RECT, BitCount and Format for the surface
HRESULT m_IDirectDrawSurfaceX::GetSurfaceInfo(D3DLOCKED_RECT *pLockRect, DWORD *lpBitCount, D3DFORMAT *lpFormat)
{
	// Make sure surface exists, if not then create it
	if (!CheckD3d9Surface())
	{
		Logging::Log() << __FUNCTION__ << " Error surface does not exist!";
		return DDERR_GENERIC;
	}

	if (pLockRect)
	{
		if (IsSurfaceLocked())
		{
			memcpy(pLockRect, &d3dlrect, sizeof(D3DLOCKED_RECT));
		}
		else
		{
			Logging::Log() << __FUNCTION__ << " Error, surface needs to be locked!";
			return DDERR_GENERIC;
		}
	}
	if (lpBitCount)
	{
		if (attachedPalette)
		{
			*lpBitCount = 8;
		}
		else
		{
			*lpBitCount = GetBitCount(surfaceDesc2.ddpfPixelFormat);
		}
	}
	if (lpFormat)
	{
		if (attachedPalette)
		{
			*lpFormat = D3DFMT_L8;
		}
		else
		{
			*lpFormat = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);
		}
	}
	return DD_OK;
}

// Add attached surface to map
void m_IDirectDrawSurfaceX::AddAttachedSurfaceToMap(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return;
	}

	// Store surface
	AttachedSurfaceMap[++MapKey] = lpSurfaceX;
}

// Remove attached surface from map
void m_IDirectDrawSurfaceX::RemoveAttachedSurfaceFromMap(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return;
	}

	auto it = std::find_if(AttachedSurfaceMap.begin(), AttachedSurfaceMap.end(),
		[=](auto Map) -> bool { return Map.second == lpSurfaceX; });

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
		[=](auto Map) -> bool { return Map.second == lpSurfaceX; });

	if (it == std::end(AttachedSurfaceMap))
	{
		return false;
	}

	if (!ddrawParent)
	{
		return false;
	}

	return true;
}

HRESULT m_IDirectDrawSurfaceX::ColorFill(RECT *pRect, DWORD dwFillColor)
{
	Logging::LogDebug() << __FUNCTION__;

	// Check and copy rect
	RECT DestRect;
	if (!UpdateRect(&DestRect, pRect))
	{
		Logging::Log() << __FUNCTION__ << " Error, invalid rect size";
		return DDERR_INVALIDRECT;
	}

	// Check if the scene needs to be presented
	bool isSkipScene = (DestRect.bottom - DestRect.top < 2 || DestRect.right - DestRect.left < 2);

	HRESULT hr = DD_OK;
	bool UnlockDest = false;
	do {
		// Check if surface is not locked then lock it
		if (!IsSurfaceLocked())
		{
			if (FAILED(SetLock(nullptr, 0, true)))
			{
				Logging::Log() << __FUNCTION__ << " Error, could not lock dest surface";
				hr = DDERR_GENERIC;
				break;
			}
			UnlockDest = true;
		}

		// Get locked rect surface information
		D3DLOCKED_RECT DestLockRect;
		GetSurfaceInfo(&DestLockRect, nullptr, nullptr);

		// Check pBits
		if (!DestLockRect.pBits)
		{
			Logging::Log() << __FUNCTION__ << " Error, could not get pBits";
			hr = DDERR_GENERIC;
			break;
		}

		// Get width and height of rect
		LONG FillWidth = DestRect.right - DestRect.left;
		LONG FillHeight = DestRect.bottom - DestRect.top;

		// Get ByteCount and FillColor
		DWORD ByteCount = DestLockRect.Pitch / surfaceDesc2.dwWidth;
		DWORD FillColor = 0;
		if (attachedPalette && attachedPalette->rgbPalette)
		{
			// ToDo: Need to fix this!!
			//FillColor = attachedPalette->rgbPalette[rawVideoBuf[dwFillColor]];
			Logging::Log() << __FUNCTION__ << " Error: not implemented for palettes!";
		}
		else if (ByteCount <= 4)
		{
			FillColor = dwFillColor;
		}
		else
		{
			Logging::Log() << __FUNCTION__ << " Error: could not find correct fill color for ByteCount " << ByteCount;
		}

		// Fill rect
		for (LONG y = 0; y < FillHeight; y++)
		{
			DWORD StartLocation = ((y + DestRect.top) * surfaceDesc2.dwWidth) + DestRect.left;
			if (ByteCount == 4)
			{
				memset((DWORD*)DestLockRect.pBits + StartLocation,	// Video memory address
					FillColor,										// Fill color
					FillWidth);										// Size of bytes to write
			}
			else
			{
				for (LONG x = 0; x < FillWidth; x++)
				{
					memcpy((BYTE*)DestLockRect.pBits + ((StartLocation + x) * ByteCount),	// Video memory address
						(BYTE*)&FillColor,													// Fill color
						ByteCount);															// Size of bytes to write
				}
			}
		}

	} while (false);

	// Unlock surfaces if needed
	if (UnlockDest)
	{
		SetUnLock(true);
	}

	// EndScene
	if (IsPrimarySurface() && !isSkipScene)
	{
		ddrawParent->EndScene();
	}

	return DD_OK;
}

// Write palette video data to surface
HRESULT m_IDirectDrawSurfaceX::WritePaletteToSurface()
{
	Logging::LogDebug() << __FUNCTION__;

	if (!CheckD3d9Surface())
	{
		Logging::Log() << __FUNCTION__ << " Error surface does not exist!";
		return DDERR_GENERIC;
	}

	if (!attachedPalette || !attachedPalette->rgbPalette)
	{
		Logging::Log() << __FUNCTION__ << " Error no attached palette!";
		return DDERR_INVALIDPARAMS;
	}

	HRESULT hr = DD_OK;
	bool UnlockDest = false;
	do {
		// Check and copy rect
		RECT DestRect;
		if (!UpdateRect(&DestRect, &lkDestRect))
		{
			Logging::Log() << __FUNCTION__ << " Error, invalid rect size";
			hr = DDERR_INVALIDRECT;
			break;
		}

		// Check if surface is not locked then lock it
		if (!IsSurfaceLocked())
		{
			if (FAILED(SetLock(&DestRect, 0, true)) || !d3dlrect.pBits)
			{
				Logging::Log() << __FUNCTION__ << " Error, could not lock source surface";
				hr = DDERR_GENERIC;
				break;
			}
			UnlockDest = true;
		}

		// Lock palette surface to write RGB data from the palette
		D3DLOCKED_RECT LockedRect = { NULL };
		if (FAILED(paletteTexture->LockRect(0, &LockedRect, &DestRect, 0)) || !LockedRect.pBits)
		{
			Logging::Log() << __FUNCTION__ << " Error: could not lock palatte video data!";
			hr = DDERR_INVALIDPARAMS;
			break;
		}

		// Create raw video memory and rgb buffer variables
		UINT32 *surfaceBuffer = (UINT32*)LockedRect.pBits;
		BYTE *videoBuffer = (BYTE*)d3dlrect.pBits;

		DWORD BitCount = GetBitCount(surfaceDesc2.ddpfPixelFormat);

		// Translate palette to rgb video buffer
		switch (BitCount)
		{
		case 8:
		{
			LONG surfacePitch = LockedRect.Pitch / 4;
			for (LONG j = DestRect.top; j < DestRect.bottom; j++)
			{
				for (LONG i = DestRect.left; i < DestRect.right; i++)
				{
					surfaceBuffer[i] = attachedPalette->rgbPalette[videoBuffer[i]];
				}
				surfaceBuffer += surfacePitch;
				videoBuffer += d3dlrect.Pitch;
			}
			break;
		}
		default:
			Logging::Log() << __FUNCTION__ << " No support for palette on " << BitCount << "-bit surfaces!";
			break;
		}

	} while (false);

	// Unlock palette display texture
	paletteTexture->UnlockRect(0);

	// Unlock surfaces if needed
	if (UnlockDest)
	{
		SetUnLock(true);
	}

	return DD_OK;
}

// Copy surface rect
HRESULT m_IDirectDrawSurfaceX::CopyRect(D3DLOCKED_RECT *pDestLockRect, RECT *pDestRect, DWORD DestBitCount, D3DFORMAT DestFormat, D3DLOCKED_RECT *pSrcLockRect, RECT *pSrcRect, DWORD SrcBitCount, D3DFORMAT SrcFormat)
{
	Logging::LogDebug() << __FUNCTION__;

	// Check destination parameters
	if (!pDestLockRect || !pDestLockRect->pBits || !pDestRect || !DestBitCount || DestFormat == D3DFMT_UNKNOWN)
	{
		Logging::Log() << __FUNCTION__ << " Invalid destination parameters";
		return DDERR_INVALIDPARAMS;
	}

	// Check source parameters
	if (!pSrcLockRect || !pSrcLockRect->pBits || !pSrcRect || !SrcBitCount || SrcFormat == D3DFMT_UNKNOWN)
	{
		Logging::Log() << __FUNCTION__ << " Invalid source parameters";
		return DDERR_INVALIDPARAMS;
	}

	// Get width and height of rect
	LONG RectWidth = min(pDestRect->right - pDestRect->left, pSrcRect->right - pSrcRect->left);
	LONG RectHeight = min(pDestRect->bottom - pDestRect->top, pSrcRect->bottom - pSrcRect->top);

	// Check if source and destination formats are the same
	if (DestFormat == SrcFormat)
	{
		// Get byte count
		DWORD ByteCount = DestBitCount / 8;
		if (!ByteCount || ByteCount * 8 != DestBitCount)
		{
			Logging::Log() << __FUNCTION__ << " Invalid bit count " << DestBitCount;
			return DDERR_GENERIC;
		}

		// Copy memory
		for (LONG y = 0; y < RectHeight; y++)
		{
			memcpy((BYTE*)pDestLockRect->pBits + ((y + pDestRect->top) * pDestLockRect->Pitch) + (pDestRect->left * ByteCount),	// Destination video memory address
				(BYTE*)pSrcLockRect->pBits + ((y + pSrcRect->top) * pSrcLockRect->Pitch) + (pSrcRect->left * ByteCount),		// Source video memory address
				RectWidth * ByteCount);																							// Size of bytes to write
		}

		// Return
		return DD_OK;
	}

	// Set raw video memory, BitCount and display format
	BYTE *SrcBuffer = (BYTE*)pSrcLockRect->pBits;

	// For destination bit count
	switch (DestBitCount)
	{
	case 32: // 32-bit destination surface
	{
		UINT32 *DestBuffer = (UINT32*)pDestLockRect->pBits;
		// For destiantion format
		switch (DestFormat)
		{
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
			// Translate source buffer to 32-bit rgb video using specified format
			switch (SrcBitCount)
			{
			case 8: // 8-bit source surface with 32-bit destination surface
				Logging::Log() << __FUNCTION__ << " No support for non-palette 8-bit source surfaces!";
				break;
			case 16: // 16-bit source surface with 32-bit destination surface
				switch (SrcFormat)
				{
				case D3DFMT_A1R5G5B5:
				case D3DFMT_X1R5G5B5:
				{
					WORD *RawBuffer = (WORD*)SrcBuffer;
					for (LONG y = 0; y < RectHeight; y++)
					{
						DWORD StartDestLoc = ((y + pDestRect->top) * pDestLockRect->Pitch) + (pDestRect->left * 2);
						DWORD StartSrcLoc = ((y + pSrcRect->top) * pSrcLockRect->Pitch) + (pSrcRect->left * 2);

						for (LONG x = 0; x < RectWidth; x++)
						{
							LONG z = StartSrcLoc + x * 2;
							DestBuffer[StartDestLoc + x * 2] =
								((RawBuffer[z] & 0x8000) << 9) * 255 +		// Alpha
								((RawBuffer[z] & 0x7C00) << 9) +			// Red
								((RawBuffer[z] & 0x03E0) << 6) +			// Green
								((RawBuffer[z] & 0x001F) << 3);				// Blue
						}
					}
					break;
				}
				case D3DFMT_R5G6B5:
				{
					WORD *RawBuffer = (WORD*)SrcBuffer;
					for (LONG y = 0; y < RectHeight; y++)
					{
						DWORD StartDestLoc = ((y + pDestRect->top) * pDestLockRect->Pitch) + (pDestRect->left * 2);
						DWORD StartSrcLoc = ((y + pSrcRect->top) * pSrcLockRect->Pitch) + (pSrcRect->left * 2);

						for (LONG x = 0; x < RectWidth; x++)
						{
							LONG z = StartSrcLoc + x * 2;
							DestBuffer[StartDestLoc + x * 2] =
								(0xFF000000) +						// Alpha
								((RawBuffer[z] & 0xF800) << 8) +	// Red
								((RawBuffer[z] & 0x07E0) << 5) +	// Green
								((RawBuffer[z] & 0x001F) << 3);		// Blue
						}
					}
					break;
				}
				default:
					Logging::Log() << __FUNCTION__ << " Unsupported 16-bit source format type: " << SrcFormat;
					break;
				}
				break;
			case 32: // 32-bit source surface with 32-bit destination surface
				switch (SrcFormat)
				{
				case D3DFMT_A8R8G8B8:
				case D3DFMT_X8R8G8B8:
				{
					// Copy memory
					for (LONG y = 0; y < RectHeight; y++)
					{
						memcpy((BYTE*)pDestLockRect->pBits + ((y + pDestRect->top) * pDestLockRect->Pitch) + (pDestRect->left * 4),	// Destination video memory address
							(BYTE*)pSrcLockRect->pBits + ((y + pSrcRect->top) * pSrcLockRect->Pitch) + (pSrcRect->left * 4),		// Source video memory address
							RectWidth * 4);																							// Size of bytes to write
					}
					break;
				}
				default: // Unsupported source surface with 32-bit destination surface
					Logging::Log() << __FUNCTION__ << " Unsupported 32-bit source format type: " << SrcFormat;
					break;
				}
				break;
			default:
				Logging::Log() << __FUNCTION__ << " Unsupported source bit count: " << SrcBitCount;
				break;
			}
			break;
		default:
			Logging::Log() << __FUNCTION__ << " Unsupported destination surface format: " << DestFormat;
			break;
		}
		break;
	}
	default:
		Logging::Log() << __FUNCTION__ << " Unsupported destination bit count: " << DestBitCount;
		break;
	}

	// Return
	return DD_OK;
}

// Copy rect with color key
HRESULT m_IDirectDrawSurfaceX::CopyRectColorKey(D3DLOCKED_RECT *pDestLockRect, RECT *pDestRect, DWORD DestBitCount, D3DFORMAT DestFormat, D3DLOCKED_RECT *pSrcLockRect, RECT *pSrcRect, DWORD SrcBitCount, D3DFORMAT SrcFormat, DDCOLORKEY ColorKey)
{
	Logging::LogDebug() << __FUNCTION__;

	// Check destination parameters
	if (!pDestLockRect || !pDestLockRect->pBits || !pDestRect || !DestBitCount || DestFormat == D3DFMT_UNKNOWN)
	{
		Logging::Log() << __FUNCTION__ << " Invalid destination parameters";
		return DDERR_INVALIDPARAMS;
	}

	// Check source parameters
	if (!pSrcLockRect || !pSrcLockRect->pBits || !pSrcRect || !SrcBitCount || SrcFormat == D3DFMT_UNKNOWN)
	{
		Logging::Log() << __FUNCTION__ << " Invalid source parameters";
		return DDERR_INVALIDPARAMS;
	}

	// Check if source and destination formats are the same
	if (DestFormat != SrcFormat)
	{
		Logging::Log() << __FUNCTION__ << " Different source and destination formats not implemented";
		return DDERR_UNSUPPORTED;
	}

	// Get width and height of rect
	LONG RectWidth = min(pDestRect->right - pDestRect->left, pSrcRect->right - pSrcRect->left);
	LONG RectHeight = min(pDestRect->bottom - pDestRect->top, pSrcRect->bottom - pSrcRect->top);

	// Copy memory using color key
	switch (DestBitCount)
	{
	case 8: // 8-bit surfaces
	case 16: // 16-bit surfaces
	case 24: // 24-bit surfaces
	case 32: // 32-bit surfaces
	{
		DWORD ByteCount = DestBitCount / 8;
		DWORD ByteMask = (DWORD)(pow(256, ByteCount)) - 1;
		DWORD ColorKeyLow = ColorKey.dwColorSpaceLowValue & ByteMask;
		DWORD ColorKeyHigh = ColorKey.dwColorSpaceHighValue & ByteMask;

		for (LONG y = 0; y < RectHeight; y++)
		{
			DWORD StartDestLoc = ((y + pDestRect->top) * pDestLockRect->Pitch) + (pDestRect->left * ByteCount);
			DWORD StartSrcLoc = ((y + pSrcRect->top) * pSrcLockRect->Pitch) + (pSrcRect->left * ByteCount);

			for (LONG x = 0; x < RectWidth; x++)
			{
				DWORD *NewPixel = (DWORD*)((BYTE*)pSrcLockRect->pBits + StartSrcLoc + x * ByteCount);
				DWORD PixelColor = (ByteCount == 1) ? (BYTE)(*NewPixel) :
					(ByteCount == 2) ? (WORD)(*NewPixel) :
					(ByteCount == 3) ? (DWORD)(*NewPixel) & ByteMask :
					(ByteCount == 4) ? (DWORD)(*NewPixel) : 0;

				if (PixelColor < ColorKeyLow || PixelColor > ColorKeyHigh)
				{
					memcpy((BYTE*)pDestLockRect->pBits + StartDestLoc + x * ByteCount, NewPixel, ByteCount);
				}
			}
		}
		break;
	}
	default: // Unsupported surface bit count
		Logging::Log() << __FUNCTION__ << " Not implemented bit count " << DestBitCount;
		return DDERR_UNSUPPORTED;
	}

	// Return
	return DD_OK;
}

// Stretch source rect to destination rect
HRESULT m_IDirectDrawSurfaceX::StretchRect(D3DLOCKED_RECT *pDestLockRect, RECT *pDestRect, DWORD DestBitCount, D3DFORMAT DestFormat, D3DLOCKED_RECT *pSrcLockRect, RECT *pSrcRect, DWORD SrcBitCount, D3DFORMAT SrcFormat)
{
	Logging::LogDebug() << __FUNCTION__;

	// Check destination parameters
	if (!pDestLockRect || !pDestLockRect->pBits || !pDestRect || !DestBitCount || DestFormat == D3DFMT_UNKNOWN)
	{
		Logging::Log() << __FUNCTION__ << " Invalid destination parameters";
		return DDERR_INVALIDPARAMS;
	}

	// Check source parameters
	if (!pSrcLockRect || !pSrcLockRect->pBits || !pSrcRect || !SrcBitCount || SrcFormat == D3DFMT_UNKNOWN)
	{
		Logging::Log() << __FUNCTION__ << " Invalid source parameters";
		return DDERR_INVALIDPARAMS;
	}

	// Check if source and destination formats are the same
	if (DestFormat != SrcFormat)
	{
		Logging::Log() << __FUNCTION__ << " Different source and destination formats not implemented";
		return DDERR_UNSUPPORTED;
	}

	// Get width and height of rect
	LONG DestRectWidth = pDestRect->right - pDestRect->left;
	LONG DestRectHeight = pDestRect->bottom - pDestRect->top;
	LONG SrcRectWidth = pSrcRect->right - pSrcRect->left;
	LONG SrcRectHeight = pSrcRect->bottom - pSrcRect->top;

	// Get ratio
	float WidthRatio = (float)SrcRectWidth / (float)DestRectWidth;
	float HeightRatio = (float)SrcRectHeight / (float)DestRectHeight;

	// Copy memory using color key
	switch (DestBitCount)
	{
	case 8: // 8-bit surfaces
	case 16: // 16-bit surfaces
	case 24: // 24-bit surfaces
	case 32: // 32-bit surfaces
	{
		DWORD ByteCount = DestBitCount / 8;
		for (LONG y = 0; y < DestRectHeight; y++)
		{
			DWORD StartDestLoc = ((y + pDestRect->top) * pDestLockRect->Pitch) + (pDestRect->left * ByteCount);
			DWORD StartSrcLoc = ((((DWORD)((float)y * HeightRatio)) + pSrcRect->top) * pSrcLockRect->Pitch) + (pSrcRect->left * ByteCount);

			for (LONG x = 0; x < DestRectWidth; x++)
			{
				memcpy((BYTE*)pDestLockRect->pBits + StartDestLoc + x * ByteCount, (BYTE*)((BYTE*)pSrcLockRect->pBits + StartSrcLoc + ((DWORD)((float)x * WidthRatio)) * ByteCount), ByteCount);
			}
		}
		break;
	}
	default: // Unsupported surface bit count
		Logging::Log() << __FUNCTION__ << " Not implemented bit count " << DestBitCount;
		return DDERR_UNSUPPORTED;
	}

	// Return
	return DD_OK;
}

void m_IDirectDrawSurfaceX::ReleaseSurface()
{
	SetCriticalSection();
	if (ddrawParent)
	{
		ddrawParent->RemoveSurfaceFromVector(this);
	}
	ReleaseCriticalSection();
}
