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
* Code taken from: https://github.com/strangebytes/diablo-ddrawwrapper
*/

#include "ddraw.h"
#include "ddrawExternal.h"
#include "Utils\Utils.h"
#include "GDI\GDI.h"
#include "Dllmain\DllMain.h"
#include "d3d9\d3d9External.h"
#include "d3dddi\d3dddiExternal.h"
#include "Shaders\PaletteShader.h"
#include "Shaders\ColorKeyShader.h"

constexpr DWORD MaxVidMemory		= 0x20000000;	// 512 MBs
constexpr DWORD MinUsedVidMemory	= 0x00100000;	// 1 MB

const D3DFORMAT D9DisplayFormat = D3DFMT_X8R8G8B8;

DWORD WINAPI PresentThreadFunction(LPVOID);

float ScaleDDWidthRatio = 1.0f;
float ScaleDDHeightRatio = 1.0f;
DWORD ScaleDDLastWidth = 0;
DWORD ScaleDDLastHeight = 0;
DWORD ScaleDDCurrentWidth = 0;
DWORD ScaleDDCurrentHeight = 0;
DWORD ScaleDDPadX = 0;
DWORD ScaleDDPadY = 0;

struct DRAWSTATEBACKUP
{
	DWORD ssMagFilter = 0;
	DWORD tsColorOP = 0;
	DWORD tsColorArg1 = 0;
	DWORD tsColorArg2 = 0;
	DWORD tsAlphaOP = 0;
	DWORD rsLighting = 0;
	DWORD rsAlphaBlendEnable = 0;
	DWORD rsAlphaTestEnable = 0;
	DWORD rsFogEnable = 0;
	DWORD rsZEnable = 0;
	DWORD rsZWriteEnable = 0;
	DWORD rsStencilEnable = 0;
};

// Mouse hook
struct MOUSEHOOK
{
	HANDLE ghWriteEvent = nullptr;
	HANDLE threadID = nullptr;
	HHOOK m_hook = nullptr;
	bool bChange = false;
	POINT Pos = {};
};

// Custom vertex
const DWORD TLVERTEXFVF = (D3DFVF_XYZRHW | D3DFVF_TEX1);
struct TLVERTEX
{
	float x, y, z, rhw;
	float u, v;
};

struct DISPLAYSETTINGS
{
	HWND hWnd;
	HDC DC;
	m_IDirectDrawX* SetBy;
	DWORD Width;
	DWORD Height;
	DWORD BPP;
	DWORD RefreshRate;
};

struct DEVICESETTINGS
{
	bool IsWindowed;
	bool AntiAliasing;
	bool AllowModeX;
	bool MultiThreaded;
	bool FPUPreserve;
	bool NoWindowChanges;
	DWORD Width;
	DWORD Height;
	DWORD RefreshRate;
};

struct HIGHRESCOUNTER
{
	LARGE_INTEGER Frequency = {};
	LARGE_INTEGER LastPresentTime = {};
	LONGLONG LastFrameTime = 0;
	DWORD FrameCounter = 0;
	DWORD FrameSkipCounter = 0;
	double PerFrameMS = 1000.0 / 60.0;
};

struct PRESENTTHREAD
{
	bool IsInitialized = false;
	CRITICAL_SECTION ddpt = {};
	HANDLE workerEvent = {};
	HANDLE workerThread = {};
	LARGE_INTEGER LastPresentTime = {};
	bool EnableThreadFlag = false;
};

// Store a list of ddraw devices
std::vector<m_IDirectDrawX*> DDrawVector;

// Default resolution
DWORD DefaultWidth;
DWORD DefaultHeight;
RECT LastWindowLocation;

// Exclusive mode settings
bool SetResolution;
bool ExclusiveMode;
DISPLAYSETTINGS Exclusive;
HWND LastUsedHWnd;

// Clipper
HWND ClipperHWnd;

// Display mode settings
DISPLAYSETTINGS DisplayMode;

// Device settings
DEVICESETTINGS Device;

// Display pixel format
DDPIXELFORMAT DisplayPixelFormat;

// Last used surface resolution
DWORD LastSetWidth;
DWORD LastSetHeight;
DWORD LastSetBPP;

// Initial screen resolution
DWORD InitWidth;
DWORD InitHeight;

// Cached FourCC list
std::vector<D3DFORMAT> FourCCsList;

// Mouse hook
MOUSEHOOK MouseHook;

// High resolution counter used for auto frame skipping
HIGHRESCOUNTER Counter;

// Preset from another thread
PRESENTTHREAD PresentThread;

inline void SetPTCriticalSection()
{
	if (PresentThread.IsInitialized)
	{
		EnterCriticalSection(&PresentThread.ddpt);
	}
}

inline void ReleasePTCriticalSection()
{
	if (PresentThread.IsInitialized)
	{
		LeaveCriticalSection(&PresentThread.ddpt);
	}
}

// Direct3D9 flags
bool EnableWaitVsync;

