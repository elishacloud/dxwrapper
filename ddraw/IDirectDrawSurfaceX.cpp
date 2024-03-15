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
*
* Code initial taken from: https://github.com/strangebytes/diablo-ddrawwrapper
*
* Code to create emulated surface taken from: https://github.com/CnCNet/cnc-ddraw
*/

#include <sstream>
#include "ddraw.h"
#include "d3dx9.h"
#include "Utils\Utils.h"

// Used to allow presenting non-primary surfaces in case the primary surface present fails
bool dirtyFlag = false;
bool SceneReady = false;
bool IsPresentRunning = false;

// Cached surface wrapper interface v1 list
std::vector<m_IDirectDrawSurface*> SurfaceWrapperListV1;

// Used for sharing emulated memory
bool ShareEmulatedMemory = false;
std::vector<EMUSURFACE*> memorySurfaces;

/************************/
/*** IUnknown methods ***/
/************************/

HRESULT m_IDirectDrawSurfaceX::QueryInterface(REFIID riid, LPVOID FAR* ppvObj, DWORD DirectXVersion)
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
		return E_NOINTERFACE;
	}

	DWORD DxVersion = (CheckWrapperType(riid) && (Config.Dd7to9 || Config.ConvertToDirectDraw7)) ? GetGUIDVersion(riid) : DirectXVersion;

	bool IsD3DDevice = (riid == IID_IDirect3DHALDevice || riid == IID_IDirect3DTnLHalDevice ||
		riid == IID_IDirect3DRGBDevice || riid == IID_IDirect3DRampDevice || riid == IID_IDirect3DMMXDevice ||
		riid == IID_IDirect3DRefDevice || riid == IID_IDirect3DNullDevice);

	if (riid == GetWrapperType(DxVersion) || riid == IID_IUnknown)
	{
		*ppvObj = GetWrapperInterfaceX(DxVersion);

		AddRef(DxVersion);

		return DD_OK;
	}

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, false, false)))
		{
			return E_NOINTERFACE;
		}

		if (IsD3DDevice)
		{
			DxVersion = (DxVersion == 4) ? 3 : DxVersion;

			m_IDirect3DDeviceX* D3DDeviceX = *ddrawParent->GetCurrentD3DDevice();

			if (D3DDeviceX)
			{
				*ppvObj = D3DDeviceX->GetWrapperInterfaceX(DxVersion);

				D3DDeviceX->AddRef(DxVersion);

				return DD_OK;
			}

			m_IDirect3DX* D3DX = *ddrawParent->GetCurrentD3D();

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
			return SUCCEEDED(ddrawParent->CreateColorInterface(ppvObj)) ? DD_OK : E_NOINTERFACE;
		}
		if (riid == IID_IDirectDrawGammaControl)
		{
			return SUCCEEDED(ddrawParent->CreateGammaInterface(ppvObj)) ? DD_OK : E_NOINTERFACE;
		}
	}

	if (Config.ConvertToDirect3D7 && (riid == IID_IDirect3DTexture || riid == IID_IDirect3DTexture2) && ddrawParent)
	{
		DxVersion = GetGUIDVersion(riid);

		m_IDirect3DTextureX* InterfaceX = nullptr;

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

	if (Config.ConvertToDirect3D7 && IsD3DDevice && ddrawParent)
	{
		HRESULT hr = E_NOINTERFACE;

		m_IDirect3DDeviceX** lplpD3DDevice = ddrawParent->GetCurrentD3DDevice();
		if (lplpD3DDevice && *lplpD3DDevice)
		{
			hr = DD_OK;
		}

		if (FAILED(hr))
		{
			m_IDirect3DX** lplpD3D = ddrawParent->GetCurrentD3D();
			if (lplpD3D && *lplpD3D)
			{

				hr = (*lplpD3D)->CreateDevice(IID_IDirect3DHALDevice,
					(LPDIRECTDRAWSURFACE7)GetWrapperInterfaceX(DirectXVersion),
					(LPDIRECT3DDEVICE7*)ppvObj,
					nullptr,
					(DirectXVersion != 4) ? DirectXVersion : 3);
			}
		}

		if (SUCCEEDED(hr))
		{
			m_IDirect3DDeviceX* lpD3DDeviceX = nullptr;

			((IDirect3DDevice7*)*ppvObj)->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpD3DDeviceX);

			if (lpD3DDeviceX)
			{
				lpD3DDeviceX->SetDdrawParent(ddrawParent);

				SetCriticalSection();
				ddrawParent->SetD3DDevice(lpD3DDeviceX, this);
				ReleaseCriticalSection();
			}
		}

		return SUCCEEDED(hr) ? DD_OK : E_NOINTERFACE;
	}

	HRESULT hr = ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DxVersion));

	if (IsD3DDevice && SUCCEEDED(hr))
	{
		if (DirectXVersion == 1)
		{
			*ppvObj = ProxyAddressLookupTable.FindAddress<m_IDirect3DDevice>(*ppvObj);
		}
		else if (DirectXVersion == 2)
		{
			*ppvObj = ProxyAddressLookupTable.FindAddress<m_IDirect3DDevice2>(*ppvObj);
		}
		else if (DirectXVersion == 3 || DirectXVersion == 4)
		{
			*ppvObj = ProxyAddressLookupTable.FindAddress<m_IDirect3DDevice3>(*ppvObj);
		}
		else
		{
			*ppvObj = ProxyAddressLookupTable.FindAddress<m_IDirect3DDevice7>(*ppvObj);
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
				// Handle cases where games use surface addresses after the surface is released (Final Liberation: Warhammer Epic 40,000)
				if (IsSurfaceBusy())
				{
					Logging::Log() << __FUNCTION__ << " Warning: surface still in use! Locked: " << IsSurfaceLocked() << " DC: " << IsSurfaceInDC() << " Blt: " << IsSurfaceBlitting();
					if (ddrawParent)
					{
						ddrawParent->AddReleasedSurfaceToVector(this);
					}
					ReleaseD9ContextSurface();
					ReleaseDirectDrawResources();
				}
				else
				{
					delete this;
				}
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
		HRESULT c_hr = CheckInterface(__FUNCTION__, false, false, false);
		if (FAILED(c_hr))
		{
			return c_hr;
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

HRESULT m_IDirectDrawSurfaceX::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx, bool DontPresentBlt)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Check if source Surface exists
	if (lpDDSrcSurface && !CheckSurfaceExists(lpDDSrcSurface))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not find source surface! " << lpDDSrcSurface);
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
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

		// Do supported raster operations
		if ((dwFlags & DDBLT_ROP) && (lpDDBltFx->dwROP != SRCCOPY && lpDDBltFx->dwROP != BLACKNESS && lpDDBltFx->dwROP == WHITENESS))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Raster operation Not Implemented " << Logging::hex(lpDDBltFx->dwROP));
			return DDERR_NORASTEROPHW;
		}

		// Typically, Blt returns immediately with an error if the bitbltter is busy and the bitblt could not be set up. Specify the DDBLT_WAIT flag to request a synchronous bitblt.
		const bool BltWait = ((dwFlags & DDBLT_WAIT) && (dwFlags & DDBLT_DONOTWAIT) == 0);

		// Check if the scene needs to be presented
		const bool IsSkipScene = (lpDestRect) ? CheckRectforSkipScene(*lpDestRect) : false;

		// Other flags, not yet implemented in dxwrapper
		// DDBLT_ASYNC - Current dxwrapper implementation never does async if calling from multiple threads

		// Get source surface
		m_IDirectDrawSurfaceX* lpDDSrcSurfaceX = nullptr;
		if (lpDDSrcSurface)
		{
			lpDDSrcSurface->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpDDSrcSurfaceX);
			if (!lpDDSrcSurfaceX)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surfaceX!");
				return DDERR_INVALIDPARAMS;
			}
		}
		else
		{
			lpDDSrcSurfaceX = this;
		}

		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
		HRESULT s_hr = (lpDDSrcSurfaceX == this) ? c_hr : lpDDSrcSurfaceX->CheckInterface(__FUNCTION__, true, true, true);
		if (FAILED(c_hr) || FAILED(s_hr))
		{
			return (c_hr == DDERR_SURFACELOST || s_hr == DDERR_SURFACELOST) ? DDERR_SURFACELOST : FAILED(c_hr) ? c_hr : s_hr;
		}

		// Set critical section
		SetLockCriticalSection();
		lpDDSrcSurfaceX->SetLockCriticalSection();

		// Present before write if needed
		if (!DontPresentBlt)
		{
			BeginWritePresent(IsSkipScene);
		}

		HRESULT hr = DD_OK;

		do {
			// Check if locked from other thread
			if (BltWait && (IsLockedFromOtherThread() || lpDDSrcSurfaceX->IsLockedFromOtherThread()))
			{
				// Wait for lock from other thread
				while (IsLockedFromOtherThread() || lpDDSrcSurfaceX->IsLockedFromOtherThread())
				{
					Utils::BusyWaitYield();
					if (!surface.Texture && !surface.Surface)
					{
						break;
					}
				}
				if (!surface.Texture && !surface.Surface)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: surface texture missing!");
					hr = DDERR_SURFACELOST;
					break;
				}
			}

			// Set blt flag
			IsInBlt = true;
			lpDDSrcSurfaceX->IsInBlt = true;

			// Set locked ID
			if (LockedWithID || lpDDSrcSurfaceX->LockedWithID)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: surface locked thread ID set! " << LockedWithID << " " << lpDDSrcSurfaceX->LockedWithID);
			}
			LockedWithID = GetCurrentThreadId();
			lpDDSrcSurfaceX->LockedWithID = GetCurrentThreadId();

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
			lpDDSrcSurfaceX->IsInBlt = false;
			IsInBlt = false;

			// Reset locked thread ID
			if (!IsSurfaceBlitting() && !IsSurfaceLocked())
			{
				LockedWithID = 0;
			}
			if (!lpDDSrcSurfaceX->IsSurfaceBlitting() && !lpDDSrcSurfaceX->IsSurfaceLocked())
			{
				lpDDSrcSurfaceX->LockedWithID = 0;
			}

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
				if (!DontPresentBlt)
				{
					EndWritePresent(IsSkipScene);
				}
			}

		} while (false);

		// Check if surface was busy
		if (!BltWait && (hr == DDERR_SURFACEBUSY || IsLockedFromOtherThread() || lpDDSrcSurfaceX->IsLockedFromOtherThread()))
		{
			hr = D3DERR_WASSTILLDRAWING;
		}

		// Release critical section
		lpDDSrcSurfaceX->ReleaseLockCriticalSection();
		ReleaseLockCriticalSection();

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
			SrcRect = *lpSrcRect;
			SrcRect.left -= 1;
			SrcRect.bottom -= 1;
			lpSrcRect = &SrcRect;
		}
		if (lpDestRect)
		{
			DestRect = *lpDestRect;
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

	HRESULT hr = DD_OK;

	bool IsSkipScene = false;

	SetLockCriticalSection();

	// Present before write if needed
	BeginWritePresent(IsSkipScene);

	IsInBltBatch = true;

	for (DWORD x = 0; x < dwCount; x++)
	{
		IsSkipScene |= (lpDDBltBatch[x].lprDest) ? CheckRectforSkipScene(*lpDDBltBatch[x].lprDest) : false;

		hr = Blt(lpDDBltBatch[x].lprDest, (LPDIRECTDRAWSURFACE7)lpDDBltBatch[x].lpDDSSrc, lpDDBltBatch[x].lprSrc, lpDDBltBatch[x].dwFlags, lpDDBltBatch[x].lpDDBltFx, true);
		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: BltBatch failed before the end! " << x << " of " << dwCount << " " << (DDERR)hr);
			break;
		}
	}

	IsInBltBatch = false;

	if (!IsSurfaceBlitting() && !IsSurfaceLocked())
	{
		LockedWithID = 0;
	}

	if (SUCCEEDED(hr))
	{
		EndWritePresent(IsSkipScene);
	}

	ReleaseLockCriticalSection();

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::BltFast(DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Check if source Surface exists
	if (lpDDSrcSurface && !CheckSurfaceExists(lpDDSrcSurface))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not find source surface! " << Logging::hex(lpDDSrcSurface));
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		// NOTE: If you call IDirectDrawSurface7::BltFast on a surface with an attached clipper, it returns DDERR_UNSUPPORTED.
		if (attachedClipper)
		{
			return DDERR_UNSUPPORTED;
		}

		// Convert BltFast flags into Blt flags
		DWORD Flags = DDBLT_ASYNC;
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

		// Get source surface
		m_IDirectDrawSurfaceX* lpDDSrcSurfaceX = nullptr;
		if (lpDDSrcSurface)
		{
			lpDDSrcSurface->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpDDSrcSurfaceX);
			if (!lpDDSrcSurfaceX)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surfaceX!");
				return DDERR_GENERIC;
			}
		}
		else
		{
			lpDDSrcSurfaceX = this;
		}

		// Get SrcRect
		RECT SrcRect = {};
		if (!lpDDSrcSurfaceX->CheckCoordinates(SrcRect, lpSrcRect))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Invalid rect: " << lpSrcRect);
		}

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
		RECT SrcRect = *lpSrcRect;
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
	} CallbackContext = {};
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

	struct EnumSurface
	{
		LPVOID lpContext;
		LPDDENUMSURFACESCALLBACK7 lpCallback;
		DWORD DirectXVersion;
		bool ConvertSurfaceDescTo2;

		static HRESULT CALLBACK ConvertCallback(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext)
		{
			EnumSurface* self = (EnumSurface*)lpContext;

			if (!Config.Dd7to9 && lpDDSurface)
			{
				lpDDSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(lpDDSurface, self->DirectXVersion);
			}

			// Game using old DirectX, Convert back to LPDDSURFACEDESC
			if (self->ConvertSurfaceDescTo2)
			{
				DDSURFACEDESC Desc = {};
				Desc.dwSize = sizeof(DDSURFACEDESC);
				ConvertSurfaceDesc(Desc, *lpDDSurfaceDesc2);

				return ((LPDDENUMSURFACESCALLBACK)self->lpCallback)((LPDIRECTDRAWSURFACE)lpDDSurface, &Desc, self->lpContext);
			}

			return self->lpCallback(lpDDSurface, lpDDSurfaceDesc2, self->lpContext);
		}
	} CallbackContext = {};
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = lpEnumSurfacesCallback7;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ConvertSurfaceDescTo2 = (ProxyDirectXVersion > 3 && DirectXVersion < 4);

	if (Config.Dd7to9)
	{
		for (auto& it : AttachedSurfaceMap)
		{
			// This method enumerates all the surfaces attached to a given surface.
			// In a flipping chain of three or more surfaces, only one surface is enumerated because each surface is attached only to the next surface in the flipping chain.
			// In such a configuration, you can call EnumAttachedSurfaces on each successive surface to walk the entire flipping chain.
			// The front buffer should not be returned as attached.
			if (!(it.second.pSurface->surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER))
			{
				DDSURFACEDESC2 Desc2 = {};
				Desc2.dwSize = sizeof(DDSURFACEDESC2);
				it.second.pSurface->GetSurfaceDesc2(&Desc2);
				LPDIRECTDRAWSURFACE7 lpSurface = (LPDIRECTDRAWSURFACE7)it.second.pSurface->GetWrapperInterfaceX(DirectXVersion);
				if (EnumSurface::ConvertCallback(lpSurface, &Desc2, &CallbackContext) == DDENUMRET_CANCEL)
				{
					return DD_OK;
				}
			}
		}

		return DD_OK;
	}

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
	} CallbackContext = {};
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

	struct EnumSurface
	{
		LPVOID lpContext;
		LPDDENUMSURFACESCALLBACK7 lpCallback;
		DWORD DirectXVersion;
		bool ConvertSurfaceDescTo2;

		static HRESULT CALLBACK ConvertCallback(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext)
		{
			EnumSurface *self = (EnumSurface*)lpContext;

			if (!Config.Dd7to9 && lpDDSurface)
			{
				lpDDSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(lpDDSurface, self->DirectXVersion);
			}

			// Game using old DirectX, Convert back to LPDDSURFACEDESC
			if (self->ConvertSurfaceDescTo2)
			{
				DDSURFACEDESC Desc = {};
				Desc.dwSize = sizeof(DDSURFACEDESC);
				ConvertSurfaceDesc(Desc, *lpDDSurfaceDesc2);

				return ((LPDDENUMSURFACESCALLBACK)self->lpCallback)((LPDIRECTDRAWSURFACE)lpDDSurface, &Desc, self->lpContext);
			}

			return self->lpCallback(lpDDSurface, lpDDSurfaceDesc2, self->lpContext);
		}
	} CallbackContext = {};
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = lpfnCallback7;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ConvertSurfaceDescTo2 = (ProxyDirectXVersion > 3 && DirectXVersion < 4);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->EnumOverlayZOrders(dwFlags, &CallbackContext, EnumSurface::ConvertCallback);
}

