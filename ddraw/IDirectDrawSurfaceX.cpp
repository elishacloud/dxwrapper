/**
* Copyright (C) 2022 Elisha Riedlinger
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
* Code initial taken from: https://github.com/strangebytes/diablo-ddrawwrapper
*
* Code to create emulated surface taken from: https://github.com/CnCNet/cnc-ddraw
*/

#include <sstream>
#include "ddraw.h"
#include "d3d9ShaderPalette.h"
#include "d3dx9.h"
#include "Utils\Utils.h"

extern float ScaleDDWidthRatio;
extern float ScaleDDHeightRatio;
extern DWORD ScaleDDCurrentWidth;
extern DWORD ScaleDDCurrentHeight;
extern DWORD ScaleDDPadX;
extern DWORD ScaleDDPadY;

// Used to allow presenting non-primary surfaces in case the primary surface present fails
bool dirtyFlag = false;
bool SceneReady = false;
bool IsPresentRunning = false;

// Used for sharing emulated memory
bool ShareEmulatedMemory = false;
CRITICAL_SECTION smcs;
std::vector<EMUSURFACE*> memorySurfaces;

/************************/
/*** IUnknown methods ***/
/************************/

HRESULT m_IDirectDrawSurfaceX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ppvObj)
	{
		return E_POINTER;
	}

	if (riid == IID_GetRealInterface)
	{
		*ppvObj = ProxyInterface;
		return DD_OK;
	}
	if (riid == IID_GetInterfaceX)
	{
		*ppvObj = this;
		return DD_OK;
	}

	if (DirectXVersion != 1 && DirectXVersion != 2 && DirectXVersion != 3 && DirectXVersion != 4 && DirectXVersion != 7)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return DDERR_GENERIC;
	}

	DWORD DxVersion = (CheckWrapperType(riid) && (Config.Dd7to9 || Config.ConvertToDirectDraw7)) ? GetGUIDVersion(riid) : DirectXVersion;

	if (riid == GetWrapperType(DxVersion) || riid == IID_IUnknown)
	{
		*ppvObj = GetWrapperInterfaceX(DxVersion);

		AddRef(DxVersion);

		return DD_OK;
	}

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, false)))
		{
			return DDERR_GENERIC;
		}

		if (riid == IID_IDirect3DHALDevice || riid == IID_IDirect3DRGBDevice || riid == IID_IDirect3DRampDevice || riid == IID_IDirect3DNullDevice)
		{
			DxVersion = (DxVersion == 4) ? 3 : DxVersion;

			m_IDirect3DDeviceX *D3DDeviceX = *ddrawParent->GetCurrentD3DDevice();

			if (D3DDeviceX)
			{
				*ppvObj = D3DDeviceX->GetWrapperInterfaceX(DxVersion);

				D3DDeviceX->AddRef(DxVersion);

				return DD_OK;
			}

			m_IDirect3DX *D3DX = *ddrawParent->GetCurrentD3D();

			if (D3DX)
			{
				D3DX->CreateDevice(riid, (LPDIRECTDRAWSURFACE7)this, (LPDIRECT3DDEVICE7*)&D3DDeviceX, nullptr, DxVersion);

				*ppvObj = D3DDeviceX;

				return DD_OK;
			}

			LOG_LIMIT(100, __FUNCTION__ << " Query failed for " << riid << " from " << GetWrapperType(DirectXVersion));

			return E_NOINTERFACE;
		}
		if (riid == IID_IDirectDrawColorControl)
		{
			return ddrawParent->CreateColorInterface(ppvObj);
		}
		if (riid == IID_IDirectDrawGammaControl)
		{
			return ddrawParent->CreateGammaInterface(ppvObj);
		}
	}

	if (Config.ConvertToDirect3D7 && (riid == IID_IDirect3DTexture || riid == IID_IDirect3DTexture2))
	{
		DxVersion = GetGUIDVersion(riid);

		m_IDirect3DTextureX *InterfaceX = nullptr;
		
		if (attachedTexture)
		{
			InterfaceX = attachedTexture;
			InterfaceX->AddRef(DxVersion);
		}
		else
		{
			InterfaceX = new m_IDirect3DTextureX(ddrawParent->GetCurrentD3DDevice(), DxVersion, this);
			attachedTexture = InterfaceX;
		}

		*ppvObj = InterfaceX->GetWrapperInterfaceX(DxVersion);

		return DD_OK;
	}

	HRESULT hr = ProxyQueryInterface(ProxyInterface, (IID_IDirect3DRampDevice == riid ? IID_IDirect3DRGBDevice : riid), ppvObj, GetWrapperType(DxVersion));

	if (SUCCEEDED(hr) && Config.ConvertToDirect3D7 && ddrawParent)
	{
		if (riid == IID_IDirect3DHALDevice || riid == IID_IDirect3DRGBDevice || riid == IID_IDirect3DRampDevice || riid == IID_IDirect3DNullDevice)
		{
			m_IDirect3DDeviceX *lpD3DDeviceX = nullptr;

			((IDirect3DDevice7*)*ppvObj)->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpD3DDeviceX);

			if (lpD3DDeviceX)
			{
				lpD3DDeviceX->SetDdrawParent(ddrawParent);

				ddrawParent->SetD3DDevice(lpD3DDeviceX);
			}
		}
	}

	return hr;
}

void *m_IDirectDrawSurfaceX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 1:
		return WrapperInterface;
	case 2:
		return WrapperInterface2;
	case 3:
		return WrapperInterface3;
	case 4:
		return WrapperInterface4;
	case 7:
		return WrapperInterface7;
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return nullptr;
	}
}

ULONG m_IDirectDrawSurfaceX::AddRef(DWORD DirectXVersion)
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
		case 4:
			return InterlockedIncrement(&RefCount4);
		case 7:
			return InterlockedIncrement(&RefCount7);
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			return 0;
		}
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawSurfaceX::Release(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") v" << DirectXVersion;

	ULONG ref;

	if (Config.Dd7to9)
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
		case 4:
			ref = (InterlockedCompareExchange(&RefCount4, 0, 0)) ? InterlockedDecrement(&RefCount4) : 0;
			break;
		case 7:
			ref = (InterlockedCompareExchange(&RefCount7, 0, 0)) ? InterlockedDecrement(&RefCount7) : 0;
			break;
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			ref = 0;
		}

		if (InterlockedCompareExchange(&RefCount1, 0, 0) + InterlockedCompareExchange(&RefCount2, 0, 0) +
			InterlockedCompareExchange(&RefCount3, 0, 0) + InterlockedCompareExchange(&RefCount4, 0, 0) +
			InterlockedCompareExchange(&RefCount7, 0, 0) == 0)
		{
			if (CanSurfaceBeDeleted())
			{
				delete this;
			}
			else
			{
				ref = 1;
			}
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

/**********************************/
/*** IDirectDrawSurface methods ***/
/**********************************/

HRESULT m_IDirectDrawSurfaceX::AddAttachedSurface(LPDIRECTDRAWSURFACE7 lpDDSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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

		m_IDirectDrawSurfaceX *lpAttachedSurface = nullptr;

		lpDDSurface->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpAttachedSurface);

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
			((AttachedSurfaceCaps & DDSCAPS_MIPMAP) && (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_MIPMAP)) ||
			(AttachedSurfaceCaps & DDSCAPS_ZBUFFER)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: cannot attach surface with this method. dwCaps: " << Logging::hex(lpAttachedSurface->GetSurfaceCaps().dwCaps));
			return DDERR_CANNOTATTACHSURFACE;
		}

		AddAttachedSurfaceToMap(lpAttachedSurface, true);

		lpDDSurface->AddRef();

		return DD_OK;
	}

	if (lpDDSurface)
	{
		lpDDSurface->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDSurface);
	}

	return ProxyInterface->AddAttachedSurface(lpDDSurface);
}

HRESULT m_IDirectDrawSurfaceX::AddOverlayDirtyRect(LPRECT lpRect)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->AddOverlayDirtyRect(lpRect);
}

HRESULT m_IDirectDrawSurfaceX::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx, bool isSkipScene)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Check if source Surface exists
	if (lpDDSrcSurface && !CheckSurfaceExists(lpDDSrcSurface))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not find source surface! " << Logging::hex(lpDDSrcSurface));
		return DD_OK;	// Just return OK
	}

	if (Config.Dd7to9)
	{
		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
		if (FAILED(c_hr) && !IsUsingEmulation())
		{
			return c_hr;
		}

		// All DDBLT_ALPHA flag values, Not currently implemented in DirectDraw.
		if (dwFlags & (DDBLT_ALPHADEST | DDBLT_ALPHADESTCONSTOVERRIDE | DDBLT_ALPHADESTNEG | DDBLT_ALPHADESTSURFACEOVERRIDE | DDBLT_ALPHAEDGEBLEND |
			DDBLT_ALPHASRC | DDBLT_ALPHASRCCONSTOVERRIDE | DDBLT_ALPHASRCNEG | DDBLT_ALPHASRCSURFACEOVERRIDE))
		{
			return DDERR_NOALPHAHW;
		}

		// All DDBLT_ZBUFFER flag values: This method does not currently support z-aware bitblt operations. None of the flags beginning with "DDBLT_ZBUFFER" are supported in DirectDraw.
		if (dwFlags & (DDBLT_ZBUFFER | DDBLT_ZBUFFERDESTCONSTOVERRIDE | DDBLT_ZBUFFERDESTOVERRIDE | DDBLT_ZBUFFERSRCCONSTOVERRIDE | DDBLT_ZBUFFERSRCOVERRIDE))
		{
			return DDERR_NOZBUFFERHW;
		}

		// DDBLT_DDROPS - dwDDROP is ignored as "no such ROPs are currently defined" in DirectDraw
		if (dwFlags & DDBLT_DDROPS)
		{
			return DDERR_NODDROPSHW;
		}

		// Check for required DDBLTFX structure
		if (!lpDDBltFx && (dwFlags & (DDBLT_DDFX | DDBLT_COLORFILL | DDBLT_DEPTHFILL | DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_ROP | DDBLT_ROTATIONANGLE)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: DDBLTFX structure not found");
			return DDERR_INVALIDPARAMS;
		}

		// Check for DDBLTFX structure size
		if (lpDDBltFx && lpDDBltFx->dwSize != sizeof(DDBLTFX))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: DDBLTFX structure is not initialized to the right size: " << lpDDBltFx->dwSize);
			return DDERR_INVALIDPARAMS;
		}

		// Check for depth fill flag
		if (dwFlags & DDBLT_DEPTHFILL)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Depth Fill Not Implemented");
			return DDERR_NOZBUFFERHW;
		}

		// Check for rotation flags
		// ToDo: add support for other rotation flags (90,180, 270).  Not sure if any game uses these other flags.
		if ((dwFlags & DDBLT_ROTATIONANGLE) || ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & (DDBLTFX_ROTATE90 | DDBLTFX_ROTATE180 | DDBLTFX_ROTATE270))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Rotation operations Not Implemented: " << Logging::hex(lpDDBltFx->dwDDFX & (DDBLTFX_ROTATE90 | DDBLTFX_ROTATE180 | DDBLTFX_ROTATE270)));
			return DDERR_NOROTATIONHW;
		}

		// ToDo: add support for waiting when surface is busy because of another thread
		const bool BltWait = ((dwFlags & DDBLT_WAIT) && (dwFlags & DDBLT_DONOTWAIT) == 0);

		// Other flags, not yet implemented in dxwrapper
		// DDBLT_ASYNC - Current dxwrapper implementation always allows async if calling from multiple threads

		// Check if the scene needs to be presented
		isSkipScene = isSkipScene || ((lpDestRect) ? CheckRectforSkipScene(*lpDestRect) : false);

		// Present before write if needed
		BeginWritePresent(isSkipScene);

		IsInBlt = true;

		HRESULT hr = DD_OK;

		do {
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
					LOG_LIMIT(100, __FUNCTION__ << " Error: Raster operation Not Implemented " << Logging::hex(lpDDBltFx->dwROP));
					hr = DDERR_NORASTEROPHW;
					break;
				}
			}

			// Get source surface
			m_IDirectDrawSurfaceX* lpDDSrcSurfaceX = (m_IDirectDrawSurfaceX*)lpDDSrcSurface;
			if (!lpDDSrcSurfaceX)
			{
				lpDDSrcSurfaceX = this;
			}
			else
			{
				lpDDSrcSurfaceX->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpDDSrcSurfaceX);
			}

			// Get surface copy flags
			DWORD Flags =
				(dwFlags & (DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_KEYDEST | DDBLT_KEYSRC) ? BLT_COLORKEY : 0) |
				((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & DDBLTFX_MIRRORLEFTRIGHT) ? BLT_MIRRORLEFTRIGHT : 0) |
				((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & DDBLTFX_MIRRORUPDOWN) ? BLT_MIRRORUPDOWN : 0);

			// Get color key
			DDCOLORKEY ColorKey = {};
			if (dwFlags & DDBLT_KEYDESTOVERRIDE)
			{
				ColorKey = lpDDBltFx->ddckDestColorkey;
			}
			else if (dwFlags & DDBLT_KEYSRCOVERRIDE)
			{
				ColorKey = lpDDBltFx->ddckSrcColorkey;
			}
			else if ((dwFlags & DDBLT_KEYDEST) && (surfaceDesc2.ddsCaps.dwCaps & DDSD_CKDESTBLT))
			{
				ColorKey = surfaceDesc2.ddckCKDestBlt;
			}
			else if ((dwFlags & DDBLT_KEYSRC) && (lpDDSrcSurfaceX->surfaceDesc2.ddsCaps.dwCaps & DDSD_CKSRCBLT))
			{
				ColorKey = lpDDSrcSurfaceX->surfaceDesc2.ddckCKSrcBlt;
			}
			else if (dwFlags & (DDBLT_KEYDEST | DDBLT_KEYSRC))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: color key not found!");
				Flags &= ~BLT_COLORKEY;
			}

			D3DTEXTUREFILTERTYPE Filter = ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & DDBLTFX_ARITHSTRETCHY)) ? D3DTEXF_LINEAR : D3DTEXF_NONE;

			hr = CopySurface(lpDDSrcSurfaceX, lpSrcRect, lpDestRect, Filter, ColorKey, Flags);

		} while (false);

		// Reset Blt flag
		IsInBlt = false;

		// If successful
		if (SUCCEEDED(hr))
		{
			// Set dirty flag
			SetDirtyFlag();

			// Set vertical sync wait timer
			if (SUCCEEDED(c_hr) && (dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & DDBLTFX_NOTEARING))
			{
				ddrawParent->SetVsync();
			}

			// Present surface
			EndWritePresent(isSkipScene);
		}

		// Check if surface was busy
		if (!BltWait && hr == DDERR_SURFACEBUSY && LockedWithID && LockedWithID != GetCurrentThreadId())
		{
			return D3DERR_WASSTILLDRAWING;
		}

		// Return
		return hr;
	}

	RECT DstRect = { 0, 0, 0, 0 };
	if (Config.DdrawUseNativeResolution)
	{
		if (lpDestRect)
		{
			DstRect.left = (LONG)(lpDestRect->left * ScaleDDWidthRatio) + ScaleDDPadX;
			DstRect.top = (LONG)(lpDestRect->top * ScaleDDHeightRatio) + ScaleDDPadY;
			DstRect.right = (LONG)(lpDestRect->right * ScaleDDWidthRatio) + ScaleDDPadX;
			DstRect.bottom = (LONG)(lpDestRect->bottom * ScaleDDHeightRatio) + ScaleDDPadY;
		}
		else
		{
			DstRect.left = ScaleDDPadX;
			DstRect.top = ScaleDDPadY;
			DstRect.right = ScaleDDCurrentWidth - (ScaleDDPadX * 2);
			DstRect.bottom = ScaleDDCurrentHeight - (ScaleDDPadY * 2);
		}

		DDSURFACEDESC Desc = {};
		Desc.dwSize = sizeof(Desc);
		GetSurfaceDesc(&Desc);

		if ((Desc.ddsCaps.dwCaps & (DDSCAPS_PRIMARYSURFACE | DDSCAPS_FRONTBUFFER | DDSCAPS_BACKBUFFER)) && DstRect.right <= (LONG)ScaleDDCurrentWidth && DstRect.bottom <= (LONG)ScaleDDCurrentHeight)
		{
			lpDestRect = &DstRect;
		}
	}

	if (lpDDSrcSurface)
	{
		lpDDSrcSurface->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDSrcSurface);
	}

	HRESULT hr = ProxyInterface->Blt(lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);

	// Fix for some games that calculate the rect incorrectly
	if (hr == DDERR_INVALIDRECT)
	{
		RECT SrcRect, DestRect;
		if (lpSrcRect)
		{
			memcpy(&SrcRect, lpSrcRect, sizeof(RECT));
			SrcRect.left -= 1;
			SrcRect.bottom -= 1;
			lpSrcRect = &SrcRect;
		}
		if (lpDestRect)
		{
			memcpy(&DestRect, lpDestRect, sizeof(RECT));
			DestRect.left -= 1;
			DestRect.bottom -= 1;
			lpDestRect = &DestRect;
		}
		hr = ProxyInterface->Blt(lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::BltBatch(LPDDBLTBATCH lpDDBltBatch, DWORD dwCount, DWORD dwFlags)
{
	UNREFERENCED_PARAMETER(dwFlags);

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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

HRESULT m_IDirectDrawSurfaceX::BltFast(DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Check if source Surface exists
	if (lpDDSrcSurface && !CheckSurfaceExists(lpDDSrcSurface))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not find source surface! " << Logging::hex(lpDDSrcSurface));
		return DD_OK;	// Just return OK
	}

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

		// Get SurfaceX
		m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = (m_IDirectDrawSurfaceX*)lpDDSrcSurface;
		if (!lpDDSrcSurfaceX)
		{
			lpDDSrcSurfaceX = this;
		}
		else
		{
			lpDDSrcSurfaceX->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpDDSrcSurfaceX);
		}

		// Get SrcRect
		RECT SrcRect = {};
		lpDDSrcSurfaceX->CheckCoordinates(&SrcRect, lpSrcRect);

		// Create DestRect
		RECT DestRect = { (LONG)dwX, (LONG)dwY, SrcRect.right - SrcRect.left + (LONG)dwX , SrcRect.bottom - SrcRect.top + (LONG)dwY };
		LPRECT pDestRect = &DestRect;
		if (!lpSrcRect && !dwX && !dwY)
		{
			pDestRect = nullptr;
		}

		// Call Blt
		return Blt(pDestRect, lpDDSrcSurface, lpSrcRect, Flags, nullptr);
	}

	if (lpDDSrcSurface)
	{
		lpDDSrcSurface->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDSrcSurface);
	}

	if (Config.DdrawUseNativeResolution)
	{
		RECT DstRect = { 0, 0, 0, 0 };
		if (lpSrcRect)
		{
			DstRect.left = (LONG)(dwX * ScaleDDWidthRatio) + ScaleDDPadX;
			DstRect.top = (LONG)(dwY * ScaleDDHeightRatio) + ScaleDDPadY;
			DstRect.right = (LONG)((lpSrcRect->right - lpSrcRect->left) * ScaleDDWidthRatio) + DstRect.left;
			DstRect.bottom = (LONG)((lpSrcRect->bottom - lpSrcRect->top) * ScaleDDHeightRatio) + DstRect.top;
		}
		else
		{
			DstRect.left = ScaleDDPadX;
			DstRect.top = ScaleDDPadY;
			DstRect.right = ScaleDDCurrentWidth - (ScaleDDPadX * 2);
			DstRect.bottom = ScaleDDCurrentHeight - (ScaleDDPadY * 2);
		}

		DDSURFACEDESC Desc = {};
		Desc.dwSize = sizeof(Desc);
		GetSurfaceDesc(&Desc);

		if ((Desc.ddsCaps.dwCaps & (DDSCAPS_PRIMARYSURFACE | DDSCAPS_FRONTBUFFER | DDSCAPS_BACKBUFFER)) && DstRect.right <= (LONG)ScaleDDCurrentWidth && DstRect.bottom <= (LONG)ScaleDDCurrentHeight)
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

			// Call Blt
			return ProxyInterface->Blt(&DstRect, lpDDSrcSurface, lpSrcRect, Flags, nullptr);
		}
	}

	HRESULT hr = ProxyInterface->BltFast(dwX, dwY, lpDDSrcSurface, lpSrcRect, dwFlags);

	// Fix for some games that calculate the rect incorrectly
	if (lpSrcRect && hr == DDERR_INVALIDRECT)
	{
		RECT SrcRect;
		memcpy(&SrcRect, lpSrcRect, sizeof(RECT));
		SrcRect.left -= 1;
		SrcRect.bottom -= 1;
		hr = ProxyInterface->BltFast(dwX, dwY, lpDDSrcSurface, &SrcRect, dwFlags);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::DeleteAttachedSurface(DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSAttachedSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// dwFlags: Reserved. Must be zero.
		if (!lpDDSAttachedSurface || dwFlags)
		{
			return DDERR_INVALIDPARAMS;
		}

		m_IDirectDrawSurfaceX *lpAttachedSurface = nullptr;

		lpDDSAttachedSurface->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpAttachedSurface);

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

		lpDDSAttachedSurface->Release();

		return DD_OK;
	}

	if (lpDDSAttachedSurface)
	{
		lpDDSAttachedSurface->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDSAttachedSurface);
	}

	return ProxyInterface->DeleteAttachedSurface(dwFlags, lpDDSAttachedSurface);
}

