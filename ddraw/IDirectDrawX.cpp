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
* Code taken from: https://github.com/strangebytes/diablo-ddrawwrapper
*/

#include "ddraw.h"
#include "ddrawExternal.h"
#include "Utils\Utils.h"
#include "Dllmain\DllMain.h"
#include "d3d9\d3d9External.h"
#include "d3dddi\d3dddiExternal.h"
#include "Shaders\PaletteShader.h"
#include "Shaders\ColorKeyShader.h"

constexpr DWORD MaxVidMemory		= 0x20000000;	// 512 MBs
constexpr DWORD MinUsedVidMemory	= 0x00100000;	// 1 MB

const D3DFORMAT D9DisplayFormat = D3DFMT_X8R8G8B8;

// ddraw interface counter
DWORD ddrawRefCount = 0;

// Store a list of ddraw devices
std::vector<m_IDirectDrawX*> DDrawVector;

// Mouse hook
HANDLE ghWriteEvent = nullptr;
HANDLE threadID = nullptr;
HHOOK m_hook = nullptr;
bool bMouseChange = false;
POINT mousePos = {};

// Cooperative level settings
HWND MainhWnd;
HDC MainhDC;
m_IDirectDrawX* MainSetBy;

// Exclusive mode
bool ExclusiveMode;
HWND ExclusiveHwnd;
m_IDirectDrawX* ExclusiveSetBy;
DWORD ExclusiveWidth;
DWORD ExclusiveHeight;
DWORD ExclusiveBPP;
DWORD ExclusiveRefreshRate;

// Application set display mode
DWORD displayModeWidth;
DWORD displayModeHeight;
DWORD displayModeBPP;
DWORD displayModeRefreshRate;

float ScaleDDWidthRatio = 1.0f;
float ScaleDDHeightRatio = 1.0f;
DWORD ScaleDDLastWidth = 0;
DWORD ScaleDDLastHeight = 0;
DWORD ScaleDDCurrentWidth = 0;
DWORD ScaleDDCurrentHeight = 0;
DWORD ScaleDDPadX = 0;
DWORD ScaleDDPadY = 0;

// Display resolution
bool SetDefaultDisplayMode;			// Set native resolution
bool SetResolution;
DWORD displayWidth;
DWORD displayHeight;
DWORD displayRefreshRate;			// Refresh rate for fullscreen

// Viewport resolution
DWORD viewportWidth;
DWORD viewportHeight;

// Surface resolution
DWORD surfaceWidth;
DWORD surfaceHeight;

// Display pixel format
DDPIXELFORMAT DisplayPixelFormat;

// Last resolution
DWORD LastWidth;
DWORD LastHeight;
DWORD LastBPP;

// Display mode settings
bool isWindowed;					// Window mode enabled
bool AntiAliasing;
D3DFORMAT DepthStencilSurface;
bool AllowModeX;
bool MultiThreaded;
bool FPUPreserve;
bool NoWindowChanges;

// High resolution counter used for auto frame skipping
bool FrequencyFlag;
LARGE_INTEGER clockFrequency, clickTime, lastPresentTime;
LONGLONG lastFrameTime;
DWORD FrameCounter;
DWORD monitorRefreshRate;
DWORD monitorHeight;

// Direct3D9 flags
bool EnableWaitVsync;

// Direct3D9 Objects
LPDIRECT3D9 d3d9Object;
LPDIRECT3DDEVICE9 d3d9Device;
D3DPRESENT_PARAMETERS presParams;
LPDIRECT3DPIXELSHADER9 palettePixelShader = nullptr;
LPDIRECT3DPIXELSHADER9 colorkeyPixelShader = nullptr;
DWORD BehaviorFlags;
HWND hFocusWindow;

std::unordered_map<HWND, m_IDirectDrawX*> g_hookmap;

/************************/
/*** IUnknown methods ***/
/************************/

HRESULT m_IDirectDrawX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
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
		if (riid == IID_IDirect3D || riid == IID_IDirect3D2 || riid == IID_IDirect3D3 || riid == IID_IDirect3D7)
		{
			DxVersion = GetGUIDVersion(riid);

			SetCriticalSection();
			if (D3DInterface)
			{
				*ppvObj = D3DInterface->GetWrapperInterfaceX(DxVersion);

				D3DInterface->AddRef(DxVersion);
			}
			else
			{
				m_IDirect3DX *p_IDirect3DX = new m_IDirect3DX(this, DxVersion);

				*ppvObj = p_IDirect3DX->GetWrapperInterfaceX(DxVersion);

				D3DInterface = p_IDirect3DX;
			}
			ReleaseCriticalSection();

			return DD_OK;
		}
		if (riid == IID_IDirectDrawColorControl)
		{
			return CreateColorInterface(ppvObj);
		}
		if (riid == IID_IDirectDrawGammaControl)
		{
			return CreateGammaInterface(ppvObj);
		}
	}

	HRESULT hr = ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DxVersion));

	if (SUCCEEDED(hr) && Config.ConvertToDirect3D7)
	{
		if (riid == IID_IDirect3D || riid == IID_IDirect3D2 || riid == IID_IDirect3D3 || riid == IID_IDirect3D7)
		{
			m_IDirect3DX *lpD3DirectX = nullptr;

			((IDirect3D7*)*ppvObj)->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpD3DirectX);

			if (lpD3DirectX)
			{
				lpD3DirectX->SetDdrawParent(this);

				D3DInterface = lpD3DirectX;
			}
		}
	}

	return hr;
}

void *m_IDirectDrawX::GetWrapperInterfaceX(DWORD DirectXVersion)
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

ULONG m_IDirectDrawX::AddRef(DWORD DirectXVersion)
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

ULONG m_IDirectDrawX::Release(DWORD DirectXVersion)
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
			delete this;
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

/***************************/
/*** IDirectDraw methods ***/
/***************************/

HRESULT m_IDirectDrawX::Compact()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// This method is not currently implemented even in ddraw.
		return DD_OK;
	}

	return ProxyInterface->Compact();
}

HRESULT m_IDirectDrawX::CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR * lplpDDClipper, IUnknown FAR * pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpDDClipper || pUnkOuter)
		{
			return DDERR_INVALIDPARAMS;
		}

		m_IDirectDrawClipper* ClipperX = new m_IDirectDrawClipper(this, dwFlags);

		*lplpDDClipper = ClipperX;

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->CreateClipper(dwFlags, lplpDDClipper, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDClipper)
	{
		*lplpDDClipper = new m_IDirectDrawClipper(*lplpDDClipper);
	}

	return hr;
}

HRESULT m_IDirectDrawX::CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, LPDIRECTDRAWPALETTE FAR * lplpDDPalette, IUnknown FAR * pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpDDPalette || !lpDDColorArray || pUnkOuter)
		{
			return DDERR_INVALIDPARAMS;
		}

		m_IDirectDrawPalette *PaletteX = new m_IDirectDrawPalette(this, dwFlags, lpDDColorArray);

		*lplpDDPalette = PaletteX;

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->CreatePalette(dwFlags, lpDDColorArray, lplpDDPalette, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDPalette)
	{
		*lplpDDPalette = new m_IDirectDrawPalette(*lplpDDPalette);
	}

	return hr;
}

HRESULT m_IDirectDrawX::CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE7 FAR * lplpDDSurface, IUnknown FAR * pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lplpDDSurface || !lpDDSurfaceDesc || pUnkOuter)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (ProxyDirectXVersion > 3)
	{
		if (lpDDSurfaceDesc->dwSize != sizeof(DDSURFACEDESC))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << lpDDSurfaceDesc->dwSize);
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2 = {};
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);

		HRESULT hr = CreateSurface2(&Desc2, lplpDDSurface, pUnkOuter, DirectXVersion);

		Desc2.ddsCaps.dwCaps2 = 0;
		Desc2.ddsCaps.dwVolumeDepth = 0;
		ConvertSurfaceDesc(*lpDDSurfaceDesc, Desc2);

		return hr;
	}

	DDSURFACEDESC Desc = *lpDDSurfaceDesc;

	// Adjust Height and Width
	DWORD Width = 0;
	DWORD Height = 0;
	if ((Desc.dwFlags & DDSD_WIDTH) && (Desc.dwFlags & DDSD_HEIGHT))
	{
		Width = Desc.dwWidth;
		Height = Desc.dwHeight;
		Desc.dwWidth += Desc.dwWidth % 2;
	}

	HRESULT hr = GetProxyInterfaceV3()->CreateSurface(&Desc, (LPDIRECTDRAWSURFACE*)lplpDDSurface, pUnkOuter);

	if (SUCCEEDED(hr))
	{
		m_IDirectDrawSurfaceX *D3DSurfaceDevice = new m_IDirectDrawSurfaceX((IDirectDrawSurface7*)*lplpDDSurface, DirectXVersion);

		*lplpDDSurface = (LPDIRECTDRAWSURFACE7)D3DSurfaceDevice->GetWrapperInterfaceX(DirectXVersion);

		// Set original Height and Width
		if (Height && Width)
		{
			D3DSurfaceDevice->SetWrapperSurfaceSize(Height, Width);
		}
	}

	return hr;
}