// Check if backbuffer can flip with current surface
inline HRESULT m_IDirectDrawSurfaceX::CheckBackBufferForFlip(m_IDirectDrawSurfaceX* lpTargetSurface)
{
	// Check if target surface exists
	if (!lpTargetSurface || lpTargetSurface == this || !DoesFlipBackBufferExist(lpTargetSurface))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid surface!");
		return DDERR_INVALIDPARAMS;
	}

	// Make sure that surface description on target is updated
	lpTargetSurface->UpdateSurfaceDesc();

	// Check for device interface
	HRESULT c_hr = lpTargetSurface->CheckInterface(__FUNCTION__, true, true, true);
	if (FAILED(c_hr))
	{
		return c_hr;
	}

	// Check if surface format and size matches
	if (surfaceFormat != lpTargetSurface->surfaceFormat ||
		surfaceDesc2.dwWidth != lpTargetSurface->surfaceDesc2.dwWidth ||
		surfaceDesc2.dwHeight != lpTargetSurface->surfaceDesc2.dwHeight)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: backbuffer surface does not match: " <<
			surfaceFormat << " -> " << lpTargetSurface->surfaceFormat << " " <<
			surfaceDesc2.dwWidth << "x" << surfaceDesc2.dwHeight << " -> " <<
			lpTargetSurface->surfaceDesc2.dwWidth << "x" << lpTargetSurface->surfaceDesc2.dwHeight);
		return DDERR_INVALIDPARAMS;
	}

	return DD_OK;
}

HRESULT m_IDirectDrawSurfaceX::Flip(LPDIRECTDRAWSURFACE7 lpDDSurfaceTargetOverride, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if ((dwFlags & (DDFLIP_EVEN | DDFLIP_ODD)) == (DDFLIP_EVEN | DDFLIP_ODD))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid flags!");
			return DDERR_INVALIDPARAMS;
		}

		// Flip can be called only for a surface that has the DDSCAPS_FLIP and DDSCAPS_FRONTBUFFER capabilities
		if (!isFlipSurface())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: This surface cannot be flipped");
			return DDERR_NOTFLIPPABLE;
		}

		if (dwFlags & DDFLIP_STEREO)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Stereo flipping not implemented");
			return DDERR_NOSTEREOHARDWARE;
		}

		if ((dwFlags & (DDFLIP_INTERVAL2 | DDFLIP_INTERVAL3 | DDFLIP_INTERVAL4)) && (surfaceDesc2.ddsCaps.dwCaps2 & DDCAPS2_FLIPINTERVAL))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Interval flipping not implemented");
			return DDERR_UNSUPPORTED;
		}

		if (dwFlags & (DDFLIP_ODD | DDFLIP_EVEN))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Even and odd flipping not implemented");
			return DDERR_UNSUPPORTED;
		}

		const bool FlipWait = (((dwFlags & DDFLIP_WAIT) || DirectXVersion == 7) && (dwFlags & DDFLIP_DONOTWAIT) == 0);

		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		// Create flip list
		std::vector<m_IDirectDrawSurfaceX*> FlipList;
		FlipList.push_back(this);

		// If SurfaceTargetOverride then use that surface
		if (lpDDSurfaceTargetOverride)
		{
			m_IDirectDrawSurfaceX* lpTargetSurface = nullptr;

			lpDDSurfaceTargetOverride->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpTargetSurface);

			// Check backbuffer
			HRESULT hr = CheckBackBufferForFlip(lpTargetSurface);
			if (FAILED(hr))
			{
				return hr;
			}

			FlipList.push_back(lpTargetSurface);
		}
		// Get list for all attached surfaces
		else
		{
			m_IDirectDrawSurfaceX* lpTargetSurface = this;
			do {
				DWORD dwCaps = 0;
				m_IDirectDrawSurfaceX* lpNewTargetSurface = nullptr;

				// Loop through each surface
				for (auto& it : lpTargetSurface->AttachedSurfaceMap)
				{
					dwCaps = it.second.pSurface->GetSurfaceCaps().dwCaps;
					if (dwCaps & DDSCAPS_FLIP)
					{
						lpNewTargetSurface = it.second.pSurface;
						break;
					}
				}
				lpTargetSurface = lpNewTargetSurface;

				// Stop looping when frontbuffer is found
				if (lpTargetSurface == this || dwCaps & DDSCAPS_FRONTBUFFER)
				{
					break;
				}

				// Check backbuffer
				HRESULT hr = CheckBackBufferForFlip(lpTargetSurface);
				if (FAILED(hr))
				{
					return hr;
				}

				// Add target surface to list
				FlipList.push_back(lpTargetSurface);

			} while (true);
		}

		// Lambda function to check if any surface is busy
		auto FlipSurfacesAreLockedFromOtherThread = [&FlipList]() {
			for (m_IDirectDrawSurfaceX*& pSurfaceX : FlipList)
			{
				if (pSurfaceX->IsLockedFromOtherThread())
				{
					return true;
				}
			}
			return false; };

		// Set critical section for each surface
		for (m_IDirectDrawSurfaceX*& pSurfaceX : FlipList)
		{
			pSurfaceX->SetLockCriticalSection();
		}

		// Present before write if needed
		BeginWritePresent(false);

		HRESULT hr = DD_OK;

		do {
			// Check if locked from other thread
			if (FlipWait)
			{
				// Wait for locks from other threads
				while (FlipSurfacesAreLockedFromOtherThread())
				{
					Utils::BusyWaitYield();
				}
			}

			// Check if any surface is busy
			for (m_IDirectDrawSurfaceX*& pSurfaceX : FlipList)
			{
				if (pSurfaceX->IsSurfaceBusy())
				{
					if (FlipWait)
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: surface is busy: " <<
							pSurfaceX->IsSurfaceLocked() << " DC: " << pSurfaceX->IsSurfaceInDC() << " Blt: " << pSurfaceX->IsSurfaceBlitting());
						hr = DDERR_WASSTILLDRAWING;
						break;
					}
					hr = DDERR_SURFACEBUSY;
					break;
				}
			}

			// Set flip flag
			IsInFlip = true;

			// Clear dirty surface before flip if system memory or 3D device
			if (surfaceDesc2.ddsCaps.dwCaps & (DDSCAPS_SYSTEMMEMORY | DDSCAPS_3DDEVICE))
			{
				ClearPrimarySurface();
			}

			// Execute flip
			for (size_t x = 0; x < FlipList.size() - 1; x++)
			{
				SwapAddresses(&FlipList[x]->surface, &FlipList[x + 1]->surface);
			}

			// Reset flip flag
			IsInFlip = false;

			// Set vertical sync wait timer
			if ((dwFlags & DDFLIP_NOVSYNC) == 0)
			{
				ddrawParent->SetVsync();
			}

			// Preset surface to window
			if (!Using3D)
			{
				RECT Rect = { 0, 0, (LONG)surfaceDesc2.dwWidth, (LONG)surfaceDesc2.dwHeight };

				// Blt surface directly to GDI
				if (Config.DdrawWriteToGDI)
				{
					CopyEmulatedSurfaceToGDI(Rect);
				}
				// Handle windowed mode
				else if (surface.IsUsingWindowedMode)
				{
					PresentSurfaceToWindow(Rect);
				}
			}

			// Present surface
			if (!Using3D)
			{
				EndWritePresent(false);
			}
			else
			{
				ddrawParent->Present(nullptr, nullptr);
			}

		} while (false);

		// Release critical section for each surface
		for (m_IDirectDrawSurfaceX*& pSurfaceX : FlipList)
		{
			pSurfaceX->ReleaseLockCriticalSection();
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
		HRESULT c_hr = CheckInterface(__FUNCTION__, false, false, false);
		if (FAILED(c_hr))
		{
			return c_hr;
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
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to find attached surface that matches the capabilities requested: " << *lpDDSCaps2 <<
				" Attached number of surfaces: " << AttachedSurfaceMap.size());
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
		DDSCaps2 = *lpDDSCaps2;

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
		// Check if device interface is lost
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, false, false);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		// Inquires whether a blit involving this surface can occur immediately, and returns DD_OK if the blit can be completed.
		if (dwFlags == DDGBS_CANBLT)
		{
			if (IsSurfaceBlitting())
			{
				return DDERR_WASSTILLDRAWING;
			}
			if (IsSurfaceBusy())
			{
				return DDERR_SURFACEBUSY;
			}
			return DD_OK;
		}
		// Inquires whether the blit is done, and returns DD_OK if the last blit on this surface has completed.
		else if (dwFlags == DDGBS_ISBLTDONE)
		{
			if (IsSurfaceBlitting())
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

		*lpDDSCaps2 = surfaceDesc2.ddsCaps;

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

		// Reset shader flag
		ShaderColorKey.IsSet = false;

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

bool m_IDirectDrawSurfaceX::GetColorKeyForShader(float(&lowColorKey)[4], float(&highColorKey)[4], bool PrimarySurface)
{
	// Primary 2D surface background color
	if (PrimarySurface && IsPrimarySurface())
	{
		if (!primary.ShaderColorKey.IsSet)
		{
			GetColorKeyArray(primary.ShaderColorKey.lowColorKey, primary.ShaderColorKey.highColorKey, 0x00000000, 0x00000000, surfaceDesc2.ddpfPixelFormat);
			primary.ShaderColorKey.IsSet = true;
		}
		lowColorKey[0] = primary.ShaderColorKey.lowColorKey[0];
		lowColorKey[1] = primary.ShaderColorKey.lowColorKey[1];
		lowColorKey[2] = primary.ShaderColorKey.lowColorKey[2];
		lowColorKey[3] = primary.ShaderColorKey.lowColorKey[3];
		highColorKey[0] = primary.ShaderColorKey.highColorKey[0];
		highColorKey[1] = primary.ShaderColorKey.highColorKey[1];
		highColorKey[2] = primary.ShaderColorKey.highColorKey[2];
		highColorKey[3] = primary.ShaderColorKey.highColorKey[3];
		return true;
	}

	// Surface low and high color space
	if (!ShaderColorKey.IsSet)
	{
		if (surfaceDesc2.ddsCaps.dwCaps & DDSD_CKSRCBLT)
		{
			GetColorKeyArray(ShaderColorKey.lowColorKey, ShaderColorKey.highColorKey,
				surfaceDesc2.ddckCKSrcBlt.dwColorSpaceLowValue, surfaceDesc2.ddckCKSrcBlt.dwColorSpaceHighValue, surfaceDesc2.ddpfPixelFormat);
			ShaderColorKey.IsSet = true;
		}
		else if (surfaceDesc2.ddsCaps.dwCaps & DDCKEY_SRCOVERLAY)
		{
			GetColorKeyArray(ShaderColorKey.lowColorKey, ShaderColorKey.highColorKey,
				surfaceDesc2.ddckCKSrcOverlay.dwColorSpaceLowValue, surfaceDesc2.ddckCKSrcOverlay.dwColorSpaceHighValue, surfaceDesc2.ddpfPixelFormat);
			ShaderColorKey.IsSet = true;
		}
		else
		{
			return false;
		}
	}
	lowColorKey[0] = ShaderColorKey.lowColorKey[0];
	lowColorKey[1] = ShaderColorKey.lowColorKey[1];
	lowColorKey[2] = ShaderColorKey.lowColorKey[2];
	lowColorKey[3] = ShaderColorKey.lowColorKey[3];
	highColorKey[0] = ShaderColorKey.highColorKey[0];
	highColorKey[1] = ShaderColorKey.highColorKey[1];
	highColorKey[2] = ShaderColorKey.highColorKey[2];
	highColorKey[3] = ShaderColorKey.highColorKey[3];
	return true;
}

HRESULT m_IDirectDrawSurfaceX::GetDC(HDC FAR* lphDC)
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
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
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

		if (EmuLock.Locked && EmuLock.Addr)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: does not support device context with bit alignment!");
		}

		SetLockCriticalSection();

		// Present before write if needed
		BeginWritePresent(false);

		HRESULT hr = DD_OK;

		do {

			if (IsUsingEmulation() || DCRequiresEmulation)
			{
				if (!IsUsingEmulation())
				{
					if (FAILED(CreateDCSurface()))
					{
						hr = DDERR_GENERIC;
						break;
					}

					CopyToEmulatedSurface(nullptr);
				}

				// Set new palette data
				UpdatePaletteData();

				// Read surface from GDI
				if (Config.DdrawReadFromGDI && IsPrimaryOrBackBuffer() && !Using3D)
				{
					RECT Rect = { 0, 0, (LONG)surfaceDesc2.dwWidth, (LONG)surfaceDesc2.dwHeight };
					CopyEmulatedSurfaceFromGDI(Rect);
				}

				// Prepare GameDC
				SetEmulationGameDC();

				*lphDC = surface.emu->GameDC;
			}
			else
			{
				// Get surface
				IDirect3DSurface9* pSurfaceD9 = GetD3D9Surface();
				if (!pSurfaceD9)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
					hr = DDERR_GENERIC;
					break;
				}

				// Get device context
				if (FAILED(pSurfaceD9->GetDC(lphDC)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not get device context!");
					hr = DDERR_GENERIC;
					break;
				}
			}

			// Set DC flag
			IsInDC = true;

			// Set LastDC
			LastDC = *lphDC;

		} while (false);

		ReleaseLockCriticalSection();

		return hr;
	}

	return ProxyInterface->GetDC(lphDC);
}