HRESULT m_IDirectDrawSurfaceX::EnumAttachedSurfaces(LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lpEnumSurfacesCallback)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (ProxyDirectXVersion > 3)
	{
		return EnumAttachedSurfaces2(lpContext, (LPDDENUMSURFACESCALLBACK7)lpEnumSurfacesCallback, DirectXVersion);
	}

	struct EnumSurface
	{
		LPVOID lpContext;
		LPDDENUMSURFACESCALLBACK lpCallback;
		DWORD DirectXVersion;

		static HRESULT CALLBACK ConvertCallback(LPDIRECTDRAWSURFACE lpDDSurface, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext)
		{
			EnumSurface *self = (EnumSurface*)lpContext;

			if (lpDDSurface)
			{
				lpDDSurface = (LPDIRECTDRAWSURFACE)ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(lpDDSurface, self->DirectXVersion);
			}

			return self->lpCallback(lpDDSurface, lpDDSurfaceDesc, self->lpContext);
		}
	} CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = lpEnumSurfacesCallback;
	CallbackContext.DirectXVersion = DirectXVersion;

	return GetProxyInterfaceV3()->EnumAttachedSurfaces(&CallbackContext, EnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawSurfaceX::EnumAttachedSurfaces2(LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback7, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lpEnumSurfacesCallback7)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		for (auto& it : AttachedSurfaceMap)
		{
			DDSURFACEDESC2 Desc2;
			Desc2.dwSize = sizeof(DDSURFACEDESC2);
			it.second.pSurface->GetSurfaceDesc2(&Desc2);
			if (lpEnumSurfacesCallback7((LPDIRECTDRAWSURFACE7)it.second.pSurface->GetWrapperInterfaceX(DirectXVersion), &Desc2, lpContext) == DDENUMRET_CANCEL)
			{
				return DD_OK;
			}
		}

		return DD_OK;
	}

	struct EnumSurface
	{
		LPVOID lpContext;
		LPDDENUMSURFACESCALLBACK7 lpCallback;
		DWORD DirectXVersion;
		bool ConvertSurfaceDescTo2;

		static HRESULT CALLBACK ConvertCallback(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext)
		{
			EnumSurface *self = (EnumSurface*)lpContext;

			if (lpDDSurface)
			{
				lpDDSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(lpDDSurface, self->DirectXVersion);
			}

			// Game using old DirectX, Convert back to LPDDSURFACEDESC
			if (self->ConvertSurfaceDescTo2)
			{
				DDSURFACEDESC Desc;
				Desc.dwSize = sizeof(DDSURFACEDESC);
				ConvertSurfaceDesc(Desc, *lpDDSurfaceDesc2);

				return ((LPDDENUMSURFACESCALLBACK)self->lpCallback)((LPDIRECTDRAWSURFACE)lpDDSurface, &Desc, self->lpContext);
			}

			return self->lpCallback(lpDDSurface, lpDDSurfaceDesc2, self->lpContext);
		}
	} CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = lpEnumSurfacesCallback7;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ConvertSurfaceDescTo2 = (ProxyDirectXVersion > 3 && DirectXVersion < 4);

	return ProxyInterface->EnumAttachedSurfaces(&CallbackContext, EnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawSurfaceX::EnumOverlayZOrders(DWORD dwFlags, LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpfnCallback, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lpfnCallback)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (ProxyDirectXVersion > 3)
	{
		return EnumOverlayZOrders2(dwFlags, lpContext, (LPDDENUMSURFACESCALLBACK7)lpfnCallback, DirectXVersion);
	}

	struct EnumSurface
	{
		LPVOID lpContext;
		LPDDENUMSURFACESCALLBACK lpCallback;
		DWORD DirectXVersion;

		static HRESULT CALLBACK ConvertCallback(LPDIRECTDRAWSURFACE lpDDSurface, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext)
		{
			EnumSurface *self = (EnumSurface*)lpContext;

			if (lpDDSurface)
			{
				lpDDSurface = (LPDIRECTDRAWSURFACE)ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(lpDDSurface, self->DirectXVersion);
			}

			return self->lpCallback(lpDDSurface, lpDDSurfaceDesc, self->lpContext);
		}
	} CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = lpfnCallback;
	CallbackContext.DirectXVersion = DirectXVersion;

	return GetProxyInterfaceV3()->EnumOverlayZOrders(dwFlags, &CallbackContext, EnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawSurfaceX::EnumOverlayZOrders2(DWORD dwFlags, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpfnCallback7, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lpfnCallback7)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	struct EnumSurface
	{
		LPVOID lpContext;
		LPDDENUMSURFACESCALLBACK7 lpCallback;
		DWORD DirectXVersion;
		bool ConvertSurfaceDescTo2;

		static HRESULT CALLBACK ConvertCallback(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext)
		{
			EnumSurface *self = (EnumSurface*)lpContext;

			if (lpDDSurface)
			{
				lpDDSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(lpDDSurface, self->DirectXVersion);
			}

			// Game using old DirectX, Convert back to LPDDSURFACEDESC
			if (self->ConvertSurfaceDescTo2)
			{
				DDSURFACEDESC Desc;
				Desc.dwSize = sizeof(DDSURFACEDESC);
				ConvertSurfaceDesc(Desc, *lpDDSurfaceDesc2);

				return ((LPDDENUMSURFACESCALLBACK)self->lpCallback)((LPDIRECTDRAWSURFACE)lpDDSurface, &Desc, self->lpContext);
			}

			return self->lpCallback(lpDDSurface, lpDDSurfaceDesc2, self->lpContext);
		}
	} CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = lpfnCallback7;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ConvertSurfaceDescTo2 = (ProxyDirectXVersion > 3 && DirectXVersion < 4);

	return ProxyInterface->EnumOverlayZOrders(dwFlags, &CallbackContext, EnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawSurfaceX::FlipBackBuffer()
{
	DWORD dwCaps = 0;
	m_IDirectDrawSurfaceX *lpTargetSurface = nullptr;

	// Loop through each surface
	for (auto& it : AttachedSurfaceMap)
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
		LOG_LIMIT(100, __FUNCTION__ << " Error: Could not find backbuffer!");
		return DDERR_GENERIC;
	}

	// Check for device interface
	HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
	if (FAILED(c_hr))
	{
		return c_hr;
	}

	// Check if surface is busy
	if (lpTargetSurface->IsSurfaceLocked() || lpTargetSurface->IsSurfaceInDC())
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

HRESULT m_IDirectDrawSurfaceX::Flip(LPDIRECTDRAWSURFACE7 lpDDSurfaceTargetOverride, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		// Flip can be called only for a surface that has the DDSCAPS_FLIP and DDSCAPS_FRONTBUFFER capabilities
		if ((surfaceDesc2.ddsCaps.dwCaps & (DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER)) != (DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: This surface cannot be flipped");
			return DDERR_INVALIDOBJECT;
		}

		// Check if surface is locked or has an open DC
		if (IsSurfaceLocked() || IsSurfaceInDC())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: surface is busy!");

			// On IDirectDrawSurface7 and higher interfaces, the default is DDFLIP_WAIT.
			if (((dwFlags & DDFLIP_WAIT) || DirectXVersion == 7) && (dwFlags & DDFLIP_DONOTWAIT) == 0)
			{
				// ToDo: if surface is busy on another thread then wait for it here.
				return DDERR_SURFACEBUSY;
			}
			else
			{
				return DDERR_WASSTILLDRAWING;
			}
		}

		if ((dwFlags & (DDFLIP_INTERVAL2 | DDFLIP_INTERVAL3 | DDFLIP_INTERVAL4)) && (surfaceDesc2.ddsCaps.dwCaps2 & DDCAPS2_FLIPINTERVAL))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Interval flipping not fully implemented");
		}

		// Present before write if needed
		BeginWritePresent(false);

		// Set flip flag
		IsInFlip = true;

		HRESULT hr = DD_OK;

		do {
			// If SurfaceTargetOverride then use that surface
			if (lpDDSurfaceTargetOverride)
			{
				m_IDirectDrawSurfaceX *lpTargetSurface = nullptr;

				lpDDSurfaceTargetOverride->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpTargetSurface);

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
					hr = DDERR_SURFACEBUSY;
					break;
				}

				// Check if surface is locked or has an open DC
				if (lpTargetSurface->IsSurfaceLocked() || lpTargetSurface->IsSurfaceInDC())
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: backbuffer surface is busy!");
					hr = DDERR_SURFACEBUSY;
					break;
				}

				// Clear dirty surface before flip
				if (DirtyFlip)
				{
					DirtyFlip = false;
					ColorFill(nullptr, 0x00000000);
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
					LOG_LIMIT(100, __FUNCTION__ << " Error: Stereo flipping not implemented");
					hr = DDERR_NOSTEREOHARDWARE;
					break;
				}

				if (dwFlags & (DDFLIP_ODD | DDFLIP_EVEN))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: Even and odd flipping not implemented");
					hr = DDERR_UNSUPPORTED;
					break;
				}

				// Clear dirty surface before flip
				if (DirtyFlip)
				{
					DirtyFlip = false;
					ColorFill(nullptr, 0x00000000);
				}

				// Flip surface
				hr = FlipBackBuffer();
			}

		} while (false);

		// Reset flip flag
		IsInFlip = false;

		// Present surface
		if (SUCCEEDED(hr))
		{
			// Set dirty flag
			SetDirtyFlag();

			// Set vertical sync wait timer
			if ((dwFlags & DDFLIP_NOVSYNC) == 0)
			{
				ddrawParent->SetVsync();
			}

			// Present surface
			EndWritePresent(false);
		}

		return hr;
	}

	if (lpDDSurfaceTargetOverride)
	{
		lpDDSurfaceTargetOverride->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDSurfaceTargetOverride);
	}

	return ProxyInterface->Flip(lpDDSurfaceTargetOverride, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetAttachedSurface(LPDDSCAPS lpDDSCaps, LPDIRECTDRAWSURFACE7 FAR * lplpDDAttachedSurface, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpDDAttachedSurface || !lpDDSCaps2)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, false)))
		{
			return DDERR_GENERIC;
		}

		m_IDirectDrawSurfaceX *lpFoundSurface = nullptr;

		// Check if attached surface exists
		for (auto& it : AttachedSurfaceMap)
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

		*lplpDDAttachedSurface = (LPDIRECTDRAWSURFACE7)lpAttachedSurface;

		(*lplpDDAttachedSurface)->AddRef();

		return DD_OK;
	}

	DDSCAPS2 DDSCaps2;
	
	if (lpDDSCaps2)
	{
		memcpy(&DDSCaps2, lpDDSCaps2, sizeof(DDSCAPS2));

		lpDDSCaps2 = &DDSCaps2;

		if (ProxyDirectXVersion != DirectXVersion)
		{
			DDSCaps2.dwCaps2 = 0;
			DDSCaps2.dwCaps3 = 0;
			DDSCaps2.dwCaps4 = 0;
		}
	}

	HRESULT hr = ProxyInterface->GetAttachedSurface(lpDDSCaps2, lplpDDAttachedSurface);

	if (SUCCEEDED(hr) && lplpDDAttachedSurface)
	{
		*lplpDDAttachedSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpDDAttachedSurface, DirectXVersion);

		if (Config.ConvertToDirectDraw7 && *lplpDDAttachedSurface)
		{
			m_IDirectDrawSurfaceX *lpDDSurfaceX = nullptr;

			(*lplpDDAttachedSurface)->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpDDSurfaceX);

			if (lpDDSurfaceX)
			{
				lpDDSurfaceX->SetDdrawParent(ddrawParent);
			}
		}
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetBltStatus(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Inquires whether a blit involving this surface can occur immediately, and returns DD_OK if the blit can be completed.
		if (dwFlags == DDGBS_CANBLT)
		{
			if (IsInBlt)
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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpDDClipper)
		{
			return DDERR_INVALIDPARAMS;
		}

		// No clipper attached
		if (!attachedClipper)
		{
			*lplpDDClipper = nullptr;
			return DDERR_NOCLIPPERATTACHED;
		}

		// Return attached clipper
		*lplpDDClipper = (LPDIRECTDRAWCLIPPER)attachedClipper;

		// Increase ref counter
		(*lplpDDClipper)->AddRef();

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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check index
		if (!lpDDColorKey)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Get color key index
		DWORD dds = 0;
		switch (dwFlags)
		{
		case DDCKEY_DESTBLT:
			dds = DDSD_CKDESTBLT;
			break;
		case DDCKEY_DESTOVERLAY:
			dds = DDSD_CKDESTOVERLAY;
			break;
		case DDCKEY_SRCBLT:
			dds = DDSD_CKSRCBLT;
			break;
		case DDCKEY_SRCOVERLAY:
			dds = DDSD_CKSRCOVERLAY;
			break;
		default:
			return DDERR_INVALIDPARAMS;
		}

		// Check if color key is set
		if (!(surfaceDesc2.ddsCaps.dwCaps & dds))
		{
			return DDERR_NOCOLORKEY;
		}

		// Set color key
		switch (dds)
		{
		case DDSD_CKDESTBLT:
			*lpDDColorKey = surfaceDesc2.ddckCKDestBlt;
			break;
		case DDSD_CKDESTOVERLAY:
			*lpDDColorKey = surfaceDesc2.ddckCKDestOverlay;
			break;
		case DDSD_CKSRCBLT:
			*lpDDColorKey = surfaceDesc2.ddckCKSrcBlt;
			break;
		case DDSD_CKSRCOVERLAY:
			*lpDDColorKey = surfaceDesc2.ddckCKSrcOverlay;
			break;
		}

		// Return
		return DD_OK;
	}

	return ProxyInterface->GetColorKey(dwFlags, lpDDColorKey);
}

HRESULT m_IDirectDrawSurfaceX::GetDC(HDC FAR * lphDC)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lphDC)
		{
			return DDERR_INVALIDPARAMS;
		}

		*lphDC = nullptr;

		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
		if (FAILED(c_hr) && !IsUsingEmulation() && !DCRequiresEmulation)
		{
			return c_hr;
		}

		if (LastDC && IsSurfaceInDC())
		{
			*lphDC = LastDC;
			return DD_OK;
		}

		if (IsSurfaceInDC())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: does not support getting device context twice!");
			return DDERR_GENERIC;
		}

		// Present before write if needed
		BeginWritePresent(false);

		if (IsUsingEmulation() || DCRequiresEmulation)
		{
			if (!IsUsingEmulation())
			{
				if (FAILED(CreateDCSurface()))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not create DC!");
					return DDERR_GENERIC;
				}

				CopyToEmulatedSurface(nullptr);
			}

			// Set new palette data
			UpdatePaletteData();

			// Read surface from GDI
			if (Config.DdrawReadFromGDI && (IsPrimarySurface() || IsBackBuffer()))
			{
				RECT Rect = { 0, 0, (LONG)surfaceDesc2.dwWidth, (LONG)surfaceDesc2.dwHeight };
				CopyEmulatedSurfaceFromGDI(Rect);
			}

			*lphDC = emu->surfaceDC;
		}
		else if (surfaceTexture)
		{
			if (!contextSurface && FAILED(surfaceTexture->GetSurfaceLevel(0, &contextSurface)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surface level!");
				return DDERR_GENERIC;
			}

			if (FAILED(contextSurface->GetDC(lphDC)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get device context!");
				return DDERR_GENERIC;
			}
		}
		else if (surface3D)
		{
			if (FAILED(surface3D->GetDC(lphDC)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get device context!");
				return DDERR_GENERIC;
			}
		}
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
			return DDERR_GENERIC;
		}

		// Set DC flag
		IsInDC = true;

		// Set LastDC
		LastDC = *lphDC;

		// Set dirty flag
		SetDirtyFlag();

		return DD_OK;
	}

	return ProxyInterface->GetDC(lphDC);
}

HRESULT m_IDirectDrawSurfaceX::GetFlipStatus(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Get backbuffer
		m_IDirectDrawSurfaceX *lpBackBuffer = this;
		for (auto& it : AttachedSurfaceMap)
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
			if (IsInFlip || IsSurfaceLocked() || IsSurfaceInDC() || lpBackBuffer->IsSurfaceLocked() || lpBackBuffer->IsSurfaceInDC())
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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpDDPalette)
		{
			return DDERR_INVALIDPARAMS;
		}

		// No palette attached
		if (!attachedPalette)
		{
			*lplpDDPalette = nullptr;
			return DDERR_NOPALETTEATTACHED;
		}

		// Return attached palette
		*lplpDDPalette = (LPDIRECTDRAWPALETTE)attachedPalette;

		// Increase ref counter
		(*lplpDDPalette)->AddRef();

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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpDDPixelFormat || lpDDPixelFormat->dwSize != sizeof(DDPIXELFORMAT))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDPixelFormat) ? lpDDPixelFormat->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		// Update surface description
		UpdateSurfaceDesc();

		// Copy pixel format to lpDDPixelFormat
		ConvertPixelFormat(*lpDDPixelFormat, surfaceDesc2.ddpfPixelFormat);

		return DD_OK;
	}

	return ProxyInterface->GetPixelFormat(lpDDPixelFormat);
}

