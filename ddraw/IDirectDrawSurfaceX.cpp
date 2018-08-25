/**
* Copyright (C) 2018 Elisha Riedlinger
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

bool SceneReady = false;

/************************/
/*** IUnknown methods ***/
/************************/

HRESULT m_IDirectDrawSurfaceX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	if (Config.Dd7to9)
	{
		if ((riid == IID_IDirectDrawSurface || riid == IID_IDirectDrawSurface2 || riid == IID_IDirectDrawSurface3 || riid == IID_IDirectDrawSurface4 || riid == IID_IDirectDrawSurface7 || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return S_OK;
		}
	}
	else
	{
		if (ProxyDirectXVersion > 4 && ppvObj && (riid == IID_IDirect3DTexture || riid == IID_IDirect3DTexture2))
		{
			if (lpCurrentD3DDevice)
			{
				ProxyInterface->AddRef();

				*ppvObj = new m_IDirect3DTextureX(lpCurrentD3DDevice, 7, ProxyInterface);

				return S_OK;
			}
		}
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, WrapperInterface);
}

ULONG m_IDirectDrawSurfaceX::AddRef()
{
	if (Config.Dd7to9)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawSurfaceX::Release()
{
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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpDDSurface)
	{
		lpDDSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDSurface)->GetProxyInterface();
	}

	return ProxyInterface->AddAttachedSurface(lpDDSurface);
}

HRESULT m_IDirectDrawSurfaceX::AddOverlayDirtyRect(LPRECT lpRect)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->AddOverlayDirtyRect(lpRect);
}

HRESULT m_IDirectDrawSurfaceX::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
	if (Config.Dd7to9)
	{
		// Check for required DDBLTFX structure
		if (!lpDDBltFx && (dwFlags & (DDBLT_DDFX | DDBLT_COLORFILL | DDBLT_DEPTHFILL | DDBLT_DDROPS | DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_ROP | DDBLT_ROTATIONANGLE)))
		{
			Logging::Log() << __FUNCTION__ << " DDBLTFX structure not found";
			return DDERR_INVALIDPARAMS;
		}

		// Check for device
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " D3d9 Device not setup.";
			return DDERR_GENERIC;
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

		// Check if source Surface exists
		m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = (m_IDirectDrawSurfaceX*)lpDDSrcSurface;
		if (!lpDDSrcSurfaceX)
		{
			lpDDSrcSurfaceX = this;
		}
		else if (!ddrawParent->DoesSurfaceExist(lpDDSrcSurfaceX))
		{
			Logging::Log() << __FUNCTION__ << " Error, could not find source surface";
			return DDERR_INVALIDPARAMS;
		}

		// Check if source Surface is ready for Blt
		else if (!lpDDSrcSurfaceX->IsSurfaceLocked() && !lpDDSrcSurfaceX->NeedsLock())
		{
			Logging::Log() << __FUNCTION__ << " Error, Blting from a surface before it has been written to.";
			return DDERR_GENERIC;
		}

		// Do color fill
		if (dwFlags & DDBLT_COLORFILL)
		{
			return ColorFill(lpDestRect, lpDDBltFx->dwFillColor);
		}

		// Check if destination Surface is ready for Blt
		if (!IsSurfaceLocked() && !NeedsLock())
		{
			Logging::Log() << __FUNCTION__ << " Error, Blting to a surface before it has been written to.";
			return DDERR_GENERIC;
		}

		// Check and copy destination and source rect
		RECT DestRect, SrcRect;
		if (!FixRect(&DestRect, lpDestRect) || !lpDDSrcSurfaceX->FixRect(&SrcRect, lpSrcRect))
		{
			Logging::Log() << __FUNCTION__ << " Error, invalid rect size";
			return DDERR_INVALIDRECT;
		}

		// Check if destination surface is not locked then lock it
		bool UnlockDest = false;
		if (!IsSurfaceLocked())
		{
			HRESULT hr = SetLock(nullptr, 0);
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
				hr = lpDDSrcSurfaceX->SetLock(nullptr, 0);
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
			if (abs((DestRect.right - DestRect.left) - (SrcRect.right - SrcRect.left)) > 1 && abs((DestRect.bottom - DestRect.top) - (SrcRect.bottom - SrcRect.top)) > 1 &&
				(dwFlags & (DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_KEYDEST | DDBLT_KEYSRC)))
			{
				Logging::Log() << __FUNCTION__ << " stretch rect plus color key not imlpemented";
				hr = DDERR_GENERIC;
			}
			// Check for color key flags
			else if (dwFlags & (DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_KEYDEST | DDBLT_KEYSRC))
			{
				// Check if color key is set
				if (((dwFlags & DDBLT_KEYDEST) && !ColorKeys[0].IsSet) || ((dwFlags & DDBLT_KEYSRC) && !ColorKeys[2].IsSet))
				{
					Logging::Log() << __FUNCTION__ << " Error color key not set";
				}

				// Get color key
				DDCOLORKEY ColorKey = { 0, 0 };
				ColorKey = (dwFlags & DDBLT_KEYDESTOVERRIDE) ? lpDDBltFx->ddckDestColorkey :
					(dwFlags & DDBLT_KEYSRCOVERRIDE) ? lpDDBltFx->ddckSrcColorkey :
					(dwFlags & DDBLT_KEYDEST) ? ColorKeys[0].Key :
					(dwFlags & DDBLT_KEYSRC) ? ColorKeys[2].Key : ColorKey;

				// copy rect using color key
				hr = CopyRectColorKey(&DestLockRect, &DestRect, DestBitCount, DestFormat, &SrcLockRect, &SrcRect, SrcBitCount, SrcFormat, ColorKey);
				if (FAILED(hr))
				{
					Logging::Log() << __FUNCTION__ << " failed to copy rect";
				}
			}
			// Strect rect
			else if (abs((DestRect.right - DestRect.left) - (SrcRect.right - SrcRect.left)) > 1 && abs((DestRect.bottom - DestRect.top) - (SrcRect.bottom - SrcRect.top)) > 1)
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
			SetUnLock();
		}
		if (UnlockSrc)
		{
			lpDDSrcSurfaceX->SetUnLock();
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
		m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = (!lpDDSrcSurface) ? this : (m_IDirectDrawSurfaceX*)lpDDSrcSurface;
		lpDDSrcSurfaceX->FixRect(&SrcRect, lpSrcRect);

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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpDDSAttachedSurface)
	{
		lpDDSAttachedSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDSAttachedSurface)->GetProxyInterface();
	}

	return ProxyInterface->DeleteAttachedSurface(dwFlags, lpDDSAttachedSurface);
}

