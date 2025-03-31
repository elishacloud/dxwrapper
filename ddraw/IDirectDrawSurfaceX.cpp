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
*
* Code initial taken from: https://github.com/strangebytes/diablo-ddrawwrapper
*
* Code to create emulated surface taken from: https://github.com/CnCNet/cnc-ddraw
*/

#include "winmm.h"
#include "ddraw.h"
#include <sstream>
#include "Utils\Utils.h"

namespace {
	constexpr DWORD ExtraDataBufferSize = 200;
	constexpr DWORD SurfaceWaitTimeoutMS = 4;

	// Used for dummy mipmaps
	std::vector<BYTE> dummySurface;

	// Used to allow presenting non-primary surfaces in case the primary surface present fails
	bool dirtyFlag = false;
	bool SceneReady = false;
	bool IsPresentRunning = false;

	// Used for sharing emulated memory
	bool ShareEmulatedMemory = false;
	std::vector<EMUSURFACE*> memorySurfaces;

	m_IDirectDrawSurface* WrapperInterfaceBackup = nullptr;
	m_IDirectDrawSurface2* WrapperInterfaceBackup2 = nullptr;
	m_IDirectDrawSurface3* WrapperInterfaceBackup3 = nullptr;
	m_IDirectDrawSurface4* WrapperInterfaceBackup4 = nullptr;
	m_IDirectDrawSurface7* WrapperInterfaceBackup7 = nullptr;
}

// ******************************
// IUnknown functions
// ******************************

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

	DWORD DxVersion = (CheckWrapperType(riid) && Config.Dd7to9) ? GetGUIDVersion(riid) : DirectXVersion;

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
			// Check for Direct3D surface
			if (!IsSurface3D())
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not a Direct3D surface!");
				return E_NOINTERFACE;
			}

			m_IDirect3DDeviceX* D3DDeviceX = *ddrawParent->GetCurrentD3DDevice();

			if (D3DDeviceX)
			{
				if (!attached3DDevice)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: Direct3DDevice is already setup. Multiple Direct3DDevice's are not implemented!");
					return DDERR_GENERIC;
				}
				else if (attached3DDevice != D3DDeviceX)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: multiple Direct3DDevice's created!");
					return DDERR_GENERIC;
				}
			}

			m_IDirect3DX* D3DX = *ddrawParent->GetCurrentD3D();

			if (!D3DX)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: Direct3D not setup when creating Direct3DDevice.");
			}

			DxVersion = (DxVersion == 4) ? 3 : DxVersion;

			if (!attached3DDevice)
			{
				attached3DDevice = new m_IDirect3DDeviceX(ddrawParent, D3DX, (LPDIRECTDRAWSURFACE7)GetWrapperInterfaceX(DirectXVersion), riid, DirectXVersion);

				attached3DDevice->SetParent3DSurface(this, DirectXVersion);
			}
			else
			{
				attached3DDevice->AddRef(DxVersion);	// No need to add a ref when creating a device because it is already added when creating the device
			}

			*ppvObj = (LPDIRECT3DDEVICE7)attached3DDevice->GetWrapperInterfaceX(DxVersion);

			return D3D_OK;
		}
		// ColorControl doesn't work on native ddraw
		/*if (riid == IID_IDirectDrawColorControl)
		{
			m_IDirectDrawColorControl* lpColorControl = ddrawParent->GetColorControlInterface();

			if (lpColorControl)
			{
				*ppvObj = lpColorControl;

				lpColorControl->AddRef();
			}
			else
			{
				if (FAILED(ddrawParent->CreateColorControl(reinterpret_cast<m_IDirectDrawColorControl**>(ppvObj))))
				{
					return E_NOINTERFACE;
				}
			}

			return DD_OK;
		}*/
		if (riid == IID_IDirectDrawGammaControl)
		{
			m_IDirectDrawGammaControl* lpGammaControl = ddrawParent->GetGammaControlInterface();

			if (lpGammaControl)
			{
				*ppvObj = lpGammaControl;

				lpGammaControl->AddRef();
			}
			else
			{
				if (FAILED(ddrawParent->CreateGammaControl(reinterpret_cast<m_IDirectDrawGammaControl**>(ppvObj))))
				{
					return E_NOINTERFACE;
				}
			}

			return DD_OK;
		}
		if (riid == IID_IDirect3DTexture || riid == IID_IDirect3DTexture2)
		{
			if (ddrawParent->IsCreatedEx())
			{
				return E_NOINTERFACE;
			}

			DxVersion = GetGUIDVersion(riid);

			m_IDirect3DTextureX* InterfaceX = nullptr;

			if (!attached3DTexture)
			{
				attached3DTexture = new m_IDirect3DTextureX(ddrawParent->GetCurrentD3DDevice(), DxVersion, this, DirectXVersion);
			}
			else
			{
				attached3DTexture->AddRef(DxVersion);	// No need to add a ref when creating a texture because it is already added when creating the texture
			}

			InterfaceX = attached3DTexture;

			*ppvObj = InterfaceX->GetWrapperInterfaceX(DxVersion);

			return DD_OK;
		}
	}

	HRESULT hr = ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DirectXVersion));

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
						ddrawParent->AddReleasedSurface(this);
					}
					ReleaseD9AuxiliarySurfaces();
					ReleaseDirectDrawResources();
				}
				else
				{
					delete this;
				}
			}
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
// IDirectDrawSurface v1 functions
// ******************************

HRESULT m_IDirectDrawSurfaceX::AddAttachedSurface(LPDIRECTDRAWSURFACE7 lpDDSurface, DWORD DirectXVersion)
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

		AddAttachedSurfaceToMap(lpAttachedSurfaceX, true, DirectXVersion, 1);

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
		bool RequiresFxStruct = (dwFlags & (DDBLT_DDFX | DDBLT_COLORFILL | DDBLT_DEPTHFILL | DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_ROP | DDBLT_ROTATIONANGLE));
		if (RequiresFxStruct && !lpDDBltFx)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: DDBLTFX structure not found!");
			return DDERR_INVALIDPARAMS;
		}

		// Check for DDBLTFX structure size
		if (RequiresFxStruct && lpDDBltFx->dwSize != sizeof(DDBLTFX))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: DDBLTFX structure is not initialized to the correct size: " << lpDDBltFx->dwSize);
			return DDERR_INVALIDPARAMS;
		}

		// Check for rotation flags
		// ToDo: add support for other rotation flags (90,180, 270).  Not sure if any game uses these other flags.
		if ((dwFlags & DDBLT_ROTATIONANGLE) || ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & (DDBLTFX_ROTATE90 | DDBLTFX_ROTATE180 | DDBLTFX_ROTATE270))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Rotation operations Not Implemented: " << Logging::hex(lpDDBltFx->dwDDFX & (DDBLTFX_ROTATE90 | DDBLTFX_ROTATE180 | DDBLTFX_ROTATE270)));
			return DDERR_NOROTATIONHW;
		}

		// Check supported raster operations
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

		// Set critical section for current surface
		std::vector<ScopedCriticalSection> ThreadLockDS;
		ThreadLockDS.reserve(2);
		ThreadLockDS.emplace_back(&ddscs);
		ThreadLockDS.emplace_back(&lpDDSrcSurfaceX->ddscs);

		// Check if locked from other thread
		if (BltWait)
		{
			// Wait for lock from other thread
			DWORD beginTime = timeGetTime();
			while (IsLockedFromOtherThread() || lpDDSrcSurfaceX->IsLockedFromOtherThread())
			{
				Utils::BusyWaitYield((DWORD)-1);

				// Break once timeout has passed
				if ((timeGetTime() - beginTime) >= SurfaceWaitTimeoutMS)
				{
					break;
				}
			}
		}

		// Set critical section
		ScopedDDCriticalSection ThreadLockDD;
		ThreadLockDS.clear();

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
			return (*d3d9Device)->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, float(double(lpDDBltFx->dwFillDepth) / double(ConvertDepthValue(0xFFFFFFFF, surface.Format))), 0);
		}

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
			// Set blt flag
			IsInBlt = true;
			lpDDSrcSurfaceX->IsInBlt = true;

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
				// Set vertical sync wait timer
				if (SUCCEEDED(c_hr) && (dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & DDBLTFX_NOTEARING))
				{
					ddrawParent->SetVsync();
				}

				if (PresentBlt)
				{
					// Set dirty flag
					SetDirtyFlag(MipMapLevel);

					// Present surface
					EndWritePresent(lpDestRect, true, PresentBlt, IsSkipScene);
				}
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

	ScopedDDCriticalSection ThreadLockDD;

	// Check for device interface before doing batch
	HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
	if (FAILED(c_hr))
	{
		return c_hr;
	}

	HRESULT hr = DD_OK;

	bool IsSkipScene = false;

	// Present before write if needed
	BeginWritePresent(IsSkipScene);

	IsInBltBatch = true;

	for (DWORD x = 0; x < dwCount; x++)
	{
		IsSkipScene |= (lpDDBltBatch[x].lprDest) ? CheckRectforSkipScene(*lpDDBltBatch[x].lprDest) : false;

		hr = Blt(lpDDBltBatch[x].lprDest, (LPDIRECTDRAWSURFACE7)lpDDBltBatch[x].lpDDSSrc, lpDDBltBatch[x].lprSrc, lpDDBltBatch[x].dwFlags | DDBLT_DONOTWAIT, lpDDBltBatch[x].lpDDBltFx, MipMapLevel, false);
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
		SetDirtyFlag(MipMapLevel);

		// Present surface
		EndWritePresent(nullptr, false, true, IsSkipScene);
	}

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

HRESULT m_IDirectDrawSurfaceX::EnumAttachedSurfaces(LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback, DWORD MipMapLevel, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		return EnumAttachedSurfaces2(lpContext, (LPDDENUMSURFACESCALLBACK7)lpEnumSurfacesCallback, MipMapLevel, DirectXVersion);
	}

	if (!lpEnumSurfacesCallback)
	{
		return DDERR_INVALIDPARAMS;
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

	if (Config.Dd7to9)
	{
		return EnumOverlayZOrders2(dwFlags, lpContext, (LPDDENUMSURFACESCALLBACK7)lpfnCallback, DirectXVersion);
	}

	if (!lpfnCallback)
	{
		return DDERR_INVALIDPARAMS;
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

		// Prepare critical sections
		std::vector<ScopedCriticalSection> ThreadLockDS;
		ThreadLockDS.reserve(FlipList.size() + 1);
		ThreadLockDS.emplace_back(&ddscs);

		// Construct ScopedCriticalSection and locks each surface
		for (auto& pSurfaceX : FlipList)
		{
			ThreadLockDS.emplace_back(&pSurfaceX->ddscs);
		}

		// Check if locked from other thread
		if (FlipWait)
		{
			// Wait for locks from other threads
			DWORD beginTime = timeGetTime();
			while (FlipSurfacesAreLockedFromOtherThread())
			{
				Utils::BusyWaitYield((DWORD)-1);

				// Break once timeout has passed
				if ((timeGetTime() - beginTime) >= SurfaceWaitTimeoutMS)
				{
					break;
				}
			}
		}

		ScopedDDCriticalSection ThreadLockDD;
		ThreadLockDS.clear();

		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
		if (FAILED(c_hr))
		{
			return c_hr;
		}

		// Present before write if needed
		BeginWritePresent(false);

		HRESULT hr = DD_OK;

		do {
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
				SetDirtyFlag(0);

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
				ddrawParent->SetCurrentRenderTarget();
			}

		} while (false);

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

	if (Config.Dd7to9)
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
				if (Level < GetMaxMipMapLevel(surfaceDesc2.dwWidth, surfaceDesc2.dwHeight) - 1)
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

	if (Config.Dd7to9)
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

		// Set critical section for current surface
		std::vector<ScopedCriticalSection> ThreadLockDS;
		ThreadLockDS.emplace_back(&ddscs);

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

		ScopedDDCriticalSection ThreadLockDD;
		ThreadLockDS.clear();

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

		// Present before write if needed
		BeginWritePresent(false);

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		IsPreparingDC = true;

		// Check if render target should use shadow
		if ((surface.Usage & D3DUSAGE_RENDERTARGET) && !IsUsingShadowSurface())
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
						(hr == DDERR_WASSTILLDRAWING || IsSurfaceBusy(true, false)) ? DDERR_SURFACEBUSY : DDERR_GENERIC;
					break;
				}
			}

			// Set DC flag
			IsInDC = true;

			// Set LastDC
			LastDC = *lphDC;

		} while (false);

		IsPreparingDC = false;

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