HRESULT m_IDirectDrawX::CreateSurface2(LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPDIRECTDRAWSURFACE7 FAR * lplpDDSurface, IUnknown FAR * pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lplpDDSurface || !lpDDSurfaceDesc2 || pUnkOuter)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		if (lpDDSurfaceDesc2->dwSize != sizeof(DDSURFACEDESC2))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << lpDDSurfaceDesc2->dwSize);
			return DDERR_INVALIDPARAMS;
		}

		// Check for existing primary surface
		if ((lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) && GetPrimarySurface())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: primary surface already exists!");
			return DDERR_PRIMARYSURFACEALREADYEXISTS;
		}

		// Check for invalid surface flip flags
		if ((lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_FLIP) &&
			(!(lpDDSurfaceDesc2->dwFlags & DDSD_BACKBUFFERCOUNT) || !(lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_COMPLEX)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid flip surface flags!");
			return DDERR_INVALIDPARAMS;
		}

		// Check for invalid zbuffer flags
		if ((lpDDSurfaceDesc2->dwFlags & DDSD_PIXELFORMAT) && (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_ZBUFFER) &&
			!(lpDDSurfaceDesc2->ddpfPixelFormat.dwFlags & (DDPF_ZBUFFER | DDPF_STENCILBUFFER)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid zbuffer surface flags!");
			return DDERR_INVALIDPARAMS;
		}

		// Check for other unsupported pixel formats
		if ((lpDDSurfaceDesc2->dwFlags & DDSD_PIXELFORMAT) && (lpDDSurfaceDesc2->ddpfPixelFormat.dwFlags & 
			(DDPF_RGBTOYUV | DDPF_YUV | DDPF_BUMPDUDV | DDPF_BUMPLUMINANCE | DDPF_ALPHAPREMULT | DDPF_COMPRESSED | DDPF_ZPIXELS |
				DDPF_PALETTEINDEXED1 | DDPF_PALETTEINDEXED2 | DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXEDTO8)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: PixelForamt not Implemented: " << lpDDSurfaceDesc2->ddpfPixelFormat);
			return DDERR_INVALIDPIXELFORMAT;
		}

		// Check for Cube map
		if (lpDDSurfaceDesc2->ddsCaps.dwCaps2 & (DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_ALLFACES))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Cube map not Implemented.");
		}

		// Check for Volume
		if (lpDDSurfaceDesc2->ddsCaps.dwCaps2 & DDSCAPS2_VOLUME)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Volume not Implemented.");
		}

		// Check for MipMap
		// DDSCAPS_MIPMAP Indicates surface is one level of a mip-map. This surface will be attached to other DDSCAPS_MIPMAP surfaces to form the mip-map.
		// This can be done explicitly, by creating a number of surfaces and attaching them with AddAttachedSurface or by implicitly by CreateSurface.
		// If this bit is set then DDSCAPS_TEXTURE must also be set.
		if (((lpDDSurfaceDesc2->dwFlags & DDSD_MIPMAPCOUNT) && (lpDDSurfaceDesc2->dwMipMapCount != 1)) &&
			(lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_MIPMAP))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: MipMap not Implemented. Count: " << lpDDSurfaceDesc2->dwMipMapCount);
		}

		// Check for Overlay
		if ((lpDDSurfaceDesc2->dwFlags & (DDSD_CKDESTOVERLAY | DDSD_CKSRCOVERLAY)) || (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_OVERLAY))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Overlay not Implemented.");
		}

		// Check for unsupported flags
		DWORD UnsupportedDDSDFlags = (DDSD_ALPHABITDEPTH | DDSD_LPSURFACE | DDSD_LINEARSIZE | DDSD_FVF | DDSD_SRCVBHANDLE | DDSD_DEPTH);
		if (lpDDSurfaceDesc2->dwFlags & UnsupportedDDSDFlags)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: non-supported dwFlags! " << Logging::hex(lpDDSurfaceDesc2->dwFlags & UnsupportedDDSDFlags));
		}

		// Check for unsupported ddsCaps
		DWORD UnsupportedDDSCaps = (DDSCAPS_LIVEVIDEO | DDSCAPS_HWCODEC | DDSCAPS_ALLOCONLOAD | DDSCAPS_VIDEOPORT);
		DWORD UnsupportedDDSCaps2 = (DDSCAPS2_HINTDYNAMIC | DDSCAPS2_HINTSTATIC | DDSCAPS2_OPAQUE | DDSCAPS2_NOTUSERLOCKABLE);
		if ((lpDDSurfaceDesc2->ddsCaps.dwCaps & UnsupportedDDSCaps) || (lpDDSurfaceDesc2->ddsCaps.dwCaps2 & UnsupportedDDSCaps2))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: non-supported ddsCaps: " << Logging::hex(lpDDSurfaceDesc2->ddsCaps.dwCaps & UnsupportedDDSCaps) << " " <<
				Logging::hex(lpDDSurfaceDesc2->ddsCaps.dwCaps2 & UnsupportedDDSCaps2));
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false)))
		{
			return DDERR_GENERIC;
		}

		bool ReCreateD3D9Device = false;

		DDSURFACEDESC2 Desc2 = *lpDDSurfaceDesc2;

		// Check pixel format flag
		if ((Desc2.dwFlags & DDSD_PIXELFORMAT) && !Desc2.ddpfPixelFormat.dwFlags)
		{
			Desc2.dwFlags &= ~DDSD_PIXELFORMAT;
		}

		// Check pixel format
		if (Desc2.dwFlags & DDSD_PIXELFORMAT)
		{
			Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
			const DWORD Usage = (Desc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) ? D3DUSAGE_RENDERTARGET :
				((Desc2.dwFlags & DDSD_MIPMAPCOUNT) || (Desc2.ddsCaps.dwCaps & DDSCAPS_MIPMAP)) ? D3DUSAGE_AUTOGENMIPMAP :
				(Desc2.ddpfPixelFormat.dwFlags & (DDPF_ZBUFFER | DDPF_STENCILBUFFER)) ? D3DUSAGE_DEPTHSTENCIL : 0;
			const D3DRESOURCETYPE Resource = (Desc2.ddpfPixelFormat.dwFlags & (DDPF_ZBUFFER | DDPF_STENCILBUFFER)) ? D3DRTYPE_SURFACE : D3DRTYPE_TEXTURE;
			D3DFORMAT tmpFormat = GetDisplayFormat(Desc2.ddpfPixelFormat);
			const D3DFORMAT Format = (tmpFormat == D3DFMT_X8B8G8R8 || tmpFormat == D3DFMT_B8G8R8 || tmpFormat == D3DFMT_R8G8B8) ? D3DFMT_X8R8G8B8 :
				(tmpFormat == D3DFMT_A8B8G8R8) ? D3DFMT_A8R8G8B8 :
				(Resource == D3DRTYPE_TEXTURE && tmpFormat == D3DFMT_P8) ? D3DFMT_L8 : tmpFormat;

			if (FAILED(d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D9DisplayFormat, Usage, Resource, Format)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: non-supported pixel format! " << Usage << " " << Format << " " << Desc2.ddpfPixelFormat);
				return DDERR_INVALIDPIXELFORMAT;
			}

			if (Desc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
			{
				DisplayPixelFormat = Desc2.ddpfPixelFormat;
			}
		}

		// Updates for surface description
		Desc2.dwFlags |= DDSD_CAPS;
		Desc2.ddsCaps.dwCaps4 = DDSCAPS4_CREATESURFACE |											// Indicates surface was created using CreateSurface()
			((Desc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) ? DDSCAPS4_PRIMARYSURFACE : NULL);		// Indicates surface is a primary surface or a backbuffer of a primary surface
		if (Desc2.ddsCaps.dwCaps & DDSCAPS_FLIP)
		{
			Desc2.ddsCaps.dwCaps |= DDSCAPS_FRONTBUFFER;
		}
		Desc2.dwReserved = 0;

		// BackBufferCount must be at least 1
		if (Desc2.dwFlags & DDSD_BACKBUFFERCOUNT)
		{
			if (!Desc2.dwBackBufferCount)
			{
				Desc2.dwBackBufferCount = 1;
			}
		}
		else
		{
			Desc2.dwBackBufferCount = 0;
		}

		// Remove unused flags
		if (!Desc2.dwWidth || !Desc2.dwHeight)
		{
			Desc2.dwFlags &= ~(DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH);
		}
		if (!Desc2.dwRefreshRate)
		{
			Desc2.dwFlags &= ~DDSD_REFRESHRATE;
		}

		// Check for depth stencil surface
		if (!Config.DdrawOverrideStencilFormat && (Desc2.dwFlags & DDSD_PIXELFORMAT) && (Desc2.ddpfPixelFormat.dwFlags & (DDPF_ZBUFFER | DDPF_STENCILBUFFER)))
		{
			D3DFORMAT NewDepthStencilSurface = GetDisplayFormat(Desc2.ddpfPixelFormat);
			const bool IsDepthStencilSurface = (Desc2.ddpfPixelFormat.dwFlags & (DDPF_ZBUFFER | DDPF_STENCILBUFFER));

			if (IsDepthStencilSurface && NewDepthStencilSurface != DepthStencilSurface)
			{
				if (DepthStencilSurface != D3DFMT_UNKNOWN)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: existing stencil surface already setup.");
				}
				DepthStencilSurface = NewDepthStencilSurface;

				ReCreateD3D9Device = true;
			}
		}

		// Get surface size
		if (!displayWidth && !displayHeight && (Desc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT)) == (DDSD_WIDTH | DDSD_HEIGHT) &&
			(Desc2.ddsCaps.dwCaps & (DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE)))
		{
			surfaceWidth = Desc2.dwWidth;
			surfaceHeight = Desc2.dwHeight;

			// Check if there is a change in the present parameters
			if (surfaceWidth != presParams.BackBufferWidth || surfaceHeight != presParams.BackBufferHeight)
			{
				ReCreateD3D9Device = true;
			}
		}

		// Get present parameters
		if (Desc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
		{
			Logging::LogDebug() << __FUNCTION__ << " Primary surface " << Desc2.dwWidth << "x" << Desc2.dwHeight << " ddsCaps: " << Logging::hex(Desc2.ddsCaps.dwCaps) << " " <<
				Logging::hex(Desc2.ddsCaps.dwCaps2) << " " << LOWORD(Desc2.ddsCaps.dwVolumeDepth);

			// Anti-aliasing
			if (!Config.AntiAliasing)
			{
				bool OldAntiAliasing = AntiAliasing;
				AntiAliasing = ((Desc2.ddsCaps.dwCaps2 & DDSCAPS2_HINTANTIALIASING) && (Desc2.ddsCaps.dwCaps & DDSCAPS_3DDEVICE));

				// Check if there is a change in the present parameters
				if (AntiAliasing != OldAntiAliasing)
				{
					ReCreateD3D9Device = true;
				}
			}
		}

		// Recreate d3d9 device
		if (ReCreateD3D9Device)
		{
			CreateD3D9Device();
		}

		m_IDirectDrawSurfaceX *p_IDirectDrawSurfaceX = new m_IDirectDrawSurfaceX(this, DirectXVersion, &Desc2);

		*lplpDDSurface = (LPDIRECTDRAWSURFACE7)p_IDirectDrawSurfaceX->GetWrapperInterfaceX(DirectXVersion);

		return DD_OK;
	}

	DDSURFACEDESC2 Desc2 = *lpDDSurfaceDesc2;

	if (ProxyDirectXVersion != DirectXVersion)
	{
		// BackBufferCount must be at least 1
		if (Desc2.dwFlags & DDSD_BACKBUFFERCOUNT)
		{
			if (!Desc2.dwBackBufferCount)
			{
				Desc2.dwBackBufferCount = 1;
			}
		}
		else
		{
			Desc2.dwBackBufferCount = 0;
		}

		// Add flag for 3D device
		if ((DirectXVersion < 4) && (Desc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE))
		{
			Desc2.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
		}
	}

	// Adjust Height and Width
	DWORD Width = 0;
	DWORD Height = 0;
	if ((Desc2.dwFlags & DDSD_WIDTH) && (Desc2.dwFlags & DDSD_HEIGHT))
	{
		Width = Desc2.dwWidth;
		Height = Desc2.dwHeight;
		Desc2.dwWidth += Desc2.dwWidth % 2;
	}

	HRESULT hr = ProxyInterface->CreateSurface(&Desc2, lplpDDSurface, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDSurface)
	{
		m_IDirectDrawSurfaceX *D3DSurfaceDevice = new m_IDirectDrawSurfaceX((IDirectDrawSurface7*)*lplpDDSurface, DirectXVersion);

		*lplpDDSurface = (LPDIRECTDRAWSURFACE7)D3DSurfaceDevice->GetWrapperInterfaceX(DirectXVersion);

		// Set original Height and Width
		if (Height && Width)
		{
			D3DSurfaceDevice->SetWrapperSurfaceSize(Height, Width);
		}

		if (Config.ConvertToDirectDraw7)
		{
			D3DSurfaceDevice->SetDdrawParent(this);
		}
	}

	return hr;
}

HRESULT m_IDirectDrawX::DuplicateSurface(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDIRECTDRAWSURFACE7 FAR * lplpDupDDSurface, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		m_IDirectDrawSurfaceX *lpDDSurfaceX = (m_IDirectDrawSurfaceX*)lpDDSurface;
		if (!DoesSurfaceExist(lpDDSurfaceX))
		{
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2 = {};
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		lpDDSurfaceX->GetSurfaceDesc2(&Desc2);
		Desc2.ddsCaps.dwCaps &= ~DDSCAPS_PRIMARYSURFACE;		// Remove Primary surface flag

		m_IDirectDrawSurfaceX *p_IDirectDrawSurfaceX = new m_IDirectDrawSurfaceX(this, DirectXVersion, &Desc2);

		*lplpDupDDSurface = (LPDIRECTDRAWSURFACE7)p_IDirectDrawSurfaceX->GetWrapperInterfaceX(DirectXVersion);

		return DD_OK;
	}

	if (lpDDSurface)
	{
		lpDDSurface->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDSurface);
	}

	HRESULT hr = ProxyInterface->DuplicateSurface(lpDDSurface, lplpDupDDSurface);

	if (SUCCEEDED(hr) && lplpDupDDSurface && lpDDSurface)
	{
		m_IDirectDrawSurfaceX *D3DSurfaceDevice = new m_IDirectDrawSurfaceX((IDirectDrawSurface7*)*lplpDupDDSurface, DirectXVersion);

		*lplpDupDDSurface = (LPDIRECTDRAWSURFACE7)D3DSurfaceDevice->GetWrapperInterfaceX(DirectXVersion);

		if (Config.ConvertToDirectDraw7)
		{
			D3DSurfaceDevice->SetDdrawParent(this);
		}
	}

	return hr;
}

HRESULT m_IDirectDrawX::EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (ProxyDirectXVersion > 3)
	{
		if (!lpEnumModesCallback || (lpDDSurfaceDesc && lpDDSurfaceDesc->dwSize != sizeof(DDSURFACEDESC)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc) ? lpDDSurfaceDesc->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		struct EnumDisplay
		{
			LPVOID lpContext;
			LPDDENUMMODESCALLBACK lpCallback;

			static HRESULT CALLBACK ConvertCallback(LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext)
			{
				EnumDisplay *self = (EnumDisplay*)lpContext;

				DDSURFACEDESC Desc = {};
				Desc.dwSize = sizeof(DDSURFACEDESC);
				ConvertSurfaceDesc(Desc, *lpDDSurfaceDesc2);

				return self->lpCallback(&Desc, self->lpContext);
			}
		} CallbackContext = {};
		CallbackContext.lpContext = lpContext;
		CallbackContext.lpCallback = lpEnumModesCallback;

		DDSURFACEDESC2 Desc2 = {};
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		if (lpDDSurfaceDesc)
		{
			ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);
		}

		return EnumDisplayModes2(dwFlags, (lpDDSurfaceDesc) ? &Desc2 : nullptr, &CallbackContext, EnumDisplay::ConvertCallback);
	}

	return GetProxyInterfaceV3()->EnumDisplayModes(dwFlags, lpDDSurfaceDesc, lpContext, lpEnumModesCallback);
}

HRESULT m_IDirectDrawX::EnumDisplayModes2(DWORD dwFlags, LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext, LPDDENUMMODESCALLBACK2 lpEnumModesCallback2)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpEnumModesCallback2 || (lpDDSurfaceDesc2 && lpDDSurfaceDesc2->dwSize != sizeof(DDSURFACEDESC2)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc2) ? lpDDSurfaceDesc2->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false)))
		{
			return DDERR_GENERIC;
		}

		// Save width, height and refresh rate
		bool SetRefreshRate = true;
		DWORD EnumWidth = 0;
		DWORD EnumHeight = 0;
		DWORD EnumRefreshRate = 0;
		if (lpDDSurfaceDesc2)
		{
			EnumWidth = (lpDDSurfaceDesc2->dwFlags & DDSD_WIDTH) ? lpDDSurfaceDesc2->dwWidth : 0;
			EnumHeight = (lpDDSurfaceDesc2->dwFlags & DDSD_HEIGHT) ? lpDDSurfaceDesc2->dwHeight : 0;
			EnumRefreshRate = (lpDDSurfaceDesc2->dwFlags & DDSD_REFRESHRATE) ? lpDDSurfaceDesc2->dwRefreshRate : 0;
		}
		if (!(dwFlags & DDEDM_REFRESHRATES) && !EnumRefreshRate)
		{
			SetRefreshRate = false;
			EnumRefreshRate = Utils::GetRefreshRate(GetHwnd());
		}

		// Get display modes to enum
		DWORD DisplayBitCount = (displayModeBPP) ? displayModeBPP : 0;
		if (lpDDSurfaceDesc2 && (lpDDSurfaceDesc2->dwFlags & DDSD_PIXELFORMAT))
		{
			DisplayBitCount = GetBitCount(lpDDSurfaceDesc2->ddpfPixelFormat);
		}
		bool DisplayAllModes = (DisplayBitCount != 8 && DisplayBitCount != 16 && DisplayBitCount != 24 && DisplayBitCount != 32);

		// Setup surface desc
		DDSURFACEDESC2 Desc2 = {};
		Desc2.dwSize = sizeof(DDSURFACEDESC2);

		// Setup display mode
		D3DDISPLAYMODE d3ddispmode;

		// Enumerate modes for format XRGB
		UINT modeCount = d3d9Object->GetAdapterModeCount(D3DADAPTER_DEFAULT, D9DisplayFormat);

		// Loop through all modes
		DWORD Loop = 0;
		for (UINT i = 0; i < modeCount; i++)
		{
			// Get display modes
			ZeroMemory(&d3ddispmode, sizeof(D3DDISPLAYMODE));
			if (FAILED(d3d9Object->EnumAdapterModes(D3DADAPTER_DEFAULT, D9DisplayFormat, i, &d3ddispmode)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: EnumAdapterModes failed");
				break;
			}

			// Loop through each bit count
			for (DWORD bpMode : {8, 16, 32})
			{
				// Set display bit count
				if (DisplayAllModes)
				{
					DisplayBitCount = bpMode;
				}

				// Check refresh mode
				if ((!EnumWidth || d3ddispmode.Width == EnumWidth) &&
					(!EnumHeight || d3ddispmode.Height == EnumHeight) &&
					(!EnumRefreshRate || d3ddispmode.RefreshRate == EnumRefreshRate))
				{
					if (++Loop > Config.DdrawLimitDisplayModeCount && Config.DdrawLimitDisplayModeCount)
					{
						return DD_OK;
					}

					// Set surface desc options
					Desc2.dwSize = sizeof(DDSURFACEDESC2);
					Desc2.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
					Desc2.dwWidth = d3ddispmode.Width;
					Desc2.dwHeight = d3ddispmode.Height;
					if (SetRefreshRate)
					{
						Desc2.dwFlags |= DDSD_REFRESHRATE;
						Desc2.dwRefreshRate = d3ddispmode.RefreshRate;
					}

					// Set adapter pixel format
					Desc2.dwFlags |= DDSD_PIXELFORMAT;
					Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
					SetDisplayFormat(Desc2.ddpfPixelFormat, DisplayBitCount);

					// Set pitch
					Desc2.dwFlags |= DDSD_PITCH;
					Desc2.lPitch = (Desc2.ddpfPixelFormat.dwRGBBitCount / 8) * Desc2.dwWidth;

					if (lpEnumModesCallback2(&Desc2, lpContext) == DDENUMRET_CANCEL)
					{
						return DD_OK;
					}
				}

				// Break if not displaying all modes
				if (!DisplayAllModes)
				{
					break;
				}
			}
		}

		return DD_OK;
	}

	return ProxyInterface->EnumDisplayModes(dwFlags, lpDDSurfaceDesc2, lpContext, lpEnumModesCallback2);
}