HRESULT m_IDirectDrawSurfaceX::GetSurfaceDesc(LPDDSURFACEDESC lpDDSurfaceDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (ProxyDirectXVersion > 3)
	{
		if (!lpDDSurfaceDesc || lpDDSurfaceDesc->dwSize != sizeof(DDSURFACEDESC))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc) ? lpDDSurfaceDesc->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);

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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpDDSurfaceDesc2 || lpDDSurfaceDesc2->dwSize != sizeof(DDSURFACEDESC2))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc2) ? lpDDSurfaceDesc2->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		// Update surfacedesc
		UpdateSurfaceDesc();

		// Copy surfacedesc to lpDDSurfaceDesc2
		ConvertSurfaceDesc(*lpDDSurfaceDesc2, surfaceDesc2);

		// Set lPitch
		if ((lpDDSurfaceDesc2->dwFlags & (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT)) == (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT) &&
			(lpDDSurfaceDesc2->ddpfPixelFormat.dwFlags & DDPF_RGB) && !(lpDDSurfaceDesc2->dwFlags & DDSD_PITCH) && !(lpDDSurfaceDesc2->dwFlags & DDSD_LINEARSIZE))
		{
			lpDDSurfaceDesc2->dwFlags |= DDSD_PITCH;
			lpDDSurfaceDesc2->lPitch = ComputePitch(GetByteAlignedWidth(surfaceDesc2.dwWidth, surfaceBitCount), GetBitCount(lpDDSurfaceDesc2->ddpfPixelFormat));
		}

		if (!(surfaceDesc2.dwFlags & DDSD_LPSURFACE))
		{
			lpDDSurfaceDesc2->lpSurface = nullptr;
		}

		// Return
		return DD_OK;
	}

	return ProxyInterface->GetSurfaceDesc(lpDDSurfaceDesc2);
}

HRESULT m_IDirectDrawSurfaceX::Initialize(LPDIRECTDRAW lpDD, LPDDSURFACEDESC lpDDSurfaceDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 3)
	{
		if (lpDDSurfaceDesc && lpDDSurfaceDesc->dwSize != sizeof(DDSURFACEDESC))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc) ? lpDDSurfaceDesc->dwSize : -1));
		}

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		if (lpDDSurfaceDesc)
		{
			ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);
		}

		return Initialize2(lpDD, (lpDDSurfaceDesc) ? &Desc2 : nullptr);
	}

	if (lpDD)
	{
		lpDD->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDD);
	}

	return GetProxyInterfaceV3()->Initialize(lpDD, lpDDSurfaceDesc);
}

HRESULT m_IDirectDrawSurfaceX::Initialize2(LPDIRECTDRAW lpDD, LPDDSURFACEDESC2 lpDDSurfaceDesc2)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Because the DirectDrawSurface object is initialized when it is created, this method always returns DDERR_ALREADYINITIALIZED.
		return DDERR_ALREADYINITIALIZED;
	}

	if (lpDD)
	{
		lpDD->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDD);
	}

	return ProxyInterface->Initialize(lpDD, lpDDSurfaceDesc2);
}

HRESULT m_IDirectDrawSurfaceX::IsLost()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, false);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		// Check device status
		switch ((*d3d9Device)->TestCooperativeLevel())
		{
		case D3D_OK:
			return DD_OK;
		case D3DERR_DEVICELOST:
		case D3DERR_DEVICENOTRESET:
			return DDERR_SURFACELOST;
		case D3DERR_DRIVERINTERNALERROR:
		case D3DERR_INVALIDCALL:
		default:
			return DDERR_GENERIC;
		}
	}

	return ProxyInterface->IsLost();
}

HRESULT m_IDirectDrawSurfaceX::Lock(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (ProxyDirectXVersion > 3)
	{
		if (!lpDDSurfaceDesc || lpDDSurfaceDesc->dwSize != sizeof(DDSURFACEDESC))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc) ? lpDDSurfaceDesc->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);

		HRESULT hr = Lock2(lpDestRect, &Desc2, dwFlags, hEvent, DirectXVersion);

		// Convert back to LPDDSURFACEDESC
		ConvertSurfaceDesc(*lpDDSurfaceDesc, Desc2);

		return hr;
	}

	HRESULT hr = GetProxyInterfaceV3()->Lock(lpDestRect, lpDDSurfaceDesc, dwFlags, hEvent);

	if (SUCCEEDED(hr))
	{
		// Fix misaligned bytes
		if (Config.DdrawFixByteAlignment)
		{
			LockBitAlign<LPDDSURFACEDESC>(lpDestRect, lpDDSurfaceDesc);
		}
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::Lock2(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD dwFlags, HANDLE hEvent, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check surfaceDesc size
		if (lpDDSurfaceDesc2 && lpDDSurfaceDesc2->dwSize == sizeof(DDSURFACEDESC))
		{
			return Lock(lpDestRect, (LPDDSURFACEDESC)lpDDSurfaceDesc2, dwFlags, hEvent, DirectXVersion);
		}

		// Check surfaceDesc
		if (!lpDDSurfaceDesc2 || lpDDSurfaceDesc2->dwSize != sizeof(DDSURFACEDESC2))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc2) ? lpDDSurfaceDesc2->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);

		// Prepare surfaceDesc
		ZeroMemory(lpDDSurfaceDesc2, sizeof(DDSURFACEDESC2));
		lpDDSurfaceDesc2->dwSize = sizeof(DDSURFACEDESC2);

		ConvertSurfaceDesc(*lpDDSurfaceDesc2, surfaceDesc2);
		if (!(lpDDSurfaceDesc2->dwFlags & DDSD_LPSURFACE))
		{
			lpDDSurfaceDesc2->lpSurface = nullptr;
		}

		// Return error for CheckInterface after preparing surfaceDesc
		if (FAILED(c_hr) && !IsUsingEmulation())
		{
			return c_hr;
		}

		// Check for already locked state
		if (!lpDestRect && !surfaceLockRectList.empty())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: locking surface with NULL rect when surface is already locked!");
			return DDERR_INVALIDRECT;
		}

		// Update rect
		RECT DestRect = {};
		if (!CheckCoordinates(&DestRect, lpDestRect) || (lpDestRect && (lpDestRect->left < 0 || lpDestRect->top < 0 ||
			lpDestRect->right <= lpDestRect->left || lpDestRect->bottom <= lpDestRect->top ||
			lpDestRect->right >(LONG)surfaceDesc2.dwWidth || lpDestRect->bottom >(LONG)surfaceDesc2.dwHeight)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << lpDestRect);
			return DDERR_INVALIDRECT;
		}

		// Convert flags to d3d9
		const bool LockWait = (((dwFlags & DDLOCK_WAIT) || DirectXVersion == 7) && (dwFlags & DDLOCK_DONOTWAIT) == 0);
		DWORD Flags = (dwFlags & (D3DLOCK_READONLY | D3DLOCK_NOOVERWRITE | (!IsPrimarySurface() ? D3DLOCK_NOSYSLOCK : 0))) |
			(!LockWait ? D3DLOCK_DONOTWAIT : 0) |
			((dwFlags & DDLOCK_NODIRTYUPDATE) ? D3DLOCK_NO_DIRTY_UPDATE : 0);

		// Check if the scene needs to be presented
		bool isSkipScene = (CheckRectforSkipScene(DestRect) || (Flags & D3DLOCK_READONLY));

		// Present before write if needed
		BeginWritePresent(isSkipScene);

		// Emulated surface
		D3DLOCKED_RECT LockedRect = {};
		if (IsUsingEmulation())
		{
			// Set locked rect
			if (FAILED(LockEmulatedSurface(&LockedRect, &DestRect)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock emulated surface!");
				return DDERR_GENERIC;
			}

			// Read surface from GDI
			if (Config.DdrawReadFromGDI && (IsPrimarySurface() || IsBackBuffer()))
			{
				CopyEmulatedSurfaceFromGDI(DestRect);
			}
		}
		// Lock surface
		else if (surfaceTexture || surface3D)
		{
			// Try to lock the rect
			HRESULT hr = LockD39Surface(&LockedRect, &DestRect, Flags);
			if (FAILED(hr))
			{
				while (LockWait && LockedWithID && LockedWithID != GetCurrentThreadId())
				{
					Sleep(0);
					if (!surfaceTexture && !surface3D)
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: surface texture missing!");
						return DDERR_SURFACELOST;
					}
				}
				hr = LockD39Surface(&LockedRect, &DestRect, Flags);
				if (FAILED(hr))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock surface texture." << (surface3D ? " is 3DSurface" : " is Texture") <<
						" Size: " << surfaceDesc2.dwWidth << "x" << surfaceDesc2.dwHeight << " Format: " << surfaceFormat <<
						" dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps) << " IsLocked: " << IsLocked << " IsInDC: " << IsInDC);
					return (hr == D3DERR_WASSTILLDRAWING || (LockedWithID && !LockWait)) ? DDERR_WASSTILLDRAWING :
						DDERR_GENERIC;
				}
			}
		}
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
			return DDERR_GENERIC;
		}

		// Check pointer and pitch
		if (!LockedRect.pBits || !LockedRect.Pitch)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get surface address or pitch!");
			return DDERR_GENERIC;
		}

		// Set locked ID
		LockedWithID = GetCurrentThreadId();

		// Set lock flag
		IsLocked = true;

		// Store locked rect
		if (lpDestRect)
		{
			RECT lRect = { lpDestRect->left, lpDestRect->top, lpDestRect->right, lpDestRect->bottom };
			surfaceLockRectList.push_back(lRect);
		}

		// Set surfaceDesc
		if (!(lpDDSurfaceDesc2->dwFlags & DDSD_LPSURFACE))
		{
			lpDDSurfaceDesc2->lpSurface = LockedRect.pBits;
			lpDDSurfaceDesc2->dwFlags |= DDSD_LPSURFACE;
		}
		lpDDSurfaceDesc2->lPitch = LockedRect.Pitch * (ISDXTEX(surfaceFormat) ? 64 : 1);
		lpDDSurfaceDesc2->dwFlags |= DDSD_PITCH;

		// Fix misaligned bytes
		if (Config.DdrawFixByteAlignment)
		{
			LockBitAlign<LPDDSURFACEDESC2>(lpDestRect, lpDDSurfaceDesc2);
		}

		// Backup last rect before removing scanlines
		LastLock.ReadOnly = ((Flags & D3DLOCK_READONLY) != 0);
		LastLock.isSkipScene = isSkipScene;
		LastLock.Rect = DestRect;
		LastLock.LockedRect.pBits = LockedRect.pBits;
		LastLock.LockedRect.Pitch = LockedRect.Pitch;

		// Restore scanlines before returing surface memory
		if (Config.DdrawRemoveScanlines)
		{
			RestoreScanlines(LastLock);
		}

		// Set dirty flag
		if (!(Flags & D3DLOCK_READONLY))
		{
			SetDirtyFlag();
		}

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->Lock(lpDestRect, lpDDSurfaceDesc2, dwFlags, hEvent);

	if (SUCCEEDED(hr))
	{
		// Fix misaligned bytes
		if (Config.DdrawFixByteAlignment)
		{
			LockBitAlign<LPDDSURFACEDESC2>(lpDestRect, lpDDSurfaceDesc2);
		}
	}

	return hr;
}

inline HRESULT m_IDirectDrawSurfaceX::LockD39Surface(D3DLOCKED_RECT* pLockedRect, RECT* pRect, DWORD Flags)
{
	// Lock surface texture
	if (surfaceTexture)
	{
		return surfaceTexture->LockRect(0, pLockedRect, pRect, Flags);
	}
	// Lock 3D surface
	else if (surface3D)
	{
		return surface3D->LockRect(pLockedRect, pRect, Flags);
	}

	return DDERR_GENERIC;
}

HRESULT m_IDirectDrawSurfaceX::ReleaseDC(HDC hDC)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
		if (FAILED(c_hr) && !IsUsingEmulation() && !DCRequiresEmulation)
		{
			return c_hr;
		}

		if (!IsSurfaceInDC())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not in DC!");
			return DDERR_GENERIC;
		}

		if (IsUsingEmulation() || DCRequiresEmulation)
		{
			// Copy emulated surface to real texture
			CopyFromEmulatedSurface(nullptr);

			// Blt surface directly to GDI
			if (Config.DdrawWriteToGDI && (IsPrimarySurface() || IsBackBuffer()))
			{
				RECT Rect = { 0, 0, (LONG)surfaceDesc2.dwWidth, (LONG)surfaceDesc2.dwHeight };
				CopyEmulatedSurfaceToGDI(Rect);
			}
		}
		else if (surfaceTexture)
		{
			if (!contextSurface || FAILED(contextSurface->ReleaseDC(hDC)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to release surface DC!");
				return DDERR_GENERIC;
			}
		}
		else if (surface3D)
		{
			if (FAILED(surface3D->ReleaseDC(hDC)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to release surface DC!");
				return DDERR_GENERIC;
			}
		}
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
			return DDERR_GENERIC;
		}

		// Reset DC flag
		IsInDC = false;

		// Set LastDC
		LastDC = nullptr;

		// Present surface
		EndWritePresent(false);

		return DD_OK;
	}

	return ProxyInterface->ReleaseDC(hDC);
}

HRESULT m_IDirectDrawSurfaceX::Restore()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, false);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		// Check device status
		switch ((*d3d9Device)->TestCooperativeLevel())
		{
		case D3D_OK:
			return DD_OK;
		case D3DERR_DEVICENOTRESET:
			if (SUCCEEDED(ddrawParent->ReinitDevice()))
			{
				return DD_OK;
			}
			[[fallthrough]];
		case D3DERR_DEVICELOST:
			return DDERR_WRONGMODE;
		case D3DERR_DRIVERINTERNALERROR:
		case D3DERR_INVALIDCALL:
		default:
			return DDERR_GENERIC;
		}
	}

	return ProxyInterface->Restore();
}

HRESULT m_IDirectDrawSurfaceX::SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// If lpDDClipper is nullptr then detach the current clipper if it exists
		if (!lpDDClipper)
		{
			if (!attachedClipper)
			{
				return DDERR_NOCLIPPERATTACHED;
			}

			// Decrement ref count
			attachedClipper->Release();

			// Detach
			attachedClipper = nullptr;

			// Reset FirstRun
			ClipperFirstRun = true;
		}
		else
		{
			// Check if palette exists
			if (!ddrawParent || !ddrawParent->DoesClipperExist((m_IDirectDrawClipper*)lpDDClipper))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not find clipper");
				return DDERR_INVALIDOBJECT;
			}

			// Set clipper address
			attachedClipper = (m_IDirectDrawClipper*)lpDDClipper;

			// When you call SetClipper to set a clipper to a surface for the first time, 
			// SetClipper increments the clipper's reference count; subsequent calls to 
			// SetClipper do not affect the clipper's reference count.
			if (ClipperFirstRun)
			{
				attachedClipper->AddRef();
				ClipperFirstRun = false;
			}
		}

		return DD_OK;
	}

	if (lpDDClipper)
	{
		lpDDClipper->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDClipper);
	}

	return ProxyInterface->SetClipper(lpDDClipper);
}

HRESULT m_IDirectDrawSurfaceX::SetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Get color key index
		DWORD dds = 0;
		switch (dwFlags & ~DDCKEY_COLORSPACE)
		{
		case DDCKEY_DESTBLT:
			dds = DDSD_CKDESTBLT;
			break;
		case DDCKEY_DESTOVERLAY:
			dds = DDSD_CKDESTOVERLAY;
			break;
		case DDCKEY_SRCBLT:
			dds = DDSD_CKSRCBLT;
			break;
		case DDCKEY_SRCOVERLAY:
			dds = DDSD_CKSRCOVERLAY;
			break;
		default:
			return DDERR_INVALIDPARAMS;
		}

		// Set color key
		if (!lpDDColorKey)
		{
			surfaceDesc2.ddsCaps.dwCaps &= ~dds;
		}
		else
		{
			// Set color key flag
			surfaceDesc2.ddsCaps.dwCaps |= dds;

			// Get ColorKey
			DDCOLORKEY ColorKey;
			if (!(dwFlags & DDCKEY_COLORSPACE))
			{
				// You must add the flag DDCKEY_COLORSPACE, otherwise DirectDraw will collapse the range to one value
				ColorKey.dwColorSpaceLowValue = *(DWORD*)lpDDColorKey;
				ColorKey.dwColorSpaceHighValue = *(DWORD*)lpDDColorKey;
			}
			else
			{
				ColorKey = *lpDDColorKey;
			}

			// Make sure HighValue is not lower than LowValue 
			if (ColorKey.dwColorSpaceHighValue < ColorKey.dwColorSpaceLowValue)
			{
				ColorKey.dwColorSpaceHighValue = ColorKey.dwColorSpaceLowValue;
			}

			// Set color key
			switch (dds)
			{
			case DDSD_CKDESTBLT:
				surfaceDesc2.ddckCKDestBlt = ColorKey;
				break;
			case DDSD_CKDESTOVERLAY:
				surfaceDesc2.ddckCKDestOverlay = ColorKey;
				break;
			case DDSD_CKSRCBLT:
				surfaceDesc2.ddckCKSrcBlt = ColorKey;
				break;
			case DDSD_CKSRCOVERLAY:
				surfaceDesc2.ddckCKSrcOverlay = ColorKey;
				break;
			}
		}

		// Return
		return DD_OK;
	}

	return ProxyInterface->SetColorKey(dwFlags, lpDDColorKey);
}

HRESULT m_IDirectDrawSurfaceX::SetOverlayPosition(LONG lX, LONG lY)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// If lpDDPalette is nullptr then detach the current palette if it exists
		if (!lpDDPalette)
		{
			if (!attachedPalette)
			{
				return DDERR_NOPALETTEATTACHED;
			}

			// Decrement ref count
			attachedPalette->Release();

			// Detach
			attachedPalette = nullptr;

			// Reset FirstRun
			PaletteFirstRun = true;
		}
		// If lpDDPalette is not null
		else
		{
			// Check if palette exists
			if (!ddrawParent || !ddrawParent->DoesPaletteExist((m_IDirectDrawPalette*)lpDDPalette))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not find palette");
				return DDERR_INVALIDOBJECT;
			}

			// Set palette address
			attachedPalette = (m_IDirectDrawPalette*)lpDDPalette;

			// When you call SetPalette to set a palette to a surface for the first time, 
			// SetPalette increments the palette's reference count; subsequent calls to 
			// SetPalette do not affect the palette's reference count.
			if (PaletteFirstRun)
			{
				attachedPalette->AddRef();
				PaletteFirstRun = false;
			}

			// Set primary
			if (IsPrimarySurface())
			{
				attachedPalette->SetPrimary();
			}
		}

		// If new palette is set
		PaletteUSN++;

		// Set new palette data
		UpdatePaletteData();

		return DD_OK;
	}

	if (lpDDPalette)
	{
		lpDDPalette->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDPalette);
	}

	return ProxyInterface->SetPalette(lpDDPalette);
}