HRESULT m_IDirectDrawSurfaceX::GetFlipStatus(DWORD dwFlags, bool CheckOnly)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Flip can be called only for a surface that has the DDSCAPS_FLIP and DDSCAPS_FRONTBUFFER capabilities
		if (!IsFlipSurface())
		{
			if (!CheckOnly)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: This surface cannot be flipped");
			}
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

	if (Config.Dd7to9)
	{
		if (lpDDSurfaceDesc && lpDDSurfaceDesc->dwSize == sizeof(DDSURFACEDESC2))
		{
			return GetSurfaceDesc2((LPDDSURFACEDESC2)lpDDSurfaceDesc, MipMapLevel, DirectXVersion);
		}

		if (!lpDDSurfaceDesc || lpDDSurfaceDesc->dwSize != sizeof(DDSURFACEDESC))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc) ? lpDDSurfaceDesc->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2 = {};
		Desc2.dwSize = sizeof(DDSURFACEDESC2);

		HRESULT hr = GetSurfaceDesc2(&Desc2, MipMapLevel, DirectXVersion);

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

		// Handle mipmaps
		if (MipMapLevel && MipMaps.size())
		{
			// Remove a couple of flags
			lpDDSurfaceDesc2->dwFlags &= ~(DDSD_LPSURFACE | DDSD_PITCH | DDSD_LINEARSIZE);
			lpDDSurfaceDesc2->lpSurface = nullptr;
			lpDDSurfaceDesc2->lPitch = 0;

			// Handle new v7 flag
			if (DirectXVersion == 7)
			{
				lpDDSurfaceDesc2->ddsCaps.dwCaps2 |= DDSCAPS2_MIPMAPSUBLEVEL;
			}

			// Handle dummy mipmaps
			if (IsDummyMipMap(MipMapLevel))
			{
				DWORD Level = (MipMapLevel & ~DXW_IS_MIPMAP_DUMMY);

				// Get width and height
				DWORD BitCount = surface.BitCount ? surface.BitCount : GetBitCount(lpDDSurfaceDesc2->ddpfPixelFormat);
				DWORD Width = surface.Width ? surface.Width : GetByteAlignedWidth(lpDDSurfaceDesc2->dwWidth, BitCount);
				DWORD Height = surface.Height ? surface.Height : GetByteAlignedWidth(lpDDSurfaceDesc2->dwHeight, BitCount);
				lpDDSurfaceDesc2->dwWidth = max(1, Width >> Level);
				lpDDSurfaceDesc2->dwHeight = max(1, Height >> Level);

				// Mipmap count
				lpDDSurfaceDesc2->dwMipMapCount = 1;
			}
			// Handle normal mipmaps
			else
			{
				// Check for device interface to ensure correct max MipMap level
				CheckInterface(__FUNCTION__, true, true, false);

				// Get width and height
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

				// Set pitch
				if (MipMaps[Level].lPitch)
				{
					lpDDSurfaceDesc2->dwFlags |= DDSD_PITCH;
					lpDDSurfaceDesc2->lPitch = MipMaps[Level].lPitch;
				}

				// Mipmap count
				lpDDSurfaceDesc2->dwMipMapCount = MaxMipMapLevel + 1 > MipMapLevel ? MaxMipMapLevel + 1 - MipMapLevel : 1;
			}

			// Set pitch
			if (!(lpDDSurfaceDesc2->dwFlags & DDSD_PITCH))
			{
				DWORD Pitch = ComputePitch(GetDisplayFormat(lpDDSurfaceDesc2->ddpfPixelFormat), lpDDSurfaceDesc2->dwWidth, lpDDSurfaceDesc2->dwHeight);
				if (Pitch)
				{
					lpDDSurfaceDesc2->dwFlags |= DDSD_PITCH;
					lpDDSurfaceDesc2->lPitch = Pitch;
				}
			}
		}
		else if (MipMapLevel)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: MipMap found with no MipMap list!");
		}

		// Set correct flags for textures
		if (ISDXTEX(surface.Format))
		{
			FixTextureFlags(lpDDSurfaceDesc2);
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

	if (Config.Dd7to9)
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
				if (IsSurfaceLost && !ComplexChild)	// Complex children don't get surface lost notice
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
	if (Config.Dd7to9)
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

		// If primary surface and palette surface and created via Lock() then mark as created by lock to emulate surface (eg. Diablo, Wizardry 8, Wizards and Warriors)
		if (!IsUsingEmulation() && !IsSurfaceTexture() && surfaceDesc2.dwBackBufferCount == 0 && (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_FLIP) == 0 && ddrawParent &&
			(((surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) && !IsPrimaryOrBackBuffer() && ddrawParent->IsExclusiveMode() && IsDisplayResolution(surfaceDesc2.dwWidth, surfaceDesc2.dwHeight))) ||
			(ShouldEmulate == SC_NOT_CREATED && IsPrimarySurface() && surfaceDesc2.dwFlags == DDSD_CAPS && ddrawParent->GetDisplayBPP(nullptr) == 8))
		{
			ShouldEmulate = SC_FORCE_EMULATED;
		}

		// Set to indicate that Lock should wait until it can obtain a valid memory pointer before returning.
		const bool LockWait = (((dwFlags & DDLOCK_WAIT) || DirectXVersion == 7) && (dwFlags & DDLOCK_DONOTWAIT) == 0);

		// Set critical section for current surface
		std::vector<ScopedCriticalSection> ThreadLockDS;
		ThreadLockDS.emplace_back(&ddscs);

		// Check if locked from other thread
		if (LockWait)
		{
			// Wait for lock from other thread
			DWORD beginTime = timeGetTime();
			while (IsLockedFromOtherThread())
			{
				Utils::BusyWaitYield((DWORD)-1);

				// Break once timeout has passed
				if ((timeGetTime() - beginTime) >= SurfaceWaitTimeoutMS)
				{
					break;
				}
			}
		}

		ScopedDDCriticalSection ThreadLockDD;
		ThreadLockDS.clear();

		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, false);

		// Prepare surfaceDesc
		GetSurfaceDesc2(lpDDSurfaceDesc2, MipMapLevel, DirectXVersion);
		if (!surface.UsingSurfaceMemory && !IsUsingEmulation())
		{
			lpDDSurfaceDesc2->lpSurface = dummySurface.data();
		}
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
			lpDDSurfaceDesc2->dwFlags |= DDSD_LPSURFACE;
			// Add surface size to dummy data address to ensure that each mipmap gets a unique address
			lpDDSurfaceDesc2->lpSurface = dummySurface.data() + (lpDDSurfaceDesc2->dwWidth * lpDDSurfaceDesc2->dwHeight * surface.BitCount);
			if (!(lpDDSurfaceDesc2->dwFlags & DDSD_PITCH))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: no pitch found!");
				return DDERR_GENERIC;
			}
			return DD_OK;
		}

		// Convert flags to d3d9
		DWORD Flags = (dwFlags & (D3DLOCK_READONLY | D3DLOCK_NOOVERWRITE)) |
			((dwFlags & D3DLOCK_NOSYSLOCK) ? D3DLOCK_NOSYSLOCK : 0) |
			(!LockWait ? D3DLOCK_DONOTWAIT : 0) |
			((dwFlags & DDLOCK_NODIRTYUPDATE) ? D3DLOCK_NO_DIRTY_UPDATE : 0);

		// Check if the scene needs to be presented
		const bool IsSkipScene = (CheckRectforSkipScene(DestRect) || (Flags & D3DLOCK_READONLY));

		// Present before write if needed
		BeginWritePresent(IsSkipScene);

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		IsLocking = true;

		// Check if render target should use shadow
		if (surface.Usage & D3DUSAGE_RENDERTARGET)
		{
			if (surface.Type == D3DTYPE_RENDERTARGET)
			{
				// Don't use shadow for Lock()
				// Some games write to surface without locking so we don't want to give them a shadow surface or it could make the shadow surface out of sync
				PrepareRenderTarget();
			}
			else if (!IsUsingShadowSurface())
			{
				SetRenderTargetShadow();
			}
		}

		HRESULT hr = DD_OK;

		do {
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
					if (IsSurfaceLocked(false))
					{
						LOG_LIMIT(100, __FUNCTION__ << " Warning: attempting to lock surface twice!");
						UnLockD3d9Surface(MipMapLevel);
					}
					ret = LockD3d9Surface(&LockedRect, &DestRect, Flags, MipMapLevel);
				}
				if (FAILED(ret))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock surface." << (surface.Surface ? " Is Surface." : " Is Texture.") <<
						" Size: " << lpDDSurfaceDesc2->dwWidth << "x" << lpDDSurfaceDesc2->dwHeight << " Format: " << surface.Format << " Flags: " << Logging::hex(Flags) <<
						" HasData: " << surface.HasData << " Locked: " << IsSurfaceLocked(false) << " DC: " << IsSurfaceInDC() << " Blt: " << IsSurfaceBlitting() << " hr: " << (D3DERR)ret);
					hr = (ret == D3DERR_DEVICELOST || IsLost() == DDERR_SURFACELOST) ? DDERR_SURFACELOST :
						(IsSurfaceBusy(false, true)) ? DDERR_SURFACEBUSY :
						(ret == DDERR_WASSTILLDRAWING || (!LockWait && IsPresentRunning)) ? DDERR_WASSTILLDRAWING : DDERR_GENERIC;
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
				(ISDXTEX(surface.Format) || surface.Format == D3DFMT_YV12 || surface.Format == D3DFMT_NV12) ?
				ComputePitch(surface.Format, lpDDSurfaceDesc2->dwWidth, lpDDSurfaceDesc2->dwHeight) :
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
			}
			else
			{
				if ((surfaceDesc2.dwFlags & DDSD_PITCH) && surfaceDesc2.lPitch != LockedRect.Pitch)
				{
					LOG_LIMIT(100, __FUNCTION__ << " (" << this << ")" << " Warning: surface pitch does not match locked pitch! Format: " << surface.Format <<
						" Width: " << surfaceDesc2.dwWidth << " Pitch: " << surfaceDesc2.lPitch << "->" << LockedRect.Pitch <<
						" Default: " << ComputePitch(surface.Format, surface.Width, surface.BitCount) << " BitCount: " << surface.BitCount);
				}
				surfaceDesc2.lPitch = LockedRect.Pitch;
				surfaceDesc2.dwFlags |= DDSD_PITCH;
			}

			// Set correct flags for textures
			if (ISDXTEX(surface.Format))
			{
				FixTextureFlags(lpDDSurfaceDesc2);
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

		IsLocking = false;

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

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

		if (SUCCEEDED(hr))
		{
			// Set dirty flag
			SetDirtyFlag(0);

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
			if (FAILED(ddrawParent->ResetD9Device()))
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

		// Check for color space
		if (lpDDColorKey && (dwFlags & DDCKEY_COLORSPACE) && lpDDColorKey->dwColorSpaceLowValue != lpDDColorKey->dwColorSpaceHighValue)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: color space not supported!");
			return DDERR_NOCOLORKEYHW;
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
				if (!(surfaceDesc2.dwFlags & dds) || ColorKey.dwColorSpaceLowValue != surfaceDesc2.ddckCKSrcBlt.dwColorSpaceLowValue)
				{
					ShaderColorKey.IsSet = false;
					surface.IsDrawTextureDirty = true;
				}
				surfaceDesc2.ddckCKSrcBlt = ColorKey;
				break;
			case DDSD_CKSRCOVERLAY:
				surfaceDesc2.ddckCKSrcOverlay = ColorKey;
				break;
			}

			// Set color key flag
			surfaceDesc2.dwFlags |= dds;
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

		ScopedDDCriticalSection ThreadLockDD;

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

		// Reset data for new palette
		surface.LastPaletteUSN = 0;
		surface.PaletteEntryArray = nullptr;

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
			SetDirtyFlag(LastLock.MipMapLevel);

			// Keep surface insync
			EndWriteSyncSurfaces(&LastLock.Rect);

			// Present surface
			EndWritePresent(&LastLock.Rect, true, true, LastLock.IsSkipScene);
		}

		return hr;
	}

	return ProxyInterface->Unlock(lpRect);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlay(LPRECT lpSrcRect, LPDIRECTDRAWSURFACE7 lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, LPDDOVERLAYFX lpDDOverlayFx)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for overlay flag
		if (!(surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_OVERLAY))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Not an overlay surface!");
			return DDERR_NOTAOVERLAYSURFACE;
		}

		// Turns off this overlay.
		if (dwFlags & DDOVER_HIDE)
		{
			SurfaceOverlay.OverlayEnabled = false;
			return DD_OK;
		}

		// Check for required DDOVERLAYFX structure
		bool RequiresFxStruct = (dwFlags & (DDOVER_DDFX | DDOVER_ALPHADESTCONSTOVERRIDE | DDOVER_ALPHADESTSURFACEOVERRIDE | DDOVER_ALPHAEDGEBLEND | DDOVER_ALPHASRCCONSTOVERRIDE |
			DDOVER_ALPHASRCSURFACEOVERRIDE | DDOVER_ARGBSCALEFACTORS | DDOVER_KEYDESTOVERRIDE | DDOVER_KEYSRCOVERRIDE));
		if (RequiresFxStruct && !lpDDOverlayFx)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: DDOVERLAYFX structure not found!");
			return DDERR_INVALIDPARAMS;
		}

		// Check for DDOVERLAYFX structure size
		if (RequiresFxStruct && lpDDOverlayFx->dwSize != sizeof(DDOVERLAYFX))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: DDOVERLAYFX structure is not initialized to the correct size: " << lpDDOverlayFx->dwSize);
			return DDERR_INVALIDPARAMS;
		}

		// Cehck for auto flip flag
		if (dwFlags & DDOVER_AUTOFLIP)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: overlay flip not implemented!");
			return DDERR_UNSUPPORTED;
		}

		// Check for alpha flags
		if (dwFlags & (DDOVER_ALPHADEST | DDOVER_ALPHADESTCONSTOVERRIDE | DDOVER_ALPHADESTNEG | DDOVER_ALPHADESTSURFACEOVERRIDE |
			DDOVER_ALPHAEDGEBLEND | DDOVER_ALPHASRC | DDOVER_ALPHASRCCONSTOVERRIDE | DDOVER_ALPHASRCNEG | DDOVER_ALPHASRCSURFACEOVERRIDE))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: overlay alpha not implemented!");
			return DDERR_NOALPHAHW;
		}

		// Check scaling flags
		if (dwFlags & (DDOVER_ARGBSCALEFACTORS | DDOVER_DEGRADEARGBSCALING))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: overlay scale factors not implemented!");
		}

		// Check BOB flags
		if (dwFlags & (DDOVER_BOB | DDOVER_BOBHARDWARE | DDOVER_OVERRIDEBOBWEAVE))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: overlay BOB not implemented!");
		}

		// Check interleave flags
		if (dwFlags & (DDOVER_INTERLEAVED))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: overlay interleave not implemented!");
		}

		// Handle refresh flags
		if (dwFlags & (DDOVER_REFRESHALL | DDOVER_REFRESHDIRTYRECTS))
		{
			// Just refresh whole surface
			return PresentOverlay(nullptr);
		}

		// Check dirty flag
		if (dwFlags & DDOVER_ADDDIRTYRECT)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: overlay dirty rect not implemented!");
			return DD_OK;	// Just return ok for now, nothing else may be needed here
		}

		// Get WrapperX
		if (!lpDDDestSurface)
		{
			return DDERR_INVALIDPARAMS;
		}
		m_IDirectDrawSurfaceX* lpDDDestSurfaceX = nullptr;
		lpDDDestSurface->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpDDDestSurfaceX);
		if (!lpDDDestSurfaceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surfaceX!");
			return DDERR_INVALIDPARAMS;
		}
		if (lpDDDestSurfaceX == this)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: cannot overlay surface onto itself!");
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		HRESULT c_hr = CheckInterface(__FUNCTION__, true, true, true);
		HRESULT s_hr = lpDDDestSurfaceX->CheckInterface(__FUNCTION__, true, true, true);
		if (FAILED(c_hr) || FAILED(s_hr))
		{
			return (c_hr == DDERR_SURFACELOST || s_hr == DDERR_SURFACELOST) ? DDERR_SURFACELOST : FAILED(c_hr) ? c_hr : s_hr;
		}

		// Check rect
		if ((lpSrcRect && !CheckCoordinates(lpSrcRect)) ||
			(lpDestRect && !lpDDDestSurfaceX->CheckCoordinates(lpDestRect)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid rect: " << lpSrcRect << " -> " << lpDestRect);
			return DDERR_INVALIDRECT;
		}

		// Add entry to overlay vector
		SURFACEOVERLAY Overlay;
		Overlay.OverlayEnabled = (SurfaceOverlay.OverlayEnabled || (dwFlags & DDOVER_SHOW));
		if (lpSrcRect)
		{
			Overlay.isSrcRectNull = false;
			Overlay.SrcRect = *lpSrcRect;
		}
		Overlay.lpDDDestSurface = lpDDDestSurface;
		Overlay.lpDDDestSurfaceX = lpDDDestSurfaceX;
		if (lpDestRect)
		{
			Overlay.isDestRectNull = false;
			Overlay.DestRect = *lpDestRect;
		}
		Overlay.DDOverlayFxFlags = dwFlags;
		Overlay.DDBltFx.dwSize = sizeof(DDBLTFX);
		if (lpDDOverlayFx)
		{
			Overlay.DDOverlayFx = *lpDDOverlayFx;

			// Color keying
			if (dwFlags & DDOVER_KEYDESTOVERRIDE)
			{
				Overlay.DDBltFxFlags |= (DDBLT_DDFX | DDBLT_KEYDESTOVERRIDE);
				Overlay.DDBltFx.ddckDestColorkey = lpDDOverlayFx->dckDestColorkey;
			}
			else if (dwFlags & DDOVER_KEYSRCOVERRIDE)
			{
				Overlay.DDBltFxFlags |= (DDBLT_DDFX | DDBLT_KEYSRCOVERRIDE);
				Overlay.DDBltFx.ddckSrcColorkey = lpDDOverlayFx->dckSrcColorkey;
			}
			// DDOverlayFx flags
			if (dwFlags & DDOVER_DDFX)
			{
				Overlay.DDBltFxFlags |= DDBLT_DDFX;
				Overlay.DDBltFx.dwDDFX = (lpDDOverlayFx->dwFlags & (DDBLTFX_ARITHSTRETCHY | DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN));
			}
		}

		// Update overlay
		SurfaceOverlay = Overlay;

		// Return
		return DD_OK;
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

// ******************************
// IDirectDrawSurface v2 functions
// ******************************

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
		// Prevents a system-memory surface from being paged out while a bit block transfer (bitblt) operation
		// that uses direct memory access (DMA) transfers to or from system memory is in progress.
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

// ******************************
// IDirectDrawSurface v3 functions
// ******************************

HRESULT m_IDirectDrawSurfaceX::SetSurfaceDesc(LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
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

		// Handle lpSurface flag
		if ((SurfaceFlags & DDSD_LPSURFACE) && lpDDSurfaceDesc2->lpSurface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: lpSurface not fully Implemented.");

			SurfaceFlags &= ~DDSD_LPSURFACE;
			surfaceDesc2.dwFlags |= DDSD_LPSURFACE;
			surfaceDesc2.lpSurface = lpDDSurfaceDesc2->lpSurface;
			surface.UsingSurfaceMemory = true;
			if (surface.Surface || surface.Texture)
			{
				CreateD9Surface();
			}
		}

		// Handle width, height and pitch flags
		if (SurfaceFlags & (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: surfaceDesc flags being updated: " << Logging::hex(SurfaceFlags));
			bool Flag = false;
			if ((SurfaceFlags & DDSD_WIDTH) && lpDDSurfaceDesc2->dwWidth)
			{
				Flag = Flag || (surfaceDesc2.dwWidth != lpDDSurfaceDesc2->dwWidth);
				SurfaceFlags &= ~DDSD_WIDTH;
				ResetDisplayFlags &= ~DDSD_WIDTH;
				surfaceDesc2.dwFlags |= DDSD_WIDTH;
				surfaceDesc2.dwWidth = lpDDSurfaceDesc2->dwWidth;
			}
			if ((SurfaceFlags & DDSD_HEIGHT) && lpDDSurfaceDesc2->dwHeight)
			{
				Flag = Flag || (surfaceDesc2.dwHeight != lpDDSurfaceDesc2->dwHeight);
				SurfaceFlags &= ~DDSD_HEIGHT;
				ResetDisplayFlags &= ~DDSD_HEIGHT;
				surfaceDesc2.dwFlags |= DDSD_HEIGHT;
				surfaceDesc2.dwHeight = lpDDSurfaceDesc2->dwHeight;
			}
			if (SurfaceFlags & DDSD_PITCH)
			{
				SurfaceFlags &= ~DDSD_PITCH;
			}
			if (Flag && (surface.Surface || surface.Texture))
			{
				CreateD9Surface();
			}
		}

		// Check for unhandled flags
		if (SurfaceFlags)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: flags not implemented " << Logging::hex(SurfaceFlags));
			return DDERR_UNSUPPORTED;
		}

		return DD_OK;
	}

	return ProxyInterface->SetSurfaceDesc(lpDDSurfaceDesc2, dwFlags);
}

