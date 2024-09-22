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

// Used for dummy mipmaps
std::vector<BYTE> dummySurface;

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
	if (riid == IID_GetMipMapLevel)
	{
		*ppvObj = 0;
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

		if (attached3DTexture)
		{
			InterfaceX = attached3DTexture;
			InterfaceX->AddRef(DxVersion);
		}
		else
		{
			InterfaceX = new m_IDirect3DTextureX(ddrawParent->GetCurrentD3DDevice(), DxVersion, this);
			attached3DTexture = InterfaceX;
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
				ddrawParent->SetD3DDevice(lpD3DDeviceX);
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
		if (!WrapperInterface)
		{
			WrapperInterface = new m_IDirectDrawSurface((LPDIRECTDRAWSURFACE)ProxyInterface, this);
		}
		return WrapperInterface;
	case 2:
		if (!WrapperInterface2)
		{
			WrapperInterface2 = new m_IDirectDrawSurface2((LPDIRECTDRAWSURFACE2)ProxyInterface, this);
		}
		return WrapperInterface2;
	case 3:
		if (!WrapperInterface3)
		{
			WrapperInterface3 = new m_IDirectDrawSurface3((LPDIRECTDRAWSURFACE3)ProxyInterface, this);
		}
		return WrapperInterface3;
	case 4:
		if (!WrapperInterface4)
		{
			WrapperInterface4 = new m_IDirectDrawSurface4((LPDIRECTDRAWSURFACE4)ProxyInterface, this);
		}
		return WrapperInterface4;
	case 7:
		if (!WrapperInterface7)
		{
			WrapperInterface7 = new m_IDirectDrawSurface7((LPDIRECTDRAWSURFACE7)ProxyInterface, this);
		}
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

		m_IDirectDrawSurfaceX *lpAttachedSurfaceX = nullptr;

		lpDDSurface->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpAttachedSurfaceX);

		if (lpAttachedSurfaceX == this)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: cannot attach self");
			return DDERR_CANNOTATTACHSURFACE;
		}

		if (!ddrawParent->DoesSurfaceExist(lpAttachedSurfaceX))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid surface!");
			return DDERR_INVALIDPARAMS;
		}

		if (DoesAttachedSurfaceExist(lpAttachedSurfaceX))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: attached surface already exists");
			return DDERR_SURFACEALREADYATTACHED;
		}

		if (lpAttachedSurfaceX->IsDepthStencil() && GetAttachedDepthStencil())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: zbuffer surface already exists");
			return DDERR_CANNOTATTACHSURFACE;
		}

		DWORD AttachedSurfaceCaps = lpAttachedSurfaceX->GetSurfaceCaps().dwCaps;
		if (!(((AttachedSurfaceCaps & DDSCAPS_BACKBUFFER) && (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER)) ||
			((AttachedSurfaceCaps & DDSCAPS_FRONTBUFFER) && (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_BACKBUFFER)) ||
			((AttachedSurfaceCaps & DDSCAPS_MIPMAP) && (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_MIPMAP)) ||
			(AttachedSurfaceCaps & DDSCAPS_ZBUFFER)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: cannot attach surface with this method. dwCaps: " << lpAttachedSurfaceX->GetSurfaceCaps());
			return DDERR_CANNOTATTACHSURFACE;
		}

		// Check for MipMaps
		if (AttachedSurfaceCaps & DDSCAPS_MIPMAP)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Attaching a MipMap not Implemented.");
			return DDERR_CANNOTATTACHSURFACE;
		}

		// Update attached stencil surface
		if (lpAttachedSurfaceX->IsDepthStencil())
		{
			UpdateAttachedDepthStencil(lpAttachedSurfaceX);
		}

		AddAttachedSurfaceToMap(lpAttachedSurfaceX, true);

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

HRESULT m_IDirectDrawSurfaceX::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx, DWORD MipMapLevel, bool PresentBlt)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")" <<
		" DestRect = " << lpDestRect <<
		" SrcSurface = " << lpDDSrcSurface <<
		" SrcRect = " << lpSrcRect <<
		" Flags = " << Logging::hex(dwFlags) <<
		" BltFX = " << lpDDBltFx <<
		" MipMapLevel = " << MipMapLevel <<
		" PresentBlt = " << PresentBlt;

	// Check if source Surface exists
	if (lpDDSrcSurface && !ProxyAddressLookupTable.CheckSurfaceExists(lpDDSrcSurface))
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
			LOG_LIMIT(100, __FUNCTION__ << " Error: alpha values not implemented!");
			return DDERR_NOALPHAHW;
		}

		// All DDBLT_ZBUFFER flag values: This method does not currently support z-aware bitblt operations. None of the flags beginning with "DDBLT_ZBUFFER" are supported in DirectDraw.
		if (dwFlags & (DDBLT_ZBUFFER | DDBLT_ZBUFFERDESTCONSTOVERRIDE | DDBLT_ZBUFFERDESTOVERRIDE | DDBLT_ZBUFFERSRCCONSTOVERRIDE | DDBLT_ZBUFFERSRCOVERRIDE))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: zbuffer values not implemented!");
			return DDERR_NOZBUFFERHW;
		}

		// DDBLT_DDROPS - dwDDROP is ignored as "no such ROPs are currently defined" in DirectDraw
		if (dwFlags & DDBLT_DDROPS)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: DDROP values not implemented!");
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

		// Check for rotation flags
		// ToDo: add support for other rotation flags (90,180, 270).  Not sure if any game uses these other flags.
		if ((dwFlags & DDBLT_ROTATIONANGLE) || ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & (DDBLTFX_ROTATE90 | DDBLTFX_ROTATE180 | DDBLTFX_ROTATE270))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Rotation operations Not Implemented: " << Logging::hex(lpDDBltFx->dwDDFX & (DDBLTFX_ROTATE90 | DDBLTFX_ROTATE180 | DDBLTFX_ROTATE270)));
			return DDERR_NOROTATIONHW;
		}

		// Do supported raster operations
		if ((dwFlags & DDBLT_ROP) && (lpDDBltFx->dwROP != SRCCOPY && lpDDBltFx->dwROP != BLACKNESS && lpDDBltFx->dwROP != WHITENESS))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Raster operation Not Implemented " << Logging::hex(lpDDBltFx->dwROP));
			return DDERR_NORASTEROPHW;
		}

		// Get source mipmap level
		DWORD SrcMipMapLevel = 0;
		if (lpDDSrcSurface)
		{
			lpDDSrcSurface->QueryInterface(IID_GetMipMapLevel, (LPVOID*)&SrcMipMapLevel);
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

		// Clear the depth stencil surface
		if (dwFlags & DDBLT_DEPTHFILL)
		{
			// Check if the surface or the attached surface is the current depth stencil
			if (ddrawParent->GetDepthStencilSurface() != this && ddrawParent->GetDepthStencilSurface() != GetAttachedDepthStencil())
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: not current Depth Stencil format: " << surfaceDesc2);
				return DDERR_INVALIDPARAMS;
			}
			return (*d3d9Device)->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, ConvertDepthValue(lpDDBltFx->dwFillDepth, surface.Format), 0);
		}

		// Set critical section
		SetLockCriticalSection();
		lpDDSrcSurfaceX->SetLockCriticalSection();

		// Present before write if needed
		if (PresentBlt)
		{
			BeginWritePresent(IsSkipScene);
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
		bool CopySurfaceFlag = false;
#endif

		HRESULT hr = DD_OK;

		do {
			// Check if locked from other thread
			if (BltWait && (IsLockedFromOtherThread() || lpDDSrcSurfaceX->IsLockedFromOtherThread()))
			{
				// Wait for lock from other thread
				while (IsLockedFromOtherThread() || lpDDSrcSurfaceX->IsLockedFromOtherThread())
				{
					Utils::BusyWaitYield();
					if (!surface.Surface && !surface.Texture)
					{
						break;
					}
				}
				if (!surface.Surface && !surface.Texture)
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
			if ((LockedWithID && LockedWithID != GetCurrentThreadId()) || (lpDDSrcSurfaceX->LockedWithID && lpDDSrcSurfaceX->LockedWithID != GetCurrentThreadId()))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: surface locked thread ID set! " << LockedWithID << " " << lpDDSrcSurfaceX->LockedWithID);
			}
			LockedWithID = GetCurrentThreadId();
			lpDDSrcSurfaceX->LockedWithID = GetCurrentThreadId();

			do {
				// Do color fill
				if (dwFlags & DDBLT_COLORFILL)
				{
					hr = ColorFill(lpDestRect, lpDDBltFx->dwFillColor, MipMapLevel);
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
						hr = ColorFill(lpDestRect, 0x00000000, MipMapLevel);
						break;
					}
					else if (lpDDBltFx->dwROP == WHITENESS)
					{
						hr = ColorFill(lpDestRect, 0xFFFFFFFF, MipMapLevel);
						break;
					}
					else
					{
						LOG_LIMIT(100, __FUNCTION__ << " Warning: Unknown ROP: " << Logging::hex(lpDDBltFx->dwROP));
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
				else if ((dwFlags & DDBLT_KEYDEST) && (surfaceDesc2.dwFlags & DDSD_CKDESTBLT))
				{
					ColorKey = surfaceDesc2.ddckCKDestBlt;
				}
				else if ((dwFlags & DDBLT_KEYSRC) && (lpDDSrcSurfaceX->surfaceDesc2.dwFlags & DDSD_CKSRCBLT))
				{
					ColorKey = lpDDSrcSurfaceX->surfaceDesc2.ddckCKSrcBlt;
				}
				else if (dwFlags & (DDBLT_KEYDEST | DDBLT_KEYSRC))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: color key not found!");
					hr = DDERR_INVALIDPARAMS;
					break;
				}

				D3DTEXTUREFILTERTYPE Filter = ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & DDBLTFX_ARITHSTRETCHY)) ? D3DTEXF_LINEAR : D3DTEXF_NONE;

				hr = CopySurface(lpDDSrcSurfaceX, lpSrcRect, lpDestRect, Filter, ColorKey.dwColorSpaceLowValue, Flags, SrcMipMapLevel, MipMapLevel);
#ifdef ENABLE_PROFILING
				CopySurfaceFlag = true;
#endif

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

#ifdef ENABLE_PROFILING
			Logging::Log() << __FUNCTION__ << " (" << lpDDSrcSurfaceX << ") -> (" << this << ")" <<
				(CopySurfaceFlag ? " CopySurface()" : " ColorFill()") <<
				" Type = " << lpDDSrcSurfaceX->surface.Type << " " << lpDDSrcSurfaceX->surface.Pool << " -> " << surface.Type << " " << surface.Pool <<
				" hr = " << (D3DERR)hr <<
				" Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

			// If successful
			if (SUCCEEDED(hr))
			{
				// Set dirty flag
				if (PresentBlt && MipMapLevel == 0)
				{
					SetDirtyFlag();
				}

				// Mark mipmap data flag
				if (MipMapLevel && MipMaps.size())
				{
					DWORD Level = min(MipMaps.size(), MipMapLevel) - 1;
					MipMaps[Level].UniquenessValue = UniquenessValue;
					CheckMipMapLevelGen();
				}

				// Set vertical sync wait timer
				if (SUCCEEDED(c_hr) && (dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & DDBLTFX_NOTEARING))
				{
					ddrawParent->SetVsync();
				}

				// Present surface
				EndWritePresent(lpDestRect, true, PresentBlt, IsSkipScene);
			}

		} while (false);

		// Check if surface was busy
		if (!BltWait && (hr == DDERR_SURFACEBUSY || IsLockedFromOtherThread() || lpDDSrcSurfaceX->IsLockedFromOtherThread()))
		{
			hr = DDERR_WASSTILLDRAWING;
		}
		else if (FAILED(hr) && (IsLost() == DDERR_SURFACELOST || lpDDSrcSurfaceX->IsLost() == DDERR_SURFACELOST))
		{
			hr = DDERR_SURFACELOST;
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
		GetSurfaceDesc(&Desc, 0, 0);

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

HRESULT m_IDirectDrawSurfaceX::BltBatch(LPDDBLTBATCH lpDDBltBatch, DWORD dwCount, DWORD dwFlags, DWORD MipMapLevel)
{
	UNREFERENCED_PARAMETER(dwFlags);

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lpDDBltBatch)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Check for device interface before doing batch
	HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
	if (FAILED(c_hr))
	{
		return c_hr;
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

		hr = Blt(lpDDBltBatch[x].lprDest, (LPDIRECTDRAWSURFACE7)lpDDBltBatch[x].lpDDSSrc, lpDDBltBatch[x].lprSrc, lpDDBltBatch[x].dwFlags, lpDDBltBatch[x].lpDDBltFx, MipMapLevel, false);
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
		// Set dirty flag
		if (MipMapLevel == 0)
		{
			SetDirtyFlag();
		}

		// Mark mipmap data flag
		if (MipMapLevel && MipMaps.size())
		{
			DWORD Level = min(MipMaps.size(), MipMapLevel) - 1;
			MipMaps[Level].UniquenessValue = UniquenessValue;
			CheckMipMapLevelGen();
		}

		// Present surface
		EndWritePresent(nullptr, false, true, IsSkipScene);
	}

	ReleaseLockCriticalSection();

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::BltFast(DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, DWORD MipMapLevel)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Check if source Surface exists
	if (lpDDSrcSurface && !ProxyAddressLookupTable.CheckSurfaceExists(lpDDSrcSurface))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not find source surface! " << lpDDSrcSurface);
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
		if (!lpDDSrcSurfaceX->CheckCoordinates(SrcRect, lpSrcRect, nullptr))
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
		return Blt(pDestRect, lpDDSrcSurface, lpSrcRect, Flags, nullptr, MipMapLevel);
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
		GetSurfaceDesc(&Desc, 0, 0);

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

		m_IDirectDrawSurfaceX *lpAttachedSurfaceX = nullptr;

		lpDDSAttachedSurface->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpAttachedSurfaceX);

		if (!DoesAttachedSurfaceExist(lpAttachedSurfaceX))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find attached surface");
			return DDERR_SURFACENOTATTACHED;
		}

		if (!WasAttachedSurfaceAdded(lpAttachedSurfaceX))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: only surfaces added with AddAttachedSurface can be deleted with this method");
			return DDERR_CANNOTDETACHSURFACE;
		}

		// clear zbuffer
		if (lpAttachedSurfaceX->IsDepthStencil() &&
			(ddrawParent->GetDepthStencilSurface() == lpAttachedSurfaceX || ddrawParent->GetRenderTargetSurface() == this))
		{
			ddrawParent->SetDepthStencilSurface(nullptr);
		}

		RemoveAttachedSurfaceFromMap(lpAttachedSurfaceX);

		lpDDSAttachedSurface->Release();

		return DD_OK;
	}

	if (lpDDSAttachedSurface)
	{
		lpDDSAttachedSurface->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDSAttachedSurface);
	}

	return ProxyInterface->DeleteAttachedSurface(dwFlags, lpDDSAttachedSurface);
}

m_IDirectDrawSurfaceX* m_IDirectDrawSurfaceX::GetAttachedDepthStencil()
{
	for (auto& it : AttachedSurfaceMap)
	{
		if (it.second.pSurface->IsDepthStencil())
		{
			return it.second.pSurface;
		}
	}
	return nullptr;
}

HRESULT m_IDirectDrawSurfaceX::GetMipMapLevelAddr(LPDIRECTDRAWSURFACE7 FAR* lplpDDAttachedSurface, MIPMAP& MipMapSurface, DWORD MipMapLevel, DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 1:
		if (!MipMapSurface.Addr)
		{
			MipMapSurface.Addr = new m_IDirectDrawSurface(this, MipMapLevel);
		}
		*lplpDDAttachedSurface = (LPDIRECTDRAWSURFACE7)MipMapSurface.Addr;
		break;
	case 2:
		if (!MipMapSurface.Addr2)
		{
			MipMapSurface.Addr2 = new m_IDirectDrawSurface2(this, MipMapLevel);
		}
		*lplpDDAttachedSurface = (LPDIRECTDRAWSURFACE7)MipMapSurface.Addr2;
		break;
	case 3:
		if (!MipMapSurface.Addr3)
		{
			MipMapSurface.Addr3 = new m_IDirectDrawSurface3(this, MipMapLevel);
		}
		*lplpDDAttachedSurface = (LPDIRECTDRAWSURFACE7)MipMapSurface.Addr3;
		break;
	case 4:
		if (!MipMapSurface.Addr4)
		{
			MipMapSurface.Addr4 = new m_IDirectDrawSurface4(this, MipMapLevel);
		}
		*lplpDDAttachedSurface = (LPDIRECTDRAWSURFACE7)MipMapSurface.Addr4;
		break;
	case 7:
		if (!MipMapSurface.Addr7)
		{
			MipMapSurface.Addr7 = new m_IDirectDrawSurface7(this, MipMapLevel);
		}
		*lplpDDAttachedSurface = (LPDIRECTDRAWSURFACE7)MipMapSurface.Addr7;
		break;
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: incorrect DirectX version: " << DirectXVersion);
		return DDERR_NOTFOUND;
	}

	return DD_OK;
}