HRESULT m_IDirectDrawSurfaceX::EnumAttachedSurfaces(LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback7)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not fully Implemented.";

		if (!lpEnumSurfacesCallback7)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_INVALIDOBJECT;
		}

		for (auto it : AttachedSurfaceMap)
		{
			DDSURFACEDESC2 Desc2;
			it.second->GetSurfaceDesc(&Desc2);
			if (lpEnumSurfacesCallback7(it.second, &Desc2, lpContext) != DDENUMRET_OK)
			{
				return DD_OK;
			}
		}

		return DD_OK;
	}

	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = lpEnumSurfacesCallback7;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ConvertSurfaceDescTo2 = ConvertSurfaceDescTo2;

	return ProxyInterface->EnumAttachedSurfaces(&CallbackContext, m_IDirectDrawEnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawSurfaceX::EnumOverlayZOrders(DWORD dwFlags, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpfnCallback)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = lpfnCallback;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ConvertSurfaceDescTo2 = ConvertSurfaceDescTo2;

	return ProxyInterface->EnumOverlayZOrders(dwFlags, &CallbackContext, m_IDirectDrawEnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawSurfaceX::Flip(LPDIRECTDRAWSURFACE7 lpDDSurfaceTargetOverride, DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		// Check for device
		if (!d3d9Device || !*d3d9Device || !ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " D3d9 Device not setup.";
			return DDERR_GENERIC;
		}

		// Make sure surface exists, if not then create it
		if (!surfaceTexture)
		{
			if (FAILED(CreateD3d9Surface()))
			{
				Logging::Log() << __FUNCTION__ << " could not recreate surface";
				return DDERR_SURFACELOST;
			}
		}

		// Flip only supported from primary surface
		if (!IsPrimarySurface())
		{
			Logging::Log() << __FUNCTION__ << " Non-primary surface Flip not implimented";
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
			m_IDirectDrawSurfaceX *lpTargetSurface = (m_IDirectDrawSurfaceX*)lpDDSurfaceTargetOverride;

			// Check if target surface exists
			if (!DoesAttachedSurfaceExist(lpTargetSurface) || lpTargetSurface == this)
			{
				Logging::Log() << __FUNCTION__ << " Invalid SurfaceTarget";
				return DDERR_INVALIDPARAMS;
			}

			// Found surface
			FoundAttachedSurface = true;

			// Swap textures
			LPDIRECT3DTEXTURE9 tmpAddr = surfaceTexture;
			surfaceTexture = *lpTargetSurface->GetSurfaceTexture();
			*lpTargetSurface->GetSurfaceTexture() = tmpAddr;
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
				return E_NOTIMPL;
			}

			if (dwFlags & (DDFLIP_ODD | DDFLIP_EVEN))
			{
				Logging::Log() << __FUNCTION__ << " Even and odd flipping not implemented";
				return E_NOTIMPL;
			}

			// Loop through each surface and swap them
			for (auto it : AttachedSurfaceMap)
			{
				m_IDirectDrawSurfaceX *lpTargetSurface = (m_IDirectDrawSurfaceX*)it.second;

				// Found surface
				FoundAttachedSurface = true;

				// Swap textures
				LPDIRECT3DTEXTURE9 tmpAddr = surfaceTexture;
				surfaceTexture = *lpTargetSurface->GetSurfaceTexture();
				*lpTargetSurface->GetSurfaceTexture() = tmpAddr;
			}
		}

		// Return error if no attached surfaces found
		if (!FoundAttachedSurface)
		{
			Logging::Log() << __FUNCTION__ << " No attached surfaces found";
			return DDERR_GENERIC;
		}

		// Run BeginScene (ignore results)
		ddrawParent->BeginScene();

		// Set new texture
		HRESULT hr = (*d3d9Device)->SetTexture(0, surfaceTexture);
		if (FAILED(hr))
		{
			Logging::Log() << __FUNCTION__ << " Failed to set texture";
		}

		// Run EndScene (ignore results)
		ddrawParent->EndScene();

		return hr;
	}

	if (lpDDSurfaceTargetOverride)
	{
		lpDDSurfaceTargetOverride = static_cast<m_IDirectDrawSurface7 *>(lpDDSurfaceTargetOverride)->GetProxyInterface();
	}

	return ProxyInterface->Flip(lpDDSurfaceTargetOverride, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetAttachedSurface(LPDDSCAPS2 lpDDSCaps, LPDIRECTDRAWSURFACE7 FAR * lplpDDAttachedSurface)
{
	if (!lplpDDAttachedSurface)
	{
		return DDERR_INVALIDPARAMS;
	}

	DDSCAPS2 Caps2;
	// Game using old DirectX, Convert DDSCAPS to DDSCAPS2
	if (lpDDSCaps && ConvertSurfaceDescTo2)
	{
		ConvertCaps(Caps2, *(LPDDSCAPS)lpDDSCaps);
		lpDDSCaps = &Caps2;
	}

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not fully Implemented.";

		// Check for device
		if (!d3d9Device || !*d3d9Device || !ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " D3d9 Device not setup.";
			return DDERR_GENERIC;
		}

		/*ToDo: GetAttachedSurface fails if more than one surface is attached that matches the capabilities requested. 
		In this case, the application must use the IDirectDrawSurface7::EnumAttachedSurfaces method to obtain the attached surfaces.*/

		DDSURFACEDESC2 DDSurfaceDesc2;
		memcpy(&DDSurfaceDesc2, &surfaceDesc2, sizeof(DDSURFACEDESC2));
		if (lpDDSCaps)
		{
			memcpy(&DDSurfaceDesc2.ddsCaps, lpDDSCaps, sizeof(DDSCAPS2));
		}
		DDSurfaceDesc2.ddsCaps.dwCaps &= ~DDSCAPS_PRIMARYSURFACE;		// Remove Primary surface flag

		m_IDirectDrawSurfaceX *attachedSurface = new m_IDirectDrawSurfaceX(d3d9Device, ddrawParent, DirectXVersion, &DDSurfaceDesc2, displayWidth, displayHeight);

		*lplpDDAttachedSurface = attachedSurface;

		AddAttachedSurfaceToMap(attachedSurface);

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->GetAttachedSurface(lpDDSCaps, lplpDDAttachedSurface);

	if (SUCCEEDED(hr))
	{
		*lplpDDAttachedSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpDDAttachedSurface, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetBltStatus(DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetBltStatus(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetCaps(LPDDSCAPS2 lpDDSCaps)
{
	if (!lpDDSCaps)
	{
		return DDERR_INVALIDPARAMS;
	}

	LPDDSCAPS2 lpDDSCaps_tmp = lpDDSCaps;
	DDSCAPS2 Caps2;

	// Game using old DirectX, Convert DDSCAPS to DDSCAPS2
	if (ConvertSurfaceDescTo2)
	{
		lpDDSCaps = &Caps2;
	}

	HRESULT hr = DD_OK;

	if (Config.Dd7to9)
	{
		ConvertCaps(*lpDDSCaps, surfaceDesc2.ddsCaps);
	}
	else
	{
		hr = ProxyInterface->GetCaps(lpDDSCaps);
	}

	// Convert back to DDSCAPS
	if (SUCCEEDED(hr) && ConvertSurfaceDescTo2)
	{
		lpDDSCaps = lpDDSCaps_tmp;
		ConvertCaps(*(LPDDSCAPS)lpDDSCaps, Caps2);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetClipper(LPDIRECTDRAWCLIPPER FAR * lplpDDClipper)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
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
	if (!lpDDColorKey)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not fully Implemented.";

		if (!lphDC)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_INVALIDOBJECT;
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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetFlipStatus(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetOverlayPosition(LPLONG lplX, LPLONG lplY)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not fully Implemented.";

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
	if (!lplpDDPalette)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		// No palette attached
		if (!attachedPalette)
		{
			return DDERR_NOPALETTEATTACHED;
		}

		// Check exclusive mode

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
	if (!lpDDPixelFormat)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);

		// Update surface description
		GetSurfaceDesc2(&Desc2);

		// Copy pixel format to lpDDPixelFormat
		memcpy(lpDDPixelFormat, &Desc2.ddpfPixelFormat, sizeof(DDPIXELFORMAT));

		return DD_OK;
	}

	return ProxyInterface->GetPixelFormat(lpDDPixelFormat);
}

HRESULT m_IDirectDrawSurfaceX::GetSurfaceDesc(LPDDSURFACEDESC2 lpDDSurfaceDesc2)
{
	if (!lpDDSurfaceDesc2)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	LPDDSURFACEDESC2 lpDDSurfaceDesc_tmp = lpDDSurfaceDesc2;
	DDSURFACEDESC2 Desc2;
	if (ConvertSurfaceDescTo2)
	{
		ConvertSurfaceDesc(Desc2, *(LPDDSURFACEDESC)lpDDSurfaceDesc2);
		lpDDSurfaceDesc2 = &Desc2;
	}

	HRESULT hr;

	if (Config.Dd7to9)
	{
		// Update surface description
		hr = GetSurfaceDesc2(lpDDSurfaceDesc2);
	}
	else
	{
		hr = ProxyInterface->GetSurfaceDesc(lpDDSurfaceDesc2);
	}

	// Convert back to LPDDSURFACEDESC
	if (SUCCEEDED(hr) && ConvertSurfaceDescTo2)
	{
		lpDDSurfaceDesc2 = lpDDSurfaceDesc_tmp;
		ConvertSurfaceDesc(*(LPDDSURFACEDESC)lpDDSurfaceDesc2, Desc2);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::Initialize(LPDIRECTDRAW lpDD, LPDDSURFACEDESC2 lpDDSurfaceDesc2)
{
	if (Config.Dd7to9)
	{
		// Not needed
		return DD_OK;
	}

	DDSURFACEDESC2 Desc2;
	if (lpDDSurfaceDesc2 && ConvertSurfaceDescTo2)
	{
		ConvertSurfaceDesc(Desc2, *(LPDDSURFACEDESC)lpDDSurfaceDesc2);
		lpDDSurfaceDesc2 = &Desc2;
	}

	if (lpDD)
	{
		lpDD = static_cast<m_IDirectDraw *>(lpDD)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(lpDD, lpDDSurfaceDesc2);
}

HRESULT m_IDirectDrawSurfaceX::IsLost()
{
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

HRESULT m_IDirectDrawSurfaceX::Lock(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD dwFlags, HANDLE hEvent)
{
	if (!lpDDSurfaceDesc2)
	{
		return DDERR_INVALIDPARAMS;
	}

	LPDDSURFACEDESC2 lpDDSurfaceDesc_tmp = lpDDSurfaceDesc2;
	DDSURFACEDESC2 Desc2;

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (lpDDSurfaceDesc2 && ConvertSurfaceDescTo2)
	{
		ConvertSurfaceDesc(Desc2, *(LPDDSURFACEDESC)lpDDSurfaceDesc2);
		lpDDSurfaceDesc2 = &Desc2;
	}

	HRESULT hr;

	if (Config.Dd7to9)
	{
		// Check if locked
		if (IsLocked && d3dlrect.pBits)
		{
			Logging::Log() << __FUNCTION__ << " Locking surface twice not fully implemented";
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
		DWORD LockFlags = dwFlags & (D3DLOCK_DISCARD | D3DLOCK_DONOTWAIT | D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY);

		// Lock surface
		hr = SetLock(lpDestRect, LockFlags);

		// Set desc and video memory
		if (SUCCEEDED(hr))
		{
			// Copy desc to passed in desc
			memcpy(lpDDSurfaceDesc2, &surfaceDesc2, sizeof(DDSURFACEDESC2));

			// Set video memory and pitch
			lpDDSurfaceDesc2->dwFlags |= DDSD_LPSURFACE | DDSD_PITCH;
			if (WriteDirectlyToSurface && d3dlrect.pBits)
			{
				lpDDSurfaceDesc2->lpSurface = d3dlrect.pBits;
				lpDDSurfaceDesc2->lPitch = d3dlrect.Pitch;
			}
			else
			{
				if (!rawVideoBuf)
				{
					AlocateVideoBuffer();
				}
				lpDDSurfaceDesc2->lpSurface = (LPVOID)rawVideoBuf;
				lpDDSurfaceDesc2->lPitch = surfaceDesc2.dwWidth * (GetBitCount(surfaceDesc2.ddpfPixelFormat) / 8);
			}
		}
	}
	else
	{
		hr = ProxyInterface->Lock(lpDestRect, lpDDSurfaceDesc2, dwFlags, hEvent);
	}

	// Convert back to LPDDSURFACEDESC
	if (SUCCEEDED(hr) && ConvertSurfaceDescTo2)
	{
		lpDDSurfaceDesc2 = lpDDSurfaceDesc_tmp;
		ConvertSurfaceDesc(*(LPDDSURFACEDESC)lpDDSurfaceDesc2, Desc2);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::ReleaseDC(HDC hDC)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not fully Implemented.";

		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_INVALIDOBJECT;
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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->Restore();
}

HRESULT m_IDirectDrawSurfaceX::SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpDDClipper)
	{
		lpDDClipper = static_cast<m_IDirectDrawClipper *>(lpDDClipper)->GetProxyInterface();
	}

	return ProxyInterface->SetClipper(lpDDClipper);
}

HRESULT m_IDirectDrawSurfaceX::SetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey)
{
	if (!lpDDColorKey)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
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
			ColorKeys[x].IdColorSpace = ((dwFlags & DDCKEY_COLORSPACE) != 0);
			memcpy(&ColorKeys[x].Key, lpDDColorKey, sizeof(DDCOLORKEY));
		}

		// Return
		return DD_OK;
	}

	return ProxyInterface->SetColorKey(dwFlags, lpDDColorKey);
}

HRESULT m_IDirectDrawSurfaceX::SetOverlayPosition(LONG lX, LONG lY)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not fully Implemented.";

		// Store the new overlay position
		overlayX = lX;
		overlayY = lY;

		return DD_OK;
	}

	return ProxyInterface->SetOverlayPosition(lX, lY);
}

HRESULT m_IDirectDrawSurfaceX::SetPalette(LPDIRECTDRAWPALETTE lpDDPalette)
{
	if (Config.Dd7to9)
	{
		static bool FirstRun = true;

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
			FirstRun = true;

			return DD_OK;
		}

		// Set palette address
		attachedPalette = (m_IDirectDrawPalette *)lpDDPalette;

		// Don't write to surface when using palettes
		WriteDirectlyToSurface = false;
		AlocateVideoBuffer();

		// When you call SetPalette to set a palette to a surface for the first time, 
		// SetPalette increments the palette's reference count; subsequent calls to 
		// SetPalette do not affect the palette's reference count.
		if (FirstRun)
		{
			attachedPalette->AddRef();
			FirstRun = false;
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
	if (Config.Dd7to9)
	{
		// If no video memory than nothing to do...
		if (!d3dlrect.pBits)
		{
			Logging::Log() << __FUNCTION__ << " No video memory!";
			return DD_OK;
		}

		// Check for video buffer and create it if missing
		if (!WriteDirectlyToSurface && !rawVideoBuf)
		{
			AlocateVideoBuffer();
		}

		// Write to surface using a palette
		if (attachedPalette)
		{
			WritePaletteToSurface(attachedPalette, &lkDestRect, rawVideoBuf, GetBitCount(surfaceDesc2.ddpfPixelFormat));
		}

		// Write to surface from the memory buffer
		else if (!WriteDirectlyToSurface)
		{
			DWORD SrcBitCount = GetBitCount(surfaceDesc2.ddpfPixelFormat);
			D3DLOCKED_RECT SrcLockRect = { (INT)(surfaceDesc2.dwWidth * SrcBitCount / 8), rawVideoBuf };
			HRESULT hr = CopyRect(&d3dlrect, &lkDestRect, 32, D3DFMT_X8R8G8B8, &SrcLockRect, &lkDestRect, SrcBitCount, GetDisplayFormat(surfaceDesc2.ddpfPixelFormat));
			if (FAILED(hr))
			{
				Logging::Log() << __FUNCTION__ << " failed to copy rect";
			}
		}

		// Unlock surface
		return SetUnLock();
	}

	return ProxyInterface->Unlock(lpRect);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlay(LPRECT lpSrcRect, LPDIRECTDRAWSURFACE7 lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, LPDDOVERLAYFX lpDDOverlayFx)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpDDDestSurface)
	{
		lpDDDestSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDDestSurface)->GetProxyInterface();
	}

	return ProxyInterface->UpdateOverlay(lpSrcRect, lpDDDestSurface, lpDestRect, dwFlags, lpDDOverlayFx);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlayDisplay(DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->UpdateOverlayDisplay(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlayZOrder(DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSReference)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
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

HRESULT m_IDirectDrawSurfaceX::GetDDInterface(LPVOID FAR * lplpDD)
{
	if (!lplpDD)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_INVALIDOBJECT;
		}

		// Set lplpDD to directdraw object that created this surface
		*lplpDD = ddrawParent;

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->GetDDInterface(lplpDD);

	if (SUCCEEDED(hr))
	{
		// Calling the GetDDInterface method from any surface created under DirectDrawEx will return a pointer to the 
		// IUnknown interface instead of a pointer to an IDirectDraw interface. Applications must use the
		// IUnknown::QueryInterface method to retrieve the IDirectDraw, IDirectDraw2, or IDirectDraw3 interfaces.
		IID tmpID = (ProxyDirectXVersion == 1) ? IID_IDirectDraw :
			(ProxyDirectXVersion == 2) ? IID_IDirectDraw2 :
			(ProxyDirectXVersion == 3) ? IID_IDirectDraw3 :
			(ProxyDirectXVersion == 4) ? IID_IDirectDraw4 :
			(ProxyDirectXVersion == 7) ? IID_IDirectDraw7 : IID_IDirectDraw7;

		LPDIRECTDRAW7 lpDD = (LPDIRECTDRAW7)*lplpDD;

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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->PageLock(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::PageUnlock(DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->PageUnlock(dwFlags);
}

/*********************************/
/*** Added in the v3 interface ***/
/*********************************/

HRESULT m_IDirectDrawSurfaceX::SetSurfaceDesc(LPDDSURFACEDESC2 lpDDsd, DWORD dwFlags)
{
	if (!lpDDsd)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	DDSURFACEDESC2 Desc2;
	if (ConvertSurfaceDescTo2)
	{
		ConvertSurfaceDesc(Desc2, *(LPDDSURFACEDESC)lpDDsd);
		lpDDsd = &Desc2;
	}

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->SetSurfaceDesc(lpDDsd, dwFlags);
}

/*********************************/
/*** Added in the v4 interface ***/
/*********************************/

HRESULT m_IDirectDrawSurfaceX::SetPrivateData(REFGUID guidTag, LPVOID lpData, DWORD cbSize, DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->SetPrivateData(guidTag, lpData, cbSize, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetPrivateData(REFGUID guidTag, LPVOID lpBuffer, LPDWORD lpcbBufferSize)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetPrivateData(guidTag, lpBuffer, lpcbBufferSize);
}

HRESULT m_IDirectDrawSurfaceX::FreePrivateData(REFGUID guidTag)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->FreePrivateData(guidTag);
}

HRESULT m_IDirectDrawSurfaceX::GetUniquenessValue(LPDWORD lpValue)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetUniquenessValue(lpValue);
}

HRESULT m_IDirectDrawSurfaceX::ChangeUniquenessValue()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->ChangeUniquenessValue();
}

/***********************************/
/*** Moved Texture7 methods here ***/
/***********************************/

HRESULT m_IDirectDrawSurfaceX::SetPriority(DWORD dwPriority)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->SetPriority(dwPriority);
}

HRESULT m_IDirectDrawSurfaceX::GetPriority(LPDWORD lpdwPriority)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetPriority(lpdwPriority);
}

HRESULT m_IDirectDrawSurfaceX::SetLOD(DWORD dwMaxLOD)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->SetLOD(dwMaxLOD);
}

HRESULT m_IDirectDrawSurfaceX::GetLOD(LPDWORD lpdwMaxLOD)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetLOD(lpdwMaxLOD);
}

/************************/
/*** Helper functions ***/
/************************/

// Alocate buffer for surface if format is not supported by d3d9 to support backwards compatibility
void m_IDirectDrawSurfaceX::AlocateVideoBuffer()
{
	// Store old temp buffer
	BYTE *tempBuf = rawVideoBuf;

	// No need to create a buffer
	if (WriteDirectlyToSurface)
	{
		return;
	}

	// Buffer size, always support 32bit
	BufferSize = surfaceDesc2.dwWidth * surfaceDesc2.dwHeight * sizeof(INT32);

	// Allocate the raw video buffer
	rawVideoBuf = new BYTE[BufferSize];

	// Clear raw memory
	ZeroMemory(rawVideoBuf, BufferSize * sizeof(BYTE));

	// Free memory in case there was an old one setup
	if (tempBuf)
	{
		delete tempBuf;
		tempBuf = nullptr;
	}
}

// Create surface
HRESULT m_IDirectDrawSurfaceX::CreateD3d9Surface()
{
	// Release existing surface
	ReleaseD9Surface();

	// Check for device
	if (!d3d9Device || !*d3d9Device || !ddrawParent)
	{
		Logging::Log() << __FUNCTION__ << " D3d9 Device not setup.";
		return DDERR_INVALIDOBJECT;
	}

	// Update surface description
	GetSurfaceDesc2(&surfaceDesc2);
	
	// Get d3d9Object
	IDirect3D9 *d3d9Object = ddrawParent->GetDirect3D();

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

	// If test fails or there is a palette then write to memory buffer
	if (FAILED(d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Format, D3DUSAGE_QUERY_FILTER, D3DRTYPE_TEXTURE, Format)) || attachedPalette)
	{
		WriteDirectlyToSurface = false;
		Format = D3DFMT_A8R8G8B8;
		// Test format
		if (FAILED(d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Format, D3DUSAGE_QUERY_FILTER, D3DRTYPE_TEXTURE, Format)))
		{
			Format = D3DFMT_X8R8G8B8;
		}
		AlocateVideoBuffer();
	}
	// Write directly to surface
	else
	{
		WriteDirectlyToSurface = true;
	}

	// Create surface
	if (FAILED((*d3d9Device)->CreateTexture(surfaceDesc2.dwWidth, surfaceDesc2.dwHeight, 1, Usage, Format, Pool, &surfaceTexture, nullptr)))
	{
		Logging::Log() << __FUNCTION__ << " Unable to create surface";
		return DDERR_GENERIC;
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

	// Set vertex points
	// 0, 0
	vertices[0].x = -0.5f;
	vertices[0].y = -0.5f;
	vertices[0].z = 0.0f;
	vertices[0].rhw = 1.0f;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;

	// currentWidth, 0
	vertices[1].x = (float)displayWidth - 0.5f;
	vertices[1].y = -0.5f;
	vertices[1].z = 0.0f;
	vertices[1].rhw = 1.0f;
	vertices[1].u = 1.0f;
	vertices[1].v = 0.0f;

	// currentWidth, scaledHeight
	vertices[2].x = (float)displayWidth - 0.5f;
	vertices[2].y = (float)displayHeight - 0.5f;
	vertices[2].z = 0.0f;
	vertices[2].rhw = 1.0f;
	vertices[2].u = 1.0f;
	vertices[2].v = 1.0f;

	// 0, currentHeight
	vertices[3].x = -0.5f;
	vertices[3].y = (float)displayHeight - 0.5f;
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
	if (FAILED((*d3d9Device)->SetTexture(0, surfaceTexture)))
	{
		Logging::Log() << __FUNCTION__ << " Failed to set texture";
		return DDERR_GENERIC;
	}

	// BeginScene
	ddrawParent->BeginScene();

	return DD_OK;
}

// Release surface and vertext buffer
void m_IDirectDrawSurfaceX::ReleaseD9Surface()
{
	// Release d3d9 surface
	if (surfaceTexture)
	{
		DWORD x = 0, z = 100;
		while (z != 0 && ++x < 100)
		{
			z = surfaceTexture->Release();
		}

		// Add error checking
		if (z != 0)
		{
			Logging::Log() << __FUNCTION__ << " Unable to release Direct3D9 surface";
		}

		surfaceTexture = nullptr;
	}

	// Release d3d9 vertex buffer
	if (vertexBuffer)
	{
		DWORD x = 0, z = 100;
		while (z != 0 && ++x < 100)
		{
			z = vertexBuffer->Release();
		}

		// Add error checking
		if (z != 0)
		{
			Logging::Log() << __FUNCTION__ << " Unable to release Direct3D9 vertext buffer";
		}

		vertexBuffer = nullptr;
	}

	// Set surface video buffer to nullptr
	d3dlrect.pBits = nullptr;
}

// Check surface reck dimensions and copy rect to new rect
bool m_IDirectDrawSurfaceX::FixRect(LPRECT lpOutRect, LPRECT lpInRect)
{
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
HRESULT m_IDirectDrawSurfaceX::SetLock(LPRECT lpDestRect, DWORD dwFlags)
{
	if (!ddrawParent)
	{
		return DDERR_GENERIC;
	}

	// Make sure surface exists, if not then create it
	if (!surfaceTexture)
	{
		if (FAILED(CreateD3d9Surface()))
		{
			Logging::Log() << __FUNCTION__ << " could not recreate surface";
			return DDERR_SURFACELOST;
		}
	}

	// Run BeginScene (ignore results)
	ddrawParent->BeginScene();

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
HRESULT m_IDirectDrawSurfaceX::SetUnLock()
{
	if (!surfaceTexture || !ddrawParent)
	{
		return DDERR_SURFACELOST;
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
	if (IsPrimarySurface() || SceneReady)
	{
		SceneReady = FAILED(ddrawParent->EndScene());
	}

	return DD_OK;
}

// Get LOCKED_RECT, BitCount and Format for the surface
HRESULT m_IDirectDrawSurfaceX::GetSurfaceInfo(D3DLOCKED_RECT *pLockRect, DWORD *lpBitCount, D3DFORMAT *lpFormat)
{
	if (pLockRect)
	{
		if (NeedsLock())
		{
			return DDERR_GENERIC;
		}
		if (IsLocked)
		{
			memcpy(pLockRect, &d3dlrect, sizeof(D3DLOCKED_RECT));
		}
		else
		{
			pLockRect->pBits = rawVideoBuf;
			pLockRect->Pitch = surfaceDesc2.dwWidth * (GetBitCount(surfaceDesc2.ddpfPixelFormat) / 8);
		}
	}
	if (lpBitCount)
	{
		*lpBitCount = GetBitCount(surfaceDesc2.ddpfPixelFormat);
	}
	if (lpFormat)
	{
		*lpFormat = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);
	}
	return DD_OK;
}

// Always get SurfaceDesc2 and update it no matter what DirectXVersion is used
HRESULT m_IDirectDrawSurfaceX::GetSurfaceDesc2(LPDDSURFACEDESC2 lpDDSurfaceDesc2)
{
	if (!ddrawParent)
	{
		Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
		return DDERR_INVALIDOBJECT;
	}

	// Copy surfacedesc to lpDDSurfaceDesc2
	if (lpDDSurfaceDesc2 != &surfaceDesc2)
	{
		memcpy(lpDDSurfaceDesc2, &surfaceDesc2, sizeof(DDSURFACEDESC2));
	}

	// Set Height and Width
	if ((lpDDSurfaceDesc2->dwFlags & (DDSD_HEIGHT | DDSD_WIDTH)) != (DDSD_HEIGHT | DDSD_WIDTH))
	{
		lpDDSurfaceDesc2->dwFlags |= DDSD_HEIGHT | DDSD_WIDTH;
		lpDDSurfaceDesc2->dwWidth = ddrawParent->GetDisplayModeWidth();
		lpDDSurfaceDesc2->dwHeight = ddrawParent->GetDisplayModeHeight();
	}
	// Set Refresh Rate
	if ((lpDDSurfaceDesc2->dwFlags & DDSD_REFRESHRATE) == 0)
	{
		lpDDSurfaceDesc2->dwFlags |= DDSD_REFRESHRATE;
		lpDDSurfaceDesc2->dwRefreshRate = ddrawParent->GetDisplayModeRefreshRate();
	}
	// Set PixelFormat
	if ((lpDDSurfaceDesc2->dwFlags & DDSD_PIXELFORMAT) == 0)
	{
		// Set PixelFormat flags
		lpDDSurfaceDesc2->dwFlags |= DDSD_PIXELFORMAT;
		lpDDSurfaceDesc2->ddpfPixelFormat.dwFlags = DDPF_RGB;

		// Set BitCount
		lpDDSurfaceDesc2->ddpfPixelFormat.dwRGBBitCount = ddrawParent->GetDisplayModeBPP();

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
			return E_NOTIMPL;
		}
	}

	// Return
	return DD_OK;
}

// Add attached surface to map
void m_IDirectDrawSurfaceX::AddAttachedSurfaceToMap(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return;
	}

	// Get map Key
	DWORD Key = 0;
	if (AttachedSurfaceMap.size() != 0)
	{
		Key = AttachedSurfaceMap.end()->first + 1;
	}

	// Store surface
	AttachedSurfaceMap[Key] = lpSurfaceX;
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
	// Check and copy rect
	RECT DestRect;
	if (!FixRect(&DestRect, pRect))
	{
		Logging::Log() << __FUNCTION__ << " Error, invalid rect size";
		return DDERR_INVALIDRECT;
	}

	// Check if surface is not locked then lock it
	bool UnlockDest = false;
	if (!IsSurfaceLocked())
	{
		HRESULT hr = SetLock(nullptr, 0);
		if (FAILED(hr))
		{
			Logging::Log() << __FUNCTION__ << " Error, could not lock dest surface";
			return DDERR_GENERIC;
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
		return DDERR_GENERIC;
	}

	// Get width and height of rect
	LONG FillWidth = DestRect.right - DestRect.left;
	LONG FillHeight = DestRect.bottom - DestRect.top;

	// Get ByteCount and FillColor
	DWORD ByteCount = DestLockRect.Pitch / surfaceDesc2.dwWidth;
	DWORD FillColor = 0;
	if (attachedPalette && attachedPalette->rgbPalette && !WriteDirectlyToSurface)
	{
		FillColor = attachedPalette->rgbPalette[rawVideoBuf[dwFillColor]];
	}
	else if (ByteCount <= 4)
	{
		FillColor = dwFillColor;
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error, could not find correct fill color for ByteCount " << ByteCount;
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

	// Unlock surfaces if needed
	if (UnlockDest)
	{
		SetUnLock();
	}

	return DD_OK;
}

// Write palette video data to surface
HRESULT m_IDirectDrawSurfaceX::WritePaletteToSurface(m_IDirectDrawPalette *lpDDPalette, RECT *pRect, BYTE *lpVideoBuf, DWORD BitCount)
{
	if (!lpDDPalette || !lpDDPalette->rgbPalette || !lpVideoBuf)
	{
		Logging::Log() << __FUNCTION__ << " Error invaled parameters!";
		return DDERR_INVALIDPARAMS;
	}

	// Check and copy rect
	RECT DestRect;
	if (!FixRect(&DestRect, pRect))
	{
		Logging::Log() << __FUNCTION__ << " Error, invalid rect size";
		return DDERR_INVALIDRECT;
	}

	// Check if surface is not locked then lock it
	bool UnlockDest = false;
	if (!IsSurfaceLocked())
	{
		if (FAILED(SetLock(nullptr, 0)))
		{
			Logging::Log() << __FUNCTION__ << " Error, could not lock dest surface";
			return DDERR_GENERIC;
		}
		UnlockDest = true;
	}

	// Create raw video memory and rgb buffer variables
	UINT32 *surfaceBuffer = (UINT32*)d3dlrect.pBits;

	// Translate palette to rgb video buffer
	switch (BitCount)
	{
	case 8:
	{
		for (LONG j = DestRect.top; j < DestRect.bottom; j++)
		{
			LONG z = j * (d3dlrect.Pitch / 4);
			for (LONG i = DestRect.left; i < DestRect.right; i++)
			{
				LONG x = z + i;
				surfaceBuffer[x] = lpDDPalette->rgbPalette[lpVideoBuf[x]] | 0xFF000000;
			}
		}
		break;
	}
	default:
		Logging::Log() << __FUNCTION__ << " No support for palette on " << BitCount << "-bit surfaces!";
		break;
	}

	// Unlock surfaces if needed
	if (UnlockDest)
	{
		SetUnLock();
	}

	return DD_OK;
}

// Copy surface rect
HRESULT m_IDirectDrawSurfaceX::CopyRect(D3DLOCKED_RECT *pDestLockRect, RECT *pDestRect, DWORD DestBitCount, D3DFORMAT DestFormat, D3DLOCKED_RECT *pSrcLockRect, RECT *pSrcRect, DWORD SrcBitCount, D3DFORMAT SrcFormat)
{
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
		return DDERR_GENERIC;
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
		return DDERR_GENERIC;
	}

	// Return
	return DD_OK;
}

// Stretch source rect to destination rect
HRESULT m_IDirectDrawSurfaceX::StretchRect(D3DLOCKED_RECT *pDestLockRect, RECT *pDestRect, DWORD DestBitCount, D3DFORMAT DestFormat, D3DLOCKED_RECT *pSrcLockRect, RECT *pSrcRect, DWORD SrcBitCount, D3DFORMAT SrcFormat)
{
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
		return DDERR_GENERIC;
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
		return DDERR_GENERIC;
	}

	// Return
	return DD_OK;
}
