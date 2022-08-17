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
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->AddOverlayDirtyRect(lpRect);
}

HRESULT m_IDirectDrawSurfaceX::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx, BOOL isSkipScene)
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
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		// Check for required DDBLTFX structure
		if (!lpDDBltFx && (dwFlags & (DDBLT_DDFX | DDBLT_COLORFILL | DDBLT_DEPTHFILL | DDBLT_DDROPS | DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_ROP | DDBLT_ROTATIONANGLE)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: DDBLTFX structure not found");
			return DDERR_INVALIDPARAMS;
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
		if ((dwFlags & DDBLT_ROTATIONANGLE) || ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & (DDBLTFX_ROTATE90 | DDBLTFX_ROTATE180 | DDBLTFX_ROTATE270))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Rotation operations Not Implemented");
			return DDERR_NOROTATIONHW;
		}

		// Check for Alpha flags
		if (dwFlags & 0x1FF)
		{
			LOG_LIMIT(100, __FUNCTION__ << " DDBLT_ALPHA flags Not Implemented " << Logging::hex(dwFlags & 0x1FF));
		}

		// Other flags (can be safely ignored?)
		// DDBLT_DDROPS - dwDDROP is ignored as "no such ROPs are currently defined"
		// DDBLT_ASYNC
		// DDBLT_DONOTWAIT
		// DDBLT_WAIT

		// Get source surface
		m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = (m_IDirectDrawSurfaceX*)lpDDSrcSurface;
		if (!lpDDSrcSurfaceX)
		{
			lpDDSrcSurfaceX = this;
		}
		else
		{
			lpDDSrcSurfaceX->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpDDSrcSurfaceX);
		}

		HRESULT hr = DD_OK;

		do {
			// Check if the scene needs to be presented
			isSkipScene |= ((lpDestRect) ? (abs(lpDestRect->bottom - lpDestRect->top) < 2 || abs(lpDestRect->right - lpDestRect->left) < 2) : FALSE);

			// Present before write if needed
			BeginWritePresent(isSkipScene);

			IsInBlt = true;

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
					LOG_LIMIT(100, __FUNCTION__ << " Raster operation Not Implemented " << Logging::hex(lpDDBltFx->dwROP));
					hr = DDERR_NORASTEROPHW;
					break;
				}
			}

			// Get surface copy flags
			DWORD Flags = ((dwFlags & (DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_KEYDEST | DDBLT_KEYSRC)) ? DDBLT_KEYDEST : 0) |		// Color key flags
				((lpDDBltFx && (dwFlags & DDBLT_DDFX)) ? (lpDDBltFx->dwDDFX & (DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN)) : 0);			// Mirror flags

			// Check if color key is set
			if (((dwFlags & DDBLT_KEYDEST) && !(surfaceDesc2.ddsCaps.dwCaps & DDSD_CKDESTBLT)) || ((dwFlags & DDBLT_KEYSRC) && !(lpDDSrcSurfaceX->surfaceDesc2.ddsCaps.dwCaps & DDSD_CKSRCBLT)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: color key not set");
				Flags &= ~(DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_KEYDEST | DDBLT_KEYSRC);
			}

			// Get color key
			DDCOLORKEY ColorKey = (dwFlags & DDBLT_KEYDESTOVERRIDE) ? lpDDBltFx->ddckDestColorkey :
				(dwFlags & DDBLT_KEYSRCOVERRIDE) ? lpDDBltFx->ddckSrcColorkey :
				(dwFlags & DDBLT_KEYDEST) ? surfaceDesc2.ddckCKDestBlt :
				(dwFlags & DDBLT_KEYSRC) ? lpDDSrcSurfaceX->surfaceDesc2.ddckCKSrcBlt : surfaceDesc2.ddckCKDestBlt;

			D3DTEXTUREFILTERTYPE Filter = ((lpDDBltFx && dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & DDBLTFX_ARITHSTRETCHY)) ? D3DTEXF_LINEAR : D3DTEXF_NONE;

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
			if ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & DDBLTFX_NOTEARING))
			{
				ddrawParent->SetVsync();
			}

			// Present surface
			EndWritePresent(isSkipScene);
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

	return ProxyInterface->Blt(lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
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

	return ProxyInterface->BltFast(dwX, dwY, lpDDSrcSurface, lpSrcRect, dwFlags);
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
		for (auto it : AttachedSurfaceMap)
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
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
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

HRESULT m_IDirectDrawSurfaceX::Flip(LPDIRECTDRAWSURFACE7 lpDDSurfaceTargetOverride, DWORD dwFlags)
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
			return DDERR_SURFACEBUSY;
		}

		if ((dwFlags & (DDFLIP_INTERVAL2 | DDFLIP_INTERVAL3 | DDFLIP_INTERVAL4)) && (surfaceDesc2.ddsCaps.dwCaps2 & DDCAPS2_FLIPINTERVAL))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Interval flipping not fully implemented");
		}

		// Other flags (can be safely ignored?)
		// - DDFLIP_DONOTWAIT
		// - DDFLIP_WAIT

		// Present before write if needed
		BeginWritePresent();

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
			EndWritePresent();
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

		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
		if (FAILED(c_hr))
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
		BeginWritePresent();

		if (IsSurfaceEmulated || DCRequiresEmulation)
		{
			if (!emu)
			{
				CreateDCSurface();
			}

			if (IsSurfaceEmulated)
			{
				// Read surface from GDI
				if (Config.DdrawReadFromGDI && (IsPrimarySurface() || IsBackBuffer()))
				{
					CopyEmulatedSurfaceFromGDI(LastLock.Rect);
				}
			}
			// Copy surface data to device context
			else
			{
				CopyEmulatedSurface(nullptr, false);
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
			lpDDSurfaceDesc2->lPitch = ComputePitch(lpDDSurfaceDesc2->dwWidth, GetBitCount(lpDDSurfaceDesc2->ddpfPixelFormat));
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

HRESULT m_IDirectDrawSurfaceX::Lock(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent)
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

		HRESULT hr = Lock2(lpDestRect, &Desc2, dwFlags, hEvent);

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

HRESULT m_IDirectDrawSurfaceX::Lock2(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD dwFlags, HANDLE hEvent)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check surfaceDesc
		if (lpDDSurfaceDesc2 && lpDDSurfaceDesc2->dwSize == sizeof(DDSURFACEDESC))
		{
			return Lock(lpDestRect, (LPDDSURFACEDESC)lpDDSurfaceDesc2, dwFlags, hEvent);
		}

		// Check parameters
		if (!lpDDSurfaceDesc2 || lpDDSurfaceDesc2->dwSize != sizeof(DDSURFACEDESC2))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc2) ? lpDDSurfaceDesc2->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		// Clear surfaceDesc
		ZeroMemory(lpDDSurfaceDesc2, sizeof(DDSURFACEDESC2));
		lpDDSurfaceDesc2->dwSize = sizeof(DDSURFACEDESC2);

		// Convert flags to d3d9
		DWORD Flags = dwFlags & (D3DLOCK_READONLY | D3DLOCK_DONOTWAIT | (!IsPrimarySurface() ? D3DLOCK_NOSYSLOCK : 0));

		// Locked rect
		D3DLOCKED_RECT LockedRect = {};

		HRESULT hr = DDERR_INVALIDPARAMS;

		do {
			// Update rect
			RECT DestRect = {};
			if (lpDestRect && (!CheckCoordinates(&DestRect, lpDestRect) || (lpDestRect->left < 0 || lpDestRect->top < 0 ||
				lpDestRect->right <= lpDestRect->left || lpDestRect->bottom <= lpDestRect->top ||
				lpDestRect->right >(LONG)surfaceDesc2.dwWidth || lpDestRect->bottom >(LONG)surfaceDesc2.dwHeight)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << lpDestRect);
				hr = DDERR_INVALIDRECT;
				break;
			}

			// Lock surface
			hr = SetLock(&LockedRect, (lpDestRect) ? &DestRect : nullptr, Flags);
			if (FAILED(hr))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock texture surface! " << (DDERR)hr);
				break;
			}

			// Check pointer and pitch
			if (!LockedRect.pBits || !LockedRect.Pitch)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get surface address or pitch!");
				hr = DDERR_GENERIC;
				break;
			}

			// Store locked rect
			if (lpDestRect)
			{
				RECT lRect = { lpDestRect->left, lpDestRect->top, lpDestRect->right, lpDestRect->bottom };
				surfaceLockRectList.push_back(lRect);
			}
			else if (!lpDestRect && !surfaceLockRectList.empty())
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: locking surface with NULL rect when surface is already locked!");
				hr = DDERR_INVALIDRECT;
				break;
			}
		} while (FALSE);

		if (FAILED(hr))
		{
			ConvertSurfaceDesc(*lpDDSurfaceDesc2, surfaceDesc2);
			if (!(lpDDSurfaceDesc2->dwFlags & DDSD_LPSURFACE))
			{
				lpDDSurfaceDesc2->lpSurface = nullptr;
			}
			return hr;
		}

		// Set surfaceDesc
		ConvertSurfaceDesc(*lpDDSurfaceDesc2, surfaceDesc2);
		if (!(lpDDSurfaceDesc2->dwFlags & DDSD_LPSURFACE))
		{
			lpDDSurfaceDesc2->lpSurface = LockedRect.pBits;
		}
		lpDDSurfaceDesc2->lPitch = LockedRect.Pitch;
		lpDDSurfaceDesc2->dwFlags |= DDSD_PITCH | DDSD_LPSURFACE;

		// Fix misaligned bytes
		if (Config.DdrawFixByteAlignment)
		{
			LockBitAlign<LPDDSURFACEDESC2>(lpDestRect, lpDDSurfaceDesc2);
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

HRESULT m_IDirectDrawSurfaceX::ReleaseDC(HDC hDC)
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

		if (!IsSurfaceInDC())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not in DC!");
			return DDERR_GENERIC;
		}

		if (IsSurfaceEmulated || DCRequiresEmulation)
		{
			// Blt surface directly to GDI
			if (Config.DdrawWriteToGDI && (IsPrimarySurface() || IsBackBuffer()))
			{
				CopyEmulatedSurfaceToGDI(LastLock.Rect);
			}
			// Copy emulated surface to real surface
			else
			{
				CopyEmulatedSurface(nullptr, true);
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
		EndWritePresent();

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

			// Check if palette exists
			if (ddrawParent && ddrawParent->DoesPaletteExist((m_IDirectDrawPalette *)lpDDPalette))
			{
				// Decrement ref count
				attachedPalette->Release();
			}

			// Detach
			attachedPalette = nullptr;

			// Reset FirstRun
			PaletteFirstRun = true;
		}
		// If lpDDPalette is not null
		else
		{
			// Check if palette exists
			if (!ddrawParent || !ddrawParent->DoesPaletteExist((m_IDirectDrawPalette *)lpDDPalette))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not find palette");
				return DDERR_INVALIDOBJECT;
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

			// Set primary
			if (IsPrimarySurface())
			{
				attachedPalette->SetPrimary();
			}

			// If new palette is set
			PaletteUSN++;
		}

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
		BYTE *InAddr = &EmuLock.surfaceMem[0];
		DWORD InPitch = (EmuLock.BBP / 8) * EmuLock.Width;
		BYTE *OutAddr = (BYTE*)EmuLock.Addr;
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

		// Unlock surface
		return SetUnlock();
	}

	return ProxyInterface->Unlock(lpRect);
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
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
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
		lpDDSurfaceDesc->dwHeight += lpDDSurfaceDesc->dwHeight % 2;
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
		lpDDSurfaceDesc2->dwHeight += lpDDSurfaceDesc2->dwHeight % 2;
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

	// Initialize Critical Section for surface array
	InitializeCriticalSection(&ddscs);

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

	if (attachedTexture)
	{
		attachedTexture->ClearSurface();
	}

	if (ddrawParent)
	{
		ddrawParent->RemoveSurfaceFromVector(this);
	}

	ReleaseD9Surface(false);

	// Delete Critical Section for surface array
	DeleteCriticalSection(&ddscs);
}