// Direct3D9 Objects
LPDIRECT3D9 d3d9Object;
LPDIRECT3DDEVICE9 d3d9Device;
LPDIRECT3DSURFACE9 d3d9RenderTarget;
D3DPRESENT_PARAMETERS presParams;
LPDIRECT3DPIXELSHADER9 palettePixelShader;
LPDIRECT3DPIXELSHADER9 colorkeyPixelShader;
LPDIRECT3DVERTEXBUFFER9 VertexBuffer;
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
		return E_NOINTERFACE;
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
			}
			ReleaseCriticalSection();

			return DD_OK;
		}
		if (riid == IID_IDirectDrawColorControl)
		{
			return SUCCEEDED(CreateColorInterface(ppvObj)) ? DD_OK : E_NOINTERFACE;
		}
		if (riid == IID_IDirectDrawGammaControl)
		{
			return SUCCEEDED(CreateGammaInterface(ppvObj)) ? DD_OK : E_NOINTERFACE;
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

	HRESULT hr = GetProxyInterfaceV3()->CreateSurface(lpDDSurfaceDesc, (LPDIRECTDRAWSURFACE*)lplpDDSurface, pUnkOuter);

	if (SUCCEEDED(hr))
	{
		m_IDirectDrawSurfaceX *D3DSurfaceDevice = new m_IDirectDrawSurfaceX((IDirectDrawSurface7*)*lplpDDSurface, DirectXVersion);

		*lplpDDSurface = (LPDIRECTDRAWSURFACE7)D3DSurfaceDevice->GetWrapperInterfaceX(DirectXVersion);
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

		// Check MipMap count
		if ((lpDDSurfaceDesc2->dwFlags & DDSD_MIPMAPCOUNT) && (lpDDSurfaceDesc2->dwMipMapCount != 1) &&
			(lpDDSurfaceDesc2->ddsCaps.dwCaps & (DDSCAPS_MIPMAP | DDSCAPS_COMPLEX | DDSCAPS_TEXTURE)) == (DDSCAPS_MIPMAP | DDSCAPS_COMPLEX | DDSCAPS_TEXTURE) &&
			GetMaxMipMapLevel(lpDDSurfaceDesc2->dwWidth, lpDDSurfaceDesc2->dwHeight) < lpDDSurfaceDesc2->dwMipMapCount)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: MipMap count too large. Count: " << lpDDSurfaceDesc2->dwMipMapCount <<
				" " << lpDDSurfaceDesc2->dwWidth << "x" << lpDDSurfaceDesc2->dwHeight << " Max: " << GetMaxMipMapLevel(lpDDSurfaceDesc2->dwWidth, lpDDSurfaceDesc2->dwHeight));
			return DDERR_INVALIDPARAMS;
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

		// Check for Overlay
		if ((lpDDSurfaceDesc2->dwFlags & (DDSD_CKDESTOVERLAY | DDSD_CKSRCOVERLAY)) || (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_OVERLAY))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Overlay not Implemented.");
		}

		// Check for own dc
		if (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_OWNDC)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: DDSCAPS_OWNDC not Implemented.");
		}

		// Check for unsupported flags
		DWORD UnsupportedDDSDFlags = (DDSD_ALPHABITDEPTH | DDSD_LINEARSIZE | DDSD_FVF | DDSD_SRCVBHANDLE | DDSD_DEPTH);
		if (lpDDSurfaceDesc2->dwFlags & UnsupportedDDSDFlags)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: non-supported dwFlags! " << Logging::hex(lpDDSurfaceDesc2->dwFlags & UnsupportedDDSDFlags));
		}

		// Check for unsupported ddsCaps
		DWORD UnsupportedDDSCaps = (DDSCAPS_LIVEVIDEO | DDSCAPS_HWCODEC | DDSCAPS_VIDEOPORT);
		DWORD UnsupportedDDSCaps2 = (DDSCAPS2_OPAQUE);	// DDSCAPS2_HINTDYNAMIC | DDSCAPS2_HINTSTATIC | DDSCAPS2_NOTUSERLOCKABLE
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

		bool ResetD3D9Device = false;

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
			const D3DRESOURCETYPE Resource = ((lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_TEXTURE)) ? D3DRTYPE_TEXTURE : D3DRTYPE_SURFACE;
			const D3DFORMAT Format = GetDisplayFormat(Desc2.ddpfPixelFormat);
			const D3DFORMAT TestFormat = ConvertSurfaceFormat(Format);

			if (FAILED(d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D9DisplayFormat, Usage, Resource, TestFormat)) &&
				FAILED(d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D9DisplayFormat, Usage, Resource, GetFailoverFormat(TestFormat))))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: non-supported pixel format! " << Usage << " " << Format << "->" << TestFormat << " " << Desc2.ddpfPixelFormat);
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
		if (!Desc2.dwWidth || !Desc2.dwHeight || !(Desc2.dwFlags & DDSD_WIDTH) || !(Desc2.dwFlags & DDSD_HEIGHT))
		{
			Desc2.dwFlags &= ~(DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH);
			Desc2.dwWidth = 0;
			Desc2.dwHeight = 0;
		}
		if (!Desc2.dwRefreshRate)
		{
			Desc2.dwFlags &= ~DDSD_REFRESHRATE;
		}

		// Check for depth stencil surface
		if (!Config.DdrawOverrideStencilFormat && (Desc2.dwFlags & DDSD_PIXELFORMAT) && (Desc2.ddpfPixelFormat.dwFlags & (DDPF_ZBUFFER | DDPF_STENCILBUFFER)))
		{
			if (Config.DdrawOverrideStencilFormat)
			{
				SetPixelDisplayFormat((D3DFORMAT)Config.DdrawOverrideStencilFormat, Desc2.ddpfPixelFormat);
			}

			Logging::Log() << __FUNCTION__ << " Found Stencil surface: " << GetDisplayFormat(Desc2.ddpfPixelFormat);
		}

		// Check if there is a change in the present parameters
		if ((Desc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) && !Device.Width && !Device.Height &&
			(Desc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT)) == (DDSD_WIDTH | DDSD_HEIGHT) &&
			(Desc2.dwWidth != presParams.BackBufferWidth || Desc2.dwHeight != presParams.BackBufferHeight))
		{
			ResetD3D9Device = true;
		}

		// Get present parameters
		if (Desc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
		{
			Logging::Log() << __FUNCTION__ << " Primary surface " << Desc2.dwWidth << "x" << Desc2.dwHeight <<
				" dwFlags: " << Logging::hex(Desc2.dwFlags) <<
				" ddsCaps: " << Logging::hex(Desc2.ddsCaps.dwCaps) << ", " << Logging::hex(Desc2.ddsCaps.dwCaps2) << ", " << LOWORD(Desc2.ddsCaps.dwVolumeDepth);
		}

		// Create interface
		m_IDirectDrawSurfaceX *p_IDirectDrawSurfaceX = new m_IDirectDrawSurfaceX(this, DirectXVersion, &Desc2);
		*lplpDDSurface = (LPDIRECTDRAWSURFACE7)p_IDirectDrawSurfaceX->GetWrapperInterfaceX(DirectXVersion);

		// Reset d3d9 device after creating interface if needed
		if (ResetD3D9Device && d3d9Device)
		{
			CreateD3D9Device();
		}

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

	HRESULT hr = ProxyInterface->CreateSurface(&Desc2, lplpDDSurface, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDSurface)
	{
		m_IDirectDrawSurfaceX *D3DSurfaceDevice = new m_IDirectDrawSurfaceX((IDirectDrawSurface7*)*lplpDDSurface, DirectXVersion);

		*lplpDDSurface = (LPDIRECTDRAWSURFACE7)D3DSurfaceDevice->GetWrapperInterfaceX(DirectXVersion);

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
		lpDDSurfaceX->GetSurfaceDesc2(&Desc2, 0, DirectXVersion);
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

		// ToDo: need to support DDEDM_STANDARDVGAMODES
		//bool UseStandardVGAModes = ((dwFlags & DDEDM_STANDARDVGAMODES) || Device.AllowModeX);

		// Save width and height
		DWORD EnumWidth = (lpDDSurfaceDesc2 && (lpDDSurfaceDesc2->dwFlags & DDSD_WIDTH)) ? lpDDSurfaceDesc2->dwWidth : 0;
		DWORD EnumHeight = (lpDDSurfaceDesc2 && (lpDDSurfaceDesc2->dwFlags & DDSD_HEIGHT)) ? lpDDSurfaceDesc2->dwHeight : 0;

		// Get display modes to enum
		DWORD DisplayBitCount = (DisplayMode.BPP) ? DisplayMode.BPP :
			(lpDDSurfaceDesc2 && (lpDDSurfaceDesc2->dwFlags & DDSD_PIXELFORMAT)) ? GetBitCount(lpDDSurfaceDesc2->ddpfPixelFormat) : 0;
		bool DisplayAllModes = (DisplayBitCount != 8 && DisplayBitCount != 16 && DisplayBitCount != 24 && DisplayBitCount != 32);

		// For deduplicating resolutions
		struct RESLIST {
			DWORD Width = 0;
			DWORD Height = 0;
			DWORD RefreshRate = 0;
			DWORD BitCount = 0;
		};
		std::vector<RESLIST> ResolutionList;

		// For games that require limited resolution return
		SIZE LimitedResolutionList[] =
		{
			{ 320, 200 },
			{ 320, 240 },
			{ 512, 384 },
			{ 640, 400 },
			{ 640, 480 },
			{ 800, 600 },
			{ 1024, 768 },
			{ 1152, 864 },
			{ 1280, 720 },
			{ 1280, 1024 },
			{ 1600, 1200 },
			{ (LONG)InitWidth, (LONG)InitHeight },
			{ (LONG)Config.DdrawCustomWidth, (LONG)Config.DdrawCustomHeight },
		};

		// Enumerate modes for format XRGB
		UINT modeCount = d3d9Object->GetAdapterModeCount(D3DADAPTER_DEFAULT, D9DisplayFormat);

		// Loop through all modes
		for (UINT i = 0; i < modeCount; i++)
		{
			// Get display modes
			D3DDISPLAYMODE d3ddispmode = {};
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

				// Set display refresh rate
				DWORD RefreshRate = (dwFlags & DDEDM_REFRESHRATES) ? d3ddispmode.RefreshRate : 0;

				// Check if the resolution is on the LimitedResolutionList
				bool ResolutionOkToUse = (!Config.DdrawLimitDisplayModeCount ||
					std::any_of(std::begin(LimitedResolutionList), std::end(LimitedResolutionList),
						[&](const SIZE& entry) {
							return ((DWORD)entry.cx == d3ddispmode.Width && (DWORD)entry.cy == d3ddispmode.Height);
						}));

				// Check if resolution has already been sent
				bool ResolutionAlreadySent = std::any_of(ResolutionList.begin(), ResolutionList.end(),
					[&](const RESLIST& Entry) {
						return (Entry.Width == d3ddispmode.Width && Entry.Height == d3ddispmode.Height && Entry.RefreshRate == RefreshRate && Entry.BitCount == DisplayBitCount);
					});

				// Check mode
				if (ResolutionOkToUse && !ResolutionAlreadySent &&
					(!EnumWidth || d3ddispmode.Width == EnumWidth) &&
					(!EnumHeight || d3ddispmode.Height == EnumHeight))
				{
					// Store resolution
					ResolutionList.push_back({ d3ddispmode.Width, d3ddispmode.Height, RefreshRate, DisplayBitCount });

					// Set surface desc options
					DDSURFACEDESC2 Desc2 = {};
					Desc2.dwSize = sizeof(DDSURFACEDESC2);
					Desc2.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_REFRESHRATE;
					Desc2.dwWidth = d3ddispmode.Width;
					Desc2.dwHeight = d3ddispmode.Height;
					Desc2.dwRefreshRate = RefreshRate;

					// Set adapter pixel format
					Desc2.dwFlags |= DDSD_PIXELFORMAT;
					Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
					SetDisplayFormat(Desc2.ddpfPixelFormat, DisplayBitCount);

					// Set pitch
					Desc2.dwFlags |= DDSD_PITCH;
					Desc2.lPitch = ComputePitch(GetByteAlignedWidth(d3ddispmode.Width, DisplayBitCount), DisplayBitCount);

					if (lpEnumModesCallback2(&Desc2, lpContext) == DDENUMRET_CANCEL)
					{
						return DD_OK;
					}
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
			for (m_IDirectDrawSurfaceX*& pSurfaceX : SurfaceVector)
			{
				LPDIRECTDRAWSURFACE7 pSurface7 = (LPDIRECTDRAWSURFACE7)pSurfaceX->GetWrapperInterfaceX(DirectXVersion);

				if (pSurface7)
				{
					DDSURFACEDESC2 Desc2 = {};
					Desc2.dwSize = sizeof(DDSURFACEDESC2);
					pSurfaceX->GetSurfaceDesc2(&Desc2, 0, DirectXVersion);

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
			hr = d3d9Object->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &Caps9);
			ConvertCaps(DriverCaps, Caps9);
			DriverCaps.dwVidMemTotal = dwVidTotal;
			DriverCaps.dwVidMemFree = dwVidFree;
		}
		if (lpDDHELCaps)
		{
			hr = d3d9Object->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, &Caps9);
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
		DWORD displayModeBits = DisplayMode.BPP;
		if (ExclusiveMode && Exclusive.Width && Exclusive.Height && Exclusive.BPP)
		{
			lpDDSurfaceDesc2->dwWidth = Exclusive.Width;
			lpDDSurfaceDesc2->dwHeight = Exclusive.Height;
			lpDDSurfaceDesc2->dwRefreshRate = Exclusive.RefreshRate;
			displayModeBits = Exclusive.BPP;
		}
		else
		{
			HWND hWnd = GetHwnd();
			Utils::GetScreenSize(hWnd, (LONG&)lpDDSurfaceDesc2->dwWidth, (LONG&)lpDDSurfaceDesc2->dwHeight);
			lpDDSurfaceDesc2->dwRefreshRate = Utils::GetRefreshRate(hWnd);
			displayModeBits = GetDisplayBPP(hWnd);
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
				if (SUCCEEDED(d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, format)))
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
		if (!ExclusiveMode && DisplayMode.Width && DisplayMode.Height && DisplayMode.BPP)
		{
			return DDERR_NOEXCLUSIVEMODE;
		}

		// Reset mode
		DisplayMode.Width = 0;
		DisplayMode.Height = 0;
		DisplayMode.BPP = 0;
		DisplayMode.RefreshRate = 0;
		Device.Width = (Config.DdrawUseNativeResolution || Config.DdrawOverrideWidth) ? Device.Width : 0;
		Device.Height = (Config.DdrawUseNativeResolution || Config.DdrawOverrideHeight) ? Device.Height : 0;
		Device.RefreshRate = 0;

		// Release d3d9 device
		ReleaseAllD9Resources(false, false);
		ReleaseD3D9Device();

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
		if (!(dwFlags & (DDSCL_NORMAL | DDSCL_EXCLUSIVE | DDSCL_SETDEVICEWINDOW | DDSCL_SETFOCUSWINDOW)) ||								// An application must set either of these flags
			((dwFlags & DDSCL_NORMAL) && (dwFlags & (DDSCL_ALLOWMODEX | DDSCL_EXCLUSIVE))) ||											// Normal flag cannot be used with Modex, Exclusive or Fullscreen flags
			((dwFlags & DDSCL_EXCLUSIVE) && !(dwFlags & DDSCL_FULLSCREEN)) ||															// If Exclusive flag is set then Fullscreen flag must be set
			((dwFlags & DDSCL_FULLSCREEN) && !(dwFlags & DDSCL_EXCLUSIVE)) ||															// If Fullscreen flag is set then Exclusive flag must be set
			((dwFlags & DDSCL_ALLOWMODEX) && (!(dwFlags & DDSCL_EXCLUSIVE) || !(dwFlags & DDSCL_FULLSCREEN))) ||						// If AllowModeX is set then Exclusive and Fullscreen flags must be set
			((dwFlags & DDSCL_SETDEVICEWINDOW) && (dwFlags & DDSCL_SETFOCUSWINDOW)) ||													// SetDeviceWindow flag cannot be used with SetFocusWindow flag
			(!(dwFlags & DDSCL_NORMAL) && !IsWindow(hWnd)))																				// When using Exclusive mode the hwnd must be valid
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
		if ((((dwFlags & DDSCL_EXCLUSIVE) || ExclusiveMode) && GetWindowThreadProcessId((hWnd) ? hWnd : DisplayMode.hWnd, nullptr) != GetCurrentThreadId()))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: attempt to set exclusive mode from a different thread than the hwnd was created from! dwFlags: " << Logging::hex(dwFlags) << " " << hWnd);
		}

		HWND LasthWnd = DisplayMode.hWnd;
		bool LastFPUPreserve = Device.FPUPreserve;
		bool LastExclusiveMode = ExclusiveMode;

		// Set windowed mode
		if (dwFlags & DDSCL_NORMAL)
		{
			// Check for exclusive mode
			if ((ExclusiveMode && hWnd && Exclusive.hWnd == hWnd && Exclusive.SetBy == this) || !IsWindow(Exclusive.hWnd))
			{
				ExclusiveMode = false;
				Exclusive = {};
			}
		}
		else if (dwFlags & DDSCL_FULLSCREEN)
		{
			if (ExclusiveMode && Exclusive.hWnd != hWnd && IsWindow(Exclusive.hWnd))
			{
				// If SetCooperativeLevel is called once in a process, a binding is established between the process and the window.
				// If it is called again in the same process with a different non-null window handle, it returns the DDERR_HWNDALREADYSET error value.
				LOG_LIMIT(100, __FUNCTION__ << " Error: Exclusive mode already set.");
				return DDERR_HWNDALREADYSET;
			}
			ExclusiveMode = true;
			Exclusive.hWnd = hWnd;
			Exclusive.SetBy = this;
		}

		// Check window handle
		if (IsWindow(hWnd) && (((!ExclusiveMode || Exclusive.hWnd == hWnd) && (!DisplayMode.hWnd || !DisplayMode.SetBy || DisplayMode.SetBy == this)) || !IsWindow(DisplayMode.hWnd)))
		{
			// Check if DC needs to be released
			if (DisplayMode.hWnd && DisplayMode.DC && (DisplayMode.hWnd != hWnd))
			{
				CloseD3DDDI();
				ReleaseDC(DisplayMode.hWnd, DisplayMode.DC);
				DisplayMode.DC = nullptr;
			}

			DisplayMode.hWnd = hWnd;
			DisplayMode.SetBy = this;

			if (DisplayMode.hWnd && !DisplayMode.DC)
			{
				DisplayMode.DC = ::GetDC(DisplayMode.hWnd);
			}
		}

		// Check if handle is valid
		if (IsWindow(DisplayMode.hWnd) && DisplayMode.hWnd == hWnd)
		{
			// Hook WndProc before creating device
			WndProc::AddWndProc(hWnd, true);

			// Set exclusive mode resolution
			if (ExclusiveMode && DisplayMode.Width && DisplayMode.Height && DisplayMode.BPP)
			{
				Exclusive.Width = DisplayMode.Width;
				Exclusive.Height = DisplayMode.Height;
				Exclusive.BPP = DisplayMode.BPP;
				Exclusive.RefreshRate = DisplayMode.RefreshRate;
			}

			// Set device flags
			Device.IsWindowed = (!ExclusiveMode || Config.EnableWindowMode);
			Device.AllowModeX = ((dwFlags & DDSCL_ALLOWMODEX) != 0);
			Device.MultiThreaded = ((dwFlags & DDSCL_MULTITHREADED) != 0);
			// The flag (DDSCL_FPUPRESERVE) is assumed by default in DirectX 6 and earlier.
			Device.FPUPreserve = (((dwFlags & DDSCL_FPUPRESERVE) || DirectXVersion <= 6) && (dwFlags & DDSCL_FPUSETUP) == 0);
			// The flag (DDSCL_NOWINDOWCHANGES) means DirectDraw is not allowed to minimize or restore the application window on activation.
			Device.NoWindowChanges = ((dwFlags & DDSCL_NOWINDOWCHANGES) != 0);

			// Reset if mode was changed
			if ((dwFlags & (DDSCL_NORMAL | DDSCL_EXCLUSIVE)) && (d3d9Device || LastUsedHWnd == DisplayMode.hWnd) &&
				(LastExclusiveMode != ExclusiveMode || LasthWnd != DisplayMode.hWnd || LastFPUPreserve != Device.FPUPreserve))
			{
				CreateD3D9Device();
			}
		}

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->SetCooperativeLevel(hWnd, dwFlags);

	// Release previouse Exclusive flag
	// Hook window message to get notified when the window is about to exit to remove the exclusive flag
	if (SUCCEEDED(hr) && (dwFlags & DDSCL_EXCLUSIVE) && IsWindow(hWnd) && hWnd != chWnd)
	{
		static DWORD WindowsGDIHook_DirectXVersion = 0;

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
			SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
		}
	}

	return hr;
}

HRESULT m_IDirectDrawX::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << dwWidth << "x" << dwHeight << " " << dwBPP << " " << dwRefreshRate << " " << Logging::hex(dwFlags);

	if (Config.Dd7to9)
	{
		if (!dwWidth || !dwHeight || (dwBPP != 8 && dwBPP != 16 && dwBPP != 24 && dwBPP != 32))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters: " << dwWidth << "x" << dwHeight << " " << dwBPP);
			return DDERR_INVALIDPARAMS;
		}

		// Check window handle thread
		if (ExclusiveMode && IsWindow(DisplayMode.hWnd) && GetWindowThreadProcessId(DisplayMode.hWnd, nullptr) != GetCurrentThreadId())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: set exclusive display from a different thread than the hwnd was created from! " << dwWidth << "x" << dwHeight << " " << dwBPP);
		}

		DWORD LastWidth = Device.Width;
		DWORD LastHeight = Device.Height;
		DWORD LastBPP = DisplayMode.BPP;
		DWORD LastRefreshRate = DisplayMode.RefreshRate;

		DWORD NewBPP = (Config.DdrawOverrideBitMode) ? Config.DdrawOverrideBitMode : dwBPP;

		if (DisplayMode.Width != dwWidth || DisplayMode.Height != dwHeight || DisplayMode.BPP != NewBPP || (dwRefreshRate && DisplayMode.RefreshRate != dwRefreshRate))
		{
			DWORD FoundWidth = dwWidth;
			DWORD FoundHeight = dwHeight;

			// Check if it is a supported resolution
			if ((ExclusiveMode && !Config.EnableWindowMode) || Config.FullscreenWindowMode || Config.ForceExclusiveFullscreen)
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
					d3ddispmode = {};
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
					LOG_LIMIT(100, __FUNCTION__ << " Error: Mode not found: " << dwWidth << "x" << dwHeight);
					return DDERR_INVALIDMODE;
				}
			}

			DisplayMode.Width = dwWidth;
			DisplayMode.Height = dwHeight;
			DisplayMode.BPP = NewBPP;
			DisplayMode.RefreshRate = dwRefreshRate;

			// Display resolution
			Device.Width = (Config.DdrawUseNativeResolution || Config.DdrawOverrideWidth) ? Device.Width : FoundWidth;
			Device.Height = (Config.DdrawUseNativeResolution || Config.DdrawOverrideHeight) ? Device.Height : FoundHeight;
			Device.RefreshRate = DisplayMode.RefreshRate;
		}

		// Set exclusive mode resolution
		if (ExclusiveMode && Exclusive.hWnd == DisplayMode.hWnd)
		{
			Exclusive.Width = dwWidth;
			Exclusive.Height = dwHeight;
			Exclusive.BPP = NewBPP;
			Exclusive.RefreshRate = dwRefreshRate;
		}

		// Get screensize
		if (!d3d9Device || presParams.Windowed)
		{
			Utils::GetScreenSize(DisplayMode.hWnd, (LONG&)DefaultWidth, (LONG&)DefaultHeight);
		}

		// Update the d3d9 device to use new display mode
		if (LastWidth != Device.Width || LastHeight != Device.Height || (!Device.IsWindowed && LastRefreshRate != DisplayMode.RefreshRate))
		{
			// Mark flag that resolution has changed
			SetResolution = ExclusiveMode;

			// Reset d3d9 device
			CreateD3D9Device();
		}
		else if (LastBPP != DisplayMode.BPP)
		{
			// Reset all surfaces
			ResetAllSurfaceDisplay();
		}

		return DD_OK;
	}

	// Force color mode
	dwBPP = (Config.DdrawOverrideBitMode) ? Config.DdrawOverrideBitMode : dwBPP;

	if (Config.DdrawUseNativeResolution && dwWidth && dwHeight)
	{
		ScaleDDLastWidth = dwWidth;
		ScaleDDLastHeight = dwHeight;
		Utils::GetScreenSize(nullptr, (LONG&)ScaleDDCurrentWidth, (LONG&)ScaleDDCurrentHeight);
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

		if (Config.ForceVsyncMode || IsUsing3D())
		{
			return DD_OK;
		}

		D3DRASTER_STATUS RasterStatus;

		// Check flags
		switch (dwFlags)
		{
		case DDWAITVB_BLOCKBEGIN:
			// Check if already in vertical blank
			if (SUCCEEDED(d3d9Device->GetRasterStatus(0, &RasterStatus)) && RasterStatus.InVBlank)
			{
				return DD_OK;
			}
			[[fallthrough]];
		case DDWAITVB_BLOCKEND:
			// Use D3DKMTWaitForVerticalBlankEvent for vertical blank begin
			if (OpenD3DDDI(GetDC()) && D3DDDIWaitForVsync())
			{
				// Success using D3DKMTWaitForVerticalBlankEvent
			}
			// Use raster status for vertical blank begin (uses high CPU)
			else while (SUCCEEDED(d3d9Device->GetRasterStatus(0, &RasterStatus)) && !RasterStatus.InVBlank)
			{
				Utils::BusyWaitYield();
			}
			// Exit if just waiting for vertical blank begin
			if (dwFlags == DDWAITVB_BLOCKBEGIN)
			{
				return DD_OK;
			}
			// Use raster status for vertical blank end (uses high CPU)
			while (SUCCEEDED(d3d9Device->GetRasterStatus(0, &RasterStatus)) && RasterStatus.InVBlank)
			{
				Utils::BusyWaitYield();
			}
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
		// Get non-local video memory
		else if (lpDDSCaps2 && (lpDDSCaps2->dwCaps & DDSCAPS_NONLOCALVIDMEM))
		{
			if (lpdwTotal)
			{
				*lpdwTotal = 0;
			}
			if (lpdwFree)
			{
				*lpdwFree = 0;
			}
			return DD_OK;
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
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_WRONGMODE;
		}

		// Check device state
		HRESULT hr = TestD3D9CooperativeLevel();
		if (hr == D3DERR_DEVICENOTRESET)
		{
			hr = ReinitDevice();
		}

		// Check device status
		if (hr == DD_OK || hr == DDERR_NOEXCLUSIVEMODE)
		{
			SetCriticalSection();

			for (m_IDirectDrawX*& pDDraw : DDrawVector)
			{
				for (m_IDirectDrawSurfaceX*& pSurface : pDDraw->SurfaceVector)
				{
					pSurface->Restore();
				}
			}

			ReleaseCriticalSection();
		}

		return hr;
	}

	return ProxyInterface->RestoreAllSurfaces();
}