HRESULT m_IDirectDrawSurfaceX::GetMipMapSubLevel(LPDIRECTDRAWSURFACE7 FAR* lplpDDAttachedSurface, DWORD MipMapLevel, DWORD DirectXVersion)
{
	// Check for device interface to ensure correct max MipMap level
	CheckInterface(__FUNCTION__, true, true, false);

	if (MaxMipMapLevel > MipMapLevel)
	{
		while (MipMaps.size() < MipMapLevel + 1)
		{
			MIPMAP MipMap;
			MipMaps.push_back(MipMap);
		}

		return GetMipMapLevelAddr(lplpDDAttachedSurface, MipMaps[MipMapLevel], MipMapLevel + 1, DirectXVersion);
	}
	return DDERR_NOTFOUND;
}

HRESULT m_IDirectDrawSurfaceX::EnumAttachedSurfaces(LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback, DWORD MipMapLevel, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lpEnumSurfacesCallback)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (ProxyDirectXVersion > 3)
	{
		return EnumAttachedSurfaces2(lpContext, (LPDDENUMSURFACESCALLBACK7)lpEnumSurfacesCallback, MipMapLevel, DirectXVersion);
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

HRESULT m_IDirectDrawSurfaceX::EnumAttachedSurfaces2(LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback7, DWORD MipMapLevel, DWORD DirectXVersion)
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
		// Handle mipmaps
		if (!MipMaps.empty())
		{
			LPDIRECTDRAWSURFACE7 lpDDAttachedSurface = nullptr;
			if (SUCCEEDED(GetMipMapSubLevel(&lpDDAttachedSurface, MipMapLevel, DirectXVersion)))
			{
				DDSURFACEDESC2 Desc2 = {};
				Desc2.dwSize = sizeof(DDSURFACEDESC2);
				GetSurfaceDesc2(&Desc2, MipMapLevel + 1, DirectXVersion);
				if (EnumSurface::ConvertCallback(lpDDAttachedSurface, &Desc2, &CallbackContext) == DDENUMRET_CANCEL)
				{
					return DD_OK;
				}
			}
		}
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
				it.second.pSurface->GetSurfaceDesc2(&Desc2, 0, DirectXVersion);
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
	if (surface.Format != lpTargetSurface->surface.Format ||
		surfaceDesc2.dwWidth != lpTargetSurface->surfaceDesc2.dwWidth ||
		surfaceDesc2.dwHeight != lpTargetSurface->surfaceDesc2.dwHeight)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: backbuffer surface does not match: " <<
			surface.Format << " -> " << lpTargetSurface->surface.Format << " " <<
			surfaceDesc2.dwWidth << "x" << surfaceDesc2.dwHeight << " -> " <<
			lpTargetSurface->surfaceDesc2.dwWidth << "x" << lpTargetSurface->surfaceDesc2.dwHeight);
		return DDERR_INVALIDPARAMS;
	}

	return DD_OK;
}

HRESULT m_IDirectDrawSurfaceX::Flip(LPDIRECTDRAWSURFACE7 lpDDSurfaceTargetOverride, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")" <<
		" SrcSurface = " << lpDDSurfaceTargetOverride <<
		" Flags = " << Logging::hex(dwFlags) <<
		" Version = " << DirectXVersion;

	if (Config.Dd7to9)
	{
		if ((dwFlags & (DDFLIP_EVEN | DDFLIP_ODD)) == (DDFLIP_EVEN | DDFLIP_ODD))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid flags!");
			return DDERR_INVALIDPARAMS;
		}

		// Flip can be called only for a surface that has the DDSCAPS_FLIP and DDSCAPS_FRONTBUFFER capabilities
		if (!IsFlipSurface())
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

		// Check if is in scene
		if (Using3D)
		{
			m_IDirect3DDeviceX** pD3DDeviceInterface = ddrawParent->GetCurrentD3DDevice();
			if (pD3DDeviceInterface && *pD3DDeviceInterface && (*pD3DDeviceInterface)->IsDeviceInScene())
			{
				return DDERR_GENERIC;
			}
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
					hr = IsLost() == DDERR_SURFACELOST ? DDERR_SURFACELOST : DDERR_GENERIC;
					break;
				}
			}

			// Set flip flag
			IsInFlip = true;

			// Clear surface before flip if system memory
			if (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
			{
				if (FAILED(ColorFill(nullptr, Config.DdrawFlipFillColor, 0)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not color fill surface.");
				}
				ClearDirtyFlags();
			}

			// Execute flip
			for (size_t x = 0; x < FlipList.size() - 1; x++)
			{
				SwapAddresses(&FlipList[x]->surface, &FlipList[x + 1]->surface);
			}

			// Reset flip flag
			IsInFlip = false;

#ifdef ENABLE_PROFILING
			Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr;
#endif

			// If texture is not dirty then mark it as dirty in case the game wrote to the memory directly (Nox does this)
			if (!surface.IsDirtyFlag)
			{
				// Set dirty flag
				SetDirtyFlag();

				// Keep surface insync
				EndWriteSyncSurfaces(nullptr);

				// Add dirty rect
				LPDIRECT3DTEXTURE9 displayTexture = Get3DTexture();
				if (displayTexture)
				{
					displayTexture->AddDirtyRect(nullptr);
				}
			}

			// Set vertical sync wait timer
			if ((dwFlags & DDFLIP_NOVSYNC) == 0)
			{
				ddrawParent->SetVsync();
			}

			// Present surface
			EndWritePresent(nullptr, true, true, false);

			if (IsRenderTarget())
			{
				ddrawParent->ReSetRenderTarget();
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

HRESULT m_IDirectDrawSurfaceX::GetAttachedSurface(LPDDSCAPS lpDDSCaps, LPDIRECTDRAWSURFACE7 FAR * lplpDDAttachedSurface, DWORD MipMapLevel, DWORD DirectXVersion)
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

		return GetAttachedSurface2((lpDDSCaps) ? &Caps2 : nullptr, lplpDDAttachedSurface, MipMapLevel, DirectXVersion);
	}

	HRESULT hr = GetProxyInterfaceV3()->GetAttachedSurface(lpDDSCaps, (LPDIRECTDRAWSURFACE3*)lplpDDAttachedSurface);

	if (SUCCEEDED(hr) && lplpDDAttachedSurface)
	{
		*lplpDDAttachedSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpDDAttachedSurface, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetAttachedSurface2(LPDDSCAPS2 lpDDSCaps2, LPDIRECTDRAWSURFACE7 FAR * lplpDDAttachedSurface, DWORD MipMapLevel, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpDDAttachedSurface || !lpDDSCaps2)
		{
			return DDERR_INVALIDPARAMS;
		}
		*lplpDDAttachedSurface = nullptr;

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
			// Handle mipmaps
			if ((lpDDSCaps2->dwCaps & DDSCAPS_MIPMAP) && (GetSurfaceCaps().dwCaps & lpDDSCaps2->dwCaps) == lpDDSCaps2->dwCaps)
			{
				// Normal MipMaps
				if (SUCCEEDED(GetMipMapSubLevel(lplpDDAttachedSurface, MipMapLevel, DirectXVersion)))
				{
					(*lplpDDAttachedSurface)->AddRef();

					return DD_OK;
				}
				// Use dummy mipmap surface to prevent some games from crashing
				DWORD Level = (MipMapLevel & ~DXW_IS_MIPMAP_DUMMY);
				if (Level < GetMaxMipMapLevel(surfaceDesc2.dwWidth, surfaceDesc2.dwHeight) + 1)
				{
					while (MipMaps.size() < Level + 1)
					{
						MIPMAP MipMap;
						MipMaps.push_back(MipMap);
					}

					if (SUCCEEDED(GetMipMapLevelAddr(lplpDDAttachedSurface, MipMaps[Level], DXW_IS_MIPMAP_DUMMY + Level + 1, DirectXVersion)))
					{
						MipMaps[Level].IsDummy = true;

						(*lplpDDAttachedSurface)->AddRef();

						return DD_OK;
					}
				}
				return DDERR_NOTFOUND;
			}

			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to find attached surface that matches the capabilities requested: " << *lpDDSCaps2 <<
				" Attached number of surfaces: " << AttachedSurfaceMap.size() << " MaxMipMapLevel: " << MaxMipMapLevel << " Caps: " << surfaceDesc2.ddsCaps);
			return DDERR_NOTFOUND;
		}

		*lplpDDAttachedSurface = (LPDIRECTDRAWSURFACE7)lpFoundSurface->GetWrapperInterfaceX(DirectXVersion);

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
		if (!(surfaceDesc2.dwFlags & dds))
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

bool m_IDirectDrawSurfaceX::GetColorKeyForPrimaryShader(float(&lowColorKey)[4], float(&highColorKey)[4])
{
	// Primary 2D surface background color
	if (IsPrimarySurface())
	{
		if (!primary.ShaderColorKey.IsSet)
		{
			GetColorKeyArray(primary.ShaderColorKey.lowColorKey, primary.ShaderColorKey.highColorKey, Config.DdrawFlipFillColor, Config.DdrawFlipFillColor, surfaceDesc2.ddpfPixelFormat);
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
	return false;
}

bool m_IDirectDrawSurfaceX::GetColorKeyForShader(float(&lowColorKey)[4], float(&highColorKey)[4])
{
	// Surface low and high color space
	if (!ShaderColorKey.IsSet)
	{
		if (surfaceDesc2.dwFlags & DDSD_CKSRCBLT)
		{
			GetColorKeyArray(ShaderColorKey.lowColorKey, ShaderColorKey.highColorKey,
				surfaceDesc2.ddckCKSrcBlt.dwColorSpaceLowValue, surfaceDesc2.ddckCKSrcBlt.dwColorSpaceHighValue, surfaceDesc2.ddpfPixelFormat);
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

HRESULT m_IDirectDrawSurfaceX::GetDC(HDC FAR* lphDC, DWORD MipMapLevel)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")" <<
		" lpDC = " << (void*)lphDC <<
		" MipMapLevel = " << MipMapLevel;

	if (Config.Dd7to9)
	{
		if (!lphDC)
		{
			return DDERR_INVALIDPARAMS;
		}
		*lphDC = nullptr;

		// MipMap level support
		if (MipMapLevel)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: GetDC from MipMap level Not Implemented: " << MipMapLevel);
			return DDERR_UNSUPPORTED;
		}

		if (LastDC && IsSurfaceInDC())
		{
			*lphDC = LastDC;
		}
		else if (IsUsingEmulation())
		{
			// Prepare GameDC
			SetEmulationGameDC();

			*lphDC = surface.emu->GameDC;
		}

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

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		// Check if render target should use shadow
		if (surface.Type == D3DTYPE_RENDERTARGET && !IsUsingShadowSurface())
		{
			SetRenderTargetShadow();
		}

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
				if (ShouldReadFromGDI())
				{
					CopyEmulatedSurfaceFromGDI(nullptr);
				}

				// Prepare GameDC
				SetEmulationGameDC();

				*lphDC = surface.emu->GameDC;
			}
			else
			{
				// Get surface
				IDirect3DSurface9* pSurfaceD9 = Get3DMipMapSurface(0);
				if (!pSurfaceD9)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not find surface!");
					hr = DDERR_GENERIC;
					break;
				}

				// Get device context
				hr = pSurfaceD9->GetDC(lphDC);
				if (FAILED(hr))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not get device context!");
					hr = (hr == D3DERR_DEVICELOST || IsLost() == DDERR_SURFACELOST) ? DDERR_SURFACELOST :
						(hr == DDERR_WASSTILLDRAWING || IsSurfaceBusy()) ? DDERR_SURFACEBUSY : DDERR_GENERIC;
					break;
				}
			}

			// Set DC flag
			IsInDC = true;

			// Set LastDC
			LastDC = *lphDC;

		} while (false);

		ReleaseLockCriticalSection();

		if (FAILED(hr))
		{
			hr = IsSurfaceBusy() ? DDERR_SURFACEBUSY : IsLost() == DDERR_SURFACELOST ? DDERR_SURFACELOST : DDERR_GENERIC;
		}

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ")" <<
			" Type = " << surface.Type << " " << surface.Pool <<
			" hr = " << (D3DERR)hr <<
			" Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

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
		if (!IsFlipSurface())
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
		*lplpDDPalette = nullptr;

		// No palette attached
		if (!attachedPalette)
		{
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

HRESULT m_IDirectDrawSurfaceX::GetSurfaceDesc(LPDDSURFACEDESC lpDDSurfaceDesc, DWORD MipMapLevel, DWORD DirectXVersion)
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

		HRESULT hr = hr = GetSurfaceDesc2(&Desc2, MipMapLevel, DirectXVersion);

		// Convert back to LPDDSURFACEDESC
		if (SUCCEEDED(hr))
		{
			ConvertSurfaceDesc(*lpDDSurfaceDesc, Desc2);
		}

		return hr;
	}

	return GetProxyInterfaceV3()->GetSurfaceDesc(lpDDSurfaceDesc);
}

HRESULT m_IDirectDrawSurfaceX::GetSurfaceDesc2(LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD MipMapLevel, DWORD DirectXVersion)
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
		if (!surface.UsingSurfaceMemory)
		{
			lpDDSurfaceDesc2->dwFlags &= ~DDSD_LPSURFACE;
			lpDDSurfaceDesc2->lpSurface = nullptr;
		}

		// Handle dummy mipmaps
		if (IsDummyMipMap(MipMapLevel))
		{
			DWORD Level = (MipMapLevel & ~DXW_IS_MIPMAP_DUMMY);
			lpDDSurfaceDesc2->dwFlags &= ~DDSD_LPSURFACE;
			lpDDSurfaceDesc2->dwFlags |= DDSD_PITCH;
			DWORD BitCount = surface.BitCount ? surface.BitCount : GetBitCount(lpDDSurfaceDesc2->ddpfPixelFormat);
			DWORD Width = surface.Width ? surface.Width : GetByteAlignedWidth(lpDDSurfaceDesc2->dwWidth, BitCount);
			DWORD Height = surface.Height ? surface.Height : GetByteAlignedWidth(lpDDSurfaceDesc2->dwHeight, BitCount);
			lpDDSurfaceDesc2->dwWidth = max(1, Width >> Level);
			lpDDSurfaceDesc2->dwHeight = max(1, Height >> Level);
			lpDDSurfaceDesc2->lpSurface = nullptr;
			lpDDSurfaceDesc2->lPitch = ComputePitch(lpDDSurfaceDesc2->dwWidth, BitCount);
			if (DirectXVersion == 7)
			{
				lpDDSurfaceDesc2->ddsCaps.dwCaps2 |= DDSCAPS2_MIPMAPSUBLEVEL;
			}
			if (lpDDSurfaceDesc2->dwFlags & DDSD_MIPMAPCOUNT)
			{
				lpDDSurfaceDesc2->dwMipMapCount = surfaceDesc2.dwMipMapCount > Level ? surfaceDesc2.dwMipMapCount - Level : 1;
			}
		}
		// Handle MipMap sub-level
		else if (MipMapLevel && MipMaps.size())
		{
			// Check for device interface to ensure correct max MipMap level
			CheckInterface(__FUNCTION__, true, true, false);

			// Remove a couple of flags
			lpDDSurfaceDesc2->dwFlags &= ~(DDSD_LPSURFACE | DDSD_PITCH);
			lpDDSurfaceDesc2->lpSurface = nullptr;

			// Get surface level desc
			DWORD Level = min(MipMaps.size(), MipMapLevel) - 1;
			if ((!MipMaps[Level].dwWidth || !MipMaps[Level].dwHeight) && surface.Texture)
			{
				D3DSURFACE_DESC Desc = {};
				surface.Texture->GetLevelDesc(GetD3d9MipMapLevel(MipMapLevel), &Desc);
				MipMaps[Level].dwWidth = Desc.Width;
				MipMaps[Level].dwHeight = Desc.Height;
			}
			lpDDSurfaceDesc2->dwWidth = MipMaps[Level].dwWidth;
			lpDDSurfaceDesc2->dwHeight = MipMaps[Level].dwHeight;
			if (MipMaps[Level].lPitch)
			{
				lpDDSurfaceDesc2->lPitch = MipMaps[Level].lPitch;
				lpDDSurfaceDesc2->dwFlags |= DDSD_PITCH;
			}
			else
			{
				DWORD BitCount = surface.BitCount ? surface.BitCount : GetBitCount(lpDDSurfaceDesc2->ddpfPixelFormat);
				lpDDSurfaceDesc2->lPitch = ComputePitch(lpDDSurfaceDesc2->dwWidth, BitCount);
				lpDDSurfaceDesc2->dwFlags |= DDSD_PITCH;
			}
			if (MipMapLevel != 0 && DirectXVersion == 7)
			{
				lpDDSurfaceDesc2->ddsCaps.dwCaps2 |= DDSCAPS2_MIPMAPSUBLEVEL;
			}
			lpDDSurfaceDesc2->dwMipMapCount = MaxMipMapLevel + 1 > MipMapLevel ? MaxMipMapLevel + 1 - MipMapLevel : 1;
		}
		// Root mipmap or no mipmaps
		else
		{
			// Get lPitch if not set
			if ((lpDDSurfaceDesc2->dwFlags & (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT)) == (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT) &&
				(lpDDSurfaceDesc2->ddpfPixelFormat.dwFlags & DDPF_RGB) && !(lpDDSurfaceDesc2->dwFlags & (DDSD_PITCH | DDSD_LINEARSIZE)))
			{
				DWORD BitCount = surface.BitCount ? surface.BitCount : GetBitCount(lpDDSurfaceDesc2->ddpfPixelFormat);
				DWORD Width = surface.Width ? surface.Width : GetByteAlignedWidth(lpDDSurfaceDesc2->dwWidth, BitCount);
				lpDDSurfaceDesc2->lPitch = ComputePitch(Width, BitCount);
				lpDDSurfaceDesc2->dwFlags |= DDSD_PITCH;
			}
		}

		// Handle managed texture memory type
		if ((lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_TEXTURE) && (lpDDSurfaceDesc2->ddsCaps.dwCaps2 & DDSCAPS2_TEXTUREMANAGE))
		{
			lpDDSurfaceDesc2->ddsCaps.dwCaps = (lpDDSurfaceDesc2->ddsCaps.dwCaps & ~(DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY)) | DDSCAPS_SYSTEMMEMORY;
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
		// Check device interface
		if ((surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) || IsD9UsingVideoMemory())
		{
			// Check for device interface
			HRESULT c_hr = CheckInterface(__FUNCTION__, false, false, false);
			if (FAILED(c_hr))
			{
				return c_hr;
			}

			switch (ddrawParent->TestD3D9CooperativeLevel())
			{
			case D3DERR_DEVICELOST:
				IsSurfaceLost = true;
				return DD_OK;		// Native DriectDraw returns ok here, until surface is ready to be reset
			case D3DERR_DEVICENOTRESET:
				IsSurfaceLost = true;
				return DDERR_SURFACELOST;
			case D3D_OK:
			case DDERR_NOEXCLUSIVEMODE:
				if (IsSurfaceLost && (ComplexRoot || !(surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_COMPLEX)))
				{
					return DDERR_SURFACELOST;
				}
				return DD_OK;
			default:
				return DDERR_WRONGMODE;
			}
		}

		return DD_OK;
	}

	return ProxyInterface->IsLost();
}