// ******************************
// IDirectDrawSurface v4 functions
// ******************************

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

// ******************************
// IDirect3DTexture v7 functions moved here
// ******************************

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

// ******************************
// Helper functions
// ******************************

void m_IDirectDrawSurfaceX::InitInterface(DWORD DirectXVersion)
{
	ScopedDDCriticalSection ThreadLockDD;

	if (ddrawParent)
	{
		ddrawParent->AddSurface(this);
	}

	if (Config.Dd7to9)
	{
		AddRef(DirectXVersion);

		InitializeCriticalSection(&ddscs);

		if (ddrawParent)
		{
			d3d9Device = ddrawParent->GetDirectD9Device();
		}

		// Set Uniqueness Value
		UniquenessValue = 1;

		// Update surface description and create backbuffers
		InitSurfaceDesc(DirectXVersion);
	}
}

void m_IDirectDrawSurfaceX::ReleaseInterface()
{
	if (Config.Exiting)
	{
		return;
	}

	ScopedDDCriticalSection ThreadLockDD;

	if (ddrawParent)
	{
		ddrawParent->ClearSurface(this);
	}

	// Don't delete wrapper interface
	SaveInterfaceAddress(WrapperInterface, WrapperInterfaceBackup);
	SaveInterfaceAddress(WrapperInterface2, WrapperInterfaceBackup2);
	SaveInterfaceAddress(WrapperInterface3, WrapperInterfaceBackup3);
	SaveInterfaceAddress(WrapperInterface4, WrapperInterfaceBackup4);
	SaveInterfaceAddress(WrapperInterface7, WrapperInterfaceBackup7);

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

	ReleaseDirectDrawResources();

	if (Config.Dd7to9)
	{
		ReleaseD9Surface(false, false);

		// Delete critical section last
		DeleteCriticalSection(&ddscs);
	}
}

HRESULT m_IDirectDrawSurfaceX::CheckInterface(char* FunctionName, bool CheckD3DDevice, bool CheckD3DSurface, bool CheckLostSurface)
{
	// Check ddrawParent device
	if (!ddrawParent)
	{
		ScopedDDCriticalSection ThreadLockDD;

		m_IDirectDrawX* pInterface = m_IDirectDrawX::GetDirectDrawInterface();
		if (pInterface)
		{
			LOG_LIMIT(100, FunctionName << " Error: no ddraw parent!");
			return DDERR_INVALIDOBJECT;
		}

		SetDdrawParent(pInterface);
		ddrawParent->AddSurface(this);
	}

	// Check d3d9 device
	if (CheckD3DDevice)
	{
		if (!ddrawParent->CheckD9Device(FunctionName) || !d3d9Device || !*d3d9Device)
		{
			LOG_LIMIT(100, FunctionName << " Error: d3d9 device not setup!");
			return DDERR_INVALIDOBJECT;
		}
		if (ShouldPresentToWindow(true))
		{
			HWND CurrentClipperHWnd = ddrawParent->GetClipperHWnd();

			HWND hWnd = nullptr;
			if (attachedClipper)
			{
				attachedClipper->GetHWnd(&hWnd);
				if (IsWindow(hWnd) && hWnd != CurrentClipperHWnd)
				{
					ddrawParent->SetClipperHWnd(hWnd);
				}
			}
			if (!IsWindow(hWnd) && (!IsWindow(CurrentClipperHWnd) || !Utils::IsMainWindow(CurrentClipperHWnd)))
			{
				hWnd = Utils::GetMainWindowForProcess(GetCurrentProcessId());
				if (hWnd != CurrentClipperHWnd)
				{
					ddrawParent->SetClipperHWnd(hWnd);
				}
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
			if (SUCCEEDED(ddrawParent->ResetD9Device()))
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
		if (IsUsingEmulation() && !CanSurfaceUseEmulation() && !IsSurfaceBusy())
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
			ScopedDDCriticalSection ThreadLockDD;

			ReleaseD9Surface(true, false);
		}

		// Make sure surface exists, if not then create it
		if ((!surface.Surface && !surface.Texture) ||
			(IsPrimaryOrBackBuffer() && LastWindowedMode != surface.IsUsingWindowedMode) ||
			(PrimaryDisplayTexture && !ShouldPresentToWindow(false)))
		{
			if (FAILED(CreateD9Surface()))
			{
				LOG_LIMIT(100, FunctionName << " Error: d3d9 surface texture not setup!");
				return DDERR_WRONGMODE;
			}
		}

		// Check auxiliary surfaces
		if ((RecreateAuxiliarySurfaces || surface.RecreateAuxiliarySurfaces) && FAILED(CreateD9AuxiliarySurfaces()))
		{
			return DDERR_WRONGMODE;
		}
	}

	return DD_OK;
}

void* m_IDirectDrawSurfaceX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 0:
		if (WrapperInterface7) return WrapperInterface7;
		if (WrapperInterface4) return WrapperInterface4;
		if (WrapperInterface3) return WrapperInterface3;
		if (WrapperInterface2) return WrapperInterface2;
		if (WrapperInterface) return WrapperInterface;
		break;
	case 1:
		return GetInterfaceAddress(WrapperInterface, WrapperInterfaceBackup, (LPDIRECTDRAWSURFACE)ProxyInterface, this);
	case 2:
		return GetInterfaceAddress(WrapperInterface2, WrapperInterfaceBackup2, (LPDIRECTDRAWSURFACE2)ProxyInterface, this);
	case 3:
		return GetInterfaceAddress(WrapperInterface3, WrapperInterfaceBackup3, (LPDIRECTDRAWSURFACE3)ProxyInterface, this);
	case 4:
		return GetInterfaceAddress(WrapperInterface4, WrapperInterfaceBackup4, (LPDIRECTDRAWSURFACE4)ProxyInterface, this);
	case 7:
		return GetInterfaceAddress(WrapperInterface7, WrapperInterfaceBackup7, (LPDIRECTDRAWSURFACE7)ProxyInterface, this);
	}
	LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
	return nullptr;
}