HRESULT m_IDirectDrawX::TestCooperativeLevel()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		switch (TestD3D9CooperativeLevel())
		{
		case D3DERR_INVALIDCALL:
		case D3DERR_DRIVERINTERNALERROR:
			return DDERR_WRONGMODE;
		case D3DERR_DEVICELOST:
			// Full-screen applications receive the DDERR_NOEXCLUSIVEMODE return value if they lose exclusive device access
			if (ExclusiveMode)
			{
				return DDERR_NOEXCLUSIVEMODE;
			}
			// Windowed applications(those that use the normal cooperative level) receive DDERR_EXCLUSIVEMODEALREADYSET if another application has taken exclusive device access
			else
			{
				return DDERR_EXCLUSIVEMODEALREADYSET;
			}
		case D3DERR_DEVICENOTRESET:
			//The TestCooperativeLevel method succeeds, returning DD_OK, if your application can restore its surfaces
		case DDERR_NOEXCLUSIVEMODE:
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

	SetCriticalSection();

	DDrawVector.push_back(this);

	if (DDrawVector.size() == 1)
	{
		// Get screensize
		DefaultWidth = 0;
		DefaultHeight = 0;
		Utils::GetScreenSize(nullptr, (LONG&)DefaultWidth, (LONG&)DefaultHeight);
		LastWindowLocation = {};

		// Release DC
		if (DisplayMode.hWnd && DisplayMode.DC)
		{
			ReleaseDC(DisplayMode.hWnd, DisplayMode.DC);
			DisplayMode.DC = nullptr;
		}
		LastUsedHWnd = nullptr;
		ClipperHWnd = nullptr;

		// Display mode
		DisplayMode = {};

		// Exclusive mode
		SetResolution = false;
		ExclusiveMode = false;
		Exclusive = {};

		// Display pixel format
		DisplayPixelFormat = {};

		// Last surface resolution
		LastSetWidth = 0;
		LastSetHeight = 0;
		LastSetBPP = 0;

		// Device settings
		Device = {};
		Device.IsWindowed = true;

		// High resolution counter
		Counter = {};
		QueryPerformanceFrequency(&Counter.Frequency);

		// Direct3D9 flags
		EnableWaitVsync = false;

		// Direct3D9 Objects
		d3d9Object = nullptr;
		d3d9Device = nullptr;
		d3d9RenderTarget = nullptr;
		palettePixelShader = nullptr;
		colorkeyPixelShader = nullptr;
		VertexBuffer = nullptr;

		presParams = {};
		BehaviorFlags = 0;
		hFocusWindow = nullptr;

		// Display resolution
		Utils::GetScreenSize(GetHwnd(), (LONG&)InitWidth, (LONG&)InitHeight);
		if (Config.DdrawUseNativeResolution)
		{
			Device.Width = InitWidth;
			Device.Height = InitHeight;
		}
		else
		{
			Device.Width = (Config.DdrawOverrideWidth) ? Config.DdrawOverrideWidth : 0;
			Device.Height = (Config.DdrawOverrideHeight) ? Config.DdrawOverrideHeight : 0;
		}
		Device.RefreshRate = 0;

		// Prepare for present from another thread
		if (Config.DdrawAutoFrameSkip)
		{
			PresentThread.IsInitialized = true;
			InitializeCriticalSection(&PresentThread.ddpt);
			PresentThread.workerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			PresentThread.workerThread = CreateThread(NULL, 0, PresentThreadFunction, NULL, 0, NULL);
		}

		// Mouse hook
		bool EnableMouseHook = Config.DdrawEnableMouseHook &&
			((Config.DdrawUseNativeResolution || Config.DdrawOverrideWidth || Config.DdrawOverrideHeight) &&
			(!Config.EnableWindowMode || Config.FullscreenWindowMode));

		// Set mouse hook
		if (!MouseHook.m_hook && EnableMouseHook)
		{
			struct WindowsMouseHook
			{
				static LRESULT CALLBACK mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
				{
					POINT p;
					if (nCode == HC_ACTION && wParam == WM_MOUSEMOVE)
					{
						if (DDrawVector.size() && MouseHook.threadID && MouseHook.ghWriteEvent &&
							DisplayMode.Width && DisplayMode.Height && Device.Width && Device.Height &&
							DisplayMode.Width != Device.Width && DisplayMode.Height != Device.Height &&
							!Device.IsWindowed && IsWindow(DisplayMode.hWnd) && !IsIconic(DisplayMode.hWnd) &&
							GetCursorPos(&p))
						{
							MouseHook.Pos.x = min(p.x, (LONG)DisplayMode.Width - 1);
							MouseHook.Pos.y = min(p.y, (LONG)DisplayMode.Height - 1);

							if (MouseHook.Pos.x != p.x || MouseHook.Pos.y != p.y)
							{
								MouseHook.bChange = true;
								SetEvent(MouseHook.ghWriteEvent);
							}
						}
					}
					return CallNextHookEx(nullptr, nCode, wParam, lParam);
				}
			};

			Logging::Log() << __FUNCTION__ << " Hooking mouse cursor!";
			MouseHook.m_hook = SetWindowsHookEx(WH_MOUSE_LL, WindowsMouseHook::mouseHookProc, hModule_dll, 0);
		}

		// Start thread
		if (!MouseHook.threadID && EnableMouseHook)
		{
			// A thread to bypass Windows preventing hooks from modifying mouse position
			struct WindowsMouseThread
			{
				static DWORD WINAPI setMousePosThread(LPVOID)
				{
					DWORD dwWaitResult = 0;
					do {
						dwWaitResult = WaitForSingleObject(MouseHook.ghWriteEvent, INFINITE);
						if (MouseHook.bChange)
						{
							SetCursorPos(MouseHook.Pos.x, MouseHook.Pos.y);
							MouseHook.bChange = false;
						}
					} while (!Config.Exiting && dwWaitResult == WAIT_OBJECT_0);

					// Unhook mouse
					if (MouseHook.m_hook)
					{
						UnhookWindowsHookEx(MouseHook.m_hook);
						MouseHook.m_hook = nullptr;
					}

					// Close handle
					if (MouseHook.ghWriteEvent)
					{
						CloseHandle(MouseHook.ghWriteEvent);
						MouseHook.ghWriteEvent = nullptr;
					}

					MouseHook.threadID = nullptr;
					return 0;
				}
			};

			MouseHook.threadID = CreateThread(nullptr, 0, WindowsMouseThread::setMousePosThread, nullptr, 0, nullptr);
		}

		// Create event
		if (!MouseHook.ghWriteEvent && EnableMouseHook)
		{
			MouseHook.ghWriteEvent = CreateEvent(nullptr, FALSE, FALSE, TEXT("Local\\DxwrapperMouseEvent"));
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

	SetCriticalSection();
	SetPTCriticalSection();

	// Clear SetBy handles
	if (DisplayMode.SetBy == this)
	{
		DisplayMode.SetBy = nullptr;
	}
	if (Exclusive.SetBy == this)
	{
		Exclusive.SetBy = nullptr;
	}

	// Remove ddraw device from vector
	DDrawVector.erase(std::remove(DDrawVector.begin(), DDrawVector.end(), this), DDrawVector.end());

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
	for (m_IDirectDrawSurfaceX*& pSurface : SurfaceVector)
	{
		pSurface->ReleaseD9Surface(false, false);
		pSurface->ClearDdraw();
	}
	SurfaceVector.clear();

	// Release Clippers
	for (m_IDirectDrawClipper*& pClipper : ClipperVector)
	{
		pClipper->ClearDdraw();
	}
	ClipperVector.clear();

	// Release palettes
	for (m_IDirectDrawPalette*& pPalette : PaletteVector)
	{
		pPalette->ClearDdraw();
	}
	PaletteVector.clear();

	// Release vertex buffers
	for (m_IDirect3DVertexBufferX*& pVertexBuffer : VertexBufferVector)
	{
		pVertexBuffer->ReleaseD9Buffers(false, false);
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

	ReleasePTCriticalSection();

	if (DDrawVector.size() == 0)
	{
		// Close present thread first
		if (PresentThread.IsInitialized)
		{
			PresentThread.EnableThreadFlag = false;						// Tell thread to exit
			EnterCriticalSection(&PresentThread.ddpt);					// Ensure thread is not running present
			SetEvent(PresentThread.workerEvent);						// Trigger thread
			LeaveCriticalSection(&PresentThread.ddpt);
			WaitForSingleObject(PresentThread.workerThread, INFINITE);	// Wait for thread to finish
			CloseHandle(PresentThread.workerThread);					// Close thread handle
			CloseHandle(PresentThread.workerEvent);						// Close event handle
			PresentThread.IsInitialized = false;
		}

		// Release all resources
		ReleaseAllD9Resources(false, false);

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
		if (DisplayMode.hWnd && DisplayMode.DC)
		{
			ReleaseDC(DisplayMode.hWnd, DisplayMode.DC);
			DisplayMode.DC = nullptr;
		}

		// Clean up shared memory
		m_IDirectDrawSurfaceX::CleanupSharedEmulatedMemory();

		// Delete critical section
		if (PresentThread.IsInitialized)
		{
			DeleteCriticalSection(&PresentThread.ddpt);
		}
	}

	ReleaseCriticalSection();
}

HWND m_IDirectDrawX::GetHwnd()
{
	return IsWindow(DisplayMode.hWnd) ? DisplayMode.hWnd : ClipperHWnd;
}

HDC m_IDirectDrawX::GetDC()
{
	return WindowFromDC(DisplayMode.DC) ? DisplayMode.DC : nullptr;
}

DWORD m_IDirectDrawX::GetDisplayBPP(HWND hWnd)
{
	return (ExclusiveMode && Exclusive.BPP) ? Exclusive.BPP : Utils::GetBitCount(hWnd);
}

bool m_IDirectDrawX::IsExclusiveMode()
{
	return ExclusiveMode;
}

void m_IDirectDrawX::GetSurfaceDisplay(DWORD& Width, DWORD& Height, DWORD& BPP, DWORD& RefreshRate)
{
	// Init settings
	Width = 0;
	Height = 0;
	RefreshRate = 0;
	BPP = 0;

	// Get hwnd
	HWND hWnd = GetHwnd();

	// Width, Height, RefreshMode
	if (ExclusiveMode && Exclusive.Width && Exclusive.Height && Exclusive.BPP)
	{
		Width = Exclusive.Width;
		Height = Exclusive.Height;
		RefreshRate = Exclusive.RefreshRate;
		BPP = Exclusive.BPP;
	}
	else
	{
		if (!Config.DdrawWriteToGDI && !Using3D)
		{
			Utils::GetScreenSize(hWnd, (LONG&)Width, (LONG&)Height);
		}
		else if (d3d9Device)
		{
			Width = presParams.BackBufferWidth;
			Height = presParams.BackBufferHeight;
		}
		BPP = (DisplayMode.BPP) ? DisplayMode.BPP : GetDisplayBPP(hWnd);
	}

	// Force color mode
	BPP = (Config.DdrawOverrideBitMode) ? Config.DdrawOverrideBitMode : BPP;

	// Check if resolution changed
	if ((LastSetWidth && Width && LastSetWidth != Width) ||
		(LastSetHeight && Height && LastSetHeight != Height) ||
		(LastSetBPP && BPP && LastSetBPP != BPP))
	{
		ResetAllSurfaceDisplay();
	}
	LastSetWidth = Width;
	LastSetHeight = Height;
	LastSetBPP = BPP;
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

void m_IDirectDrawX::SetD3D(m_IDirect3DX* D3D)
{
	D3DInterface = D3D;
}

void m_IDirectDrawX::SetD3DDevice(m_IDirect3DDeviceX* D3DDevice)
{
	D3DDeviceInterface = D3DDevice;
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

bool m_IDirectDrawX::CreatePaletteShader()
{
	// Create pixel shaders
	if (d3d9Device && !palettePixelShader)
	{
		d3d9Device->CreatePixelShader((DWORD*)PalettePixelShaderSrc, &palettePixelShader);
	}
	return (palettePixelShader != nullptr);
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

// Creates or resets the d3d9 device
HRESULT m_IDirectDrawX::CreateD3D9Device()
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, false)))
	{
		return DDERR_GENERIC;
	}

	SetCriticalSection();
	SetPTCriticalSection();

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

		// Remove tool window
		if (hWnd && hWnd != LastHWnd)
		{
			LONG ExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);

			if (ExStyle & WS_EX_TOOLWINDOW)
			{
				LOG_LIMIT(3, __FUNCTION__ << " Removing window WS_EX_TOOLWINDOW!");

				SetWindowLong(hWnd, GWL_EXSTYLE, ExStyle & ~WS_EX_TOOLWINDOW);
			}
		}

		// Get current resolution and rect
		DWORD CurrentWidth, CurrentHeight;
		Utils::GetScreenSize(hWnd, (LONG&)CurrentWidth, (LONG&)CurrentHeight);

		// Get current window size
		if (hWnd)
		{
			GetWindowRect(hWnd, &LastWindowLocation);
		}

		// Get width and height
		DWORD BackBufferWidth = 0;
		DWORD BackBufferHeight = 0;
		if (Device.Width && Device.Height)
		{
			BackBufferWidth = Device.Width;
			BackBufferHeight = Device.Height;
		}
		else
		{
			if (ExclusiveMode || Config.FullscreenWindowMode || Config.ForceExclusiveFullscreen)
			{
				// Use resolution set by game 
				if (Device.Width && Device.Height)
				{
					BackBufferWidth = Device.Width;
					BackBufferHeight = Device.Height;
				}
				// Use default desktop resolution
				else
				{
					BackBufferWidth = DefaultWidth;
					BackBufferHeight = DefaultHeight;
				}
			}
			else
			{
				// Reset display to get proper screen size
				if (d3d9Device && !LastWindowedMode && !Config.EnableWindowMode)
				{
					ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, CDS_RESET, nullptr);
				}
				BackBufferWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
				BackBufferHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
			}
		}

		// Backup last present parameters
		D3DPRESENT_PARAMETERS presParamsBackup = presParams;

		// Set display window
		presParams = {};

		// Width/height
		presParams.BackBufferWidth = BackBufferWidth;
		presParams.BackBufferHeight = BackBufferHeight;
		// Discard swap
		presParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
		// Backbuffer
		presParams.BackBufferCount = 1;
		// Auto stencel format
		presParams.AutoDepthStencilFormat = Config.DdrawOverrideStencilFormat ? (D3DFORMAT)Config.DdrawOverrideStencilFormat : D3DFMT_UNKNOWN;
		// Auto stencel
		presParams.EnableAutoDepthStencil = Config.DdrawOverrideStencilFormat ? TRUE : FALSE;
		// Interval level
		presParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		// Anti-aliasing
		presParams.MultiSampleType = D3DMULTISAMPLE_NONE;
		presParams.MultiSampleQuality = 0;
		// Present flags
		presParams.Flags = 0;
		// Window handle
		presParams.hDeviceWindow = hWnd;

		// Set parameters for the current display mode
		if (Device.IsWindowed || !hWnd)
		{
			// Window mode
			presParams.Windowed = TRUE;
			// Backbuffer
			presParams.BackBufferFormat = D3DFMT_UNKNOWN;
			// Display mode refresh
			presParams.FullScreen_RefreshRateInHz = 0;
		}
		else
		{
			// Fullscreen
			presParams.Windowed = FALSE;
			// Backbuffer
			presParams.BackBufferFormat = D9DisplayFormat;
			// Display mode refresh
			presParams.FullScreen_RefreshRateInHz = Device.RefreshRate;
		}

		// Enable antialiasing
		if (Device.AntiAliasing)
		{
			DWORD QualityLevels = 0;

			// Check AntiAliasing quality
			if (SUCCEEDED(d3d9Object->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D9DisplayFormat, presParams.Windowed, D3DMULTISAMPLE_NONMASKABLE, &QualityLevels)))
			{
				presParams.MultiSampleType = D3DMULTISAMPLE_NONMASKABLE;
				presParams.MultiSampleQuality = QualityLevels ? QualityLevels - 1 : 0;
			}
		}

		// Check device caps for vertex processing support
		D3DCAPS9 d3dcaps = {};
		hr = d3d9Object->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dcaps);
		if (FAILED(hr))
		{
			Logging::Log() << __FUNCTION__ << " Failed to get Direct3D9 device caps: " << (DDERR)hr;
		}

		// Set behavior flags
		BehaviorFlags = (d3dcaps.VertexProcessingCaps ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING) |
			D3DCREATE_MULTITHREADED |
			(Device.FPUPreserve ? D3DCREATE_FPU_PRESERVE : 0);

		Logging::Log() << __FUNCTION__ << " Direct3D9 device! " <<
			presParams.BackBufferWidth << "x" << presParams.BackBufferHeight << " refresh: " << presParams.FullScreen_RefreshRateInHz <<
			" format: " << presParams.BackBufferFormat << " wnd: " << hWnd << " params: " << presParams << " flags: " << Logging::hex(BehaviorFlags);

		// Check if existing device exists
		if (d3d9Device)
		{
			// Check if there are any device changes
			if (presParamsBackup.BackBufferWidth == presParams.BackBufferWidth &&
				presParamsBackup.BackBufferHeight == presParams.BackBufferHeight &&
				presParamsBackup.Windowed == presParams.Windowed &&
				presParamsBackup.hDeviceWindow == presParams.hDeviceWindow &&
				presParamsBackup.FullScreen_RefreshRateInHz == presParams.FullScreen_RefreshRateInHz &&
				LastBehaviorFlags == BehaviorFlags)
			{
				hr = DD_OK;
				break;
			}

			// Try to reset existing device
			if (LastHWnd == hWnd && LastBehaviorFlags == BehaviorFlags)
			{
				// Prepare for reset
				ReleaseAllD9Resources(true, false);

				// Reset device. When this method returns: BackBufferCount, BackBufferWidth, and BackBufferHeight are set to zero.
				D3DPRESENT_PARAMETERS newParams = presParams;
				hr = d3d9Device->Reset(&newParams);

				// Resetting the device failed
				if (FAILED(hr))
				{
					Logging::Log() << __FUNCTION__ << " Failed to reset device! " << (D3DERR)hr << " Last create: " << LastHWnd << "->" << hWnd << " " <<
						" Windowed: " << LastWindowedMode << "->" << presParams.Windowed <<
						" BehaviorFlags: " << Logging::hex(LastBehaviorFlags) << "->" << Logging::hex(BehaviorFlags);

					ReleaseD3D9Device();
				}
			}
			// Release existing device
			else
			{
				Logging::Log() << __FUNCTION__ << " Recreate device! Last create: " << LastHWnd << "->" << hWnd << " " <<
					" Windowed: " << LastWindowedMode << "->" << presParams.Windowed << " " <<
					Logging::hex(LastBehaviorFlags) << "->" << Logging::hex(BehaviorFlags);

				ReleaseAllD9Resources(true, false);
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

		// Set render target
		d3d9RenderTarget = nullptr;
		if (RenderTargetSurface)
		{
			SetRenderTargetSurface(RenderTargetSurface);
		}

		// Reset D3D device settings
		if (D3DDeviceInterface)
		{
			D3DDeviceInterface->ResetDevice();
		}

		// Reset flags after creating device
		LastUsedHWnd = hWnd;
		EnableWaitVsync = false;
		FourCCsList.clear();

		// Send window and display change messages
		if (hWnd)
		{
			// Get new resolution
			DWORD NewWidth = presParams.BackBufferWidth, NewHeight = presParams.BackBufferHeight;
			Utils::GetScreenSize(hWnd, (LONG&)NewWidth, (LONG&)NewHeight);

			// Send display change message
			if ((SetResolution || NewWidth != CurrentWidth || NewHeight != CurrentHeight) && NewWidth && NewHeight)
			{
				SetResolution = false;
				SendMessage(hWnd, WM_DISPLAYCHANGE, GetDisplayBPP(hWnd), MAKELPARAM(NewWidth, NewHeight));
			}

			// Get window size
			RECT NewRect = { 0, 0, (LONG)presParams.BackBufferWidth, (LONG)presParams.BackBufferHeight };
			GetWindowRect(hWnd, &NewRect);

			// Window position change
			HWND WindowInsert = GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST ? HWND_TOPMOST : HWND_TOP;
			static WINDOWPOS winpos;
			winpos = { hWnd, WindowInsert, NewRect.left, NewRect.top, NewRect.right - NewRect.left, NewRect.bottom - NewRect.top, WM_NULL };
			SendMessage(hWnd, WM_WINDOWPOSCHANGING, 0, (LPARAM)&winpos);
			SendMessage(hWnd, WM_WINDOWPOSCHANGED, 0, (LPARAM)&winpos);

			// Window moved
			if ((NewRect.left != LastWindowLocation.left || NewRect.top != LastWindowLocation.top ||
				NewRect.right != LastWindowLocation.right || NewRect.bottom != LastWindowLocation.bottom) &&
				LastWindowLocation.right && LastWindowLocation.bottom)
			{
				SendMessage(hWnd, WM_ENTERSIZEMOVE, 0, 0);
				SendMessage(hWnd, WM_MOVING, 0, (LPARAM)&NewRect);
				SendMessage(hWnd, WM_EXITSIZEMOVE, 0, 0);
				SendMessage(hWnd, WM_MOVE, 0, MAKELPARAM(NewRect.left, NewRect.top));
			}
			// Window sized
			if ((NewRect.right - NewRect.left != LastWindowLocation.right - LastWindowLocation.left ||
				NewRect.bottom - NewRect.top != LastWindowLocation.bottom - LastWindowLocation.top) &&
				LastWindowLocation.right && LastWindowLocation.bottom)
			{
				SendMessage(hWnd, WM_ENTERSIZEMOVE, 0, 0);
				SendMessage(hWnd, WM_SIZING, WMSZ_BOTTOMRIGHT, (LPARAM)&NewRect);
				SendMessage(hWnd, WM_EXITSIZEMOVE, 0, 0);
				SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(NewRect.right - NewRect.left, NewRect.bottom - NewRect.top));
			}
		}

		// Store display frequency
		DWORD RefreshRate = (presParams.FullScreen_RefreshRateInHz) ? presParams.FullScreen_RefreshRateInHz : Utils::GetRefreshRate(hWnd);
		Counter.PerFrameMS = 1000.0 / (RefreshRate ? RefreshRate : 60);

	} while (false);

	ReleasePTCriticalSection();
	ReleaseCriticalSection();

	// Return result
	return hr;
}