HRESULT m_IDirectDrawSurfaceX::Lock(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent, DWORD MipMapLevel, DWORD DirectXVersion)
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

		HRESULT hr = Lock2(lpDestRect, &Desc2, dwFlags, hEvent, MipMapLevel, DirectXVersion);

		// Convert back to LPDDSURFACEDESC
		ConvertSurfaceDesc(*lpDDSurfaceDesc, Desc2);

		return hr;
	}

	return GetProxyInterfaceV3()->Lock(lpDestRect, lpDDSurfaceDesc, dwFlags, hEvent);
}

HRESULT m_IDirectDrawSurfaceX::Lock2(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD dwFlags, HANDLE hEvent, DWORD MipMapLevel, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")" <<
		" Rect = " << lpDestRect <<
		" Desc = " << lpDDSurfaceDesc2 <<
		" Flags = " << Logging::hex(dwFlags) <<
		" Event = " << hEvent <<
		" MipMapLevel = " << MipMapLevel <<
		" Version = " << DirectXVersion;

	if (Config.Dd7to9)
	{
		// Check surfaceDesc size
		if (lpDDSurfaceDesc2 && lpDDSurfaceDesc2->dwSize == sizeof(DDSURFACEDESC))
		{
			return Lock(lpDestRect, (LPDDSURFACEDESC)lpDDSurfaceDesc2, dwFlags, hEvent, MipMapLevel, DirectXVersion);
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
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, false);

		// Prepare surfaceDesc
		GetSurfaceDesc2(lpDDSurfaceDesc2, MipMapLevel, DirectXVersion);
		if (IsUsingEmulation())
		{
			D3DLOCKED_RECT LockedRect = {};
			if (lpDestRect && SUCCEEDED(LockEmulatedSurface(&LockedRect, lpDestRect)))
			{
				lpDDSurfaceDesc2->dwFlags |= DDSD_LPSURFACE | DDSD_PITCH;
				lpDDSurfaceDesc2->lpSurface = LockedRect.pBits;
				lpDDSurfaceDesc2->lPitch = LockedRect.Pitch;
			}
			else
			{
				lpDDSurfaceDesc2->dwFlags |= DDSD_LPSURFACE | DDSD_PITCH;
				lpDDSurfaceDesc2->lpSurface = surface.emu->pBits;
				lpDDSurfaceDesc2->lPitch = surface.emu->Pitch;
			}
		}
		else if (lpDDSurfaceDesc2->dwFlags & DDSD_LINEARSIZE)
		{
			surfaceDesc2.dwFlags &= ~(DDSD_PITCH | DDSD_LINEARSIZE);
			surfaceDesc2.dwLinearSize = 0;
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
		if (!CheckCoordinates(DestRect, lpDestRect, lpDDSurfaceDesc2) || (lpDestRect && (lpDestRect->left < 0 || lpDestRect->top < 0 ||
			lpDestRect->right <= lpDestRect->left || lpDestRect->bottom <= lpDestRect->top ||
			lpDestRect->right > (LONG)lpDDSurfaceDesc2->dwWidth || lpDestRect->bottom > (LONG)lpDDSurfaceDesc2->dwHeight)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << lpDestRect);
			return DDERR_INVALIDRECT;
		}

		// Handle dummy mipmaps
		if (IsDummyMipMap(MipMapLevel))
		{
			lpDDSurfaceDesc2->dwFlags |= DDSD_LPSURFACE | DDSD_PITCH;
			lpDDSurfaceDesc2->lPitch = ComputePitch(lpDDSurfaceDesc2->dwWidth, surface.BitCount);
			// Add surface size to dummy data address to ensure that each mipmap gets a unique address
			lpDDSurfaceDesc2->lpSurface = dummySurface.data() + (lpDDSurfaceDesc2->dwWidth * lpDDSurfaceDesc2->dwHeight * surface.BitCount);
			return DD_OK;
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

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		// Don't use shadow for Lock()
		// Some games write to surface without locking so we don't want to give them a shadow surface or it could make the shadow surface out of sync
		PrepareRenderTarget();

		HRESULT hr = DD_OK;

		do {
			// Check if locked from other thread
			if (LockWait && IsLockedFromOtherThread())
			{
				// Wait for lock from other thread
				while (IsLockedFromOtherThread())
				{
					Utils::BusyWaitYield();
					if (!surface.Surface && !surface.Texture)
					{
						break;
					}
				}
				if (!surface.Surface && !surface.Texture)
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
				if (ShouldReadFromGDI())
				{
					CopyEmulatedSurfaceFromGDI(&DestRect);
				}
			}
			// Lock surface
			else if (surface.Surface || surface.Texture)
			{
				// Lock surface
				HRESULT ret = LockD3d9Surface(&LockedRect, &DestRect, Flags, MipMapLevel);
				if (FAILED(ret))
				{
					if (IsSurfaceLocked())
					{
						LOG_LIMIT(100, __FUNCTION__ << " Warning: attempting to lock surface twice!");
						UnLockD3d9Surface(MipMapLevel);
					}
					ret = LockD3d9Surface(&LockedRect, &DestRect, Flags, MipMapLevel);
				}
				if (FAILED(ret))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock surface." << (surface.Surface ? " Is Surface." : " Is Texture.") <<
						" Size: " << lpDDSurfaceDesc2->dwWidth << "x" << lpDDSurfaceDesc2->dwHeight << " Format: " << surface.Format <<
						" Flags: " << Logging::hex(Flags) << " Locked: " << IsSurfaceLocked() << " DC: " << IsSurfaceInDC() << " Blt: " << IsSurfaceBlitting() << " hr: " << (D3DERR)ret);
					hr = (ret == D3DERR_DEVICELOST || IsLost() == DDERR_SURFACELOST) ? DDERR_SURFACELOST :
						(ret == DDERR_WASSTILLDRAWING || (!LockWait && IsSurfaceBusy())) ? DDERR_WASSTILLDRAWING : DDERR_GENERIC;
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

			// Set thread ID
			if (LockedWithID && LockedWithID != GetCurrentThreadId())
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
			lpDDSurfaceDesc2->lpSurface = LockedRect.pBits;
			lpDDSurfaceDesc2->dwFlags |= DDSD_LPSURFACE;

			// Pitch for DXT surfaces in DirectDraw is the full surface byte size
			LockedRect.Pitch =
				ISDXTEX(surface.Format) ? ((surface.Width + 3) / 4) * ((lpDDSurfaceDesc2->dwHeight + 3) / 4) * (surface.Format == D3DFMT_DXT1 ? 8 : 16) :
				(surface.Format == D3DFMT_YV12 || surface.Format == D3DFMT_NV12) ? surface.Width :
				LockedRect.Pitch;
			lpDDSurfaceDesc2->lPitch = LockedRect.Pitch;
			lpDDSurfaceDesc2->dwFlags |= DDSD_PITCH;

			// Set surface pitch
			if (MipMapLevel && MipMaps.size())
			{
				DWORD Level = min(MipMaps.size(), MipMapLevel) - 1;
				if (MipMaps[Level].lPitch && MipMaps[Level].lPitch != LockedRect.Pitch)
				{
					LOG_LIMIT(100, __FUNCTION__ << " (" << this << ")" << " Warning: surface pitch does not match locked pitch! Format: " << surface.Format <<
						" Width: " << lpDDSurfaceDesc2->dwWidth << " Pitch: " << MipMaps[Level].lPitch << "->" << LockedRect.Pitch
						<< " MipMapLevel: " << MipMapLevel);
				}
				MipMaps[Level].lPitch = LockedRect.Pitch;
				MipMaps[Level].UniquenessValue = UniquenessValue;
				CheckMipMapLevelGen();
			}
			else
			{
				if ((surfaceDesc2.dwFlags & DDSD_PITCH) && surfaceDesc2.lPitch != LockedRect.Pitch)
				{
					LOG_LIMIT(100, __FUNCTION__ << " (" << this << ")" << " Warning: surface pitch does not match locked pitch! Format: " << surface.Format <<
						" Width: " << surfaceDesc2.dwWidth << " Pitch: " << surfaceDesc2.lPitch << "->" << LockedRect.Pitch <<
						" Default: " << ComputePitch(surface.Width, surface.BitCount) << " BitCount: " << surface.BitCount);
				}
				surfaceDesc2.lPitch = LockedRect.Pitch;
				surfaceDesc2.dwFlags |= DDSD_PITCH;
			}

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
			LastLock.MipMapLevel = MipMapLevel;

			// Restore scanlines before returing surface memory
			if (Config.DdrawRemoveScanlines && IsPrimaryOrBackBuffer() && !(Flags & D3DLOCK_READONLY))
			{
				RestoreScanlines(LastLock);
			}

		} while (false);

		ReleaseLockCriticalSection();

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ")" <<
			" Type = " << surface.Type << " " << surface.Pool <<
			" hr = " << (D3DERR)hr <<
			" Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

		return hr;
	}

	return ProxyInterface->Lock(lpDestRect, lpDDSurfaceDesc2, dwFlags, hEvent);
}

inline HRESULT m_IDirectDrawSurfaceX::LockD3d9Surface(D3DLOCKED_RECT* pLockedRect, RECT* pRect, DWORD Flags, DWORD MipMapLevel)
{
	if (surface.UsingSurfaceMemory)
	{
		pLockedRect->Pitch = surfaceDesc2.dwWidth * surface.BitCount / 8;
		pLockedRect->pBits = (pRect) ? (void*)((DWORD)surfaceDesc2.lpSurface + ((pRect->top * pLockedRect->Pitch) + (pRect->left * (surface.BitCount / 8)))) : surfaceDesc2.lpSurface;
		return DD_OK;
	}
	// Lock shadow surface
	else if (IsUsingShadowSurface())
	{
		return surface.Shadow->LockRect(pLockedRect, pRect, Flags);
	}
	// Lock 3D surface
	else if (surface.Surface)
	{
		return surface.Surface->LockRect(pLockedRect, pRect, Flags);
	}
	// Lock surface texture
	else if (surface.Texture)
	{
		return surface.Texture->LockRect(GetD3d9MipMapLevel(MipMapLevel), pLockedRect, pRect, Flags);
	}

	return DDERR_GENERIC;
}

HRESULT m_IDirectDrawSurfaceX::ReleaseDC(HDC hDC)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")" <<
		" DC = " << hDC;

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

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

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
			}
			else
			{
				// Get surface
				IDirect3DSurface9* pSurfaceD9 = Get3DMipMapSurface(0);
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

		} while (false);

		ReleaseSurfaceCriticalSection();

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

		if (SUCCEEDED(hr))
		{
			// Set dirty flag
			SetDirtyFlag();

			// Keep surface insync
			EndWriteSyncSurfaces(nullptr);

			// Present surface
			EndWritePresent(nullptr, true, true, false);
		}

		if (FAILED(hr))
		{
			hr = IsSurfaceBusy() ? DDERR_SURFACEBUSY : IsLost() == DDERR_SURFACELOST ? DDERR_SURFACELOST : DDERR_GENERIC;
		}

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

		switch (ddrawParent->TestD3D9CooperativeLevel())
		{
		case D3DERR_DEVICENOTRESET:
			if (FAILED(ddrawParent->ReinitDevice()))
			{
				return DDERR_WRONGMODE;
			}
			[[fallthrough]];
		case D3D_OK:
		case DDERR_NOEXCLUSIVEMODE:
			if (FAILED(CheckInterface(__FUNCTION__, true, true, false)))
			{
				return DDERR_WRONGMODE;
			}
			IsSurfaceLost = false;
			return DD_OK;
		case D3DERR_DEVICELOST:
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
			LOG_LIMIT(100, __FUNCTION__ << " Warning: color key overlay not supported!");
			break;
		case DDCKEY_SRCBLT:
			dds = DDSD_CKSRCBLT;
			break;
		case DDCKEY_SRCOVERLAY:
			dds = DDSD_CKSRCOVERLAY;
			LOG_LIMIT(100, __FUNCTION__ << " Warning: color key overlay not supported!");
			break;
		default:
			return DDERR_INVALIDPARAMS;
		}

		// Check for color space
		if (lpDDColorKey && (dwFlags & DDCKEY_COLORSPACE) && lpDDColorKey->dwColorSpaceLowValue != lpDDColorKey->dwColorSpaceHighValue)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: color space not supported!");
			return DDERR_NOCOLORKEYHW;
		}

		// Reset shader flag
		if (dds == DDSD_CKSRCBLT)
		{
			ShaderColorKey.IsSet = false;
		}

		// Set color key
		if (!lpDDColorKey)
		{
			surfaceDesc2.dwFlags &= ~dds;
		}
		else
		{
			// You must add the flag DDCKEY_COLORSPACE, otherwise DirectDraw will collapse the range to one value
			DDCOLORKEY ColorKey = { lpDDColorKey->dwColorSpaceLowValue, lpDDColorKey->dwColorSpaceLowValue };

			// Set color key flag
			surfaceDesc2.dwFlags |= dds;

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

HRESULT m_IDirectDrawSurfaceX::Unlock(LPRECT lpRect, DWORD MipMapLevel)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")" <<
		" Rect = " << lpRect <<
		" MipMapLevel = " << MipMapLevel;

	if (Config.Dd7to9)
	{

		// Handle dummy mipmaps
		if (IsDummyMipMap(MipMapLevel))
		{
			return DD_OK;
		}

		SetSurfaceCriticalSection();

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

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
			HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, false);
			if (FAILED(c_hr))
			{
				hr = c_hr;
				break;
			}

			// Emulate unlock
			if (EmuLock.Locked)
			{
				UnlockEmuLock();
			}

			// Remove scanlines before unlocking surface
			if (Config.DdrawRemoveScanlines && IsPrimaryOrBackBuffer())
			{
				RemoveScanlines(LastLock);
			}

			// Emulated surface
			if (IsUsingEmulation())
			{
				// No need to unlock emulated surface
			}
			// Lock surface
			else if (surface.Surface || surface.Texture)
			{
				HRESULT ret = UnLockD3d9Surface(MipMapLevel);
				if (FAILED(ret))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to unlock surface texture");
					hr = (ret == DDERR_WASSTILLDRAWING) ? DDERR_WASSTILLDRAWING :
						IsLost() == DDERR_SURFACELOST ? DDERR_SURFACELOST : DDERR_GENERIC;
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

		} while (false);

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

		// If surface was changed
		if (SUCCEEDED(hr) && !LastLock.ReadOnly)
		{
			// Set dirty flag
			if (LastLock.MipMapLevel == 0)
			{
				SetDirtyFlag();
			}

			// Keep surface insync
			EndWriteSyncSurfaces(&LastLock.Rect);

			// Present surface
			EndWritePresent(&LastLock.Rect, true, true, LastLock.IsSkipScene);
		}

		ReleaseSurfaceCriticalSection();

		return hr;
	}

	return ProxyInterface->Unlock(lpRect);
}