HRESULT m_IDirectDrawX::EnumSurfaces(DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lpEnumSurfacesCallback)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (ProxyDirectXVersion > 3)
	{
		if ((lpDDSurfaceDesc && lpDDSurfaceDesc->dwSize != sizeof(DDSURFACEDESC)) || (!lpDDSurfaceDesc && !(dwFlags & DDENUMSURFACES_ALL)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc) ? lpDDSurfaceDesc->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2 = {};
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		if (lpDDSurfaceDesc)
		{
			ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);
		}

		return EnumSurfaces2(dwFlags, (lpDDSurfaceDesc) ? &Desc2 : nullptr, lpContext, (LPDDENUMSURFACESCALLBACK7)lpEnumSurfacesCallback, DirectXVersion);
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

	return GetProxyInterfaceV3()->EnumSurfaces(dwFlags, lpDDSurfaceDesc, &CallbackContext, EnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawX::EnumSurfaces2(DWORD dwFlags, LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback7, DWORD DirectXVersion)
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
		if ((lpDDSurfaceDesc2 && lpDDSurfaceDesc2->dwSize != sizeof(DDSURFACEDESC2)) || (!lpDDSurfaceDesc2 && !(dwFlags & DDENUMSURFACES_ALL)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc2) ? lpDDSurfaceDesc2->dwSize : -1) << " dwFlags: " << Logging::hex(dwFlags));
			return DDERR_INVALIDPARAMS;
		}

		switch (dwFlags)
		{
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid dwFlags. dwFlags: " << Logging::hex(dwFlags));
			return DDERR_INVALIDPARAMS;

		case (DDENUMSURFACES_DOESEXIST | DDENUMSURFACES_ALL):
			for (m_IDirectDrawSurfaceX* pSurfaceX : SurfaceVector)
			{
				LPDIRECTDRAWSURFACE7 pSurface7 = (LPDIRECTDRAWSURFACE7)pSurfaceX->GetWrapperInterfaceX(DirectXVersion);

				if (pSurface7)
				{
					DDSURFACEDESC2 Desc2 = {};
					Desc2.dwSize = sizeof(DDSURFACEDESC2);
					pSurfaceX->GetSurfaceDesc2(&Desc2);

					// When using the DDENUMSURFACES_DOESEXIST flag, an enumerated surface's reference count is incremented
					pSurface7->AddRef();

					if (EnumSurface::ConvertCallback(pSurface7, &Desc2, &CallbackContext) == DDENUMRET_CANCEL)
					{
						return DD_OK;
					}
				}
			}
			break;

		case (DDENUMSURFACES_DOESEXIST | DDENUMSURFACES_MATCH):
		case (DDENUMSURFACES_DOESEXIST | DDENUMSURFACES_NOMATCH):
			LOG_LIMIT(100, __FUNCTION__ << " Error: surface matching Not Implemented!");
			return DDERR_UNSUPPORTED;

		case (DDENUMSURFACES_CANBECREATED | DDENUMSURFACES_MATCH):
			LOG_LIMIT(100, __FUNCTION__ << " Error: creating enumerated surface Not Implemented!");
			return DDERR_UNSUPPORTED;
		}

		return DD_OK;
	}

	return ProxyInterface->EnumSurfaces(dwFlags, lpDDSurfaceDesc2, &CallbackContext, EnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawX::FlipToGDISurface()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// ToDo: Do proper implementation here
		return DD_OK;
	}

	return ProxyInterface->FlipToGDISurface();
}

HRESULT m_IDirectDrawX::GetCaps(LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDHELCaps)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lpDDDriverCaps && !lpDDHELCaps)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (lpDDDriverCaps)
	{
		lpDDDriverCaps->dwSize =
			lpDDDriverCaps->dwSize == sizeof(DDCAPS_DX3) ? sizeof(DDCAPS_DX3) :
			lpDDDriverCaps->dwSize == sizeof(DDCAPS_DX5) ? sizeof(DDCAPS_DX5) :
			lpDDDriverCaps->dwSize == sizeof(DDCAPS_DX6) ? sizeof(DDCAPS_DX6) :
			lpDDDriverCaps->dwSize == sizeof(DDCAPS_DX7) ? sizeof(DDCAPS_DX7) :
			sizeof(DDCAPS_DX1);
	}
	if (lpDDHELCaps)
	{
		lpDDHELCaps->dwSize =
			lpDDHELCaps->dwSize == sizeof(DDCAPS_DX3) ? sizeof(DDCAPS_DX3) :
			lpDDHELCaps->dwSize == sizeof(DDCAPS_DX5) ? sizeof(DDCAPS_DX5) :
			lpDDHELCaps->dwSize == sizeof(DDCAPS_DX6) ? sizeof(DDCAPS_DX6) :
			lpDDHELCaps->dwSize == sizeof(DDCAPS_DX7) ? sizeof(DDCAPS_DX7) :
			sizeof(DDCAPS_DX1);
	}

	DDCAPS DriverCaps = {}, HELCaps = {};
	DriverCaps.dwSize = sizeof(DDCAPS);
	HELCaps.dwSize = sizeof(DDCAPS);

	HRESULT hr = DD_OK;

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false)))
		{
			return DDERR_GENERIC;
		}

		// Get video memory
		DDSCAPS2 ddsCaps2 = {};
		ddsCaps2.dwCaps = DDSCAPS_VIDEOMEMORY;
		DWORD dwVidTotal, dwVidFree;
		GetAvailableVidMem2(&ddsCaps2, &dwVidTotal, &dwVidFree);

		// Get caps
		D3DCAPS9 Caps9;
		if (lpDDDriverCaps)
		{
			hr = d3d9Object->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, &Caps9);
			ConvertCaps(DriverCaps, Caps9);
			DriverCaps.dwVidMemTotal = dwVidTotal;
			DriverCaps.dwVidMemFree = dwVidFree;
		}
		if (lpDDHELCaps)
		{
			hr = d3d9Object->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &Caps9);
			ConvertCaps(HELCaps, Caps9);
			HELCaps.dwVidMemTotal = dwVidTotal;
			HELCaps.dwVidMemFree = dwVidFree;
		}

		// Get FourCCs count
		if ((DriverCaps.dwCaps & DDCAPS_BLTFOURCC) || (HELCaps.dwCaps & DDCAPS_BLTFOURCC))
		{
			DWORD dwNumFourCCCodes = 0;
			GetFourCCCodes(&dwNumFourCCCodes, nullptr);
			DriverCaps.dwNumFourCCCodes = dwNumFourCCCodes;
			HELCaps.dwNumFourCCCodes = dwNumFourCCCodes;
		}
	}
	else
	{
		if (lpDDDriverCaps)
		{
			DriverCaps.dwSize = lpDDDriverCaps->dwSize;
		}
		if (lpDDHELCaps)
		{
			HELCaps.dwSize = lpDDHELCaps->dwSize;
		}

		hr = ProxyInterface->GetCaps((lpDDDriverCaps) ? &DriverCaps : nullptr, (lpDDHELCaps) ? &HELCaps : nullptr);
	}

	if (SUCCEEDED(hr))
	{
		if (lpDDDriverCaps)
		{
			ConvertCaps(*lpDDDriverCaps, DriverCaps);
		}
		if (lpDDHELCaps)
		{
			ConvertCaps(*lpDDHELCaps, HELCaps);
		}
	}
	else
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to GetCaps!");
	}

	return hr;
}

HRESULT m_IDirectDrawX::GetDisplayMode(LPDDSURFACEDESC lpDDSurfaceDesc)
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

		HRESULT hr = GetDisplayMode2(&Desc2);

		// Convert back to LPDDSURFACEDESC
		if (SUCCEEDED(hr))
		{
			ConvertSurfaceDesc(*lpDDSurfaceDesc, Desc2);
		}

		return hr;
	}

	return GetProxyInterfaceV3()->GetDisplayMode(lpDDSurfaceDesc);
}

HRESULT m_IDirectDrawX::GetDisplayMode2(LPDDSURFACEDESC2 lpDDSurfaceDesc2)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpDDSurfaceDesc2 || lpDDSurfaceDesc2->dwSize != sizeof(DDSURFACEDESC2))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc2) ? lpDDSurfaceDesc2->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		// Set Surface Desc
		ZeroMemory(lpDDSurfaceDesc2, sizeof(DDSURFACEDESC2));
		lpDDSurfaceDesc2->dwSize = sizeof(DDSURFACEDESC2);
		lpDDSurfaceDesc2->dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_REFRESHRATE;
		DWORD displayModeBits = displayModeBPP;
		if (ExclusiveMode && ExclusiveWidth && ExclusiveHeight && ExclusiveBPP)
		{
			lpDDSurfaceDesc2->dwWidth = ExclusiveWidth;
			lpDDSurfaceDesc2->dwHeight = ExclusiveHeight;
			lpDDSurfaceDesc2->dwRefreshRate = ExclusiveRefreshRate;
			displayModeBits = ExclusiveBPP;
		}
		else
		{
			HWND hWnd = GetHwnd();
			Utils::GetScreenSize(hWnd, lpDDSurfaceDesc2->dwWidth, lpDDSurfaceDesc2->dwHeight);
			lpDDSurfaceDesc2->dwRefreshRate = Utils::GetRefreshRate(hWnd);
			displayModeBits = Utils::GetBitCount(hWnd);
		}

		// Force color mode
		displayModeBits = (Config.DdrawOverrideBitMode) ? Config.DdrawOverrideBitMode : displayModeBits;

		// Set Pixel Format
		lpDDSurfaceDesc2->dwFlags |= DDSD_PIXELFORMAT;
		lpDDSurfaceDesc2->ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		if (displayModeBits == DisplayPixelFormat.dwRGBBitCount)
		{
			lpDDSurfaceDesc2->ddpfPixelFormat = DisplayPixelFormat;
		}
		else if (FAILED(SetDisplayFormat(lpDDSurfaceDesc2->ddpfPixelFormat, displayModeBits)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Not implemented bit count " << displayModeBits);
			return DDERR_UNSUPPORTED;
		}

		return DD_OK;
	}

	return ProxyInterface->GetDisplayMode(lpDDSurfaceDesc2);
}

HRESULT m_IDirectDrawX::GetFourCCCodes(LPDWORD lpNumCodes, LPDWORD lpCodes)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpNumCodes)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters.");
			return DDERR_INVALIDPARAMS;
		}

		// Get FourCC list
		if (FourCCsList.size() == 0)
		{
			// Check for device interface
			if (FAILED(CheckInterface(__FUNCTION__, false)))
			{
				return DDERR_GENERIC;
			}

			// Test FourCCs that are supported
			for (D3DFORMAT format : FourCCTypes)
			{
				if (SUCCEEDED(d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, format)) ||
					SUCCEEDED(d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_TEXTURE, format)))
				{
					FourCCsList.push_back(format);
				}
			}
		}

		// If the number of entries is too small to accommodate all the codes, lpNumCodes is set to the required number,
		// and the array specified by lpCodes is filled with all that fits.
		if (lpCodes)
		{
			// Copy data to array
			const DWORD SizeToCopy = min(FourCCsList.size(), *lpNumCodes);
			memcpy(lpCodes, FourCCsList.data(), SizeToCopy * sizeof(D3DFORMAT));
		}

		// Set total number of FourCCs
		*lpNumCodes = FourCCsList.size();

		// Return value
		return DD_OK;
	}

	return ProxyInterface->GetFourCCCodes(lpNumCodes, lpCodes);
}