HRESULT m_IDirectDrawSurfaceX::GetFlipStatus(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Flip can be called only for a surface that has the DDSCAPS_FLIP and DDSCAPS_FRONTBUFFER capabilities
		if (!isFlipSurface())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: This surface cannot be flipped");
			return DDERR_INVALIDSURFACETYPE;
		}

		// Check if device interface is lost
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, false, false);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		// Queries whether the surface can flip now. The method returns DD_OK if the flip can be completed.
		if ((dwFlags == DDGFS_CANFLIP))
		{
			// Check if flip is still happening
			if (IsInFlip)
			{
				return DDERR_WASSTILLDRAWING;
			}

			// Get backbuffer
			m_IDirectDrawSurfaceX* lpBackBuffer = nullptr;
			for (auto& it : AttachedSurfaceMap)
			{
				if (it.second.pSurface->GetSurfaceCaps().dwCaps & DDSCAPS_BACKBUFFER)
				{
					lpBackBuffer = it.second.pSurface;
					break;
				}
			}

			// Check if there is a backbuffer
			if (!lpBackBuffer)
			{
				return DDERR_INVALIDSURFACETYPE;
			}

			// Check if surface is busy
			if (IsSurfaceBusy() || lpBackBuffer->IsSurfaceBusy())
			{
				return DDERR_SURFACEBUSY;
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
		*lpDDPixelFormat = surfaceDesc2.ddpfPixelFormat;

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

		DDSURFACEDESC2 Desc2 = {};
		Desc2.dwSize = sizeof(DDSURFACEDESC2);

		HRESULT hr = hr = GetSurfaceDesc2(&Desc2);

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
		*lpDDSurfaceDesc2 = surfaceDesc2;

		// Remove surface memory pointer
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

		DDSURFACEDESC2 Desc2 = {};
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
		// Check surface lost flag
		if (surface.IsSurfaceLost)
		{
			return DDERR_SURFACELOST;
		}

		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, false, true);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		// Return
		return DD_OK;
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

		DDSURFACEDESC2 Desc2 = {};
		Desc2.dwSize = sizeof(DDSURFACEDESC2);

		HRESULT hr = Lock2(lpDestRect, &Desc2, dwFlags, hEvent, DirectXVersion);

		// Convert back to LPDDSURFACEDESC
		ConvertSurfaceDesc(*lpDDSurfaceDesc, Desc2);

		return hr;
	}

	return GetProxyInterfaceV3()->Lock(lpDestRect, lpDDSurfaceDesc, dwFlags, hEvent);
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

		// If primary surface and palette surface and created via Lock() then mark as created by lock to emulate surface (eg. Diablo)
		if (IsPrimarySurface() && ShouldEmulate == SC_NOT_CREATED && surfaceDesc2.dwFlags == DDSD_CAPS)
		{
			UpdateSurfaceDesc();

			if (surfaceDesc2.ddpfPixelFormat.dwRGBBitCount == 8 && (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) &&
				surfaceDesc2.dwBackBufferCount == 0 && (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_FLIP) == 0)
			{
				ShouldEmulate = SC_FORCE_EMULATED;
			}
		}

		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);

		// Prepare surfaceDesc
		*lpDDSurfaceDesc2 = surfaceDesc2;
		if (!(lpDDSurfaceDesc2->dwFlags & DDSD_LPSURFACE))
		{
			lpDDSurfaceDesc2->lpSurface = nullptr;
		}

		// Return error for CheckInterface after preparing surfaceDesc
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		// Check for already locked state
		if (!lpDestRect && !LockRectList.empty())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: locking surface with NULL rect when surface is already locked!");
			return DDERR_INVALIDRECT;
		}

		// Update rect
		RECT DestRect = {};
		if (!CheckCoordinates(DestRect, lpDestRect) || (lpDestRect && (lpDestRect->left < 0 || lpDestRect->top < 0 ||
			lpDestRect->right <= lpDestRect->left || lpDestRect->bottom <= lpDestRect->top ||
			lpDestRect->right >(LONG)surfaceDesc2.dwWidth || lpDestRect->bottom >(LONG)surfaceDesc2.dwHeight)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << lpDestRect);
			return DDERR_INVALIDRECT;
		}

		// Set to indicate that Lock should wait until it can obtain a valid memory pointer before returning.
		const bool LockWait = (((dwFlags & DDLOCK_WAIT) || DirectXVersion == 7) && (dwFlags & DDLOCK_DONOTWAIT) == 0);

		// Convert flags to d3d9
		DWORD Flags = (dwFlags & (D3DLOCK_READONLY | D3DLOCK_NOOVERWRITE | (!IsPrimarySurface() ? D3DLOCK_NOSYSLOCK : 0))) |
			(!LockWait ? D3DLOCK_DONOTWAIT : 0) |
			((dwFlags & DDLOCK_NODIRTYUPDATE) ? D3DLOCK_NO_DIRTY_UPDATE : 0);

		// Check if the scene needs to be presented
		const bool IsSkipScene = (CheckRectforSkipScene(DestRect) || (Flags & D3DLOCK_READONLY));

		SetLockCriticalSection();

		// Present before write if needed
		BeginWritePresent(IsSkipScene);

		HRESULT hr = DD_OK;

		do {
			// Check if locked from other thread
			if (LockWait && IsLockedFromOtherThread())
			{
				// Wait for lock from other thread
				while (IsLockedFromOtherThread())
				{
					Utils::BusyWaitYield();
					if (!surface.Texture && !surface.Surface)
					{
						break;
					}
				}
				if (!surface.Texture && !surface.Surface)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: surface texture missing!");
					hr = DDERR_SURFACELOST;
					break;
				}
			}

			// Emulated surface
			D3DLOCKED_RECT LockedRect = {};
			if (IsUsingEmulation())
			{
				// Set locked rect
				if (FAILED(LockEmulatedSurface(&LockedRect, &DestRect)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock emulated surface!");
					hr = DDERR_GENERIC;
					break;
				}

				// Read surface from GDI
				if (Config.DdrawReadFromGDI && IsPrimaryOrBackBuffer() && !Using3D)
				{
					CopyEmulatedSurfaceFromGDI(DestRect);
				}
			}
			// Lock surface
			else if (surface.Texture || surface.Surface)
			{
				// Lock surface
				HRESULT ret = LockD39Surface(&LockedRect, &DestRect, Flags);
				if (FAILED(ret))
				{
					if (IsSurfaceLocked())
					{
						LOG_LIMIT(100, __FUNCTION__ << " Warning: attempting to lock surface twice!");
					}
					UnlockD39Surface();
					ret = LockD39Surface(&LockedRect, &DestRect, Flags);
				}
				if (FAILED(ret))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock surface texture." << (surface.Surface ? " Is 3DSurface." : " Is Texture.") <<
						" Size: " << surfaceDesc2.dwWidth << "x" << surfaceDesc2.dwHeight << " Format: " << surfaceFormat <<
						" Flags: " << Logging::hex(Flags) << " Locked: " << IsSurfaceLocked() << " DC: " << IsSurfaceInDC() << " Blt: " << IsSurfaceBlitting() << " hr: " << (D3DERR)ret);
					hr = (ret == D3DERR_WASSTILLDRAWING || (!LockWait && IsSurfaceBusy())) ? DDERR_WASSTILLDRAWING : DDERR_GENERIC;
					break;
				}
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
				hr = DDERR_GENERIC;
				break;
			}

			// Check pointer and pitch
			if (!LockedRect.pBits || !LockedRect.Pitch)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get surface address or pitch!");
				hr = DDERR_GENERIC;
				break;
			}

			// Set lock flag
			IsLocked = true;

			if (LockedWithID)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: surface locked thread ID set! " << LockedWithID);
			}
			LockedWithID = GetCurrentThreadId();

			// Store locked rect
			if (lpDestRect)
			{
				RECT lRect = { lpDestRect->left, lpDestRect->top, lpDestRect->right, lpDestRect->bottom };
				LockRectList.push_back(lRect);
			}

			// Set surfaceDesc
			if (!(lpDDSurfaceDesc2->dwFlags & DDSD_LPSURFACE))
			{
				lpDDSurfaceDesc2->lpSurface = LockedRect.pBits;
				lpDDSurfaceDesc2->dwFlags |= DDSD_LPSURFACE;
			}
			// Pitch for DXT surfaces in DirectDraw is the full surface byte size
			LockedRect.Pitch =
				ISDXTEX(surfaceFormat) ? ((GetByteAlignedWidth(surfaceDesc2.dwWidth, surfaceBitCount) + 3) / 4) * ((surfaceDesc2.dwHeight + 3) / 4) * (surfaceFormat == D3DFMT_DXT1 ? 8 : 16) :
				(surfaceFormat == D3DFMT_YV12) ? GetByteAlignedWidth(surfaceDesc2.dwWidth, surfaceBitCount) :
				LockedRect.Pitch;
			lpDDSurfaceDesc2->lPitch = LockedRect.Pitch;
			lpDDSurfaceDesc2->dwFlags |= DDSD_PITCH;

			// Set surface pitch
			if ((surfaceDesc2.dwFlags & DDSD_PITCH) && surfaceDesc2.lPitch != LockedRect.Pitch)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: surface pitch does not match locked pitch! Format: " << surfaceFormat <<
					" Width: " << surfaceDesc2.dwWidth << " Pitch: " << surfaceDesc2.lPitch << "->" << LockedRect.Pitch);
			}
			surfaceDesc2.lPitch = LockedRect.Pitch;
			surfaceDesc2.dwFlags |= DDSD_PITCH;

			// Emulate lock
			if (!(Flags & D3DLOCK_READONLY) && (Config.DdrawEmulateLock || Config.DdrawFixByteAlignment))
			{
				LockEmuLock(lpDestRect, lpDDSurfaceDesc2);
			}

			// Backup last rect before removing scanlines
			LastLock.ReadOnly = (Flags & D3DLOCK_READONLY);
			LastLock.IsSkipScene = IsSkipScene;
			LastLock.Rect = DestRect;
			LastLock.LockedRect.pBits = LockedRect.pBits;
			LastLock.LockedRect.Pitch = LockedRect.Pitch;

			// Restore scanlines before returing surface memory
			if (Config.DdrawRemoveScanlines && IsPrimaryOrBackBuffer() && !(Flags & D3DLOCK_READONLY))
			{
				RestoreScanlines(LastLock);
			}

		} while (false);

		ReleaseLockCriticalSection();

		return hr;
	}

	return ProxyInterface->Lock(lpDestRect, lpDDSurfaceDesc2, dwFlags, hEvent);
}

inline HRESULT m_IDirectDrawSurfaceX::LockD39Surface(D3DLOCKED_RECT* pLockedRect, RECT* pRect, DWORD Flags)
{
	// Lock surface texture
	if (surface.Texture)
	{
		return surface.Texture->LockRect(0, pLockedRect, pRect, Flags);
	}
	// Lock 3D surface
	else if (surface.Surface)
	{
		return surface.Surface->LockRect(pLockedRect, pRect, Flags);
	}

	return DDERR_GENERIC;
}

HRESULT m_IDirectDrawSurfaceX::ReleaseDC(HDC hDC)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		if (!IsSurfaceInDC())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not in DC!");
			return DDERR_GENERIC;
		}

		SetSurfaceCriticalSection();

		HRESULT hr = DD_OK;

		do {

			if (IsUsingEmulation() || DCRequiresEmulation)
			{
				if (!IsUsingEmulation())
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: surface not using emulated DC!");
					break;
				}

				// Restore DC
				UnsetEmulationGameDC();

				// Copy emulated surface to real texture
				CopyFromEmulatedSurface(nullptr);

				// Blt surface directly to GDI
				if (Config.DdrawWriteToGDI && IsPrimarySurface() && !Using3D)
				{
					RECT Rect = { 0, 0, (LONG)surfaceDesc2.dwWidth, (LONG)surfaceDesc2.dwHeight };
					CopyEmulatedSurfaceToGDI(Rect);
				}
			}
			else
			{
				// Get surface
				IDirect3DSurface9* pSurfaceD9 = GetD3D9Surface();
				if (!pSurfaceD9)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
					hr = DDERR_GENERIC;
					break;
				}

				// Release device context
				if (FAILED(pSurfaceD9->ReleaseDC(hDC)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to release surface DC!");
					hr = DDERR_GENERIC;
					break;
				}
			}

			// Reset DC flag
			IsInDC = false;

			// Set LastDC
			LastDC = nullptr;

			// Set dirty flag
			SetDirtyFlag();

			// Preset surface to window
			if (surface.IsUsingWindowedMode && IsPrimarySurface() && !Config.DdrawWriteToGDI && !Using3D)
			{
				RECT Rect = { 0, 0, (LONG)surfaceDesc2.dwWidth, (LONG)surfaceDesc2.dwHeight };
				PresentSurfaceToWindow(Rect);
			}

			// Present surface
			EndWritePresent(false);

		} while (false);

		ReleaseSurfaceCriticalSection();

		return hr;
	}

	return ProxyInterface->ReleaseDC(hDC);
}

HRESULT m_IDirectDrawSurfaceX::Restore()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, false, false, false);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		// ToDo: A single call to this method will restore a DirectDrawSurface object's associated implicit surfaces (back buffers, and so on). 

		HRESULT hr = ddrawParent->ReinitDevice();
		switch (hr)
		{
		case D3D_OK:
		case DDERR_NOEXCLUSIVEMODE:
			if (FAILED(CheckInterface(__FUNCTION__, true, true, false)))
			{
				return DDERR_WRONGMODE;
			}
			if (surface.IsSurfaceLost)
			{
				surface.IsSurfaceLost = false;
				surface.SurfaceHasData = false;
			}
			return hr;
		case DDERR_SURFACELOST:
			return DDERR_SURFACELOST;
		default:
			return DDERR_WRONGMODE;
		}
	}

	return ProxyInterface->Restore();
}

HRESULT m_IDirectDrawSurfaceX::SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpDDClipper && !attachedClipper)
		{
			return DDERR_NOCLIPPERATTACHED;
		}

		if (lpDDClipper == attachedClipper)
		{
			return DD_OK;
		}

		// If clipper exists increament ref
		if (lpDDClipper)
		{
			if (!ddrawParent || !ddrawParent->DoesClipperExist((m_IDirectDrawClipper*)lpDDClipper))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not find clipper");
				return DDERR_INVALIDOBJECT;
			}

			lpDDClipper->AddRef();
		}

		// Decrement ref count
		if (attachedClipper && ddrawParent && ddrawParent->DoesClipperExist(attachedClipper))
		{
			attachedClipper->Release();
		}

		// Set clipper address
		attachedClipper = (m_IDirectDrawClipper*)lpDDClipper;

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
			DDCOLORKEY ColorKey = {};
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
		if (!lpDDPalette && !attachedPalette)
		{
			return DDERR_NOPALETTEATTACHED;
		}

		if (lpDDPalette == attachedPalette)
		{
			return DD_OK;
		}

		// If palette exists increament ref
		if (lpDDPalette)
		{
			if (!ddrawParent || !ddrawParent->DoesPaletteExist((m_IDirectDrawPalette*)lpDDPalette))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not find palette");
				return DDERR_INVALIDOBJECT;
			}

			lpDDPalette->AddRef();

			// Set primary flag
			if (IsPrimarySurface())
			{
				((m_IDirectDrawPalette*)lpDDPalette)->SetPrimary();
			}
		}

		// Decrement ref count
		if (attachedPalette && ddrawParent && ddrawParent->DoesPaletteExist(attachedPalette))
		{
			// Remove primary flag
			if (IsPrimarySurface() && attachedPalette != lpDDPalette)
			{
				attachedPalette->RemovePrimary();
			}

			attachedPalette->Release();
		}

		// Set palette address
		attachedPalette = (m_IDirectDrawPalette*)lpDDPalette;

		SetCriticalSection();

		// Reset data for new palette
		surface.LastPaletteUSN = 0;
		surface.PaletteEntryArray = nullptr;

		// Set new palette data
		UpdatePaletteData();

		ReleaseCriticalSection();

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

	if (Config.Dd7to9)
	{
		SetSurfaceCriticalSection();

		// Emulate unlock
		if (EmuLock.Locked)
		{
			UnlockEmuLock();
		}

		HRESULT hr = DD_OK;

		do {
			// Check rect
			if (!lpRect && LockRectList.size() > 1)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Rect cannot be NULL when locked with a specific rect!");
				hr = DDERR_INVALIDRECT;
				break;
			}

			// Check stored rect
			if (lpRect && LockRectList.size() > 1)
			{
				auto it = std::find_if(LockRectList.begin(), LockRectList.end(),
					[=](auto Rect) -> bool { return (Rect.left == lpRect->left && Rect.top == lpRect->top && Rect.right == lpRect->right && Rect.bottom == lpRect->bottom); });

				if (it != std::end(LockRectList))
				{
					LockRectList.erase(it);

					// Unlock once all rects have been unlocked
					if (!LockRectList.empty())
					{
						LOG_LIMIT(100, __FUNCTION__ << " Warning: multiple locked rects found: " << LockRectList.size());
						hr = DD_OK;
						break;
					}
				}
				else
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: Rect does not match locked rect: " << lpRect);
					hr = DDERR_INVALIDRECT;
					break;
				}
			}

			// Check for device interface
			HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
			if (FAILED(c_hr))
			{
				hr = c_hr;
				break;
			}

			// Remove scanlines before unlocking surface
			if (Config.DdrawRemoveScanlines && IsPrimaryOrBackBuffer())
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
					if (Config.DdrawWriteToGDI && IsPrimarySurface() && !Using3D)
					{
						CopyEmulatedSurfaceToGDI(LastLock.Rect);
					}
				}
			}
			// Lock surface
			else if (surface.Texture || surface.Surface)
			{
				HRESULT ret = UnlockD39Surface();
				if (FAILED(ret))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to unlock surface texture");
					hr = (ret == D3DERR_INVALIDCALL) ? DDERR_GENERIC :
						(ret == D3DERR_WASSTILLDRAWING) ? DDERR_WASSTILLDRAWING :
						DDERR_SURFACELOST;
					break;
				}
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
				hr = DDERR_GENERIC;
				break;
			}

			// Clear memory pointer
			LastLock.LockedRect.pBits = nullptr;

			// Clear vector
			LockRectList.clear();

			// Reset locked flag
			IsLocked = false;

			// Reset locked thread ID
			if (!IsSurfaceBlitting() && !IsSurfaceLocked())
			{
				LockedWithID = 0;
			}

			// Set dirty flag
			if (!LastLock.ReadOnly)
			{
				SetDirtyFlag();
			}

			// Preset surface to window
			if (surface.IsUsingWindowedMode && IsPrimarySurface() && !Config.DdrawWriteToGDI && !Using3D)
			{
				PresentSurfaceToWindow(LastLock.Rect);
			}

			// Present surface
			EndWritePresent(LastLock.IsSkipScene);

		} while (false);

		ReleaseSurfaceCriticalSection();

		return hr;
	}

	return ProxyInterface->Unlock(lpRect);
}