inline HRESULT m_IDirectDrawSurfaceX::UnLockD3d9Surface(DWORD MipMapLevel)
{
	if (surface.UsingSurfaceMemory)
	{
		return DD_OK;
	}
	// Unlock shadow surface
	else if (IsUsingShadowSurface())
	{
		return surface.Shadow->UnlockRect();
	}
	// Unlock 3D surface
	else if (surface.Surface)
	{
		return surface.Surface->UnlockRect();
	}
	// Unlock surface texture
	else if (surface.Texture)
	{
		return surface.Texture->UnlockRect(GetD3d9MipMapLevel(MipMapLevel));
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
		if ((SurfaceFlags & DDSD_LPSURFACE) && lpDDSurfaceDesc2->lpSurface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: lpSurface not fully Implemented.");

			SurfaceFlags &= ~DDSD_LPSURFACE;
			surfaceDesc2.dwFlags |= DDSD_LPSURFACE;
			surfaceDesc2.lpSurface = lpDDSurfaceDesc2->lpSurface;
			surface.UsingSurfaceMemory = true;
			if (surface.Surface || surface.Texture)
			{
				CreateD3d9Surface();
			}
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

		if (surface.Surface)
		{
			return surface.Surface->SetPrivateData(guidTag, lpData, cbSize, dwFlags);
		}
		else if (surface.Texture)
		{
			return surface.Texture->SetPrivateData(guidTag, lpData, cbSize, dwFlags);
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

		if (surface.Surface)
		{
			return surface.Surface->GetPrivateData(guidTag, lpBuffer, lpcbBufferSize);
		}
		else if (surface.Texture)
		{
			return surface.Texture->GetPrivateData(guidTag, lpBuffer, lpcbBufferSize);
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

		if (surface.Surface)
		{
			return surface.Surface->FreePrivateData(guidTag);
		}
		else if (surface.Texture)
		{
			return surface.Texture->FreePrivateData(guidTag);
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
		if ((surfaceDesc2.ddsCaps.dwCaps2 & DDSCAPS2_TEXTUREMANAGE) == 0)
		{
			return DDERR_INVALIDOBJECT;
		}

		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		if (surface.Texture)
		{
			surface.Texture->SetLOD(dwMaxLOD);
		}

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
		if ((surfaceDesc2.ddsCaps.dwCaps2 & DDSCAPS2_TEXTUREMANAGE) == 0)
		{
			return DDERR_INVALIDOBJECT;
		}

		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		*lpdwMaxLOD = 0;
		if (surface.Texture)
		{
			*lpdwMaxLOD = surface.Texture->GetLOD();
		}

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
	ReleaseCriticalSection();

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

	if (attached3DTexture)
	{
		attached3DTexture->ClearSurface();
	}

	if (ddrawParent)
	{
		ddrawParent->RemoveSurfaceFromVector(this);
	}
}

void m_IDirectDrawSurfaceX::ReleaseSurface()
{
	// Don't delete surface wrapper v1 interface
	if (WrapperInterface)
	{
		SetCriticalSection();
		WrapperInterface->SetProxy(nullptr);
		SurfaceWrapperListV1.push_back(WrapperInterface);
		ReleaseCriticalSection();
	}
	if (WrapperInterface2)
	{
		WrapperInterface2->DeleteMe();
	}
	if (WrapperInterface3)
	{
		WrapperInterface3->DeleteMe();
	}
	if (WrapperInterface4)
	{
		WrapperInterface4->DeleteMe();
	}
	if (WrapperInterface7)
	{
		WrapperInterface7->DeleteMe();
	}

	// Clean up mipmaps
	if (!MipMaps.empty())
	{
		for (auto& entry : MipMaps)
		{
			if (entry.Addr) entry.Addr->DeleteMe();
			if (entry.Addr2) entry.Addr2->DeleteMe();
			if (entry.Addr3) entry.Addr3->DeleteMe();
			if (entry.Addr4) entry.Addr4->DeleteMe();
			if (entry.Addr7) entry.Addr7->DeleteMe();
		}
	}

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

LPDIRECT3DSURFACE9 m_IDirectDrawSurfaceX::GetD3d9Surface()
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, true, true)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface not setup!");
		return nullptr;
	}

	return Get3DSurface();
}

inline LPDIRECT3DSURFACE9 m_IDirectDrawSurfaceX::Get3DSurface()
{
	if (surface.Surface)
	{
		return surface.Surface;
	}
	else if (surface.Texture)
	{
		if (surface.Context || (!surface.Context && SUCCEEDED(surface.Texture->GetSurfaceLevel(0, &surface.Context))))
		{
			return surface.Context;
		}
	}
	return nullptr;
}

inline LPDIRECT3DSURFACE9 m_IDirectDrawSurfaceX::Get3DMipMapSurface(DWORD MipMapLevel)
{
	if (IsUsingShadowSurface())
	{
		return surface.Shadow;
	}
	else if (MipMapLevel == 0 || surface.Type != D3DTYPE_TEXTURE)
	{
		return Get3DSurface();
	}
	else if (surface.Texture)
	{
		LPDIRECT3DSURFACE9 pSurfaceD9 = nullptr;
		surface.Texture->GetSurfaceLevel(GetD3d9MipMapLevel(MipMapLevel), &pSurfaceD9);
		return pSurfaceD9;
	}
	return nullptr;
}

inline void m_IDirectDrawSurfaceX::Release3DMipMapSurface(LPDIRECT3DSURFACE9 pSurfaceD9, DWORD MipMapLevel)
{
	if (pSurfaceD9 && MipMapLevel != 0 && surface.Type == D3DTYPE_TEXTURE && !IsUsingShadowSurface())
	{
		pSurfaceD9->Release();
	}
}

LPDIRECT3DTEXTURE9 m_IDirectDrawSurfaceX::GetD3d9DrawTexture()
{
	// Check if texture already exists
	if (surface.DrawTexture)
	{
		if (surface.IsDrawTextureDirty && FAILED(CopyToDrawTexture(nullptr)))
		{
			return nullptr;
		}
		return surface.DrawTexture;
	}

	// Create texture
	if (surface.Texture)
	{
		DWORD Level = IsMipMapAutogen() ? 0 : MaxMipMapLevel + 1;
		if (FAILED((*d3d9Device)->CreateTexture(surface.Width, surface.Height, Level, surface.Usage, D3DFMT_A8R8G8B8, surface.Pool, &surface.DrawTexture, nullptr)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create surface texture. Size: " << surface.Width << "x" << surface.Height <<
				" Format: " << surface.Format << " dwCaps: " << surfaceDesc2.ddsCaps);
			return nullptr;
		}
		if (FAILED(CopyToDrawTexture(nullptr)))
		{
			return nullptr;
		}
		return surface.DrawTexture;
	}
	return nullptr;
}

LPDIRECT3DTEXTURE9 m_IDirectDrawSurfaceX::GetD3d9Texture()
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, true, true)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: texture not setup!");
		return nullptr;
	}

	// Check texture pool
	if ((surface.Pool == D3DPOOL_SYSTEMMEM || surface.Pool == D3DPOOL_SCRATCH) && IsSurfaceTexture())
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: texture pool does not support Driect3D: " << surface.Format << " Pool: " << surface.Pool <<
			" Caps: " << surfaceDesc2.ddsCaps << " Attached: " << attached3DTexture);
		return nullptr;
	}

	// Check if surface is render target
	if (IsRenderTarget())
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Render Target textures not implemented!");
	}

	return Get3DTexture();
}

inline LPDIRECT3DTEXTURE9 m_IDirectDrawSurfaceX::Get3DTexture()
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

void m_IDirectDrawSurfaceX::CheckMipMapLevelGen()
{
	if (!IsMipMapReadyToUse)
	{
		for (UINT x = 0; x < min(MaxMipMapLevel, MipMaps.size()); x++)
		{
			if (!MipMaps[x].IsDummy && MipMaps[x].UniquenessValue < UniquenessValue)
			{
				return;
			}
		}
		IsMipMapReadyToUse = true;
	}
}