HRESULT m_IDirectDrawX::GetGDISurface(LPDIRECTDRAWSURFACE7 FAR * lplpGDIDDSSurface, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// ToDo: Do proper implementation here

		if (!lplpGDIDDSSurface)
		{
			return DDERR_INVALIDPARAMS;
		}

		*lplpGDIDDSSurface = (LPDIRECTDRAWSURFACE7)GetPrimarySurface();

		if (!*lplpGDIDDSSurface)
		{
			return DDERR_NOTFOUND;
		}

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->GetGDISurface(lplpGDIDDSSurface);

	if (SUCCEEDED(hr) && lplpGDIDDSSurface)
	{
		*lplpGDIDDSSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpGDIDDSSurface, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawX::GetMonitorFrequency(LPDWORD lpdwFrequency)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwFrequency)
		{
			return DDERR_INVALIDPARAMS;
		}

		DWORD Frequency = 0;

		if (d3d9Device)
		{
			D3DDISPLAYMODE Mode;
			if (SUCCEEDED(d3d9Device->GetDisplayMode(0, &Mode)))
			{
				Frequency = Mode.RefreshRate;
			}
		}

		// If frequency cannot be found
		if (!Frequency)
		{
			Frequency = Utils::GetRefreshRate(GetHwnd());
		}

		*lpdwFrequency = Frequency;

		return DD_OK;
	}

	return ProxyInterface->GetMonitorFrequency(lpdwFrequency);
}

HRESULT m_IDirectDrawX::GetScanLine(LPDWORD lpdwScanLine)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwScanLine)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		D3DRASTER_STATUS RasterStatus;
		if (FAILED(d3d9Device->GetRasterStatus(0, &RasterStatus)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get raster status!");
			return DDERR_GENERIC;
		}

		*lpdwScanLine = RasterStatus.ScanLine;

		return DD_OK;
	}

	return ProxyInterface->GetScanLine(lpdwScanLine);
}

HRESULT m_IDirectDrawX::GetVerticalBlankStatus(LPBOOL lpbIsInVB)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpbIsInVB)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		D3DRASTER_STATUS RasterStatus;
		if (FAILED(d3d9Device->GetRasterStatus(0, &RasterStatus)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get raster status!");
			return DDERR_GENERIC;
		}

		*lpbIsInVB = RasterStatus.InVBlank;

		return DD_OK;
	}

	return ProxyInterface->GetVerticalBlankStatus(lpbIsInVB);
}

HRESULT m_IDirectDrawX::Initialize(GUID FAR * lpGUID)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// ToDo: If you already used the DirectDrawCreate function to create a DirectDraw object, this method returns DDERR_ALREADYINITIALIZED.
		// If you do not call IDirectDraw7::Initialize when you use CoCreateInstance to create a DirectDraw object, any method that you
		// call afterward returns DDERR_NOTINITIALIZED.
		if (IsInitialize)
		{
			return DDERR_ALREADYINITIALIZED;
		}
		else
		{
			IsInitialize = true;
			return DD_OK;
		}
	}

	HRESULT hr = ProxyInterface->Initialize(lpGUID);

	if (IsInitialize)
	{
		return hr;
	}
	else
	{
		IsInitialize = true;
		return (hr == DDERR_ALREADYINITIALIZED) ? DD_OK : hr;
	}
}

// Resets the mode of the display device hardware for the primary surface to what it was before the IDirectDraw7::SetDisplayMode method was called.
HRESULT m_IDirectDrawX::RestoreDisplayMode()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Exclusive-level access is required to use this method.
		if (displayModeWidth && displayModeHeight && displayModeBPP && !ExclusiveMode)
		{
			return DDERR_NOEXCLUSIVEMODE;
		}

		// Reset mode
		displayModeWidth = 0;
		displayModeHeight = 0;
		displayModeBPP = 0;
		displayModeRefreshRate = 0;
		viewportWidth = 0;
		viewportHeight = 0;
		surfaceWidth = 0;
		surfaceHeight = 0;
		isWindowed = true;

		// Restore all existing surfaces
		if (d3d9Device)
		{
			RestoreAllSurfaces();
		}

		return DD_OK;
	}

	return ProxyInterface->RestoreDisplayMode();
}

HRESULT m_IDirectDrawX::SetCooperativeLevel(HWND hWnd, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << hWnd << " " << Logging::hex(dwFlags);

	if (Config.Dd7to9)
	{
		// Check for valid parameters
		// Note: real DirectDraw will allow both DDSCL_NORMAL and DDSCL_FULLSCREEN in some cases
		if (!(dwFlags & (DDSCL_EXCLUSIVE | DDSCL_NORMAL)) ||																			// An application must set either the DDSCL_EXCLUSIVE or the DDSCL_NORMAL flag
			((dwFlags & DDSCL_NORMAL) && (dwFlags & (DDSCL_ALLOWMODEX | DDSCL_EXCLUSIVE))) ||											// Normal flag cannot be used with Modex, Exclusive or Fullscreen flags
			((dwFlags & DDSCL_EXCLUSIVE) && !(dwFlags & DDSCL_FULLSCREEN)) ||															// If Exclusive flag is set then Fullscreen flag must be set
			((dwFlags & DDSCL_FULLSCREEN) && !(dwFlags & DDSCL_EXCLUSIVE)) ||															// If Fullscreen flag is set then Exclusive flag must be set
			((dwFlags & DDSCL_ALLOWMODEX) && (!(dwFlags & DDSCL_EXCLUSIVE) || !(dwFlags & DDSCL_FULLSCREEN))) ||						// If AllowModeX is set then Exclusive and Fullscreen flags must be set
			((dwFlags & DDSCL_SETDEVICEWINDOW) && (dwFlags & DDSCL_SETFOCUSWINDOW)) ||													// SetDeviceWindow flag cannot be used with SetFocusWindow flag
			((dwFlags & DDSCL_EXCLUSIVE) && !IsWindow(hWnd)))																			// When using Exclusive mode the hwnd must be valid
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwFlags: " << Logging::hex(dwFlags) << " " << hWnd);
			return DDERR_INVALIDPARAMS;
		}

		// Check for unsupported flags
		if (dwFlags & (DDSCL_CREATEDEVICEWINDOW | DDSCL_SETDEVICEWINDOW | DDSCL_SETFOCUSWINDOW))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Flags not supported. dwFlags: " << Logging::hex(dwFlags) << " " << hWnd);
		}

		// Check window handle thread
		if ((((dwFlags & DDSCL_EXCLUSIVE) || ExclusiveMode) && GetWindowThreadProcessId((hWnd) ? hWnd : MainhWnd, nullptr) != GetCurrentThreadId()))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: attempt to set exclusive mode from a different thread than the hwnd was created from! dwFlags: " << Logging::hex(dwFlags) << " " << hWnd);
		}

		HWND LastMainhWnd = MainhWnd;
		bool LastMultiThreaded = MultiThreaded;
		bool LastFPUPreserve = FPUPreserve;
		bool LastNoWindowChanges = NoWindowChanges;

		// Set windowed mode
		if (dwFlags & DDSCL_NORMAL)
		{
			// Check for exclusive mode
			if ((ExclusiveMode && hWnd && ExclusiveHwnd == hWnd && ExclusiveSetBy == this) || !IsWindow(ExclusiveHwnd))
			{
				ExclusiveMode = false;
				ExclusiveHwnd = nullptr;
				ExclusiveSetBy = nullptr;
				ExclusiveWidth = 0;
				ExclusiveHeight = 0;
				ExclusiveBPP = 0;
				ExclusiveRefreshRate = 0;
			}
		}
		else if (dwFlags & DDSCL_FULLSCREEN)
		{
			if (ExclusiveMode && ExclusiveHwnd != hWnd && IsWindow(ExclusiveHwnd))
			{
				// If SetCooperativeLevel is called once in a process, a binding is established between the process and the window.
				// If it is called again in the same process with a different non-null window handle, it returns the DDERR_HWNDALREADYSET error value.
				LOG_LIMIT(100, __FUNCTION__ << " Error: Exclusive mode already set.");
				return DDERR_HWNDALREADYSET;
			}
			ExclusiveMode = true;
			ExclusiveHwnd = hWnd;
			ExclusiveSetBy = this;
		}

		// Set device flags
		AllowModeX = ((dwFlags & DDSCL_ALLOWMODEX) != 0);
		MultiThreaded = ((dwFlags & DDSCL_MULTITHREADED) != 0);
		// The flag (DDSCL_FPUPRESERVE) is assumed by default in DirectX 6 and earlier.
		FPUPreserve = (((dwFlags & DDSCL_FPUPRESERVE) || DirectXVersion <= 6) && (dwFlags & DDSCL_FPUSETUP) == 0);
		NoWindowChanges = ((dwFlags & DDSCL_NOWINDOWCHANGES) != 0);

		// Check window handle
		if (hWnd && (((!ExclusiveMode || ExclusiveHwnd == hWnd) && (!MainhWnd || !MainSetBy || MainSetBy == this)) || !IsWindow(MainhWnd)))
		{
			// Check if DC needs to be released
			if (MainhWnd && MainhDC && (MainhWnd != hWnd))
			{
				CloseD3DDDI();
				ReleaseDC(MainhWnd, MainhDC);
				MainhDC = nullptr;
			}

			MainhWnd = hWnd;
			MainSetBy = this;

			if (MainhWnd && !MainhDC)
			{
				MainhDC = ::GetDC(MainhWnd);
			}
		}

		// Reset if mode was changed
		if (d3d9Device && MainhWnd && hWnd && (
			LastMainhWnd != MainhWnd ||
			LastMultiThreaded != MultiThreaded ||
			LastFPUPreserve != FPUPreserve ||
			LastNoWindowChanges != NoWindowChanges))
		{
			// Recreate d3d9 device
			CreateD3D9Device();
		}

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->SetCooperativeLevel(hWnd, dwFlags);

	// Release previouse Exclusive flag
	// Hook window message to get notified when the window is about to exit to remove the exclusive flag
	if (SUCCEEDED(hr) && (dwFlags & DDSCL_EXCLUSIVE) && IsWindow(hWnd) && hWnd != chWnd)
	{
		static DWORD WindowsGDIHook_DirectXVersion = DirectXVersion;

		// Fixes a bug in ddraw in Windows 8 and 10 where the exclusive flag remains even after the window (hWnd) closes
		struct WindowsGDIHook
		{
			static LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
			{
				UNREFERENCED_PARAMETER(lParam);

				if (nCode == HCBT_DESTROYWND && !Config.Exiting)
				{
					Logging::LogDebug() << __FUNCTION__;

					HWND hWnd = (HWND)wParam;
					auto it = g_hookmap.find(hWnd);
					if (it != std::end(g_hookmap))
					{
						m_IDirectDrawX *lpDDraw = it->second;
						if (lpDDraw && (ProxyAddressLookupTable.IsValidWrapperAddress(lpDDraw) ||
							ProxyAddressLookupTable.IsValidProxyAddress<m_IDirectDrawX>(lpDDraw)))
						{
							LOG_LIMIT(3, __FUNCTION__ << " Removing exclusive flag from closing window!");
							lpDDraw->SetCooperativeLevel(hWnd, DDSCL_NORMAL, WindowsGDIHook_DirectXVersion);
						}
						g_hookmap.clear();
					}
				}

				return CallNextHookEx(nullptr, nCode, wParam, lParam);
			}
		};

		g_hookmap.clear();

		if (g_hook)
		{
			UnhookWindowsHookEx(g_hook);
			g_hook = nullptr;
		}

		WindowsGDIHook_DirectXVersion = DirectXVersion;
		g_hookmap[hWnd] = this;
		g_hook = SetWindowsHookEx(WH_CBT, WindowsGDIHook::CBTProc, GetModuleHandle(nullptr), GetWindowThreadProcessId(hWnd, nullptr));

		chWnd = hWnd;
	}

	// Remove hWnd ExclusiveMode
	if (SUCCEEDED(hr) && (dwFlags & DDSCL_NORMAL) && IsWindow(hWnd) && hWnd == chWnd)
	{
		g_hookmap.clear();
		chWnd = nullptr;
	}

	// Remove window border on fullscreen windows 
	// Fixes a bug in ddraw in Windows 8 and 10 where the window border is visible in fullscreen mode
	if (SUCCEEDED(hr) && (dwFlags & DDSCL_FULLSCREEN) && IsWindow(hWnd))
	{
		LONG lStyle = GetWindowLong(hWnd, GWL_STYLE);
		if (lStyle & WS_CAPTION)
		{
			LOG_LIMIT(3, __FUNCTION__ << " Removing window WS_CAPTION!");

			// Removing WS_CAPTION
			SetWindowLong(hWnd, GWL_STYLE, lStyle & ~WS_CAPTION);
			SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING);

			// Peek messages to help prevent a "Not Responding" window
			Utils::CheckMessageQueue(hWnd);
		}
	}

	return hr;
}