LPDIRECT3DSURFACE9 m_IDirectDrawSurfaceX::Get3DSurface()
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, true)))
	{
		return nullptr;
	}

	return surface3D;
}

LPDIRECT3DTEXTURE9 m_IDirectDrawSurfaceX::Get3DTexture()
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, true)))
	{
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
		if (!IsSurfaceEmulated && CheckD3DDevice)
		{
			HRESULT hr = (*d3d9Device)->TestCooperativeLevel();
			if (hr == D3DERR_DEVICELOST || (hr == D3DERR_DEVICENOTRESET && FAILED(ddrawParent->ReinitDevice())))
			{
				return DDERR_SURFACELOST;
			}
			else if (FAILED(hr))
			{
				LOG_LIMIT(100, FunctionName << " Error: TestCooperativeLevel = " << (DDERR)hr);
				return DDERR_GENERIC;
			}
		}

		// Check if using Direct3D
		IsDirect3DSurface = ddrawParent->IsUsing3D();

		// Make sure surface exists, if not then create it
		if ((!surfaceTexture && !surface3D) || (IsDirect3DSurface && !IsTexture() && !surface3D))
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
	ReleaseD9Surface();

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, false)))
	{
		return DDERR_GENERIC;
	}

	// Update surface description
	UpdateSurfaceDesc();

	// Adjust Height and Width
	DWORD Width = surfaceDesc2.dwWidth + (surfaceDesc2.dwWidth % 2);
	DWORD Height = surfaceDesc2.dwHeight + (surfaceDesc2.dwHeight % 2);

	// Get texture data
	surfaceFormat = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);
	surfaceBitCount = GetBitCount(surfaceFormat);
	IsSurfaceEmulated = (surfaceFormat == D3DFMT_X4R4G4B4 || surfaceFormat == D3DFMT_A4R4G4B4 || surfaceFormat == D3DFMT_R8G8B8 || surfaceFormat == D3DFMT_B8G8R8 ||
		surfaceFormat == D3DFMT_X8B8G8R8 || surfaceFormat == D3DFMT_A8B8G8R8) ||
		((Config.DdrawEmulateSurface || ((IsPrimarySurface() || IsBackBuffer()) && (Config.DdrawWriteToGDI || Config.DdrawReadFromGDI))) &&
		(surfaceFormat == D3DFMT_P8 || surfaceFormat == D3DFMT_R5G6B5 || surfaceFormat == D3DFMT_A1R5G5B5 || surfaceFormat == D3DFMT_X1R5G5B5 ||
			surfaceFormat == D3DFMT_A8R8G8B8 || surfaceFormat == D3DFMT_X8R8G8B8));
	DCRequiresEmulation = (surfaceFormat != D3DFMT_R5G6B5 && surfaceFormat != D3DFMT_X1R5G5B5 && surfaceFormat != D3DFMT_R8G8B8 && surfaceFormat != D3DFMT_X8R8G8B8);
	D3DFORMAT TextureFormat = (surfaceFormat == D3DFMT_P8) ? D3DFMT_L8 : (surfaceFormat == D3DFMT_R8G8B8) ? D3DFMT_X8R8G8B8 :
		(surfaceFormat == D3DFMT_X4R4G4B4 || surfaceFormat == D3DFMT_R8G8B8 || surfaceFormat == D3DFMT_B8G8R8 || surfaceFormat == D3DFMT_X8B8G8R8) ? D3DFMT_X8R8G8B8 :
		(surfaceFormat == D3DFMT_A4R4G4B4 || surfaceFormat == D3DFMT_A8B8G8R8) ? D3DFMT_A8R8G8B8 : surfaceFormat;
	D3DFORMAT Format = (surfaceFormat == D3DFMT_R8G8B8 || surfaceFormat == D3DFMT_B8G8R8 || surfaceFormat == D3DFMT_X8B8G8R8) ? D3DFMT_X8R8G8B8 : (surfaceFormat == D3DFMT_A8B8G8R8) ? D3DFMT_A8R8G8B8 : surfaceFormat;
	bool UseDynamic = (ddrawParent->IsDynamicTexturesSupported() && !IsTexture() && !(surfaceDesc2.ddsCaps.dwCaps & (DDSCAPS_SYSTEMMEMORY | DDSCAPS_FLIP)) && !Config.DdrawWriteToGDI);

	Logging::LogDebug() << __FUNCTION__ " (" << this << ") D3d9 Surface size: " << Width << "x" << Height << " Format: " << surfaceFormat;

	// Create emulated surface using device context for creation
	if (IsSurfaceEmulated)
	{
		CreateDCSurface();
	}

	// Create primary texture
	if (!IsDirect3DSurface && (IsPrimarySurface() || IsBackBuffer()))
	{
		if (FAILED(D3DXCreateTexture(*d3d9Device, Width, Height, 1, (UseDynamic ? D3DUSAGE_DYNAMIC : 0), TextureFormat, (UseDynamic ? D3DPOOL_DEFAULT : D3DPOOL_SYSTEMMEM), &surfaceTexture)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create primary surface texture size: " << Width << "x" << Height << " Format: " << surfaceFormat);
			return DDERR_GENERIC;
		}

		// Create display texture
		if (IsPrimarySurface() && !UseDynamic && !Config.DdrawWriteToGDI)
		{
			if (FAILED((D3DXCreateTexture(*d3d9Device, Width, Height, 1, 0, TextureFormat, D3DPOOL_DEFAULT, &displayTexture))))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create display surface texture size: " << Width << "x" << Height << " Format: " << surfaceFormat);
				return DDERR_GENERIC;
			}
		}

		// Create palette surface
		if (IsPrimarySurface() && surfaceFormat == D3DFMT_P8)
		{
			if (FAILED((D3DXCreateTexture(*d3d9Device, 256, 256, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &paletteTexture))) ||
				FAILED((*d3d9Device)->CreatePixelShader((DWORD*)PalettePixelShaderSrc, &pixelShader)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create palette surface texture");
				return DDERR_GENERIC;
			}
		}
	}
	// Create texture
	else if (IsTexture() || !IsDirect3DSurface)
	{
		if (FAILED((D3DXCreateTexture(*d3d9Device, Width, Height, 1, 0, TextureFormat, D3DPOOL_SYSTEMMEM, &surfaceTexture))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create surface texture size: " << Width << "x" << Height << " Format: " << surfaceFormat);
			return DDERR_GENERIC;
		}
	}
	// Create offplain surface
	else
	{
		if (FAILED((*d3d9Device)->CreateOffscreenPlainSurface(Width, Height, Format, D3DPOOL_SYSTEMMEM, &surface3D, nullptr)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create surface size: " << Width << "x" << Height << " Format: " << surfaceFormat);
			return DDERR_GENERIC;
		}
	}

	// Reset d3d9 surface texture data
	if (!surfaceBackup.empty() && (surfaceTexture || surface3D))
	{
		do {
			D3DLOCKED_RECT LockRect = { NULL };
			HRESULT hr = (surfaceTexture) ? surfaceTexture->LockRect(0, &LockRect, nullptr, 0) :
				(surface3D) ? surface3D->LockRect(&LockRect, nullptr, 0) : DDERR_GENERIC;
			if (FAILED(hr))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock texture surface!");
				break;
			}

			size_t size = Height * LockRect.Pitch;

			if (size == surfaceBackup.size())
			{
				Logging::LogDebug() << __FUNCTION__ << " Resetting Direct3D9 texture surface data";

				memcpy(LockRect.pBits, &surfaceBackup[0], size);
			}

			(surfaceTexture) ? surfaceTexture->UnlockRect(0) :
				(surface3D) ? surface3D->UnlockRect() : DDERR_GENERIC;

		} while (false);
	}

	// Data is no longer needed
	surfaceBackup.clear();

	// Mark palette as updated
	PaletteUSN++;

	// Only need to create vertex buffer for primary surface when using DirectDraw and not writing to GDI
	if (!IsPrimarySurface() || IsDirect3DSurface || Config.DdrawWriteToGDI)
	{
		return DD_OK;
	}

	// Create vertex buffer
	if (FAILED((*d3d9Device)->CreateVertexBuffer(sizeof(TLVERTEX) * 4, D3DUSAGE_DYNAMIC, TLVERTEXFVF, D3DPOOL_DEFAULT, &vertexBuffer, nullptr)))
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

	return DD_OK;
}

HRESULT m_IDirectDrawSurfaceX::CreateDCSurface()
{
	// Check if color masks are needed
	bool ColorMaskReq = ((surfaceBitCount == 16 || surfaceBitCount == 32) &&																// Only valid when used with 16 bit and 32 bit surfaces
		(surfaceDesc2.ddpfPixelFormat.dwFlags & DDPF_RGB) &&																				// Check to make sure it is an RGB surface
		(surfaceDesc2.ddpfPixelFormat.dwRBitMask && surfaceDesc2.ddpfPixelFormat.dwGBitMask && surfaceDesc2.ddpfPixelFormat.dwBBitMask));	// Check to make sure the masks actually exist
	
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
			if (emu->bmi->bmiHeader.biWidth == (LONG)surfaceDesc2.dwWidth && -emu->bmi->bmiHeader.biHeight == (LONG)surfaceDesc2.dwHeight &&
				emu->bmi->bmiHeader.biBitCount == surfaceBitCount &&
				(!ColorMaskReq || ((DWORD*)emu->bmi->bmiColors)[0] == surfaceDesc2.ddpfPixelFormat.dwRBitMask &&
				((DWORD*)emu->bmi->bmiColors)[1] == surfaceDesc2.ddpfPixelFormat.dwGBitMask &&
					((DWORD*)emu->bmi->bmiColors)[2] == surfaceDesc2.ddpfPixelFormat.dwBBitMask))
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

			if (pEmuSurface->bmi->bmiHeader.biWidth == (LONG)surfaceDesc2.dwWidth && -pEmuSurface->bmi->bmiHeader.biHeight == (LONG)surfaceDesc2.dwHeight &&
				pEmuSurface->bmi->bmiHeader.biBitCount == surfaceBitCount &&
				(!ColorMaskReq || ((DWORD*)pEmuSurface->bmi->bmiColors)[0] == surfaceDesc2.ddpfPixelFormat.dwRBitMask &&
				((DWORD*)pEmuSurface->bmi->bmiColors)[1] == surfaceDesc2.ddpfPixelFormat.dwGBitMask &&
					((DWORD*)pEmuSurface->bmi->bmiColors)[2] == surfaceDesc2.ddpfPixelFormat.dwBBitMask))
			{
				emu = pEmuSurface;

				it = memorySurfaces.erase(it);

				break;
			}
		}
		LeaveCriticalSection(&smcs);

		if (emu)
		{
			ZeroMemory(emu->surfacepBits, emu->surfaceSize);

			PaletteUSN++;

			return DD_OK;
		}
	}

	// Create new emulated surface structure
	emu = new EMUSURFACE;

	LOG_LIMIT(100, __FUNCTION__ << " Creating new emulated surface (" << emu << ")");

	// Add some padding to surface size to a avoid overflow with some games
	DWORD padding = 200;

	// Create device context memory
	ZeroMemory(emu->bmiMemory, sizeof(emu->bmiMemory));
	emu->bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	emu->bmi->bmiHeader.biWidth = surfaceDesc2.dwWidth;
	emu->bmi->bmiHeader.biHeight = -(LONG)(surfaceDesc2.dwHeight + padding);
	emu->bmi->bmiHeader.biPlanes = 1;
	emu->bmi->bmiHeader.biBitCount = (WORD)surfaceBitCount;
	emu->bmi->bmiHeader.biCompression = (ColorMaskReq) ? BI_BITFIELDS : BI_RGB;
	emu->bmi->bmiHeader.biSizeImage = ((surfaceDesc2.dwWidth * surfaceBitCount + 31) & ~31) / 8 * surfaceDesc2.dwHeight;

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
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set bmi colors!");
		return DDERR_GENERIC;
	}

	emu->surfaceDC = CreateCompatibleDC(ddrawParent->GetDC());
	if (!emu->surfaceDC)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create compatible DC!");
		return DDERR_GENERIC;
	}
	emu->bitmap = CreateDIBSection(emu->surfaceDC, emu->bmi, (surfaceBitCount == 8) ? DIB_PAL_COLORS : DIB_RGB_COLORS, (void**)&emu->surfacepBits, nullptr, 0);
	emu->bmi->bmiHeader.biHeight = -(LONG)surfaceDesc2.dwHeight;
	if (!emu->bitmap)
	{
		emu->surfacepBits = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (surfaceDesc2.dwHeight + padding) * emu->surfacePitch);
	}
	else
	{
		emu->OldDCObject = SelectObject(emu->surfaceDC, emu->bitmap);
		if (!emu->OldDCObject)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to replace object in DC!");
			return DDERR_GENERIC;
		}
	}
	emu->surfacePitch = ComputePitch(emu->bmi->bmiHeader.biWidth, emu->bmi->bmiHeader.biBitCount);
	emu->surfaceSize = surfaceDesc2.dwHeight * emu->surfacePitch;
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
		Logging::Log() << __FUNCTION__ << " Warning: surface still in use!";
	}

	// Store d3d9 surface texture
	if (BackupData && (surfaceTexture || surface3D))
	{
		Logging::LogDebug() << __FUNCTION__ << " Storing Direct3D9 texture surface data";
		do {
			D3DLOCKED_RECT LockRect = { NULL };
			HRESULT hr = (surfaceTexture) ? surfaceTexture->LockRect(0, &LockRect, nullptr, 0) :
				(surface3D) ? surface3D->LockRect(&LockRect, nullptr, 0) : DDERR_GENERIC;
			if (FAILED(hr))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock texture surface!");
				break;
			}

			size_t size = surfaceDesc2.dwHeight * LockRect.Pitch;

			surfaceBackup.resize(size);

			memcpy(&surfaceBackup[0], LockRect.pBits, size);

			(surfaceTexture) ? surfaceTexture->UnlockRect(0) :
				(surface3D) ? surface3D->UnlockRect() : DDERR_GENERIC;

		} while (false);
	}

	// Release DC
	if (LastDC && IsSurfaceInDC())
	{
		ReleaseDC(LastDC);
	}

	// Release d3d9 3D surface
	if (surface3D)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 surface";
		surface3D->Release();
		surface3D = nullptr;
	}

	// Release d3d9 context surface
	if (contextSurface)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 context surface";
		contextSurface->Release();
		contextSurface = nullptr;
	}

	// Release d3d9 surface texture
	if (surfaceTexture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 texture surface";
		surfaceTexture->Release();
		surfaceTexture = nullptr;
	}

	// Release d3d9 display surface texture
	if (displayTexture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 display texture surface";
		displayTexture->Release();
		displayTexture = nullptr;
	}

	// Release d3d9 palette surface texture
	if (paletteTexture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 palette texture surface";
		paletteTexture->Release();
		paletteTexture = nullptr;
	}

	// Release d3d9 pixel shader
	if (pixelShader)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 pixel shader";
		pixelShader->Release();
		pixelShader = nullptr;
	}

	// Release d3d9 vertex buffer
	if (vertexBuffer)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 vertext buffer";
		vertexBuffer->Release();
		vertexBuffer = nullptr;
	}

	// Emulated surface
	if (emu && !BackupData)
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

	// Clear locked rects
	surfaceLockRectList.clear();

	// Set flags
	IsInDC = false;
	IsLocked = false;

	// Set LastDC
	LastDC = nullptr;

	// Reset scanline flags
	LastLock.LockedRect.pBits = nullptr;
	LastLock.bEvenScanlines = false;
	LastLock.bOddScanlines = false;

	// Reset locked ID
	LockedWithID = 0;

	// Reset display flags
	if (ResetDisplayFlags)
	{
		surfaceDesc2.dwFlags &= ~ResetDisplayFlags;
	}
}