void m_IDirectDrawSurfaceX::SetDdrawParent(m_IDirectDrawX* ddraw)
{
	if (!ddraw)
	{
		return;
	}

	if (ddrawParent && ddrawParent != ddraw)
	{
		Logging::Log() << __FUNCTION__ << " Warning: ddrawParent interface has already been set!";
	}

	ddrawParent = ddraw;

	if (attached3DTexture)
	{
		attached3DTexture->SetD3DDevice(ddrawParent->GetCurrentD3DDevice());
	}

	d3d9Device = ddrawParent->GetDirectD9Device();
}

void m_IDirectDrawSurfaceX::ClearDdraw()
{
	ddrawParent = nullptr;
	d3d9Device = nullptr;

	if (attached3DTexture)
	{
		attached3DTexture->ClearD3DDevice();
	}
}

void m_IDirectDrawSurfaceX::ReleaseDirectDrawResources()
{
	if (attachedClipper)
	{
		attachedClipper->Release();
		attachedClipper = nullptr;
	}

	if (attachedPalette)
	{
		attachedPalette->Release();
		attachedPalette = nullptr;
	}

	if (attached3DTexture)
	{
		attached3DTexture->DeleteMe();
		attached3DTexture = nullptr;
	}

	if (attached3DDevice)
	{
		attached3DDevice->DeleteMe();
		attached3DDevice = nullptr;
	}

	if (ddrawParent)
	{
		ddrawParent->ClearSurface(this);
	}

	for (auto& entry : AttachedSurfaceMap)
	{
		if (entry.second.RefCount == 1)
		{
			entry.second.RefCount = 0;		// Clear ref count before release
			entry.second.pSurface->Release(entry.second.DxVersion);
		}
	}
	AttachedSurfaceMap.clear();
}

LPDIRECT3DSURFACE9 m_IDirectDrawSurfaceX::GetD3d9Surface(bool ShouldCheckInterface)
{
	// Check for device interface
	if (ShouldCheckInterface)
	{
		if (FAILED(CheckInterface(__FUNCTION__, true, true, true)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: surface cannot be setup!");
			return nullptr;
		}
	}
	// For threads that require no thread locks (CreateD9Device can be called from a different thread)
	else
	{
		if (!surface.Surface && !surface.Texture)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not setup!");
			return nullptr;
		}
	}

	return Get3DSurface();
}

LPDIRECT3DSURFACE9 m_IDirectDrawSurfaceX::Get3DSurface()
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

LPDIRECT3DSURFACE9 m_IDirectDrawSurfaceX::Get3DMipMapSurface(DWORD MipMapLevel)
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

void m_IDirectDrawSurfaceX::Release3DMipMapSurface(LPDIRECT3DSURFACE9 pSurfaceD9, DWORD MipMapLevel)
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

	return Get3DTexture();
}