HRESULT m_IDirectDrawX::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << dwWidth << "x" << dwHeight << " " << dwBPP << " " << dwRefreshRate << Logging::hex(dwFlags);

	if (Config.Dd7to9)
	{
		if (!dwWidth || !dwHeight || (dwBPP != 8 && dwBPP != 16 && dwBPP != 24 && dwBPP != 32))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. " << dwWidth << "x" << dwHeight << " " << dwBPP);
			return DDERR_INVALIDPARAMS;
		}

		// Check window handle thread
		if (ExclusiveMode && IsWindow(MainhWnd) && GetWindowThreadProcessId(MainhWnd, nullptr) != GetCurrentThreadId())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: set exclusive display from a different thread than the hwnd was created from! " << dwWidth << "x" << dwHeight << " " << dwBPP);
		}

		bool ChangeMode = false;
		bool ChangeBPP = false;
		DWORD NewBPP = (Config.DdrawOverrideBitMode) ? Config.DdrawOverrideBitMode : dwBPP;

		if (displayModeWidth != dwWidth || displayModeHeight != dwHeight || displayModeBPP != NewBPP || (dwRefreshRate && displayModeRefreshRate != dwRefreshRate))
		{
			DWORD FoundWidth = dwWidth;
			DWORD FoundHeight = dwHeight;

			// Check if it is a supported resolution
			if ((ExclusiveMode && !Config.EnableWindowMode) || Config.FullscreenWindowMode)
			{
				// Check for device interface
				if (FAILED(CheckInterface(__FUNCTION__, false)))
				{
					return DDERR_GENERIC;
				}

				// Enumerate modes for format XRGB
				UINT modeCount = d3d9Object->GetAdapterModeCount(D3DADAPTER_DEFAULT, D9DisplayFormat);

				D3DDISPLAYMODE d3ddispmode;
				bool modeFound = false;

				// Loop through all modes looking for our requested resolution
				for (UINT i = 0; i < modeCount; i++)
				{
					// Get display modes here
					ZeroMemory(&d3ddispmode, sizeof(D3DDISPLAYMODE));
					if (FAILED(d3d9Object->EnumAdapterModes(D3DADAPTER_DEFAULT, D9DisplayFormat, i, &d3ddispmode)))
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: EnumAdapterModes failed");
						break;
					}
					// Check exact height and width match
					if (d3ddispmode.Width == dwWidth && d3ddispmode.Height == dwHeight)
					{
						modeFound = true;
						FoundWidth = d3ddispmode.Width;
						FoundHeight = d3ddispmode.Height;
						break;
					}
					// Check partial height and width match
					if (((d3ddispmode.Width == 320 || d3ddispmode.Width == 640) && d3ddispmode.Width == dwWidth && d3ddispmode.Height == dwHeight + (dwHeight / 5)) ||
						(d3ddispmode.Width == 640 && d3ddispmode.Height == 480 && (dwWidth == 320 && (dwHeight == 200 || dwHeight == 240))) ||
						(d3ddispmode.Width == 800 && d3ddispmode.Height == 600 && dwWidth == 400 && dwHeight == 300) ||
						(d3ddispmode.Width == 1024 && d3ddispmode.Height == 768 && dwWidth == 512 && dwHeight == 384))
					{
						modeFound = true;
						FoundWidth = d3ddispmode.Width;
						FoundHeight = d3ddispmode.Height;
					}
				}

				// Mode not found
				if (!modeFound)
				{
					return DDERR_INVALIDMODE;
				}
			}

			ChangeMode = (displayModeWidth != dwWidth || displayModeHeight != dwHeight || (dwRefreshRate && displayModeRefreshRate != dwRefreshRate));
			ChangeBPP = (displayModeBPP != NewBPP);
			displayModeWidth = dwWidth;
			displayModeHeight = dwHeight;
			displayModeBPP = NewBPP;
			displayModeRefreshRate = dwRefreshRate;
			isWindowed = (!ExclusiveMode || Config.EnableWindowMode || Config.FullscreenWindowMode);

			// Display resolution
			if (SetDefaultDisplayMode)
			{
				displayWidth = (Config.DdrawUseNativeResolution || Config.DdrawOverrideWidth) ? displayWidth : FoundWidth;
				displayHeight = (Config.DdrawUseNativeResolution || Config.DdrawOverrideHeight) ? displayHeight : FoundHeight;
				displayRefreshRate = (Config.DdrawOverrideRefreshRate) ? displayRefreshRate : displayModeRefreshRate;
			}
		}

		// Set exclusive mode resolution
		if (ExclusiveMode && ExclusiveHwnd == MainhWnd)
		{
			ExclusiveWidth = dwWidth;
			ExclusiveHeight = dwHeight;
			ExclusiveBPP = NewBPP;
			ExclusiveRefreshRate = dwRefreshRate;
		}

		// Reset viewport resolution
		viewportWidth = 0;
		viewportHeight = 0;

		// Reset surface resolution
		surfaceWidth = 0;
		surfaceHeight = 0;

		// Update the d3d9 device to use new display mode
		if (ChangeMode)
		{
			SetResolution = ExclusiveMode;

			// Recreate d3d9 device
			CreateD3D9Device();
		}
		else if (ChangeBPP)
		{
			// Reset all surfaces
			RestoreAllSurfaces();
		}

		return DD_OK;
	}

	// Force color mode
	dwBPP = (Config.DdrawOverrideBitMode) ? Config.DdrawOverrideBitMode : dwBPP;

	if (Config.DdrawUseNativeResolution && dwWidth && dwHeight)
	{
		ScaleDDLastWidth = dwWidth;
		ScaleDDLastHeight = dwHeight;
		Utils::GetScreenSize(nullptr, ScaleDDCurrentWidth, ScaleDDCurrentHeight);
		dwWidth = ScaleDDCurrentWidth;
		dwHeight = ScaleDDCurrentHeight;
		ScaleDDWidthRatio = (float)ScaleDDCurrentWidth / (float)ScaleDDLastWidth;
		ScaleDDHeightRatio = (float)ScaleDDCurrentHeight / (float)ScaleDDLastHeight;
		if (Config.DdrawIntegerScalingClamp)
		{
			ScaleDDWidthRatio = truncf(ScaleDDWidthRatio);
			ScaleDDHeightRatio = truncf(ScaleDDHeightRatio);
		}
		if (Config.DdrawMaintainAspectRatio)
		{
				ScaleDDWidthRatio = min(ScaleDDWidthRatio, ScaleDDHeightRatio);
				ScaleDDHeightRatio = min(ScaleDDWidthRatio, ScaleDDHeightRatio);
		}
		if (Config.DdrawIntegerScalingClamp || Config.DdrawMaintainAspectRatio)
		{
			ScaleDDPadX = (DWORD)((ScaleDDCurrentWidth - (ScaleDDLastWidth * ScaleDDWidthRatio)) / 2.0f);
			ScaleDDPadY = (DWORD)((ScaleDDCurrentHeight - (ScaleDDLastHeight * ScaleDDHeightRatio)) / 2.0f);
		}
	}

	if (ProxyDirectXVersion == 1)
	{
		return GetProxyInterfaceV1()->SetDisplayMode(dwWidth, dwHeight, dwBPP);
	}

	return ProxyInterface->SetDisplayMode(dwWidth, dwHeight, dwBPP, dwRefreshRate, dwFlags);
}

HRESULT m_IDirectDrawX::WaitForVerticalBlank(DWORD dwFlags, HANDLE hEvent)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		if (Config.ForceVsyncMode)
		{
			return DD_OK;
		}

		D3DRASTER_STATUS RasterStatus;

		// Check flags
		switch (dwFlags)
		{
		case DDWAITVB_BLOCKBEGIN:
		case DDWAITVB_BLOCKEND:
			// Get vertical blank begin first, then get vertical blank end if requested
			// Use D3DKMT for vertical blank begin
			if (OpenD3DDDI(GetDC()) && D3DDDIWaitForVsync())
			{
				if (dwFlags == DDWAITVB_BLOCKBEGIN)
				{
					return DD_OK;
				}
			}
			// Use raster status for vertical blank begin (uses high CPU)
			else if (SUCCEEDED(d3d9Device->GetRasterStatus(0, &RasterStatus)))
			{
				while (SUCCEEDED(d3d9Device->GetRasterStatus(0, &RasterStatus)) && !RasterStatus.InVBlank) { Sleep(0); };
				if (dwFlags == DDWAITVB_BLOCKBEGIN)
				{
					return DD_OK;
				}
			}
			// Use raster status for vertical blank end (uses high CPU)
			while (SUCCEEDED(d3d9Device->GetRasterStatus(0, &RasterStatus)) && RasterStatus.InVBlank) { Sleep(0); };
			return DD_OK;
		case DDWAITVB_BLOCKBEGINEVENT:
			// Triggers an event when the vertical blank begins. This value is not supported.
			return DDERR_UNSUPPORTED;
		default:
			return DDERR_INVALIDPARAMS;
		}
	}

	return ProxyInterface->WaitForVerticalBlank(dwFlags, hEvent);
}

/*********************************/
/*** Added in the v2 interface ***/
/*********************************/

HRESULT m_IDirectDrawX::GetAvailableVidMem(LPDDSCAPS lpDDSCaps, LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Game using old DirectX, Convert DDSCAPS to DDSCAPS2
	if (ProxyDirectXVersion > 3)
	{
		DDSCAPS2 Caps2;
		if (lpDDSCaps)
		{
			ConvertCaps(Caps2, *lpDDSCaps);
		}

		return GetAvailableVidMem2((lpDDSCaps) ? &Caps2 : nullptr, lpdwTotal, lpdwFree);
	}

	HRESULT hr = GetProxyInterfaceV3()->GetAvailableVidMem(lpDDSCaps, lpdwTotal, lpdwFree);

	// Set available memory
	AdjustVidMemory(lpdwTotal, lpdwFree);

	return hr;
}

HRESULT m_IDirectDrawX::GetAvailableVidMem2(LPDDSCAPS2 lpDDSCaps2, LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = DD_OK;

	if (Config.Dd7to9)
	{
		if (!lpdwTotal && !lpdwFree)
		{
			return DDERR_INVALIDPARAMS;
		}

		// ToDo: Get correct total video memory size
		DWORD TotalMemory = 0;
		DWORD AvailableMemory = 0;

		// Get texture/surface memory
		if (lpDDSCaps2 && (lpDDSCaps2->dwCaps & (DDSCAPS_TEXTURE | DDSCAPS_OFFSCREENPLAIN)))
		{
			if (d3d9Device)
			{
				AvailableMemory = d3d9Device->GetAvailableTextureMem();
			}
		}
		// Get video memory
		else if (lpDDSCaps2 && (lpDDSCaps2->dwCaps & (DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM | DDSCAPS_3DDEVICE)))
		{
			// Open the first adapter in the system
			if (OpenD3DDDI(GetDC()) && D3DDDIGetVideoMemory(TotalMemory, AvailableMemory))
			{
				// Memory acquired using D3DDDI
			}
			else
			{
				// ToDo: figure out how to get the correct adapter number. For now just return the first video adapter
				TotalMemory = Utils::GetVideoRam(1);
			}
		}
		// Unknown memory type request
		else
		{
			Logging::Log() << __FUNCTION__ << " Error: Unknown memory type.  dwCaps: " << ((lpDDSCaps2) ? (void*)lpDDSCaps2->dwCaps : nullptr);
			return DDERR_INVALIDPARAMS;
		}

		// If memory cannot be found just return default memory
		if (!TotalMemory)
		{
			TotalMemory = (AvailableMemory) ? AvailableMemory + MinUsedVidMemory : MaxVidMemory;
		}

		// If memory cannot be found just return default memory
		if (!AvailableMemory)
		{
			AvailableMemory = TotalMemory - MinUsedVidMemory;
		}

		if (lpdwTotal)
		{
			*lpdwTotal = TotalMemory;
		}
		if (lpdwFree)
		{
			*lpdwFree = AvailableMemory;
		}
	}
	else
	{
		hr = ProxyInterface->GetAvailableVidMem(lpDDSCaps2, lpdwTotal, lpdwFree);
	}

	// Ajdust available memory
	AdjustVidMemory(lpdwTotal, lpdwFree);

	return hr;
}

/*********************************/
/*** Added in the V4 Interface ***/
/*********************************/

HRESULT m_IDirectDrawX::GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 * lpDDS, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	HRESULT hr = ProxyInterface->GetSurfaceFromDC(hdc, lpDDS);

	if (SUCCEEDED(hr) && lpDDS)
	{
		*lpDDS = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lpDDS, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawX::RestoreAllSurfaces()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check device status
		if (d3d9Device && d3d9Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			ReinitDevice();
		}
		else
		{
			SetCriticalSection();

			for (m_IDirectDrawX* pDDraw : DDrawVector)
			{
				for (m_IDirectDrawSurfaceX* pSurface : pDDraw->SurfaceVector)
				{
					pSurface->ResetSurfaceDisplay();
				}
			}

			ReleaseCriticalSection();
		}

		return DD_OK;
	}

	return ProxyInterface->RestoreAllSurfaces();
}

HRESULT m_IDirectDrawX::TestCooperativeLevel()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!ExclusiveMode)
		{
			return DDERR_NOEXCLUSIVEMODE;
		}

		if (!d3d9Device)
		{
			// Just return OK until device is setup
			return DD_OK;
		}

		switch (d3d9Device->TestCooperativeLevel())
		{
		case D3DERR_DRIVERINTERNALERROR:
		case D3DERR_INVALIDCALL:
			return DDERR_WRONGMODE;
		case D3DERR_DEVICENOTRESET:
			ReinitDevice();
			[[fallthrough]];
		case D3DERR_DEVICELOST:
		case D3D_OK:
		default:
			return DD_OK;
		}
	}

	return ProxyInterface->TestCooperativeLevel();
}

HRESULT m_IDirectDrawX::GetDeviceIdentifier(LPDDDEVICEIDENTIFIER lpdddi, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 4)
	{
		if (!lpdddi)
		{
			return DDERR_INVALIDPARAMS;
		}

		DDDEVICEIDENTIFIER2 Id2;

		HRESULT hr = GetDeviceIdentifier2(&Id2, dwFlags);

		if (SUCCEEDED(hr))
		{
			ConvertDeviceIdentifier(*lpdddi, Id2);
		}

		return hr;
	}

	return GetProxyInterfaceV4()->GetDeviceIdentifier(lpdddi, dwFlags);
}

HRESULT m_IDirectDrawX::GetDeviceIdentifier2(LPDDDEVICEIDENTIFIER2 lpdddi2, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdddi2)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false)))
		{
			return DDERR_GENERIC;
		}

		D3DADAPTER_IDENTIFIER9 Identifier9;

		HRESULT hr = d3d9Object->GetAdapterIdentifier(D3DADAPTER_DEFAULT, D3DENUM_WHQL_LEVEL, &Identifier9);

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get Adapter Identifier");
			return hr;
		}

		ConvertDeviceIdentifier(*lpdddi2, Identifier9);

		return DD_OK;
	}

	return ProxyInterface->GetDeviceIdentifier(lpdddi2, dwFlags);
}

HRESULT m_IDirectDrawX::StartModeTest(LPSIZE lpModesToTest, DWORD dwNumEntries, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->StartModeTest(lpModesToTest, dwNumEntries, dwFlags);
}

HRESULT m_IDirectDrawX::EvaluateMode(DWORD dwFlags, DWORD * pSecondsUntilTimeout)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->EvaluateMode(dwFlags, pSecondsUntilTimeout);
}

/************************/
/*** Helper functions ***/
/************************/