inline HRESULT m_IDirectDrawSurfaceX::UnlockD39Surface()
{
	// Lock surface texture
	if (surface.Texture)
	{
		return surface.Texture->UnlockRect(0);
	}
	// Lock 3D surface
	else if (surface.Surface)
	{
		return surface.Surface->UnlockRect();
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

	if (!lplpDD)
	{
		return DDERR_INVALIDPARAMS;
	}
	*lplpDD = nullptr;

	if (Config.Dd7to9)
	{
		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, false, false, false);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		*lplpDD = ddrawParent->GetWrapperInterfaceX(DirectXVersion);

		ddrawParent->AddRef(DirectXVersion);

		return DD_OK;
	}

	LPVOID NewDD = nullptr;
	HRESULT hr = ProxyInterface->GetDDInterface(&NewDD);

	if (SUCCEEDED(hr))
	{
		// Calling the GetDDInterface method from any surface created under DirectDrawEx will return a pointer to the 
		// IUnknown interface instead of a pointer to an IDirectDraw interface. Applications must use the
		// IUnknown::QueryInterface method to retrieve the IDirectDraw, IDirectDraw2, or IDirectDraw3 interfaces.
		IID tmpID = (DirectXVersion == 1) ? IID_IDirectDraw :
			(DirectXVersion == 2) ? IID_IDirectDraw2 :
			(DirectXVersion == 3) ? IID_IDirectDraw3 :
			(DirectXVersion == 4) ? IID_IDirectDraw4 :
			(DirectXVersion == 7) ? IID_IDirectDraw7 : IID_IDirectDraw7;

		hr = ((IUnknown*)NewDD)->QueryInterface(tmpID, lplpDD);
		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get DirectDraw interface: " << (DDERR)hr);
			return hr;
		}

		((IUnknown*)NewDD)->Release();

		*lplpDD = ProxyAddressLookupTable.FindAddress<m_IDirectDraw7>(*lplpDD, DirectXVersion);
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

		DDSURFACEDESC2 Desc2 = {};
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);

		return SetSurfaceDesc2(&Desc2, dwFlags);
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
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		if (surface.Texture)
		{
			return surface.Texture->SetPrivateData(guidTag, lpData, cbSize, dwFlags);
		}
		else if (surface.Surface)
		{
			return surface.Surface->SetPrivateData(guidTag, lpData, cbSize, dwFlags);
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
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		if (surface.Texture)
		{
			return surface.Texture->GetPrivateData(guidTag, lpBuffer, lpcbBufferSize);
		}
		else if (surface.Surface)
		{
			return surface.Surface->GetPrivateData(guidTag, lpBuffer, lpcbBufferSize);
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
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		if (surface.Texture)
		{
			return surface.Texture->FreePrivateData(guidTag);
		}
		else if (surface.Surface)
		{
			return surface.Surface->FreePrivateData(guidTag);
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

		if (IsSurfaceBusy())
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
		UniquenessValue++;
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
	SetCriticalSection();
	if (SurfaceWrapperListV1.size())
	{
		WrapperInterface = SurfaceWrapperListV1.back();
		SurfaceWrapperListV1.pop_back();
		WrapperInterface->SetProxy(this);
	}
	else
	{
		WrapperInterface = new m_IDirectDrawSurface((LPDIRECTDRAWSURFACE)ProxyInterface, this);
	}
	ReleaseCriticalSection();
	WrapperInterface2 = new m_IDirectDrawSurface2((LPDIRECTDRAWSURFACE2)ProxyInterface, this);
	WrapperInterface3 = new m_IDirectDrawSurface3((LPDIRECTDRAWSURFACE3)ProxyInterface, this);
	WrapperInterface4 = new m_IDirectDrawSurface4((LPDIRECTDRAWSURFACE4)ProxyInterface, this);
	WrapperInterface7 = new m_IDirectDrawSurface7((LPDIRECTDRAWSURFACE7)ProxyInterface, this);

	if (!Config.Dd7to9)
	{
		return;
	}

	AddRef(DirectXVersion);

	InitializeCriticalSection(&ddscs);
	InitializeCriticalSection(&ddlcs);

	// Store surface, needs to run before InitSurfaceDesc()
	if (ddrawParent)
	{
		ddrawParent->AddSurfaceToVector(this);

		d3d9Device = ddrawParent->GetDirect3D9Device();
	}

	// Set Uniqueness Value
	UniquenessValue = 1;

	// Update surface description and create backbuffers
	InitSurfaceDesc(DirectXVersion);
}

inline void m_IDirectDrawSurfaceX::ReleaseDirectDrawResources()
{
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
}

void m_IDirectDrawSurfaceX::ReleaseSurface()
{
	// Don't delete surface wrapper v1 interface
	SetCriticalSection();
	WrapperInterface->SetProxy(nullptr);
	SurfaceWrapperListV1.push_back(WrapperInterface);
	ReleaseCriticalSection();
	WrapperInterface2->DeleteMe();
	WrapperInterface3->DeleteMe();
	WrapperInterface4->DeleteMe();
	WrapperInterface7->DeleteMe();

	if (!Config.Dd7to9 || Config.Exiting)
	{
		return;
	}

	ReleaseDirectDrawResources();

	ReleaseD9Surface(false, false);

	// Delete critical section last
	DeleteCriticalSection(&ddscs);
	DeleteCriticalSection(&ddlcs);
}

LPDIRECT3DSURFACE9 m_IDirectDrawSurfaceX::Get3DSurface()
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, true, true)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface not setup!");
		return nullptr;
	}

	return GetD3D9Surface();
}

LPDIRECT3DTEXTURE9 m_IDirectDrawSurfaceX::Get3DTexture()
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, true, true)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: texture not setup!");
		return nullptr;
	}
	return GetD3D9Texture();
}

inline LPDIRECT3DTEXTURE9 m_IDirectDrawSurfaceX::GetD3D9Texture()
{
	// Primary display texture
	if (PrimaryDisplayTexture)
	{
		if (IsPalette() && surface.IsUsingWindowedMode && (surface.DisplayTexture || !primary.PaletteTexture))
		{
			Logging::Log() << __FUNCTION__ << " Error: using non-shader palette surface on window mode not supported!";
		}
		return PrimaryDisplayTexture;
	}

	// Prepare paletted surface for display
	if (surface.IsPaletteDirty && !primary.PaletteTexture)
	{
		CopyEmulatedPaletteSurface(nullptr);
	}

	// Return palette display texture
	if (surface.DisplayTexture)
	{
		return surface.DisplayTexture;
	}

	// Return surface texture
	return surface.Texture;
}

inline LPDIRECT3DSURFACE9 m_IDirectDrawSurfaceX::GetD3D9Surface()
{
	if (surface.Texture)
	{
		if (surface.Context || (!surface.Context && SUCCEEDED(surface.Texture->GetSurfaceLevel(0, &surface.Context))))
		{
			return surface.Context;
		}
	}
	else if (surface.Surface)
	{
		return surface.Surface;
	}
	return nullptr;
}

HRESULT m_IDirectDrawSurfaceX::CheckInterface(char *FunctionName, bool CheckD3DDevice, bool CheckD3DSurface, bool CheckLostSurface)
{
	// Check ddrawParent device
	if (!ddrawParent)
	{
		if (DDrawVector.empty())
		{
			LOG_LIMIT(100, FunctionName << " Error: no ddraw parent!");
			return DDERR_INVALIDOBJECT;
		}
		ddrawParent = DDrawVector[0];
		ddrawParent->AddSurfaceToVector(this);
	}

	// Check d3d9 device
	if (CheckD3DDevice)
	{
		if (!ddrawParent->CheckD3D9Device() || !d3d9Device || !*d3d9Device)
		{
			LOG_LIMIT(100, FunctionName << " Error: d3d9 device not setup!");
			return DDERR_INVALIDOBJECT;
		}
	}

	// Check if device is lost
	if (CheckLostSurface)
	{
		if (d3d9Device && *d3d9Device)
		{
			HRESULT hr = (*d3d9Device)->TestCooperativeLevel();
			switch (hr)
			{
			case DD_OK:
			case DDERR_NOEXCLUSIVEMODE:
				break;
			case D3DERR_DEVICENOTRESET:
				if (SUCCEEDED(ddrawParent->ReinitDevice()))
				{
					break;
				}
				[[fallthrough]];
			case D3DERR_DEVICELOST:
				return DDERR_SURFACELOST;
			default:
				LOG_LIMIT(100, FunctionName << " Error: TestCooperativeLevel = " << (D3DERR)hr);
				return DDERR_WRONGMODE;
			}
		}
		if (surface.IsSurfaceLost)
		{
			LOG_LIMIT(100, FunctionName << " Warning: surface lost!");
			return DDERR_SURFACELOST;
		}
	}

	// Check surface
	if (CheckD3DSurface)
	{
		// Check if using Direct3D and remove emulated surface if not needed
		bool LastUsing3D = Using3D;
		Using3D = ddrawParent->IsUsing3D();
		if (Using3D && !LastUsing3D && IsUsingEmulation() && !Config.DdrawEmulateSurface && !SurfaceRequiresEmulation && !IsSurfaceBusy())
		{
			ReleaseDCSurface();
		}

		// Check if using windowed mode
		bool LastWindowedMode = surface.IsUsingWindowedMode;
		surface.IsUsingWindowedMode = !ddrawParent->IsExclusiveMode();

		// Make sure surface exists, if not then create it
		if ((!surface.Texture && !surface.Surface) ||
			(IsPrimaryOrBackBuffer() && LastWindowedMode != surface.IsUsingWindowedMode) ||
			(Using3D && PrimaryDisplayTexture))
		{
			if (FAILED(CreateD3d9Surface()))
			{
				LOG_LIMIT(100, FunctionName << " Error: d3d9 surface texture not setup!");
				return DDERR_WRONGMODE;
			}
		}
	}

	return DD_OK;
}