HRESULT m_IDirectDrawSurfaceX::GenerateMipMapLevels()
{
	IDirect3DSurface9* pSourceSurfaceD9 = Get3DMipMapSurface(0);
	if (!pSourceSurfaceD9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surface level!");
		return DDERR_GENERIC;
	}

	for (UINT x = 0; x < min(MaxMipMapLevel, MipMaps.size()); x++)
	{
		if (!MipMaps[x].IsDummy && MipMaps[x].UniquenessValue < UniquenessValue)
		{
			IDirect3DSurface9* pDestSurfaceD9 = Get3DMipMapSurface(x + 1);
			if (pDestSurfaceD9)
			{
				LOG_LIMIT(100, __FUNCTION__ << " (" << this << ") Warning: attempting to add missing data to MipMap surface level: " << (x + 1));
				if (SUCCEEDED(D3DXLoadSurfaceFromSurface(pDestSurfaceD9, nullptr, nullptr, pSourceSurfaceD9, nullptr, nullptr, D3DX_FILTER_LINEAR, 0x00000000)))
				{
					MipMaps[x].UniquenessValue = UniquenessValue;
				}
				else
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not copy MipMap surface level!");
				}
				Release3DMipMapSurface(pDestSurfaceD9, x + 1);
			}
		}
	}

	CheckMipMapLevelGen();

	return DD_OK;
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
		if (ShouldPresentToWindow(true))
		{
			HWND hWnd = nullptr;
			if (attachedClipper)
			{
				attachedClipper->GetHWnd(&hWnd);
			}
			if (!IsWindow(hWnd))
			{
				hWnd = Fullscreen::FindMainWindow(GetCurrentProcessId(), true);
			}
			if (hWnd != ddrawParent->GetClipperHWnd())
			{
				ddrawParent->SetClipperHWnd(hWnd);
			}
		}
	}

	// Check if device is lost
	if (CheckLostSurface && (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY))
	{
		HRESULT hr = ddrawParent->TestD3D9CooperativeLevel();
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

	// Check surface
	if (CheckD3DSurface)
	{
		// Check if using windowed mode
		bool LastWindowedMode = surface.IsUsingWindowedMode;
		surface.IsUsingWindowedMode = !ddrawParent->IsExclusiveMode();

		// Check if using Direct3D
		bool LastUsing3D = Using3D;
		Using3D = ddrawParent->IsUsing3D();

		// Remove emulated surface if not needed
		if (Using3D && !LastUsing3D && IsUsingEmulation() && !Config.DdrawEmulateSurface && !SurfaceRequiresEmulation && !IsSurfaceBusy())
		{
			ReleaseDCSurface();
		}

		// Clear Using 3D if not needed
		if (!Using3D && LastUsing3D)
		{
			ClearUsing3DFlag();
		}

		// Release d3d9 surface
		if (attached3DTexture && surface.Pool != D3DPOOL_MANAGED)
		{
			ReleaseD9Surface(true, false);
		}

		// Make sure surface exists, if not then create it
		if ((!surface.Surface && !surface.Texture) ||
			(IsPrimaryOrBackBuffer() && LastWindowedMode != surface.IsUsingWindowedMode) ||
			(PrimaryDisplayTexture && !ShouldPresentToWindow(false)))
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
	if ((surface.Surface || surface.Texture) && IsSurfaceBusy())
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface is busy! Locked: " << IsSurfaceLocked() << " DC: " << IsSurfaceInDC() << " Blt: " << IsSurfaceBlitting() << " ThreadID " << LockedWithID);
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
	surface.Format = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);
	surface.BitCount = GetBitCount(surface.Format);
	SurfaceRequiresEmulation = ((surface.Format == D3DFMT_A8B8G8R8 || surface.Format == D3DFMT_X8B8G8R8 || surface.Format == D3DFMT_B8G8R8 || surface.Format == D3DFMT_R8G8B8 ||
		Config.DdrawEmulateSurface || (Config.DdrawRemoveScanlines && IsPrimaryOrBackBuffer()) || ShouldEmulate == SC_FORCE_EMULATED) &&
			!IsDepthStencil() && !(surface.Format & 0xFF000000 /*FOURCC or D3DFMT_DXTx*/) && !surface.UsingSurfaceMemory);
	const bool IsSurfaceEmulated = (SurfaceRequiresEmulation || (IsPrimaryOrBackBuffer() && (Config.DdrawWriteToGDI || Config.DdrawReadFromGDI) && !Using3D));
	DCRequiresEmulation = (surface.Format != D3DFMT_R5G6B5 && surface.Format != D3DFMT_X1R5G5B5 && surface.Format != D3DFMT_R8G8B8 && surface.Format != D3DFMT_X8R8G8B8);
	const D3DFORMAT Format = ((surfaceDesc2.ddsCaps.dwCaps2 & DDSCAPS2_NOTUSERLOCKABLE) && surface.Format == D3DFMT_D16_LOCKABLE) ? D3DFMT_D16 : ConvertSurfaceFormat(surface.Format);

	// Check if surface should be a texture
	bool IsTexture = ((IsPrimaryOrBackBuffer() && !ShouldPresentToWindow(false)) || IsPalette() || IsSurfaceTexture());

	// Get memory pool
	bool UseVideoMemory = false;
	surface.Pool = (IsPrimaryOrBackBuffer() && ShouldPresentToWindow(false)) ? D3DPOOL_SYSTEMMEM :
		(surfaceDesc2.ddsCaps.dwCaps2 & (DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE)) ? D3DPOOL_MANAGED :
		UseVideoMemory ? (((surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) || IsPrimaryOrBackBuffer()) ? D3DPOOL_DEFAULT :
			(surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) ? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT) :
		((IsPrimaryOrBackBuffer() || IsSurfaceTexture()) ? D3DPOOL_MANAGED :									// For now use managed for all textures
			(surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) ? D3DPOOL_SYSTEMMEM : D3DPOOL_SYSTEMMEM);		// Default to system memory for Lock()

	// Default usage
	surface.Usage = 0;

	// Adjust Width to be byte-aligned
	const DWORD Width = GetByteAlignedWidth(surfaceDesc2.dwWidth, surface.BitCount);
	const DWORD Height = surfaceDesc2.dwHeight;

	// Anti-aliasing
	if (IsRenderTarget())
	{
		bool AntiAliasing = (surfaceDesc2.ddsCaps.dwCaps2 & DDSCAPS2_HINTANTIALIASING);
		if (AntiAliasing && !surface.MultiSampleType)
		{
			// Default to 8 samples as some games have issues with more samples
			DWORD MaxSamples = (surfaceDesc2.ddsCaps.dwCaps3 & DDSCAPS3_MULTISAMPLE_MASK) ? (surfaceDesc2.ddsCaps.dwCaps3 & DDSCAPS3_MULTISAMPLE_MASK) : D3DMULTISAMPLE_8_SAMPLES;
			surface.MultiSampleType = ddrawParent->GetMultiSampleTypeQuality(Format, MaxSamples, surface.MultiSampleQuality);
		}
	}

	// Set created by
	ShouldEmulate = (ShouldEmulate == SC_NOT_CREATED) ? SC_DONT_FORCE : ShouldEmulate;

	Logging::LogDebug() << __FUNCTION__ " (" << this << ") D3d9 Surface. Size: " << Width << "x" << Height << " Format: " << surface.Format <<
		" Pool: " << surface.Pool << " dwCaps: " << surfaceDesc2.ddsCaps << " " << surfaceDesc2;

	HRESULT hr = DD_OK;

	do {
		// Create depth stencil
		if (IsDepthStencil())
		{
			surface.Type = D3DTYPE_DEPTHSTENCIL;
			surface.Usage = D3DUSAGE_DEPTHSTENCIL;
			surface.Pool = D3DPOOL_DEFAULT;
			if (FAILED((*d3d9Device)->CreateDepthStencilSurface(Width, Height, Format, surface.MultiSampleType, surface.MultiSampleQuality, surface.MultiSampleType ? TRUE : FALSE, &surface.Surface, nullptr)) &&
				FAILED((*d3d9Device)->CreateDepthStencilSurface(Width, Height, GetFailoverFormat(Format), surface.MultiSampleType, surface.MultiSampleQuality, surface.MultiSampleType ? TRUE : FALSE, &surface.Surface, nullptr)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create depth stencil surface. Size: " << Width << "x" << Height << " Format: " << surface.Format << " dwCaps: " << surfaceDesc2.ddsCaps);
				hr = DDERR_GENERIC;
				break;
			}
		}
		// Create render target
		else if (IsRenderTarget())
		{
			// ToDo: if render surface is a texture then create as a texture (MipMaps can be supported on render target textures)
			surface.Usage = D3DUSAGE_RENDERTARGET;
			surface.Pool = D3DPOOL_DEFAULT;
			if (IsSurfaceTexture())
			{
				surface.Type = D3DTYPE_TEXTURE;
				if (FAILED((*d3d9Device)->CreateTexture(Width, Height, 1, surface.Usage, Format, surface.Pool, &surface.Texture, nullptr)) &&
					FAILED((*d3d9Device)->CreateTexture(Width, Height, 1, surface.Usage, GetFailoverFormat(Format), surface.Pool, &surface.Texture, nullptr)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create render target texture. Size: " << Width << "x" << Height << " Format: " << surface.Format << " dwCaps: " << surfaceDesc2.ddsCaps);
					hr = DDERR_GENERIC;
					break;
				}
			}
			else
			{
				surface.Type = D3DTYPE_RENDERTARGET;
				BOOL IsLockable = (surface.MultiSampleType || (surfaceDesc2.ddsCaps.dwCaps2 & DDSCAPS2_NOTUSERLOCKABLE)) ? FALSE : TRUE;
				if (FAILED((*d3d9Device)->CreateRenderTarget(Width, Height, Format, surface.MultiSampleType, surface.MultiSampleQuality, IsLockable, &surface.Surface, nullptr)) &&
					FAILED((*d3d9Device)->CreateRenderTarget(Width, Height, GetFailoverFormat(Format), surface.MultiSampleType, surface.MultiSampleQuality, IsLockable, &surface.Surface, nullptr)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create render target surface. Size: " << Width << "x" << Height << " Format: " << surface.Format << " dwCaps: " << surfaceDesc2.ddsCaps);
					hr = DDERR_GENERIC;
					break;
				}
			}
			// Update attached stencil surface
			m_IDirectDrawSurfaceX* lpAttachedSurfaceX = GetAttachedDepthStencil();
			if (lpAttachedSurfaceX)
			{
				UpdateAttachedDepthStencil(lpAttachedSurfaceX);
			}
		}
		// Create texture
		else if (IsTexture)
		{
			surface.Type = D3DTYPE_TEXTURE;
			DWORD MipMapCount = (surfaceDesc2.dwFlags & DDSD_MIPMAPCOUNT) ? surfaceDesc2.dwMipMapCount : 1;
			DWORD MipMapLevel = (SurfaceRequiresEmulation || !MipMapCount) ? 1 : MipMapCount;
			HRESULT hr_t;
			do {
				surface.Usage = Config.DdrawForceMipMapAutoGen && MipMapLevel > 1 ? D3DUSAGE_AUTOGENMIPMAP : 0;
				DWORD Level = surface.Usage == D3DUSAGE_AUTOGENMIPMAP && MipMapLevel == MipMapCount ? 0 : MipMapLevel;
				// Create texture
				hr_t = (*d3d9Device)->CreateTexture(Width, Height, Level, surface.Usage, Format, surface.Pool, &surface.Texture, nullptr);
				if (FAILED(hr_t))
				{
					hr_t = (*d3d9Device)->CreateTexture(Width, Height, Level, surface.Usage, GetFailoverFormat(Format), surface.Pool, &surface.Texture, nullptr);
				}
			} while (FAILED(hr_t) && ((!MipMapLevel && ++MipMapLevel) || --MipMapLevel > 0));
			if (FAILED(hr_t))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create surface texture. Size: " << Width << "x" << Height << " Format: " << surface.Format << " dwCaps: " << surfaceDesc2.ddsCaps);
				hr = DDERR_GENERIC;
				break;
			}
			MaxMipMapLevel = MipMapLevel > 1 && !IsMipMapAutogen() ? MipMapLevel - 1 : 0;
			while (MipMaps.size() < MaxMipMapLevel)
			{
				MIPMAP MipMap;
				MipMaps.push_back(MipMap);
			}
			if ((surfaceDesc2.dwFlags & DDSD_MIPMAPCOUNT) && !IsMipMapAutogen())
			{
				surfaceDesc2.dwMipMapCount = MipMapLevel;
			}
		}
		else
		{
			surface.Type = D3DTYPE_OFFPLAINSURFACE;
			if (FAILED((*d3d9Device)->CreateOffscreenPlainSurface(Width, Height, Format, surface.Pool, &surface.Surface, nullptr)) &&
				FAILED((*d3d9Device)->CreateOffscreenPlainSurface(Width, Height, GetFailoverFormat(Format), surface.Pool, &surface.Surface, nullptr)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create offplain surface. Size: " << Width << "x" << Height << " Format: " << Format << " dwCaps: " << surfaceDesc2.ddsCaps);
				hr = DDERR_GENERIC;
				break;
			}
		}

		// Create shadow surface
		surface.UsingShadowSurface = false;
		if (surface.Usage == D3DUSAGE_RENDERTARGET)
		{
			D3DSURFACE_DESC Desc;
			if (FAILED(surface.Surface ? surface.Surface->GetDesc(&Desc) : surface.Texture->GetLevelDesc(0, &Desc)) ||
				FAILED((*d3d9Device)->CreateOffscreenPlainSurface(Desc.Width, Desc.Height, Desc.Format, D3DPOOL_SYSTEMMEM, &surface.Shadow, nullptr)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create shadow surface. Size: " << Width << "x" << Height << " Format: " << Format << " dwCaps: " << surfaceDesc2.ddsCaps);
				hr = DDERR_GENERIC;
				break;
			}
		}

		// Create primary surface texture
		if (ShouldPresentToWindow(false))
		{
			if (FAILED((*d3d9Device)->CreateTexture(Width, Height, 1, 0, Format, D3DPOOL_DEFAULT, &PrimaryDisplayTexture, nullptr)) &&
				FAILED((*d3d9Device)->CreateTexture(Width, Height, 1, 0, GetFailoverFormat(Format), D3DPOOL_DEFAULT, &PrimaryDisplayTexture, nullptr)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create primary surface texture. Size: " << Width << "x" << Height << " Format: " << surface.Format << " dwCaps: " << surfaceDesc2.ddsCaps);
				hr = DDERR_GENERIC;
				break;
			}
		}

		// Create palette surface
		if (IsPrimarySurface() && surface.Format == D3DFMT_P8 && !IsRenderTarget())
		{
			if (FAILED((*d3d9Device)->CreateTexture(MaxPaletteSize, MaxPaletteSize, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &primary.PaletteTexture, nullptr)))
			{
				// Try failover format
				if (FAILED((*d3d9Device)->CreateTexture(MaxPaletteSize, MaxPaletteSize, 1, 0, GetFailoverFormat(D3DFMT_X8R8G8B8), D3DPOOL_MANAGED, &primary.PaletteTexture, nullptr)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create palette surface texture");
					hr = DDERR_GENERIC;
					break;
				}
			}
		}

		surface.Width = Width;
		surface.Height = Height;
		surface.IsPaletteDirty = IsPalette();

	} while (false);

	// Create emulated surface using device context for creation
	bool EmuSurfaceCreated = false;
	if ((IsSurfaceEmulated || IsUsingEmulation()) && !DoesDCMatch(surface.emu))
	{
		EmuSurfaceCreated = true;
		CreateDCSurface();
	}

	// Reset flags
	surface.HasData = false;

	// Restore d3d9 surface texture data
	if (surface.Surface || surface.Texture)
	{
		bool RestoreData = false;
		if (IsUsingEmulation() && !EmuSurfaceCreated)
		{
			// Copy surface to emulated surface
			CopyFromEmulatedSurface(nullptr);
			RestoreData = true;
			surface.HasData = true;
		}
		else if (!LostDeviceBackup.empty())
		{
			if (LostDeviceBackup[0].Format == surface.Format && LostDeviceBackup[0].Width == surface.Width && LostDeviceBackup[0].Height == surface.Height)
			{
				for (UINT Level = 0; Level < LostDeviceBackup.size(); Level++)
				{
					D3DLOCKED_RECT LockRect = {};
					if (FAILED(LockD3d9Surface(&LockRect, nullptr, 0, Level)))
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to restore surface data!");
						break;
					}

					Logging::LogDebug() << __FUNCTION__ << " Restoring Direct3D9 texture surface data: " << surface.Format;

					D3DSURFACE_DESC Desc = {};
					if (FAILED(surface.Surface ? surface.Surface->GetDesc(&Desc) : surface.Texture->GetLevelDesc(GetD3d9MipMapLevel(Level), &Desc)))
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get surface desc!");
						break;
					}

					size_t size = GetSurfaceSize(surface.Format, Desc.Width, Desc.Height, LockRect.Pitch);

					if (size == LostDeviceBackup[Level].Bits.size())
					{
						memcpy(LockRect.pBits, LostDeviceBackup[Level].Bits.data(), size);

						RestoreData = true;
						surface.HasData = true;
					}
					else
					{
						LOG_LIMIT(100, __FUNCTION__ << " Warning: restore backup surface data mismatch! For Level: " << Level << " " <<
							LostDeviceBackup[Level].Format << " -> " << surface.Format << " " << LostDeviceBackup[Level].Width << "x" << LostDeviceBackup[Level].Height << " -> " <<
							surface.Width << "x" << surface.Height);
					}

					UnLockD3d9Surface(Level);

					// Copy surface to emulated surface
					if (IsUsingEmulation() && Level == 0 && surface.HasData)
					{
						CopyToEmulatedSurface(nullptr);
					}
				}
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: restore backup surface data mismatch!: " <<
					LostDeviceBackup[0].Format << " -> " << surface.Format << " " << LostDeviceBackup[0].Width << "x" << LostDeviceBackup[0].Height << " -> " <<
					surface.Width << "x" << surface.Height);
			}
		}

		// Copy surface to display texture
		if (RestoreData && PrimaryDisplayTexture)
		{
			IDirect3DSurface9* pSrcSurfaceD9 = Get3DSurface();
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
		LostDeviceBackup.clear();
	}

	// Delete emulatd surface if not needed
	if (!IsSurfaceEmulated && IsUsingEmulation())
	{
		ReleaseDCSurface();
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
	DWORD Width = GetByteAlignedWidth(surfaceDesc2.dwWidth, surface.BitCount);
	DWORD Height = surfaceDesc2.dwHeight;
	DWORD Pitch = ComputePitch(Width, surface.BitCount);

	if (pEmuSurface->bmi->bmiHeader.biWidth == (LONG)Width &&
		pEmuSurface->bmi->bmiHeader.biHeight == -(LONG)Height &&
		pEmuSurface->bmi->bmiHeader.biBitCount == surface.BitCount &&
		pEmuSurface->Format == surface.Format &&
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
	bool ColorMaskReq = ((surface.BitCount == 16 || surface.BitCount == 24 || surface.BitCount == 32) &&									// Only valid when used with 16 bit, 24 bit and 32 bit surfaces
		(surfaceDesc2.ddpfPixelFormat.dwFlags & DDPF_RGB) &&																				// Check to make sure it is an RGB surface
		(surfaceDesc2.ddpfPixelFormat.dwRBitMask && surfaceDesc2.ddpfPixelFormat.dwGBitMask && surfaceDesc2.ddpfPixelFormat.dwBBitMask));	// Check to make sure the masks actually exist

	// Adjust Width to be byte-aligned
	DWORD Width = GetByteAlignedWidth(surfaceDesc2.dwWidth, surface.BitCount);
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

	Logging::LogDebug() << __FUNCTION__ " (" << this << ") creating emulated surface. Size: " << Width << "x" << Height << " Format: " << surface.Format << " dwCaps: " << surfaceDesc2.ddsCaps;

	// Create new emulated surface structure
	surface.emu = new EMUSURFACE;

	// Create device context memory
	ZeroMemory(surface.emu->bmiMemory, sizeof(surface.emu->bmiMemory));
	surface.emu->bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	surface.emu->bmi->bmiHeader.biWidth = Width;
	surface.emu->bmi->bmiHeader.biHeight = -((LONG)Height + 200);
	surface.emu->bmi->bmiHeader.biPlanes = 1;
	surface.emu->bmi->bmiHeader.biBitCount = (WORD)surface.BitCount;
	surface.emu->bmi->bmiHeader.biCompression =
		(surface.BitCount == 8 || surface.BitCount == 24) ? BI_RGB :
		(ColorMaskReq) ? BI_BITFIELDS : 0;	// BI_BITFIELDS is only valid for 16-bpp and 32-bpp bitmaps.
	surface.emu->bmi->bmiHeader.biSizeImage = ((Width * surface.BitCount + 31) & ~31) / 8 * Height;

	if (surface.BitCount == 8)
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
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set bmi colors! " << surface.BitCount);
		DeleteEmulatedMemory(&surface.emu);
		return DDERR_GENERIC;
	}
	HDC hDC = ddrawParent->GetDC();
	surface.emu->DC = CreateCompatibleDC(hDC);
	surface.emu->GameDC = CreateCompatibleDC(hDC);
	if (!surface.emu->DC || !surface.emu->GameDC)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create compatible DC: " << hDC << " " << surface.Format);
		DeleteEmulatedMemory(&surface.emu);
		return DDERR_GENERIC;
	}
	surface.emu->bitmap = CreateDIBSection(surface.emu->DC, surface.emu->bmi, (surface.BitCount == 8) ? DIB_PAL_COLORS : DIB_RGB_COLORS, (void**)&surface.emu->pBits, nullptr, 0);
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
	surface.emu->Format = surface.Format;
	surface.emu->Pitch = ComputePitch(surface.emu->bmi->bmiHeader.biWidth, surface.emu->bmi->bmiHeader.biBitCount);
	surface.emu->Size = Height * surface.emu->Pitch;

	return DD_OK;
}

void m_IDirectDrawSurfaceX::UpdateAttachedDepthStencil(m_IDirectDrawSurfaceX* lpAttachedSurfaceX)
{
	bool HasChanged = false;
	// Verify depth stencil's with and height
	if ((surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT)) == (DDSD_WIDTH | DDSD_HEIGHT) &&
		(surfaceDesc2.dwWidth != lpAttachedSurfaceX->surfaceDesc2.dwWidth || surfaceDesc2.dwHeight != lpAttachedSurfaceX->surfaceDesc2.dwHeight))
	{
		HasChanged = true;
		lpAttachedSurfaceX->surfaceDesc2.dwWidth = surfaceDesc2.dwWidth;
		lpAttachedSurfaceX->surfaceDesc2.dwHeight = surfaceDesc2.dwHeight;
	}
	// Set depth stencil multisampling
	if (lpAttachedSurfaceX && (surface.MultiSampleType != lpAttachedSurfaceX->surface.MultiSampleType || surface.MultiSampleQuality != lpAttachedSurfaceX->surface.MultiSampleQuality))
	{
		HasChanged = true;
		lpAttachedSurfaceX->surface.MultiSampleType = surface.MultiSampleType;
		lpAttachedSurfaceX->surface.MultiSampleQuality = surface.MultiSampleQuality;
	}
	// If depth stencil changed
	if (HasChanged)
	{
		lpAttachedSurfaceX->ReleaseD9Surface(false, false);
	}
	// Set depth stencil
	if (ddrawParent->GetRenderTargetSurface() == this)
	{
		ddrawParent->SetDepthStencilSurface(lpAttachedSurfaceX);
	}
}

// Update surface description
void m_IDirectDrawSurfaceX::UpdateSurfaceDesc()
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, false, false, false)))
	{
		return;
	}

	bool IsChanged = false;
	if ((surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT)) != (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT) ||
		((surfaceDesc2.dwFlags & DDSD_REFRESHRATE) && !surfaceDesc2.dwRefreshRate))
	{
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
			IsChanged = true;
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
			IsChanged = true;
		}
		// Reset MipMap level pitch
		if (IsChanged && MipMaps.size())
		{
			for (auto& entry : MipMaps)
			{
				entry.dwWidth = 0;
				entry.dwHeight = 0;
				entry.lPitch = 0;
			}
		}
	}
	// Unset lPitch
	if ((((surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT)) != (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT) ||
		!(surfaceDesc2.dwFlags & DDSD_PITCH)) && !(surfaceDesc2.dwFlags & DDSD_LINEARSIZE)) || !surfaceDesc2.lPitch)
	{
		surfaceDesc2.dwFlags &= ~(DDSD_PITCH | DDSD_LINEARSIZE);
		surfaceDesc2.lPitch = 0;
	}
	// Set lPitch
	if ((surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT)) == (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT) &&
		(surfaceDesc2.ddpfPixelFormat.dwFlags & DDPF_RGB) && !(surfaceDesc2.dwFlags & DDSD_LINEARSIZE) && !(surfaceDesc2.dwFlags & DDSD_PITCH))
	{
		surfaceDesc2.dwFlags |= DDSD_PITCH;
		DWORD BitCount = surface.BitCount ? surface.BitCount : GetBitCount(surfaceDesc2.ddpfPixelFormat);
		DWORD Width = surface.Width ? surface.Width : GetByteAlignedWidth(surfaceDesc2.dwWidth, BitCount);
		surfaceDesc2.lPitch = surface.UsingSurfaceMemory ? surfaceDesc2.dwWidth * BitCount : ComputePitch(Width, BitCount);
	}
	// Set surface format
	if (surface.Format == D3DFMT_UNKNOWN && (surfaceDesc2.dwFlags & DDSD_PIXELFORMAT))
	{
		surface.Format = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);
	}
	// Set attached stencil surface size
	if (IsChanged && (surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT)) == (DDSD_WIDTH | DDSD_HEIGHT))
	{
		m_IDirectDrawSurfaceX* lpAttachedSurfaceX = GetAttachedDepthStencil();
		if (lpAttachedSurfaceX && (surfaceDesc2.dwWidth != lpAttachedSurfaceX->surfaceDesc2.dwWidth || surfaceDesc2.dwHeight != lpAttachedSurfaceX->surfaceDesc2.dwHeight))
		{
			lpAttachedSurfaceX->ReleaseD9Surface(false, false);
			lpAttachedSurfaceX->surfaceDesc2.dwWidth = surfaceDesc2.dwWidth;
			lpAttachedSurfaceX->surfaceDesc2.dwHeight = surfaceDesc2.dwHeight;
			if (ddrawParent->GetRenderTargetSurface() == this)
			{
				ddrawParent->SetDepthStencilSurface(lpAttachedSurfaceX);
			}
		}
	}
}