LPDIRECT3DTEXTURE9 m_IDirectDrawSurfaceX::Get3DTexture()
{
	// Primary display texture
	if (PrimaryDisplayTexture && ShouldPresentToWindow(true))
	{
		if (IsPalette() && surface.IsUsingWindowedMode && (surface.DisplayTexture || !primary.PaletteTexture))
		{
			Logging::Log() << __FUNCTION__ << " Error: using non-shader palette surface on window mode not supported!";
		}
		return PrimaryDisplayTexture;
	}

	// Prepare paletted surface for display
	if (surface.IsPaletteDirty && IsUsingEmulation() && !primary.PaletteTexture)
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

HRESULT m_IDirectDrawSurfaceX::CreateD9AuxiliarySurfaces()
{
	// Create shadow surface
	if (!surface.Shadow && (surface.Usage & D3DUSAGE_RENDERTARGET))
	{
		D3DSURFACE_DESC Desc;
		if (FAILED(surface.Surface ? surface.Surface->GetDesc(&Desc) : surface.Texture->GetLevelDesc(0, &Desc)) ||
			FAILED((*d3d9Device)->CreateOffscreenPlainSurface(Desc.Width, Desc.Height, Desc.Format, D3DPOOL_SYSTEMMEM, &surface.Shadow, nullptr)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create shadow surface. Size: " << surface.Width << "x" << surface.Height << " Format: " << surface.Format << " dwCaps: " << surfaceDesc2.ddsCaps);
			return DDERR_GENERIC;
		}
	}

	// Create primary surface texture
	if (!PrimaryDisplayTexture && ShouldPresentToWindow(false))
	{
		D3DSURFACE_DESC Desc;
		if (FAILED(surface.Surface ? surface.Surface->GetDesc(&Desc) : surface.Texture->GetLevelDesc(0, &Desc)) ||
			FAILED((*d3d9Device)->CreateTexture(Desc.Width, Desc.Height, 1, 0, Desc.Format, D3DPOOL_DEFAULT, &PrimaryDisplayTexture, nullptr)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create primary surface texture. Size: " << surface.Width << "x" << surface.Height << " Format: " << surface.Format << " dwCaps: " << surfaceDesc2.ddsCaps);
			return DDERR_GENERIC;
		}
	}

	// Create palette surface
	if (!primary.PaletteTexture && IsPrimarySurface() && surface.Format == D3DFMT_P8)
	{
		if (FAILED((*d3d9Device)->CreateTexture(MaxPaletteSize, MaxPaletteSize, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &primary.PaletteTexture, nullptr)))
		{
			// Try failover format
			if (FAILED((*d3d9Device)->CreateTexture(MaxPaletteSize, MaxPaletteSize, 1, 0, GetFailoverFormat(D3DFMT_X8R8G8B8), D3DPOOL_MANAGED, &primary.PaletteTexture, nullptr)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create palette surface texture");
				return DDERR_GENERIC;
			}
		}
	}

	// Reset flags
	RecreateAuxiliarySurfaces = false;
	surface.RecreateAuxiliarySurfaces = false;

	return DD_OK;
}

HRESULT m_IDirectDrawSurfaceX::CreateD9Surface()
{
	// Don't recreate surface while it is locked
	if ((surface.Surface || surface.Texture) && IsSurfaceBusy(false, false))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface is busy! Locked: " << IsSurfaceLocked() << " DC: " << IsSurfaceInDC() << " Blt: " << IsSurfaceBlitting() << " ThreadID " << LockedWithID);
		return DDERR_GENERIC;
	}

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, false, false)))
	{
		return DDERR_GENERIC;
	}

	ScopedDDCriticalSection ThreadLockDD;

	// Release existing surface
	ReleaseD9Surface(true, false);

	// Update surface description
	UpdateSurfaceDesc();

	// Get texture format
	surface.Format = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);
	surface.BitCount = GetBitCount(surface.Format);
	SurfaceRequiresEmulation = (CanSurfaceUseEmulation() && (Config.DdrawEmulateSurface || ShouldEmulate == SC_FORCE_EMULATED ||
		surface.Format == D3DFMT_A8B8G8R8 || surface.Format == D3DFMT_X8B8G8R8 || surface.Format == D3DFMT_B8G8R8 || surface.Format == D3DFMT_R8G8B8));
	const bool CreateSurfaceEmulated = (CanSurfaceUseEmulation() && (SurfaceRequiresEmulation ||
		(IsPrimaryOrBackBuffer() && (Config.DdrawWriteToGDI || Config.DdrawReadFromGDI || Config.DdrawRemoveScanlines))));
	DCRequiresEmulation = (CanSurfaceUseEmulation() && surface.Format != D3DFMT_R5G6B5 && surface.Format != D3DFMT_X1R5G5B5 && surface.Format != D3DFMT_R8G8B8 && surface.Format != D3DFMT_X8R8G8B8);
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
	surface.Width = GetByteAlignedWidth(surfaceDesc2.dwWidth, surface.BitCount);
	surface.Height = surfaceDesc2.dwHeight;

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

	Logging::LogDebug() << __FUNCTION__ " (" << this << ") D3d9 Surface. Size: " << surface.Width << "x" << surface.Height << " Format: " << surface.Format <<
		" Pool: " << surface.Pool << " dwCaps: " << surfaceDesc2.ddsCaps << " " << surfaceDesc2;

	HRESULT hr = DD_OK;

	do {
		// Create depth stencil
		if (IsDepthStencil())
		{
			surface.Type = D3DTYPE_DEPTHSTENCIL;
			surface.Usage = D3DUSAGE_DEPTHSTENCIL;
			surface.Pool = D3DPOOL_DEFAULT;
			if (FAILED((*d3d9Device)->CreateDepthStencilSurface(surface.Width, surface.Height, Format, surface.MultiSampleType, surface.MultiSampleQuality, surface.MultiSampleType ? TRUE : FALSE, &surface.Surface, nullptr)) &&
				FAILED((*d3d9Device)->CreateDepthStencilSurface(surface.Width, surface.Height, GetFailoverFormat(Format), surface.MultiSampleType, surface.MultiSampleQuality, surface.MultiSampleType ? TRUE : FALSE, &surface.Surface, nullptr)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create depth stencil surface. Size: " << surface.Width << "x" << surface.Height << " Format: " << Format << " dwCaps: " << surfaceDesc2.ddsCaps);
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
			if (IsSurfaceTexture() || IsPalette())
			{
				surface.Type = D3DTYPE_TEXTURE;
				if (FAILED((*d3d9Device)->CreateTexture(surface.Width, surface.Height, 1, surface.Usage, Format, surface.Pool, &surface.Texture, nullptr)) &&
					FAILED((*d3d9Device)->CreateTexture(surface.Width, surface.Height, 1, surface.Usage, GetFailoverFormat(Format), surface.Pool, &surface.Texture, nullptr)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create render target texture. Size: " << surface.Width << "x" << surface.Height << " Format: " << Format << " dwCaps: " << surfaceDesc2.ddsCaps);
					hr = DDERR_GENERIC;
					break;
				}
			}
			else
			{
				surface.Type = D3DTYPE_RENDERTARGET;
				BOOL IsLockable = (surface.MultiSampleType || (surfaceDesc2.ddsCaps.dwCaps2 & DDSCAPS2_NOTUSERLOCKABLE)) ? FALSE : TRUE;
				if (FAILED((*d3d9Device)->CreateRenderTarget(surface.Width, surface.Height, Format, surface.MultiSampleType, surface.MultiSampleQuality, IsLockable, &surface.Surface, nullptr)) &&
					FAILED((*d3d9Device)->CreateRenderTarget(surface.Width, surface.Height, GetFailoverFormat(Format), surface.MultiSampleType, surface.MultiSampleQuality, IsLockable, &surface.Surface, nullptr)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create render target surface. Size: " << surface.Width << "x" << surface.Height << " Format: " << Format << " dwCaps: " << surfaceDesc2.ddsCaps);
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
			DWORD MipMapLevel = (CreateSurfaceEmulated || !MipMapCount) ? 1 : MipMapCount;
			HRESULT hr_t;
			do {
				surface.Usage = (Config.DdrawForceMipMapAutoGen && MipMapLevel > 1) ? D3DUSAGE_AUTOGENMIPMAP : 0;
				DWORD Level = ((surface.Usage & D3DUSAGE_AUTOGENMIPMAP) && MipMapLevel == MipMapCount) ? 0 : MipMapLevel;
				// Create texture
				hr_t = (*d3d9Device)->CreateTexture(surface.Width, surface.Height, Level, surface.Usage, Format, surface.Pool, &surface.Texture, nullptr);
				if (FAILED(hr_t))
				{
					hr_t = (*d3d9Device)->CreateTexture(surface.Width, surface.Height, Level, surface.Usage, GetFailoverFormat(Format), surface.Pool, &surface.Texture, nullptr);
				}
			} while (FAILED(hr_t) && ((!MipMapLevel && ++MipMapLevel) || --MipMapLevel > 0));
			if (FAILED(hr_t))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create surface texture. Size: " << surface.Width << "x" << surface.Height << " Format: " << Format << " dwCaps: " << surfaceDesc2.ddsCaps);
				hr = DDERR_GENERIC;
				break;
			}
			MaxMipMapLevel = (MipMapLevel > 1 && !IsMipMapAutogen()) ? MipMapLevel - 1 : 0;
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
			if (FAILED((*d3d9Device)->CreateOffscreenPlainSurface(surface.Width, surface.Height, Format, surface.Pool, &surface.Surface, nullptr)) &&
				FAILED((*d3d9Device)->CreateOffscreenPlainSurface(surface.Width, surface.Height, GetFailoverFormat(Format), surface.Pool, &surface.Surface, nullptr)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create offplain surface. Size: " << surface.Width << "x" << surface.Height << " Format: " << Format << " dwCaps: " << surfaceDesc2.ddsCaps);
				hr = DDERR_GENERIC;
				break;
			}
		}

		if (FAILED(CreateD9AuxiliarySurfaces()))
		{
			hr = DDERR_GENERIC;
			break;
		}

		surface.IsPaletteDirty = IsPalette();

	} while (false);

	// Create emulated surface using device context for creation
	bool EmuSurfaceCreated = false;
	if ((CreateSurfaceEmulated || IsUsingEmulation()) && !DoesDCMatch(surface.emu))
	{
		EmuSurfaceCreated = true;
		CreateDCSurface();
	}

	// Reset flags
	surface.HasData = false;
	surface.UsingShadowSurface = false;

	// Restore d3d9 surface texture data
	if (surface.Surface || surface.Texture)
	{
		// Fill surface with color
		if (Config.DdrawFillSurfaceColor)
		{
			static DWORD Count = 0;
			struct COLORS {
				DWORD a;
				DWORD r;
				DWORD g;
				DWORD b;
			};
			COLORS Colors[] = {
				{ 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000 },
				{ 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000 },
				{ 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFFFFFFFF },

				{ 0xFFFFFFFF, 0xFFFFFFFF, 0x55555555, 0x00000000 },
				{ 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x55555555 },
				{ 0xFFFFFFFF, 0x55555555, 0x00000000, 0xFFFFFFFF },

				{ 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x55555555 },
				{ 0xFFFFFFFF, 0x55555555, 0xFFFFFFFF, 0x00000000 },
				{ 0xFFFFFFFF, 0x00000000, 0x55555555, 0xFFFFFFFF },

				{ 0xFFFFFFFF, 0xFFFFFFFF, 0x55555555, 0x55555555 },
				{ 0xFFFFFFFF, 0x55555555, 0xFFFFFFFF, 0x55555555 },
				{ 0xFFFFFFFF, 0x55555555, 0x55555555, 0xFFFFFFFF },

				{ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 },
				{ 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF },
				{ 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF },

				{ 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF },
				{ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 },
				{ 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF },

				{ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x55555555 },
				{ 0xFFFFFFFF, 0x55555555, 0xFFFFFFFF, 0xFFFFFFFF },
				{ 0xFFFFFFFF, 0xFFFFFFFF, 0x55555555, 0xFFFFFFFF },

				{ 0xFFFFFFFF, 0xFFFFFFFF, 0x55555555, 0xFFFFFFFF },
				{ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x55555555 },
				{ 0xFFFFFFFF, 0x55555555, 0xFFFFFFFF, 0xFFFFFFFF },
			};

			if (Format == D3DFMT_P8)
			{
				ColorFill(nullptr, 10 * (Count + 1), 0);
			}
			else if (IsPixelFormatRGB(surfaceDesc2.ddpfPixelFormat))
			{
				ColorFill(nullptr,
					(Colors[Count].a & surfaceDesc2.ddpfPixelFormat.dwRGBAlphaBitMask) +
					(Colors[Count].r & surfaceDesc2.ddpfPixelFormat.dwRBitMask) +
					(Colors[Count].g & surfaceDesc2.ddpfPixelFormat.dwGBitMask) +
					(Colors[Count].b & surfaceDesc2.ddpfPixelFormat.dwBBitMask), 0);
			}
			else
			{
				ComPtr<IDirect3DSurface9> SrcSurface;
				DWORD t_Width = 128, t_Height = 128;
				if (SUCCEEDED((*d3d9Device)->CreateOffscreenPlainSurface(t_Width, t_Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, SrcSurface.GetAddressOf(), nullptr)))
				{
					D3DCOLOR NewColor =
						(Colors[Count].a & 0xFF000000) +
						(Colors[Count].r & 0x00FF0000) +
						(Colors[Count].g & 0x0000FF00) +
						(Colors[Count].b & 0x000000FF);
					D3DLOCKED_RECT LockedRect = {};
					if (SUCCEEDED(SrcSurface->LockRect(&LockedRect, nullptr, 0)))
					{
						BYTE* pBuffer = (BYTE*)LockedRect.pBits;
						for (UINT x = 0; x < t_Width; x++)
						{
							for (UINT y = 0; y < LockedRect.Pitch / sizeof(D3DCOLOR); y++)
							{
								((DWORD*)pBuffer)[y] = NewColor;
							}
							pBuffer += LockedRect.Pitch;
						}
						SrcSurface->UnlockRect();

						LPDIRECT3DSURFACE9 DstSurface = Get3DSurface();
						if (DstSurface)
						{
							D3DXLoadSurfaceFromSurface(DstSurface, nullptr, nullptr, SrcSurface.Get(), nullptr, nullptr, D3DX_FILTER_POINT, 0);
						}
					}
				}
			}
			if (++Count >= 24)
			{
				Count = 0;
			}
		}

		// Restore surface texture data
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
			if ((LostDeviceBackup[0].Format == Format || GetFailoverFormat(LostDeviceBackup[0].Format) == Format || LostDeviceBackup[0].Format == GetFailoverFormat(Format)) &&
				LostDeviceBackup[0].Width == surface.Width && LostDeviceBackup[0].Height == surface.Height)
			{
				for (UINT Level = 0; Level < LostDeviceBackup.size(); Level++)
				{
					D3DLOCKED_RECT LockRect = {};
					if (FAILED(LockD3d9Surface(&LockRect, nullptr, 0, Level)))
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to restore surface data!");
						break;
					}

					Logging::LogDebug() << __FUNCTION__ << " Restoring Direct3D9 texture surface data: " << Format;

					D3DSURFACE_DESC Desc = {};
					if (FAILED(surface.Surface ? surface.Surface->GetDesc(&Desc) : surface.Texture->GetLevelDesc(GetD3d9MipMapLevel(Level), &Desc)))
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get surface desc!");
						break;
					}

					if (LostDeviceBackup[Level].Format == Desc.Format && LostDeviceBackup[Level].Width == Desc.Width && LostDeviceBackup[Level].Height == Desc.Height)
					{
						size_t size = GetSurfaceSize(Desc.Format, Desc.Width, Desc.Height, LockRect.Pitch);

						if (size == LostDeviceBackup[Level].Bits.size())
						{
							memcpy(LockRect.pBits, LostDeviceBackup[Level].Bits.data(), size);
						}
						else
						{
							BYTE* pSrcSurface = LostDeviceBackup[Level].Bits.data();
							BYTE* pDestSurface = (BYTE*)LockRect.pBits;
							DWORD MinPitchSize = min((UINT)LockRect.Pitch, LostDeviceBackup[Level].Pitch);

							for (UINT x = 0; x < surface.Height; x++)
							{
								memcpy(pDestSurface, pSrcSurface, MinPitchSize);

								pSrcSurface += LostDeviceBackup[Level].Pitch;
								pDestSurface += LockRect.Pitch;
							}
						}

						RestoreData = true;
						surface.HasData = true;
					}
					else
					{
						LOG_LIMIT(100, __FUNCTION__ << " Warning: restore backup surface data mismatch! For Level: " << Level << " " <<
							LostDeviceBackup[Level].Format << " -> " << Format << " " << LostDeviceBackup[Level].Width << "x" << LostDeviceBackup[Level].Height << " -> " <<
							surface.Width << "x" << surface.Height << " " << LostDeviceBackup[Level].Pitch << " - > " << LockRect.Pitch);
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
					LostDeviceBackup[0].Format << " -> " << Format << " " << LostDeviceBackup[0].Width << "x" << LostDeviceBackup[0].Height << " -> " <<
					surface.Width << "x" << surface.Height);
			}
		}

		// Copy surface to display texture
		if (RestoreData && PrimaryDisplayTexture)
		{
			IDirect3DSurface9* pSrcSurfaceD9 = Get3DSurface();
			if (pSrcSurfaceD9)
			{
				ComPtr<IDirect3DSurface9> pPrimaryDisplaySurfaceD9;
				if (SUCCEEDED(PrimaryDisplayTexture->GetSurfaceLevel(0, pPrimaryDisplaySurfaceD9.GetAddressOf())))
				{
					D3DXLoadSurfaceFromSurface(pPrimaryDisplaySurfaceD9.Get(), nullptr, nullptr, pSrcSurfaceD9, nullptr, nullptr, D3DX_FILTER_NONE, 0);
				}
			}
		}

		// Data is no longer needed
		LostDeviceBackup.clear();
	}

	// Delete emulatd surface if not needed
	if (!CreateSurfaceEmulated && IsUsingEmulation())
	{
		ReleaseDCSurface();
	}

	return hr;
}

bool m_IDirectDrawSurfaceX::DoesDCMatch(EMUSURFACE* pEmuSurface) const
{
	if (!pEmuSurface || !pEmuSurface->DC || !pEmuSurface->pBits)
	{
		return false;
	}

	// Adjust Width to be byte-aligned
	DWORD Width = GetByteAlignedWidth(surfaceDesc2.dwWidth, surface.BitCount);
	DWORD Height = surfaceDesc2.dwHeight;
	DWORD Pitch = ComputePitch(surface.Format, Width, surface.BitCount);

	if (pEmuSurface->bmi->bmiHeader.biWidth == (LONG)Width &&
		pEmuSurface->bmi->bmiHeader.biHeight == -(LONG)Height &&
		pEmuSurface->bmi->bmiHeader.biBitCount == (WORD)surface.BitCount &&
		pEmuSurface->Format == surface.Format &&
		pEmuSurface->Pitch == Pitch)
	{
		return true;
	}

	return false;
}

void m_IDirectDrawSurfaceX::SetEmulationGameDC()
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

void m_IDirectDrawSurfaceX::UnsetEmulationGameDC()
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
	ScopedDDCriticalSection ThreadLockDD;

	// Check if color masks are needed
	bool ColorMaskReq = ((surface.BitCount == 16 || surface.BitCount == 24 || surface.BitCount == 32) &&									// Only valid when used with 16 bit, 24 bit and 32 bit surfaces
		(surfaceDesc2.ddpfPixelFormat.dwRBitMask || surfaceDesc2.ddpfPixelFormat.dwGBitMask || surfaceDesc2.ddpfPixelFormat.dwBBitMask));	// Check to make sure the masks actually exist

	// Adjust Width to be byte-aligned
	DWORD Width = GetByteAlignedWidth(surfaceDesc2.dwWidth, surface.BitCount);
	DWORD Height = surfaceDesc2.dwHeight;
	DWORD Pitch = ComputePitch(surface.Format, Width, surface.BitCount);

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
				memorySurfaces.push_back(surface.emu);
				surface.emu = nullptr;
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
	surface.emu->bmi->bmiHeader.biHeight = -((LONG)Height + (LONG)ExtraDataBufferSize);
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
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set bmi colors! " << surface.Format << " " << surface.BitCount);
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
	surface.emu->Pitch = Pitch;
	surface.emu->Size = Height * Pitch;

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
		ScopedDDCriticalSection ThreadLockDD;

		lpAttachedSurfaceX->ReleaseD9Surface(false, false);
	}
	// Set depth stencil
	if (ddrawParent->GetRenderTargetSurface() == this)
	{
		ddrawParent->SetDepthStencilSurface(lpAttachedSurfaceX);
	}
}

void m_IDirectDrawSurfaceX::UpdateSurfaceDesc()
{
	bool IsChanged = false;
	if (SUCCEEDED(CheckInterface(__FUNCTION__, false, false, false)) &&
		((surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT)) != (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT) ||
		((surfaceDesc2.dwFlags & DDSD_REFRESHRATE) && !surfaceDesc2.dwRefreshRate)))
	{
		// Get resolution
		DWORD Width, Height, RefreshRate, BPP;
		ddrawParent->GetSurfaceDisplay(Width, Height, BPP, RefreshRate);

		// Set Height and Width
		if (Width && Height &&
			(surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT)) != (DDSD_WIDTH | DDSD_HEIGHT))
		{
			ResetDisplayFlags |= DDSD_WIDTH | DDSD_HEIGHT;
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
			ResetDisplayFlags |= DDSD_PIXELFORMAT;
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
	// Remove surface memory pointer
	if (!surface.UsingSurfaceMemory)
	{
		surfaceDesc2.dwFlags &= ~DDSD_LPSURFACE;
		surfaceDesc2.lpSurface = nullptr;
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
		!(surfaceDesc2.dwFlags & DDSD_LINEARSIZE) && !(surfaceDesc2.dwFlags & DDSD_PITCH))
	{
		DWORD Pitch = ComputePitch(GetDisplayFormat(surfaceDesc2.ddpfPixelFormat), surfaceDesc2.dwWidth, surfaceDesc2.dwHeight);
		if (Pitch)
		{
			surfaceDesc2.dwFlags |= DDSD_PITCH;
			surfaceDesc2.lPitch = Pitch;
		}
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
			ScopedDDCriticalSection ThreadLockDD;

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
			CreateD9Surface();
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
			CreateD9Surface();
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

void m_IDirectDrawSurfaceX::ReleaseD9AuxiliarySurfaces()
{
	// Release d3d9 shadow surface when surface is released
	if (surface.Shadow)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 surface";
		ULONG ref = surface.Shadow->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'surface.Shadow' " << ref;
		}
		surface.Shadow = nullptr;
	}

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

	// Release d3d9 palette surface texture
	if (primary.PaletteTexture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 palette texture surface";
		ULONG ref = primary.PaletteTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'paletteTexture' " << ref;
		}
		primary.PaletteTexture = nullptr;
	}

	// Release d3d9 color keyed surface texture
	if (surface.DrawTexture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 DrawTexture surface";
		ULONG ref = surface.DrawTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'DrawTexture' " << ref;
		}
		surface.DrawTexture = nullptr;
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

	// Set flags
	RecreateAuxiliarySurfaces = true;
	surface.RecreateAuxiliarySurfaces = true;
}