// Create surface
HRESULT m_IDirectDrawSurfaceX::CreateD3d9Surface()
{
	// Don't recreate surface while it is locked
	if ((surface.Texture || surface.Surface) && IsSurfaceBusy())
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface is busy! Locked: " << IsSurfaceLocked() << " DC: " << IsSurfaceInDC() << " Blt: " << IsSurfaceBlitting());
		return DDERR_GENERIC;
	}

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, false, false)))
	{
		return DDERR_GENERIC;
	}

	SetLockCriticalSection();

	// Release existing surface
	ReleaseD9Surface(true, false);

	// Update surface description
	UpdateSurfaceDesc();

	// Get texture format
	surfaceFormat = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);
	surfaceBitCount = GetBitCount(surfaceFormat);
	SurfaceRequiresEmulation = ((surfaceFormat == D3DFMT_A8B8G8R8 || surfaceFormat == D3DFMT_X8B8G8R8 || surfaceFormat == D3DFMT_B8G8R8 || surfaceFormat == D3DFMT_R8G8B8 ||
		Config.DdrawEmulateSurface || (Config.DdrawRemoveScanlines && IsPrimaryOrBackBuffer()) || ShouldEmulate == SC_FORCE_EMULATED) &&
			!IsDepthBuffer() && !(surfaceFormat & 0xFF000000 /*FOURCC or D3DFMT_DXTx*/));
	const bool IsSurfaceEmulated = (SurfaceRequiresEmulation || (IsPrimaryOrBackBuffer() && (Config.DdrawWriteToGDI || Config.DdrawReadFromGDI) && !Using3D));
	DCRequiresEmulation = (surfaceFormat != D3DFMT_R5G6B5 && surfaceFormat != D3DFMT_X1R5G5B5 && surfaceFormat != D3DFMT_R8G8B8 && surfaceFormat != D3DFMT_X8R8G8B8);
	const D3DFORMAT Format = ConvertSurfaceFormat(surfaceFormat);
	const D3DFORMAT TextureFormat = (surfaceFormat == D3DFMT_P8) ? D3DFMT_L8 : Format;

	// Get texture memory pool
	const D3DPOOL TexturePool = (IsPrimaryOrBackBuffer() && surface.IsUsingWindowedMode && !Using3D) ? D3DPOOL_SYSTEMMEM : IsPrimaryOrBackBuffer() ? D3DPOOL_MANAGED :
		(surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) ? D3DPOOL_SYSTEMMEM : D3DPOOL_MANAGED;

	// Adjust Width to be byte-aligned
	const DWORD Width = GetByteAlignedWidth(surfaceDesc2.dwWidth, surfaceBitCount);
	const DWORD Height = surfaceDesc2.dwHeight;

	// Set created by
	ShouldEmulate = (ShouldEmulate == SC_NOT_CREATED) ? SC_DONT_FORCE : ShouldEmulate;

	Logging::LogDebug() << __FUNCTION__ " (" << this << ") D3d9 Surface. Size: " << Width << "x" << Height << " Format: " << surfaceFormat << " dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps);

	HRESULT hr = DD_OK;

	do {
		// Create depth buffer
		if (IsDepthBuffer())
		{
			// ToDo: Get existing stencil surface rather than creating a new one
			if (FAILED(((*d3d9Device)->CreateDepthStencilSurface(Width, Height, Format, ddrawParent->GetMultiSampleType(), ddrawParent->GetMultiSampleQuality(), FALSE, &surface.Surface, nullptr))))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create depth buffer surface. Size: " << Width << "x" << Height << " Format: " << surfaceFormat << " dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps));
				hr = DDERR_GENERIC;
				break;
			}
		}
		// Create texture
		else
		{
			if (FAILED(((*d3d9Device)->CreateTexture(Width, Height, 1, 0, TextureFormat, TexturePool, &surface.Texture, nullptr))))
			{
				// Try failover format
				if (FAILED(((*d3d9Device)->CreateTexture(Width, Height, 1, 0, GetFailoverFormat(TextureFormat), TexturePool, &surface.Texture, nullptr))))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create surface texture. Size: " << Width << "x" << Height << " Format: " << surfaceFormat << " dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps));
					hr = DDERR_GENERIC;
					break;
				}
			}
		}

		// Create blank surface
		if (IsPrimarySurface() && !IsPalette())
		{
			if (FAILED(((*d3d9Device)->CreateOffscreenPlainSurface(Width, Height, Format, D3DPOOL_DEFAULT, &primary.BlankSurface, nullptr))))
			{
				// Try failover format
				if (FAILED(((*d3d9Device)->CreateOffscreenPlainSurface(Width, Height, GetFailoverFormat(Format), D3DPOOL_DEFAULT, &primary.BlankSurface, nullptr))))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create blank surface. Size: " << Width << "x" << Height << " Format: " << surfaceFormat << " dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps));
					hr = DDERR_GENERIC;
					break;
				}
			}
		}

		// Create primary surface texture
		if (IsPrimarySurface() && surface.IsUsingWindowedMode && !Using3D)
		{
			if (FAILED(((*d3d9Device)->CreateTexture(Width, Height, 1, 0, TextureFormat, D3DPOOL_DEFAULT, &PrimaryDisplayTexture, nullptr))))
			{
				// Try failover format
				if (FAILED(((*d3d9Device)->CreateTexture(Width, Height, 1, 0, GetFailoverFormat(TextureFormat), D3DPOOL_DEFAULT, &PrimaryDisplayTexture, nullptr))))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create primary surface texture. Size: " << Width << "x" << Height << " Format: " << surfaceFormat << " dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps));
					hr = DDERR_GENERIC;
					break;
				}
			}
		}

		// Create palette surface
		if (IsPrimarySurface() && surfaceFormat == D3DFMT_P8)
		{
			if (FAILED(((*d3d9Device)->CreateTexture(MaxPaletteSize, MaxPaletteSize, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &primary.PaletteTexture, nullptr))))
			{
				// Try failover format
				if (FAILED(((*d3d9Device)->CreateTexture(MaxPaletteSize, MaxPaletteSize, 1, 0, GetFailoverFormat(D3DFMT_X8R8G8B8), D3DPOOL_MANAGED, &primary.PaletteTexture, nullptr))))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create palette surface texture");
					hr = DDERR_GENERIC;
					break;
				}
			}
		}

		surface.IsPaletteDirty = IsPalette();

	} while (false);

	// Create emulated surface using device context for creation
	bool EmuSurfaceCreated = false;
	if ((IsSurfaceEmulated || IsUsingEmulation()) && !DoesDCMatch(surface.emu))
	{
		EmuSurfaceCreated = true;
		CreateDCSurface();
	}

	// Restore d3d9 surface texture data
	if (surface.Texture || surface.Surface)
	{
		bool RestoreData = false;
		if (IsUsingEmulation() && !EmuSurfaceCreated)
		{
			// Copy surface to emulated surface
			CopyFromEmulatedSurface(nullptr);
			RestoreData = true;
		}
		else if (!Backup.empty())
		{
			if (Backup.size() == GetSurfaceSize(surfaceFormat, surfaceDesc2.dwWidth, surfaceDesc2.dwHeight, surfaceDesc2.lPitch))
			{
				IDirect3DSurface9* pDestSurfaceD9 = GetD3D9Surface();
				if (pDestSurfaceD9)
				{
					// Copy backup data to surface
					RECT Rect = { 0, 0, (LONG)surfaceDesc2.dwWidth, (LONG)surfaceDesc2.dwHeight };
					if (SUCCEEDED(D3DXLoadSurfaceFromMemory(pDestSurfaceD9, nullptr, &Rect, Backup.data(), (surfaceFormat == D3DFMT_P8) ? D3DFMT_L8 : surfaceFormat, surfaceDesc2.lPitch, nullptr, &Rect, D3DX_FILTER_NONE, 0)))
					{
						// Copy surface to emulated surface
						if (IsUsingEmulation())
						{
							CopyToEmulatedSurface(&Rect);
						}
						RestoreData = true;
					}
					else
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to restore surface data!");
					}
				}
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: restore backup surface data size mismatch! Size: " << Backup.size() <<
					" Surface pitch: " << surfaceDesc2.lPitch << " " << surfaceDesc2.dwWidth << "x" << surfaceDesc2.dwHeight);
			}
		}

		// Copy surface to display texture
		if (RestoreData && PrimaryDisplayTexture)
		{
			IDirect3DSurface9* pSrcSurfaceD9 = GetD3D9Surface();
			if (pSrcSurfaceD9)
			{
				IDirect3DSurface9* pPrimaryDisplaySurfaceD9 = nullptr;
				if (SUCCEEDED(PrimaryDisplayTexture->GetSurfaceLevel(0, &pPrimaryDisplaySurfaceD9)))
				{
					D3DXLoadSurfaceFromSurface(pPrimaryDisplaySurfaceD9, nullptr, nullptr, pSrcSurfaceD9, nullptr, nullptr, D3DX_FILTER_NONE, 0);
					pPrimaryDisplaySurfaceD9->Release();
				}
			}
		}

		// Data is no longer needed
		Backup.clear();
	}

	ReleaseLockCriticalSection();

	return hr;
}

inline bool m_IDirectDrawSurfaceX::DoesDCMatch(EMUSURFACE* pEmuSurface)
{
	if (!pEmuSurface || !pEmuSurface->DC || !pEmuSurface->pBits)
	{
		return false;
	}

	// Adjust Width to be byte-aligned
	DWORD Width = GetByteAlignedWidth(surfaceDesc2.dwWidth, surfaceBitCount);
	DWORD Height = surfaceDesc2.dwHeight;
	DWORD Pitch = ComputePitch(Width, surfaceBitCount);

	if (pEmuSurface->bmi->bmiHeader.biWidth == (LONG)Width &&
		pEmuSurface->bmi->bmiHeader.biHeight == -(LONG)Height &&
		pEmuSurface->bmi->bmiHeader.biBitCount == surfaceBitCount &&
		pEmuSurface->Format == surfaceFormat &&
		pEmuSurface->Pitch == Pitch)
	{
		return true;
	}

	return false;
}

inline void m_IDirectDrawSurfaceX::SetEmulationGameDC()
{
	if (IsUsingEmulation() && !surface.emu->UsingGameDC)
	{
		// Restore old object into DC
		HGDIOBJ NewObject = SelectObject(surface.emu->DC, surface.emu->OldDCObject);
		if (!NewObject || NewObject == HGDI_ERROR)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to select old object into DC!";
			return;
		}
		// Select bitmap into GameDC
		surface.emu->OldGameDCObject = SelectObject(surface.emu->GameDC, surface.emu->bitmap);
		if (!surface.emu->OldGameDCObject || surface.emu->OldGameDCObject == HGDI_ERROR)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to select bitmap into GameDC!";
			return;
		}
		// Set DC flag
		surface.emu->UsingGameDC = true;
	}
}

inline void m_IDirectDrawSurfaceX::UnsetEmulationGameDC()
{
	if (IsUsingEmulation() && surface.emu->UsingGameDC)
	{
		// Restore old object into GameDC
		HGDIOBJ NewObject = SelectObject(surface.emu->GameDC, surface.emu->OldGameDCObject);
		if (!NewObject || NewObject == HGDI_ERROR)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to select old object into GameDC!";
			return;
		}
		// Select bitmap into DC
		surface.emu->OldDCObject = SelectObject(surface.emu->DC, surface.emu->bitmap);
		if (!surface.emu->OldDCObject || surface.emu->OldDCObject == HGDI_ERROR)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to select bitmap into GameDC!";
			return;
		}
		// Unset DC flag
		surface.emu->UsingGameDC = false;
	}
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
	if (surface.emu)
	{
		// Restore DC
		UnsetEmulationGameDC();

		// Check if emulated memory is good
		if (!IsUsingEmulation())
		{
			DeleteEmulatedMemory(&surface.emu);
		}
		else
		{
			// Check if current emulated surface is still ok
			if (DoesDCMatch(surface.emu))
			{
				return DD_OK;
			}

			// Save current emulated surface and prepare for creating a new one.
			if (ShareEmulatedMemory)
			{
				SetCriticalSection();
				memorySurfaces.push_back(surface.emu);
				surface.emu = nullptr;
				ReleaseCriticalSection();
			}
			else
			{
				DeleteEmulatedMemory(&surface.emu);
			}
		}
	}

	// If sharing memory than check the shared memory vector for a surface that matches
	if (ShareEmulatedMemory)
	{
		SetCriticalSection();
		for (auto it = memorySurfaces.begin(); it != memorySurfaces.end(); it++)
		{
			EMUSURFACE* pEmuSurface = *it;

			if (DoesDCMatch(pEmuSurface))
			{
				surface.emu = pEmuSurface;

				it = memorySurfaces.erase(it);

				break;
			}
		}
		ReleaseCriticalSection();

		if (surface.emu && surface.emu->pBits)
		{
			ZeroMemory(surface.emu->pBits, surface.emu->Size);

			return DD_OK;
		}
	}

	Logging::LogDebug() << __FUNCTION__ " (" << this << ") creating emulated surface. Size: " << Width << "x" << Height << " Format: " << surfaceFormat << " dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps);

	// Create new emulated surface structure
	surface.emu = new EMUSURFACE;

	// Create device context memory
	ZeroMemory(surface.emu->bmiMemory, sizeof(surface.emu->bmiMemory));
	surface.emu->bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	surface.emu->bmi->bmiHeader.biWidth = Width;
	surface.emu->bmi->bmiHeader.biHeight = -((LONG)Height + 200);
	surface.emu->bmi->bmiHeader.biPlanes = 1;
	surface.emu->bmi->bmiHeader.biBitCount = (WORD)surfaceBitCount;
	surface.emu->bmi->bmiHeader.biCompression =
		(surfaceBitCount == 8 || surfaceBitCount == 24) ? BI_RGB :
		(ColorMaskReq) ? BI_BITFIELDS : 0;	// BI_BITFIELDS is only valid for 16-bpp and 32-bpp bitmaps.
	surface.emu->bmi->bmiHeader.biSizeImage = ((Width * surfaceBitCount + 31) & ~31) / 8 * Height;

	if (surfaceBitCount == 8)
	{
		for (int i = 0; i < 256; i++)
		{
			surface.emu->bmi->bmiColors[i].rgbRed = (byte)i;
			surface.emu->bmi->bmiColors[i].rgbGreen = (byte)i;
			surface.emu->bmi->bmiColors[i].rgbBlue = (byte)i;
			surface.emu->bmi->bmiColors[i].rgbReserved = 0;
		}
	}
	else if (ColorMaskReq)
	{
		((DWORD*)surface.emu->bmi->bmiColors)[0] = surfaceDesc2.ddpfPixelFormat.dwRBitMask;
		((DWORD*)surface.emu->bmi->bmiColors)[1] = surfaceDesc2.ddpfPixelFormat.dwGBitMask;
		((DWORD*)surface.emu->bmi->bmiColors)[2] = surfaceDesc2.ddpfPixelFormat.dwBBitMask;
		((DWORD*)surface.emu->bmi->bmiColors)[3] = surfaceDesc2.ddpfPixelFormat.dwRGBAlphaBitMask;
	}
	else
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set bmi colors! " << surfaceBitCount);
		DeleteEmulatedMemory(&surface.emu);
		return DDERR_GENERIC;
	}
	HDC hDC = ddrawParent->GetDC();
	surface.emu->DC = CreateCompatibleDC(hDC);
	surface.emu->GameDC = CreateCompatibleDC(hDC);
	if (!surface.emu->DC || !surface.emu->GameDC)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create compatible DC: " << hDC << " " << surfaceFormat);
		DeleteEmulatedMemory(&surface.emu);
		return DDERR_GENERIC;
	}
	surface.emu->bitmap = CreateDIBSection(surface.emu->DC, surface.emu->bmi, (surfaceBitCount == 8) ? DIB_PAL_COLORS : DIB_RGB_COLORS, (void**)&surface.emu->pBits, nullptr, 0);
	if (!surface.emu->bitmap)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create bitmap!");
		DeleteEmulatedMemory(&surface.emu);
		return DDERR_GENERIC;
	}
	surface.emu->OldDCObject = SelectObject(surface.emu->DC, surface.emu->bitmap);
	if (!surface.emu->OldDCObject || surface.emu->OldDCObject == HGDI_ERROR)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to replace object in DC!");
		DeleteEmulatedMemory(&surface.emu);
		return DDERR_GENERIC;
	}
	surface.emu->bmi->bmiHeader.biHeight = -(LONG)Height;
	surface.emu->Format = surfaceFormat;
	surface.emu->Pitch = ComputePitch(surface.emu->bmi->bmiHeader.biWidth, surface.emu->bmi->bmiHeader.biBitCount);
	surface.emu->Size = Height * surface.emu->Pitch;

	return DD_OK;
}

// Update surface description
void m_IDirectDrawSurfaceX::UpdateSurfaceDesc()
{
	if ((surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT)) != (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT))
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, false, false)))
		{
			return;
		}

		// Get resolution
		DWORD Width, Height, RefreshRate, BPP;
		ddrawParent->GetSurfaceDisplay(Width, Height, BPP, RefreshRate);

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
		if (RefreshRate && ((surfaceDesc2.dwFlags & DDSD_REFRESHRATE) || IsPrimaryOrBackBuffer()))
		{
			surfaceDesc2.dwFlags |= DDSD_REFRESHRATE;
			surfaceDesc2.dwRefreshRate = RefreshRate;
		}
		// Set PixelFormat
		if (BPP && !(surfaceDesc2.dwFlags & DDSD_PIXELFORMAT))
		{
			ResetDisplayFlags |= DDSD_PIXELFORMAT | DDSD_PITCH;
			surfaceDesc2.dwFlags |= DDSD_PIXELFORMAT;
			ddrawParent->GetDisplayPixelFormat(surfaceDesc2.ddpfPixelFormat, BPP);
			surfaceDesc2.lPitch = 0;
		}
	}
	// Unset lPitch
	if ((surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT)) != (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT) ||
		!surfaceDesc2.lPitch || !(surfaceDesc2.dwFlags & DDSD_PITCH))
	{
		surfaceDesc2.dwFlags &= ~DDSD_PITCH;
		surfaceDesc2.lPitch = 0;
	}
	// Set lPitch
	if ((surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT)) == (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT) &&
		(surfaceDesc2.ddpfPixelFormat.dwFlags & DDPF_RGB) && !(surfaceDesc2.dwFlags & DDSD_LINEARSIZE) && !(surfaceDesc2.dwFlags & DDSD_PITCH))
	{
		surfaceDesc2.dwFlags |= DDSD_PITCH;
		DWORD BitCount = BitCount = GetBitCount(surfaceDesc2.ddpfPixelFormat);
		surfaceDesc2.lPitch = ComputePitch(GetByteAlignedWidth(surfaceDesc2.dwWidth, BitCount), BitCount);
	}
	// Set surface format
	if (surfaceFormat == D3DFMT_UNKNOWN && (surfaceDesc2.dwFlags & DDSD_PIXELFORMAT))
	{
		surfaceFormat = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);
	}
}