HRESULT m_IDirectDrawX::CreateVertexBuffer(DWORD Width, DWORD Height)
{
	// Check if vertex buffer is already created
	if (VertexBuffer)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: vertex buffer already created!");
		return DDERR_GENERIC;
	}

	// Create vertex buffer
	if (FAILED(d3d9Device->CreateVertexBuffer(sizeof(TLVERTEX) * 4, D3DUSAGE_WRITEONLY, TLVERTEXFVF, D3DPOOL_DEFAULT, &VertexBuffer, nullptr)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create vertex buffer!");
		return DDERR_GENERIC;
	}

	// Setup verticies (0, 0, currentWidth, currentHeight)
	TLVERTEX* vertices = nullptr;

	// Lock vertex buffer
	if (FAILED(VertexBuffer->Lock(0, 0, (void**)&vertices, 0)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock vertex buffer!");
		return DDERR_GENERIC;
	}

	// Get width and height
	DWORD displayWidth, displayHeight;
	GetDisplay(displayWidth, displayHeight);
	bool displayflag = (Width < displayWidth) && (Height < displayHeight);
	DWORD BackBufferWidth = (displayflag) ? displayWidth : Width;
	DWORD BackBufferHeight = (displayflag) ? displayHeight : Height;
	if (!BackBufferWidth || !BackBufferHeight)
	{
		Utils::GetScreenSize(GetHwnd(), (LONG&)BackBufferWidth, (LONG&)BackBufferHeight);
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
	if (FAILED(VertexBuffer->Unlock()))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to unlock vertex buffer!");
		return DDERR_GENERIC;
	}
	
	return DD_OK;
}

// Creates d3d9 object
HRESULT m_IDirectDrawX::CreateD3D9Object()
{
	// Create d3d9 object
	if (!d3d9Object)
	{
		// Declare Direct3DCreate9
		DEFINE_STATIC_PROC_ADDRESS(Direct3DCreate9Proc, Direct3DCreate9, Direct3DCreate9_out);

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
		return DDERR_WRONGMODE;
	}

	// Check if device is ready to be restored
	HRESULT hr = TestD3D9CooperativeLevel();
	if (SUCCEEDED(hr) || hr == DDERR_NOEXCLUSIVEMODE)
	{
		return hr;
	}
	else if (hr == D3DERR_DEVICELOST)
	{
		HWND hWnd = GetHwnd();
		if (!IsIconic(hWnd) && hWnd == GetForegroundWindow())
		{
			return DDERR_WRONGMODE;
		}
		else
		{
			return DDERR_SURFACELOST;
		}
	}
	else if (hr != D3DERR_DEVICENOTRESET)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: TestCooperativeLevel = " << (D3DERR)hr);
		return DDERR_WRONGMODE;
	}

	SetCriticalSection();
	SetPTCriticalSection();

	do {
		// Check window handle thread
		if (IsWindow(presParams.hDeviceWindow) && GetWindowThreadProcessId(presParams.hDeviceWindow, nullptr) != GetCurrentThreadId())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: trying to reset Direct3D9 device from a different thread than the hwnd was created from!");
		}

		// Prepare for reset
		ReleaseAllD9Resources(true, true);

		// Reset device. When this method returns: BackBufferCount, BackBufferWidth, and BackBufferHeight are set to zero.
		D3DPRESENT_PARAMETERS newParams = presParams;
		hr = d3d9Device->Reset(&newParams);

		// Reset render target
		if (SUCCEEDED(hr))
		{
			d3d9RenderTarget = nullptr;
			if (RenderTargetSurface)
			{
				SetRenderTargetSurface(RenderTargetSurface);
			}
		}

		// If Reset() fails
		if (hr == D3DERR_DEVICEREMOVED || hr == D3DERR_DRIVERINTERNALERROR)
		{
			ReleaseAllD9Resources(true, false);
			ReleaseD3D9Device();
			ReleaseD3D9Object();
			CreateD3D9Object();
			hr = CreateD3D9Device();
		}

		// Check if reset device failed
		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to reset Direct3D9 device: " << (D3DERR)hr);
			hr = DDERR_WRONGMODE;
			break;
		}

		// Reset D3D device settings
		if (D3DDeviceInterface)
		{
			D3DDeviceInterface->ResetDevice();
		}

	} while (false);

	ReleasePTCriticalSection();
	ReleaseCriticalSection();

	// Return
	return hr;
}