void m_IDirectDrawSurfaceX::ReleaseD9Surface(bool BackupData, bool ResetSurface)
{
	// To avoid threadlock, cannot have any critical sections in this function or any sub-functions

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
	IsPreparingDC = false;

	// Restore DC
	UnsetEmulationGameDC();

	// Unlock surface (before releasing)
	if (IsSurfaceLocked())
	{
		UnLockD3d9Surface(0);
		IsLocked = false;
	}
	IsLocking = false;
	LockedWithID = 0;
	LastLock.LockedRect = {};
	IsInBlt = false;
	IsInBltBatch = false;

	// Backup d3d9 surface texture
	if (BackupData)
	{
		if (surface.HasData && (surface.Surface || surface.Texture) && !(surface.Usage & D3DUSAGE_RENDERTARGET) && !IsDepthStencil() && (!ResetSurface || IsD9UsingVideoMemory()))
		{
			IsSurfaceLost = true;

			if (!IsUsingEmulation() && LostDeviceBackup.empty())
			{
				for (UINT Level = 0; Level < ((IsMipMapAutogen() || !MaxMipMapLevel) ? 1 : MaxMipMapLevel); Level++)
				{
					D3DLOCKED_RECT LockRect = {};
					if (FAILED(LockD3d9Surface(&LockRect, nullptr, D3DLOCK_READONLY, Level)))
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to backup surface data!");
						break;
					}

					D3DSURFACE_DESC Desc = {};
					if (FAILED(surface.Surface ? surface.Surface->GetDesc(&Desc) : surface.Texture->GetLevelDesc(GetD3d9MipMapLevel(Level), &Desc)))
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get surface desc!");
						break;
					}

					Logging::LogDebug() << __FUNCTION__ << " Storing Direct3D9 texture surface data: " << Desc.Format;

					size_t size = GetSurfaceSize(Desc.Format, Desc.Width, Desc.Height, LockRect.Pitch);

					if (size)
					{
						DDBACKUP entry;
						LostDeviceBackup.push_back(entry);
						LostDeviceBackup[Level].Format = Desc.Format;
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

	ReleaseD9AuxiliarySurfaces();

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
		ClearUnusedValues(surfaceDesc2);
	}
	if (surfaceDesc2.dwFlags & DDSD_REFRESHRATE)
	{
		surfaceDesc2.dwRefreshRate = 0;
	}
}

void m_IDirectDrawSurfaceX::ReleaseDCSurface()
{
	if (surface.emu)
	{
		ScopedDDCriticalSection ThreadLockDD;

		if (!ShareEmulatedMemory || !IsUsingEmulation())
		{
			DeleteEmulatedMemory(&surface.emu);
		}
		else
		{
			memorySurfaces.push_back(surface.emu);
			surface.emu = nullptr;
		}
	}
}

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

void m_IDirectDrawSurfaceX::ResetSurfaceDisplay()
{
	if (ResetDisplayFlags)
	{
		ReleaseD9Surface(true, false);
	}
}

bool m_IDirectDrawSurfaceX::CheckCoordinates(RECT& OutRect, LPRECT lpInRect, LPDDSURFACEDESC2 lpDDSurfaceDesc2)
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

void m_IDirectDrawSurfaceX::LockEmuLock(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc)
{
	// Only works if entire surface is locked
	if (!lpDDSurfaceDesc || !lpDDSurfaceDesc->lPitch || (lpDestRect && lpDestRect->top != 0 && lpDestRect->left != 0))
	{
		return;
	}

	DWORD BBP = surface.BitCount;
	LONG NewPitch = (BBP / 8) * lpDDSurfaceDesc->dwWidth;

	bool LockOffPlain = Config.DdrawEmulateLock;
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
		size_t Size = NewPitch * (lpDDSurfaceDesc->dwHeight + ExtraDataBufferSize);
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

void m_IDirectDrawSurfaceX::RestoreScanlines(LASTLOCK& LLock) const
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

void m_IDirectDrawSurfaceX::RemoveScanlines(LASTLOCK& LLock) const
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

HRESULT m_IDirectDrawSurfaceX::LockEmulatedSurface(D3DLOCKED_RECT* pLockedRect, LPRECT lpDestRect) const
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

void m_IDirectDrawSurfaceX::SetDirtyFlag(DWORD MipMapLevel)
{
	if (MipMapLevel == 0)
	{
		if (IsPrimarySurface() && ddrawParent && !ddrawParent->IsInScene())
		{
			dirtyFlag = true;
		}
		surface.IsDirtyFlag = true;
		surface.HasData = true;
		surface.IsDrawTextureDirty = true;
		IsMipMapReadyToUse = (IsMipMapAutogen() || MipMaps.empty());

		LostDeviceBackup.clear();

		// Update Uniqueness Value
		ChangeUniquenessValue();
	}
	// Mark mipmap data flag
	if (MipMaps.size())
	{
		if (MipMapLevel && MipMapLevel <= MipMaps.size())
		{
			MipMaps[MipMapLevel - 1].UniquenessValue = UniquenessValue;
		}
		CheckMipMapLevelGen();
	}
}

void m_IDirectDrawSurfaceX::ClearDirtyFlags()
{
	// Reset dirty flag
	dirtyFlag = false;
	surface.IsDirtyFlag = false;

	// Reset scene ready
	SceneReady = false;
}

bool m_IDirectDrawSurfaceX::CheckRectforSkipScene(RECT& DestRect)
{
	bool isSingleLine = (DestRect.bottom - DestRect.top == 1);	// Only handles horizontal lines at this point

	return Config.DdrawRemoveInterlacing ? isSingleLine : false;
}

void m_IDirectDrawSurfaceX::BeginWritePresent(bool IsSkipScene)
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

void m_IDirectDrawSurfaceX::EndWritePresent(LPRECT lpDestRect, bool WriteToWindow, bool FullPresent, bool IsSkipScene)
{
	// Handle overlays
	PresentOverlay(lpDestRect);

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

void m_IDirectDrawSurfaceX::EndWriteSyncSurfaces(LPRECT lpDestRect)
{
	// Copy emulated surface to real surface
	if (IsUsingEmulation())
	{
		CopyFromEmulatedSurface(lpDestRect);
	}
}

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
	if ((surfaceDesc2.dwFlags & DDSD_BACKBUFFERCOUNT) && surfaceDesc2.dwBackBufferCount)
	{
		DDSURFACEDESC2 Desc2 = surfaceDesc2;
		Desc2.ddsCaps.dwCaps4 &= ~(DDSCAPS4_CREATESURFACE);	// Clear surface creation flag
		Desc2.dwBackBufferCount--;
		Desc2.ddsCaps.dwCaps |= DDSCAPS_BACKBUFFER;
		Desc2.ddsCaps.dwCaps &= ~(DDSCAPS_VISIBLE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FRONTBUFFER);
		Desc2.ddsCaps.dwCaps4 |= DDSCAPS4_COMPLEXCHILD;

		if (surfaceDesc2.ddsCaps.dwCaps4 & DDSCAPS4_CREATESURFACE)
		{
			ComplexRoot = true;
			Desc2.dwReserved = (DWORD)this;
		}

		// Create complex surfaces
		BackBufferInterface = std::make_unique<m_IDirectDrawSurfaceX>(ddrawParent, DirectXVersion, &Desc2);

		m_IDirectDrawSurfaceX* attachedSurface = BackBufferInterface.get();

		AddAttachedSurfaceToMap(attachedSurface, false, DirectXVersion, 1);
	}

	// Set flags for complex child surface
	if (surfaceDesc2.ddsCaps.dwCaps4 & DDSCAPS4_COMPLEXCHILD)
	{
		ComplexChild = true;
		surfaceDesc2.dwFlags &= ~DDSD_BACKBUFFERCOUNT;
		surfaceDesc2.dwBackBufferCount = 0;

		// Add first surface as attached surface to the last surface in a surface chain
		if (surfaceDesc2.dwReserved)
		{
			m_IDirectDrawSurfaceX* attachedSurface = (m_IDirectDrawSurfaceX*)surfaceDesc2.dwReserved;

			// Check if source Surface exists add to surface map
			if (ddrawParent && ddrawParent->DoesSurfaceExist(attachedSurface))
			{
				AddAttachedSurfaceToMap(attachedSurface, false, DirectXVersion, 0);
			}
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
		MaxMipMapLevel = MipMapLevelCount - 1;
		surfaceDesc2.dwMipMapCount = MaxMipMapLevel + 1;
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
	if (!(surfaceDesc2.dwFlags & DDSD_LPSURFACE))
	{
		surfaceDesc2.dwFlags &= ~DDSD_PITCH;
		surfaceDesc2.lPitch = 0;
	}
	surface.UsingSurfaceMemory = ((surfaceDesc2.dwFlags & DDSD_LPSURFACE) && surfaceDesc2.lpSurface);

	// Clear flags used in creating a surface structure
	surfaceDesc2.ddsCaps.dwCaps4 = 0;
	surfaceDesc2.dwReserved = 0;

	// Clear unused values
	ClearUnusedValues(surfaceDesc2);
}

void m_IDirectDrawSurfaceX::AddAttachedSurfaceToMap(m_IDirectDrawSurfaceX* lpSurfaceX, bool MarkAttached, DWORD DxVersion, DWORD RefCount)
{
	if (!lpSurfaceX)
	{
		return;
	}

	// Store surface
	AttachedSurfaceMap[++MapKey].pSurface = lpSurfaceX;
	AttachedSurfaceMap[MapKey].isAttachedSurfaceAdded = MarkAttached;

	AttachedSurfaceMap[MapKey].DxVersion = DxVersion;
	AttachedSurfaceMap[MapKey].RefCount = RefCount;

	if (RefCount == 1)
	{
		lpSurfaceX->AddRef(DxVersion);
	}
}

void m_IDirectDrawSurfaceX::RemoveAttachedSurfaceFromMap(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	auto it = std::find_if(AttachedSurfaceMap.begin(), AttachedSurfaceMap.end(),
		[=](auto Map) -> bool { return Map.second.pSurface == lpSurfaceX; });

	if (it != std::end(AttachedSurfaceMap))
	{
		if (it->second.RefCount == 1)
		{
			it->second.RefCount = 0;		// Clear ref count before release
			it->second.pSurface->Release(it->second.DxVersion);
		}
		AttachedSurfaceMap.erase(it);
	}
}

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

		bool CanUseMemSet = surface.BitCount == 8 ? true :
			surface.BitCount == 12 ||
			surface.BitCount == 16 ? (dwFillColor & 0xFF) == ((dwFillColor >> 8) & 0xFF) :
			surface.BitCount == 24 ? (dwFillColor & 0xFF) == ((dwFillColor >> 8) & 0xFF) &&
									 (dwFillColor & 0xFF) == ((dwFillColor >> 16) & 0xFF) :
			surface.BitCount == 32 ? (dwFillColor & 0xFF) == ((dwFillColor >> 8) & 0xFF) &&
									 (dwFillColor & 0xFF) == ((dwFillColor >> 16) & 0xFF) &&
									 (dwFillColor & 0xFF) == ((dwFillColor >> 24) & 0xFF) : false;

		if (FillWidth == (LONG)surfaceDesc2.dwWidth && CanUseMemSet)
		{
			memset(DestLockRect.pBits, dwFillColor, DestLockRect.Pitch * FillHeight);
		}
		else if (surface.BitCount == 8 || (surface.BitCount == 12 && FillWidth % 2 == 0) || surface.BitCount == 16 || surface.BitCount == 24 || surface.BitCount == 32)
		{
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
			if ((surface.BitCount == 8 || surface.BitCount == 16 || surface.BitCount == 32) &&								// Check bit count
				(FillWidth % (sizeof(DWORD) / ByteCount) == 0) && reinterpret_cast<uintptr_t>(DestLockRect.pBits) % sizeof(DWORD) == 0)	// Check for aligned width and memory
			{
				DWORD Color = (surface.BitCount == 8) ? (dwFillColor & 0xFF) * 0x01010101 :
					(surface.BitCount == 16) ? (dwFillColor & 0xFFFF) * 0x00010001 : dwFillColor;

				DWORD* DestBuffer = reinterpret_cast<DWORD*>(DestLockRect.pBits);
				LONG Iterations = FillWidth / (sizeof(DWORD) / ByteCount);

				for (LONG x = 0; x < Iterations; ++x)
				{
					*DestBuffer++ = Color;
				}
			}
			else
			{
				BYTE* SrcColor = reinterpret_cast<BYTE*>(&dwFillColor);
				BYTE* DestBuffer = reinterpret_cast<BYTE*>(DestLockRect.pBits);

				for (LONG x = 0; x < FillWidth; ++x)
				{
					BYTE* Color = SrcColor;
					for (DWORD y = 0; y < ByteCount; ++y)
					{
						*DestBuffer++ = *Color;
						Color++;
					}
				}
			}

			// Fill rest of surface rect using the first line as a template
			BYTE* SrcBuffer = (BYTE*)DestLockRect.pBits;
			BYTE* DestBuffer = (BYTE*)DestLockRect.pBits + DestLockRect.Pitch;
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

HRESULT m_IDirectDrawSurfaceX::Load(LPDIRECTDRAWSURFACE7 lpDestTex, LPPOINT lpDestPoint, LPDIRECTDRAWSURFACE7 lpSrcTex, LPRECT lprcSrcRect, DWORD dwFlags)
{
	if (!lpDestTex || !lpSrcTex)
	{
		return  DDERR_INVALIDPARAMS;
	}

	// ToDo: support the following dwFlags: 
	// DDSCAPS2_CUBEMAP_ALLFACES - All faces should be loaded with the image data within the source texture.
	// DDSCAPS2_CUBEMAP_NEGATIVEX, DDSCAPS2_CUBEMAP_NEGATIVEY, or DDSCAPS2_CUBEMAP_NEGATIVEZ
	//     The negative x, y, or z faces should receive the image data.
	// DDSCAPS2_CUBEMAP_POSITIVEX, DDSCAPS2_CUBEMAP_POSITIVEY, or DDSCAPS2_CUBEMAP_POSITIVEZ
	//     The positive x, y, or z faces should receive the image data.

	if (dwFlags)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: flags not supported. dwFlags: " << Logging::hex(dwFlags));
	}

	if (!lprcSrcRect && (!lpDestPoint || (lpDestPoint && lpDestPoint->x == 0 && lpDestPoint->y == 0)))
	{
		return lpDestTex->Blt(nullptr, lpSrcTex, nullptr, 0, nullptr);
	}
	else
	{
		// Get source rect
		RECT SrcRect = {};
		if (lprcSrcRect)
		{
			SrcRect = *lprcSrcRect;
		}
		else
		{
			DDSURFACEDESC2 Desc2 = {};
			Desc2.dwSize = sizeof(DDSURFACEDESC2);
			lpSrcTex->GetSurfaceDesc(&Desc2);

			if ((Desc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT)) != (DDSD_WIDTH | DDSD_HEIGHT))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: rect size doesn't match!");
				return DDERR_GENERIC;
			}

			SrcRect = { 0, 0, (LONG)Desc2.dwWidth, (LONG)Desc2.dwHeight };
		}

		// Get destination point
		POINT DestPoint = {};
		if (lpDestPoint)
		{
			DestPoint = *lpDestPoint;
		}

		// Get destination rect
		RECT DestRect = {
			DestPoint.x,									// left
			DestPoint.y,									// top
			DestPoint.x + (SrcRect.right - SrcRect.left),	// right
			DestPoint.y + (SrcRect.bottom - SrcRect.top),	// bottom
		};

		return lpDestTex->Blt(&DestRect, lpSrcTex, &SrcRect, 0, nullptr);
	}
}

HRESULT m_IDirectDrawSurfaceX::SaveSurfaceToFile(const char *filename, D3DXIMAGE_FILEFORMAT format)
{
	ComPtr<ID3DXBuffer> pDestBuf;
	HRESULT hr = D3DXSaveSurfaceToFileInMemory(pDestBuf.GetAddressOf(), format, Get3DSurface(), nullptr, nullptr);

	if (SUCCEEDED(hr))
	{
		// Save the buffer to a file
		std::ofstream outFile(filename, std::ios::binary | std::ios::out);
		if (outFile.is_open())
		{
			outFile.write((const char*)pDestBuf->GetBufferPointer(), pDestBuf->GetBufferSize());
			outFile.close();
		}
	}

	return hr;
}

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
			(pSourceSurface->surface.Type == surface.Type || (pSourceSurface->surface.Type == D3DTYPE_OFFPLAINSURFACE && (surface.Usage & D3DUSAGE_RENDERTARGET))) &&
			(!IsStretchRect || (this != pSourceSurface && !ISDXTEX(SrcFormat) && !ISDXTEX(DestFormat) && (surface.Usage & D3DUSAGE_RENDERTARGET))) &&
			(surface.Type != D3DTYPE_DEPTHSTENCIL || !ddrawParent->IsInScene()) &&
			(surface.Type != D3DTYPE_DEPTHSTENCIL || (surface.Type == D3DTYPE_DEPTHSTENCIL &&
				!SrcRect.left && SrcRect.left == SrcRect.top && SrcRect.top == DestRect.left && DestRect.left == DestRect.top && DestRect.top)) &&
			(surface.Type != D3DTYPE_TEXTURE) &&
			(!pSourceSurface->IsPalette() && !IsPalette()) &&
			!IsMirrorLeftRight && !IsMirrorUpDown && !IsColorKey)
		{
			IDirect3DSurface9* pSourceSurfaceD9 = pSourceSurface->Get3DMipMapSurface(SrcMipMapLevel);
			IDirect3DSurface9* pDestSurfaceD9 = Get3DMipMapSurface(MipMapLevel);

			if (pSourceSurfaceD9 && pDestSurfaceD9)
			{
				if (surface.Type == D3DTYPE_DEPTHSTENCIL)
				{
					hr = (*d3d9Device)->StretchRect(pSourceSurfaceD9, nullptr, pDestSurfaceD9, nullptr, D3DTEXF_NONE);
				}
				else
				{
					hr = (*d3d9Device)->StretchRect(pSourceSurfaceD9, &SrcRect, pDestSurfaceD9, &DestRect, Filter);
				}

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
		if ((surface.Usage & D3DUSAGE_RENDERTARGET) && !IsUsingShadowSurface())
		{
			SetRenderTargetShadow();
		}

		// Decode DirectX textures and FourCCs
		if ((FormatMismatch && !IsUsingEmulation()) ||
			(!IsPixelFormatRGB(pSourceSurface->surfaceDesc2.ddpfPixelFormat) && !IsPixelFormatPalette(pSourceSurface->surfaceDesc2.ddpfPixelFormat)))
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
					LOG_LIMIT(100, __FUNCTION__ << " Error: could not decode source texture. " << (D3DERR)hr << " " << SrcFormat << "->" << DestFormat);
				}
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get source or destination surface level: " << pSourceSurfaceD9 << "->" << pDestSurfaceD9);
			}

			pSourceSurface->Release3DMipMapSurface(pSourceSurfaceD9, SrcMipMapLevel);
			Release3DMipMapSurface(pDestSurfaceD9, MipMapLevel);

			if (SUCCEEDED(hr))
			{
				break;
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

		// Simple copy with ColorKey and Mirroring
		if (!IsStretchRect)
		{
			switch (ByteCount)
			{
			case 1:
				SimpleColorKeyCopy<BYTE>((BYTE)ColorKey, SrcBuffer, DestBuffer, SrcLockRect.Pitch, DestPitch, DestRectWidth, DestRectHeight, IsColorKey, IsMirrorLeftRight);
				break;
			case 2:
				SimpleColorKeyCopy<WORD>((WORD)ColorKey, SrcBuffer, DestBuffer, SrcLockRect.Pitch, DestPitch, DestRectWidth, DestRectHeight, IsColorKey, IsMirrorLeftRight);
				break;
			case 3:
				SimpleColorKeyCopy<TRIBYTE>(*reinterpret_cast<TRIBYTE*>(&ColorKey), SrcBuffer, DestBuffer, SrcLockRect.Pitch, DestPitch, DestRectWidth, DestRectHeight, IsColorKey, IsMirrorLeftRight);
				break;
			case 4:
				SimpleColorKeyCopy<DWORD>((DWORD)ColorKey, SrcBuffer, DestBuffer, SrcLockRect.Pitch, DestPitch, DestRectWidth, DestRectHeight, IsColorKey, IsMirrorLeftRight);
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
			ComplexCopy<TRIBYTE>(*reinterpret_cast<TRIBYTE*>(&ColorKey), SrcLockRect, DestLockRect, SrcRectWidth, SrcRectHeight, DestRectWidth, DestRectHeight, IsColorKey, IsMirrorUpDown, IsMirrorLeftRight);
			break;
		case 4:
			ComplexCopy<DWORD>((DWORD)ColorKey, SrcLockRect, DestLockRect, SrcRectWidth, SrcRectHeight, DestRectWidth, DestRectHeight, IsColorKey, IsMirrorUpDown, IsMirrorLeftRight);
		}
		hr = DD_OK;
		break;

	} while (false);

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
			RemoveScanlines(LLock);
		}
		else if (UnlockDest)
		{
			LLock.LockedRect = DestLockRect;
			RemoveScanlines(LLock);
		}
	}

	// Unlock surfaces if needed
	if (UnlockSrc)
	{
		pSourceSurface->IsUsingEmulation() ? DD_OK : pSourceSurface->UnLockD3d9Surface(SrcMipMapLevel);
	}
	if (UnlockDest)
	{
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
	ComPtr<IDirect3DSurface9> DestSurface;
	if (!SrcSurface || FAILED(surface.DrawTexture->GetSurfaceLevel(0, DestSurface.GetAddressOf())))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get surface texture!");
		return DDERR_GENERIC;
	}

	// Get color key
	DWORD ColorKey = 0;
	if (surfaceDesc2.dwFlags & DDSD_CKSRCBLT)
	{
		if (IsPalette())
		{
			UpdatePaletteData();
			if (surface.PaletteEntryArray)
			{
				PALETTEENTRY PaletteEntry = surface.PaletteEntryArray[surfaceDesc2.ddckCKSrcBlt.dwColorSpaceLowValue & 0xFF];
				ColorKey = D3DCOLOR_ARGB(PaletteEntry.peFlags, PaletteEntry.peRed, PaletteEntry.peGreen, PaletteEntry.peBlue);
			}
		}
		else if (surfaceDesc2.ddpfPixelFormat.dwRGBBitCount)
		{
			ColorKey = GetARGBColorKey(surfaceDesc2.ddckCKSrcBlt.dwColorSpaceLowValue, surfaceDesc2.ddpfPixelFormat);
		}
	}

	if (FAILED(D3DXLoadSurfaceFromSurface(DestSurface.Get(), nullptr, lpDestRect, SrcSurface, surface.PaletteEntryArray, lpDestRect, D3DX_FILTER_NONE, ColorKey)))
	{
		Logging::Log() << __FUNCTION__ " Error: failed to copy data from surface: " << surface.Format << " " << (void*)ColorKey << " " << lpDestRect;

		return DDERR_GENERIC;
	}

	surface.IsDrawTextureDirty = false;

	return DD_OK;
}

