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
		if (ProxyDirectXVersion == 7 && ppvObj && (riid == IID_IDirect3DTexture || riid == IID_IDirect3DTexture2))
		{
			ProxyInterface->AddRef();

			*ppvObj = new m_IDirect3DTextureX((IDirect3DTexture2*)ProxyInterface, 7, (m_IDirect3DTexture2*)this);

			return S_OK;
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

HRESULT m_IDirectDrawSurfaceX::AddAttachedSurface(LPDIRECTDRAWSURFACE7 lpDDSurface)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
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
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->AddOverlayDirtyRect(lpRect);
}

HRESULT m_IDirectDrawSurfaceX::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
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
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
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
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
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
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	if (lpDDSAttachedSurface)
	{
		lpDDSAttachedSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDSAttachedSurface)->GetProxyInterface();
	}

	return ProxyInterface->DeleteAttachedSurface(dwFlags, lpDDSAttachedSurface);
}

HRESULT m_IDirectDrawSurfaceX::EnumAttachedSurfaces(LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = lpEnumSurfacesCallback;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ConvertSurfaceDescTo2 = ConvertSurfaceDescTo2;

	return ProxyInterface->EnumAttachedSurfaces(&CallbackContext, m_IDirectDrawEnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawSurfaceX::EnumOverlayZOrders(DWORD dwFlags, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpfnCallback)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
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
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
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
		Logging::Log() << __FUNCTION__ << " Not fully implimented.";

		*lplpDDAttachedSurface = this;
		
		AddRef();

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
		Logging::Log() << __FUNCTION__ << " Not Implimented";
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

	HRESULT hr;

	if (Config.Dd7to9)
	{
		ConvertCaps(*lpDDSCaps, surfaceDesc2.ddsCaps);

		hr = DD_OK;
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
		Logging::Log() << __FUNCTION__ << " Not Implimented";
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
		if (dwFlags & DDCKEY_DESTBLT)
		{
			// A color key or color space to be used as a destination color key for bit block transfer(bitblt) operations.
			memcpy(lpDDColorKey, &colorKeys[0], sizeof(DDCOLORKEY));
		}
		else if (dwFlags & DDCKEY_DESTOVERLAY)
		{
			// A color key or color space to be used as a destination color key for overlay operations.
			memcpy(lpDDColorKey, &colorKeys[1], sizeof(DDCOLORKEY));
		}
		else if (dwFlags & DDCKEY_SRCBLT)
		{
			// A color key or color space to be used as a source color key for bitblt operations.
			memcpy(lpDDColorKey, &colorKeys[2], sizeof(DDCOLORKEY));
		}
		if (dwFlags & DDCKEY_SRCOVERLAY)
		{
			// A color key or color space to be used as a source color key for overlay operations.
			memcpy(lpDDColorKey, &colorKeys[3], sizeof(DDCOLORKEY));
		}

		return DD_OK;
	}

	return ProxyInterface->GetColorKey(dwFlags, lpDDColorKey);
}

HRESULT m_IDirectDrawSurfaceX::GetDC(HDC FAR * lphDC)
{
	if (Config.Dd7to9)
	{
		if (d3d9Device && *d3d9Device)
		{
			IDirect3DSurface9 *pBackBuffer;
			HRESULT hr = (*d3d9Device)->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
			if (SUCCEEDED(hr))
			{
				return pBackBuffer->GetDC(lphDC);
			}
			else
			{
				Logging::Log() << __FUNCTION__ << " Failed to get BackBuffer error = " << hr;
			}
		}

		Logging::Log() << __FUNCTION__ << " Failed to get d3d9Device";
		return DDERR_GENERIC;
	}

	return ProxyInterface->GetDC(lphDC);
}

HRESULT m_IDirectDrawSurfaceX::GetFlipStatus(DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetFlipStatus(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetOverlayPosition(LPLONG lplX, LPLONG lplY)
{
	if (Config.Dd7to9)
	{
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
		// lpDDPixelFormat receives a detailed description of the current pixel and 
		// color space format of this surface.

		// Copy pixel format to lpDDPixelFormat
		memcpy(lpDDPixelFormat, &surfaceDesc2.ddpfPixelFormat, sizeof(DDPIXELFORMAT));

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
		// Fill lpDDSurfaceDesc2 with this surface description

		// Copy surfacedesc to lpDDSurfaceDesc2
		memcpy(lpDDSurfaceDesc2, &surfaceDesc2, sizeof(DDSURFACEDESC2));

		hr = DD_OK;
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
	DDSURFACEDESC2 Desc2;
	if (lpDDSurfaceDesc2 && ConvertSurfaceDescTo2)
	{
		ConvertSurfaceDesc(Desc2, *(LPDDSURFACEDESC)lpDDSurfaceDesc2);
		lpDDSurfaceDesc2 = &Desc2;
	}

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
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
		// Currently always locks full rect
		// ToDo: Fix this to only lock specified rect

		if (lpDestRect)
		{
			memcpy(&DestRect, lpDestRect, sizeof(RECT));
		}
		else
		{
			DestRect.top = 0;
			DestRect.left = 0;
			DestRect.right = surfaceWidth;
			DestRect.bottom = surfaceHeight;
		}

		// ToDo: Update this to work with games that don't use primary surface
		// Only use primary surface for now...
		if ((surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) != 0)
		{
			// Make sure surface texture exists
			if (!d3d9Surface || !*d3d9Surface)
			{
				d3dlrect.pBits = nullptr;
				Logging::Log() << __FUNCTION__ << " called when texture doesn't exist";
				return DDERR_SURFACELOST;
			}

			// Check for BeginScene
			hr = ddrawParent->CheckBeginScene(*d3d9Surface);

			// Lock rect
			if (SUCCEEDED(hr))
			{
				// Lock full dynamic texture
				hr = ((IDirect3DTexture9*)*d3d9Surface)->LockRect(0, &d3dlrect, ((lpDestRect) ? &DestRect : nullptr), D3DLOCK_DISCARD);
				if (FAILED(hr))
				{
					d3dlrect.pBits = nullptr;
					Logging::Log() << __FUNCTION__ << " Failed to lock texture memory";
				}
			}
		}
		else
		{
			hr = DD_OK;
		}

		// Set desc and video memory
		if (SUCCEEDED(hr))
		{
			// Copy desc to passed in desc
			memcpy(lpDDSurfaceDesc2, &surfaceDesc2, sizeof(DDSURFACEDESC2));

			// Set video memory and pitch
			lpDDSurfaceDesc2->lpSurface = (LPVOID)rawVideoBuf;
			lpDDSurfaceDesc2->dwFlags |= DDSD_LPSURFACE;
			lpDDSurfaceDesc2->lPitch = surfaceWidth * (surfaceDesc2.ddpfPixelFormat.dwRGBBitCount / 8);
			lpDDSurfaceDesc2->dwFlags |= DDSD_PITCH;
		}

		/*sprintf_s(message, 2048, "Unsupported lpDestRect[%d,%d,%d,%d]", lpDestRect->left, lpDestRect->top, lpDestRect->right, lpDestRect->bottom);

		if(!lpDestRect)
		{
			debugMessage(2, "IDirectDrawSurfaceWrapper::Lock", message);
		}
		else
		{
			debugMessage(0, "IDirectDrawSurfaceWrapper::Lock", message);
			// Is error, unsupported
			return DDERR_GENERIC;
		}*/

		/*
		DDLOCK_DONOTWAIT
		On IDirectDrawSurface7 interfaces, the default is DDLOCK_WAIT. If you want to override the default and use time when the accelerator is busy (as denoted by the DDERR_WASSTILLDRAWING return value), use DDLOCK_DONOTWAIT.

		DDLOCK_EVENT
		Not currently implemented.

		DDLOCK_NOOVERWRITE
		New for DirectX 7.0. Used only with Direct3D vertex-buffer locks. Indicates that no vertices that were referred to in a draw operation since the start of the frame (or the last lock without this flag) are modified during the lock. This can be useful when you want only to append data to the vertex buffer.

		DDLOCK_NOSYSLOCK
		Do not take the Win16Mutex (also known as Win16Lock). This flag is ignored when locking the primary surface.

		DDLOCK_DISCARDCONTENTS
		New for DirectX 7.0. Used only with Direct3D vertex-buffer locks. Indicates that no assumptions are made about the contents of the vertex buffer during this lock. This enables Direct3D or the driver to provide an alternative memory area as the vertex buffer. This is useful when you plan to clear the contents of the vertex buffer and fill in new data.

		DDLOCK_OKTOSWAP
		This flag is obsolete and was replaced by the DDLOCK_DISCARDCONTENTS flag.

		DDLOCK_READONLY
		Indicates that the surface being locked can only be read.

		DDLOCK_SURFACEMEMORYPTR
		Indicates that a valid memory pointer to the top of the specified rectangle should be returned. If no rectangle is specified, a pointer to the top of the surface is returned. This is the default.

		DDLOCK_WAIT
		If a lock cannot be obtained because a bit block transfer (bitblt) operation is in progress, Lock retries until a lock is obtained or another error occurs, such as DDERR_SURFACEBUSY.

		DDLOCK_WRITEONLY
		Indicates that the surface being locked is write-enabled.
		*/
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
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->ReleaseDC(hDC);
}

HRESULT m_IDirectDrawSurfaceX::Restore()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->Restore();
}

HRESULT m_IDirectDrawSurfaceX::SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
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
		/*
		DDCKEY_COLORSPACE
		The structure contains a color space. Not set if the structure contains a single color key.
		*/

		//store color key information for the appropriate color key
		if (dwFlags & DDCKEY_DESTBLT)
		{
			memcpy(&colorKeys[0], lpDDColorKey, sizeof(DDCOLORKEY));
		}
		else if (dwFlags & DDCKEY_DESTOVERLAY)
		{
			memcpy(&colorKeys[1], lpDDColorKey, sizeof(DDCOLORKEY));
		}
		else if (dwFlags & DDCKEY_SRCBLT)
		{
			memcpy(&colorKeys[2], lpDDColorKey, sizeof(DDCOLORKEY));
		}
		if (dwFlags & DDCKEY_SRCOVERLAY)
		{
			memcpy(&colorKeys[3], lpDDColorKey, sizeof(DDCOLORKEY));
		}

		return DD_OK;
	}

	return ProxyInterface->SetColorKey(dwFlags, lpDDColorKey);
}

HRESULT m_IDirectDrawSurfaceX::SetOverlayPosition(LONG lX, LONG lY)
{
	if (Config.Dd7to9)
	{
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
		// ToDo: Update this to work with games that don't use primary surface
		// Only use primary surface for now...
		if (!d3dlrect.pBits || (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) == 0)
		{
			return DD_OK;
		}

		// ToDo: Update this to work with games that use non-RGB formats
		// Only support RGB for now...
		if ((surfaceDesc2.ddpfPixelFormat.dwFlags & DDPF_RGB) == 0)
		{
			Logging::Log() << __FUNCTION__ << " Unsupported surface format: " << surfaceDesc2.ddpfPixelFormat.dwFlags;
			return DD_OK;
		}

		// Translate raw video memory to rgb buffer
		UINT32 *surfaceBuffer = (UINT32*)d3dlrect.pBits;
		DWORD BitCount = GetBitCount(surfaceDesc2.ddpfPixelFormat);

		// From a palette
		if (attachedPalette && attachedPalette->rgbPalette)
		{
			// Translate palette to rgb video buffer
			switch (BitCount)
			{
			case 8:
			{
				LONG x = 0, z = 0;
				for (LONG j = DestRect.top; j < DestRect.bottom; j++)
				{
					z = j * (d3dlrect.Pitch / 4);
					for (LONG i = DestRect.left; i < DestRect.right; i++)
					{
						x = z + i;
						surfaceBuffer[x] = attachedPalette->rgbPalette[rawVideoBuf[x]];
					}
				}
				break;
			}
			default:
				Logging::Log() << __FUNCTION__ << " No support for palette on " << BitCount << "-bit surfaces!";
				break;
			}
		}

		// From a surface
		else
		{
			// Get display format
			D3DFORMAT Format = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);

			// Translate palette to rgb video buffer
			switch (BitCount)
			{
			case 8:
				Logging::Log() << __FUNCTION__ << " No support for non-palette 8-bit surfaces!";
				break;
			case 16:
				switch (Format)
				{
				case D3DFMT_R5G6B5:
				{
					LONG x = 0, z = 0;
					WORD *RawBuffer = (WORD*)&rawVideoBuf[0];
					for (LONG j = DestRect.top; j < DestRect.bottom; j++)
					{
						z = j * (d3dlrect.Pitch / 4);
						for (LONG i = DestRect.left; i < DestRect.right; i++)
						{
							x = z + i;

							// More accurate but twice as slow
							/*surfaceBuffer[x] = D3DCOLOR_XRGB(
							((RawBuffer[x] & 0xF800) >> 11) * 255 / 31,				// Red
							((RawBuffer[x] & 0x07E0) >> 5) * 255 / 63,				// Green
							((RawBuffer[x] & 0x001F)) * 255 / 31);					// Blue	*/

							// Fastest but not as accurate
							surfaceBuffer[x] = ((RawBuffer[x] & 0xF800) << 8) +		// Red
								((RawBuffer[x] & 0x07E0) << 5) +					// Green
								((RawBuffer[x] & 0x001F) << 3);						// Blue
						}
					}
					break;
				}
				default:
					Logging::Log() << __FUNCTION__ << " Unsupported format type: " << Format;
					break;
				}
				break;
			case 24:
				Logging::Log() << __FUNCTION__ << " 24-bit surfaces not implimented!";
				break;
			case 32:
				switch (Format)
				{
				case D3DFMT_A8R8G8B8:
				case D3DFMT_X8R8G8B8:
				{
					// Copy rgb video memory to texture memory by scanline observing pitch
					const LONG x = DestRect.right - DestRect.left;
					for (LONG y = DestRect.top; y < DestRect.bottom; y++)
					{
						memcpy((BYTE *)d3dlrect.pBits + (y * d3dlrect.Pitch), &rawVideoBuf[y * x], x * (BitCount / 8));
					}
					break;
				}
				default:
					Logging::Log() << __FUNCTION__ << " Unsupported format type: " << Format;
					break;
				}
				break;
			default:
				Logging::Log() << __FUNCTION__ << " Unsupported bit count: " << BitCount;
				break;
			}
		}


		/*
		A pointer to a RECT structure that was used to lock the surface in the corresponding
		call to the IDirectDrawSurface7::Lock method. This parameter can be NULL only if the
		entire surface was locked by passing NULL in the lpDestRect parameter of the corresponding
		call to the IDirectDrawSurface7::Lock method.

		Because you can call IDirectDrawSurface7::Lock multiple times for the same surface with
		different destination rectangles, the pointer in lpRect links the calls to the
		IDirectDrawSurface7::Lock and IDirectDrawSurface7::Unlock methods.
		*/


		// Make sure surface texture exists
		if (!d3d9Surface || !*d3d9Surface)
		{
			Logging::Log() << __FUNCTION__ << " called when texture doesn't exist";
			return DDERR_SURFACELOST;
		}

		HRESULT hr;

		// Unlock dynamic texture
		hr = ((IDirect3DTexture9*)*d3d9Surface)->UnlockRect(0);
		if (FAILED(hr))
		{
			Logging::Log() << __FUNCTION__ << " Failed to unlock texture memory";
			return hr;
		}

		// Present the surface if needed
		hr = ddrawParent->CheckEndScene(*d3d9Surface);
		if (FAILED(hr))
		{
			// Failed to presnt the surface, error reporting handled previously
			return hr;
		}

		d3dlrect.pBits = nullptr;

		// Success
		return DD_OK;
	}

	return ProxyInterface->Unlock(lpRect);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlay(LPRECT lpSrcRect, LPDIRECTDRAWSURFACE7 lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, LPDDOVERLAYFX lpDDOverlayFx)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
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
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->UpdateOverlayDisplay(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlayZOrder(DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSReference)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	if (lpDDSReference)
	{
		lpDDSReference = static_cast<m_IDirectDrawSurface7 *>(lpDDSReference)->GetProxyInterface();
	}

	return ProxyInterface->UpdateOverlayZOrder(dwFlags, lpDDSReference);
}

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
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->PageLock(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::PageUnlock(DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->PageUnlock(dwFlags);
}

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
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->SetSurfaceDesc(lpDDsd, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::SetPrivateData(REFGUID guidTag, LPVOID lpData, DWORD cbSize, DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->SetPrivateData(guidTag, lpData, cbSize, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetPrivateData(REFGUID guidTag, LPVOID lpBuffer, LPDWORD lpcbBufferSize)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetPrivateData(guidTag, lpBuffer, lpcbBufferSize);
}

HRESULT m_IDirectDrawSurfaceX::FreePrivateData(REFGUID guidTag)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->FreePrivateData(guidTag);
}

HRESULT m_IDirectDrawSurfaceX::GetUniquenessValue(LPDWORD lpValue)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetUniquenessValue(lpValue);
}

HRESULT m_IDirectDrawSurfaceX::ChangeUniquenessValue()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->ChangeUniquenessValue();
}

HRESULT m_IDirectDrawSurfaceX::SetPriority(DWORD dwPriority)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->SetPriority(dwPriority);
}

HRESULT m_IDirectDrawSurfaceX::GetPriority(LPDWORD lpdwPriority)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetPriority(lpdwPriority);
}

HRESULT m_IDirectDrawSurfaceX::SetLOD(DWORD dwMaxLOD)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->SetLOD(dwMaxLOD);
}

HRESULT m_IDirectDrawSurfaceX::GetLOD(LPDWORD lpdwMaxLOD)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetLOD(lpdwMaxLOD);
}