HRESULT m_IDirectDrawX::TestD3D9CooperativeLevel()
{
	if (d3d9Device)
	{
		return d3d9Device->TestCooperativeLevel();
	}

	return DD_OK;
}

HRESULT m_IDirectDrawX::SetRenderTargetSurface(m_IDirectDrawSurfaceX* lpSurface)
{
	// Remove render target
	if (!lpSurface)
	{
		RenderTargetSurface = lpSurface;

		SetDepthStencilSurface(nullptr);

		if (d3d9Device && d3d9RenderTarget)
		{
			if (SUCCEEDED(d3d9Device->SetRenderTarget(0, d3d9RenderTarget)))
			{
				d3d9RenderTarget = nullptr;
			}
		}

		return D3D_OK;
	}

	// Check for Direct3D surface
	if (!lpSurface->IsSurface3D())
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not a Direct3D surface!");
		return DDERR_INVALIDPARAMS;
	}

	// Set surface as render target
	RenderTargetSurface = lpSurface;
	RenderTargetSurface->SetAsRenderTarget();

	Logging::LogDebug() << __FUNCTION__ << " Setting 3D Device Surface: " << RenderTargetSurface;

	HRESULT hr = D3D_OK;
	if (d3d9Device)
	{
		// Backup original render target
		if (!d3d9RenderTarget)
		{
			if (SUCCEEDED(d3d9Device->GetRenderTarget(0, &d3d9RenderTarget)))
			{
				d3d9RenderTarget->Release();
			}
		}

		// Set new render target
		LPDIRECT3DSURFACE9 pSurfaceD9 = RenderTargetSurface->GetD3d9Surface();
		if (pSurfaceD9)
		{
			hr = d3d9Device->SetRenderTarget(0, pSurfaceD9);
		}
	}

	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set render target: " << (D3DERR)hr);
		return hr;
	}

	m_IDirectDrawSurfaceX* pSurfaceZBuffer = RenderTargetSurface->GetAttachedZBuffer();
	hr = SetDepthStencilSurface(pSurfaceZBuffer);

	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set depth stencil surface: " << (D3DERR)hr);
		return hr;
	}

	return hr;
}