HRESULT m_IDirectDrawSurfaceX::LoadSurfaceFromMemory(LPDIRECT3DSURFACE9 pDestSurface, const RECT& Rect, LPCVOID pSrcMemory, D3DFORMAT SrcFormat, UINT SrcPitch)
{
	if (!pDestSurface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: destination surface is NULL!");
		return DDERR_GENERIC;
	}

	if (!pSrcMemory)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: source memory is NULL!");
		return DDERR_GENERIC;
	}

	// Get actual surface format
	D3DSURFACE_DESC Desc = {};
	if (FAILED(pDestSurface->GetDesc(&Desc)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get surface description!");
		return DDERR_GENERIC;
	}

	// Ensure bit counts match for manual copy
	const UINT SrcBitCount = GetBitCount(SrcFormat);
	const UINT DestBitCount = GetBitCount(Desc.Format);

	if (SrcBitCount == DestBitCount && (SrcFormat == Desc.Format || GetFailoverFormat(SrcFormat) == Desc.Format))
	{
		// Lock destination surface
		D3DLOCKED_RECT LockedRect = {};
		if (FAILED(pDestSurface->LockRect(&LockedRect, nullptr, 0)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock destination surface!");
			return DDERR_GENERIC;
		}

		// Calculate bytes per pixel
		const LONG BytesPerPixel = SrcBitCount / 8;

		// Validate rectangle dimensions
		if (Rect.left < 0 || Rect.top < 0 ||
			Rect.right >(LONG)Desc.Width || Rect.bottom >(LONG)Desc.Height)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid rectangle dimensions!");
			pDestSurface->UnlockRect();
			return DDERR_INVALIDRECT;
		}

		// Calculate source and destination buffers
		const BYTE* SrcBuffer = (const BYTE*)pSrcMemory + (SrcPitch * Rect.top) + (BytesPerPixel * Rect.left);
		BYTE* DestBuffer = (BYTE*)LockedRect.pBits + (LockedRect.Pitch * Rect.top) + (BytesPerPixel * Rect.left);

		// Check dest buffer
		if (!DestBuffer || !SrcBuffer)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: source or destination buffer is null!");
			pDestSurface->UnlockRect();
			return DDERR_GENERIC;
		}

		// Calculate copy pitch and height
		const LONG CopyPitch = (Rect.right - Rect.left) == (LONG)Desc.Width
			? min(LockedRect.Pitch, (INT)SrcPitch)
			: (Rect.right - Rect.left) * BytesPerPixel;
		const LONG CopyHeight = Rect.bottom - Rect.top;

		// Copy surface data row by row
		for (LONG row = 0; row < CopyHeight; ++row)
		{
			memcpy(DestBuffer, SrcBuffer, CopyPitch);
			SrcBuffer += SrcPitch;
			DestBuffer += LockedRect.Pitch;
		}

		// Unlock destination surface
		pDestSurface->UnlockRect();
	}
	else
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: using slower D3DXLoadSurfaceFromMemory for copy: " << SrcFormat << "->" << Desc.Format);

		// Use D3DXLoadSurfaceFromMemory for format conversion
		if (FAILED(D3DXLoadSurfaceFromMemory(pDestSurface, nullptr, &Rect, pSrcMemory, SrcFormat, SrcPitch, nullptr, &Rect, D3DX_FILTER_NONE, 0)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not copy surface using D3DXLoadSurfaceFromMemory!");
			return DDERR_GENERIC;
		}
	}

	return DD_OK;
}

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

	// LoadSurfaceFromMemory to copy to the surface
	if (FAILED(LoadSurfaceFromMemory(pDestSurfaceD9, DestRect, surface.emu->pBits, (surface.Format == D3DFMT_P8) ? D3DFMT_L8 : surface.Format, surface.emu->Pitch)))
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