void m_IDirectDrawSurfaceX::SetAsRenderTarget()
{
	if (!surface.CanBeRenderTarget)
	{
		surface.CanBeRenderTarget = true;
		if (surface.Surface || surface.Texture)
		{
			CreateD3d9Surface();
		}
		if (!AttachedSurfaceMap.empty())
		{
			for (auto& entry : AttachedSurfaceMap)
			{
				if (entry.second.pSurface->IsPrimaryOrBackBuffer() && entry.second.pSurface->IsSurface3D() && !entry.second.pSurface->IsRenderTarget())
				{
					entry.second.pSurface->SetAsRenderTarget();
				}
			}
		}
	}
}

void m_IDirectDrawSurfaceX::ClearUsing3DFlag()
{
	Using3D = false;

	if (surface.CanBeRenderTarget)
	{
		surface.CanBeRenderTarget = false;
		if (surface.Surface || surface.Texture)
		{
			CreateD3d9Surface();
		}
		if (!AttachedSurfaceMap.empty())
		{
			for (auto& entry : AttachedSurfaceMap)
			{
				if (entry.second.pSurface->IsPrimaryOrBackBuffer() && entry.second.pSurface->IsRenderTarget())
				{
					entry.second.pSurface->ClearUsing3DFlag();
				}
			}
		}
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
}

// Release surface and vertext buffer
void m_IDirectDrawSurfaceX::ReleaseD9Surface(bool BackupData, bool ResetSurface)
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
		UnLockD3d9Surface(0);
		IsLocked = false;
	}
	LockedWithID = 0;

	// Backup d3d9 surface texture
	if (BackupData)
	{
		if (surface.HasData && (surface.Surface || surface.Texture) && !IsRenderTarget() && !IsDepthStencil() && (!ResetSurface || IsD9UsingVideoMemory()))
		{
			IsSurfaceLost = true;
			LostDeviceBackup.clear();

			if (!IsUsingEmulation())
			{
				for (UINT Level = 0; Level < ((IsMipMapAutogen() || !MaxMipMapLevel) ? 1 : MaxMipMapLevel); Level++)
				{
					D3DLOCKED_RECT LockRect = {};
					if (FAILED(LockD3d9Surface(&LockRect, nullptr, D3DLOCK_READONLY, Level)))
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to backup surface data!");
						break;
					}

					Logging::LogDebug() << __FUNCTION__ << " Storing Direct3D9 texture surface data: " << surface.Format;

					D3DSURFACE_DESC Desc = {};
					if (FAILED(surface.Surface ? surface.Surface->GetDesc(&Desc) : surface.Texture->GetLevelDesc(GetD3d9MipMapLevel(Level), &Desc)))
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get surface desc!");
						break;
					}

					size_t size = GetSurfaceSize(surface.Format, Desc.Width, Desc.Height, LockRect.Pitch);

					if (size)
					{
						DDBACKUP entry;
						LostDeviceBackup.push_back(entry);
						LostDeviceBackup[Level].Format = surface.Format;
						LostDeviceBackup[Level].Width = Desc.Width;
						LostDeviceBackup[Level].Height = Desc.Height;
						LostDeviceBackup[Level].Pitch = LockRect.Pitch;
						LostDeviceBackup[Level].Bits.resize(size);

						memcpy(LostDeviceBackup[Level].Bits.data(), LockRect.pBits, size);
					}

					UnLockD3d9Surface(Level);
				}
			}
		}
	}
	// Release emulated surface if not backing up surface
	else if (IsUsingEmulation())
	{
		ReleaseDCSurface();
	}

	ReleaseD9ContextSurface();

	// Release d3d9 3D surface
	if (surface.Surface && (!ResetSurface || IsD9UsingVideoMemory() || IsDepthStencil()))
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 surface";
		ULONG ref = surface.Surface->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'surface3D' " << ref;
		}
		surface.Surface = nullptr;
	}

	// Release d3d9 shadow surface when surface is released
	if (surface.Shadow && !surface.Surface)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 surface";
		ULONG ref = surface.Shadow->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'surface.Shadow' " << ref;
		}
		surface.Shadow = nullptr;
	}

	// Release d3d9 surface texture
	if (surface.Texture && (!ResetSurface || IsD9UsingVideoMemory()))
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 texture surface";
		ULONG ref = surface.Texture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'surfaceTexture' " << ref;
		}
		surface.Texture = nullptr;
	}

	// Release d3d9 color keyed surface texture
	if (surface.DrawTexture && (!ResetSurface || IsD9UsingVideoMemory()))
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 DrawTexture surface";
		ULONG ref = surface.DrawTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'DrawTexture' " << ref;
		}
		surface.DrawTexture = nullptr;
	}

	// Release d3d9 palette surface texture
	if (primary.PaletteTexture && !ResetSurface)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 palette texture surface";
		ULONG ref = primary.PaletteTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'paletteTexture' " << ref;
		}
		primary.PaletteTexture = nullptr;
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
	if (ResetDisplayFlags && !ResetSurface)
	{
		surfaceDesc2.dwFlags &= ~ResetDisplayFlags;
	}
	if (surfaceDesc2.dwFlags & DDSD_REFRESHRATE)
	{
		surfaceDesc2.dwRefreshRate = 0;
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
	if (ShouldWriteToGDI() || ShouldPresentToWindow(true) || ddrawParent->IsInScene())
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

	// Present to d3d9
	HRESULT hr = DD_OK;
	if (FAILED(ddrawParent->PresentScene(nullptr)))
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
inline bool m_IDirectDrawSurfaceX::CheckCoordinates(RECT& OutRect, LPRECT lpInRect, LPDDSURFACEDESC2 lpDDSurfaceDesc2)
{
	if (!lpDDSurfaceDesc2)
	{
		lpDDSurfaceDesc2 = &surfaceDesc2;
	}

	// Check device coordinates
	if (!lpDDSurfaceDesc2->dwWidth || !lpDDSurfaceDesc2->dwHeight)
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
		OutRect.right = lpDDSurfaceDesc2->dwWidth;
		OutRect.bottom = lpDDSurfaceDesc2->dwHeight;
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

	if (OutRect.left > (LONG)lpDDSurfaceDesc2->dwWidth)
	{
		OutRect.left = lpDDSurfaceDesc2->dwWidth;
	}

	if (OutRect.right > (LONG)lpDDSurfaceDesc2->dwWidth)
	{
		OutRect.right = lpDDSurfaceDesc2->dwWidth;
	}

	if (OutRect.top > (LONG)lpDDSurfaceDesc2->dwHeight)
	{
		OutRect.top = lpDDSurfaceDesc2->dwHeight;
	}

	if (OutRect.bottom > (LONG)lpDDSurfaceDesc2->dwHeight)
	{
		OutRect.bottom = lpDDSurfaceDesc2->dwHeight;
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

	DWORD BBP = surface.BitCount;
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
		size_t MemWidth = (EmuLock.BBP / 8) * EmuLock.Width;
		for (DWORD x = 0; x < EmuLock.Height; x++)
		{
			memcpy(OutAddr, InAddr, MemWidth);
			InAddr += InPitch;
			OutAddr += OutPitch;
		}

		// Mark as byte align locked
		WasBitAlignLocked = LockByteAlign;
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
		size_t MemWidth = (EmuLock.BBP / 8) * EmuLock.Width;
		for (DWORD x = 0; x < EmuLock.Height; x++)
		{
			memcpy(OutAddr, InAddr, MemWidth);
			InAddr += InPitch;
			OutAddr += OutPitch;
		}

		EmuLock.Locked = false;
		EmuLock.Addr = nullptr;
	}
}

// Restore removed scanlines before locking surface
void m_IDirectDrawSurfaceX::RestoreScanlines(LASTLOCK& LLock)
{
	DWORD ByteCount = surface.BitCount / 8;
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
	DWORD ByteCount = surface.BitCount / 8;
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
	pLockedRect->pBits = (lpDestRect) ? (void*)((DWORD)surface.emu->pBits + ((lpDestRect->top * pLockedRect->Pitch) + (lpDestRect->left * (surface.BitCount / 8)))) : surface.emu->pBits;

	return DD_OK;
}

void m_IDirectDrawSurfaceX::PrepareRenderTarget()
{
	if (surface.UsingShadowSurface && surface.Shadow)
	{
		if (SUCCEEDED((*d3d9Device)->UpdateSurface(surface.Shadow, nullptr, Get3DSurface(), nullptr)))
		{
			surface.UsingShadowSurface = false;
			return;
		}
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to update render target!");
	}
}

void m_IDirectDrawSurfaceX::SetRenderTargetShadow()
{
	if (!surface.UsingShadowSurface && surface.Shadow)
	{
		if (SUCCEEDED((*d3d9Device)->GetRenderTargetData(Get3DSurface(), surface.Shadow)))
		{
			surface.UsingShadowSurface = true;
			return;
		}
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get render target data!");
	}
}

// Set dirty flag
void m_IDirectDrawSurfaceX::SetDirtyFlag()
{
	if (IsPrimarySurface() && ddrawParent && !ddrawParent->IsInScene())
	{
		dirtyFlag = true;
	}
	surface.IsDirtyFlag = true;
	surface.HasData = true;
	surface.IsDrawTextureDirty = true;
	IsMipMapReadyToUse = false;

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

inline void m_IDirectDrawSurfaceX::EndWritePresent(LPRECT lpDestRect, bool WriteToWindow, bool FullPresent, bool IsSkipScene)
{
	// Blt surface directly to GDI
	if (ShouldWriteToGDI())
	{
		if (WriteToWindow)
		{
			CopyEmulatedSurfaceToGDI(lpDestRect);
		}
	}
	// Present surface to window
	else if (ShouldPresentToWindow(true))
	{
		if (WriteToWindow && ddrawParent && !ddrawParent->IsInScene())
		{
			RECT DestRect = {};
			if (CheckCoordinates(DestRect, lpDestRect, nullptr))
			{
				ddrawParent->PresentScene(&DestRect);
			}
		}
	}
	// Present surface after each draw unless removing interlacing
	else if (FullPresent && (PresentOnUnlock || !Config.DdrawRemoveInterlacing))
	{
		PresentSurface(IsSkipScene);

		// Reset endscene lock
		PresentOnUnlock = false;
	}
}

inline void m_IDirectDrawSurfaceX::EndWriteSyncSurfaces(LPRECT lpDestRect)
{
	// Copy emulated surface to real surface
	if (IsUsingEmulation())
	{
		CopyFromEmulatedSurface(lpDestRect);
	}
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

	// Handle mipmaps
	if ((!(surfaceDesc2.dwFlags & DDSD_MIPMAPCOUNT) || ((surfaceDesc2.dwFlags & DDSD_MIPMAPCOUNT) && surfaceDesc2.dwMipMapCount != 1)) &&
		(surfaceDesc2.ddsCaps.dwCaps & (DDSCAPS_MIPMAP | DDSCAPS_COMPLEX | DDSCAPS_TEXTURE)) == (DDSCAPS_MIPMAP | DDSCAPS_COMPLEX | DDSCAPS_TEXTURE))
	{
		// Compute width and height
		if ((!(surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT)) || (!surfaceDesc2.dwWidth && !surfaceDesc2.dwHeight)) &&
			(surfaceDesc2.dwFlags & DDSD_MIPMAPCOUNT) && surfaceDesc2.dwMipMapCount > 0)
		{
			surfaceDesc2.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT;
			surfaceDesc2.dwWidth = (DWORD)pow(2, surfaceDesc2.dwMipMapCount - 1);
			surfaceDesc2.dwHeight = surfaceDesc2.dwWidth;
		}
		// Compute mipcount
		DWORD MipMapLevelCount = ((surfaceDesc2.dwFlags & DDSD_MIPMAPCOUNT) && surfaceDesc2.dwMipMapCount) ? surfaceDesc2.dwMipMapCount :
			GetMaxMipMapLevel(surfaceDesc2.dwWidth, surfaceDesc2.dwHeight);
		MaxMipMapLevel = MipMapLevelCount;
		surfaceDesc2.dwMipMapCount = MaxMipMapLevel;
		surfaceDesc2.dwFlags |= DDSD_MIPMAPCOUNT;
	}
	// Mipmap textures
	else if (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_MIPMAP)
	{
		if (surfaceDesc2.dwFlags & DDSD_MIPMAPCOUNT)
		{
			surfaceDesc2.dwMipMapCount = 1;
		}
	}
	// No mipmaps
	else
	{
		if (surfaceDesc2.dwFlags & DDSD_MIPMAPCOUNT)
		{
			surfaceDesc2.dwMipMapCount = 0;
		}
		surfaceDesc2.dwFlags &= ~DDSD_MIPMAPCOUNT;
		surfaceDesc2.ddsCaps.dwCaps &= ~DDSCAPS_MIPMAP;
	}

	// Clear pitch
	if (!(surfaceDesc2.dwFlags & DDSD_LPSURFACE) && !(surfaceDesc2.dwFlags & DDSD_LINEARSIZE))
	{
		surfaceDesc2.dwFlags &= ~DDSD_PITCH;
		surfaceDesc2.lPitch = 0;
	}
	surface.UsingSurfaceMemory = ((surfaceDesc2.dwFlags & DDSD_LPSURFACE) && surfaceDesc2.lpSurface);

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

HRESULT m_IDirectDrawSurfaceX::ColorFill(RECT* pRect, D3DCOLOR dwFillColor, DWORD MipMapLevel)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Check for device interface
	HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
	if (FAILED(c_hr))
	{
		return c_hr;
	}

	// Get surface desc for mipmap
	DDSURFACEDESC2 Desc2 = {};
	Desc2.dwSize = sizeof(DDSURFACEDESC2);
	GetSurfaceDesc2(&Desc2, MipMapLevel, 7);

	// Check and copy rect
	RECT DestRect = {};
	if (!CheckCoordinates(DestRect, pRect, &Desc2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid rect: " << pRect);
		return DDERR_INVALIDRECT;
	}

	HRESULT hr = DDERR_GENERIC;

	// Use GPU ColorFill
	if (!IsUsingShadowSurface() && ((surface.Usage & D3DUSAGE_RENDERTARGET) || surface.Type == D3DTYPE_OFFPLAINSURFACE) && surface.Pool == D3DPOOL_DEFAULT)
	{
		IDirect3DSurface9* pDestSurfaceD9 = Get3DMipMapSurface(MipMapLevel);
		if (pDestSurfaceD9)
		{
			hr = (*d3d9Device)->ColorFill(pDestSurfaceD9, &DestRect, dwFillColor);

			if (FAILED(hr))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not color fill: " << (D3DERR)hr);
			}
		}
	}
	
	// Lock surface and manually fill with color
	if (FAILED(hr))
	{
		// Get width and height of rect
		LONG FillWidth = DestRect.right - DestRect.left;
		LONG FillHeight = DestRect.bottom - DestRect.top;

		// Check bit count
		if (surface.BitCount != 8 && surface.BitCount != 12 && surface.BitCount != 16 && surface.BitCount != 24 && surface.BitCount != 32)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid bit count: " << surface.BitCount << " Width: " << FillWidth);
			return DDERR_GENERIC;
		}

		// Check if surface is not locked then lock it
		D3DLOCKED_RECT DestLockRect = {};
		if (FAILED(IsUsingEmulation() ? LockEmulatedSurface(&DestLockRect, &DestRect) :
			LockD3d9Surface(&DestLockRect, &DestRect, 0, MipMapLevel)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock destination surface " << DestRect);
			return (IsSurfaceLocked()) ? DDERR_SURFACEBUSY : DDERR_GENERIC;
		}

		if (FillWidth == (LONG)surfaceDesc2.dwWidth && surface.BitCount == 8)
		{
			memset(DestLockRect.pBits, dwFillColor, DestLockRect.Pitch * FillHeight);
		}
		else if ((FillWidth * FillHeight < 640 * 480) && FillWidth == (LONG)surfaceDesc2.dwWidth && (surface.BitCount == 16 || surface.BitCount == 32) && (DestLockRect.Pitch * FillHeight) % 4 == 0)
		{
			const DWORD Color = (surface.BitCount == 16) ? ((dwFillColor & 0xFFFF) << 16) + (dwFillColor & 0xFFFF) : dwFillColor;
			const DWORD Size = (DestLockRect.Pitch * FillHeight) / 4;

			DWORD* DestBuffer = (DWORD*)DestLockRect.pBits;
			for (UINT x = 0; x < Size; x++)
			{
				DestBuffer[x] = Color;
			}
		}
		else if (surface.BitCount == 8 || (surface.BitCount == 12 && FillWidth % 2 == 0) || surface.BitCount == 16 || surface.BitCount == 24 || surface.BitCount == 32)
		{
			// Set memory address
			BYTE* SrcBuffer = (BYTE*)&dwFillColor;
			BYTE* DestBuffer = (BYTE*)DestLockRect.pBits;

			// Get byte count
			DWORD ByteCount = surface.BitCount / 8;

			// Handle 12-bit surface
			if (surface.BitCount == 12)
			{
				ByteCount = 3;
				dwFillColor = (dwFillColor & 0xFFF) + ((dwFillColor & 0xFFF) << 12);
				FillWidth /= 2;
			}

			// Fill first line memory
			if ((surface.BitCount == 8 || surface.BitCount == 16 || surface.BitCount == 32) && (FillWidth % (4 / ByteCount) == 0))
			{
				DWORD Color = (surface.BitCount == 8) ? (dwFillColor & 0xFF) + ((dwFillColor & 0xFF) << 8) + ((dwFillColor & 0xFF) << 16) + ((dwFillColor & 0xFF) << 24) :
					(surface.BitCount == 16) ? (dwFillColor & 0xFFFF) + ((dwFillColor & 0xFFFF) << 16) : dwFillColor;
				LONG Iterations = FillWidth / (4 / ByteCount);
				for (LONG x = 0; x < Iterations; x++)
				{
					*(DWORD*)DestBuffer = Color;
					DestBuffer += sizeof(DWORD);
				}
			}
			else
			{
				for (LONG x = 0; x < FillWidth; x++)
				{
					for (DWORD y = 0; y < ByteCount; y++)
					{
						*DestBuffer = SrcBuffer[y];
						DestBuffer++;
					}
				}
			}

			// Fill rest of surface rect using the first line as a template
			SrcBuffer = (BYTE*)DestLockRect.pBits;
			DestBuffer = (BYTE*)DestLockRect.pBits + DestLockRect.Pitch;
			size_t Size = FillWidth * ByteCount;
			for (LONG y = 1; y < FillHeight; y++)
			{
				memcpy(DestBuffer, SrcBuffer, Size);
				DestBuffer += DestLockRect.Pitch;
			}
		}
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid bit count: " << surface.BitCount << " Width: " << FillWidth);
			return DDERR_GENERIC;
		}

		// Unlock surface
		if (!IsUsingEmulation())
		{
			UnLockD3d9Surface(MipMapLevel);
		}
	}

	// Keep surface insync
	EndWriteSyncSurfaces(&DestRect);

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
	HRESULT hr = D3DXSaveSurfaceToFileInMemory(&pDestBuf, format, Get3DSurface(), nullptr, nullptr);

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

// Simple copy with ColorKey and Mirroring
template <typename T>
void SimpleColorKeyCopy(T ColorKey, BYTE* SrcBuffer, BYTE* DestBuffer, INT SrcPitch, INT DestPitch, LONG DestRectWidth, LONG DestRectHeight, bool IsColorKey, bool IsMirrorLeftRight)
{
	T* SrcBufferLoop = reinterpret_cast<T*>(SrcBuffer);
	T* DestBufferLoop = reinterpret_cast<T*>(DestBuffer);
	for (LONG y = 0; y < DestRectHeight; y++)
	{
		for (LONG x = 0; x < DestRectWidth; x++)
		{
			T PixelColor = SrcBufferLoop[IsMirrorLeftRight ? DestRectWidth - x - 1 : x];
			if (!IsColorKey || PixelColor != ColorKey)
			{
				DestBufferLoop[x] = PixelColor;
			}
		}
		SrcBufferLoop = reinterpret_cast<T*>((BYTE*)SrcBufferLoop + SrcPitch);
		DestBufferLoop = reinterpret_cast<T*>((BYTE*)DestBufferLoop + DestPitch);
	}
}

// Copy memory (complex)
template <typename T>
void ComplexCopy(T ColorKey, D3DLOCKED_RECT SrcLockRect, D3DLOCKED_RECT DestLockRect, LONG SrcRectWidth, LONG SrcRectHeight, LONG DestRectWidth, LONG DestRectHeight, bool IsColorKey, bool IsMirrorUpDown, bool IsMirrorLeftRight)
{
	float WidthRatio = ((float)SrcRectWidth / (float)DestRectWidth);
	float HeightRatio = ((float)SrcRectHeight / (float)DestRectHeight);

	T* SrcBufferLoop = reinterpret_cast<T*>(SrcLockRect.pBits);
	T* DestBufferLoop = reinterpret_cast<T*>(DestLockRect.pBits);

	DWORD sx;

	for (LONG y = 0; y < DestRectHeight; y++)
	{
		for (LONG x = 0; x < DestRectWidth; x++)
		{
			sx = (DWORD)((float)x * WidthRatio + 0.5f);
			T PixelColor = SrcBufferLoop[IsMirrorLeftRight ? SrcRectWidth - sx - 1 : sx];

			if (!IsColorKey || PixelColor != ColorKey)
			{
				DestBufferLoop[x] = PixelColor;
			}
		}
		sx = min((DWORD)((float)(y + 1) * HeightRatio + 0.5f), (DWORD)SrcRectHeight - 1);
		SrcBufferLoop = reinterpret_cast<T*>((BYTE*)SrcLockRect.pBits + SrcLockRect.Pitch * (IsMirrorUpDown ? SrcRectHeight - sx - 1 : sx));
		DestBufferLoop = reinterpret_cast<T*>((BYTE*)DestBufferLoop + DestLockRect.Pitch);
	}
}

// Copy surface
HRESULT m_IDirectDrawSurfaceX::CopySurface(m_IDirectDrawSurfaceX* pSourceSurface, RECT* pSourceRect, RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter, D3DCOLOR ColorKey, DWORD dwFlags, DWORD SrcMipMapLevel, DWORD MipMapLevel)
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

	// Get surface desc for mipmap
	DDSURFACEDESC2 SrcDesc2 = {}, DestDesc2 = {};
	SrcDesc2.dwSize = sizeof(DDSURFACEDESC2);
	DestDesc2.dwSize = sizeof(DDSURFACEDESC2);
	pSourceSurface->GetSurfaceDesc2(&SrcDesc2, SrcMipMapLevel, 7);
	GetSurfaceDesc2(&DestDesc2, MipMapLevel, 7);

	// Copy rect and do clipping
	RECT SrcRect = (pSourceRect) ? *pSourceRect : RECT{ 0, 0, (LONG)SrcDesc2.dwWidth, (LONG)SrcDesc2.dwHeight };
	RECT DestRect = (pDestRect) ? *pDestRect : RECT{ 0, 0, (LONG)DestDesc2.dwWidth, (LONG)DestDesc2.dwHeight };
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

	// Check source and destination format
	const bool FormatMismatch = !(SrcFormat == DestFormat || (ISDXTEX(SrcFormat) && ISDXTEX(DestFormat)) ||
		((SrcFormat == D3DFMT_A1R5G5B5 || SrcFormat == D3DFMT_X1R5G5B5) && (DestFormat == D3DFMT_A1R5G5B5 || DestFormat == D3DFMT_X1R5G5B5)) ||
		((SrcFormat == D3DFMT_A4R4G4B4 || SrcFormat == D3DFMT_X4R4G4B4) && (DestFormat == D3DFMT_A4R4G4B4 || DestFormat == D3DFMT_X4R4G4B4)) ||
		((SrcFormat == D3DFMT_A8R8G8B8 || SrcFormat == D3DFMT_X8R8G8B8) && (DestFormat == D3DFMT_A8R8G8B8 || DestFormat == D3DFMT_X8R8G8B8)) ||
		((SrcFormat == D3DFMT_A8B8G8R8 || SrcFormat == D3DFMT_X8B8G8R8) && (DestFormat == D3DFMT_A8B8G8R8 || DestFormat == D3DFMT_X8B8G8R8)));

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

#ifdef ENABLE_PROFILING
	Logging::Log() << __FUNCTION__ << " (" << pSourceSurface << ") -> (" << this << ")" <<
		" StretchRect = " << IsStretchRect <<
		" ColorKey = " << IsColorKey <<
		" MirrorLeftRight = " << IsMirrorLeftRight <<
		" MirrorUpDown = " << IsMirrorUpDown;
#endif

	// Check rect and do clipping
	if (!pSourceSurface->CheckCoordinates(SrcRect, &SrcRect, &SrcDesc2) || !CheckCoordinates(DestRect, &DestRect, &DestDesc2))
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
		Filter = D3DTEXF_NONE;
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
	if (ShouldReadFromGDI())
	{
		CopyEmulatedSurfaceFromGDI(&DestRect);
	}

	// Variables
	HRESULT hr = DDERR_GENERIC;
	bool UnlockSrc = false, UnlockDest = false;
	D3DLOCKED_RECT DestLockRect = {};

	do {
		// Use StretchRect for video memory to prevent copying out of video memory
		if (!IsUsingEmulation() && !IsUsingShadowSurface() && !pSourceSurface->IsUsingShadowSurface() &&
			(pSourceSurface->surface.Pool == D3DPOOL_DEFAULT && surface.Pool == D3DPOOL_DEFAULT) &&
			(pSourceSurface->surface.Type == surface.Type || (pSourceSurface->surface.Type == D3DTYPE_OFFPLAINSURFACE && surface.Type == D3DTYPE_RENDERTARGET)) &&
			(!IsStretchRect || (this != pSourceSurface && !ISDXTEX(SrcFormat) && !ISDXTEX(DestFormat) && surface.Type == D3DTYPE_RENDERTARGET)) &&
			(surface.Type != D3DTYPE_DEPTHSTENCIL || !ddrawParent->IsInScene()) &&
			(surface.Type != D3DTYPE_TEXTURE) &&
			(!pSourceSurface->IsPalette() && !IsPalette()) &&
			!IsMirrorLeftRight && !IsMirrorUpDown && !IsColorKey)
		{
			IDirect3DSurface9* pSourceSurfaceD9 = pSourceSurface->Get3DMipMapSurface(SrcMipMapLevel);
			IDirect3DSurface9* pDestSurfaceD9 = Get3DMipMapSurface(MipMapLevel);

			if (pSourceSurfaceD9 && pDestSurfaceD9)
			{
				hr = (*d3d9Device)->StretchRect(pSourceSurfaceD9, &SrcRect, pDestSurfaceD9, &DestRect, Filter);

				if (FAILED(hr))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not copy rect: " << SrcDesc2.ddsCaps << " -> " << DestDesc2.ddsCaps << " " <<
						SrcFormat << " -> " << DestFormat << " " << SrcRect << " -> " << DestRect << " " << IsStretchRect << " " << (D3DERR)hr);
				}
			}

			pSourceSurface->Release3DMipMapSurface(pSourceSurfaceD9, SrcMipMapLevel);
			Release3DMipMapSurface(pDestSurfaceD9, MipMapLevel);

			if (SUCCEEDED(hr))
			{
				break;
			}
		}

		// Use UpdateSurface for copying system memory to video memory
		if (!IsUsingEmulation() && !IsUsingShadowSurface() && surface.Pool == D3DPOOL_DEFAULT &&
			(pSourceSurface->surface.Pool == D3DPOOL_SYSTEMMEM || pSourceSurface->IsUsingShadowSurface() ||
				(pSourceSurface->surface.Pool == D3DPOOL_MANAGED && surface.Shadow && (surface.BitCount == 8 || surface.BitCount == 16 || surface.BitCount == 24 || surface.BitCount == 32))) &&
			(pSourceSurface->surface.Type != D3DTYPE_DEPTHSTENCIL && surface.Type != D3DTYPE_DEPTHSTENCIL) &&
			(pSourceSurface->surface.Format == surface.Format) &&
			(!pSourceSurface->IsPalette() && !IsPalette()) &&
			!IsStretchRect && !IsMirrorLeftRight && !IsMirrorUpDown && !IsColorKey)
		{
			IDirect3DSurface9* pSourceSurfaceD9 = pSourceSurface->Get3DMipMapSurface(SrcMipMapLevel);
			IDirect3DSurface9* pDestSurfaceD9 = Get3DMipMapSurface(MipMapLevel);

			if (pSourceSurfaceD9 && pDestSurfaceD9)
			{
				if (pSourceSurface->surface.Pool == D3DPOOL_SYSTEMMEM || pSourceSurface->IsUsingShadowSurface())
				{
					hr = (*d3d9Device)->UpdateSurface(pSourceSurfaceD9, &SrcRect, pDestSurfaceD9, (LPPOINT)&DestRect);
				}
				else
				{
					do {
						D3DLOCKED_RECT SrcLockedRect = {};
						if (FAILED(pSourceSurfaceD9->LockRect(&SrcLockedRect, &SrcRect, D3DLOCK_READONLY)))
						{
							LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock source surface for update!");
							break;
						}
						D3DLOCKED_RECT DestLockedRect = {};
						if (FAILED(surface.Shadow->LockRect(&DestLockedRect, &DestRect, 0)))
						{
							LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock shadow surface for update!");
							pSourceSurfaceD9->UnlockRect();
							break;
						}

						BYTE* SrcBytes = (BYTE*)SrcLockedRect.pBits;
						BYTE* DestBytes = (BYTE*)DestLockedRect.pBits;
						size_t Size = DestRectWidth * surface.BitCount / 8;
						for (int x = 0; x < DestRectHeight ; x++)
						{
							memcpy(DestBytes, SrcBytes, Size);
							SrcBytes += SrcLockedRect.Pitch;
							DestBytes += DestLockedRect.Pitch;
						}

						surface.Shadow->UnlockRect();
						pSourceSurfaceD9->UnlockRect();

						hr = (*d3d9Device)->UpdateSurface(surface.Shadow, &DestRect, pDestSurfaceD9, (LPPOINT)&DestRect);

					} while (false);
				}

				if (FAILED(hr))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not update surface: " << SrcDesc2.ddsCaps << " -> " << DestDesc2.ddsCaps << " " <<
						SrcFormat << " -> " << DestFormat << " " << SrcRect << " -> " << DestRect << " " << IsStretchRect << " " << (D3DERR)hr);
				}
			}

			pSourceSurface->Release3DMipMapSurface(pSourceSurfaceD9, SrcMipMapLevel);
			Release3DMipMapSurface(pDestSurfaceD9, MipMapLevel);

			if (SUCCEEDED(hr))
			{
				break;
			}
		}

		// Check if render target should use shadow
		if (surface.Type == D3DTYPE_RENDERTARGET && !IsUsingShadowSurface())
		{
			SetRenderTargetShadow();
		}

		// Decode DirectX textures and FourCCs
		if ((FormatMismatch && !IsUsingEmulation() && !IsColorKey && !IsMirrorLeftRight && !IsMirrorUpDown) ||
			(SrcFormat & 0xFF000000 /*FOURCC or D3DFMT_DXTx*/) ||
			SrcFormat == D3DFMT_V8U8 ||
			SrcFormat == D3DFMT_L6V5U5 ||
			SrcFormat == D3DFMT_X8L8V8U8 ||
			SrcFormat == D3DFMT_Q8W8V8U8 ||
			SrcFormat == D3DFMT_V16U16 ||
			SrcFormat == D3DFMT_A2W10V10U10)
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

			IDirect3DSurface9* pSourceSurfaceD9 = pSourceSurface->Get3DMipMapSurface(SrcMipMapLevel);
			IDirect3DSurface9* pDestSurfaceD9 = Get3DMipMapSurface(MipMapLevel);

			if (pSourceSurfaceD9 && pDestSurfaceD9)
			{
				hr = D3DXLoadSurfaceFromSurface(pDestSurfaceD9, nullptr, &DestRect, pSourceSurfaceD9, nullptr, &SrcRect, D3DXFilter, 0);

				if (FAILED(hr))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not decode source texture. " << (D3DERR)hr);
					break;
				}
			}

			pSourceSurface->Release3DMipMapSurface(pSourceSurfaceD9, SrcMipMapLevel);
			Release3DMipMapSurface(pDestSurfaceD9, MipMapLevel);

			if (SUCCEEDED(hr))
			{
				break;
			}

			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get source or destination surface level: " << pSourceSurfaceD9 << "->" << pDestSurfaceD9);
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
				hr = DD_OK;
				break;
			}
		}

		// Use D3DXLoadSurfaceFromSurface to copy the surface
		if (!IsUsingEmulation() && !IsColorKey && !IsMirrorLeftRight && !IsMirrorUpDown &&
			pSourceSurface->surface.Type == surface.Type &&	// D3DXLoadSurfaceFromSurface is very slow when copying from offplain to texture
			!surface.UsingSurfaceMemory && !pSourceSurface->surface.UsingSurfaceMemory &&
			(pSourceSurface->IsPalette() == IsPalette()))
		{
			IDirect3DSurface9* pSourceSurfaceD9 = pSourceSurface->Get3DMipMapSurface(SrcMipMapLevel);
			IDirect3DSurface9* pDestSurfaceD9 = Get3DMipMapSurface(MipMapLevel);

			if (pSourceSurfaceD9 && pDestSurfaceD9)
			{
				hr = D3DXLoadSurfaceFromSurface(pDestSurfaceD9, nullptr, &DestRect, pSourceSurfaceD9, nullptr, &SrcRect, D3DXFilter, 0);

				if (FAILED(hr))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to load surface from surface. " << (D3DERR)hr);
					break;
				}
			}

			pSourceSurface->Release3DMipMapSurface(pSourceSurfaceD9, SrcMipMapLevel);
			Release3DMipMapSurface(pDestSurfaceD9, MipMapLevel);

			if (SUCCEEDED(hr))
			{
				break;
			}
		}

		// Check for format mismatch
		const bool FormatR5G6B5toX8R8G8B8 = (SrcFormat == D3DFMT_R5G6B5 && (DestFormat == D3DFMT_A8R8G8B8 || DestFormat == D3DFMT_X8R8G8B8));
		if (FormatMismatch)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: source and destination formats don't match! " << SrcFormat << "-->" << DestFormat);

			if (!FormatR5G6B5toX8R8G8B8)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: not supported for specified source and destination formats! " << SrcFormat << "-->" << DestFormat);
				hr = DDERR_GENERIC;
				break;
			}
		}

		// Get byte count
		DWORD DestBitCount = surface.BitCount;
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
			pSourceSurface->LockD3d9Surface(&SrcLockRect, &SrcRect, D3DLOCK_READONLY, SrcMipMapLevel)) || !SrcLockRect.pBits)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock source surface " << SrcRect);
			hr = (pSourceSurface->IsSurfaceBusy()) ? DDERR_SURFACEBUSY : DDERR_GENERIC;
			break;
		}
		UnlockSrc = true;

		// Use seperate memory cache if source and destination formats mismatch or are on the same surface
		if ((pSourceSurface == this && MipMapLevel == SrcMipMapLevel) || FormatMismatch)
		{
			size_t size = SrcRectWidth * ByteCount * SrcRectHeight;
			if (size > ByteArray.size())
			{
				ByteArray.resize(size);
			}
			BYTE* SrcBuffer = (BYTE*)SrcLockRect.pBits;
			BYTE* DestBuffer = (BYTE*)ByteArray.data();
			INT DestPitch = SrcRectWidth * ByteCount;
			if (FormatR5G6B5toX8R8G8B8)
			{
				for (LONG y = 0; y < SrcRectHeight; y++)
				{
					for (LONG x = 0; x < SrcRectWidth; x++)
					{
						((DWORD*)DestBuffer)[x] = D3DFMT_R5G6B5_TO_X8R8G8B8(((WORD*)SrcBuffer)[x]);
					}
					SrcBuffer += SrcLockRect.Pitch;
					DestBuffer += DestPitch;
				}
				ColorKey = D3DFMT_R5G6B5_TO_X8R8G8B8(ColorKey);
			}
			else
			{
				for (LONG y = 0; y < SrcRectHeight; y++)
				{
					memcpy(DestBuffer, SrcBuffer, SrcRectWidth * ByteCount);
					SrcBuffer += SrcLockRect.Pitch;
					DestBuffer += DestPitch;
				}
			}
			SrcLockRect.pBits = ByteArray.data();
			SrcLockRect.Pitch = DestPitch;
			if (UnlockSrc)
			{
				pSourceSurface->IsUsingEmulation() ? DD_OK : pSourceSurface->UnLockD3d9Surface(SrcMipMapLevel);
				UnlockSrc = false;
			}
		}

		// Check if destination surface is not locked then lock it
		if (FAILED(IsUsingEmulation() ? LockEmulatedSurface(&DestLockRect, &DestRect) :
			LockD3d9Surface(&DestLockRect, &DestRect, 0, MipMapLevel)) || !DestLockRect.pBits)
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
		if (!IsStretchRect && !IsColorKey && !IsMirrorLeftRight)
		{
			if (!IsMirrorUpDown && SrcLockRect.Pitch == DestLockRect.Pitch && (DWORD)DestRectWidth == DestDesc2.dwWidth)
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
			hr = DD_OK;
			break;
		}

		// Set color variables
		DWORD ByteMask = (ByteCount == 1) ? 0x000000FF : (ByteCount == 2) ? 0x0000FFFF : (ByteCount == 3) ? 0x00FFFFFF : 0xFFFFFFFF;
		DWORD dColorKey = (ColorKey & ByteMask);

		// Simple copy with ColorKey and Mirroring
		if (!IsStretchRect)
		{
			switch (ByteCount)
			{
			case 1:
				SimpleColorKeyCopy<BYTE>((BYTE)(ColorKey & ByteMask), SrcBuffer, DestBuffer, SrcLockRect.Pitch, DestPitch, DestRectWidth, DestRectHeight, IsColorKey, IsMirrorLeftRight);
				break;
			case 2:
				SimpleColorKeyCopy<WORD>((WORD)(ColorKey & ByteMask), SrcBuffer, DestBuffer, SrcLockRect.Pitch, DestPitch, DestRectWidth, DestRectHeight, IsColorKey, IsMirrorLeftRight);
				break;
			case 3:
				SimpleColorKeyCopy<TRIBYTE>(*reinterpret_cast<TRIBYTE*>(&dColorKey), SrcBuffer, DestBuffer, SrcLockRect.Pitch, DestPitch, DestRectWidth, DestRectHeight, IsColorKey, IsMirrorLeftRight);
				break;
			case 4:
				SimpleColorKeyCopy<DWORD>((DWORD)(ColorKey & ByteMask), SrcBuffer, DestBuffer, SrcLockRect.Pitch, DestPitch, DestRectWidth, DestRectHeight, IsColorKey, IsMirrorLeftRight);
				break;
			}
			hr = DD_OK;
			break;
		}

		// Copy memory (complex)
		switch (ByteCount)
		{
		case 1:
			ComplexCopy<BYTE>((BYTE)ColorKey, SrcLockRect, DestLockRect, SrcRectWidth, SrcRectHeight, DestRectWidth, DestRectHeight, IsColorKey, IsMirrorUpDown, IsMirrorLeftRight);
			break;
		case 2:
			ComplexCopy<WORD>((WORD)ColorKey, SrcLockRect, DestLockRect, SrcRectWidth, SrcRectHeight, DestRectWidth, DestRectHeight, IsColorKey, IsMirrorUpDown, IsMirrorLeftRight);
			break;
		case 3:
			ComplexCopy<TRIBYTE>(*reinterpret_cast<TRIBYTE*>(&dColorKey), SrcLockRect, DestLockRect, SrcRectWidth, SrcRectHeight, DestRectWidth, DestRectHeight, IsColorKey, IsMirrorUpDown, IsMirrorLeftRight);
			break;
		case 4:
			ComplexCopy<DWORD>((DWORD)ColorKey, SrcLockRect, DestLockRect, SrcRectWidth, SrcRectHeight, DestRectWidth, DestRectHeight, IsColorKey, IsMirrorUpDown, IsMirrorLeftRight);
		}
		hr = DD_OK;
		break;

	} while (false);

	// Unlock surfaces if needed
	if (UnlockSrc)
	{
		pSourceSurface->IsUsingEmulation() ? DD_OK : pSourceSurface->UnLockD3d9Surface(SrcMipMapLevel);
	}
	if (UnlockDest)
	{
		// Remove scanlines before unlocking surface
		if (SUCCEEDED(hr) && Config.DdrawRemoveScanlines && IsPrimaryOrBackBuffer())
		{
			// Set last rect before removing scanlines
			LASTLOCK LLock;
			LLock.ScanlineWidth = DestRectWidth;
			LLock.Rect = DestRect;
			if (IsUsingEmulation())
			{
				LockEmulatedSurface(&LLock.LockedRect, &DestRect);
			}
			else
			{
				LLock.LockedRect = DestLockRect;
			}

			RemoveScanlines(LLock);
		}

		IsUsingEmulation() ? DD_OK : UnLockD3d9Surface(MipMapLevel);
	}

	if (SUCCEEDED(hr))
	{
		// Keep surface insync
		EndWriteSyncSurfaces(&DestRect);
	}

	// Return
	return hr;
}