HRESULT m_IDirectDrawSurfaceX::Unlock(LPRECT lpRect)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Fix issue with some games that ignore the pitch size
	if (EmuLock.Locked && EmuLock.Addr)
	{
		BYTE* InAddr = &EmuLock.surfaceMem[0];
		DWORD InPitch = (EmuLock.BBP / 8) * EmuLock.Width;
		BYTE* OutAddr = (BYTE*)EmuLock.Addr;
		DWORD OutPitch = EmuLock.Pitch;
		for (DWORD x = 0; x < EmuLock.Height; x++)
		{
			memcpy(OutAddr, InAddr, InPitch);
			InAddr += InPitch;
			OutAddr += OutPitch;
		}

		EmuLock.Locked = false;
		EmuLock.Addr = nullptr;
	}

	if (Config.Dd7to9)
	{
		// Check rect
		if (!lpRect && surfaceLockRectList.size() > 1)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Rect cannot be NULL when locked with a specific rect!");
			return DDERR_INVALIDRECT;
		}

		// Check stored rect
		if (lpRect && surfaceLockRectList.size() > 1)
		{
			auto it = std::find_if(surfaceLockRectList.begin(), surfaceLockRectList.end(),
				[=](auto Rect) -> bool { return (Rect.left == lpRect->left && Rect.top == lpRect->top && Rect.right == lpRect->right && Rect.bottom == lpRect->bottom); });

			if (it != std::end(surfaceLockRectList))
			{
				surfaceLockRectList.erase(it);

				// Unlock once all rects have been unlocked
				if (!surfaceLockRectList.empty())
				{
					return DD_OK;
				}
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Rect does not match locked rect: " << lpRect);
				return DDERR_INVALIDRECT;
			}
		}

		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
		if (FAILED(c_hr) && !IsUsingEmulation())
		{
			return c_hr;
		}

		// Remove scanlines before unlocking surface
		if (Config.DdrawRemoveScanlines)
		{
			RemoveScanlines(LastLock);
		}

		// Emulated surface
		if (IsUsingEmulation())
		{
			if (!LastLock.ReadOnly)
			{
				// Copy emulated surface to real texture
				CopyFromEmulatedSurface(&LastLock.Rect);

				// Blt surface directly to GDI
				if (Config.DdrawWriteToGDI && (IsPrimarySurface() || IsBackBuffer()))
				{
					CopyEmulatedSurfaceToGDI(LastLock.Rect);
				}
			}
		}
		// Lock surface
		else if (surfaceTexture || surface3D)
		{
			HRESULT hr = UnlockD39Surface();
			if (FAILED(hr))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to unlock surface texture");
				return (hr == D3DERR_INVALIDCALL) ? DDERR_GENERIC :
					(hr == D3DERR_WASSTILLDRAWING) ? DDERR_WASSTILLDRAWING :
					DDERR_SURFACELOST;
			}
		}
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
			return DDERR_GENERIC;
		}

		// Reset locked ID
		LockedWithID = 0;

		// Clear memory pointer
		LastLock.LockedRect.pBits = nullptr;

		// Clear vector
		surfaceLockRectList.clear();

		// Reset locked flag
		IsLocked = false;

		// Present surface
		EndWritePresent(LastLock.isSkipScene);

		return DD_OK;
	}

	return ProxyInterface->Unlock(lpRect);
}

inline HRESULT m_IDirectDrawSurfaceX::UnlockD39Surface()
{
	// Lock surface texture
	if (surfaceTexture)
	{
		return surfaceTexture->UnlockRect(0);
	}
	// Lock 3D surface
	else if (surface3D)
	{
		return surface3D->UnlockRect();
	}

	return DDERR_GENERIC;
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlay(LPRECT lpSrcRect, LPDIRECTDRAWSURFACE7 lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, LPDDOVERLAYFX lpDDOverlayFx)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (lpDDOverlayFx && lpDDOverlayFx->dwSize != sizeof(DDOVERLAYFX))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDOverlayFx) ? lpDDOverlayFx->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpDDDestSurface)
	{
		lpDDDestSurface->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDDestSurface);
	}

	return ProxyInterface->UpdateOverlay(lpSrcRect, lpDDDestSurface, lpDestRect, dwFlags, lpDDOverlayFx);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlayDisplay(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->UpdateOverlayDisplay(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlayZOrder(DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSReference)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpDDSReference)
	{
		lpDDSReference->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDSReference);
	}

	return ProxyInterface->UpdateOverlayZOrder(dwFlags, lpDDSReference);
}

/*********************************/
/*** Added in the v2 interface ***/
/*********************************/

HRESULT m_IDirectDrawSurfaceX::GetDDInterface(LPVOID FAR * lplpDD, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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

		ddrawParent->AddRef(DirectXVersion);

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
			((IUnknown*)*lplpDD)->Release();
		}
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::PageLock(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (ProxyDirectXVersion > 3)
	{
		if (!lpDDSurfaceDesc)
		{
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);

		return SetSurfaceDesc2(&Desc2, dwFlags);
	}

	// Adjust Height and Width
	if (lpDDSurfaceDesc && (lpDDSurfaceDesc->dwFlags & DDSD_WIDTH) && (lpDDSurfaceDesc->dwFlags & DDSD_HEIGHT))
	{
		SetWrapperSurfaceSize(lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight);
		lpDDSurfaceDesc->dwWidth += lpDDSurfaceDesc->dwWidth % 2;
	}

	return GetProxyInterfaceV3()->SetSurfaceDesc(lpDDSurfaceDesc, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::SetSurfaceDesc2(LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
			LOG_LIMIT(100, __FUNCTION__ << " Warning: lpSurface not fully Implemented.");

			SurfaceFlags &= ~DDSD_LPSURFACE;
			surfaceDesc2.dwFlags |= DDSD_LPSURFACE;
			surfaceDesc2.lpSurface = lpDDSurfaceDesc2->lpSurface;
		}
		if (SurfaceFlags)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: flags not implemented " << Logging::hex(SurfaceFlags));
			return DDERR_UNSUPPORTED;
		}
		return DD_OK;
	}

	// Adjust Height and Width
	if (lpDDSurfaceDesc2 && (lpDDSurfaceDesc2->dwFlags & DDSD_WIDTH) && (lpDDSurfaceDesc2->dwFlags & DDSD_HEIGHT))
	{
		SetWrapperSurfaceSize(lpDDSurfaceDesc2->dwWidth, lpDDSurfaceDesc2->dwHeight);
		lpDDSurfaceDesc2->dwWidth += lpDDSurfaceDesc2->dwWidth % 2;
	}

	return ProxyInterface->SetSurfaceDesc(lpDDSurfaceDesc2, dwFlags);
}

/*********************************/
/*** Added in the v4 interface ***/
/*********************************/

HRESULT m_IDirectDrawSurfaceX::SetPrivateData(REFGUID guidTag, LPVOID lpData, DWORD cbSize, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		if (surfaceTexture)
		{
			return surfaceTexture->SetPrivateData(guidTag, lpData, cbSize, dwFlags);
		}
		else if (surface3D)
		{
			return surface3D->SetPrivateData(guidTag, lpData, cbSize, dwFlags);
		}
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
			return DDERR_GENERIC;
		}
	}

	return ProxyInterface->SetPrivateData(guidTag, lpData, cbSize, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetPrivateData(REFGUID guidTag, LPVOID lpBuffer, LPDWORD lpcbBufferSize)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		if (surfaceTexture)
		{
			return surfaceTexture->GetPrivateData(guidTag, lpBuffer, lpcbBufferSize);
		}
		else if (surface3D)
		{
			return surface3D->GetPrivateData(guidTag, lpBuffer, lpcbBufferSize);
		}
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
			return DDERR_GENERIC;
		}
	}

	return ProxyInterface->GetPrivateData(guidTag, lpBuffer, lpcbBufferSize);
}

HRESULT m_IDirectDrawSurfaceX::FreePrivateData(REFGUID guidTag)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		if (surfaceTexture)
		{
			return surfaceTexture->FreePrivateData(guidTag);
		}
		else if (surface3D)
		{
			return surface3D->FreePrivateData(guidTag);
		}
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
			return DDERR_GENERIC;
		}
	}

	return ProxyInterface->FreePrivateData(guidTag);
}

HRESULT m_IDirectDrawSurfaceX::GetUniquenessValue(LPDWORD lpValue)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpValue)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (IsSurfaceLocked() || IsSurfaceInDC())
		{
			// The only defined uniqueness value is 0, which indicates that the surface is likely to be changing beyond the control of DirectDraw.
			*lpValue = 0;
		}
		else
		{
			*lpValue = UniquenessValue;
		}
		return DD_OK;
	}

	return ProxyInterface->GetUniquenessValue(lpValue);
}

HRESULT m_IDirectDrawSurfaceX::ChangeUniquenessValue()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Manually updates the uniqueness value for this surface.
		UniquenessValue += 1;
		return DD_OK;
	}

	return ProxyInterface->ChangeUniquenessValue();
}

/***********************************/
/*** Moved Texture7 methods here ***/
/***********************************/

HRESULT m_IDirectDrawSurfaceX::SetPriority(DWORD dwPriority)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Applications can call this method only for managed textures (those surfaces that were created with the DDSCAPS2_TEXTUREMANAGE flag).
		if ((surfaceDesc2.ddsCaps.dwCaps2 & (DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE)) == 0)
		{
			return DDERR_INVALIDOBJECT;
		}

		Priority = dwPriority;

		return DD_OK;
	}

	return ProxyInterface->SetPriority(dwPriority);
}

HRESULT m_IDirectDrawSurfaceX::GetPriority(LPDWORD lpdwPriority)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwPriority)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Applications can call this method only for managed textures (those surfaces that were created with the DDSCAPS2_TEXTUREMANAGE flag).
		if ((surfaceDesc2.ddsCaps.dwCaps2 & (DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE)) == 0)
		{
			return DDERR_INVALIDOBJECT;
		}

		*lpdwPriority = Priority;

		return DD_OK;
	}

	return ProxyInterface->GetPriority(lpdwPriority);
}

HRESULT m_IDirectDrawSurfaceX::SetLOD(DWORD dwMaxLOD)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Applications can call this method only for managed textures (those surfaces that were created with the DDSCAPS2_TEXTUREMANAGE flag).
		if ((surfaceDesc2.ddsCaps.dwCaps2 & (DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE)) == 0)
		{
			return DDERR_INVALIDOBJECT;
		}

		MaxLOD = dwMaxLOD;

		return DD_OK;
	}

	return ProxyInterface->SetLOD(dwMaxLOD);
}

HRESULT m_IDirectDrawSurfaceX::GetLOD(LPDWORD lpdwMaxLOD)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwMaxLOD)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Applications can call this method only for managed textures (those surfaces that were created with the DDSCAPS2_TEXTUREMANAGE flag).
		if ((surfaceDesc2.ddsCaps.dwCaps2 & (DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE)) == 0)
		{
			return DDERR_INVALIDOBJECT;
		}

		*lpdwMaxLOD = MaxLOD;

		return DD_OK;
	}

	return ProxyInterface->GetLOD(lpdwMaxLOD);
}

/************************/
/*** Helper functions ***/
/************************/

void m_IDirectDrawSurfaceX::InitSurface(DWORD DirectXVersion)
{
	WrapperInterface = new m_IDirectDrawSurface((LPDIRECTDRAWSURFACE)ProxyInterface, this);
	WrapperInterface2 = new m_IDirectDrawSurface2((LPDIRECTDRAWSURFACE2)ProxyInterface, this);
	WrapperInterface3 = new m_IDirectDrawSurface3((LPDIRECTDRAWSURFACE3)ProxyInterface, this);
	WrapperInterface4 = new m_IDirectDrawSurface4((LPDIRECTDRAWSURFACE4)ProxyInterface, this);
	WrapperInterface7 = new m_IDirectDrawSurface7((LPDIRECTDRAWSURFACE7)ProxyInterface, this);

	if (!Config.Dd7to9)
	{
		return;
	}

	AddRef(DirectXVersion);

	// Store surface, needs to run before InitSurfaceDesc()
	if (ddrawParent)
	{
		ddrawParent->AddSurfaceToVector(this);
	}

	// Set Uniqueness Value
	UniquenessValue = 1;

	// Update surface description and create backbuffers
	InitSurfaceDesc(DirectXVersion);
}

void m_IDirectDrawSurfaceX::ReleaseSurface()
{
	WrapperInterface->DeleteMe();
	WrapperInterface2->DeleteMe();
	WrapperInterface3->DeleteMe();
	WrapperInterface4->DeleteMe();
	WrapperInterface7->DeleteMe();

	if (!Config.Dd7to9 || Config.Exiting)
	{
		return;
	}

	if (attachedClipper)
	{
		attachedClipper->Release();
	}

	if (attachedPalette)
	{
		attachedPalette->Release();
	}

	if (attachedTexture)
	{
		attachedTexture->ClearSurface();
	}

	if (ddrawParent)
	{
		ddrawParent->RemoveSurfaceFromVector(this);

		// ToDo: Clear stencil surface only when using the one created by the d3d9 device
		if (IsDepthBuffer())
		{
			ddrawParent->ClearDepthStencilSurface();
		}
	}

	ReleaseD9Surface(false);
}

LPDIRECT3DSURFACE9 m_IDirectDrawSurfaceX::Get3DSurface()
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, true)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface not setup!");
		return nullptr;
	}

	return GetD3D9Surface();
}

LPDIRECT3DTEXTURE9 m_IDirectDrawSurfaceX::Get3DTexture()
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, true)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: texture not setup!");
		return nullptr;
	}

	return surfaceTexture;
}

LPDIRECT3DSURFACE9 m_IDirectDrawSurfaceX::GetD3D9Surface()
{
	if (surfaceTexture)
	{
		if (contextSurface || (!contextSurface && SUCCEEDED(surfaceTexture->GetSurfaceLevel(0, &contextSurface))))
		{
			return contextSurface;
		}
	}
	else if (surface3D)
	{
		return surface3D;
	}
	return nullptr;
}