HRESULT m_IDirectDrawSurfaceX::CopyEmulatedPaletteSurface(LPRECT lpDestRect)
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

	ScopedDDCriticalSection ThreadLockDD;

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

		// Use LoadSurfaceFromMemory to copy to the surface
		if (FAILED(LoadSurfaceFromMemory(surface.DisplayContext, DestRect, surface.emu->pBits, D3DFMT_P8, surface.emu->Pitch)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: could not copy palette display texture: " << surface.Format);
			hr = DDERR_GENERIC;
			break;
		}

		// Reset palette texture dirty flag
		surface.IsPaletteDirty = false;

	} while (false);

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

	// Check for iconic window
	if (IsIconic(hWnd) || IsIconic(DDraw_hWnd))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Window is iconic!");
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

	// Check for iconic window
	if (IsIconic(hWnd) || IsIconic(DDraw_hWnd))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Window is iconic!");
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

	// Check for iconic window
	if (IsIconic(hWnd))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Window is iconic!");
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
	ComPtr<IDirect3DSurface9> pDestSurfaceD9;
	if (FAILED(PrimaryDisplayTexture->GetSurfaceLevel(0, pDestSurfaceD9.GetAddressOf())))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get destination surface!");
		return DDERR_GENERIC;
	}

	// Copy surface
	HRESULT hr = DDERR_GENERIC;
	if (IsD9UsingVideoMemory())
	{
		hr = (*d3d9Device)->StretchRect(pSourceSurfaceD9, &Rect, pDestSurfaceD9.Get(), &MapClient, D3DTEXF_NONE);
	}
	else
	{
		hr = (*d3d9Device)->UpdateSurface(pSourceSurfaceD9, &Rect, pDestSurfaceD9.Get(), (LPPOINT)&MapClient);
	}

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
	const PALETTEENTRY* NewPaletteEntry = nullptr;
	const RGBQUAD* NewRGBPalette = nullptr;

	ScopedDDCriticalSection ThreadLockDD;

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
		ComPtr<IDirect3DSurface9> paletteSurface;
		if (SUCCEEDED(primary.PaletteTexture->GetSurfaceLevel(0, paletteSurface.GetAddressOf())))
		{
			// Use LoadSurfaceFromMemory to copy to the surface
			RECT Rect = { 0, 0, MaxPaletteSize, 1 };
			if (FAILED(LoadSurfaceFromMemory(paletteSurface.Get(), Rect, NewRGBPalette, D3DFMT_X8R8G8B8, MaxPaletteSize * sizeof(D3DCOLOR))))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: could not full palette textur!");
			}
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

HRESULT m_IDirectDrawSurfaceX::CheckBackBufferForFlip(m_IDirectDrawSurfaceX* lpTargetSurface)
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

void m_IDirectDrawSurfaceX::FixTextureFlags(LPDDSURFACEDESC2 lpDDSurfaceDesc2)
{
	if (lpDDSurfaceDesc2)
	{
		if (lpDDSurfaceDesc2->dwFlags & DDSD_PITCH)
		{
			lpDDSurfaceDesc2->dwFlags |= DDSD_LINEARSIZE;
		}
		lpDDSurfaceDesc2->dwFlags &= ~(DDSD_PITCH | DDSD_LPSURFACE);
	}
}

HRESULT m_IDirectDrawSurfaceX::LockD3d9Surface(D3DLOCKED_RECT* pLockedRect, RECT* pRect, DWORD Flags, DWORD MipMapLevel)
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
		HRESULT hr = surface.Shadow->LockRect(pLockedRect, pRect, Flags);
		if (FAILED(hr) && (Flags & D3DLOCK_NOSYSLOCK))
		{
			hr = surface.Shadow->LockRect(pLockedRect, pRect, Flags & ~D3DLOCK_NOSYSLOCK);
		}
		return hr;
	}
	// Lock 3D surface
	else if (surface.Surface)
	{
		HRESULT hr = surface.Surface->LockRect(pLockedRect, pRect, Flags);
		if (FAILED(hr) && (Flags & D3DLOCK_NOSYSLOCK))
		{
			hr = surface.Surface->LockRect(pLockedRect, pRect, Flags & ~D3DLOCK_NOSYSLOCK);
		}
		return hr;
	}
	// Lock surface texture
	else if (surface.Texture)
	{
		HRESULT hr = surface.Texture->LockRect(GetD3d9MipMapLevel(MipMapLevel), pLockedRect, pRect, Flags);
		if (FAILED(hr) && (Flags & D3DLOCK_NOSYSLOCK))
		{
			hr = surface.Texture->LockRect(GetD3d9MipMapLevel(MipMapLevel), pLockedRect, pRect, Flags & ~D3DLOCK_NOSYSLOCK);
		}
		return hr;
	}

	return DDERR_GENERIC;
}

HRESULT m_IDirectDrawSurfaceX::UnLockD3d9Surface(DWORD MipMapLevel)
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

HRESULT m_IDirectDrawSurfaceX::PresentOverlay(LPRECT lpSrcRect)
{
	if (SurfaceOverlay.OverlayEnabled)
	{
		RECT SrcRect = {};
		if (!lpSrcRect || SurfaceOverlay.isSrcRectNull || GetOverlappingRect(*lpSrcRect, SurfaceOverlay.SrcRect, SrcRect))
		{
			LPRECT lpNewSrcRect = SurfaceOverlay.isSrcRectNull ? nullptr : &SurfaceOverlay.SrcRect;
			LPRECT lpNewDestRect = SurfaceOverlay.isDestRectNull ? nullptr : &SurfaceOverlay.DestRect;

			DWORD DDBltFxFlags = SurfaceOverlay.DDBltFxFlags;
			DDBLTFX DDBltFx = SurfaceOverlay.DDBltFx;

			// Handle color keying
			if (!(DDBltFxFlags & (DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE)))
			{
				if ((SurfaceOverlay.DDOverlayFxFlags & DDOVER_KEYDEST) && (SurfaceOverlay.lpDDDestSurfaceX->surfaceDesc2.dwFlags & DDSD_CKDESTOVERLAY))
				{
					DDBltFxFlags |= (DDBLT_DDFX | DDBLT_KEYDESTOVERRIDE);
					DDBltFx.ddckDestColorkey = SurfaceOverlay.lpDDDestSurfaceX->surfaceDesc2.ddckCKDestOverlay;
				}
				else if ((SurfaceOverlay.DDOverlayFxFlags & DDOVER_KEYSRC) && (surfaceDesc2.dwFlags & DDSD_CKSRCOVERLAY))
				{
					DDBltFxFlags |= (DDBLT_DDFX | DDBLT_KEYSRCOVERRIDE);
					DDBltFx.ddckSrcColorkey = surfaceDesc2.ddckCKSrcOverlay;
				}
			}

			SurfaceOverlay.lpDDDestSurfaceX->Blt(lpNewDestRect, (LPDIRECTDRAWSURFACE7)GetWrapperInterfaceX(0), lpNewSrcRect, DDBltFxFlags, &DDBltFx, 0);
		}
	}
	return DD_OK;
}

// ******************************
// External static functions
// ******************************

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

	ScopedDDCriticalSection ThreadLockDD;

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
}

void m_IDirectDrawSurfaceX::CleanupSharedEmulatedMemory()
{
	// Disable shared memory
	ShareEmulatedMemory = false;
	
	LOG_LIMIT(100, __FUNCTION__ << " Deleting " << memorySurfaces.size() << " emulated surface" << ((memorySurfaces.size() != 1) ? "s" : "") << "!");

	ScopedDDCriticalSection ThreadLockDD;

	// Clean up unused emulated surfaces
	for (EMUSURFACE* pEmuSurface: memorySurfaces)
	{
		DeleteEmulatedMemory(&pEmuSurface);
	}
	memorySurfaces.clear();
}

void m_IDirectDrawSurfaceX::SizeDummySurface(size_t size)
{
	dummySurface.resize(size);
}

void m_IDirectDrawSurfaceX::CleanupDummySurface()
{
	dummySurface.clear();
}