// Present surface
HRESULT m_IDirectDrawSurfaceX::PresentSurface(BOOL isSkipScene)
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
		// Set texture
		if (displayTexture && surfaceTexture)
		{
			if (FAILED((*d3d9Device)->UpdateTexture(surfaceTexture, displayTexture)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to update texture");
				hr = DDERR_GENERIC;
				break;
			}
			if (FAILED((*d3d9Device)->SetTexture(0, displayTexture)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set display texture");
				hr = DDERR_GENERIC;
				break;
			}
		}
		else if (surfaceTexture)
		{
			if (FAILED((*d3d9Device)->SetTexture(0, surfaceTexture)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set surface texture");
				hr = DDERR_GENERIC;
				break;
			}
		}

		// Set vertex buffer and lighting
		if (vertexBuffer)
		{
			// Set vertex shader
			if (FAILED((*d3d9Device)->SetVertexShader(nullptr)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set vertex shader");
				hr = DDERR_GENERIC;
				break;
			}

			// Set vertex format
			if (FAILED((*d3d9Device)->SetFVF(TLVERTEXFVF)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set the current vertex stream format");
				hr = DDERR_GENERIC;
				break;
			}

			// Set stream source
			if (FAILED((*d3d9Device)->SetStreamSource(0, vertexBuffer, 0, sizeof(TLVERTEX))))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set vertex buffer stream source");
				hr = DDERR_GENERIC;
				break;
			}

			// Set render states(no lighting)
			if (FAILED((*d3d9Device)->SetRenderState(D3DRS_LIGHTING, FALSE)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set device render state(no lighting)");
				hr = DDERR_GENERIC;
				break;
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
				hr = DDERR_GENERIC;
				break;
			}

			// Set pixel shader
			if (FAILED((*d3d9Device)->SetPixelShader(pixelShader)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set pixel shader");
				hr = DDERR_GENERIC;
				break;
			}
		}

		// Present to d3d9
		if (FAILED(ddrawParent->Present()))
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
	if (lpDDSurfaceDesc && !lpDestRect)
	{
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
}

// Restore removed scanlines before locking surface
void m_IDirectDrawSurfaceX::RestoreScanlines()
{
	DWORD ByteCount = surfaceBitCount / 8;
	DWORD RectWidth = LastLock.Rect.right - LastLock.Rect.left;
	DWORD RectHeight = LastLock.Rect.bottom - LastLock.Rect.top;

	if ((IsPrimarySurface() || IsBackBuffer()) && (LastLock.bEvenScanlines || LastLock.bOddScanlines) && LastLock.LockedRect.pBits &&
		ByteCount && ByteCount <= 4 && RectWidth == LastLock.ScanlineWidth)
	{
		DWORD size = RectWidth * ByteCount;
		BYTE* DestBuffer = (BYTE*)LastLock.LockedRect.pBits;

		// Restore even scanlines
		if (LastLock.bEvenScanlines)
		{
			constexpr DWORD Starting = 0;
			DestBuffer += LastLock.LockedRect.Pitch * Starting;

			for (DWORD y = Starting; y < RectHeight; y = y + 2)
			{
				memcpy(DestBuffer, &LastLock.EvenScanLine[0], size);
				DestBuffer += LastLock.LockedRect.Pitch * 2;
			}
		}
		// Restore odd scanlines
		else if (LastLock.bOddScanlines)
		{
			constexpr DWORD Starting = 1;
			DestBuffer += LastLock.LockedRect.Pitch * Starting;

			for (DWORD y = Starting; y < RectHeight; y = y + 2)
			{
				memcpy(DestBuffer, &LastLock.OddScanLine[0], size);
				DestBuffer += LastLock.LockedRect.Pitch * 2;
			}
		}
	}
}

// Remove scanlines before unlocking surface
void m_IDirectDrawSurfaceX::RemoveScanlines()
{
	DWORD ByteCount = surfaceBitCount / 8;
	DWORD RectWidth = LastLock.Rect.right - LastLock.Rect.left;
	DWORD RectHeight = LastLock.Rect.bottom - LastLock.Rect.top;

	// Reset scanline flags
	bool LastSet = (LastLock.bEvenScanlines || LastLock.bOddScanlines);
	LastLock.bOddScanlines = false;
	LastLock.bEvenScanlines = false;

	if ((IsPrimarySurface() || IsBackBuffer()) && LastLock.LockedRect.pBits &&
		ByteCount && ByteCount <= 4 && RectHeight >= 100)
	{
		DWORD size = LastLock.ScanlineWidth * ByteCount;
		if (LastLock.EvenScanLine.size() < size || LastLock.OddScanLine.size() < size)
		{
			LastLock.EvenScanLine.resize(size);
			LastLock.OddScanLine.resize(size);
		}
		LastLock.ScanlineWidth = RectWidth;

		BYTE* DestBuffer = (BYTE*)LastLock.LockedRect.pBits;

		// Check if video has scanlines
		for (DWORD y = 0; y < RectHeight; y++)
		{
			// Check for even scanlines
			if (y % 2 == 0)
			{
				if (y == 0)
				{
					LastLock.bEvenScanlines = true;
					memcpy(&LastLock.EvenScanLine[0], DestBuffer, size);
				}
				else if (LastLock.bEvenScanlines)
				{
					LastLock.bEvenScanlines = (memcmp(&LastLock.EvenScanLine[0], DestBuffer, size) == 0);
				}
			}
			// Check for odd scanlines
			else
			{
				if (y == 1)
				{
					LastLock.bOddScanlines = true;
					memcpy(&LastLock.OddScanLine[0], DestBuffer, size);
				}
				else if (LastLock.bOddScanlines)
				{
					LastLock.bOddScanlines = (memcmp(&LastLock.OddScanLine[0], DestBuffer, size) == 0);
				}
			}
			// Exit if no scanlines found
			if (!LastLock.bOddScanlines && !LastLock.bEvenScanlines)
			{
				break;
			}
			DestBuffer += LastLock.LockedRect.Pitch;
		}

		// If all scanlines are set then do nothing
		if (!LastSet && LastLock.bEvenScanlines && LastLock.bOddScanlines)
		{
			LastLock.bEvenScanlines = false;
			LastLock.bOddScanlines = false;
		}

		// Reset destination buffer
		DestBuffer = (BYTE*)LastLock.LockedRect.pBits;

		// Double even scanlines
		if (LastLock.bEvenScanlines)
		{
			constexpr DWORD Starting = 0;
			DestBuffer += LastLock.LockedRect.Pitch * Starting;

			for (DWORD y = Starting; y < RectHeight - 1; y = y + 2)
			{
				memcpy(DestBuffer, DestBuffer + LastLock.LockedRect.Pitch, size);
				DestBuffer += LastLock.LockedRect.Pitch * 2;
			}
		}
		// Double odd scanlines
		else if (LastLock.bOddScanlines)
		{
			constexpr DWORD Starting = 1;
			DestBuffer += LastLock.LockedRect.Pitch * Starting;

			for (DWORD y = Starting; y < RectHeight; y = y + 2)
			{
				memcpy(DestBuffer, DestBuffer - LastLock.LockedRect.Pitch, size);
				DestBuffer += LastLock.LockedRect.Pitch * 2;
			}
		}
	}
}

// Lock the d3d9 surface
HRESULT m_IDirectDrawSurfaceX::SetLock(D3DLOCKED_RECT* pLockedRect, LPRECT lpDestRect, DWORD dwFlags, BOOL isSkipScene)
{
	if (!pLockedRect)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Check for device interface
	HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
	if (FAILED(c_hr))
	{
		return c_hr;
	}

	// Check if the scene needs to be presented
	isSkipScene |= ((lpDestRect) ? (abs(lpDestRect->bottom - lpDestRect->top) < 2 || abs(lpDestRect->right - lpDestRect->left) < 2) : FALSE);

	// Present before write if needed
	BeginWritePresent(isSkipScene);

	// Emulated surface
	if (IsSurfaceEmulated)
	{
		// Set locked rect
		if (FAILED(LockEmulatedSurface(pLockedRect, lpDestRect)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock emulated surface!");
			return DDERR_GENERIC;
		}

		// Backup last rect
		CheckCoordinates(&LastLock.Rect, lpDestRect);
		DoCopyRect = ((dwFlags & D3DLOCK_READONLY) == 0);

		// Set new palette data
		UpdatePaletteData();

		// Read surface from GDI
		if (Config.DdrawReadFromGDI && (IsPrimarySurface() || IsBackBuffer()))
		{
			CopyEmulatedSurfaceFromGDI(LastLock.Rect);
		}
	}
	// Lock surface texture
	else if (surfaceTexture)
	{
		// Try to lock the rect
		HRESULT hr = surfaceTexture->LockRect(0, pLockedRect, lpDestRect, dwFlags);
		if (FAILED(hr))
		{
			while (!(dwFlags & D3DLOCK_DONOTWAIT) && LockedWithID && LockedWithID != GetCurrentThreadId())
			{
				Sleep(0);
				if (!surfaceTexture)
				{
					return DDERR_GENERIC;
				}
			}
			hr = surfaceTexture->LockRect(0, pLockedRect, lpDestRect, dwFlags);
			if (FAILED(hr))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock surface texture");
				return (hr == D3DERR_WASSTILLDRAWING || (LockedWithID && (dwFlags & D3DLOCK_DONOTWAIT))) ? DDERR_WASSTILLDRAWING :
					DDERR_GENERIC;
			}
		}
	}
	// Lock 3D surface
	else if (surface3D)
	{
		// Try to lock the rect
		HRESULT hr = surface3D->LockRect(pLockedRect, lpDestRect, dwFlags);
		while (!(dwFlags & D3DLOCK_DONOTWAIT) && LockedWithID && LockedWithID != GetCurrentThreadId())
		{
			Sleep(0);
			if (!surface3D)
			{
				return DDERR_GENERIC;
			}
		}
		hr = surface3D->LockRect(pLockedRect, lpDestRect, dwFlags);
		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock surface");
			return (hr == D3DERR_WASSTILLDRAWING || (LockedWithID && (dwFlags & D3DLOCK_DONOTWAIT))) ? DDERR_WASSTILLDRAWING :
				DDERR_GENERIC;
		}
	}
	else
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
		return DDERR_GENERIC;
	}

	// Set locked ID
	LockedWithID = GetCurrentThreadId();

	// Backup last rect before removing scanlines
	CheckCoordinates(&LastLock.Rect, lpDestRect);
	LastLock.LockedRect.pBits = pLockedRect->pBits;
	LastLock.LockedRect.Pitch = pLockedRect->Pitch;

	// Restore scanlines before returing surface memory
	if (Config.DdrawRemoveScanlines)
	{
		RestoreScanlines();
	}

	// Set lock flag
	IsLocked = true;

	// Check pitch
	if ((surfaceDesc2.dwFlags & DDSD_PITCH) && surfaceDesc2.lPitch != pLockedRect->Pitch)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: surface pitch changed size: " << surfaceDesc2.lPitch << " --> " << pLockedRect->Pitch);
	}

	// Set pitch
	surfaceDesc2.dwFlags |= DDSD_PITCH;
	surfaceDesc2.lPitch = pLockedRect->Pitch;

	// Set dirty flag
	if (!(dwFlags & D3DLOCK_READONLY))
	{
		SetDirtyFlag();
	}

	// Success
	return DD_OK;
}

// Unlock the d3d9 surface
HRESULT m_IDirectDrawSurfaceX::SetUnlock(BOOL isSkipScene)
{
	// Check for device interface
	HRESULT c_hr = CheckInterface(__FUNCTION__, true, true);
	if (FAILED(c_hr))
	{
		return c_hr;
	}

	// Remove scanlines before unlocking surface
	if (Config.DdrawRemoveScanlines)
	{
		RemoveScanlines();
	}

	HRESULT hr = DD_OK;

	// Emulated surface
	if (IsSurfaceEmulated)
	{
		// Blt surface directly to GDI
		if (Config.DdrawWriteToGDI && (IsPrimarySurface() || IsBackBuffer()))
		{
			CopyEmulatedSurfaceToGDI(LastLock.Rect);
		}
		// Copy emulated surface to real texture
		else if (DoCopyRect)
		{
			CopyEmulatedSurface(&LastLock.Rect, true);

			// Reset copy flag
			DoCopyRect = false;
		}
		hr = DD_OK;
	}
	// Lock surface texture
	else if (surfaceTexture)
	{
		hr = surfaceTexture->UnlockRect(0);
		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to unlock surface texture");
			hr = (hr == D3DERR_INVALIDCALL) ? DDERR_GENERIC :
				(hr == D3DERR_WASSTILLDRAWING) ? DDERR_WASSTILLDRAWING :
				DDERR_SURFACELOST;
		}
	}
	// Lock 3D surface
	else if (surface3D)
	{
		hr = surface3D->UnlockRect();
		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to unlock surface");
			hr = (hr == D3DERR_INVALIDCALL) ? DDERR_GENERIC :
				(hr == D3DERR_WASSTILLDRAWING) ? DDERR_WASSTILLDRAWING :
				DDERR_SURFACELOST;
		}
	}
	else
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
		hr = DDERR_GENERIC;
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
	if (SUCCEEDED(hr))
	{
		EndWritePresent(isSkipScene);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::LockEmulatedSurface(D3DLOCKED_RECT* pLockedRect, LPRECT lpDestRect)
{
	if (!pLockedRect || !emu || !emu->surfacepBits)
	{
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

inline void m_IDirectDrawSurfaceX::BeginWritePresent(bool isSkipScene)
{
	// Check if data needs to be presented before write
	if (dirtyFlag)
	{
		if (SUCCEEDED(PresentSurface(isSkipScene)))
		{
			PresentOnUnlock = true;
		}
	}
}

inline void m_IDirectDrawSurfaceX::EndWritePresent(bool isSkipScene)
{
	// Present surface
	if (!PresentOnUnlock)
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
	for (auto it : AttachedSurfaceMap)
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
	if (FAILED(c_hr))
	{
		return c_hr;
	}

	HRESULT hr = DD_OK;
	bool UnlockDest = false;

	do {
		// Check and copy rect
		RECT DestRect = {};
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
		DWORD ByteCount = surfaceBitCount / 8;
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
		BYTE *SrcBuffer = (BYTE*)&dwFillColor;
		BYTE *DestBuffer = (BYTE*)DestLockRect.pBits;

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

	} while (false);

	// Unlock surfaces if needed
	if (UnlockDest)
	{
		SetUnlock(true);
	}

	return hr;
}

void vmemcpy(void* _Dst, void const* _Src, size_t _Width, D3DFORMAT SrcFormat, D3DFORMAT DestFormat)
{
	// Error checking
	if (!_Dst || !_Src || !_Width)
	{
		return;
	}
	if (SrcFormat == D3DFMT_R5G6B5 && (DestFormat == D3DFMT_A8R8G8B8 || DestFormat == D3DFMT_X8R8G8B8))
	{
		WORD *SrcAddr = (WORD*)_Src;
		BYTE *DstAddr = (BYTE*)_Dst;
		for (UINT x = 0; x < _Width; x++)
		{
			DstAddr[0] = D3DCOLOR_R5G6B5_BLUE(*SrcAddr);
			DstAddr[1] = D3DCOLOR_R5G6B5_GREEN(*SrcAddr);
			DstAddr[2] = D3DCOLOR_R5G6B5_RED(*SrcAddr);
			SrcAddr += 1;
			DstAddr += 4;
		}
	}
	else
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: not supported for specified source and destination formats! " << SrcFormat << "-->" << DestFormat);
	}
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
	if (FAILED(c_hr) || FAILED(pSourceSurface->CheckInterface(__FUNCTION__, true, true)))
	{
		return FAILED(c_hr) ? c_hr : DDERR_GENERIC;
	}

	HRESULT hr = DD_OK;
	bool UnlockSrc = false, UnlockDest = false;
	bool IsCriticalSectionSet = false;

	do {
		// Get source and dest format
		D3DFORMAT SrcFormat = pSourceSurface->GetSurfaceFormat();
		D3DFORMAT DestFormat = GetSurfaceFormat();

		// Get source and dest rect
		RECT SrcRect = { 0, 0, (LONG)pSourceSurface->GetWidth(), (LONG)pSourceSurface->GetHeight() };
		RECT DestRect = { 0, 0, (LONG)surfaceDesc2.dwWidth, (LONG)surfaceDesc2.dwHeight };

		if (pSourceRect)
		{
			SrcRect.left = pSourceRect->left;
			SrcRect.top = pSourceRect->top;
			SrcRect.right = pSourceRect->right;
			SrcRect.bottom = pSourceRect->bottom;
		}

		if (pDestRect)
		{
			DestRect.left = pDestRect->left;
			DestRect.top = pDestRect->top;
			DestRect.right = pDestRect->right;
			DestRect.bottom = pDestRect->bottom;
		}

		// Get copy flags
		bool IsStretchRect = (abs((DestRect.right - DestRect.left) - (SrcRect.right - SrcRect.left)) > 1 || abs((DestRect.bottom - DestRect.top) - (SrcRect.bottom - SrcRect.top)) > 1);
		bool IsColorKey = ((dwFlags & DDBLT_KEYDEST) != 0);
		bool IsMirrorLeftRight = ((dwFlags & DDBLTFX_MIRRORLEFTRIGHT) != 0);
		bool IsMirrorUpDown = ((dwFlags & DDBLTFX_MIRRORUPDOWN) != 0);
		bool UseQuickCopy = (!IsStretchRect && !IsColorKey && !IsMirrorLeftRight);

		// Check and copy rect and do clipping
		if (!pSourceSurface->CheckCoordinates(&SrcRect, pSourceRect) || !CheckCoordinates(&DestRect, pDestRect))
		{
			hr = DDERR_INVALIDRECT;
			break;
		}

		// Use D3DXLoadSurfaceFromSurface to copy the surface
		if (!Config.DdrawReadFromGDI && !Config.DdrawWriteToGDI &&
			!pSourceSurface->IsUsingEmulation() && !IsUsingEmulation() &&
			SrcFormat != D3DFMT_P8 && DestFormat != D3DFMT_P8 &&
			!IsMirrorLeftRight && !IsMirrorUpDown && !IsColorKey)
		{
			IDirect3DSurface9* pSourceSurfaceD9 = pSourceSurface->GetD3D9Surface();
			IDirect3DSurface9* pDestSurfaceD9 = GetD3D9Surface();

			if (pSourceSurfaceD9 && pDestSurfaceD9)
			{
				DWORD DX3XFilter =
					(Filter & D3DTEXF_LINEAR) ? D3DX_FILTER_LINEAR :
					(Filter & D3DTEXF_POINT) ? D3DX_FILTER_POINT :
					(IsStretchRect) ? D3DX_FILTER_LINEAR :
					D3DX_FILTER_POINT;

				HRESULT s_hr = D3DXLoadSurfaceFromSurface(pDestSurfaceD9, nullptr, &DestRect, pSourceSurfaceD9, nullptr, &SrcRect, DX3XFilter, 0);

				if (SUCCEEDED(s_hr))
				{
					hr = DD_OK;
					break;
				}
			}
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

		// Check source and destination format
		bool FormatMismatch = false;
		if (SrcFormat == D3DFMT_R5G6B5 && (DestFormat == D3DFMT_A8R8G8B8 || DestFormat == D3DFMT_X8R8G8B8))
		{
			FormatMismatch = true;
		}
		else if (!(SrcFormat == DestFormat ||
			((SrcFormat == D3DFMT_A1R5G5B5 || SrcFormat == D3DFMT_X1R5G5B5) && (DestFormat == D3DFMT_A1R5G5B5 || DestFormat == D3DFMT_X1R5G5B5)) ||
			((SrcFormat == D3DFMT_A4R4G4B4 || SrcFormat == D3DFMT_X4R4G4B4) && (DestFormat == D3DFMT_A4R4G4B4 || DestFormat == D3DFMT_X4R4G4B4)) ||
			((SrcFormat == D3DFMT_A8R8G8B8 || SrcFormat == D3DFMT_X8R8G8B8) && (DestFormat == D3DFMT_A8R8G8B8 || DestFormat == D3DFMT_X8R8G8B8)) ||
			((SrcFormat == D3DFMT_A8B8G8R8 || SrcFormat == D3DFMT_X8B8G8R8) && (DestFormat == D3DFMT_A8B8G8R8 || DestFormat == D3DFMT_X8B8G8R8))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: not supported for specified source and destination formats! " << SrcFormat << "-->" << DestFormat);
			hr = DDERR_GENERIC;
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
		D3DLOCKED_RECT SrcLockRect;
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
			IsCriticalSectionSet = true;
			EnterCriticalSection(&ddscs);

			size_t size = SrcRectWidth * ByteCount * SrcRectHeight;
			if (size > surfaceArray.size())
			{
				surfaceArray.resize(size);
			}
			BYTE *SrcBuffer = (BYTE*)SrcLockRect.pBits;
			BYTE *DestBuffer = (BYTE*)&surfaceArray[0];
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
				pSourceSurface->SetUnlock(true);
				UnlockSrc = false;
			}
		}

		// Check if destination surface is not locked then lock it
		D3DLOCKED_RECT DestLockRect;
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
		INT DestPitch = DestLockRect.Pitch;
		if (IsMirrorUpDown)
		{
			DestPitch = -DestLockRect.Pitch;
			DestBuffer += DestLockRect.Pitch * (DestRectHeight - 1);
		}

		// Copy memory (simple)
		if (UseQuickCopy)
		{
			if (FormatMismatch)
			{
				for (LONG y = 0; y < DestRectHeight; y++)
				{
					vmemcpy(DestBuffer, SrcBuffer, DestRectWidth, SrcFormat, DestFormat);
					SrcBuffer += SrcLockRect.Pitch;
					DestBuffer += DestPitch;
				}
			}
			else if (SrcLockRect.Pitch == DestLockRect.Pitch && (LONG)ComputePitch(DestRectWidth, DestBitCount) == DestPitch)
			{
				memcpy(DestBuffer, SrcBuffer, DestRectHeight * DestLockRect.Pitch);
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

		// Get ratio
		float WidthRatio = (float)SrcRectWidth / (float)DestRectWidth;
		float HeightRatio = (float)SrcRectHeight / (float)DestRectHeight;

		// Set color variables
		DWORD ByteMask = (ByteCount == 1) ? 0x000000FF : (ByteCount == 2) ? 0x0000FFFF : (ByteCount == 3) ? 0x00FFFFFF : 0xFFFFFFFF;
		DWORD ColorKeyLow = ColorKey.dwColorSpaceLowValue & ByteMask;
		DWORD ColorKeyHigh = ColorKey.dwColorSpaceHighValue & ByteMask;

		// Source byte count
		DWORD SrcByteCount = (FormatMismatch) ? GetBitCount(SrcFormat) / 8 : ByteCount;

		// Copy memory (complex)
		for (LONG y = 0; y < DestRectHeight; y++)
		{
			BYTE *LoopBuffer = DestBuffer;
			for (LONG x = 0; x < DestRectWidth; x++)
			{
				DWORD r = (IsStretchRect) ? (DWORD)((float)x * WidthRatio) : x;
				BYTE *NewPixel = (IsMirrorLeftRight) ? SrcBuffer + ((SrcRectWidth - r - 1) * SrcByteCount) : SrcBuffer + (r * SrcByteCount);
				DWORD PixelColor = (*(DWORD*)NewPixel) & ByteMask;

				if (!IsColorKey || PixelColor < ColorKeyLow || PixelColor > ColorKeyHigh)
				{
					if (FormatMismatch && SrcFormat == D3DFMT_R5G6B5)
					{
						LoopBuffer[0] = D3DCOLOR_R5G6B5_BLUE(*(WORD*)NewPixel);
						LoopBuffer[1] = D3DCOLOR_R5G6B5_GREEN(*(WORD*)NewPixel);
						LoopBuffer[2] = D3DCOLOR_R5G6B5_RED(*(WORD*)NewPixel);
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

	// Leave Critical Section if needed
	if (IsCriticalSectionSet)
	{
		LeaveCriticalSection(&ddscs);
	}

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

// Copy from emulated surface
HRESULT m_IDirectDrawSurfaceX::CopyEmulatedSurface(LPRECT lpDestRect, bool CopyToRealSurface)
{
	if (!emu || !emu->surfaceDC || !emu->surfacepBits)
	{
		return DDERR_GENERIC;
	}

	HRESULT hr = DD_OK;

	do {
		// Update rect
		RECT DestRect = {};
		if (!CheckCoordinates(&DestRect, lpDestRect))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid rect!");
			hr = DDERR_INVALIDRECT;
			break;
		}

		UpdatePaletteData();

		// Check if destination surface is not locked then lock it
		D3DLOCKED_RECT EmulatedLockRect, SurfaceLockRect = { NULL };
		if (surfaceTexture)
		{
			surfaceTexture->LockRect(0, &SurfaceLockRect, &DestRect, (!CopyToRealSurface) ? D3DLOCK_READONLY : 0);
		}
		else if (surface3D)
		{
			surface3D->LockRect(&SurfaceLockRect, &DestRect, (!CopyToRealSurface) ? D3DLOCK_READONLY : 0);
		}
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
			hr = DDERR_GENERIC;
			break;
		}
		if (FAILED(LockEmulatedSurface(&EmulatedLockRect, &DestRect)) || FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock surface!");
			hr = DDERR_GENERIC;
			break;
		}

		// Create buffer variables
		BYTE *EmulatedBuffer = (BYTE*)EmulatedLockRect.pBits;
		BYTE *SurfaceBuffer = (BYTE*)SurfaceLockRect.pBits;
		INT WidthPitch = (surfaceBitCount / 8) * (DestRect.right - DestRect.left);
		LONG RectHeight = (DestRect.bottom - DestRect.top);

		// Copy data
		switch ((DWORD)surfaceFormat)
		{
		case D3DFMT_B8G8R8:
			if (CopyToRealSurface)
			{
				for (LONG x = DestRect.top; x < DestRect.bottom; x++)
				{
					TRIBYTE* EmulatedBufferLoop = (TRIBYTE*)EmulatedBuffer;
					DWORD* SurfaceBufferLoop = (DWORD*)SurfaceBuffer;
					for (LONG y = DestRect.left; y < DestRect.right; y++)
					{
						*SurfaceBufferLoop = D3DFMT_A8B8G8R8_COPY(*(DWORD*)EmulatedBufferLoop);
						EmulatedBufferLoop++;
						SurfaceBufferLoop++;
					}
					EmulatedBuffer += EmulatedLockRect.Pitch;
					SurfaceBuffer += SurfaceLockRect.Pitch;
				}
			}
			else
			{
				for (LONG x = DestRect.top; x < DestRect.bottom; x++)
				{
					TRIBYTE* EmulatedBufferLoop = (TRIBYTE*)EmulatedBuffer;
					DWORD* SurfaceBufferLoop = (DWORD*)SurfaceBuffer;
					for (LONG y = DestRect.left; y < DestRect.right; y++)
					{
						DWORD Pixel = D3DFMT_A8B8G8R8_COPY(*(DWORD*)SurfaceBufferLoop);
						*EmulatedBufferLoop = *(TRIBYTE*)&Pixel;
						EmulatedBufferLoop++;
						SurfaceBufferLoop++;
					}
					EmulatedBuffer += EmulatedLockRect.Pitch;
					SurfaceBuffer += SurfaceLockRect.Pitch;
				}
			}
			break;
		case D3DFMT_R8G8B8:
			if (CopyToRealSurface)
			{
				for (LONG x = DestRect.top; x < DestRect.bottom; x++)
				{
					TRIBYTE *EmulatedBufferLoop = (TRIBYTE*)EmulatedBuffer;
					DWORD *SurfaceBufferLoop = (DWORD*)SurfaceBuffer;
					for (LONG y = DestRect.left; y < DestRect.right; y++)
					{
						*SurfaceBufferLoop = *(DWORD*)EmulatedBufferLoop;
						EmulatedBufferLoop++;
						SurfaceBufferLoop++;
					}
					EmulatedBuffer += EmulatedLockRect.Pitch;
					SurfaceBuffer += SurfaceLockRect.Pitch;
				}
			}
			else
			{
				for (LONG x = DestRect.top; x < DestRect.bottom; x++)
				{
					TRIBYTE *EmulatedBufferLoop = (TRIBYTE*)EmulatedBuffer;
					DWORD *SurfaceBufferLoop = (DWORD*)SurfaceBuffer;
					for (LONG y = DestRect.left; y < DestRect.right; y++)
					{
						*EmulatedBufferLoop = *(TRIBYTE*)SurfaceBufferLoop;
						EmulatedBufferLoop++;
						SurfaceBufferLoop++;
					}
					EmulatedBuffer += EmulatedLockRect.Pitch;
					SurfaceBuffer += SurfaceLockRect.Pitch;
				}
			}
			break;
		case D3DFMT_X4R4G4B4:
		case D3DFMT_A4R4G4B4:
			if (CopyToRealSurface)
			{
				for (LONG x = DestRect.top; x < DestRect.bottom; x++)
				{
					BYTE* EmulatedBufferLoop = (BYTE*)EmulatedBuffer;
					BYTE* SurfaceBufferLoop = (BYTE*)SurfaceBuffer;
					for (LONG y = DestRect.left; y < DestRect.right; y++)
					{
						SurfaceBufferLoop[0] = (BYTE)((EmulatedBufferLoop[0] >> 4) * 17);
						SurfaceBufferLoop[1] = (BYTE)((EmulatedBufferLoop[0] & 0xF) * 17);
						SurfaceBufferLoop[2] = (BYTE)((EmulatedBufferLoop[1] >> 4) * 17);
						SurfaceBufferLoop[3] = (BYTE)((EmulatedBufferLoop[1] & 0xF) * 17);
						EmulatedBufferLoop += 2;
						SurfaceBufferLoop += 4;
					}
					EmulatedBuffer += EmulatedLockRect.Pitch;
					SurfaceBuffer += SurfaceLockRect.Pitch;
				}
			}
			else
			{
				for (LONG x = DestRect.top; x < DestRect.bottom; x++)
				{
					WORD* EmulatedBufferLoop = (WORD*)EmulatedBuffer;
					BYTE* SurfaceBufferLoop = (BYTE*)SurfaceBuffer;
					for (LONG y = DestRect.left; y < DestRect.right; y++)
					{
						*EmulatedBufferLoop = 
							(WORD)((SurfaceBufferLoop[0] / 17) << 12) +
							(WORD)((SurfaceBufferLoop[1] / 17) << 8) +
							(WORD)((SurfaceBufferLoop[2] / 17) << 4) +
							(WORD)((SurfaceBufferLoop[3] / 17));
						EmulatedBufferLoop++;
						SurfaceBufferLoop += 4;
					}
					EmulatedBuffer += EmulatedLockRect.Pitch;
					SurfaceBuffer += SurfaceLockRect.Pitch;
				}
			}
			break;
		case D3DFMT_X8B8G8R8:
		case D3DFMT_A8B8G8R8:
			if (CopyToRealSurface)
			{
				for (LONG x = DestRect.top; x < DestRect.bottom; x++)
				{
					DWORD* EmulatedBufferLoop = (DWORD*)EmulatedBuffer;
					DWORD* SurfaceBufferLoop = (DWORD*)SurfaceBuffer;
					for (LONG y = DestRect.left; y < DestRect.right; y++)
					{
						*SurfaceBufferLoop = D3DFMT_A8B8G8R8_COPY(*(DWORD*)EmulatedBufferLoop);
						EmulatedBufferLoop++;
						SurfaceBufferLoop++;
					}
					EmulatedBuffer += EmulatedLockRect.Pitch;
					SurfaceBuffer += SurfaceLockRect.Pitch;
				}
			}
			else
			{
				for (LONG x = DestRect.top; x < DestRect.bottom; x++)
				{
					DWORD* EmulatedBufferLoop = (DWORD*)EmulatedBuffer;
					DWORD* SurfaceBufferLoop = (DWORD*)SurfaceBuffer;
					for (LONG y = DestRect.left; y < DestRect.right; y++)
					{
						*EmulatedBufferLoop = D3DFMT_A8B8G8R8_COPY(*(DWORD*)SurfaceBufferLoop);
						EmulatedBufferLoop++;
						SurfaceBufferLoop++;
					}
					EmulatedBuffer += EmulatedLockRect.Pitch;
					SurfaceBuffer += SurfaceLockRect.Pitch;
				}
			}
			break;
		default:
			if (CopyToRealSurface)
			{
				if (SurfaceLockRect.Pitch == EmulatedLockRect.Pitch && (LONG)ComputePitch(DestRect.right - DestRect.left, surfaceBitCount) == SurfaceLockRect.Pitch)
				{
					memcpy(SurfaceBuffer, EmulatedBuffer, SurfaceLockRect.Pitch * RectHeight);
				}
				else
				{
					for (LONG x = DestRect.top; x < DestRect.bottom; x++)
					{
						memcpy(SurfaceBuffer, EmulatedBuffer, WidthPitch);
						EmulatedBuffer += EmulatedLockRect.Pitch;
						SurfaceBuffer += SurfaceLockRect.Pitch;
					}
				}
			}
			else
			{
				if (SurfaceLockRect.Pitch == EmulatedLockRect.Pitch && (LONG)ComputePitch(DestRect.right - DestRect.left, surfaceBitCount) == EmulatedLockRect.Pitch)
				{
					memcpy(EmulatedBuffer, SurfaceBuffer, EmulatedLockRect.Pitch * RectHeight);
				}
				else
				{
					for (LONG x = DestRect.top; x < DestRect.bottom; x++)
					{
						memcpy(EmulatedBuffer, SurfaceBuffer, WidthPitch);
						EmulatedBuffer += EmulatedLockRect.Pitch;
						SurfaceBuffer += SurfaceLockRect.Pitch;
					}
				}
			}
		}

		// Unlock surface texture
		if (surfaceTexture)
		{
			surfaceTexture->UnlockRect(0);
		}
		else if (surface3D)
		{
			surface3D->UnlockRect();
		}

	} while (false);

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::CopyEmulatedSurfaceFromGDI(RECT Rect)
{
	if (!emu || !emu->surfaceDC || !ddrawParent->GetDC())
	{
		return DDERR_GENERIC;
	}

	RECT WindowRect = { NULL };
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

	BitBlt(emu->surfaceDC, Left, Top, Width, Height, ddrawParent->GetDC(), Rect.left, Rect.top, SRCCOPY);

	return DD_OK;
}

HRESULT m_IDirectDrawSurfaceX::CopyEmulatedSurfaceToGDI(RECT Rect)
{
	if (!emu || !emu->surfaceDC || !ddrawParent->GetDC())
	{
		return DDERR_GENERIC;
	}

	RECT WindowRect = { NULL };
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

	BitBlt(ddrawParent->GetDC(), Left, Top, Width, Height, emu->surfaceDC, Rect.left, Rect.top, SRCCOPY);

	return DD_OK;
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
		if (CurrentPaletteUSN && CurrentPaletteUSN != LastPaletteUSN)
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
				if (CurrentPaletteUSN && ((CurrentPaletteUSN != LastPaletteUSN) || (emu && CurrentPaletteUSN != emu->LastPaletteUSN)))
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
	if (emu && emu->surfaceDC)
	{
		SetDIBColorTable(emu->surfaceDC, 0, entryCount, (RGBQUAD*)rgbPalette);

		emu->LastPaletteUSN = CurrentPaletteUSN;
	}

	// If new palette data then write it to texture
	if (paletteTexture)
	{
		do {
			D3DLOCKED_RECT LockRect;
			RECT Rect = { 0, 0, (LONG)entryCount, 1 };

			if (FAILED(paletteTexture->LockRect(0, &LockRect, &Rect, 0)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock palette texture!");
				break;
			}

			memcpy(LockRect.pBits, rgbPalette, entryCount * sizeof(D3DCOLOR));

			paletteTexture->UnlockRect(0);

		} while (false);
	}

	LastPaletteUSN = CurrentPaletteUSN;
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
	ppEmuSurface = nullptr;
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
	for (EMUSURFACE *pEmuSurface: memorySurfaces)
	{
		DeleteEmulatedMemory(&pEmuSurface);
	}
	memorySurfaces.clear();
}