// Release surface and vertext buffer
void m_IDirectDrawSurfaceX::ReleaseD9ContextSurface()
{
	// Release primary display texture
	if (PrimaryDisplayTexture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 primary display texture";
		ULONG ref = PrimaryDisplayTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'PrimaryDisplayTexture' " << ref;
		}
		PrimaryDisplayTexture = nullptr;
	}

	// Release d3d9 context surface
	if (surface.Context)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 context surface";
		ULONG ref = surface.Context->Release();
		if (ref > 1)	// Ref count is higher becasue it is a surface of 'surfaceTexture'
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'contextSurface' " << ref;
		}
		surface.Context = nullptr;
	}

	// Release d3d9 palette context texture
	if (surface.DisplayContext)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 palette context texture";
		ULONG ref = surface.DisplayContext->Release();
		if (ref > 1)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'paletteDisplaySurface' " << ref;
		}
		surface.DisplayContext = nullptr;
	}

	// Release d3d9 palette display texture
	if (surface.DisplayTexture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 palette display texture";
		ULONG ref = surface.DisplayTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'paletteDisplayTexture' " << ref;
		}
		surface.DisplayTexture = nullptr;
	}

	// Release blank surface
	if (primary.BlankSurface)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 blank surface";
		ULONG ref = primary.BlankSurface->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'blankSurface' " << ref;
		}
		primary.BlankSurface = nullptr;
	}

	// Release d3d9 palette surface texture
	if (primary.PaletteTexture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 palette texture surface";
		if (d3d9Device && *d3d9Device)
		{
			(*d3d9Device)->SetTexture(1, nullptr);
			(*d3d9Device)->SetPixelShader(nullptr);
		}
		ULONG ref = primary.PaletteTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'paletteTexture' " << ref;
		}
		primary.PaletteTexture = nullptr;
	}
}

// Release surface and vertext buffer
void m_IDirectDrawSurfaceX::ReleaseD9Surface(bool BackupData, bool DeviceLost)
{
	SetLockCriticalSection();

	// Check if surface is busy
	if (IsSurfaceBusy())
	{
		Logging::Log() << __FUNCTION__ << " Warning: surface still in use! Locked: " << IsSurfaceLocked() << " DC: " << IsSurfaceInDC() << " Blt: " << IsSurfaceBlitting();
	}

	// Release DC (before releasing surface)
	if (IsSurfaceInDC() || LastDC)
	{
		if (LastDC)
		{
			ReleaseDC(LastDC);
			LastDC = nullptr;
		}
		IsInDC = false;
	}

	// Restore DC
	UnsetEmulationGameDC();

	// Unlock surface (before releasing)
	if (IsSurfaceLocked())
	{
		UnlockD39Surface();
		IsLocked = false;
	}
	LockedWithID = 0;

	// Backup d3d9 surface texture
	if (BackupData)
	{
		if (surface.SurfaceHasData)
		{
			if (DeviceLost)
			{
				surface.IsSurfaceLost = true;
			}
			if (!IsUsingEmulation() && (surface.Texture || surface.Surface))
			{
				D3DLOCKED_RECT LockRect = {};
				if (SUCCEEDED(LockD39Surface(&LockRect, nullptr, D3DLOCK_READONLY)))
				{
					Logging::LogDebug() << __FUNCTION__ << " Storing Direct3D9 texture surface data: " << surfaceFormat;

					size_t size = GetSurfaceSize(surfaceFormat, surfaceDesc2.dwWidth, surfaceDesc2.dwHeight, LockRect.Pitch);

					if (size)
					{
						Backup.resize(size);

						memcpy(Backup.data(), LockRect.pBits, size);
					}

					UnlockD39Surface();
				}
				else
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to backup surface data!");
				}
			}
		}
	}
	// Emulated surface
	else if (IsUsingEmulation())
	{
		ReleaseDCSurface();
	}

	ReleaseD9ContextSurface();

	// Release d3d9 3D surface
	if (surface.Surface)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 surface";
		ULONG ref = surface.Surface->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'surface3D' " << ref;
		}
		surface.Surface = nullptr;
	}

	// Release d3d9 surface texture
	if (surface.Texture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 texture surface";
		ULONG ref = surface.Texture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'surfaceTexture' " << ref;
		}
		surface.Texture = nullptr;
	}

	// Clear locked rects
	LockRectList.clear();

	// Reset scanline flags
	LastLock.LockedRect.pBits = nullptr;
	LastLock.bEvenScanlines = false;
	LastLock.bOddScanlines = false;

	// Reset shader flag
	ShaderColorKey.IsSet = false;
	primary.ShaderColorKey.IsSet = false;

	// Reset display flags
	if (ResetDisplayFlags)
	{
		surfaceDesc2.dwFlags &= ~ResetDisplayFlags;
	}

	ReleaseLockCriticalSection();
}

// Release emulated surface
inline void m_IDirectDrawSurfaceX::ReleaseDCSurface()
{
	if (surface.emu)
	{
		if (!ShareEmulatedMemory || !IsUsingEmulation())
		{
			DeleteEmulatedMemory(&surface.emu);
		}
		else
		{
			SetCriticalSection();
			memorySurfaces.push_back(surface.emu);
			surface.emu = nullptr;
			ReleaseCriticalSection();
		}
	}
}

HRESULT m_IDirectDrawSurfaceX::ClearPrimarySurface()
{
	if (!IsPrimarySurface())
	{
		return DDERR_GENERIC;
	}

	SetLockCriticalSection();

	HRESULT hr = DD_OK;

	do {

		if (IsUsingEmulation() || IsPalette())
		{
			hr = ColorFill(nullptr, 0x00000000);

			if (FAILED(hr))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not color fill surface. " << (D3DERR)hr);
			}

			break;
		}

		IDirect3DSurface9* pDestSurfaceD9 = GetD3D9Surface();

		if (!primary.BlankSurface || !pDestSurfaceD9)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surface. " << primary.BlankSurface << "->" << pDestSurfaceD9);
			hr = DDERR_GENERIC;
			break;
		}

		hr = D3DXLoadSurfaceFromSurface(pDestSurfaceD9, nullptr, nullptr, primary.BlankSurface, nullptr, nullptr, D3DX_FILTER_POINT, 0);

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not load source from surface. " << (D3DERR)hr);
		}

	} while (false);

	ReleaseLockCriticalSection();

	return hr;
}

// Present surface
HRESULT m_IDirectDrawSurfaceX::PresentSurface(bool IsSkipScene)
{
	// Check for device interface
	HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
	if (FAILED(c_hr))
	{
		return c_hr;
	}

	// Check if is not primary surface or if scene should be skipped
	if (Config.DdrawWriteToGDI || surface.IsUsingWindowedMode || Using3D)
	{
		// Never present when using Direct3D or when writing to GDI
		return DD_OK;
	}
	else if (!IsPrimarySurface())
	{
		if (SceneReady && !IsPresentRunning)
		{
			m_IDirectDrawSurfaceX* lpDDSrcSurfaceX = ddrawParent->GetPrimarySurface();
			if (lpDDSrcSurfaceX)
			{
				return lpDDSrcSurfaceX->PresentSurface(IsSkipScene);
			}
		}
		return DDERR_GENERIC;
	}
	else if ((IsSkipScene && !SceneReady) || IsPresentRunning)
	{
		Logging::LogDebug() << __FUNCTION__ << " Skipping scene!";
		return DDERR_GENERIC;
	}

	// Set scene ready
	SceneReady = true;

	// Check if surface is locked or has an open DC
	if (IsSurfaceBusy())
	{
		Logging::LogDebug() << __FUNCTION__ << " Surface is busy!";
		return DDERR_SURFACEBUSY;
	}

	// Set present flag
	IsPresentRunning = true;

	// If texture is not dirty then mark it as dirty in case the game wrote to the memory directly (Nox does this)
	if (!surface.IsDirtyFlag)
	{
		if (IsUsingEmulation())
		{
			CopyFromEmulatedSurface(nullptr);
		}
		else
		{
			LPDIRECT3DTEXTURE9 displayTexture = GetD3D9Texture();
			if (displayTexture)
			{
				displayTexture->AddDirtyRect(nullptr);
			}
		}
		SetDirtyFlag();
	}

	// Present to d3d9
	HRESULT hr = DD_OK;
	if (FAILED(ddrawParent->Present2DScene(this, nullptr, nullptr)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to present 2D scene!");
		hr = DDERR_GENERIC;
	}

	// Reset present flag
	IsPresentRunning = false;

	return hr;
}

// Reset primary surface display settings
void m_IDirectDrawSurfaceX::ResetSurfaceDisplay()
{
	if (ResetDisplayFlags)
	{
		ReleaseD9Surface(true, false);
	}
}

// Check surface reck dimensions and copy rect to new rect
inline bool m_IDirectDrawSurfaceX::CheckCoordinates(RECT& OutRect, LPRECT lpInRect)
{
	// Check device coordinates
	if (!surfaceDesc2.dwWidth || !surfaceDesc2.dwHeight)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface has no size!");
		return false;
	}

	if (lpInRect)
	{
		OutRect.left = lpInRect->left;
		OutRect.top = lpInRect->top;
		OutRect.right = lpInRect->right;
		OutRect.bottom = lpInRect->bottom;
	}
	else
	{
		OutRect.left = 0;
		OutRect.top = 0;
		OutRect.right = surfaceDesc2.dwWidth;
		OutRect.bottom = surfaceDesc2.dwHeight;
	}

	if (OutRect.left < 0)
	{
		OutRect.left = 0;
	}

	if (OutRect.right < 0)
	{
		OutRect.right = 0;
	}

	if (OutRect.top < 0)
	{
		OutRect.top = 0;
	}

	if (OutRect.bottom < 0)
	{
		OutRect.bottom = 0;
	}

	if (OutRect.left > (LONG)surfaceDesc2.dwWidth)
	{
		OutRect.left = surfaceDesc2.dwWidth;
	}

	if (OutRect.right > (LONG)surfaceDesc2.dwWidth)
	{
		OutRect.right = surfaceDesc2.dwWidth;
	}

	if (OutRect.top > (LONG)surfaceDesc2.dwHeight)
	{
		OutRect.top = surfaceDesc2.dwHeight;
	}

	if (OutRect.bottom > (LONG)surfaceDesc2.dwHeight)
	{
		OutRect.bottom = surfaceDesc2.dwHeight;
	}

	return OutRect.left < OutRect.right && OutRect.top < OutRect.bottom;
}

// Fix issue with some games that ignore the pitch size
void m_IDirectDrawSurfaceX::LockEmuLock(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc)
{
	// Only works if entire surface is locked
	if (!lpDDSurfaceDesc || !lpDDSurfaceDesc->lPitch || lpDestRect)
	{
		return;
	}

	DWORD BBP = GetBitCount(lpDDSurfaceDesc->ddpfPixelFormat);
	LONG NewPitch = (BBP / 8) * lpDDSurfaceDesc->dwWidth;

	bool LockOffPlain = (Config.DdrawEmulateLock && (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_OFFSCREENPLAIN));
	bool LockByteAlign = (Config.DdrawFixByteAlignment && lpDDSurfaceDesc->lPitch != NewPitch);

	// Emulate lock for offscreen surfaces
	if ((BBP == 8 || BBP == 16 || BBP == 24 || BBP == 32) && (LockOffPlain || LockByteAlign))
	{
		// Set correct pitch
		NewPitch = LockByteAlign ? NewPitch : lpDDSurfaceDesc->lPitch;

		// Store old variables
		EmuLock.Locked = true;
		EmuLock.Addr = lpDDSurfaceDesc->lpSurface;
		EmuLock.Pitch = lpDDSurfaceDesc->lPitch;
		EmuLock.NewPitch = NewPitch;
		EmuLock.BBP = BBP;
		EmuLock.Width = lpDDSurfaceDesc->dwWidth;
		EmuLock.Height = lpDDSurfaceDesc->dwHeight;

		// Update surface memory and pitch
		size_t Size = NewPitch * lpDDSurfaceDesc->dwHeight;
		if (EmuLock.Mem.size() < Size)
		{
			EmuLock.Mem.resize(Size);
		}
		lpDDSurfaceDesc->lpSurface = EmuLock.Mem.data();
		lpDDSurfaceDesc->lPitch = NewPitch;

		// Copy surface data to memory
		BYTE* InAddr = (BYTE*)EmuLock.Addr;
		DWORD InPitch = EmuLock.Pitch;
		BYTE* OutAddr = EmuLock.Mem.data();
		DWORD OutPitch = EmuLock.NewPitch;
		if (InPitch == OutPitch)
		{
			memcpy(OutAddr, InAddr, OutPitch * EmuLock.Height);
		}
		else
		{
			for (DWORD x = 0; x < EmuLock.Height; x++)
			{
				memcpy(OutAddr, InAddr, OutPitch);
				InAddr += InPitch;
				OutAddr += OutPitch;
			}
		}
	}
}

// Fix issue with some games that ignore the pitch size
void m_IDirectDrawSurfaceX::UnlockEmuLock()
{
	if (EmuLock.Locked && EmuLock.Addr)
	{
		// Copy memory back to surface
		BYTE* InAddr = EmuLock.Mem.data();
		DWORD InPitch = EmuLock.NewPitch;
		BYTE* OutAddr = (BYTE*)EmuLock.Addr;
		DWORD OutPitch = EmuLock.Pitch;
		if (InPitch == OutPitch)
		{
			memcpy(OutAddr, InAddr, OutPitch * EmuLock.Height);
		}
		else
		{
			for (DWORD x = 0; x < EmuLock.Height; x++)
			{
				memcpy(OutAddr, InAddr, OutPitch);
				InAddr += InPitch;
				OutAddr += OutPitch;
			}
		}

		EmuLock.Locked = false;
		EmuLock.Addr = nullptr;
	}
}