HRESULT m_IDirectDrawSurfaceX::CheckInterface(char *FunctionName, bool CheckD3DDevice, bool CheckD3DSurface)
{
	// Check for device
	if (!ddrawParent)
	{
		LOG_LIMIT(100, FunctionName << " Error: no ddraw parent!");
		return DDERR_GENERIC;
	}

	// Check for device, if not then create it
	if (CheckD3DDevice && (!d3d9Device || !*d3d9Device))
	{
		d3d9Device = ddrawParent->GetDirect3D9Device();

		// For concurrency
		SetCriticalSection();
		bool flag = (!d3d9Device || !*d3d9Device);
		ReleaseCriticalSection();

		// Create d3d9 device
		if (flag && FAILED(ddrawParent->CreateD3D9Device()))
		{
			LOG_LIMIT(100, FunctionName << " Error: d3d9 device not setup!");
			return DDERR_GENERIC;
		}
	}

	// Check surface
	if (CheckD3DSurface)
	{
		// Check if device is lost
		if (!IsUsingEmulation() && CheckD3DDevice)
		{
			HRESULT hr = (*d3d9Device)->TestCooperativeLevel();
			if (hr == D3DERR_DEVICENOTRESET)
			{
				if (FAILED(ddrawParent->ReinitDevice()))
				{
					LOG_LIMIT(100, FunctionName << " Error: Failed to reinitialize the Driect3D device!");
					return DDERR_SURFACELOST;
				}
			}
			else if (hr == D3DERR_DEVICELOST)
			{
				LOG_LIMIT(100, FunctionName << " Error: Surface lost! = " << (D3DERR)hr);
				return DDERR_SURFACELOST;
			}
			else if (FAILED(hr))
			{
				LOG_LIMIT(100, FunctionName << " Error: TestCooperativeLevel = " << (D3DERR)hr);
				return DDERR_GENERIC;
			}
		}

		// Check if using Direct3D
		IsDirect3DSurface = ddrawParent->IsUsing3D();

		// Make sure surface exists, if not then create it
		if ((!surfaceTexture && !surface3D) || (IsDirect3DSurface && !IsTexture() && !surface3D && !(IsPrimarySurface() || IsBackBuffer())))
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
	if ((surfaceTexture || surface3D) && IsLocked)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface is locked!");
		return DDERR_GENERIC;
	}

	// Release existing surface
	ReleaseD9Surface(true);

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, false)))
	{
		return DDERR_GENERIC;
	}

	// Update surface description
	UpdateSurfaceDesc();

	// Get texture data
	surfaceFormat = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);
	surfaceBitCount = GetBitCount(surfaceFormat);
	const bool IsSurfaceEmulated = (((Config.DdrawEmulateSurface || (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_OWNDC) ||
		((IsPrimarySurface() || IsBackBuffer()) && (Config.DdrawWriteToGDI || Config.DdrawReadFromGDI || Config.DdrawRemoveScanlines))) &&
		!(surfaceFormat & 0xFF000000 /*FOURCC or D3DFMT_DXTx*/)) ||
		surfaceFormat == D3DFMT_A8B8G8R8 || surfaceFormat == D3DFMT_X8B8G8R8 || surfaceFormat == D3DFMT_B8G8R8 || surfaceFormat == D3DFMT_R8G8B8);
	DCRequiresEmulation = (surfaceFormat != D3DFMT_R5G6B5 && surfaceFormat != D3DFMT_X1R5G5B5 && surfaceFormat != D3DFMT_R8G8B8 && surfaceFormat != D3DFMT_X8R8G8B8);
	const D3DFORMAT Format = (surfaceFormat == D3DFMT_X8B8G8R8 || surfaceFormat == D3DFMT_B8G8R8 || surfaceFormat == D3DFMT_R8G8B8) ? D3DFMT_X8R8G8B8 :
		(surfaceFormat == D3DFMT_A8B8G8R8) ? D3DFMT_A8R8G8B8 : surfaceFormat;
	const D3DFORMAT TextureFormat = (surfaceFormat == D3DFMT_P8) ? D3DFMT_L8 : Format;

	// Adjust Width to be byte-aligned
	const DWORD Width = GetByteAlignedWidth(surfaceDesc2.dwWidth, surfaceBitCount);
	const DWORD Height = surfaceDesc2.dwHeight;

	Logging::LogDebug() << __FUNCTION__ " (" << this << ") D3d9 Surface. Size: " << Width << "x" << Height << " Format: " << surfaceFormat << " dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps);

	HRESULT hr = DD_OK;

	// Create surface texture
	do {
		// Create primary surface texture
		if (IsPrimarySurface() || IsBackBuffer())
		{
			// Create primary texture
			if (FAILED((*d3d9Device)->CreateTexture(Width, Height, 1, 0, TextureFormat, D3DPOOL_MANAGED, &surfaceTexture, nullptr)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create primary surface texture. Size: " << Width << "x" << Height << " Format: " << surfaceFormat << " dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps));
				hr = DDERR_GENERIC;
				break;
			}

			// Create palette surface
			if (IsPrimarySurface() && surfaceFormat == D3DFMT_P8)
			{
				if (FAILED(((*d3d9Device)->CreateTexture(256, 256, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &paletteTexture, nullptr))) ||
					FAILED((*d3d9Device)->CreatePixelShader((DWORD*)PalettePixelShaderSrc, &pixelShader)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create palette surface texture");
					hr = DDERR_GENERIC;
					break;
				}
			}
		}
		// Create depth buffer
		else if (IsDepthBuffer())
		{
			// ToDo: Get existing stencil surface rather than creating a new one
			if (FAILED(((*d3d9Device)->CreateDepthStencilSurface(Width, Height, Format, ddrawParent->GetMultiSampleType(), ddrawParent->GetMultiSampleQuality(), TRUE, &surface3D, nullptr))))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create depth buffer surface. Size: " << Width << "x" << Height << " Format: " << surfaceFormat << " dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps));
				hr = DDERR_GENERIC;
				break;
			}
		}
		// Create texture
		else if (IsTexture() || !IsDirect3DSurface)
		{
			const D3DPOOL Pool = (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) ? D3DPOOL_SYSTEMMEM : D3DPOOL_MANAGED;
			if (FAILED(((*d3d9Device)->CreateTexture(Width, Height, 1, 0, TextureFormat, Pool, &surfaceTexture, nullptr))))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create surface texture. Size: " << Width << "x" << Height << " Format: " << surfaceFormat << " dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps));
				hr = DDERR_GENERIC;
				break;
			}
		}
		// Create offplain surface
		else
		{
			const D3DPOOL Pool = (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) ? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT;
			if (FAILED((*d3d9Device)->CreateOffscreenPlainSurface(Width, Height, Format, Pool, &surface3D, nullptr)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create surface. Size: " << Width << "x" << Height << " Format: " << surfaceFormat << " dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps));
				hr = DDERR_GENERIC;
				break;
			}
		}
	} while (false);

	bool EmuSurfaceCreated = false;

	// Create emulated surface using device context for creation
	if (IsSurfaceEmulated || Config.DdrawWriteToGDI || (emu && !DoesDCMatch(emu)))
	{
		if (!DoesDCMatch(emu))
		{
			EmuSurfaceCreated = true;

			CreateDCSurface();
		}
	}

	// Restore d3d9 surface texture data
	if (surfaceTexture || surface3D)
	{
		if (DoesDCMatch(emu) && !EmuSurfaceCreated)
		{
			CopyFromEmulatedSurface(nullptr);
		}
		else if (!surfaceBackup.empty())
		{
			DWORD Bytes = surfaceBitCount / 8;
			DWORD Pitch = surfaceBackup.size() / surfaceDesc2.dwHeight;
			if (Bytes && Pitch / Bytes >= surfaceDesc2.dwWidth)
			{
				IDirect3DSurface9* pDestSurfaceD9 = GetD3D9Surface();

				if (pDestSurfaceD9)
				{
					RECT Rect = { 0, 0, (LONG)surfaceDesc2.dwWidth, (LONG)surfaceDesc2.dwHeight };

					if (SUCCEEDED(D3DXLoadSurfaceFromMemory(pDestSurfaceD9, nullptr, &Rect, &surfaceBackup[0], (surfaceFormat == D3DFMT_P8) ? D3DFMT_L8 : surfaceFormat, Pitch, nullptr, &Rect, D3DX_FILTER_NONE, 0)))
					{
						if (IsUsingEmulation())
						{
							CopyToEmulatedSurface(&Rect);
						}
					}
					else
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to restore surface data!");
					}
				}
			}
		}
	}

	// Data is no longer needed
	surfaceBackup.clear();

	// Mark palette as updated
	PaletteUSN++;

	// Only need to create vertex buffer for primary surface when using DirectDraw and not writing to GDI
	if (!IsPrimarySurface() || Config.DdrawWriteToGDI)
	{
		return hr;
	}

	// Create vertex buffer
	if (FAILED((*d3d9Device)->CreateVertexBuffer(sizeof(TLVERTEX) * 4, D3DUSAGE_WRITEONLY, TLVERTEXFVF, D3DPOOL_MANAGED, &vertexBuffer, nullptr)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create vertex buffer");
		return DDERR_GENERIC;
	}

	// Setup verticies (0, 0, currentWidth, currentHeight)
	TLVERTEX* vertices;

	// Lock vertex buffer
	if (FAILED(vertexBuffer->Lock(0, 0, (void**)&vertices, 0)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock vertex buffer");
		return DDERR_GENERIC;
	}

	// Get width and height
	DWORD displayWidth, displayHeight;
	ddrawParent->GetDisplay(displayWidth, displayHeight);
	bool displayflag = (Width < displayWidth) && (Height < displayHeight);
	DWORD BackBufferWidth = (displayflag) ? displayWidth : Width;
	DWORD BackBufferHeight = (displayflag) ? displayHeight : Height;
	if (!BackBufferWidth || !BackBufferHeight)
	{
		Utils::GetScreenSize(ddrawParent->GetHwnd(), BackBufferWidth, BackBufferHeight);
	}

	// Calculate width and height with original aspect ratio
	int xpad = 0, ypad = 0;
	float u0tex = 0.0f, u1tex = 1.0f, v0tex = 0.0f, v1tex = 1.0f;
	DWORD DisplayBufferWidth = (displayWidth > BackBufferWidth) ? displayWidth : BackBufferWidth;
	DWORD DisplayBufferHeight = (displayHeight > BackBufferHeight) ? displayHeight : BackBufferHeight;
	DWORD TexWidth = Width;
	DWORD TexHeight = Height;
	if (Config.DdrawClippedWidth && Config.DdrawClippedWidth <= TexWidth &&
		Config.DdrawClippedHeight && Config.DdrawClippedHeight <= TexHeight)
	{
		u0tex = (((TexWidth - Config.DdrawClippedWidth) / 2.0f)) / TexWidth;
		u1tex = u0tex + ((float)Config.DdrawClippedWidth / TexWidth);

		v0tex = (((TexHeight - Config.DdrawClippedHeight) / 2.0f)) / TexHeight;
		v1tex = v0tex + ((float)Config.DdrawClippedHeight / TexHeight);

		TexWidth = Config.DdrawClippedWidth;
		TexHeight = Config.DdrawClippedHeight;
	}
	if (Config.DdrawIntegerScalingClamp)
	{
		DWORD xScaleRatio = DisplayBufferWidth / TexWidth;
		DWORD yScaleRatio = DisplayBufferHeight / TexHeight;

		if (Config.DdrawMaintainAspectRatio)
		{
			xScaleRatio = min(xScaleRatio, yScaleRatio);
			yScaleRatio = min(xScaleRatio, yScaleRatio);
		}

		BackBufferWidth = xScaleRatio * TexWidth;
		BackBufferHeight = yScaleRatio * TexHeight;

		xpad = (DisplayBufferWidth - BackBufferWidth) / 2;
		ypad = (DisplayBufferHeight - BackBufferHeight) / 2;
	}
	else if (Config.DdrawMaintainAspectRatio)
	{
		if (TexWidth * DisplayBufferHeight < TexHeight * DisplayBufferWidth)
		{
			// 4:3 displayed on 16:9
			BackBufferWidth = DisplayBufferHeight * TexWidth / TexHeight;
		}
		else
		{
			// 16:9 displayed on 4:3
			BackBufferHeight = DisplayBufferWidth * TexHeight / TexWidth;
		}
		xpad = (DisplayBufferWidth - BackBufferWidth) / 2;
		ypad = (DisplayBufferHeight - BackBufferHeight) / 2;
	}

	Logging::LogDebug() << __FUNCTION__ << " D3d9 Vertex size: " << BackBufferWidth << "x" << BackBufferHeight <<
		" pad: " << xpad << "x" << ypad;

	// Set vertex points
	// 0, 0
	vertices[0].x = -0.5f + xpad;
	vertices[0].y = -0.5f + ypad;
	vertices[0].z = 0.0f;
	vertices[0].rhw = 1.0f;
	vertices[0].u = u0tex;
	vertices[0].v = v0tex;

	// scaledWidth, 0
	vertices[1].x = -0.5f + xpad + BackBufferWidth;
	vertices[1].y = vertices[0].y;
	vertices[1].z = 0.0f;
	vertices[1].rhw = 1.0f;
	vertices[1].u = u1tex;
	vertices[1].v = v0tex;

	// scaledWidth, scaledHeight
	vertices[2].x = vertices[1].x;
	vertices[2].y = -0.5f + ypad + BackBufferHeight;
	vertices[2].z = 0.0f;
	vertices[2].rhw = 1.0f;
	vertices[2].u = u1tex;
	vertices[2].v = v1tex;

	// 0, scaledHeight
	vertices[3].x = vertices[0].x;
	vertices[3].y = vertices[2].y;
	vertices[3].z = 0.0f;
	vertices[3].rhw = 1.0f;
	vertices[3].u = u0tex;
	vertices[3].v = v1tex;

	// Unlock vertex buffer
	if (FAILED(vertexBuffer->Unlock()))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to unlock vertex buffer");
		return DDERR_GENERIC;
	}

	return hr;
}

bool m_IDirectDrawSurfaceX::DoesDCMatch(EMUSURFACE* pEmuSurface)
{
	if (!pEmuSurface || !pEmuSurface->surfaceDC || !pEmuSurface->surfacepBits)
	{
		return false;
	}

	// Adjust Width to be byte-aligned
	DWORD Width = GetByteAlignedWidth(surfaceDesc2.dwWidth, surfaceBitCount);
	DWORD Height = surfaceDesc2.dwHeight;

	if (pEmuSurface->bmi->bmiHeader.biWidth == (LONG)Width && -pEmuSurface->bmi->bmiHeader.biHeight == (LONG)Height &&
		pEmuSurface->bmi->bmiHeader.biBitCount == surfaceBitCount && (
			((DWORD*)pEmuSurface->bmi->bmiColors)[0] == surfaceDesc2.ddpfPixelFormat.dwRBitMask &&
			((DWORD*)pEmuSurface->bmi->bmiColors)[1] == surfaceDesc2.ddpfPixelFormat.dwGBitMask &&
			((DWORD*)pEmuSurface->bmi->bmiColors)[2] == surfaceDesc2.ddpfPixelFormat.dwBBitMask))
	{
		return true;
	}

	return false;
}

HRESULT m_IDirectDrawSurfaceX::CreateDCSurface()
{
	// Check if color masks are needed
	bool ColorMaskReq = ((surfaceBitCount == 16 || surfaceBitCount == 24 || surfaceBitCount == 32) &&										// Only valid when used with 16 bit, 24 bit and 32 bit surfaces
		(surfaceDesc2.ddpfPixelFormat.dwFlags & DDPF_RGB) &&																				// Check to make sure it is an RGB surface
		(surfaceDesc2.ddpfPixelFormat.dwRBitMask && surfaceDesc2.ddpfPixelFormat.dwGBitMask && surfaceDesc2.ddpfPixelFormat.dwBBitMask));	// Check to make sure the masks actually exist

	// Adjust Width to be byte-aligned
	DWORD Width = GetByteAlignedWidth(surfaceDesc2.dwWidth, surfaceBitCount);
	DWORD Height = surfaceDesc2.dwHeight;

	// Check if emulated surface already exists
	if (emu)
	{
		// Check if emulated memory is good
		if (!emu->surfaceDC || !emu->surfacepBits)
		{
			DeleteEmulatedMemory(&emu);
		}
		else
		{
			// Check if current emulated surface is still ok
			if (DoesDCMatch(emu))
			{
				return DD_OK;
			}

			// Save current emulated surface and prepare for creating a new one.
			if (ShareEmulatedMemory)
			{
				EnterCriticalSection(&smcs);
				memorySurfaces.push_back(emu);
				emu = nullptr;
				LeaveCriticalSection(&smcs);
			}
			else
			{
				DeleteEmulatedMemory(&emu);
			}
		}
	}

	// If sharing memory than check the shared memory vector for a surface that matches
	if (ShareEmulatedMemory)
	{
		EnterCriticalSection(&smcs);
		for (auto it = memorySurfaces.begin(); it != memorySurfaces.end(); it++)
		{
			EMUSURFACE* pEmuSurface = *it;

			if (DoesDCMatch(pEmuSurface))
			{
				emu = pEmuSurface;

				it = memorySurfaces.erase(it);

				break;
			}
		}
		LeaveCriticalSection(&smcs);

		if (emu && emu->surfacepBits)
		{
			ZeroMemory(emu->surfacepBits, emu->surfaceSize);

			PaletteUSN++;

			return DD_OK;
		}
	}

	Logging::LogDebug() << __FUNCTION__ " (" << this << ") creating emulated surface. Size: " << Width << "x" << Height << " Format: " << surfaceFormat << " dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps);

	// Create new emulated surface structure
	emu = new EMUSURFACE;

	// Create device context memory
	ZeroMemory(emu->bmiMemory, sizeof(emu->bmiMemory));
	emu->bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	emu->bmi->bmiHeader.biWidth = Width;
	emu->bmi->bmiHeader.biHeight = -((LONG)Height + 200);
	emu->bmi->bmiHeader.biPlanes = 1;
	emu->bmi->bmiHeader.biBitCount = (WORD)surfaceBitCount;
	emu->bmi->bmiHeader.biCompression =
		(surfaceBitCount == 8 || surfaceBitCount == 24) ? BI_RGB :
		(ColorMaskReq) ? BI_BITFIELDS : 0;	// BI_BITFIELDS is only valid for 16-bpp and 32-bpp bitmaps.
	emu->bmi->bmiHeader.biSizeImage = ((Width * surfaceBitCount + 31) & ~31) / 8 * Height;

	if (surfaceBitCount == 8)
	{
		for (int i = 0; i < 256; i++)
		{
			emu->bmi->bmiColors[i].rgbRed = (byte)i;
			emu->bmi->bmiColors[i].rgbGreen = (byte)i;
			emu->bmi->bmiColors[i].rgbBlue = (byte)i;
			emu->bmi->bmiColors[i].rgbReserved = 0;
		}
	}
	else if (ColorMaskReq)
	{
		((DWORD*)emu->bmi->bmiColors)[0] = surfaceDesc2.ddpfPixelFormat.dwRBitMask;
		((DWORD*)emu->bmi->bmiColors)[1] = surfaceDesc2.ddpfPixelFormat.dwGBitMask;
		((DWORD*)emu->bmi->bmiColors)[2] = surfaceDesc2.ddpfPixelFormat.dwBBitMask;
		((DWORD*)emu->bmi->bmiColors)[3] = surfaceDesc2.ddpfPixelFormat.dwRGBAlphaBitMask;
	}
	else
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set bmi colors! " << surfaceBitCount);
		DeleteEmulatedMemory(&emu);
		return DDERR_GENERIC;
	}

	emu->surfaceDC = CreateCompatibleDC(ddrawParent->GetDC());
	if (!emu->surfaceDC)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create compatible DC!");
		DeleteEmulatedMemory(&emu);
		return DDERR_GENERIC;
	}
	emu->bitmap = CreateDIBSection(emu->surfaceDC, emu->bmi, (surfaceBitCount == 8) ? DIB_PAL_COLORS : DIB_RGB_COLORS, (void**)&emu->surfacepBits, nullptr, 0);
	if (!emu->bitmap)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create bitmap!");
		DeleteEmulatedMemory(&emu);
		return DDERR_GENERIC;
	}
	emu->OldDCObject = SelectObject(emu->surfaceDC, emu->bitmap);
	if (!emu->OldDCObject)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to replace object in DC!");
		DeleteEmulatedMemory(&emu);
		return DDERR_GENERIC;
	}
	emu->bmi->bmiHeader.biHeight = -(LONG)Height;
	emu->surfacePitch = ComputePitch(emu->bmi->bmiHeader.biWidth, emu->bmi->bmiHeader.biBitCount);
	emu->surfaceSize = Height * emu->surfacePitch;

	PaletteUSN++;

	return DD_OK;
}

// Update surface description
void m_IDirectDrawSurfaceX::UpdateSurfaceDesc()
{
	if ((surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT)) != (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT))
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, false)))
		{
			return;
		}

		// Get resolution
		DWORD Width, Height, RefreshRate, BPP;
		ddrawParent->GetFullDisplay(Width, Height, BPP, RefreshRate);

		// Set Height and Width
		if (Width && Height &&
			(surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT)) != (DDSD_WIDTH | DDSD_HEIGHT))
		{
			ResetDisplayFlags |= DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH;
			surfaceDesc2.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT;
			surfaceDesc2.dwWidth = Width;
			surfaceDesc2.dwHeight = Height;
			surfaceDesc2.lPitch = 0;
		}
		// Set Refresh Rate
		if (RefreshRate && !(surfaceDesc2.dwFlags & DDSD_REFRESHRATE))
		{
			ResetDisplayFlags |= DDSD_REFRESHRATE;
			surfaceDesc2.dwFlags |= DDSD_REFRESHRATE;
			surfaceDesc2.dwRefreshRate = RefreshRate;
		}
		// Set PixelFormat
		if (BPP && !(surfaceDesc2.dwFlags & DDSD_PIXELFORMAT))
		{
			ResetDisplayFlags |= DDSD_PIXELFORMAT | DDSD_PITCH;
			surfaceDesc2.dwFlags |= DDSD_PIXELFORMAT;
			ddrawParent->GetDisplayPixelFormat(surfaceDesc2.ddpfPixelFormat, BPP);
		}
	}
	// Unset lPitch
	if ((surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT)) != (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT) || !surfaceDesc2.lPitch)
	{
		surfaceDesc2.dwFlags &= ~DDSD_PITCH;
		surfaceDesc2.lPitch = 0;
	}
}

// Release surface and vertext buffer
void m_IDirectDrawSurfaceX::ReleaseD9Surface(bool BackupData)
{
	if (IsLocked || IsInDC)
	{
		Logging::Log() << __FUNCTION__ << " Warning: surface still in use! Locked: " << IsLocked << " DC: " << IsInDC;
	}

	// Release DC (before releasing surface)
	if (IsInDC || LastDC)
	{
		if (LastDC)
		{
			ReleaseDC(LastDC);
			LastDC = nullptr;
		}
		IsInDC = false;
	}

	// Unlock surface (before releasing)
	if (IsLocked)
	{
		UnlockD39Surface();
		IsLocked = false;
		LockedWithID = 0;
	}

	// Backup d3d9 surface texture
	if (BackupData)
	{
		if (!IsUsingEmulation() && (surfaceTexture || surface3D))
		{
			D3DLOCKED_RECT LockRect = {};
			if (SUCCEEDED(LockD39Surface(&LockRect, nullptr, 0)))
			{
				Logging::LogDebug() << __FUNCTION__ << " Storing Direct3D9 texture surface data";

				size_t size = surfaceDesc2.dwHeight * LockRect.Pitch;

				surfaceBackup.resize(size);

				memcpy(&surfaceBackup[0], LockRect.pBits, size);

				UnlockD39Surface();
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to backup surface data!");
			}
		}
	}
	// Emulated surface
	else if (emu)
	{
		if (!ShareEmulatedMemory || !emu->surfaceDC || !emu->surfacepBits)
		{
			DeleteEmulatedMemory(&emu);
		}
		else
		{
			EnterCriticalSection(&smcs);
			memorySurfaces.push_back(emu);
			emu = nullptr;
			LeaveCriticalSection(&smcs);
		}
	}

	// Release d3d9 3D surface
	if (surface3D)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 surface";
		ULONG ref = surface3D->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'surface3D' " << ref;
		}
		surface3D = nullptr;
	}

	// Release d3d9 context surface
	if (contextSurface)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 context surface";
		ULONG ref = contextSurface->Release();
		if (ref > 1)	// Ref count is higher becasue it is a surface of 'surfaceTexture'
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'contextSurface' " << ref;
		}
		contextSurface = nullptr;
	}

	// Release d3d9 surface texture
	if (surfaceTexture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 texture surface";
		ULONG ref = surfaceTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'surfaceTexture' " << ref;
		}
		surfaceTexture = nullptr;
	}

	// Release d3d9 palette surface texture
	if (paletteTexture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 palette texture surface";
		if (d3d9Device && *d3d9Device)
		{
			(*d3d9Device)->SetTexture(1, nullptr);
		}
		ULONG ref = paletteTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'paletteTexture' " << ref;
		}
		paletteTexture = nullptr;
	}

	// Release d3d9 pixel shader
	if (pixelShader)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 pixel shader";
		if (d3d9Device && *d3d9Device)
		{
			(*d3d9Device)->SetPixelShader(nullptr);
		}
		ULONG ref = pixelShader->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'pixelShader' " << ref;
		}
		pixelShader = nullptr;
	}

	// Release d3d9 vertex buffer
	if (vertexBuffer)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 vertext buffer";
		ULONG ref = vertexBuffer->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'vertexBuffer' " << ref;
		}
		vertexBuffer = nullptr;
	}

	// Clear locked rects
	surfaceLockRectList.clear();

	// Reset scanline flags
	LastLock.LockedRect.pBits = nullptr;
	LastLock.bEvenScanlines = false;
	LastLock.bOddScanlines = false;

	// Reset display flags
	if (ResetDisplayFlags)
	{
		surfaceDesc2.dwFlags &= ~ResetDisplayFlags;
	}
}