void m_IDirectDrawX::InitDdraw(DWORD DirectXVersion)
{
	WrapperInterface = new m_IDirectDraw((LPDIRECTDRAW)ProxyInterface, this);
	WrapperInterface2 = new m_IDirectDraw2((LPDIRECTDRAW2)ProxyInterface, this);
	WrapperInterface3 = new m_IDirectDraw3((LPDIRECTDRAW3)ProxyInterface, this);
	WrapperInterface4 = new m_IDirectDraw4((LPDIRECTDRAW4)ProxyInterface, this);
	WrapperInterface7 = new m_IDirectDraw7((LPDIRECTDRAW7)ProxyInterface, this);

	if (!Config.Dd7to9)
	{
		return;
	}

	AddRef(DirectXVersion);

	DWORD ref = InterlockedIncrement(&ddrawRefCount);

	SetCriticalSection();

	DDrawVector.push_back(this);

	if (ref == 1)
	{
		// Release DC
		if (MainhWnd && MainhDC)
		{
			ReleaseDC(MainhWnd, MainhDC);
			MainhDC = nullptr;
		}
		MainhWnd = nullptr;
		MainhDC = nullptr;
		MainSetBy = nullptr;

		// Exclusive mode
		ExclusiveMode = false;
		ExclusiveHwnd = nullptr;
		ExclusiveSetBy = nullptr;
		ExclusiveWidth = 0;
		ExclusiveHeight = 0;
		ExclusiveBPP = 0;
		ExclusiveRefreshRate = 0;

		// Application display mode
		displayModeWidth = 0;
		displayModeHeight = 0;
		displayModeBPP = 0;
		displayModeRefreshRate = 0;

		// Display pixel format
		ZeroMemory(&DisplayPixelFormat, sizeof(DDPIXELFORMAT));

		// Last resolution
		LastWidth = 0;
		LastHeight = 0;
		LastBPP = 0;

		// Display mode settings
		isWindowed = true;
		AntiAliasing = false;
		DepthStencilSurface = D3DFMT_UNKNOWN;
		AllowModeX = false;
		MultiThreaded = false;
		FPUPreserve = false;
		NoWindowChanges = false;

		// High resolution counter
		FrequencyFlag = (QueryPerformanceFrequency(&clockFrequency) != 0);
		clickTime.QuadPart = 0;
		lastPresentTime.QuadPart = 0;
		lastFrameTime = 0;
		FrameCounter = 0;
		monitorRefreshRate = 0;
		monitorHeight = 0;

		// Direct3D9 flags
		EnableWaitVsync = false;

		// Direct3D9 Objects
		d3d9Object = nullptr;
		d3d9Device = nullptr;

		ZeroMemory(&presParams, sizeof(D3DPRESENT_PARAMETERS));
		BehaviorFlags = 0;
		hFocusWindow = nullptr;

		// Display resolution
		if (Config.DdrawUseNativeResolution)
		{
			Utils::GetScreenSize(GetHwnd(), displayWidth, displayHeight);
		}
		else
		{
			displayWidth = (Config.DdrawOverrideWidth) ? Config.DdrawOverrideWidth : 0;
			displayHeight = (Config.DdrawOverrideHeight) ? Config.DdrawOverrideHeight : 0;
		}
		displayRefreshRate = (Config.DdrawOverrideRefreshRate) ? Config.DdrawOverrideRefreshRate : 0;
		SetDefaultDisplayMode = (!displayWidth || !displayHeight || !displayRefreshRate);
		SetResolution = false;

		viewportWidth = 0;
		viewportHeight = 0;
		surfaceWidth = 0;
		surfaceHeight = 0;

		static bool EnableMouseHook = Config.DdrawEnableMouseHook &&
			((Config.DdrawUseNativeResolution || Config.DdrawOverrideWidth || Config.DdrawOverrideHeight) &&
			(!Config.EnableWindowMode || (Config.EnableWindowMode && Config.FullscreenWindowMode)));

		// Set mouse hook
		if (!m_hook && EnableMouseHook)
		{
			struct WindowsMouseHook
			{
				static LRESULT CALLBACK mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
				{
					POINT p;
					if (nCode == HC_ACTION && wParam == WM_MOUSEMOVE)
					{
						if (ddrawRefCount && threadID && ghWriteEvent &&
							displayModeWidth && displayModeHeight && displayWidth && displayHeight &&
							displayModeWidth != displayWidth && displayModeHeight != displayHeight &&
							!isWindowed && IsWindow(MainhWnd) && !IsIconic(MainhWnd) &&
							GetCursorPos(&p))
						{
							mousePos.x = min(p.x, (LONG)displayModeWidth - 1);
							mousePos.y = min(p.y, (LONG)displayModeHeight - 1);

							if (mousePos.x != p.x || mousePos.y != p.y)
							{
								bMouseChange = true;
								SetEvent(ghWriteEvent);
							}
						}
					}
					return CallNextHookEx(nullptr, nCode, wParam, lParam);
				}
			};

			Logging::Log() << __FUNCTION__ << " Hooking mouse cursor!";
			m_hook = SetWindowsHookEx(WH_MOUSE_LL, WindowsMouseHook::mouseHookProc, hModule_dll, 0);
		}

		// Start thread
		if (!threadID && EnableMouseHook)
		{
			// A thread to bypass Windows preventing hooks from modifying mouse position
			struct WindowsMouseThread
			{
				static DWORD WINAPI setMousePosThread(LPVOID)
				{
					DWORD dwWaitResult = 0;
					do {
						dwWaitResult = WaitForSingleObject(ghWriteEvent, INFINITE);
						if (bMouseChange)
						{
							SetCursorPos(mousePos.x, mousePos.y);
							bMouseChange = false;
						}
					} while (!Config.Exiting && dwWaitResult == WAIT_OBJECT_0);

					// Unhook mouse
					if (m_hook)
					{
						UnhookWindowsHookEx(m_hook);
						m_hook = nullptr;
					}

					// Close handle
					if (ghWriteEvent)
					{
						CloseHandle(ghWriteEvent);
						ghWriteEvent = nullptr;
					}

					threadID = nullptr;
					return 0;
				}
			};

			threadID = CreateThread(nullptr, 0, WindowsMouseThread::setMousePosThread, nullptr, 0, nullptr);
		}

		// Create event
		if (!ghWriteEvent && EnableMouseHook)
		{
			ghWriteEvent = CreateEvent(nullptr, FALSE, FALSE, TEXT("Local\\DxwrapperMouseEvent"));
		}

		// Prepair shared memory
		m_IDirectDrawSurfaceX::StartSharedEmulatedMemory();
	}

	ReleaseCriticalSection();

	// Check interface to create d3d9 object
	CheckInterface(__FUNCTION__, false);
}

void m_IDirectDrawX::ReleaseDdraw()
{
	WrapperInterface->DeleteMe();
	WrapperInterface2->DeleteMe();
	WrapperInterface3->DeleteMe();
	WrapperInterface4->DeleteMe();
	WrapperInterface7->DeleteMe();

	if (g_hook)
	{
		UnhookWindowsHookEx(g_hook);
	}

	if (!Config.Dd7to9 || Config.Exiting)
	{
		return;
	}

	DWORD ddref = InterlockedDecrement(&ddrawRefCount);

	SetCriticalSection();

	// Clear SetBy handles
	if (MainSetBy == this)
	{
		MainSetBy = nullptr;
	}
	if (ExclusiveSetBy == this)
	{
		ExclusiveSetBy = nullptr;
	}

	// Remove ddraw device from vector
	auto it = std::find_if(DDrawVector.begin(), DDrawVector.end(),
		[=](auto pDDraw) -> bool { return pDDraw == this; });

	if (it != std::end(DDrawVector))
	{
		DDrawVector.erase(it);
	}

	// Release Direct3DDevice interfaces
	if (D3DDeviceInterface)
	{
		D3DDeviceInterface->ClearDdraw();
		D3DDeviceInterface = nullptr;
	}

	// Release Direct3D interfaces
	if (D3DInterface)
	{
		D3DInterface->ClearDdraw();
		D3DInterface = nullptr;
	}

	// Release surfaces
	for (m_IDirectDrawSurfaceX *pSurface : SurfaceVector)
	{
		pSurface->ReleaseD9Surface(false);
		pSurface->ClearDdraw();
	}
	SurfaceVector.clear();

	// Release Clippers
	for (m_IDirectDrawClipper* pClipper : ClipperVector)
	{
		pClipper->ClearDdraw();
	}
	ClipperVector.clear();

	// Release palettes
	for (m_IDirectDrawPalette *pPalette : PaletteVector)
	{
		pPalette->ClearDdraw();
	}
	PaletteVector.clear();

	// Release vertex buffers
	for (m_IDirect3DVertexBufferX* pVertexBuffer : VertexBufferVector)
	{
		pVertexBuffer->ReleaseD9Buffers(false);
		pVertexBuffer->ClearDdraw();
	}
	VertexBufferVector.clear();

	// Release color control
	if (ColorControlInterface)
	{
		ColorControlInterface->ClearDdraw();
	}

	// Release gamma control
	if (GammaControlInterface)
	{
		GammaControlInterface->ClearDdraw();
	}

	if (ddref == 0)
	{
		// Release all resources
		ReleaseAllD9Resources(false);

		// Release d3d9device
		if (d3d9Device)
		{
			ReleaseD3D9Device();
		}

		// Release d3d9object
		if (d3d9Object)
		{
			ReleaseD3D9Object();
		}

		// Close DDI
		CloseD3DDDI();

		// Release DC
		if (MainhWnd && MainhDC)
		{
			ReleaseDC(MainhWnd, MainhDC);
			MainhDC = nullptr;
		}

		// Clean up shared memory
		m_IDirectDrawSurfaceX::CleanupSharedEmulatedMemory();
	}

	ReleaseCriticalSection();
}

HWND m_IDirectDrawX::GetHwnd()
{
	return MainhWnd;
}

HDC m_IDirectDrawX::GetDC()
{
	return MainhDC;
}

void m_IDirectDrawX::ClearDepthStencilSurface()
{
	DepthStencilSurface = D3DFMT_UNKNOWN;
}

D3DMULTISAMPLE_TYPE m_IDirectDrawX::GetMultiSampleType()
{
	return presParams.MultiSampleType;

}
DWORD m_IDirectDrawX::GetMultiSampleQuality()
{
	return presParams.MultiSampleQuality;
}

bool m_IDirectDrawX::IsExclusiveMode()
{
	return ExclusiveMode;
}

void m_IDirectDrawX::GetFullDisplay(DWORD &Width, DWORD &Height, DWORD& BPP, DWORD &RefreshRate)
{
	// Init settings
	Width = 0;
	Height = 0;
	RefreshRate = 0;
	BPP = 0;

	// Get hwnd
	HWND hWnd = GetHwnd();

	// Width, Height, RefreshMode
	if (ExclusiveMode && ExclusiveWidth && ExclusiveHeight && ExclusiveBPP)
	{
		Width = ExclusiveWidth;
		Height = ExclusiveHeight;
		RefreshRate = ExclusiveRefreshRate;
		BPP = ExclusiveBPP;
	}
	else if (displayModeWidth && displayModeHeight && displayModeBPP)
	{
		Width = displayModeWidth;
		Height = displayModeHeight;
		RefreshRate = displayModeRefreshRate;
		BPP = displayModeBPP;
	}
	else if (d3d9Device && !(isWindowed && Config.DdrawWriteToGDI))
	{
		Width = presParams.BackBufferWidth;
		Height = presParams.BackBufferHeight;
		BPP = Utils::GetBitCount(hWnd);
	}
	else if (viewportWidth && viewportHeight)
	{
		Width = viewportWidth;
		Height = viewportHeight;
		BPP = Utils::GetBitCount(hWnd);
	}
	else if (surfaceWidth && surfaceHeight)
	{
		Width = surfaceWidth;
		Height = surfaceHeight;
		BPP = Utils::GetBitCount(hWnd);
	}
	else if (isWindowed && IsWindow(hWnd) && !Config.DdrawWriteToGDI)
	{
		RECT Rect = {};
		GetClientRect(hWnd, &Rect);
		Width = Rect.right - Rect.left;
		Height = Rect.bottom - Rect.top;
		BPP = Utils::GetBitCount(hWnd);
	}
	else
	{
		Utils::GetScreenSize(hWnd, Width, Height);
		RefreshRate = Utils::GetRefreshRate(hWnd);
		BPP = Utils::GetBitCount(hWnd);
	}

	// Force color mode
	BPP = (Config.DdrawOverrideBitMode) ? Config.DdrawOverrideBitMode : BPP;

	// Check if resolution changed
	if ((LastWidth && LastHeight && Width && Height && LastWidth != Width && LastHeight != Height) ||
		LastBPP && BPP && LastBPP != BPP)
	{
		RestoreAllSurfaces();
	}
	LastWidth = Width;
	LastHeight = Height;
	LastBPP = BPP;
}

void m_IDirectDrawX::GetDisplayPixelFormat(DDPIXELFORMAT &ddpfPixelFormat, DWORD BPP)
{
	ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	if (BPP == DisplayPixelFormat.dwRGBBitCount)
	{
		ddpfPixelFormat = DisplayPixelFormat;
	}
	else
	{
		SetDisplayFormat(ddpfPixelFormat, BPP);
	}
}

void m_IDirectDrawX::GetDisplay(DWORD &Width, DWORD &Height)
{
	Width = presParams.BackBufferWidth;
	Height = presParams.BackBufferHeight;
}

void m_IDirectDrawX::SetNewViewport(DWORD Width, DWORD Height)
{
	if (Width && Height && !displayWidth && !displayHeight)
	{
		viewportWidth = Width;
		viewportHeight = Height;

		// Check if there is a change in the present parameters
		if (d3d9Device && (viewportWidth != presParams.BackBufferWidth || viewportHeight != presParams.BackBufferHeight))
		{
			// Recreate d3d9 device
			CreateD3D9Device();
		}
	}
}