HRESULT m_IDirectDrawSurfaceX::CopyToDrawTexture(LPRECT lpDestRect)
{
	if (!surface.DrawTexture || !surface.Texture)
	{
		return DDERR_GENERIC;
	}

	IDirect3DSurface9* SrcSurface = Get3DMipMapSurface(0);
	IDirect3DSurface9* DestSurface = nullptr;
	if (!SrcSurface || FAILED(surface.DrawTexture->GetSurfaceLevel(0, &DestSurface)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get surface texture!");
		return DDERR_GENERIC;
	}

	DWORD ColorKey = (surfaceDesc2.ddpfPixelFormat.dwRGBBitCount && (surfaceDesc2.dwFlags & DDSD_CKSRCBLT)) ?
		GetARGBColorKey(surfaceDesc2.ddckCKSrcBlt.dwColorSpaceLowValue, surfaceDesc2.ddpfPixelFormat) : 0;

	if (IsPalette())
	{
		UpdatePaletteData();
		if ((surfaceDesc2.dwFlags & DDSD_CKSRCBLT) && surface.PaletteEntryArray)
		{
			PALETTEENTRY PaletteEntry = surface.PaletteEntryArray[surfaceDesc2.ddckCKSrcBlt.dwColorSpaceLowValue & 0xFF];
			ColorKey = D3DCOLOR_ARGB(PaletteEntry.peFlags, PaletteEntry.peRed, PaletteEntry.peGreen, PaletteEntry.peBlue);
		}
	}

	if (FAILED(D3DXLoadSurfaceFromSurface(DestSurface, nullptr, lpDestRect, SrcSurface, surface.PaletteEntryArray, lpDestRect, D3DX_FILTER_NONE, ColorKey)))
	{
		Logging::Log() << __FUNCTION__ " Error: failed to copy data from surface: " << surface.Format << " " << (void*)ColorKey << " " << lpDestRect;

		DestSurface->Release();

		return DDERR_GENERIC;
	}

	surface.IsDrawTextureDirty = false;

	DestSurface->Release();

	return DD_OK;
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
	if (!CheckCoordinates(DestRect, lpDestRect, nullptr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << lpDestRect);
		return DDERR_INVALIDRECT;
	}

	// Get real d3d9 surface
	IDirect3DSurface9* pDestSurfaceD9 = Get3DMipMapSurface(0);
	if (!pDestSurfaceD9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not get real surface!");
		return DDERR_GENERIC;
	}

	// Use D3DXLoadSurfaceFromMemory to copy to the surface
	if (FAILED(D3DXLoadSurfaceFromMemory(pDestSurfaceD9, nullptr, &DestRect, surface.emu->pBits, (surface.Format == D3DFMT_P8) ? D3DFMT_L8 : surface.Format, surface.emu->Pitch, nullptr, &DestRect, D3DX_FILTER_NONE, 0)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not copy emulated surface: " << surface.Format);
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
	if (!CheckCoordinates(DestRect, lpDestRect, nullptr))
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
	if (FAILED(LockD3d9Surface(&SrcLockRect, &DestRect, D3DLOCK_READONLY, 0)))
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
	switch ((DWORD)surface.Format)
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
		else if (surface.emu->bmi->bmiHeader.biBitCount == surface.BitCount)
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
			LOG_LIMIT(100, __FUNCTION__ << " Error: emulated surface format not supported: " << surface.Format);
		}
	}

	// Unlock surface
	UnLockD3d9Surface(0);

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
			const DWORD Width = surface.Width;
			const DWORD Height = surfaceDesc2.dwHeight;
			LOG_LIMIT(3, __FUNCTION__ << " Creating palette display surface texture. Size: " << Width << "x" << Height << " dwCaps: " << surfaceDesc2.ddsCaps);
			if (FAILED(((*d3d9Device)->CreateTexture(Width, Height, 1, 0, D3DFMT_X8R8G8B8, TexturePool, &surface.DisplayTexture, nullptr))))
			{
				// Try failover format
				if (FAILED(((*d3d9Device)->CreateTexture(Width, Height, 1, 0, GetFailoverFormat(D3DFMT_X8R8G8B8), TexturePool, &surface.DisplayTexture, nullptr))))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create palette display surface texture. Size: " << Width << "x" << Height << " Format: " << D3DFMT_X8R8G8B8 << " dwCaps: " << surfaceDesc2.ddsCaps);
					hr = DDERR_GENERIC;
					break;
				}
			}
		}

		// Update rect, if palette surface is dirty then update the whole surface
		RECT DestRect = {};
		if (!CheckCoordinates(DestRect, (surface.IsPaletteDirty ? nullptr : lpDestRect), nullptr))
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
			LOG_LIMIT(100, __FUNCTION__ << " Warning: could not copy palette display texture: " << surface.Format);
			hr = DDERR_GENERIC;
			break;
		}

		// Reset palette texture dirty flag
		surface.IsPaletteDirty = false;

	} while (false);

	ReleaseCriticalSection();

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::CopyEmulatedSurfaceFromGDI(LPRECT lpDestRect)
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

	// Update rect
	RECT DestRect = {};
	if (!CheckCoordinates(DestRect, lpDestRect, nullptr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << lpDestRect);
		return DDERR_INVALIDRECT;
	}

	// Clip rect
	RECT ClientRect = {};
	if (GetClientRect(DDraw_hWnd, &ClientRect) && MapWindowPoints(DDraw_hWnd, HWND_DESKTOP, (LPPOINT)&ClientRect, 2))
	{
		DestRect.left = max(DestRect.left, ClientRect.left);
		DestRect.top = max(DestRect.top, ClientRect.top);
		DestRect.right = min(DestRect.right, ClientRect.right);
		DestRect.bottom = min(DestRect.bottom, ClientRect.bottom);
	}

	// Validate rect
	if (DestRect.left >= DestRect.right || DestRect.top >= DestRect.bottom)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid DestRect: " << DestRect);
		return DDERR_GENERIC;
	}

	// Get rect size
	RECT MapRect = DestRect;
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
	BitBlt(surface.emu->DC, MapRect.left, MapRect.top, MapRect.right - MapRect.left, MapRect.bottom - MapRect.top, hdc, DestRect.left, DestRect.top, SRCCOPY);

	// Release DC
	if (UsingForgroundWindow)
	{
		::ReleaseDC(hWnd, hdc);
	}

	return DD_OK;
}