HRESULT m_IDirectDrawX::SetDepthStencilSurface(m_IDirectDrawSurfaceX* lpSurface)
{
	HRESULT hr = D3D_OK;

	if (!lpSurface)
	{
		DepthStencilSurface = nullptr;

		if (d3d9Device)
		{
			d3d9Device->SetRenderState(D3DRS_ZENABLE, FALSE);
			hr = d3d9Device->SetDepthStencilSurface(nullptr);
		}
	}
	else if (lpSurface->IsDepthBuffer())
	{
		DepthStencilSurface = lpSurface;

		Logging::LogDebug() << __FUNCTION__ << " Setting Depth Stencil Surface: " << DepthStencilSurface;

		if (d3d9Device)
		{
			LPDIRECT3DSURFACE9 pSurfaceD9 = DepthStencilSurface->GetD3d9Surface();
			if (pSurfaceD9)
			{
				d3d9Device->SetRenderState(D3DRS_ZENABLE, TRUE);
				hr = d3d9Device->SetDepthStencilSurface(pSurfaceD9);
			}
		}
	}

	return hr;
}

inline void m_IDirectDrawX::ResetAllSurfaceDisplay()
{
	SetCriticalSection();

	for (m_IDirectDrawX*& pDDraw : DDrawVector)
	{
		for (m_IDirectDrawSurfaceX*& pSurface : pDDraw->SurfaceVector)
		{
			pSurface->ResetSurfaceDisplay();
		}
	}

	ReleaseCriticalSection();
}

// Release all dd9 resources
inline void m_IDirectDrawX::ReleaseAllD9Resources(bool BackupData, bool ResetInterface)
{
	SetCriticalSection();
	SetPTCriticalSection();

	// Remove depth buffer surface
	if (d3d9Device && DepthStencilSurface)
	{
		d3d9Device->SetRenderState(D3DRS_ZENABLE, FALSE);
		d3d9Device->SetDepthStencilSurface(nullptr);
	}

	// Remove render target
	if (d3d9Device && d3d9RenderTarget)
	{
		d3d9Device->SetRenderTarget(0, d3d9RenderTarget);
	}

	// Release all surfaces from all ddraw devices
	for (m_IDirectDrawX*& pDDraw : DDrawVector)
	{
		for (m_IDirectDrawSurfaceX*& pSurface : pDDraw->SurfaceVector)
		{
			pSurface->ReleaseD9Surface(BackupData, ResetInterface);
		}
		for (m_IDirectDrawSurfaceX*& pSurface : pDDraw->ReleasedSurfaceVector)
		{
			pSurface->DeleteMe();
		}
		pDDraw->ReleasedSurfaceVector.clear();
	}

	// Release all buffers from all ddraw devices
	for (m_IDirectDrawX*& pDDraw : DDrawVector)
	{
		for (m_IDirect3DVertexBufferX*& pBuffer : pDDraw->VertexBufferVector)
		{
			pBuffer->ReleaseD9Buffers(BackupData, ResetInterface);
		}
	}

	// Release d3d9 vertex buffer
	if (VertexBuffer)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 vertext buffer";
		ULONG ref = VertexBuffer->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'vertexBuffer' " << ref;
		}
		VertexBuffer = nullptr;
	}

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
		if (d3d9Device)
		{
			d3d9Device->SetPixelShader(nullptr);
		}
		ULONG ref = colorkeyPixelShader->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'colorkeyPixelShader' " << ref;
		}
		colorkeyPixelShader = nullptr;
	}

	ReleasePTCriticalSection();
	ReleaseCriticalSection();
}

// Release d3d9 device
void m_IDirectDrawX::ReleaseD3D9Device()
{
	SetCriticalSection();
	SetPTCriticalSection();

	if (d3d9Device)
	{
		// Check window handle thread
		if (IsWindow(presParams.hDeviceWindow) && GetWindowThreadProcessId(presParams.hDeviceWindow, nullptr) != GetCurrentThreadId())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: trying to release Direct3D9 device from a different thread than the hwnd was created from!");
		}

		ULONG ref = d3d9Device->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Warning: there is still a reference to 'd3d9Device' " << ref;
			while (d3d9Device->Release()) {};
		}
		d3d9Device = nullptr;
	}

	ReleasePTCriticalSection();
	ReleaseCriticalSection();
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

// This method evicts all texture surfaces created with the DDSCAPS2_TEXTUREMANAGE or DDSCAPS2_D3DTEXTUREMANAGE flags from local or nonlocal video memory.
void m_IDirectDrawX::EvictManagedTextures()
{
	SetCriticalSection();

	// Check if any surfaces are locked
	for (m_IDirectDrawSurfaceX*& pSurface : SurfaceVector)
	{
		if (pSurface->IsSurfaceManaged())
		{
			pSurface->ReleaseD9Surface(true, false);
		}
	}

	ReleaseCriticalSection();
}

// Add surface wrapper to vector
void m_IDirectDrawX::AddSurfaceToVector(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	SetCriticalSection();

	if (lpSurfaceX && !DoesSurfaceExist(lpSurfaceX))
	{
		if (lpSurfaceX->IsPrimarySurface())
		{
			PrimarySurface = lpSurfaceX;
		}

		SurfaceVector.push_back(lpSurfaceX);
	}

	ReleaseCriticalSection();
}