HRESULT m_IDirectDrawX::CheckInterface(char *FunctionName, bool CheckD3DDevice)
{
	// Check for object, if not then create it
	if (!d3d9Object)
	{
		// Create d3d9 object
		if (FAILED(CreateD3D9Object()))
		{
			LOG_LIMIT(100, FunctionName << " Error: d3d9 object not setup!");
			return DDERR_GENERIC;
		}
	}

	// Check for device, if not then create it
	if (CheckD3DDevice && !d3d9Device)
	{
		// Create d3d9 device
		if (FAILED(CreateD3D9Device()))
		{
			LOG_LIMIT(100, FunctionName << " Error: d3d9 device not setup!");
			return DDERR_GENERIC;
		}
	}

	return DD_OK;
}

bool m_IDirectDrawX::CheckD3D9Device()
{
	if (!d3d9Device && FAILED(CreateD3D9Device()))
	{
		return false;
	}
	return true;
}

LPDIRECT3D9 m_IDirectDrawX::GetDirect3D9Object()
{
	return d3d9Object;
}

LPDIRECT3DDEVICE9* m_IDirectDrawX::GetDirect3D9Device()
{
	return &d3d9Device;
}

LPDIRECT3DPIXELSHADER9* m_IDirectDrawX::GetPaletteShader()
{
	// Create pixel shaders
	if (d3d9Device && !palettePixelShader)
	{
		d3d9Device->CreatePixelShader((DWORD*)PalettePixelShaderSrc, &palettePixelShader);
	}
	return &palettePixelShader;
}

LPDIRECT3DPIXELSHADER9* m_IDirectDrawX::GetColorKeyShader()
{
	// Create pixel shaders
	if (d3d9Device && !colorkeyPixelShader)
	{
		d3d9Device->CreatePixelShader((DWORD*)ColorKeyPixelShaderSrc, &colorkeyPixelShader);
	}
	return &colorkeyPixelShader;
}

// Creates d3d9 device, destroying the old one if exists
HRESULT m_IDirectDrawX::CreateD3D9Device()
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, false)))
	{
		return DDERR_GENERIC;
	}

	SetCriticalSection();

	HRESULT hr = DD_OK;
	do {
		// Last call variables
		HWND LastHWnd = hFocusWindow;
		BOOL LastWindowedMode = presParams.Windowed;
		DWORD LastBehaviorFlags = BehaviorFlags;

		// Get hwnd
		HWND hWnd = GetHwnd();

		// Store new focus window
		hFocusWindow = hWnd;

		// Check window handle thread
		if (IsWindow(hWnd) && GetWindowThreadProcessId(hWnd, nullptr) != GetCurrentThreadId())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: trying to create Direct3D9 device from a different thread than the hwnd was created from!");
		}

		// Get current resolution
		DWORD CurrentWidth, CurrentHeight;
		Utils::GetScreenSize(hWnd, CurrentWidth, CurrentHeight);

		// Get width and height
		DWORD BackBufferWidth = displayWidth;
		DWORD BackBufferHeight = displayHeight;
		if (!BackBufferWidth || !BackBufferHeight)
		{
			if (viewportWidth && viewportHeight)
			{
				BackBufferWidth = viewportWidth;
				BackBufferHeight = viewportHeight;
			}
			else if (surfaceWidth && surfaceHeight)
			{
				BackBufferWidth = surfaceWidth;
				BackBufferHeight = surfaceHeight;
			}
			else if (isWindowed && IsWindow(hWnd))
			{
				RECT Rect = {};
				GetClientRect(hWnd, &Rect);
				BackBufferWidth = Rect.right - Rect.left;
				BackBufferHeight = Rect.bottom - Rect.top;
			}
			if (!BackBufferWidth || !BackBufferHeight)
			{
				BackBufferWidth = CurrentWidth;
				BackBufferHeight = CurrentHeight;
			}
		}

		// Set display window
		ZeroMemory(&presParams, sizeof(presParams));

		// Width/height
		presParams.BackBufferWidth = BackBufferWidth;
		presParams.BackBufferHeight = BackBufferHeight;
		// Discard swap
		presParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
		// Backbuffer
		presParams.BackBufferCount = 1;
		// Auto stencel format
		presParams.AutoDepthStencilFormat = (Config.DdrawOverrideStencilFormat) ? (D3DFORMAT)Config.DdrawOverrideStencilFormat : DepthStencilSurface;
		// Auto stencel
		presParams.EnableAutoDepthStencil = (presParams.AutoDepthStencilFormat) ? TRUE : FALSE;
		// Interval level
		presParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		// Anti-aliasing
		presParams.MultiSampleType = D3DMULTISAMPLE_NONE;
		presParams.MultiSampleQuality = 0;
		// Present flags
		presParams.Flags = 0;

		// Set parameters for the current display mode
		if (isWindowed || !hWnd)
		{
			// Window mode
			presParams.Windowed = TRUE;
			// Backbuffer
			presParams.BackBufferFormat = D3DFMT_UNKNOWN;
			// Display mode refresh
			presParams.FullScreen_RefreshRateInHz = 0;
			// Window handle
			presParams.hDeviceWindow = hWnd;
		}
		else
		{
			// Fullscreen
			presParams.Windowed = FALSE;
			// Backbuffer
			presParams.BackBufferFormat = D9DisplayFormat;
			// Display mode refresh
			presParams.FullScreen_RefreshRateInHz = displayRefreshRate;
			// Window handle
			presParams.hDeviceWindow = nullptr;
		}

		// Enable antialiasing
		if (AntiAliasing)
		{
			DWORD QualityLevels = 0;

			// Check AntiAliasing quality
			if (SUCCEEDED(d3d9Object->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D9DisplayFormat, presParams.Windowed, D3DMULTISAMPLE_NONMASKABLE, &QualityLevels)))
			{
				presParams.MultiSampleType = D3DMULTISAMPLE_NONMASKABLE;
				presParams.MultiSampleQuality = (QualityLevels) ? QualityLevels - 1 : 0;
			}
		}

		// Check device caps for vertex processing support
		D3DCAPS9 d3dcaps;
		ZeroMemory(&d3dcaps, sizeof(D3DCAPS9));
		hr = d3d9Object->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dcaps);
		if (FAILED(hr))
		{
			Logging::Log() << __FUNCTION__ << " Failed to get Direct3D9 device caps: " << (DDERR)hr;
		}

		// Set behavior flags
		BehaviorFlags = ((d3dcaps.VertexProcessingCaps) ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING) |
			((MultiThreaded || !Config.SingleProcAffinity) ? D3DCREATE_MULTITHREADED : 0) |
			((FPUPreserve) ? D3DCREATE_FPU_PRESERVE : 0) |
			((NoWindowChanges) ? D3DCREATE_NOWINDOWCHANGES : 0);

		Logging::Log() << __FUNCTION__ << " Direct3D9 device! " <<
			presParams.BackBufferWidth << "x" << presParams.BackBufferHeight << " refresh: " << presParams.FullScreen_RefreshRateInHz <<
			" format: " << presParams.BackBufferFormat << " wnd: " << hWnd << " params: " << presParams << " flags: " << Logging::hex(BehaviorFlags);

		// Check if existing device exists
		if (d3d9Device)
		{
			// Try to reset existing device
			if (LastHWnd == hWnd && LastWindowedMode == presParams.Windowed && LastBehaviorFlags == BehaviorFlags)
			{
				// Prepare for reset
				ReleaseAllD9Resources(true);

				// Reset device. When this method returns: BackBufferCount, BackBufferWidth, and BackBufferHeight are set to zero.
				D3DPRESENT_PARAMETERS newParams = presParams;
				hr = d3d9Device->Reset(&newParams);

				// Resetting the device failed
				if (FAILED(hr))
				{
					Logging::Log() << __FUNCTION__ << " Failed to reset device! " << (D3DERR)hr << " Last create: " << LastHWnd << "->" << hWnd << " " <<
						" Windowed: " << LastWindowedMode << "->" << presParams.Windowed <<
						Logging::hex(LastBehaviorFlags) << "->" << Logging::hex(BehaviorFlags);

					ReleaseD3D9Device();
				}
			}
			// Release existing device
			else
			{
				Logging::Log() << __FUNCTION__ << " Recreate device! Last create: " << LastHWnd << "->" << hWnd << " " <<
					" Windowed: " << LastWindowedMode << "->" << presParams.Windowed << " " <<
					Logging::hex(LastBehaviorFlags) << "->" << Logging::hex(BehaviorFlags);

				ReleaseAllD9Resources(true);
				ReleaseD3D9Device();
			}
		}

		// Create d3d9 Device
		if (!d3d9Device)
		{
			hr = d3d9Object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, BehaviorFlags, &presParams, &d3d9Device);
		}
		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create Direct3D9 device! " << (DDERR)hr << " " <<
				presParams.BackBufferWidth << "x" << presParams.BackBufferHeight << " refresh: " << presParams.FullScreen_RefreshRateInHz <<
				" format: " << presParams.BackBufferFormat << " wnd: " << hWnd << " params: " << presParams << " flags: " << Logging::hex(BehaviorFlags));
			break;
		}

		// Reset flags after creating device
		EnableWaitVsync = false;
		FourCCsList.clear();

		// Set window pos
		if (IsWindow(hWnd))
		{
			// Get new resolution
			DWORD NewWidth, NewHeight;
			Utils::GetScreenSize(hWnd, NewWidth, NewHeight);

			// Send display change message
			if ((SetResolution || NewWidth != CurrentWidth || NewHeight != CurrentHeight) && NewWidth && NewHeight)
			{
				SetResolution = false;
				DWORD bpp = (displayModeBPP) ? displayModeBPP : 32;
				DWORD res = (WORD)NewWidth | ((WORD)NewHeight << 16);
				SendMessage(hWnd, WM_DISPLAYCHANGE, (WPARAM)bpp, (LPARAM)res);
			}

			// Get window size
			RECT NewRect = {};
			GetWindowRect(hWnd, &NewRect);

			// Send message about window changes
			WINDOWPOS winpos = { hWnd, HWND_TOP, NewRect.left, NewRect.top, NewRect.right - NewRect.left, NewRect.bottom - NewRect.top, WM_NULL };
			SendMessage(hWnd, WM_WINDOWPOSCHANGED, (WPARAM)TRUE, (LPARAM)&winpos);

			// Peek messages to help prevent a "Not Responding" window
			Utils::CheckMessageQueue(hWnd);
		}

		// Store display frequency
		monitorRefreshRate = (presParams.FullScreen_RefreshRateInHz) ? presParams.FullScreen_RefreshRateInHz : Utils::GetRefreshRate(hWnd);
		DWORD tmpWidth = 0;
		Utils::GetScreenSize(hWnd, tmpWidth, monitorHeight);

	} while (false);

	// Reset D3D device settings
	if (D3DDeviceInterface)
	{
		D3DDeviceInterface->ResetDevice();
	}

	ReleaseCriticalSection();

	// Return result
	return hr;
}

// Creates d3d9 object
HRESULT m_IDirectDrawX::CreateD3D9Object()
{
	// Create d3d9 object
	if (!d3d9Object)
	{
		// Declare Direct3DCreate9
		static Direct3DCreate9Proc Direct3DCreate9 = reinterpret_cast<Direct3DCreate9Proc>(Direct3DCreate9_out);

		if (!Direct3DCreate9)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get 'Direct3DCreate9' ProcAddress of d3d9.dll!");
			return DDERR_GENERIC;
		}

		d3d9Object = Direct3DCreate9(D3D_SDK_VERSION);

		// Error creating Direct3D9
		if (!d3d9Object)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: d3d9 object not setup!");
			return DDERR_GENERIC;
		}
	}

	return D3D_OK;
}

// Reinitialize d3d9 device
HRESULT m_IDirectDrawX::ReinitDevice()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true)))
	{
		return DDERR_GENERIC;
	}

	// Check if device is ready to be restored
	HRESULT hr = d3d9Device->TestCooperativeLevel();
	if (SUCCEEDED(hr))
	{
		return DD_OK;
	}
	else if (hr == D3DERR_DEVICELOST)
	{
		return DDERR_SURFACELOST;
	}
	else if (hr != D3DERR_DEVICENOTRESET && hr != D3DERR_DRIVERINTERNALERROR)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: TestCooperativeLevel = " << (D3DERR)hr);
		return DDERR_GENERIC;
	}

	SetCriticalSection();

	do {
		// Prepare for reset
		ReleaseAllD9Resources(true);

		// Reset device. When this method returns: BackBufferCount, BackBufferWidth, and BackBufferHeight are set to zero.
		D3DPRESENT_PARAMETERS newParams = presParams;
		hr = d3d9Device->Reset(&newParams);
		if (hr == D3DERR_DEVICEREMOVED || hr == D3DERR_DRIVERINTERNALERROR)
		{
			ReleaseD3D9Device();
			ReleaseD3D9Object();
			CreateD3D9Object();
			hr = CreateD3D9Device();
		}

		// Attempt to reset the device
		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to reset Direct3D9 device: " << (D3DERR)hr);
			hr = DDERR_GENERIC;
			break;
		}

		// Reset flags after resetting device
		EnableWaitVsync = false;

	} while (false);

	ReleaseCriticalSection();

	// Success
	return hr;
}

// Release all dd9 resources
inline void m_IDirectDrawX::ReleaseAllD9Resources(bool BackupData)
{
	ReleaseAllD9Buffers(BackupData);
	ReleaseAllD9Surfaces(BackupData);
	ReleaseAllD9Shaders();
}

// Release all surfaces from all ddraw devices
inline void m_IDirectDrawX::ReleaseAllD9Surfaces(bool BackupData)
{
	SetCriticalSection();

	for (m_IDirectDrawX *pDDraw : DDrawVector)
	{
		for (m_IDirectDrawSurfaceX* pSurface : pDDraw->SurfaceVector)
		{
			pSurface->ReleaseD9Surface(BackupData);
		}
	}

	ReleaseCriticalSection();
}

// Release all buffers from all ddraw devices
inline void m_IDirectDrawX::ReleaseAllD9Buffers(bool BackupData)
{
	SetCriticalSection();

	for (m_IDirectDrawX* pDDraw : DDrawVector)
	{
		for (m_IDirect3DVertexBufferX* pBuffer : pDDraw->VertexBufferVector)
		{
			pBuffer->ReleaseD9Buffers(BackupData);
		}
	}

	ReleaseCriticalSection();
}