// Restore removed scanlines before locking surface
void m_IDirectDrawSurfaceX::RestoreScanlines(LASTLOCK& LLock)
{
	DWORD ByteCount = surfaceBitCount / 8;
	DWORD RectWidth = LLock.Rect.right - LLock.Rect.left;
	DWORD RectHeight = LLock.Rect.bottom - LLock.Rect.top;

	if (!IsPrimaryOrBackBuffer() || !LLock.LockedRect.pBits ||
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
			memcpy(DestBuffer, LLock.EvenScanLine.data(), size);
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
			memcpy(DestBuffer, LLock.OddScanLine.data(), size);
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

	if (!IsPrimaryOrBackBuffer() || !LLock.LockedRect.pBits ||
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
				memcpy(LLock.EvenScanLine.data(), DestBuffer, size);
			}
			else if (LLock.bEvenScanlines)
			{
				LLock.bEvenScanlines = (memcmp(LLock.EvenScanLine.data(), DestBuffer, size) == 0);
			}
		}
		// Check for odd scanlines
		else
		{
			if (y == 1)
			{
				LLock.bOddScanlines = true;
				memcpy(LLock.OddScanLine.data(), DestBuffer, size);
			}
			else if (LLock.bOddScanlines)
			{
				LLock.bOddScanlines = (memcmp(LLock.OddScanLine.data(), DestBuffer, size) == 0);
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

inline HRESULT m_IDirectDrawSurfaceX::LockEmulatedSurface(D3DLOCKED_RECT* pLockedRect, LPRECT lpDestRect)
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

	pLockedRect->Pitch = surface.emu->Pitch;
	pLockedRect->pBits = (lpDestRect) ? (void*)((DWORD)surface.emu->pBits + ((lpDestRect->top * pLockedRect->Pitch) + (lpDestRect->left * (surfaceBitCount / 8)))) : surface.emu->pBits;

	return DD_OK;
}

// Set dirty flag
inline void m_IDirectDrawSurfaceX::SetDirtyFlag()
{
	if (IsPrimarySurface())
	{
		dirtyFlag = true;
	}
	surface.IsDirtyFlag = true;
	surface.SurfaceHasData = true;

	// Update Uniqueness Value
	ChangeUniquenessValue();
}

void m_IDirectDrawSurfaceX::ClearDirtyFlags()
{
	// Reset dirty flag
	dirtyFlag = false;
	surface.IsDirtyFlag = false;

	// Reset scene ready
	SceneReady = false;
}

// Check if rect is a single line and should be skipped
inline bool m_IDirectDrawSurfaceX::CheckRectforSkipScene(RECT& DestRect)
{
	bool isSingleLine = (DestRect.bottom - DestRect.top == 1);	// Only handles horizontal lines at this point

	return Config.DdrawRemoveInterlacing ? isSingleLine : false;
}

inline void m_IDirectDrawSurfaceX::BeginWritePresent(bool IsSkipScene)
{
	// Check if data needs to be presented before write
	if (dirtyFlag)
	{
		if (FAILED(PresentSurface(IsSkipScene)))
		{
			PresentOnUnlock = true;
		}
	}
}

inline void m_IDirectDrawSurfaceX::EndWritePresent(bool IsSkipScene)
{
	// Present surface after each draw unless removing interlacing
	if (PresentOnUnlock || !Config.DdrawRemoveInterlacing)
	{
		PresentSurface(IsSkipScene);
	}

	// Reset endscene lock
	PresentOnUnlock = false;
}

// Update surface description and create backbuffers
inline void m_IDirectDrawSurfaceX::InitSurfaceDesc(DWORD DirectXVersion)
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
		DDSURFACEDESC2 Desc2 = surfaceDesc2;
		Desc2.ddsCaps.dwCaps4 &= ~(DDSCAPS4_CREATESURFACE);	// Clear surface creation flag
		Desc2.dwBackBufferCount--;
		if (Desc2.ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER)
		{
			Desc2.ddsCaps.dwCaps &= ~(DDSCAPS_VISIBLE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FRONTBUFFER);
			Desc2.ddsCaps.dwCaps |= DDSCAPS_BACKBUFFER;
			Desc2.dwReserved = (DWORD)this;
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

	return (std::find_if(AttachedSurfaceMap.begin(), AttachedSurfaceMap.end(),
		[=](auto Map) -> bool { return Map.second.pSurface == lpSurfaceX; }) != std::end(AttachedSurfaceMap));
}

bool m_IDirectDrawSurfaceX::WasAttachedSurfaceAdded(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return false;
	}

	return (std::find_if(AttachedSurfaceMap.begin(), AttachedSurfaceMap.end(),
		[=](auto Map) -> bool { return (Map.second.pSurface == lpSurfaceX) && Map.second.isAttachedSurfaceAdded; }) != std::end(AttachedSurfaceMap));
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
	HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
	if (FAILED(c_hr))
	{
		return c_hr;
	}

	// Check and copy rect
	RECT DestRect = {};
	if (!CheckCoordinates(DestRect, pRect))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid rect: " << pRect);
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

		DWORD BitCount = GetBitCount(Desc.Format);
		DWORD ByteCount = BitCount / 8;

		if (BitCount != 8 && BitCount != 12 && BitCount != 16 && BitCount != 24 && BitCount != 32)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid bit count: " << BitCount);
			return DDERR_GENERIC;
		}

		// Handle 12-bit surface
		if (BitCount == 12)
		{
			ByteCount = 3;
			dwFillColor = (dwFillColor & 0xFFF) + ((dwFillColor & 0xFFF) << 12);
		}

		D3DCOLOR ColorSurface[9] = {};	// Nine DWORDs for a byte aligned 3x3 surface (can handle all bit counts)

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

		LONG Pitch = 12;
		RECT SrcRect = { 0, 0, (BitCount == 12) ? 8 : Pitch / (LONG)ByteCount, 3 };

		if (FAILED(D3DXLoadSurfaceFromMemory(pDestSurfaceD9, nullptr, &DestRect, ColorSurface, Desc.Format, Pitch, nullptr, &SrcRect, D3DX_FILTER_POINT, 0)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not color fill surface! " << Desc.Format);
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

		// Get width and height of rect
		LONG FillWidth = DestRect.right - DestRect.left;
		LONG FillHeight = DestRect.bottom - DestRect.top;

		if ((DWORD)FillWidth == surfaceDesc2.dwWidth && surfaceBitCount == 8)
		{
			memset(DestLockRect.pBits, dwFillColor, DestLockRect.Pitch * FillHeight);
		}
		else if ((DWORD)FillWidth == surfaceDesc2.dwWidth && (surfaceBitCount == 16 || surfaceBitCount == 32) && (DestLockRect.Pitch * FillHeight) % 4 == 0)
		{
			const DWORD Color = (surfaceBitCount == 16) ? ((dwFillColor & 0xFFFF) << 16) + (dwFillColor & 0xFFFF) : dwFillColor;
			const DWORD size = (DestLockRect.Pitch * FillHeight) / 4;

			DWORD* DestBuffer = (DWORD*)DestLockRect.pBits;
			for (UINT x = 0; x < size; x++)
			{
				DestBuffer[x] = Color;
			}
		}
		else if (surfaceBitCount == 8 || (surfaceBitCount == 12 && FillWidth % 2 == 0) || surfaceBitCount == 16 || surfaceBitCount == 24 || surfaceBitCount == 32)
		{
			// Set memory address
			BYTE* SrcBuffer = (BYTE*)&dwFillColor;
			BYTE* DestBuffer = (BYTE*)DestLockRect.pBits;

			// Get byte count
			DWORD ByteCount = surfaceBitCount / 8;

			// Handle 12-bit surface
			if (surfaceBitCount == 12)
			{
				ByteCount = 3;
				dwFillColor = (dwFillColor & 0xFFF) + ((dwFillColor & 0xFFF) << 12);
				FillWidth /= 2;
			}

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
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid bit count: " << surfaceBitCount << " Width: " << FillWidth);
			return DDERR_GENERIC;
		}

		// Copy emulated surface to real texture
		CopyFromEmulatedSurface(&DestRect);

		// Blt surface directly to GDI
		if (Config.DdrawWriteToGDI && IsPrimarySurface() && !Using3D)
		{
			CopyEmulatedSurfaceToGDI(DestRect);
		}
	}

	// Preset surface to window
	if (surface.IsUsingWindowedMode && IsPrimarySurface() && !Config.DdrawWriteToGDI && !Using3D)
	{
		PresentSurfaceToWindow(DestRect);
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
	// Check parameters
	if (!pSourceSurface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid parameters!");
		return DDERR_INVALIDPARAMS;
	}

	// Check for device interface
	HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
	HRESULT s_hr = (pSourceSurface == this) ? c_hr : pSourceSurface->CheckInterface(__FUNCTION__, true, true, true);
	if (FAILED(c_hr) || FAILED(s_hr))
	{
		return (c_hr == DDERR_SURFACELOST || s_hr == DDERR_SURFACELOST) ? DDERR_SURFACELOST : FAILED(c_hr) ? c_hr : s_hr;
	}

	// Copy rect and do clipping
	RECT SrcRect = (pSourceRect) ? *pSourceRect : pSourceSurface->GetSurfaceRect();
	RECT DestRect = (pDestRect) ? *pDestRect : GetSurfaceRect();
	LONG Left = min(SrcRect.left, DestRect.left);
	if (Left < 0)
	{
		SrcRect.left -= Left;
		DestRect.left -= Left;
	}
	LONG Top = min(SrcRect.top, DestRect.top);
	if (Top < 0)
	{
		SrcRect.top -= Top;
		DestRect.top -= Top;
	}

	// Get source and dest format
	const D3DFORMAT SrcFormat = pSourceSurface->GetSurfaceFormat();
	const D3DFORMAT DestFormat = GetSurfaceFormat();

	// Get copy flags
	const bool IsStretchRect =
		abs((SrcRect.right - SrcRect.left) - (DestRect.right - DestRect.left)) > 1 ||		// Width size
		abs((SrcRect.bottom - SrcRect.top) - (DestRect.bottom - DestRect.top)) > 1;			// Height size
	const bool IsColorKey = ((dwFlags & BLT_COLORKEY) != 0);
	const bool IsMirrorLeftRight = ((dwFlags & BLT_MIRRORLEFTRIGHT) != 0);
	const bool IsMirrorUpDown = ((dwFlags & BLT_MIRRORUPDOWN) != 0);
	const DWORD D3DXFilter =
		(IsStretchRect && IsPalette()) || (Filter & D3DTEXF_POINT) ? D3DX_FILTER_POINT :	// Force palette surfaces to use point filtering to prevent color banding
		(Filter & D3DTEXF_LINEAR) ? D3DX_FILTER_LINEAR :									// Use linear filtering when requested by the application
		(IsStretchRect) ? D3DX_FILTER_POINT :												// Default to point filtering when stretching the rect, same as DirectDraw
		D3DX_FILTER_NONE;

	// Check rect and do clipping
	if (!pSourceSurface->CheckCoordinates(SrcRect, &SrcRect) || !CheckCoordinates(DestRect, &DestRect))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << pSourceRect << " -> " << pDestRect);
		return DDERR_INVALIDRECT;
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
		DestRectWidth = SrcRectWidth;
		DestRectHeight = SrcRectHeight;
		SrcRect.right = SrcRect.left + SrcRectWidth;
		SrcRect.bottom = SrcRect.top + SrcRectHeight;
		DestRect.right = DestRect.left + DestRectWidth;
		DestRect.bottom = DestRect.top + DestRectHeight;
	}

	// Read surface from GDI
	if (Config.DdrawReadFromGDI && IsPrimaryOrBackBuffer() && !Using3D)
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
		if (IsEmulationDCReady() && pSourceSurface->IsEmulationDCReady() && !IsColorKey)
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
				GetBrushOrgEx(surface.emu->DC, &org);
				SetStretchBltMode(surface.emu->DC, (Filter & D3DTEXF_LINEAR) ? HALFTONE : COLORONCOLOR);
				SetBrushOrgEx(surface.emu->DC, org.x, org.y, nullptr);
			}

			if ((IsStretchRect || IsMirrorLeftRight || IsMirrorUpDown) ?
				StretchBlt(surface.emu->DC, DestLeft, DestTop, DestWidth, DestHeight,
					pSourceSurface->surface.emu->DC, SrcRect.left, SrcRect.top, SrcRect.right - SrcRect.left, SrcRect.bottom - SrcRect.top, SRCCOPY) :
				BitBlt(surface.emu->DC, DestRect.left, DestRect.top, DestRectWidth, DestRectHeight,
					pSourceSurface->surface.emu->DC, SrcRect.left, SrcRect.top, SRCCOPY))
			{
				break;
			}
		}

		// Use D3DXLoadSurfaceFromSurface to copy the surface
		if (!IsUsingEmulation() && !IsColorKey && !IsMirrorLeftRight && !IsMirrorUpDown &&
			((!pSourceSurface->IsPalette() && !IsPalette()) || (pSourceSurface->IsPalette() && IsPalette())))
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
		if (!ByteCount || ByteCount > 4 || DestBitCount % 8 != 0)
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
			hr = (pSourceSurface->IsSurfaceBusy()) ? DDERR_SURFACEBUSY : DDERR_GENERIC;
			break;
		}
		UnlockSrc = true;

		// Check if source and destination memory addresses are overlapping
		if (this == pSourceSurface)
		{
			size_t size = SrcRectWidth * ByteCount * SrcRectHeight;
			if (size > ByteArray.size())
			{
				ByteArray.resize(size);
			}
			BYTE* SrcBuffer = (BYTE*)SrcLockRect.pBits;
			BYTE* DestBuffer = (BYTE*)ByteArray.data();
			INT DestPitch = SrcRectWidth * ByteCount;
			for (LONG y = 0; y < SrcRectHeight; y++)
			{
				memcpy(DestBuffer, SrcBuffer, SrcRectWidth * ByteCount);
				SrcBuffer += SrcLockRect.Pitch;
				DestBuffer += DestPitch;
			}
			SrcLockRect.pBits = ByteArray.data();
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
			hr = (IsSurfaceLocked()) ? DDERR_SURFACEBUSY : DDERR_GENERIC;
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
			if (!IsMirrorUpDown && SrcLockRect.Pitch == DestLockRect.Pitch && (DWORD)DestRectWidth == surfaceDesc2.dwWidth)
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
						BYTE PixelColor = SrcBufferLoop[IsMirrorLeftRight ? DestRectWidth - x - 1 : x];
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
						WORD PixelColor = SrcBufferLoop[IsMirrorLeftRight ? DestRectWidth - x - 1 : x];
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
						LONG w = IsMirrorLeftRight ? DestRectWidth - x - 1 : x;
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
						DWORD PixelColor = SrcBufferLoop[IsMirrorLeftRight ? DestRectWidth - x - 1 : x];
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
	if (SUCCEEDED(hr) && Config.DdrawRemoveScanlines && IsPrimaryOrBackBuffer())
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
	if (SUCCEEDED(hr))
	{
		if (IsUsingEmulation())
		{
			// Copy emulated surface to real texture
			CopyFromEmulatedSurface(&DestRect);

			// Blt surface directly to GDI
			if (Config.DdrawWriteToGDI && IsPrimarySurface() && !Using3D)
			{
				CopyEmulatedSurfaceToGDI(DestRect);
			}
		}

		// Preset surface to window
		if (surface.IsUsingWindowedMode && IsPrimarySurface() && !Config.DdrawWriteToGDI && !Using3D)
		{
			PresentSurfaceToWindow(DestRect);
		}
	}

	// Return
	return hr;
}

// Copy from emulated surface to real surface
HRESULT m_IDirectDrawSurfaceX::CopyFromEmulatedSurface(LPRECT lpDestRect)
{
	if (!IsUsingEmulation())
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not using emulation!");
		return DDERR_GENERIC;
	}

	// Update rect
	RECT DestRect = {};
	if (!CheckCoordinates(DestRect, lpDestRect))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << lpDestRect);
		return DDERR_INVALIDRECT;
	}

	// Get real d3d9 surface
	IDirect3DSurface9* pDestSurfaceD9 = GetD3D9Surface();
	if (!pDestSurfaceD9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not get real surface!");
		return DDERR_GENERIC;
	}

	// Use D3DXLoadSurfaceFromMemory to copy to the surface
	if (FAILED(D3DXLoadSurfaceFromMemory(pDestSurfaceD9, nullptr, &DestRect, surface.emu->pBits, (surfaceFormat == D3DFMT_P8) ? D3DFMT_L8 : surfaceFormat, surface.emu->Pitch, nullptr, &DestRect, D3DX_FILTER_NONE, 0)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not copy emulated surface: " << surfaceFormat);
		return DDERR_GENERIC;
	}

	// Update palette surface data
	if (!surface.DisplayTexture || FAILED(CopyEmulatedPaletteSurface(&DestRect)))
	{
		surface.IsPaletteDirty = IsPalette();
	}

	return DD_OK;
}