// Add released surface wrapper to vector
void m_IDirectDrawX::AddReleasedSurfaceToVector(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	SetCriticalSection();

	if (lpSurfaceX && std::find(ReleasedSurfaceVector.begin(), ReleasedSurfaceVector.end(), lpSurfaceX) == std::end(ReleasedSurfaceVector))
	{
		ReleasedSurfaceVector.push_back(lpSurfaceX);
	}

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

	// Remove attached surface from map
	for (m_IDirectDrawX*& pDDraw : DDrawVector)
	{
		if (lpSurfaceX == pDDraw->PrimarySurface)
		{
			pDDraw->PrimarySurface = nullptr;
			ClipperHWnd = nullptr;
			DisplayPixelFormat = {};
		}
		if (lpSurfaceX == pDDraw->RenderTargetSurface)
		{
			pDDraw->RenderTargetSurface = nullptr;
		}
		if (lpSurfaceX == pDDraw->DepthStencilSurface)
		{
			pDDraw->SetDepthStencilSurface(nullptr);
		}

		auto it = std::find(pDDraw->SurfaceVector.begin(), pDDraw->SurfaceVector.end(), lpSurfaceX);

		if (it != std::end(pDDraw->SurfaceVector))
		{
			lpSurfaceX->ClearDdraw();
			pDDraw->SurfaceVector.erase(it);
		}

		for (m_IDirectDrawSurfaceX*& pSurface : pDDraw->SurfaceVector)
		{
			pSurface->RemoveAttachedSurfaceFromMap(lpSurfaceX);
		}
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

	SetCriticalSection();

	bool hr = std::find(SurfaceVector.begin(), SurfaceVector.end(), lpSurfaceX) != std::end(SurfaceVector);

	ReleaseCriticalSection();

	return hr;
}

// Add clipper wrapper to vector
void m_IDirectDrawX::AddClipperToVector(m_IDirectDrawClipper* lpClipper)
{
	SetCriticalSection();

	if (lpClipper && !DoesClipperExist(lpClipper))
	{
		ClipperVector.push_back(lpClipper);
	}

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

	// Remove clipper from attached surface
	for (m_IDirectDrawX*& pDDraw : DDrawVector)
	{
		auto it = std::find(pDDraw->ClipperVector.begin(), pDDraw->ClipperVector.end(), lpClipper);

		// Remove clipper from vector
		if (it != std::end(pDDraw->ClipperVector))
		{
			lpClipper->ClearDdraw();
			pDDraw->ClipperVector.erase(it);
		}

		for (m_IDirectDrawSurfaceX*& pSurface : pDDraw->SurfaceVector)
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

	SetCriticalSection();

	bool hr = std::find(ClipperVector.begin(), ClipperVector.end(), lpClipper) != std::end(ClipperVector);

	ReleaseCriticalSection();

	return hr;
}

// Add palette wrapper to vector
void m_IDirectDrawX::AddPaletteToVector(m_IDirectDrawPalette* lpPalette)
{
	SetCriticalSection();

	if (lpPalette && !DoesPaletteExist(lpPalette))
	{
		PaletteVector.push_back(lpPalette);
	}

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

	// Remove palette from attached surface
	for (m_IDirectDrawX*& pDDraw : DDrawVector)
	{
		auto it = std::find(pDDraw->PaletteVector.begin(), pDDraw->PaletteVector.end(), lpPalette);

		// Remove palette from vector
		if (it != std::end(pDDraw->PaletteVector))
		{
			lpPalette->ClearDdraw();
			pDDraw->PaletteVector.erase(it);
		}

		for (m_IDirectDrawSurfaceX*& pSurface : pDDraw->SurfaceVector)
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

	SetCriticalSection();

	bool hr = std::find(PaletteVector.begin(), PaletteVector.end(), lpPalette) != std::end(PaletteVector);

	ReleaseCriticalSection();

	return hr;
}

void m_IDirectDrawX::AddVertexBufferToVector(m_IDirect3DVertexBufferX* lpVertexBuffer)
{
	SetCriticalSection();

	if (lpVertexBuffer && !DoesVertexBufferExist(lpVertexBuffer))
	{
		VertexBufferVector.push_back(lpVertexBuffer);
	}

	ReleaseCriticalSection();
}

void m_IDirectDrawX::RemoveVertexBufferFromVector(m_IDirect3DVertexBufferX* lpVertexBuffer)
{
	if (!lpVertexBuffer)
	{
		return;
	}

	SetCriticalSection();

	// Remove palette from attached surface
	for (m_IDirectDrawX*& pDDraw : DDrawVector)
	{
		auto it = std::find(pDDraw->VertexBufferVector.begin(), pDDraw->VertexBufferVector.end(), lpVertexBuffer);

		// Remove vertex buffer from vector
		if (it != std::end(pDDraw->VertexBufferVector))
		{
			lpVertexBuffer->ClearDdraw();
			pDDraw->VertexBufferVector.erase(it);
		}
	}

	ReleaseCriticalSection();
}

bool m_IDirectDrawX::DoesVertexBufferExist(m_IDirect3DVertexBufferX* lpVertexBuffer)
{
	if (!lpVertexBuffer)
	{
		return false;
	}

	SetCriticalSection();

	bool hr = std::find(VertexBufferVector.begin(), VertexBufferVector.end(), lpVertexBuffer) != std::end(VertexBufferVector);

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

HWND m_IDirectDrawX::GetClipperHWnd()
{
	return ClipperHWnd;
}

HRESULT m_IDirectDrawX::SetClipperHWnd(HWND hWnd)
{
	if (Device.IsWindowed)
	{
		HWND OldClipperHWnd = ClipperHWnd;
		if (!hWnd || IsWindow(hWnd))
		{
			ClipperHWnd = hWnd;
		}
		if (!DisplayMode.hWnd && ClipperHWnd && ClipperHWnd != OldClipperHWnd)
		{
			return CreateD3D9Device();
		}
	}
	return DD_OK;
}

HRESULT m_IDirectDrawX::GetD9Gamma(DWORD dwFlags, LPDDGAMMARAMP lpRampData)
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true)))
	{
		return DDERR_GENERIC;
	}

	if (!presParams.Windowed)
	{
		d3d9Device->GetGammaRamp(dwFlags, (D3DGAMMARAMP*)lpRampData);
		return DD_OK;
	}

	return D3DERR_INVALIDCALL;
}

HRESULT m_IDirectDrawX::SetD9Gamma(DWORD dwFlags, LPDDGAMMARAMP lpRampData)
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true)))
	{
		return DDERR_GENERIC;
	}

	if (!presParams.Windowed)
	{
		d3d9Device->SetGammaRamp(0, dwFlags, (D3DGAMMARAMP*)lpRampData);
		return DD_OK;
	}

	return D3DERR_INVALIDCALL;
}