HRESULT m_IDirectDrawSurfaceX::Draw2DSurface()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") ";

	// If there is no surface texture than nothing to draw
	if (!surfaceTexture)
	{
		return DD_OK;
	}

	// Set texture
	if (surfaceTexture)
	{
		if (FAILED((*d3d9Device)->SetTexture(0, surfaceTexture)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set surface texture");
			return DDERR_GENERIC;
		}
	}

	// Set vertex buffer and lighting
	if (vertexBuffer)
	{
		// Set vertex shader
		if (FAILED((*d3d9Device)->SetVertexShader(nullptr)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set vertex shader");
			return DDERR_GENERIC;
		}

		// Set vertex format
		if (FAILED((*d3d9Device)->SetFVF(TLVERTEXFVF)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set the current vertex stream format");
			return DDERR_GENERIC;
		}

		// Set stream source
		if (FAILED((*d3d9Device)->SetStreamSource(0, vertexBuffer, 0, sizeof(TLVERTEX))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set vertex buffer stream source");
			return DDERR_GENERIC;
		}

		// Set render states (no lighting)
		if (FAILED((*d3d9Device)->SetRenderState(D3DRS_LIGHTING, FALSE)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set device render state(no lighting)");
			return DDERR_GENERIC;
		}

		// Set scale mode to linear
		if (FAILED((*d3d9Device)->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: failed to set D3D device to LINEAR sampling");
		}
	}

	// Handle palette surfaces
	if (paletteTexture && pixelShader)
	{
		// Set palette texture
		UpdatePaletteData();
		if (FAILED((*d3d9Device)->SetTexture(1, paletteTexture)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock palette texture!");
			return DDERR_GENERIC;
		}

		// Set pixel shader
		if (FAILED((*d3d9Device)->SetPixelShader(pixelShader)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set pixel shader");
			return DDERR_GENERIC;
		}
	}

	// Draw primitive
	if (FAILED((*d3d9Device)->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to draw primitive");
		return DDERR_GENERIC;
	}

	return DD_OK;
}

// Present surface
HRESULT m_IDirectDrawSurfaceX::PresentSurface(bool isSkipScene)
{
	// Check for device interface
	HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
	if (FAILED(c_hr))
	{
		return c_hr;
	}

	// Check if is not primary surface or if scene should be skipped
	if (Config.DdrawWriteToGDI || IsDirect3DSurface)
	{
		// Never present when using Direct3D or when writing to GDI
		return DD_OK;
	}
	else if (!IsPrimarySurface())
	{
		if (SceneReady && !IsPresentRunning)
		{
			m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = ddrawParent->GetPrimarySurface();
			if (lpDDSrcSurfaceX)
			{
				return lpDDSrcSurfaceX->PresentSurface(isSkipScene);
			}
		}
		return DDERR_GENERIC;
	}
	else if ((isSkipScene && !SceneReady) || IsPresentRunning)
	{
		Logging::LogDebug() << __FUNCTION__ << " Skipping scene!";
		return DDERR_GENERIC;
	}

	// Set scene ready
	SceneReady = true;

	// Check if surface is locked or has an open DC
	if (IsSurfaceLocked() || IsSurfaceInDC())
	{
		Logging::LogDebug() << __FUNCTION__ << " Surface is busy!";
		return DDERR_SURFACEBUSY;
	}

	// Set present flag
	IsPresentRunning = true;

	HRESULT hr = DD_OK;

	// Preset surface
	do {
		// Begin scene
		if (FAILED((*d3d9Device)->BeginScene()))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to begin scene!");
			hr = DDERR_GENERIC;
			break;
		}

		// Draw 2D surface before presenting
		if (FAILED(Draw2DSurface()))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to draw 2D surface!");
			hr = DDERR_GENERIC;
			break;
		}

		// End scene
		if (FAILED((*d3d9Device)->EndScene()))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to end scene!");
			hr = DDERR_GENERIC;
			break;
		}

		// Present to d3d9
		if (FAILED(ddrawParent->Present()))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to present 2D scene!");
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

// Reset primary surface display settings
void m_IDirectDrawSurfaceX::ResetSurfaceDisplay()
{
	if (ResetDisplayFlags)
	{
		ReleaseD9Surface(true);
	}
}

// Swap surface addresses for Flip
void m_IDirectDrawSurfaceX::SwapSurface(m_IDirectDrawSurfaceX *lpTargetSurface1, m_IDirectDrawSurfaceX *lpTargetSurface2)
{
	if (!lpTargetSurface1 || !lpTargetSurface2 || lpTargetSurface1 == lpTargetSurface2)
	{
		return;
	}

	// Swap dirty flip flag
	SwapAddresses(&lpTargetSurface1->DirtyFlip, &lpTargetSurface2->DirtyFlip);

	// Swap 3D surface
	SwapAddresses(lpTargetSurface1->GetSurface3D(), lpTargetSurface2->GetSurface3D());

	// Swap surface textures
	SwapAddresses(lpTargetSurface1->GetSurfaceTexture(), lpTargetSurface2->GetSurfaceTexture());

	// Swap context surfaces
	SwapAddresses(lpTargetSurface1->GetContextSurface(), lpTargetSurface2->GetContextSurface());

	// Swap emulated surfaces
	SwapAddresses(lpTargetSurface1->GetEmulatedSurface(), lpTargetSurface2->GetEmulatedSurface());
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
		lpOutRect->left = lpInRect->left;
		lpOutRect->top = lpInRect->top;
		lpOutRect->right = lpInRect->right;
		lpOutRect->bottom = lpInRect->bottom;
	}
	else
	{
		lpOutRect->left = 0;
		lpOutRect->top = 0;
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

// Fix issue with some games that ignore the pitch size
template <class T>
void m_IDirectDrawSurfaceX::LockBitAlign(LPRECT lpDestRect, T lpDDSurfaceDesc)
{
	if (!lpDDSurfaceDesc || lpDestRect)
	{
		return;
	}

	DWORD Width = (DsWrapper.Width) ? DsWrapper.Width : lpDDSurfaceDesc->dwWidth;
	DWORD Height = (DsWrapper.Height) ? DsWrapper.Height : lpDDSurfaceDesc->dwHeight;

	DWORD BBP = GetBitCount(lpDDSurfaceDesc->ddpfPixelFormat);
	LONG NewPitch = (BBP / 8) * Width;

	// Check if surface needs to be fixed
	if (Width && Height &&
		(BBP == 8 || BBP == 16 || BBP == 24 || BBP == 32) &&
		lpDDSurfaceDesc->lPitch != NewPitch)
	{
		// Store old variables
		EmuLock.Locked = true;
		EmuLock.Addr = lpDDSurfaceDesc->lpSurface;
		EmuLock.Pitch = lpDDSurfaceDesc->lPitch;
		EmuLock.BBP = BBP;
		EmuLock.Height = Height;
		EmuLock.Width = Width;

		// Update surface memory and pitch
		DWORD Size = NewPitch * Height;
		if (EmuLock.surfaceMem.size() < Size)
		{
			EmuLock.surfaceMem.resize(Size);
		}
		lpDDSurfaceDesc->lpSurface = &EmuLock.surfaceMem[0];
		lpDDSurfaceDesc->lPitch = NewPitch;
	}
}

// Restore removed scanlines before locking surface
void m_IDirectDrawSurfaceX::RestoreScanlines(LASTLOCK& LLock)
{
	DWORD ByteCount = surfaceBitCount / 8;
	DWORD RectWidth = LLock.Rect.right - LLock.Rect.left;
	DWORD RectHeight = LLock.Rect.bottom - LLock.Rect.top;

	if ((!IsPrimarySurface() && !IsBackBuffer()) || !LLock.LockedRect.pBits ||
		!ByteCount || ByteCount > 4 || RectWidth != LLock.ScanlineWidth)
	{
		return;
	}

	DWORD size = RectWidth * ByteCount;
	BYTE* DestBuffer = (BYTE*)LLock.LockedRect.pBits;

	// Restore even scanlines
	if (LLock.bEvenScanlines)
	{
		constexpr DWORD Starting = 0;
		DestBuffer += LLock.LockedRect.Pitch * Starting;

		for (DWORD y = Starting; y < RectHeight; y = y + 2)
		{
			memcpy(DestBuffer, &LLock.EvenScanLine[0], size);
			DestBuffer += LLock.LockedRect.Pitch * 2;
		}
	}
	// Restore odd scanlines
	else if (LLock.bOddScanlines)
	{
		constexpr DWORD Starting = 1;
		DestBuffer += LLock.LockedRect.Pitch * Starting;

		for (DWORD y = Starting; y < RectHeight; y = y + 2)
		{
			memcpy(DestBuffer, &LLock.OddScanLine[0], size);
			DestBuffer += LLock.LockedRect.Pitch * 2;
		}
	}
}

// Remove scanlines before unlocking surface
void m_IDirectDrawSurfaceX::RemoveScanlines(LASTLOCK& LLock)
{
	DWORD ByteCount = surfaceBitCount / 8;
	DWORD RectWidth = LLock.Rect.right - LLock.Rect.left;
	DWORD RectHeight = LLock.Rect.bottom - LLock.Rect.top;

	// Reset scanline flags
	bool LastSet = (LLock.bEvenScanlines || LLock.bOddScanlines);
	LLock.bOddScanlines = false;
	LLock.bEvenScanlines = false;

	if ((!IsPrimarySurface() && !IsBackBuffer()) || !LLock.LockedRect.pBits ||
		!ByteCount || ByteCount > 4 || RectHeight < 100)
	{
		return;
	}

	DWORD size = LLock.ScanlineWidth * ByteCount;
	if (LLock.EvenScanLine.size() < size || LLock.OddScanLine.size() < size)
	{
		LLock.EvenScanLine.resize(size);
		LLock.OddScanLine.resize(size);
	}
	LLock.ScanlineWidth = RectWidth;

	BYTE* DestBuffer = (BYTE*)LLock.LockedRect.pBits;

	// Check if video has scanlines
	for (DWORD y = 0; y < RectHeight; y++)
	{
		// Check for even scanlines
		if (y % 2 == 0)
		{
			if (y == 0)
			{
				LLock.bEvenScanlines = true;
				memcpy(&LLock.EvenScanLine[0], DestBuffer, size);
			}
			else if (LLock.bEvenScanlines)
			{
				LLock.bEvenScanlines = (memcmp(&LLock.EvenScanLine[0], DestBuffer, size) == 0);
			}
		}
		// Check for odd scanlines
		else
		{
			if (y == 1)
			{
				LLock.bOddScanlines = true;
				memcpy(&LLock.OddScanLine[0], DestBuffer, size);
			}
			else if (LLock.bOddScanlines)
			{
				LLock.bOddScanlines = (memcmp(&LLock.OddScanLine[0], DestBuffer, size) == 0);
			}
		}
		// Exit if no scanlines found
		if (!LLock.bOddScanlines && !LLock.bEvenScanlines)
		{
			break;
		}
		DestBuffer += LLock.LockedRect.Pitch;
	}

	// If all scanlines are set then do nothing
	if (!LastSet && LLock.bEvenScanlines && LLock.bOddScanlines)
	{
		LLock.bEvenScanlines = false;
		LLock.bOddScanlines = false;
	}

	// Reset destination buffer
	DestBuffer = (BYTE*)LLock.LockedRect.pBits;

	// Double even scanlines
	if (LLock.bEvenScanlines)
	{
		constexpr DWORD Starting = 0;
		DestBuffer += LLock.LockedRect.Pitch * Starting;

		for (DWORD y = Starting; y < RectHeight - 1; y = y + 2)
		{
			memcpy(DestBuffer, DestBuffer + LLock.LockedRect.Pitch, size);
			DestBuffer += LLock.LockedRect.Pitch * 2;
		}
	}
	// Double odd scanlines
	else if (LLock.bOddScanlines)
	{
		constexpr DWORD Starting = 1;
		DestBuffer += LLock.LockedRect.Pitch * Starting;

		for (DWORD y = Starting; y < RectHeight; y = y + 2)
		{
			memcpy(DestBuffer, DestBuffer - LLock.LockedRect.Pitch, size);
			DestBuffer += LLock.LockedRect.Pitch * 2;
		}
	}
}

HRESULT m_IDirectDrawSurfaceX::LockEmulatedSurface(D3DLOCKED_RECT* pLockedRect, LPRECT lpDestRect)
{
	if (!pLockedRect)
	{
		return DDERR_GENERIC;
	}
	if (!IsUsingEmulation())
	{
		pLockedRect->Pitch = 0;
		pLockedRect->pBits = nullptr;
		return DDERR_GENERIC;
	}

	pLockedRect->Pitch = emu->surfacePitch;
	pLockedRect->pBits = (lpDestRect) ? (void*)((DWORD)emu->surfacepBits + ((lpDestRect->top * pLockedRect->Pitch) + (lpDestRect->left * (surfaceBitCount / 8)))) : emu->surfacepBits;

	return DD_OK;
}

// Set dirty flag
inline void m_IDirectDrawSurfaceX::SetDirtyFlag()
{
	if (IsPrimarySurface())
	{
		dirtyFlag = true;
	}

	// Update Uniqueness Value
	ChangeUniquenessValue();
}

// Set dirty flip flag
void m_IDirectDrawSurfaceX::SetDirtyFlipFlag()
{
	if (!DirtyFlip)
	{
		DirtyFlip = true;
		for (auto& it : AttachedSurfaceMap)
		{
			it.second.pSurface->SetDirtyFlipFlag();
		}
	}
}

// Check if rect is a single line and should be skipped
inline bool m_IDirectDrawSurfaceX::CheckRectforSkipScene(RECT& DestRect)
{
	bool isSingleLine = (DestRect.bottom - DestRect.top == 1);	// Only handles horizontal lines at this point

	return Config.DdrawRemoveInterlacing ? isSingleLine : false;
}

inline void m_IDirectDrawSurfaceX::BeginWritePresent(bool isSkipScene)
{
	// Check if data needs to be presented before write
	if (dirtyFlag)
	{
		if (FAILED(PresentSurface(isSkipScene)))
		{
			PresentOnUnlock = true;
		}
	}
}

inline void m_IDirectDrawSurfaceX::EndWritePresent(bool isSkipScene)
{
	// Present surface after each draw unless removing interlacing
	if (PresentOnUnlock || !Config.DdrawRemoveInterlacing)
	{
		PresentSurface(isSkipScene);
	}

	// Reset endscene lock
	PresentOnUnlock = false;
}

// Update surface description and create backbuffers
void m_IDirectDrawSurfaceX::InitSurfaceDesc(DWORD DirectXVersion)
{
	// Update dds caps flags
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
		ConvertSurfaceDesc(Desc2, surfaceDesc2);
		Desc2.ddsCaps.dwCaps4 &= ~(DDSCAPS4_CREATESURFACE);	// Clear surface creation flag
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
			if (surfaceDesc2.ddsCaps.dwCaps4 & DDSCAPS4_CREATESURFACE)
			{
				ComplexRoot = true;
			}

			BackBufferInterface = std::make_unique<m_IDirectDrawSurfaceX>(ddrawParent, DirectXVersion, &Desc2);

			m_IDirectDrawSurfaceX *attachedSurface = BackBufferInterface.get();

			AddAttachedSurfaceToMap(attachedSurface);

			attachedSurface->AddRef(DirectXVersion);
		}
		else
		{
			m_IDirectDrawSurfaceX *attachedSurface = new m_IDirectDrawSurfaceX(ddrawParent, DirectXVersion, &Desc2);

			AddAttachedSurfaceToMap(attachedSurface);
		}
	}

	// Add first surface as attached surface to the last surface in a surface chain
	else if (surfaceDesc2.dwReserved)
	{
		m_IDirectDrawSurfaceX *attachedSurface = (m_IDirectDrawSurfaceX *)surfaceDesc2.dwReserved;

		// Check if source Surface exists and add to surface map
		if (ddrawParent && ddrawParent->DoesSurfaceExist(attachedSurface))
		{
			AddAttachedSurfaceToMap(attachedSurface);
		}
	}

	// Clear comlpex flag if not part of a complex structure
	else
	{
		surfaceDesc2.dwFlags &= ~DDSCAPS_COMPLEX;
	}

	// Clear flags used in creating a surface structure
	if (!(surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER))
	{
		surfaceDesc2.dwFlags &= ~DDSD_BACKBUFFERCOUNT;
		surfaceDesc2.dwBackBufferCount = 0;
	}
	surfaceDesc2.ddsCaps.dwCaps4 = 0x00;
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
	for (auto& it : AttachedSurfaceMap)
	{
		if (it.second.pSurface && (it.second.pSurface->GetSurfaceCaps().dwCaps & DDSCAPS_FLIP))
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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Check for device interface
	HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
	if (FAILED(c_hr) && !IsUsingEmulation())
	{
		return c_hr;
	}

	// Check and copy rect
	RECT DestRect = {};
	if (!CheckCoordinates(&DestRect, pRect))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid rect!");
		return DDERR_INVALIDRECT;
	}

	// Use D3DXLoadSurfaceFromMemory to color fill the surface
	if (!IsUsingEmulation())
	{
		IDirect3DSurface9* pDestSurfaceD9 = GetD3D9Surface();

		if (!pDestSurfaceD9)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get texture surface!");
			return DDERR_GENERIC;
		}

		D3DSURFACE_DESC Desc = {};
		pDestSurfaceD9->GetDesc(&Desc);

		DWORD ByteCount = GetBitCount(Desc.Format) / 8;

		if (!ByteCount || ByteCount > 4)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid byte count: " << ByteCount);
			return DDERR_GENERIC;
		}

		D3DCOLOR ColorSurface[9] = {};	// Nine DOUBLE WORDs for a byte aligned 3x3 surface

		BYTE* Buffer = (BYTE*)ColorSurface;
		BYTE* SrcBuffer = (BYTE*)&dwFillColor;

		// Fill 3x3 surface with correct color
		DWORD Count = 36 / ByteCount;
		for (UINT x = 0; x < Count; x++)
		{
			for (UINT c = 0; c < ByteCount; c++)
			{
				*Buffer = SrcBuffer[c];
				Buffer++;
			}
		}

		RECT SrcRect = { 0, 0, 12 / (LONG)ByteCount, 3 };

		if (FAILED(D3DXLoadSurfaceFromMemory(pDestSurfaceD9, nullptr, &DestRect, ColorSurface, Desc.Format, 12, nullptr, &SrcRect, D3DX_FILTER_POINT, 0)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not color fill surface!");
			return DDERR_GENERIC;
		}
	}
	else
	{
		// Check if surface is not locked then lock it
		D3DLOCKED_RECT DestLockRect = {};
		if (FAILED(LockEmulatedSurface(&DestLockRect, &DestRect)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get emulated surface lock!");
			return DDERR_GENERIC;
		}

		// Get byte count
		DWORD ByteCount = surfaceBitCount / 8;
		if (!ByteCount || ByteCount > 4)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find correct fill color for ByteCount " << ByteCount);
			return DDERR_GENERIC;
		}

		// Get width and height of rect
		LONG FillWidth = DestRect.right - DestRect.left;
		LONG FillHeight = DestRect.bottom - DestRect.top;

		if ((LONG)ComputePitch(FillWidth, ByteCount) == DestLockRect.Pitch && ByteCount != 3)
		{
			DWORD Color =
				(ByteCount == 1) ? ((dwFillColor & 0xFF) << 24) + ((dwFillColor & 0xFF) << 16) +
				((dwFillColor & 0xFF) << 8) + (dwFillColor & 0xFF) :
				(ByteCount == 2) ? ((dwFillColor & 0xFFFF) << 16) + (dwFillColor & 0xFFFF) :
				(ByteCount == 4) ? dwFillColor : 0;

			memset(DestLockRect.pBits, Color, DestLockRect.Pitch * FillHeight);
		}
		else
		{
			// Set memory address
			BYTE* SrcBuffer = (BYTE*)&dwFillColor;
			BYTE* DestBuffer = (BYTE*)DestLockRect.pBits;

			// Fill first line memory
			for (LONG x = 0; x < FillWidth; x++)
			{
				for (DWORD y = 0; y < ByteCount; y++)
				{
					*DestBuffer = SrcBuffer[y];
					DestBuffer++;
				}
			}

			// Fill rest of surface rect using the first line as a template
			SrcBuffer = (BYTE*)DestLockRect.pBits;
			DestBuffer = (BYTE*)DestLockRect.pBits + DestLockRect.Pitch;
			size_t size = FillWidth * ByteCount;
			for (LONG y = 1; y < FillHeight; y++)
			{
				memcpy(DestBuffer, SrcBuffer, size);
				DestBuffer += DestLockRect.Pitch;
			}
		}

		// Copy emulated surface to real texture
		CopyFromEmulatedSurface(&DestRect);

		// Blt surface directly to GDI
		if (Config.DdrawWriteToGDI && (IsPrimarySurface() || IsBackBuffer()))
		{
			CopyEmulatedSurfaceToGDI(DestRect);
		}
	}

	return DD_OK;
}

// Save DXT data as a DDS file
HRESULT m_IDirectDrawSurfaceX::SaveDXTDataToDDS(const void *data, size_t dataSize, const char *filename, int dxtVersion) const
{
	int blockSize = 0;
	DWORD fourCC = 0;

	switch(dxtVersion)
	{
	case 1:
		blockSize = 8;
		fourCC = '1TXD';
		break;

	case 3:
		blockSize = 16;
		fourCC = '3TXD';
		break;

	case 5:
		blockSize = 16;
		fourCC = '5TXD';
		break;

	default:
		Logging::Log() << __FUNCTION__ << " Error: unsupported DXT version!";
		return D3DERR_INVALIDCALL;
	}

	std::ofstream outFile(filename, std::ios::binary | std::ios::out);
	if (outFile.is_open())
	{
		DDS_HEADER header = {};
		header.dwSize = sizeof(DDS_HEADER);
		header.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE;
		header.dwHeight = surfaceDesc2.dwHeight;
		header.dwWidth = surfaceDesc2.dwHeight;
		header.dwPitchOrLinearSize = max(1, (surfaceDesc2.dwWidth + 3) / 4) * blockSize;  // 8 for DXT1, 16 for others
		header.dwDepth = 0;
		header.dwMipMapCount = 0;
		header.ddspf.dwSize = sizeof(DDS_PIXELFORMAT);
		header.ddspf.dwFlags = DDPF_FOURCC;
		header.ddspf.dwFourCC = fourCC;
		header.dwCaps = DDSCAPS_TEXTURE;// | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
		header.dwCaps2 = 0x00000000;
		header.dwCaps3 = 0x00000000;
		header.dwCaps4 = 0x00000000;
		header.dwReserved2 = 0;

		outFile.write("DDS ", 4);
		outFile.write((char*)&header, sizeof(DDS_HEADER));
		outFile.write((char*)data, dataSize);
		outFile.close();

		return D3D_OK;
	}

	return DDERR_GENERIC;
}

// Save a surface for debugging
HRESULT m_IDirectDrawSurfaceX::SaveSurfaceToFile(const char *filename, D3DXIMAGE_FILEFORMAT format)
{
	LPD3DXBUFFER pDestBuf = nullptr;
	HRESULT hr = D3DXSaveSurfaceToFileInMemory(&pDestBuf, format, GetD3D9Surface(), nullptr, nullptr);

	if (SUCCEEDED(hr))
	{
		// Save the buffer to a file
		std::ofstream outFile(filename, std::ios::binary | std::ios::out);
		if (outFile.is_open())
		{
			outFile.write((const char*)pDestBuf->GetBufferPointer(), pDestBuf->GetBufferSize());
			outFile.close();
		}

		// Release the buffer
		pDestBuf->Release();
	}

	return hr;
}

// Copy surface
HRESULT m_IDirectDrawSurfaceX::CopySurface(m_IDirectDrawSurfaceX* pSourceSurface, RECT* pSourceRect, RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter, DDCOLORKEY ColorKey, DWORD dwFlags)
{
	UNREFERENCED_PARAMETER(Filter);

	// Check parameters
	if (!pSourceSurface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid parameters!");
		return DDERR_INVALIDPARAMS;
	}

	// Check for device interface
	HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
	if ((FAILED(c_hr) && !IsUsingEmulation()) || (FAILED(pSourceSurface->CheckInterface(__FUNCTION__, true, true)) && !pSourceSurface->IsUsingEmulation()))
	{
		return FAILED(c_hr) ? c_hr : DDERR_GENERIC;
	}

	// Check and copy rect and do clipping
	RECT SrcRect = {}, DestRect = {};
	if (!pSourceSurface->CheckCoordinates(&SrcRect, pSourceRect) || !CheckCoordinates(&DestRect, pDestRect))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid rect!");
		return DDERR_INVALIDRECT;
	}

	// Get source and dest format
	const D3DFORMAT SrcFormat = pSourceSurface->GetSurfaceFormat();
	const D3DFORMAT DestFormat = GetSurfaceFormat();

	// Get copy flags
	const bool IsStretchRect =
		abs((pSourceRect ? pSourceRect->right - pSourceRect->left : SrcRect.right - SrcRect.left) -		// SrcWidth
			(pDestRect ? pDestRect->right - pDestRect->left : DestRect.right - DestRect.left)) > 1 ||	// DestWidth
		abs((pSourceRect ? pSourceRect->bottom - pSourceRect->top : SrcRect.bottom - SrcRect.top) -		// SrcHeight
			(pDestRect ? pDestRect->bottom - pDestRect->top : DestRect.bottom - DestRect.top)) > 1;		// DestHeight
	const bool IsColorKey = ((dwFlags & BLT_COLORKEY) != 0);
	const bool IsMirrorLeftRight = ((dwFlags & BLT_MIRRORLEFTRIGHT) != 0);
	const bool IsMirrorUpDown = ((dwFlags & BLT_MIRRORUPDOWN) != 0);
	const DWORD D3DXFilter =
		(IsStretchRect && DestFormat == D3DFMT_P8) || (Filter & D3DTEXF_POINT) ? D3DX_FILTER_POINT :	// Force palette surfaces to use point filtering to prevent color banding
		(Filter & D3DTEXF_LINEAR) ? D3DX_FILTER_LINEAR :												// Use linear filtering when requested by the application
		(IsStretchRect) ? D3DX_FILTER_POINT :															// Default to point filtering when stretching the rect, same as DirectDraw
		D3DX_FILTER_NONE;

	// Get width and height of rect
	LONG SrcRectWidth = SrcRect.right - SrcRect.left;
	LONG SrcRectHeight = SrcRect.bottom - SrcRect.top;
	LONG DestRectWidth = DestRect.right - DestRect.left;
	LONG DestRectHeight = DestRect.bottom - DestRect.top;

	if (!IsStretchRect)
	{
		SrcRectWidth = min(SrcRectWidth, DestRectWidth);
		SrcRectHeight = min(SrcRectHeight, DestRectHeight);
		DestRectWidth = SrcRectWidth;
		DestRectHeight = SrcRectHeight;
		SrcRect.right = SrcRect.left + SrcRectWidth;
		SrcRect.bottom = SrcRect.top + SrcRectHeight;
		DestRect.right = DestRect.left + DestRectWidth;
		DestRect.bottom = DestRect.top + DestRectHeight;
	}

	// Read surface from GDI
	if (IsColorKey && Config.DdrawReadFromGDI && (IsPrimarySurface() || IsBackBuffer()))
	{
		CopyEmulatedSurfaceFromGDI(DestRect);
	}

	// Variables
	HRESULT hr = DD_OK;
	bool UnlockSrc = false, UnlockDest = false;
	D3DLOCKED_RECT DestLockRect = {};

	do {
		// Decode DirectX texture
		if (ISDXTEX(SrcFormat))
		{
			if (IsColorKey)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: color key not supported with DirectX textures!");
			}

			if (IsMirrorLeftRight || IsMirrorUpDown)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: mirroring not supported with DirectX textures!");
			}

			if (IsUsingEmulation())
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: copying DirectX textures to emulated surfaces is not supported!");
				hr = DDERR_GENERIC;
				break;
			}

			IDirect3DSurface9* pSourceSurfaceD9 = pSourceSurface->GetD3D9Surface();
			IDirect3DSurface9* pDestSurfaceD9 = GetD3D9Surface();

			if (!pSourceSurfaceD9 || !pDestSurfaceD9)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get texture surface. " << pSourceSurfaceD9 << "->" << pDestSurfaceD9);
				hr = DDERR_GENERIC;
				break;
			}

			hr = D3DXLoadSurfaceFromSurface(pDestSurfaceD9, nullptr, &DestRect, pSourceSurfaceD9, nullptr, &SrcRect, D3DXFilter, 0);

			if (FAILED(hr))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not decode source texture. " << (D3DERR)hr);
			}

			break;
		}

		// Use BitBlt/StretchBlt to copy the surface
		if (IsUsingEmulation() && pSourceSurface->IsUsingEmulation() && !IsColorKey)
		{
			LONG DestLeft = DestRect.left;
			LONG DestTop = DestRect.top;
			LONG DestWidth = DestRectWidth;
			LONG DestHeight = DestRectHeight;

			if (IsMirrorLeftRight)
			{
				DestLeft = DestRect.right;
				DestWidth = -DestWidth;
			}
			if (IsMirrorUpDown)
			{
				DestTop = DestRect.bottom;
				DestHeight = -DestHeight;
			}

			// Set new palette data
			UpdatePaletteData();
			pSourceSurface->UpdatePaletteData();

			// Set stretch mode
			if (IsStretchRect)
			{
				// After setting the HALFTONE stretching mode, an application must call the SetBrushOrgEx
				// function to set the brush origin. If it fails to do so, brush misalignment occurs.
				POINT org;
				GetBrushOrgEx(emu->surfaceDC, &org);
				SetStretchBltMode(emu->surfaceDC, (Filter & D3DTEXF_LINEAR) ? HALFTONE : COLORONCOLOR);
				SetBrushOrgEx(emu->surfaceDC, org.x, org.y, nullptr);
			}

			if ((IsStretchRect || IsMirrorLeftRight || IsMirrorUpDown) ?
				StretchBlt(emu->surfaceDC, DestLeft, DestTop, DestWidth, DestHeight,
					pSourceSurface->emu->surfaceDC, SrcRect.left, SrcRect.top, SrcRect.right - SrcRect.left, SrcRect.bottom - SrcRect.top, SRCCOPY) :
				BitBlt(emu->surfaceDC, DestRect.left, DestRect.top, DestRectWidth, DestRectHeight,
					pSourceSurface->emu->surfaceDC, SrcRect.left, SrcRect.top, SRCCOPY))
			{
				break;
			}
		}

		// Use D3DXLoadSurfaceFromSurface to copy the surface
		if (!IsUsingEmulation() && !IsColorKey && !IsMirrorLeftRight && !IsMirrorUpDown &&
			((SrcFormat != D3DFMT_P8 && DestFormat != D3DFMT_P8) || (SrcFormat == D3DFMT_P8 && DestFormat == D3DFMT_P8)))
		{
			IDirect3DSurface9* pSourceSurfaceD9 = pSourceSurface->GetD3D9Surface();
			IDirect3DSurface9* pDestSurfaceD9 = GetD3D9Surface();

			if (pSourceSurfaceD9 && pDestSurfaceD9)
			{
				if (SUCCEEDED(D3DXLoadSurfaceFromSurface(pDestSurfaceD9, nullptr, &DestRect, pSourceSurfaceD9, nullptr, &SrcRect, D3DXFilter, 0)))
				{
					break;
				}
			}
		}

		// Check source and destination format
		bool FormatMismatch = false;
		const bool FormatR5G6B5toX8R8G8B8 = (SrcFormat == D3DFMT_R5G6B5 && (DestFormat == D3DFMT_A8R8G8B8 || DestFormat == D3DFMT_X8R8G8B8));
		if (FormatR5G6B5toX8R8G8B8)
		{
			FormatMismatch = true;
			LOG_LIMIT(100, __FUNCTION__ << " Warning: source and destination formats don't match! " << SrcFormat << "-->" << DestFormat);
		}
		else if (!(SrcFormat == DestFormat || ISDXTEX(SrcFormat) && ISDXTEX(DestFormat) ||
			((SrcFormat == D3DFMT_A1R5G5B5 || SrcFormat == D3DFMT_X1R5G5B5) && (DestFormat == D3DFMT_A1R5G5B5 || DestFormat == D3DFMT_X1R5G5B5)) ||
			((SrcFormat == D3DFMT_A4R4G4B4 || SrcFormat == D3DFMT_X4R4G4B4) && (DestFormat == D3DFMT_A4R4G4B4 || DestFormat == D3DFMT_X4R4G4B4)) ||
			((SrcFormat == D3DFMT_A8R8G8B8 || SrcFormat == D3DFMT_X8R8G8B8) && (DestFormat == D3DFMT_A8R8G8B8 || DestFormat == D3DFMT_X8R8G8B8)) ||
			((SrcFormat == D3DFMT_A8B8G8R8 || SrcFormat == D3DFMT_X8B8G8R8) && (DestFormat == D3DFMT_A8B8G8R8 || DestFormat == D3DFMT_X8B8G8R8))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: not supported for specified source and destination formats! " << SrcFormat << "-->" << DestFormat);
			hr = DDERR_GENERIC;
			break;
		}

		// Get byte count
		DWORD DestBitCount = surfaceBitCount;
		DWORD ByteCount = DestBitCount / 8;
		if (!ByteCount || ByteCount > 4)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrong bit count " << DestBitCount);
			hr = DDERR_GENERIC;
			break;
		}

		// Check if source surface is not locked then lock it
		D3DLOCKED_RECT SrcLockRect = {};
		if (FAILED(pSourceSurface->IsUsingEmulation() ? pSourceSurface->LockEmulatedSurface(&SrcLockRect, &SrcRect) :
			pSourceSurface->LockD39Surface(&SrcLockRect, &SrcRect, D3DLOCK_READONLY)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock source surface " << SrcRect);
			hr = (pSourceSurface->IsSurfaceLocked()) ? DDERR_SURFACEBUSY : DDERR_GENERIC;
			break;
		}
		UnlockSrc = true;

		// Check if source and destination memory addresses are overlapping
		if (this == pSourceSurface)
		{
			size_t size = SrcRectWidth * ByteCount * SrcRectHeight;
			if (size > surfaceArray.size())
			{
				surfaceArray.resize(size);
			}
			BYTE* SrcBuffer = (BYTE*)SrcLockRect.pBits;
			BYTE* DestBuffer = (BYTE*)&surfaceArray[0];
			INT DestPitch = SrcRectWidth * ByteCount;
			for (LONG y = 0; y < SrcRectHeight; y++)
			{
				memcpy(DestBuffer, SrcBuffer, SrcRectWidth * ByteCount);
				SrcBuffer += SrcLockRect.Pitch;
				DestBuffer += DestPitch;
			}
			SrcLockRect.pBits = &surfaceArray[0];
			SrcLockRect.Pitch = DestPitch;
			if (UnlockSrc)
			{
				pSourceSurface->IsUsingEmulation() ? DD_OK : pSourceSurface->UnlockD39Surface();
				UnlockSrc = false;
			}
		}

		// Check if destination surface is not locked then lock it
		if (FAILED(IsUsingEmulation() ? LockEmulatedSurface(&DestLockRect, &DestRect) :
			LockD39Surface(&DestLockRect, &DestRect, 0)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock destination surface " << DestRect);
			hr = (IsLocked) ? DDERR_SURFACEBUSY : DDERR_GENERIC;
			break;
		}
		UnlockDest = true;

		// Create buffer variables
		BYTE* SrcBuffer = (BYTE*)SrcLockRect.pBits;
		BYTE* DestBuffer = (BYTE*)DestLockRect.pBits;

		// For mirror copy up/down
		INT DestPitch = DestLockRect.Pitch;
		if (IsMirrorUpDown)
		{
			DestPitch = -DestLockRect.Pitch;
			DestBuffer += DestLockRect.Pitch * (DestRectHeight - 1);
		}

		// Simple memory copy (QuickCopy)
		if (!IsStretchRect && !IsColorKey && !IsMirrorLeftRight && !FormatMismatch)
		{
			if (!IsMirrorUpDown && SrcLockRect.Pitch == DestLockRect.Pitch && (LONG)ComputePitch(DestRectWidth, DestBitCount) == DestPitch)
			{
				memcpy(DestBuffer, SrcBuffer, DestRectHeight * DestPitch);
			}
			else
			{
				for (LONG y = 0; y < DestRectHeight; y++)
				{
					memcpy(DestBuffer, SrcBuffer, DestRectWidth * ByteCount);
					SrcBuffer += SrcLockRect.Pitch;
					DestBuffer += DestPitch;
				}
			}
			break;
		}

		// Set color variables
		DWORD ByteMask = (ByteCount == 1) ? 0x000000FF : (ByteCount == 2) ? 0x0000FFFF : (ByteCount == 3) ? 0x00FFFFFF : 0xFFFFFFFF;
		DWORD ColorKeyLow = ColorKey.dwColorSpaceLowValue & ByteMask;
		DWORD ColorKeyHigh = ColorKey.dwColorSpaceHighValue & ByteMask;

		// Simple copy with ColorKey and Mirroring
		if (!IsStretchRect && !FormatMismatch)
		{
			if (ByteCount == 1)
			{
				BYTE* SrcBufferLoop = SrcBuffer;
				BYTE* DestBufferLoop = DestBuffer;
				for (LONG y = 0; y < DestRectHeight; y++)
				{
					for (LONG x = 0; x < DestRectWidth; x++)
					{
						BYTE PixelColor = SrcBufferLoop[IsMirrorLeftRight ? DestRectWidth - x : x];
						if (!IsColorKey || PixelColor < ColorKeyLow || PixelColor > ColorKeyHigh)
						{
							DestBufferLoop[x] = PixelColor;
						}
					}
					SrcBufferLoop += SrcLockRect.Pitch;
					DestBufferLoop += DestPitch;
				}
				break;
			}
			else if (ByteCount == 2)
			{
				WORD* SrcBufferLoop = (WORD*)SrcBuffer;
				WORD* DestBufferLoop = (WORD*)DestBuffer;
				for (LONG y = 0; y < DestRectHeight; y++)
				{
					for (LONG x = 0; x < DestRectWidth; x++)
					{
						WORD PixelColor = SrcBufferLoop[IsMirrorLeftRight ? DestRectWidth - x : x];
						if (!IsColorKey || PixelColor < ColorKeyLow || PixelColor > ColorKeyHigh)
						{
							DestBufferLoop[x] = PixelColor;
						}
					}
					SrcBufferLoop = (WORD*)((BYTE*)SrcBufferLoop + SrcLockRect.Pitch);
					DestBufferLoop = (WORD*)((BYTE*)DestBufferLoop + DestPitch);
				}
				break;
			}
			else if (ByteCount == 3)
			{
				TRIBYTE* SrcBufferLoop = (TRIBYTE*)SrcBuffer;
				TRIBYTE* DestBufferLoop = (TRIBYTE*)DestBuffer;
				for (LONG y = 0; y < DestRectHeight; y++)
				{
					for (LONG x = 0; x < DestRectWidth; x++)
					{
						LONG w = IsMirrorLeftRight ? DestRectWidth - x : x;
						DWORD PixelColor = (*(DWORD*)(SrcBufferLoop + w)) & ByteMask;
						if (!IsColorKey || PixelColor < ColorKeyLow || PixelColor > ColorKeyHigh)
						{
							DestBufferLoop[x] = SrcBufferLoop[w];
						}
					}
					SrcBufferLoop = (TRIBYTE*)((BYTE*)SrcBufferLoop + SrcLockRect.Pitch);
					DestBufferLoop = (TRIBYTE*)((BYTE*)DestBufferLoop + DestPitch);
				}
				break;
			}
			else if (ByteCount == 4)
			{
				DWORD* SrcBufferLoop = (DWORD*)SrcBuffer;
				DWORD* DestBufferLoop = (DWORD*)DestBuffer;
				for (LONG y = 0; y < DestRectHeight; y++)
				{
					for (LONG x = 0; x < DestRectWidth; x++)
					{
						DWORD PixelColor = SrcBufferLoop[IsMirrorLeftRight ? DestRectWidth - x : x];
						if (!IsColorKey || PixelColor < ColorKeyLow || PixelColor > ColorKeyHigh)
						{
							DestBufferLoop[x] = PixelColor;
						}
					}
					SrcBufferLoop = (DWORD*)((BYTE*)SrcBufferLoop + SrcLockRect.Pitch);
					DestBufferLoop = (DWORD*)((BYTE*)DestBufferLoop + DestPitch);
				}
				break;
			}
		}

		// Get ratio
		float WidthRatio = (float)SrcRectWidth / (float)DestRectWidth;
		float HeightRatio = (float)SrcRectHeight / (float)DestRectHeight;

		// Source byte count
		DWORD SrcByteCount = (FormatMismatch) ? GetBitCount(SrcFormat) / 8 : ByteCount;

		// Copy memory (complex)
		for (LONG y = 0; y < DestRectHeight; y++)
		{
			BYTE* LoopBuffer = DestBuffer;
			for (LONG x = 0; x < DestRectWidth; x++)
			{
				DWORD r = (IsStretchRect) ? (DWORD)((float)x * WidthRatio) : x;
				BYTE* NewPixel = (IsMirrorLeftRight) ? SrcBuffer + ((SrcRectWidth - r - 1) * SrcByteCount) : SrcBuffer + (r * SrcByteCount);
				DWORD PixelColor = (*(DWORD*)NewPixel) & ByteMask;

				if (!IsColorKey || PixelColor < ColorKeyLow || PixelColor > ColorKeyHigh)
				{
					if (FormatR5G6B5toX8R8G8B8)
					{
						*(DWORD*)LoopBuffer = D3DFMT_R5G6B5_TO_X8R8G8B8(*(WORD*)NewPixel);
						LoopBuffer += ByteCount;
					}
					else
					{
						for (DWORD i = 0; i < ByteCount; i++)
						{
							*LoopBuffer = NewPixel[i];
							LoopBuffer++;
						}
					}
				}
				else
				{
					LoopBuffer += ByteCount;
				}
			}
			SrcBuffer = (IsStretchRect) ? (BYTE*)SrcLockRect.pBits + (DWORD)((float)y * HeightRatio) * SrcLockRect.Pitch : SrcBuffer + SrcLockRect.Pitch;
			DestBuffer += DestPitch;
		}

	} while (false);

	// Remove scanlines before unlocking surface
	if (SUCCEEDED(hr) && Config.DdrawRemoveScanlines)
	{
		// Set last rect before removing scanlines
		LASTLOCK LLock;
		LLock.ScanlineWidth = DestRectWidth;
		LLock.Rect = DestRect;
		if (UnlockDest)
		{
			LLock.LockedRect = DestLockRect;
		}
		else if (IsUsingEmulation())
		{
			LockEmulatedSurface(&LLock.LockedRect, &DestRect);
		}

		RemoveScanlines(LLock);
	}

	// Unlock surfaces if needed
	if (UnlockSrc)
	{
		pSourceSurface->IsUsingEmulation() ? DD_OK : pSourceSurface->UnlockD39Surface();
	}
	if (UnlockDest)
	{
		IsUsingEmulation() ? DD_OK : UnlockD39Surface();
	}

	// Update for emulated surface
	if (SUCCEEDED(hr) && IsUsingEmulation())
	{
		// Copy emulated surface to real texture
		CopyFromEmulatedSurface(&DestRect);

		// Blt surface directly to GDI
		if (Config.DdrawWriteToGDI && (IsPrimarySurface() || IsBackBuffer()))
		{
			CopyEmulatedSurfaceToGDI(DestRect);
		}
	}

	// Return
	return hr;
}

// Copy from emulated surface to real surface
HRESULT m_IDirectDrawSurfaceX::CopyFromEmulatedSurface(LPRECT lpDestRect)
{
	if (!IsUsingEmulation() || Config.DdrawWriteToGDI)
	{
		return DDERR_GENERIC;
	}

	// Update rect
	RECT DestRect = {};
	if (!CheckCoordinates(&DestRect, lpDestRect))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid rect!");
		return DDERR_INVALIDRECT;
	}

	// Get lock for emulated surface
	D3DLOCKED_RECT EmulatedLockRect;
	if (FAILED(LockEmulatedSurface(&EmulatedLockRect, &DestRect)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not get emulated surface lock!");
		return DDERR_GENERIC;
	}

	// Get real d3d9 surface
	IDirect3DSurface9* pDestSurfaceD9 = GetD3D9Surface();
	if (!pDestSurfaceD9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not get real surface!");
		return DDERR_GENERIC;
	}

	// Use D3DXLoadSurfaceFromMemory to copy to the surface
	if (FAILED(D3DXLoadSurfaceFromMemory(pDestSurfaceD9, nullptr, &DestRect, emu->surfacepBits, (surfaceFormat == D3DFMT_P8) ? D3DFMT_L8 : surfaceFormat, EmulatedLockRect.Pitch, nullptr, &DestRect, D3DX_FILTER_NONE, 0)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not copy emulated surface: " << surfaceFormat);
		return DDERR_GENERIC;
	}

	return DD_OK;
}

// Copy from real surface to emulated surface
HRESULT m_IDirectDrawSurfaceX::CopyToEmulatedSurface(LPRECT lpDestRect)
{
	if (!IsUsingEmulation() || Config.DdrawWriteToGDI)
	{
		return DDERR_GENERIC;
	}

	// Update rect
	RECT DestRect = {};
	if (!CheckCoordinates(&DestRect, lpDestRect))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid rect!");
		return DDERR_INVALIDRECT;
	}

	// Get lock for emulated surface
	D3DLOCKED_RECT EmulatedLockRect = {};
	if (FAILED(LockEmulatedSurface(&EmulatedLockRect, &DestRect)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not get emulated surface lock!");
		return DDERR_GENERIC;
	}

	// Get lock for real surface
	D3DLOCKED_RECT SrcLockRect = {};
	if (FAILED(LockD39Surface(&SrcLockRect, &DestRect, 0)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock destination surface " << DestRect);
		return (IsLocked) ? DDERR_SURFACEBUSY : DDERR_GENERIC;
	}

	// Create buffer variables
	BYTE* EmulatedBuffer = (BYTE*)EmulatedLockRect.pBits;
	BYTE* SurfaceBuffer = (BYTE*)SrcLockRect.pBits;

	DWORD Height = (DestRect.bottom - DestRect.top);
	INT WidthPitch = min(SrcLockRect.Pitch, EmulatedLockRect.Pitch);

	HRESULT hr = DD_OK;

	// Copy real surface data to emulated surface
	switch ((DWORD)surfaceFormat)
	{
	case D3DFMT_X4R4G4B4:
	case D3DFMT_A4R4G4B4:
		for (LONG x = DestRect.top; x < DestRect.bottom; x++)
		{
			WORD* EmulatedBufferLoop = (WORD*)EmulatedBuffer;
			DWORD* SurfaceBufferLoop = (DWORD*)SurfaceBuffer;
			for (LONG y = DestRect.left; y < DestRect.right; y++)
			{
				*EmulatedBufferLoop = D3DFMT_A8R8G8B8_TO_A4R4G4B4(*SurfaceBufferLoop);
				EmulatedBufferLoop++;
				SurfaceBufferLoop++;
			}
			EmulatedBuffer += EmulatedLockRect.Pitch;
			SurfaceBuffer += SrcLockRect.Pitch;
		}
		break;
	case D3DFMT_R8G8B8:
		for (LONG x = DestRect.top; x < DestRect.bottom; x++)
		{
			TRIBYTE* EmulatedBufferLoop = (TRIBYTE*)EmulatedBuffer;
			DWORD* SurfaceBufferLoop = (DWORD*)SurfaceBuffer;
			for (LONG y = DestRect.left; y < DestRect.right; y++)
			{
				*EmulatedBufferLoop = *(TRIBYTE*)SurfaceBufferLoop;
				EmulatedBufferLoop++;
				SurfaceBufferLoop++;
			}
			EmulatedBuffer += EmulatedLockRect.Pitch;
			SurfaceBuffer += SrcLockRect.Pitch;
		}
		break;
	case D3DFMT_B8G8R8:
		for (LONG x = DestRect.top; x < DestRect.bottom; x++)
		{
			TRIBYTE* EmulatedBufferLoop = (TRIBYTE*)EmulatedBuffer;
			DWORD* SurfaceBufferLoop = (DWORD*)SurfaceBuffer;
			for (LONG y = DestRect.left; y < DestRect.right; y++)
			{
				DWORD Pixel = D3DFMT_X8R8G8B8_TO_B8G8R8(*SurfaceBufferLoop);
				*EmulatedBufferLoop = *(TRIBYTE*)&Pixel;
				EmulatedBufferLoop++;
				SurfaceBufferLoop++;
			}
			EmulatedBuffer += EmulatedLockRect.Pitch;
			SurfaceBuffer += SrcLockRect.Pitch;
		}
		break;
	case D3DFMT_X8B8G8R8:
	case D3DFMT_A8B8G8R8:
		for (LONG x = DestRect.top; x < DestRect.bottom; x++)
		{
			DWORD* EmulatedBufferLoop = (DWORD*)EmulatedBuffer;
			DWORD* SurfaceBufferLoop = (DWORD*)SurfaceBuffer;
			for (LONG y = DestRect.left; y < DestRect.right; y++)
			{
				*EmulatedBufferLoop = D3DFMT_A8R8G8B8_TO_A8B8G8R8(*SurfaceBufferLoop);
				EmulatedBufferLoop++;
				SurfaceBufferLoop++;
			}
			EmulatedBuffer += EmulatedLockRect.Pitch;
			SurfaceBuffer += SrcLockRect.Pitch;
		}
		break;
	default:
		if (SrcLockRect.Pitch == EmulatedLockRect.Pitch && (LONG)ComputePitch((DWORD)(DestRect.right - DestRect.left), emu->bmi->bmiHeader.biBitCount) == EmulatedLockRect.Pitch)
		{
			memcpy(EmulatedBuffer, SurfaceBuffer, SrcLockRect.Pitch * Height);
		}
		else if (emu->bmi->bmiHeader.biBitCount == surfaceBitCount)
		{
			for (UINT x = 0; x < Height; x++)
			{
				memcpy(EmulatedBuffer, SurfaceBuffer, WidthPitch);
				EmulatedBuffer += EmulatedLockRect.Pitch;
				SurfaceBuffer += SrcLockRect.Pitch;
			}
		}
		else
		{
			hr = DDERR_GENERIC;
			LOG_LIMIT(100, __FUNCTION__ << " Error: emulated surface format not supported: " << surfaceFormat);
		}
	}

	// Unlock surface
	UnlockD39Surface();

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::CopyEmulatedSurfaceFromGDI(RECT Rect)
{
	if (!IsUsingEmulation() || !ddrawParent->GetDC())
	{
		return DDERR_GENERIC;
	}

	RECT WindowRect = {};
	GetClientRect(ddrawParent->GetHwnd(), &WindowRect);
	MapWindowPoints(ddrawParent->GetHwnd(), HWND_DESKTOP, (LPPOINT)&WindowRect, 2);
	LONG XOffset = WindowRect.left;
	LONG YOffset = WindowRect.top;
	LONG Left = Rect.left + XOffset;
	LONG Top = Rect.top + YOffset;
	LONG Width = Rect.right - Rect.left;
	LONG Height = Rect.bottom - Rect.top;

	if (Rect.left + Width > WindowRect.right || Rect.top + Height > WindowRect.bottom)
	{
		return DDERR_GENERIC;
	}

	// Set new palette data
	UpdatePaletteData();

	BitBlt(emu->surfaceDC, Left, Top, Width, Height, ddrawParent->GetDC(), Rect.left, Rect.top, SRCCOPY);

	return DD_OK;
}

HRESULT m_IDirectDrawSurfaceX::CopyEmulatedSurfaceToGDI(RECT Rect)
{
	if (!IsUsingEmulation() || !ddrawParent->GetDC())
	{
		return DDERR_GENERIC;
	}

	RECT WindowRect = {};
	MapWindowPoints(ddrawParent->GetHwnd(), HWND_DESKTOP, (LPPOINT)&WindowRect, 2);
	LONG XOffset = WindowRect.left;
	LONG YOffset = WindowRect.top;
	LONG Left = (Rect.left >= XOffset) ? Rect.left - XOffset : Rect.left;
	LONG Top = (Rect.top >= YOffset) ? Rect.top - YOffset : Rect.top;
	LONG Width = Rect.right - Rect.left;
	LONG Height = Rect.bottom - Rect.top;

	if (Rect.left + Width > (LONG)surfaceDesc2.dwWidth || Rect.top + Height > (LONG)surfaceDesc2.dwHeight)
	{
		return DDERR_GENERIC;
	}

	// Set new palette data
	UpdatePaletteData();

	BitBlt(ddrawParent->GetDC(), Left, Top, Width, Height, emu->surfaceDC, Rect.left, Rect.top, SRCCOPY);

	return DD_OK;
}

void m_IDirectDrawSurfaceX::RemoveClipper(m_IDirectDrawClipper* ClipperToRemove)
{
	if (ClipperToRemove == attachedClipper)
	{
		attachedClipper = nullptr;
		ClipperFirstRun = true;
	}
}

void m_IDirectDrawSurfaceX::RemovePalette(m_IDirectDrawPalette* PaletteToRemove)
{
	if (PaletteToRemove == attachedPalette)
	{
		attachedPalette = nullptr;
		PaletteFirstRun = true;
	}
}

void m_IDirectDrawSurfaceX::UpdatePaletteData()
{
	// Check surface format
	if (surfaceFormat != D3DFMT_P8)
	{
		return;
	}

	DWORD CurrentPaletteUSN = 0;
	DWORD entryCount = 0;
	D3DCOLOR *rgbPalette = nullptr;

	// Get palette data
	if (attachedPalette && attachedPalette->GetRgbPalette())
	{
		CurrentPaletteUSN = PaletteUSN + attachedPalette->GetPaletteUSN();
		if (CurrentPaletteUSN && 
			(CurrentPaletteUSN != LastPaletteUSN || (IsUsingEmulation() && CurrentPaletteUSN != emu->LastPaletteUSN)))
		{
			rgbPalette = (D3DCOLOR*)attachedPalette->GetRgbPalette();
			entryCount = attachedPalette->GetEntryCount();
		}
	}
	// Get palette from primary surface if this is not primary
	else if (!IsPrimarySurface())
	{
		m_IDirectDrawSurfaceX *lpPrimarySurface = ddrawParent->GetPrimarySurface();
		if (lpPrimarySurface)
		{
			m_IDirectDrawPalette *lpPalette = lpPrimarySurface->GetAttachedPalette();
			if (lpPalette && lpPalette->GetRgbPalette())
			{
				CurrentPaletteUSN = lpPrimarySurface->GetPaletteUSN() + lpPalette->GetPaletteUSN();
				if (CurrentPaletteUSN &&
					(CurrentPaletteUSN != LastPaletteUSN || (IsUsingEmulation() && CurrentPaletteUSN != emu->LastPaletteUSN)))
				{
					rgbPalette = (D3DCOLOR*)lpPalette->GetRgbPalette();
					entryCount = lpPalette->GetEntryCount();
				}
			}
		}
	}

	// No new palette data found
	if (!rgbPalette)
	{
		return;
	}

	// Set color palette for device context
	if (IsUsingEmulation())
	{
		SetDIBColorTable(emu->surfaceDC, 0, entryCount, (RGBQUAD*)rgbPalette);

		emu->LastPaletteUSN = CurrentPaletteUSN;
	}

	// If new palette data then write it to texture
	if (paletteTexture)
	{
		do {
			LPDIRECT3DSURFACE9 paletteSurface = nullptr;
			if (FAILED(paletteTexture->GetSurfaceLevel(0, &paletteSurface)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get palette surface!");
				break;
			}

			// Use D3DXLoadSurfaceFromMemory to copy to the surface
			RECT Rect = { 0, 0, (LONG)entryCount, 1 };
			if (FAILED(D3DXLoadSurfaceFromMemory(paletteSurface, nullptr, &Rect, rgbPalette, D3DFMT_X8R8G8B8, 256 * 4, nullptr, &Rect, D3DX_FILTER_NONE, 0)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not update palette surface!");
			}

			paletteSurface->Release();

			LastPaletteUSN = CurrentPaletteUSN;

		} while (false);
	}
}

void m_IDirectDrawSurfaceX::StartSharedEmulatedMemory()
{
	ShareEmulatedMemory = true;
	InitializeCriticalSection(&smcs);
}

void m_IDirectDrawSurfaceX::DeleteEmulatedMemory(EMUSURFACE **ppEmuSurface)
{
	if (!ppEmuSurface || !*ppEmuSurface)
	{
		return;
	}

	LOG_LIMIT(100, __FUNCTION__ << " Deleting emulated surface (" << *ppEmuSurface << ")");

	// Release device context memory
	if ((*ppEmuSurface)->surfaceDC)
	{
		SelectObject((*ppEmuSurface)->surfaceDC, (*ppEmuSurface)->OldDCObject);
		DeleteDC((*ppEmuSurface)->surfaceDC);
	}
	if ((*ppEmuSurface)->bitmap)
	{
		DeleteObject((*ppEmuSurface)->bitmap);
		(*ppEmuSurface)->surfacepBits = nullptr;
	}
	if ((*ppEmuSurface)->surfacepBits)
	{
		HeapFree(GetProcessHeap(), NULL, (*ppEmuSurface)->surfacepBits);
		(*ppEmuSurface)->surfacepBits = nullptr;
	}
	delete (*ppEmuSurface);
	*ppEmuSurface = nullptr;
}

void m_IDirectDrawSurfaceX::CleanupSharedEmulatedMemory()
{
	// Disable shared memory
	ShareEmulatedMemory = false;
	
	// Make sure that vector is not in use
	EnterCriticalSection(&smcs);
	LeaveCriticalSection(&smcs);

	// Deleted critical section
	DeleteCriticalSection(&smcs);

	LOG_LIMIT(100, __FUNCTION__ << " Deleting " << memorySurfaces.size() << " emulated surface" << ((memorySurfaces.size() != 1) ? "s" : "") << "!");

	// Clean up unused emulated surfaces
	for (EMUSURFACE* pEmuSurface: memorySurfaces)
	{
		DeleteEmulatedMemory(&pEmuSurface);
	}
	memorySurfaces.clear();
}