// Copy from real surface to emulated surface
HRESULT m_IDirectDrawSurfaceX::CopyToEmulatedSurface(LPRECT lpDestRect)
{
	if (!IsUsingEmulation())
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not using emulation!");
		return DDERR_GENERIC;
	}

	// Update rect
	RECT DestRect = {};
	if (!CheckCoordinates(DestRect, lpDestRect))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << lpDestRect);
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
	if (FAILED(LockD39Surface(&SrcLockRect, &DestRect, D3DLOCK_READONLY)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock destination surface " << DestRect);
		return (IsSurfaceLocked()) ? DDERR_SURFACEBUSY : DDERR_GENERIC;
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
		if (SrcLockRect.Pitch == EmulatedLockRect.Pitch && (DWORD)(DestRect.right - DestRect.left) == surfaceDesc2.dwWidth)
		{
			memcpy(EmulatedBuffer, SurfaceBuffer, SrcLockRect.Pitch * Height);
		}
		else if (surface.emu->bmi->bmiHeader.biBitCount == surfaceBitCount)
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

	// Update palette surface data
	if (!surface.DisplayTexture || FAILED(CopyEmulatedPaletteSurface(&DestRect)))
	{
		surface.IsPaletteDirty = IsPalette();
	}

	return hr;
}

inline HRESULT m_IDirectDrawSurfaceX::CopyEmulatedPaletteSurface(LPRECT lpDestRect)
{
	if (!IsPalette())
	{
		return DDERR_GENERIC;
	}

	if (!IsUsingEmulation())
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not using emulation!");
		return DDERR_GENERIC;
	}

	SetCriticalSection();

	HRESULT hr = DD_OK;

	do {
		// Set new palette data
		UpdatePaletteData();

		// Check for palette entry data
		if (!surface.PaletteEntryArray)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get palette data!");
			hr = DDERR_GENERIC;
			break;
		}

		// Create emulated texture for palettes
		if (!surface.DisplayTexture)
		{
			const D3DPOOL TexturePool = IsPrimaryOrBackBuffer() ? D3DPOOL_MANAGED :
				(surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) ? D3DPOOL_SYSTEMMEM : D3DPOOL_MANAGED;
			const DWORD Width = GetByteAlignedWidth(surfaceDesc2.dwWidth, surfaceBitCount);
			const DWORD Height = surfaceDesc2.dwHeight;
			LOG_LIMIT(3, __FUNCTION__ << " Creating palette display surface texture. Size: " << Width << "x" << Height << " dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps));
			if (FAILED(((*d3d9Device)->CreateTexture(Width, Height, 1, 0, D3DFMT_X8R8G8B8, TexturePool, &surface.DisplayTexture, nullptr))))
			{
				// Try failover format
				if (FAILED(((*d3d9Device)->CreateTexture(Width, Height, 1, 0, GetFailoverFormat(D3DFMT_X8R8G8B8), TexturePool, &surface.DisplayTexture, nullptr))))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create palette display surface texture. Size: " << Width << "x" << Height << " Format: " << D3DFMT_X8R8G8B8 << " dwCaps: " << Logging::hex(surfaceDesc2.ddsCaps.dwCaps));
					hr = DDERR_GENERIC;
					break;
				}
			}
		}

		// Update rect, if palette surface is dirty then update the whole surface
		RECT DestRect = {};
		if (!CheckCoordinates(DestRect, (surface.IsPaletteDirty ? nullptr : lpDestRect)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << lpDestRect);
			hr = DDERR_INVALIDRECT;
			break;
		}

		// Get palette display context surface
		if (!surface.DisplayContext)
		{
			if (FAILED(surface.DisplayTexture->GetSurfaceLevel(0, &surface.DisplayContext)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get palette display context surface!");
				hr = DDERR_GENERIC;
				break;
			}
		}

		// Use D3DXLoadSurfaceFromMemory to copy to the surface
		if (FAILED(D3DXLoadSurfaceFromMemory(surface.DisplayContext, nullptr, &DestRect, surface.emu->pBits, D3DFMT_P8, surface.emu->Pitch, surface.PaletteEntryArray, &DestRect, D3DX_FILTER_NONE, 0)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: could not copy palette display texture: " << surfaceFormat);
			hr = DDERR_GENERIC;
			break;
		}

		// Reset palette texture dirty flag
		surface.IsPaletteDirty = false;

	} while (false);

	ReleaseCriticalSection();

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::CopyEmulatedSurfaceFromGDI(RECT Rect)
{
	if (!IsUsingEmulation())
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not using emulation!");
		return DDERR_GENERIC;
	}

	// Check for forground window
	HWND DDraw_hWnd = ddrawParent->GetHwnd();
	HWND Forground_hWnd = Utils::GetTopLevelWindowOfCurrentProcess();
	bool UsingForgroundWindow = (DDraw_hWnd != Forground_hWnd && Utils::IsWindowRectEqualOrLarger(Forground_hWnd, DDraw_hWnd));

	// Get hWnd
	HWND hWnd = (UsingForgroundWindow) ? Forground_hWnd : DDraw_hWnd;
	if (!hWnd || !DDraw_hWnd)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Cannot get window handle!");
		return DDERR_GENERIC;
	}

	// Clip rect
	RECT ClientRect = {};
	if (GetClientRect(DDraw_hWnd, &ClientRect) && MapWindowPoints(DDraw_hWnd, HWND_DESKTOP, (LPPOINT)&ClientRect, 2))
	{
		Rect.left = max(Rect.left, ClientRect.left);
		Rect.top = max(Rect.top, ClientRect.top);
		Rect.right = min(Rect.right, ClientRect.right);
		Rect.bottom = min(Rect.bottom, ClientRect.bottom);
	}

	// Validate rect
	if (Rect.left >= Rect.right || Rect.top >= Rect.bottom)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << Rect);
		return DDERR_GENERIC;
	}

	// Get rect size
	RECT MapRect = Rect;
	MapWindowPoints(hWnd, HWND_DESKTOP, (LPPOINT)&MapRect, 2);

	// Get hdc
	HDC hdc = (UsingForgroundWindow) ? ::GetDC(hWnd) : ddrawParent->GetDC();
	if (!hdc)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Cannot get window DC!");
		return DDERR_GENERIC;
	}

	// Set new palette data
	UpdatePaletteData();

	// Blt to GDI
	BitBlt(surface.emu->DC, MapRect.left, MapRect.top, MapRect.right - MapRect.left, MapRect.bottom - MapRect.top, hdc, Rect.left, Rect.top, SRCCOPY);

	// Release DC
	if (UsingForgroundWindow)
	{
		::ReleaseDC(hWnd, hdc);
	}

	return DD_OK;
}

HRESULT m_IDirectDrawSurfaceX::CopyEmulatedSurfaceToGDI(RECT Rect)
{
	if (!IsUsingEmulation())
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not using emulation!");
		return DDERR_GENERIC;
	}

	// Check for forground window
	HWND DDraw_hWnd = ddrawParent->GetHwnd();
	HWND Forground_hWnd = Utils::GetTopLevelWindowOfCurrentProcess();
	bool UsingForgroundWindow = (DDraw_hWnd != Forground_hWnd && Utils::IsWindowRectEqualOrLarger(Forground_hWnd, DDraw_hWnd));

	// Get hWnd
	HWND hWnd = (UsingForgroundWindow) ? Forground_hWnd : DDraw_hWnd;
	if (!hWnd || !DDraw_hWnd)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Cannot get window handle!");
		return DDERR_GENERIC;
	}

	// Clip rect
	RECT ClientRect = {};
	if (GetClientRect(DDraw_hWnd, &ClientRect) && MapWindowPoints(DDraw_hWnd, HWND_DESKTOP, (LPPOINT)&ClientRect, 2))
	{
		Rect.left = max(Rect.left, ClientRect.left);
		Rect.top = max(Rect.top, ClientRect.top);
		Rect.right = min(Rect.right, ClientRect.right);
		Rect.bottom = min(Rect.bottom, ClientRect.bottom);
	}

	// Validate rect
	if (Rect.left >= Rect.right || Rect.top >= Rect.bottom)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << Rect);
		return DDERR_GENERIC;
	}

	// Get rect size
	RECT MapRect = Rect;
	MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&MapRect, 2);

	// Get hdc
	HDC hdc = (UsingForgroundWindow) ? ::GetDC(hWnd) : ddrawParent->GetDC();
	if (!hdc)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Cannot get window DC!");
		return DDERR_GENERIC;
	}

	// Set new palette data
	UpdatePaletteData();

	// Blt to GDI
	BitBlt(hdc, MapRect.left, MapRect.top, MapRect.right - MapRect.left, MapRect.bottom - MapRect.top, surface.emu->DC, Rect.left, Rect.top, SRCCOPY);

	// Release DC
	if (UsingForgroundWindow)
	{
		::ReleaseDC(hWnd, hdc);
	}

	return DD_OK;
}

HRESULT m_IDirectDrawSurfaceX::PresentSurfaceToWindow(RECT Rect)
{
	if (!PrimaryDisplayTexture)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Primary display texture missing!");
		return DDERR_GENERIC;
	}

	// Get hWnd
	HWND hWnd = ddrawParent->GetHwnd();
	if (!hWnd)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Cannot get window handle!");
		return DDERR_GENERIC;
	}

	// Clip rect
	RECT ClientRect = {};
	if (GetClientRect(hWnd, &ClientRect) && MapWindowPoints(hWnd, HWND_DESKTOP, (LPPOINT)&ClientRect, 2))
	{
		Rect.left = max(Rect.left, ClientRect.left);
		Rect.top = max(Rect.top, ClientRect.top);
		Rect.right = min(Rect.right, ClientRect.right);
		Rect.bottom = min(Rect.bottom, ClientRect.bottom);
	}

	// Validate rect
	if (Rect.left >= Rect.right || Rect.top >= Rect.bottom)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << Rect);
		return DDERR_GENERIC;
	}

	// Get map points
	RECT MapClient = Rect;
	if (MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&MapClient, 2))
	{
		MapClient.right = min((LONG)surfaceDesc2.dwWidth, MapClient.right);
		MapClient.bottom = min((LONG)surfaceDesc2.dwHeight, MapClient.bottom);
	}

	// Get source surface
	IDirect3DSurface9* pSourceSurfaceD9 = GetD3D9Surface();
	if (!pSourceSurfaceD9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get source surface!");
		return DDERR_GENERIC;
	}

	// Get destination surface
	IDirect3DSurface9* pDestSurfaceD9 = nullptr;
	if (FAILED(PrimaryDisplayTexture->GetSurfaceLevel(0, &pDestSurfaceD9)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get destination surface!");
		return DDERR_GENERIC;
	}

	// Copy surface
	HRESULT hr = (*d3d9Device)->UpdateSurface(pSourceSurfaceD9, &Rect, pDestSurfaceD9, (LPPOINT)&MapClient);
	pDestSurfaceD9->Release();
	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to copy surface: " << Rect << " -> " << MapClient);
		return DDERR_GENERIC;
	}

	// Present to d3d9
	if (FAILED(ddrawParent->Present2DScene(this, &MapClient, &MapClient)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to present 2D scene!");
		return DDERR_GENERIC;
	}

	return DD_OK;
}

void m_IDirectDrawSurfaceX::RemoveClipper(m_IDirectDrawClipper* ClipperToRemove)
{
	if (ClipperToRemove == attachedClipper)
	{
		SetClipper(nullptr);
	}
}

void m_IDirectDrawSurfaceX::RemovePalette(m_IDirectDrawPalette* PaletteToRemove)
{
	if (PaletteToRemove == attachedPalette)
	{
		SetPalette(nullptr);
	}
}

void m_IDirectDrawSurfaceX::UpdatePaletteData()
{
	// Check surface format
	if (!IsPalette())
	{
		return;
	}

	DWORD NewPaletteUSN = 0;
	LPPALETTEENTRY NewPaletteEntry = nullptr;
	RGBQUAD* NewRGBPalette = nullptr;

	SetCriticalSection();

	// Get palette data
	if (attachedPalette)
	{
		NewPaletteUSN = attachedPalette->GetPaletteUSN();
		NewPaletteEntry = attachedPalette->GetPaletteEntries();
		NewRGBPalette = attachedPalette->GetRGBPalette();
	}
	// Get palette from primary surface if this is not primary
	else if (!IsPrimarySurface())
	{
		m_IDirectDrawSurfaceX* lpPrimarySurface = ddrawParent->GetPrimarySurface();
		if (lpPrimarySurface)
		{
			m_IDirectDrawPalette* lpPalette = lpPrimarySurface->GetAttachedPalette();
			if (lpPalette)
			{
				NewPaletteUSN = lpPalette->GetPaletteUSN();
				NewPaletteEntry = lpPalette->GetPaletteEntries();
				NewRGBPalette = lpPalette->GetRGBPalette();
			}
		}
	}

	// Add palette data to texture
	if (primary.PaletteTexture && NewPaletteEntry && primary.LastPaletteUSN != NewPaletteUSN)
	{
		// Get palette display context surface
		LPDIRECT3DSURFACE9 paletteSurface = nullptr;
		if (SUCCEEDED(primary.PaletteTexture->GetSurfaceLevel(0, &paletteSurface)))
		{
			// Use D3DXLoadSurfaceFromMemory to copy to the surface
			RECT Rect = { 0, 0, MaxPaletteSize, 1 };
			if (FAILED(D3DXLoadSurfaceFromMemory(paletteSurface, nullptr, &Rect, NewRGBPalette, D3DFMT_X8R8G8B8, MaxPaletteSize * sizeof(D3DCOLOR), nullptr, &Rect, D3DX_FILTER_NONE, 0)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: could not full palette textur!");
			}
			paletteSurface->Release();
			primary.LastPaletteUSN = NewPaletteUSN;
		}
	}

	// Set color palette for emulation device context
	if (IsUsingEmulation() && NewRGBPalette && surface.emu->LastPaletteUSN != NewPaletteUSN)
	{
		SetDIBColorTable(surface.emu->DC, 0, MaxPaletteSize, NewRGBPalette);
		SetDIBColorTable(surface.emu->GameDC, 0, MaxPaletteSize, NewRGBPalette);
		surface.emu->LastPaletteUSN = NewPaletteUSN;
	}

	// Set new palette data
	if (NewPaletteEntry && surface.LastPaletteUSN != NewPaletteUSN)
	{
		surface.IsPaletteDirty = true;
		surface.LastPaletteUSN = NewPaletteUSN;
		surface.PaletteEntryArray = NewPaletteEntry;
	}

	ReleaseCriticalSection();
}

void m_IDirectDrawSurfaceX::StartSharedEmulatedMemory()
{
	ShareEmulatedMemory = true;
}

void m_IDirectDrawSurfaceX::DeleteEmulatedMemory(EMUSURFACE **ppEmuSurface)
{
	if (!ppEmuSurface || !*ppEmuSurface)
	{
		return;
	}

	LOG_LIMIT(100, __FUNCTION__ << " Deleting emulated surface (" << *ppEmuSurface << ")");

	SetCriticalSection();

	// Release device context memory
	if ((*ppEmuSurface)->DC)
	{
		SelectObject((*ppEmuSurface)->DC, (*ppEmuSurface)->OldDCObject);
		DeleteDC((*ppEmuSurface)->DC);
	}
	if ((*ppEmuSurface)->GameDC)
	{
		if ((*ppEmuSurface)->OldGameDCObject)
		{
			SelectObject((*ppEmuSurface)->GameDC, (*ppEmuSurface)->OldGameDCObject);
		}
		DeleteDC((*ppEmuSurface)->GameDC);
	}
	if ((*ppEmuSurface)->bitmap)
	{
		DeleteObject((*ppEmuSurface)->bitmap);
		(*ppEmuSurface)->pBits = nullptr;
	}
	if ((*ppEmuSurface)->pBits)
	{
		HeapFree(GetProcessHeap(), NULL, (*ppEmuSurface)->pBits);
		(*ppEmuSurface)->pBits = nullptr;
	}
	delete (*ppEmuSurface);
	*ppEmuSurface = nullptr;

	ReleaseCriticalSection();
}

void m_IDirectDrawSurfaceX::CleanupSharedEmulatedMemory()
{
	// Disable shared memory
	ShareEmulatedMemory = false;
	
	SetCriticalSection();

	LOG_LIMIT(100, __FUNCTION__ << " Deleting " << memorySurfaces.size() << " emulated surface" << ((memorySurfaces.size() != 1) ? "s" : "") << "!");

	// Clean up unused emulated surfaces
	for (EMUSURFACE* pEmuSurface: memorySurfaces)
	{
		DeleteEmulatedMemory(&pEmuSurface);
	}
	memorySurfaces.clear();

	ReleaseCriticalSection();
}