HRESULT m_IDirectDrawX::CopyRenderTargetToBackbuffer(m_IDirectDrawSurfaceX* RenderSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = DDERR_GENERIC;

	// Copy render target to backbuffer
	IDirect3DSurface9* pRenderTarget = RenderSurface->GetD3d9Surface();
	if (pRenderTarget)
	{
		IDirect3DSurface9* pBackBuffer = nullptr;
		if (SUCCEEDED(d3d9Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
		{
			RECT* pSourceRect = nullptr;
			RECT SrcRect = { 0, 0, (LONG)RenderSurface->GetD3d9Width(), (LONG)RenderSurface->GetD3d9Height() };
			HWND hWnd = GetHwnd();

			// Get windlow location and rect
			if (!ExclusiveMode && IsWindow(hWnd))
			{
				// Clip rect
				RECT ClientRect = {};
				if (GetClientRect(hWnd, &ClientRect) && MapWindowPoints(hWnd, HWND_DESKTOP, (LPPOINT)&ClientRect, 2))
				{
					SrcRect.left = max(SrcRect.left, ClientRect.left);
					SrcRect.top = max(SrcRect.top, ClientRect.top);
					SrcRect.right = min(SrcRect.right, ClientRect.right);
					SrcRect.bottom = min(SrcRect.bottom, ClientRect.bottom);

					// Validate rect
					if (SrcRect.left < SrcRect.right && SrcRect.top < SrcRect.bottom)
					{
						pSourceRect = &SrcRect;
					}
				}
			}

			hr = d3d9Device->StretchRect(pRenderTarget, pSourceRect, pBackBuffer, nullptr, D3DTEXF_POINT);
			if (FAILED(hr))
			{
				hr = D3DXLoadSurfaceFromSurface(pBackBuffer, nullptr, nullptr, pRenderTarget, nullptr, pSourceRect, D3DX_FILTER_POINT, 0);
				if (FAILED(hr))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to copy render target to backbuffer!");
				}
			}
			pBackBuffer->Release();
		}
	}

	if (SUCCEEDED(hr))
	{
		RenderSurface->ClearDirtyFlags();
	}

	return hr;
}

HRESULT m_IDirectDrawX::Draw2DSurface(m_IDirectDrawSurfaceX* DrawSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") ";

	// Check draw surface
	if (!DrawSurface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: bad draw surface!");
		return DDERR_GENERIC;
	}

	// Get surface texture
	LPDIRECT3DTEXTURE9 displayTexture = DrawSurface->GetD3d9Texture();
	if (!displayTexture)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get surface texture!");
		return DDERR_GENERIC;
	}

	// Create vertex buffer
	if (!VertexBuffer)
	{
		D3DSURFACE_DESC Desc = {};
		if (FAILED(displayTexture->GetLevelDesc(0, &Desc)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get texture size!");
			return DDERR_GENERIC;
		}
		CreateVertexBuffer(Desc.Width, Desc.Height);
	}

	// Set texture
	if (FAILED(d3d9Device->SetTexture(0, displayTexture)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set surface texture!");
		return DDERR_GENERIC;
	}

	// Handle palette surfaces
	if (DrawSurface->IsPalette())
	{
		// Get palette texture
		LPDIRECT3DTEXTURE9 PaletteTexture = DrawSurface->GetD3d9PaletteTexture();
		if (!PaletteTexture)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get palette texture!");
			return DDERR_GENERIC;
		}

		// Setup shaders
		if (!CreatePaletteShader())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create palette shader!");
			return DDERR_GENERIC;
		}

		// Set palette texture
		DrawSurface->UpdatePaletteData();
		if (FAILED(d3d9Device->SetTexture(1, PaletteTexture)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock palette texture!");
			return DDERR_GENERIC;
		}

		// Set pixel shader
		if (FAILED(d3d9Device->SetPixelShader(palettePixelShader)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set pixel shader!");
			return DDERR_GENERIC;
		}
	}

	// Set vertex buffer and lighting
	if (VertexBuffer)
	{
		// Set vertex shader
		if (FAILED(d3d9Device->SetVertexShader(nullptr)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set vertex shader!");
			return DDERR_GENERIC;
		}

		// Set vertex format
		if (FAILED(d3d9Device->SetFVF(TLVERTEXFVF)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set the current vertex stream format");
			return DDERR_GENERIC;
		}

		// Set stream source
		if (FAILED(d3d9Device->SetStreamSource(0, VertexBuffer, 0, sizeof(TLVERTEX))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set vertex buffer stream source");
			return DDERR_GENERIC;
		}
	}

	DRAWSTATEBACKUP DrawStates;

	// Sampler states
	d3d9Device->GetSamplerState(0, D3DSAMP_MAGFILTER, &DrawStates.ssMagFilter);
	d3d9Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

	// Texture states
	d3d9Device->GetTextureStageState(0, D3DTSS_COLOROP, &DrawStates.tsColorOP);
	d3d9Device->GetTextureStageState(0, D3DTSS_COLORARG1, &DrawStates.tsColorArg1);
	d3d9Device->GetTextureStageState(0, D3DTSS_COLORARG2, &DrawStates.tsColorArg2);
	d3d9Device->GetTextureStageState(0, D3DTSS_ALPHAOP, &DrawStates.tsAlphaOP);
	d3d9Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	d3d9Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d3d9Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	d3d9Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// Render states
	d3d9Device->GetRenderState(D3DRS_LIGHTING, &DrawStates.rsLighting);
	d3d9Device->GetRenderState(D3DRS_ALPHATESTENABLE, &DrawStates.rsAlphaTestEnable);
	d3d9Device->GetRenderState(D3DRS_ALPHABLENDENABLE, &DrawStates.rsAlphaBlendEnable);
	d3d9Device->GetRenderState(D3DRS_FOGENABLE, &DrawStates.rsFogEnable);
	d3d9Device->GetRenderState(D3DRS_ZENABLE, &DrawStates.rsZEnable);
	d3d9Device->GetRenderState(D3DRS_ZWRITEENABLE, &DrawStates.rsZWriteEnable);
	d3d9Device->GetRenderState(D3DRS_STENCILENABLE, &DrawStates.rsStencilEnable);
	d3d9Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	d3d9Device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	d3d9Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	d3d9Device->SetRenderState(D3DRS_FOGENABLE, FALSE);
	d3d9Device->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3d9Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	d3d9Device->SetRenderState(D3DRS_STENCILENABLE, FALSE);

	// Draw primitive
	HRESULT hr = DD_OK;
	if (FAILED(d3d9Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to draw primitive");
		hr = DDERR_GENERIC;
	}

	// Restore sampler states
	d3d9Device->SetSamplerState(0, D3DSAMP_MAGFILTER, DrawStates.ssMagFilter);

	// Restore texture states
	d3d9Device->SetTextureStageState(0, D3DTSS_COLOROP, DrawStates.tsColorOP);
	d3d9Device->SetTextureStageState(0, D3DTSS_COLORARG1, DrawStates.tsColorArg1);
	d3d9Device->SetTextureStageState(0, D3DTSS_COLORARG2, DrawStates.tsColorArg2);
	d3d9Device->SetTextureStageState(0, D3DTSS_ALPHAOP, DrawStates.tsAlphaOP);

	// Restore render states
	d3d9Device->SetRenderState(D3DRS_LIGHTING, DrawStates.rsLighting);
	d3d9Device->SetRenderState(D3DRS_ALPHATESTENABLE, DrawStates.rsAlphaTestEnable);
	d3d9Device->SetRenderState(D3DRS_ALPHABLENDENABLE, DrawStates.rsAlphaBlendEnable);
	d3d9Device->SetRenderState(D3DRS_FOGENABLE, DrawStates.rsFogEnable);
	d3d9Device->SetRenderState(D3DRS_ZENABLE, DrawStates.rsZEnable);
	d3d9Device->SetRenderState(D3DRS_ZWRITEENABLE, DrawStates.rsZWriteEnable);
	d3d9Device->SetRenderState(D3DRS_STENCILENABLE, DrawStates.rsStencilEnable);

	// Reset textures
	d3d9Device->SetTexture(0, nullptr);
	d3d9Device->SetTexture(1, nullptr);

	// Reset pixel shader
	d3d9Device->SetPixelShader(nullptr);

	// Reset dirty flags
	if (SUCCEEDED(hr))
	{
		DrawSurface->ClearDirtyFlags();
	}

	return hr;
}

HRESULT m_IDirectDrawX::Present2DScene(m_IDirectDrawSurfaceX* DrawSurface, RECT* pSourceRect, RECT* pDestRect)
{
	HRESULT hr = DD_OK;

	if (IsUsingThreadPresent())
	{
		return hr;
	}

	do {
		// Begin scene
		if (FAILED(d3d9Device->BeginScene()))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to begin scene!");
			hr = DDERR_GENERIC;
			break;
		}

		// Draw 2D surface before presenting
		HRESULT DrawRet = Draw2DSurface(DrawSurface);
		if (FAILED(DrawRet))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to draw 2D surface!");
			hr = DrawRet;
		}

		// End scene
		if (FAILED(d3d9Device->EndScene()))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to end scene!");
			hr = DDERR_GENERIC;
			break;
		}

		// Present to d3d9
		if (SUCCEEDED(DrawRet))
		{
			if (FAILED(Present(pSourceRect, pDestRect, false)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to present 2D scene!");
				hr = DDERR_GENERIC;
				break;
			}
		}

	} while (false);

	return hr;
}

bool m_IDirectDrawX::IsUsingThreadPresent()
{
	return (PresentThread.IsInitialized && ExclusiveMode && !IsUsing3D());
}

// Present Thread: Wait for the event
DWORD WINAPI PresentThreadFunction(LPVOID)
{
	LOG_LIMIT(100, __FUNCTION__ << " Creating thread!");

	PresentThread.EnableThreadFlag = true;
	while (PresentThread.EnableThreadFlag)
	{
		// Check how long since the last successful present
		LARGE_INTEGER ClickTime = {};
		QueryPerformanceCounter(&ClickTime);
		double DeltaPresentMS = ((ClickTime.QuadPart - PresentThread.LastPresentTime.QuadPart) * 1000.0) / Counter.Frequency.QuadPart;

		DWORD timeout = (DWORD)(Counter.PerFrameMS - DeltaPresentMS < 0.0 ? 0.0 : Counter.PerFrameMS - DeltaPresentMS);

		WaitForSingleObject(PresentThread.workerEvent, timeout);
		ResetEvent(PresentThread.workerEvent);

		if (!PresentThread.EnableThreadFlag)
		{
			break;
		}

		SetPTCriticalSection();

		if (d3d9Device)
		{
			m_IDirectDrawX* pDDraw = nullptr;
			m_IDirectDrawSurfaceX* pPrimarySurface = nullptr;
			for (m_IDirectDrawX* instance : DDrawVector)
			{
				pPrimarySurface = instance->GetPrimarySurface();
				if (pPrimarySurface)
				{
					pDDraw = instance;
					break;
				}
			}
			if (pDDraw && pDDraw->IsUsingThreadPresent() && pPrimarySurface && pPrimarySurface->GetD3d9Texture())
			{
				pPrimarySurface->SetLockCriticalSection();
				pPrimarySurface->SetSurfaceCriticalSection();

				// Begin scene
				d3d9Device->BeginScene();

				// Draw 2D surface before presenting
				pDDraw->Draw2DSurface(pPrimarySurface);

				// End scene
				d3d9Device->EndScene();

				// Present to d3d9
				d3d9Device->Present(nullptr, nullptr, nullptr, nullptr);

				// Store last successful present time
				QueryPerformanceCounter(&PresentThread.LastPresentTime);

				pPrimarySurface->ReleaseSurfaceCriticalSection();
				pPrimarySurface->ReleaseLockCriticalSection();
			}
		}

		ReleasePTCriticalSection();
	}

	LOG_LIMIT(100, __FUNCTION__ << " Closing thread!");

	return S_OK;
}

// Do d3d9 Present
HRESULT m_IDirectDrawX::Present(RECT* pSourceRect, RECT* pDestRect, bool PresentOnFlip)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Skip frame if time lapse is too small
	if (Config.DdrawAutoFrameSkip && !EnableWaitVsync && !IsUsingThreadPresent())
	{
		Counter.FrameSkipCounter++;

		// Get time since last successful endscene
		LARGE_INTEGER ClickTime = {};
		QueryPerformanceCounter(&ClickTime);
		double deltaPresentMS = ((ClickTime.QuadPart - Counter.LastPresentTime.QuadPart) * 1000.0) / Counter.Frequency.QuadPart;

		// Get time since last skipped frame
		double deltaFrameMS = (Counter.LastFrameTime) ? ((ClickTime.QuadPart - Counter.LastFrameTime) * 1000.0) / Counter.Frequency.QuadPart : deltaPresentMS;
		Counter.LastFrameTime = ClickTime.QuadPart;

		// Use last frame time and average frame time to decide if next frame will be less than the screen frequency timer
		if ((deltaPresentMS + (deltaFrameMS * 1.1) < Counter.PerFrameMS) &&
			(deltaPresentMS + ((deltaPresentMS / Counter.FrameSkipCounter) * 1.1) < Counter.PerFrameMS) &&
			deltaPresentMS > 0 && deltaFrameMS > 0)
		{
			Logging::LogDebug() << __FUNCTION__ << " Skipping frame " << deltaPresentMS << "ms screen frequancy " << Counter.PerFrameMS;
			return D3D_OK;
		}
	}

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true)))
	{
		return DDERR_GENERIC;
	}

	// Use WaitForVerticalBlank for wait timer
	if (EnableWaitVsync && !Config.EnableVSync)
	{
		// Check how long since the last successful present
		LARGE_INTEGER ClickTime = {};
		QueryPerformanceCounter(&ClickTime);
		double DeltaPresentMS = ((ClickTime.QuadPart - Counter.LastPresentTime.QuadPart) * 1000.0) / Counter.Frequency.QuadPart;

		// Don't wait for vsync if the last frame was too long ago
		if (DeltaPresentMS < Counter.PerFrameMS)
		{
			WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, nullptr);
		}
		EnableWaitVsync = false;
	}

	// Present everthing, skip Preset when using DdrawWriteToGDI
	HRESULT hr = D3DERR_DEVICELOST;
	if (!IsUsingThreadPresent())
	{
		if (RenderTargetSurface && (IsPrimaryRenderTarget() || IsPrimaryFlipSurface()))
		{
			if (D3DDeviceInterface && !D3DDeviceInterface->IsDeviceInScene())
			{
				d3d9Device->BeginScene();
				CopyRenderTargetToBackbuffer(PrimarySurface);
				d3d9Device->EndScene();

				hr = d3d9Device->Present(nullptr, nullptr, nullptr, nullptr);
			}
			if (PresentOnFlip && IsPrimaryFlipSurface())
			{
				SetRenderTargetSurface(RenderTargetSurface);
			}
		}
		else
		{
			hr = d3d9Device->Present(pSourceRect, pDestRect, nullptr, nullptr);
		}
	}
	
	// Test cooperative level
	if (hr == D3DERR_DEVICELOST)
	{
		hr = TestD3D9CooperativeLevel();
		hr = (hr == DDERR_NOEXCLUSIVEMODE) ? DD_OK : hr;
	}

	// Reset device
	if (hr == D3DERR_DEVICENOTRESET)
	{
		hr = ReinitDevice();
	}

	// Present failure
	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to present scene: " << (DDERR)hr);
		return hr;
	}

	// Redraw window if it has moved from its last location
	HWND hWnd = GetHwnd();
	RECT ClientRect = {};
	if (GetWindowRect(hWnd, &ClientRect) && LastWindowLocation.right && LastWindowLocation.bottom)
	{
		if (ClientRect.left != LastWindowLocation.left || ClientRect.top != LastWindowLocation.top ||
			ClientRect.right != LastWindowLocation.right || ClientRect.bottom != LastWindowLocation.bottom)
		{
			RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
	}
	LastWindowLocation = ClientRect;

	// Store new click time after frame draw is complete
	QueryPerformanceCounter(&Counter.LastPresentTime);

	return DD_OK;
}

DWORD GetDDrawBitsPixel(HWND hWnd)
{
	if (hWnd)
	{
		char name[256] = {};
		GetClassNameA(hWnd, name, sizeof(name));
		if ((DWORD)hWnd == 0x00010010 || strcmp(name, "#32769") == S_OK)	// Is menu
		{
			return 0;
		}
	}
	if (Config.DdrawOverrideBitMode)
	{
		return Config.DdrawOverrideBitMode;
	}
	if (!DDrawVector.empty() && DisplayMode.hWnd)
	{
		return (Exclusive.BPP) ? Exclusive.BPP : (DisplayMode.BPP) ? DisplayMode.BPP : DDrawVector.data()[0]->GetDisplayBPP(hWnd);
	}
	return 0;
}

DWORD GetDDrawWidth()
{
	if (DDrawVector.size() && DisplayMode.hWnd)
	{
		return Exclusive.Width ? Exclusive.Width : 0;
	}
	return 0;
}

DWORD GetDDrawHeight()
{
	if (DDrawVector.size() && DisplayMode.hWnd)
	{
		return Exclusive.Height ? Exclusive.Height : 0;
	}
	return 0;
}