// Release all shaders
inline void m_IDirectDrawX::ReleaseAllD9Shaders()
{
	// Release palette pixel shader
	if (palettePixelShader)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 palette pixel shader";
		if (d3d9Device)
		{
			d3d9Device->SetPixelShader(nullptr);
		}
		ULONG ref = palettePixelShader->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'palettePixelShader' " << ref;
		}
		palettePixelShader = nullptr;
	}

	// Release color key pixel shader
	if (colorkeyPixelShader)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 color key pixel shader";
		ULONG ref = colorkeyPixelShader->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'colorkeyPixelShader' " << ref;
		}
		colorkeyPixelShader = nullptr;
	}
}

// Release all d3d9 device
void m_IDirectDrawX::ReleaseD3D9Device()
{
	// Release device
	if (d3d9Device)
	{
		ULONG ref = d3d9Device->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'd3d9Device' " << ref;
		}
		d3d9Device = nullptr;
	}

	// Reset flags
	EnableWaitVsync = false;
	FourCCsList.clear();
}

// Release d3d9 object
void m_IDirectDrawX::ReleaseD3D9Object()
{
	if (d3d9Object)
	{
		ULONG ref = d3d9Object->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'd3d9Object' " << ref;
		}
		d3d9Object = nullptr;
	}
}

// Add surface wrapper to vector
void m_IDirectDrawX::AddSurfaceToVector(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX || DoesSurfaceExist(lpSurfaceX))
	{
		return;
	}

	SetCriticalSection();

	if (lpSurfaceX->IsPrimarySurface())
	{
		PrimarySurface = lpSurfaceX;
	}

	// Store surface
	SurfaceVector.push_back(lpSurfaceX);

	ReleaseCriticalSection();
}

// Remove surface wrapper from vector
void m_IDirectDrawX::RemoveSurfaceFromVector(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return;
	}

	SetCriticalSection();

	if (lpSurfaceX == PrimarySurface)
	{
		PrimarySurface = nullptr;
		ZeroMemory(&DisplayPixelFormat, sizeof(DDPIXELFORMAT));
	}

	auto it = std::find_if(SurfaceVector.begin(), SurfaceVector.end(),
		[=](auto pSurface) -> bool { return pSurface == lpSurfaceX; });

	if (it != std::end(SurfaceVector))
	{
		SurfaceVector.erase(it);

		// Clear primary surface
		if (lpSurfaceX == PrimarySurface)
		{
			PrimarySurface = nullptr;
			ZeroMemory(&DisplayPixelFormat, sizeof(DDPIXELFORMAT));
		}
	}

	// Remove attached surface from map
	for (m_IDirectDrawSurfaceX *pSurface : SurfaceVector)
	{
		pSurface->RemoveAttachedSurfaceFromMap(lpSurfaceX);
	}

	ReleaseCriticalSection();
}

// Check if surface wrapper exists
bool m_IDirectDrawX::DoesSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return false;
	}

	bool hr = false;

	SetCriticalSection();

	auto it = std::find_if(SurfaceVector.begin(), SurfaceVector.end(),
		[=](auto pSurface) -> bool { return pSurface == lpSurfaceX; });

	if (it != std::end(SurfaceVector))
	{
		hr = true;
	}

	ReleaseCriticalSection();

	return hr;
}

// This method removes any texture surfaces created with the DDSCAPS2_TEXTUREMANAGE or DDSCAPS2_D3DTEXTUREMANAGE flags
void m_IDirectDrawX::EvictManagedTextures()
{
	SetCriticalSection();

	// Check if any surfaces are locked
	for (m_IDirectDrawSurfaceX *pSurface : SurfaceVector)
	{
		if (pSurface->IsSurfaceManaged())
		{
			pSurface->ReleaseD9Surface(true);
		}
	}

	ReleaseCriticalSection();
}

// Add clipper wrapper to vector
void m_IDirectDrawX::AddClipperToVector(m_IDirectDrawClipper* lpClipper)
{
	if (!lpClipper || DoesClipperExist(lpClipper))
	{
		return;
	}

	SetCriticalSection();

	// Store clipper
	ClipperVector.push_back(lpClipper);

	ReleaseCriticalSection();
}

// Remove clipper wrapper from vector
void m_IDirectDrawX::RemoveClipperFromVector(m_IDirectDrawClipper* lpClipper)
{
	if (!lpClipper)
	{
		return;
	}

	// Remove standalone clipper
	RemoveBaseClipperFromVector(lpClipper);

	SetCriticalSection();

	auto it = std::find_if(ClipperVector.begin(), ClipperVector.end(),
		[=](auto pClipper) -> bool { return pClipper == lpClipper; });

	// Remove clipper from vector
	if (it != std::end(ClipperVector))
	{
		ClipperVector.erase(it);
	}

	// Remove clipper from attached surface
	for (m_IDirectDrawX* pDDraw : DDrawVector)
	{
		for (m_IDirectDrawSurfaceX* pSurface : pDDraw->SurfaceVector)
		{
			pSurface->RemoveClipper(lpClipper);
		}
	}

	ReleaseCriticalSection();
}

// Check if clipper wrapper exists
bool m_IDirectDrawX::DoesClipperExist(m_IDirectDrawClipper* lpClipper)
{
	if (!lpClipper)
	{
		return false;
	}

	// Check standalone clipper
	if (DoesBaseClipperExist(lpClipper))
	{
		return true;
	}

	bool hr = false;

	SetCriticalSection();

	auto it = std::find_if(ClipperVector.begin(), ClipperVector.end(),
		[=](auto pClipper) -> bool { return pClipper == lpClipper; });

	if (it != std::end(ClipperVector))
	{
		hr = true;
	}

	ReleaseCriticalSection();

	return hr;
}

// Add palette wrapper to vector
void m_IDirectDrawX::AddPaletteToVector(m_IDirectDrawPalette* lpPalette)
{
	if (!lpPalette || DoesPaletteExist(lpPalette))
	{
		return;
	}

	SetCriticalSection();

	// Store palette
	PaletteVector.push_back(lpPalette);

	ReleaseCriticalSection();
}

// Remove palette wrapper from vector
void m_IDirectDrawX::RemovePaletteFromVector(m_IDirectDrawPalette* lpPalette)
{
	if (!lpPalette)
	{
		return;
	}

	SetCriticalSection();

	auto it = std::find_if(PaletteVector.begin(), PaletteVector.end(),
		[=](auto pPalette) -> bool { return pPalette == lpPalette; });

	// Remove palette from vector
	if (it != std::end(PaletteVector))
	{
		PaletteVector.erase(it);
	}

	// Remove palette from attached surface
	for (m_IDirectDrawX* pDDraw : DDrawVector)
	{
		for (m_IDirectDrawSurfaceX* pSurface : pDDraw->SurfaceVector)
		{
			pSurface->RemovePalette(lpPalette);
		}
	}

	ReleaseCriticalSection();
}

// Check if palette wrapper exists
bool m_IDirectDrawX::DoesPaletteExist(m_IDirectDrawPalette* lpPalette)
{
	if (!lpPalette)
	{
		return false;
	}

	bool hr = false;

	SetCriticalSection();

	auto it = std::find_if(PaletteVector.begin(), PaletteVector.end(),
		[=](auto pPalette) -> bool { return pPalette == lpPalette; });

	if (it != std::end(PaletteVector))
	{
		hr = true;
	}

	ReleaseCriticalSection();

	return hr;
}

void m_IDirectDrawX::AddVertexBufferToVector(m_IDirect3DVertexBufferX* lpVertexBuffer)
{
	if (!lpVertexBuffer || DoesVertexBufferExist(lpVertexBuffer))
	{
		return;
	}

	SetCriticalSection();

	// Store vertex buffer
	VertexBufferVector.push_back(lpVertexBuffer);

	ReleaseCriticalSection();
}

void m_IDirectDrawX::RemoveVertexBufferFromVector(m_IDirect3DVertexBufferX* lpVertexBuffer)
{
	if (!lpVertexBuffer)
	{
		return;
	}

	SetCriticalSection();

	auto it = std::find_if(VertexBufferVector.begin(), VertexBufferVector.end(),
		[=](auto pVertexBuffer) -> bool { return pVertexBuffer == lpVertexBuffer; });

	// Remove vertex buffer from vector
	if (it != std::end(VertexBufferVector))
	{
		VertexBufferVector.erase(it);
	}

	ReleaseCriticalSection();
}

bool m_IDirectDrawX::DoesVertexBufferExist(m_IDirect3DVertexBufferX* lpVertexBuffer)
{
	if (!lpVertexBuffer)
	{
		return false;
	}

	bool hr = false;

	SetCriticalSection();

	auto it = std::find_if(VertexBufferVector.begin(), VertexBufferVector.end(),
		[=](auto pVertexBuffer) -> bool { return pVertexBuffer == lpVertexBuffer; });

	if (it != std::end(VertexBufferVector))
	{
		hr = true;
	}

	ReleaseCriticalSection();

	return hr;
}

HRESULT m_IDirectDrawX::CreateColorInterface(LPVOID *ppvObj)
{
	if (!ppvObj)
	{
		return DDERR_GENERIC;
	}

	if (!ColorControlInterface)
	{
		ColorControlInterface = new m_IDirectDrawColorControl(this);
	}

	*ppvObj = ColorControlInterface;

	return DD_OK;
}

HRESULT m_IDirectDrawX::CreateGammaInterface(LPVOID *ppvObj)
{
	if (!ppvObj)
	{
		return DDERR_GENERIC;
	}

	if (!GammaControlInterface)
	{
		GammaControlInterface = new m_IDirectDrawGammaControl(this);
	}

	*ppvObj = GammaControlInterface;

	return DD_OK;
}

// Adjusts available memory, some games have issues if this is set to high
void m_IDirectDrawX::AdjustVidMemory(LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
	DWORD TotalVidMem = (lpdwTotal && *lpdwTotal) ? *lpdwTotal : (lpdwFree && *lpdwFree) ? *lpdwFree + MinUsedVidMemory : MaxVidMemory;
	TotalVidMem = min(TotalVidMem, MaxVidMemory);
	DWORD AvailVidMem = (lpdwFree && *lpdwFree) ? *lpdwFree : TotalVidMem - MinUsedVidMemory;
	AvailVidMem = min(AvailVidMem, TotalVidMem - MinUsedVidMemory);
	if (lpdwTotal && *lpdwTotal)
	{
		*lpdwTotal = TotalVidMem;
	}
	if (lpdwFree && *lpdwFree)
	{
		*lpdwFree = AvailVidMem;
	}
}

void m_IDirectDrawX::SetVsync()
{
	if (!Config.ForceVsyncMode)
	{
		EnableWaitVsync = true;
	}
}

// Do d3d9 Present
HRESULT m_IDirectDrawX::Present()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true)))
	{
		return DDERR_GENERIC;
	}

	const bool UseVSync = (EnableWaitVsync && !Config.EnableVSync);

	// Skip frame if time lapse is too small
	if (Config.AutoFrameSkip && !UseVSync)
	{
		if (FrequencyFlag)
		{
			FrameCounter++;

			// Get screen frequency timer
			float MaxScreenTimer = (1000.0f / monitorRefreshRate);

			// Get time since last successful endscene
			bool CounterFlag = (QueryPerformanceCounter(&clickTime) != 0);
			float deltaPresentMS = ((clickTime.QuadPart - lastPresentTime.QuadPart) * 1000.0f) / clockFrequency.QuadPart;

			// Get time since last skipped frame
			float deltaFrameMS = (lastFrameTime) ? ((clickTime.QuadPart - lastFrameTime) * 1000.0f) / clockFrequency.QuadPart : deltaPresentMS;
			lastFrameTime = clickTime.QuadPart;

			// Use last frame time and average frame time to decide if next frame will be less than the screen frequency timer
			if (CounterFlag && (deltaPresentMS + (deltaFrameMS * 1.1f) < MaxScreenTimer) && (deltaPresentMS + ((deltaPresentMS / FrameCounter) * 1.1f) < MaxScreenTimer))
			{
				Logging::LogDebug() << __FUNCTION__ << " Skipping frame " << deltaPresentMS << "ms screen frequancy " << MaxScreenTimer;
				return D3D_OK;
			}
			Logging::LogDebug() << __FUNCTION__ << " Drawing frame " << deltaPresentMS << "ms screen frequancy " << MaxScreenTimer;
		}
	}

	// Use WaitForVerticalBlank for wait timer
	if (UseVSync)
	{
		WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, nullptr);
		EnableWaitVsync = false;
	}

	// Present everthing, skip Preset when using DdrawWriteToGDI
	HRESULT hr = d3d9Device->Present(nullptr, nullptr, nullptr, nullptr);

	// Device lost
	if (hr == D3DERR_DEVICELOST)
	{
		// Attempt to reinit device
		hr = ReinitDevice();
	}
	else if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to present scene");
	}

	// Store new click time after frame draw is complete
	if (SUCCEEDED(hr) && Config.AutoFrameSkip)
	{
		if (QueryPerformanceCounter(&clickTime))
		{
			lastPresentTime.QuadPart = clickTime.QuadPart;
			lastFrameTime = 0;
			FrameCounter = 0;
		}
	}

	return hr;
}

DWORD GetDDrawBitsPixel()
{
	if (ddrawRefCount && MainhWnd)
	{
		return (ExclusiveBPP) ? ExclusiveBPP : (displayModeBPP) ? displayModeBPP : 0;
	}
	return 0;
}

DWORD GetDDrawWidth()
{
	if (ddrawRefCount && MainhWnd)
	{
		return (ExclusiveWidth) ? ExclusiveWidth : (displayModeWidth) ? displayModeWidth : 0;
	}
	return 0;
}

DWORD GetDDrawHeight()
{
	if (ddrawRefCount && MainhWnd)
	{
		return (ExclusiveHeight) ? ExclusiveHeight : (displayModeHeight) ? displayModeHeight : 0;
	}
	return 0;
}