HRESULT m_IDirectDrawSurfaceX::CopyEmulatedSurfaceToGDI(LPRECT lpDestRect)
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

	// Update rect
	RECT DestRect = {};
	if (!CheckCoordinates(DestRect, lpDestRect, nullptr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << lpDestRect);
		return DDERR_INVALIDRECT;
	}

	// Clip rect
	RECT ClientRect = {};
	if (GetClientRect(DDraw_hWnd, &ClientRect) && MapWindowPoints(DDraw_hWnd, HWND_DESKTOP, (LPPOINT)&ClientRect, 2))
	{
		DestRect.left = max(DestRect.left, ClientRect.left);
		DestRect.top = max(DestRect.top, ClientRect.top);
		DestRect.right = min(DestRect.right, ClientRect.right);
		DestRect.bottom = min(DestRect.bottom, ClientRect.bottom);
	}

	// Validate rect
	if (DestRect.left >= DestRect.right || DestRect.top >= DestRect.bottom)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << DestRect);
		return DDERR_GENERIC;
	}

	// Get rect size
	RECT MapRect = DestRect;
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
	BitBlt(hdc, MapRect.left, MapRect.top, MapRect.right - MapRect.left, MapRect.bottom - MapRect.top, surface.emu->DC, DestRect.left, DestRect.top, SRCCOPY);

	// Release DC
	if (UsingForgroundWindow)
	{
		::ReleaseDC(hWnd, hdc);
	}

	return DD_OK;
}

HRESULT m_IDirectDrawSurfaceX::GetPresentWindowRect(LPRECT pRect, RECT& DestRect)
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
	RECT Rect = pRect ? *pRect : RECT{ 0, 0, (LONG)surfaceDesc2.dwWidth, (LONG)surfaceDesc2.dwHeight };
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
	IDirect3DSurface9* pSourceSurfaceD9 = Get3DMipMapSurface(0);
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
	HRESULT hr = DDERR_GENERIC;
	if (IsD9UsingVideoMemory())
	{
		hr = (*d3d9Device)->StretchRect(pSourceSurfaceD9, &Rect, pDestSurfaceD9, &MapClient, D3DTEXF_NONE);
	}
	else
	{
		hr = (*d3d9Device)->UpdateSurface(pSourceSurfaceD9, &Rect, pDestSurfaceD9, (LPPOINT)&MapClient);
	}
	pDestSurfaceD9->Release();
	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to copy surface: " << Rect << " -> " << MapClient);
		return DDERR_GENERIC;
	}

	DestRect = MapClient;

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
