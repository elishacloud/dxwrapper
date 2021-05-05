/**
* Copyright (C) 2021 Elisha Riedlinger
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
#include <d3dkmthk.h>
#include "Dllmain\DllMain.h"

constexpr DWORD MaxVidMemory  = 0x32000000;	// 512 MBs
constexpr DWORD UsedVidMemory = 0x00100000;	// 1 MB

const D3DFORMAT D9DisplayFormat = D3DFMT_X8R8G8B8;

#define STATUS_SUCCESS 0
typedef NTSTATUS(WINAPI *D3DKMTWaitForVerticalBlankEventProc)(const D3DKMT_WAITFORVERTICALBLANKEVENT *Arg1);
typedef NTSTATUS(WINAPI *D3DKMTOpenAdapterFromHdcProc)(D3DKMT_OPENADAPTERFROMHDC *Arg1);
typedef NTSTATUS(WINAPI *D3DKMTCloseAdapterProc)(const D3DKMT_CLOSEADAPTER *Arg1);

// ddraw interface counter
DWORD ddrawRefCount = 0;

// Store a list of ddraw devices
std::vector<m_IDirectDrawX*> DDrawVector;

// Mouse hook
HANDLE ghWriteEvent = nullptr;
HANDLE threadID = nullptr;
HHOOK m_hook = nullptr;
bool bMouseChange = false;
POINT mousePos;

// Cooperative level settings
HWND MainhWnd = nullptr;
HDC MainhDC = nullptr;

// Exclusive mode
bool ExclusiveMode;
HWND ExclusiveHwnd;
DWORD ExclusiveWidth;
DWORD ExclusiveHeight;
DWORD ExclusiveBPP;
DWORD ExclusiveRefreshRate;

// Application set display mode
DWORD displayModeWidth;
DWORD displayModeHeight;
DWORD displayModeBPP;
DWORD displayModeRefreshRate;

// Display resolution
bool SetDefaultDisplayMode;			// Set native resolution
bool SetResolution;
DWORD displayWidth;
DWORD displayHeight;
DWORD displayRefreshRate;			// Refresh rate for fullscreen

// Last resolution
DWORD LastWidth;
DWORD LastHeight;
DWORD LastBPP;

// Display mode settings
bool isWindowed;					// Window mode enabled
bool AntiAliasing;
bool AllowModeX;
bool MultiThreaded;
bool FUPPreserve;
bool NoWindowChanges;
bool DynamicTexturesSupported;

// High resolution counter used for auto frame skipping
bool FrequencyFlag;
LARGE_INTEGER clockFrequency, clickTime, lastPresentTime;
LONGLONG lastFrameTime;
DWORD FrameCounter;
DWORD monitorRefreshRate;
DWORD monitorHeight;

// Direct3D9 flags
bool IsInScene;
bool EnableWaitVsync;

// Direct3D9 Objects
LPDIRECT3D9 d3d9Object;
LPDIRECT3DDEVICE9 d3d9Device;
D3DPRESENT_PARAMETERS presParams;

// D3DKMT vertical blank
bool VSyncLoaded = false;
HMODULE gdi_dll = nullptr;
D3DKMTWaitForVerticalBlankEventProc m_pD3DKMTWaitForVerticalBlankEvent = nullptr;
D3DKMTOpenAdapterFromHdcProc m_pD3DKMTOpenAdapterFromHdc = nullptr;
D3DKMTCloseAdapterProc m_pD3DKMTCloseAdapter = nullptr;
D3DKMT_OPENADAPTERFROMHDC openAdapter = {};
D3DKMT_CLOSEADAPTER closeAdapter = {};
D3DKMT_WAITFORVERTICALBLANKEVENT VBlankEvent = {};

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
		if (!lplpDDClipper)
		{
			return DDERR_INVALIDPARAMS;
		}

		*lplpDDClipper = new m_IDirectDrawClipper(dwFlags);

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
		if (!lplpDDPalette || !lpDDColorArray)
		{
			return DDERR_INVALIDPARAMS;
		}

		m_IDirectDrawPalette *PaletteX = new m_IDirectDrawPalette(this, dwFlags, lpDDColorArray);

		AddPaletteToVector(PaletteX);

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

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (ProxyDirectXVersion > 3)
	{
		if (!lplpDDSurface || !lpDDSurfaceDesc || lpDDSurfaceDesc->dwSize != sizeof(DDSURFACEDESC))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc) ? lpDDSurfaceDesc->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);

		HRESULT hr = CreateSurface2(&Desc2, lplpDDSurface, pUnkOuter, DirectXVersion);

		Desc2.ddsCaps.dwCaps2 = 0;
		Desc2.ddsCaps.dwVolumeDepth = 0;
		ConvertSurfaceDesc(*lpDDSurfaceDesc, Desc2);

		return hr;
	}

	// Adjust Height and Width
	DWORD Width = 0;
	DWORD Height = 0;
	if (lpDDSurfaceDesc && (lpDDSurfaceDesc->dwFlags & DDSD_WIDTH) && (lpDDSurfaceDesc->dwFlags & DDSD_HEIGHT))
	{
		Width = lpDDSurfaceDesc->dwWidth;
		Height = lpDDSurfaceDesc->dwHeight;
		lpDDSurfaceDesc->dwWidth += lpDDSurfaceDesc->dwWidth % 2;
		lpDDSurfaceDesc->dwHeight += lpDDSurfaceDesc->dwHeight % 2;
	}

	HRESULT hr = GetProxyInterfaceV3()->CreateSurface(lpDDSurfaceDesc, (LPDIRECTDRAWSURFACE*)lplpDDSurface, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDSurface)
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

	if (Config.Dd7to9)
	{
		if (!lplpDDSurface || !lpDDSurfaceDesc2 || lpDDSurfaceDesc2->dwSize != sizeof(DDSURFACEDESC2))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc2) ? lpDDSurfaceDesc2->dwSize : -1));
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

		// Check for MipMap
		if ((lpDDSurfaceDesc2->dwFlags & DDSD_MIPMAPCOUNT) || (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_MIPMAP) ||
			(lpDDSurfaceDesc2->dwMipMapCount > 1))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: MipMap not Implemented.");
		}

		// Check for zbuffer
		if (((lpDDSurfaceDesc2->dwFlags & DDSD_PIXELFORMAT) && (lpDDSurfaceDesc2->ddpfPixelFormat.dwFlags & DDPF_ZBUFFER)) ||
			(lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_ZBUFFER) || (lpDDSurfaceDesc2->dwFlags & DDSD_ZBUFFERBITDEPTH))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: z-buffer not Implemented.");
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
		DWORD UnsupportedDDSCaps = (DDSCAPS_RESERVED1 | DDSCAPS_RESERVED2 | DDSCAPS_RESERVED3 | DDSCAPS_OWNDC | DDSCAPS_LIVEVIDEO |
			DDSCAPS_HWCODEC | DDSCAPS_ALLOCONLOAD | DDSCAPS_VIDEOPORT | DDSCAPS_NONLOCALVIDMEM);
		DWORD UnsupportedDDSCaps2 = (DDSCAPS2_RESERVED4 | DDSCAPS2_HINTDYNAMIC | DDSCAPS2_HINTSTATIC | DDSCAPS2_RESERVED1 | DDSCAPS2_RESERVED2 |
			DDSCAPS2_OPAQUE);
		if ((lpDDSurfaceDesc2->ddsCaps.dwCaps & UnsupportedDDSCaps) || (lpDDSurfaceDesc2->ddsCaps.dwCaps2 & UnsupportedDDSCaps2) || lpDDSurfaceDesc2->ddsCaps.dwVolumeDepth)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: non-supported ddsCaps! " << Logging::hex(lpDDSurfaceDesc2->ddsCaps.dwCaps & UnsupportedDDSCaps) << " " <<
				Logging::hex(lpDDSurfaceDesc2->ddsCaps.dwCaps2 & UnsupportedDDSCaps2) << " " << lpDDSurfaceDesc2->ddsCaps.dwVolumeDepth);
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false)))
		{
			return DDERR_GENERIC;
		}

		// Check pixel format
		if (lpDDSurfaceDesc2->dwFlags & DDSD_PIXELFORMAT)
		{
			DDPIXELFORMAT ddpfPixelFormat;
			memcpy(&ddpfPixelFormat, &lpDDSurfaceDesc2->ddpfPixelFormat, sizeof(DDPIXELFORMAT));
			ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
			D3DFORMAT Format = GetDisplayFormat(ddpfPixelFormat);
			Format = (Format == D3DFMT_R8G8B8 || Format == D3DFMT_B8G8R8 || Format == D3DFMT_X8B8G8R8) ? D3DFMT_X8R8G8B8 : (Format == D3DFMT_A8B8G8R8) ? D3DFMT_A8R8G8B8 : Format;
			DWORD Usage = (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) ? D3DUSAGE_RENDERTARGET :
				((lpDDSurfaceDesc2->dwFlags & DDSD_MIPMAPCOUNT) || ((lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_MIPMAP))) ? D3DUSAGE_AUTOGENMIPMAP :
				(ddpfPixelFormat.dwFlags & (DDPF_ZBUFFER | DDPF_STENCILBUFFER)) ? D3DUSAGE_DEPTHSTENCIL : 0;
			D3DRESOURCETYPE Resource = ((lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_TEXTURE)) ? D3DRTYPE_TEXTURE : D3DRTYPE_SURFACE;

			if (FAILED(d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D9DisplayFormat, Usage, Resource, Format)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: non-supported pixel format! " << Usage << " " << Resource << " " << Format << " " << lpDDSurfaceDesc2->ddpfPixelFormat);
				return DDERR_INVALIDPIXELFORMAT;
			}
		}

		// Updates for surface description
		lpDDSurfaceDesc2->dwFlags |= DDSD_CAPS;
		lpDDSurfaceDesc2->ddsCaps.dwCaps4 = DDSCAPS4_CREATESURFACE |											// Indicates surface was created using CreateSurface()
			((lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) ? DDSCAPS4_PRIMARYSURFACE : NULL);		// Indicates surface is a primary surface or a backbuffer of a primary surface
		if (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_FLIP)
		{
			lpDDSurfaceDesc2->ddsCaps.dwCaps |= DDSCAPS_FRONTBUFFER;
		}
		lpDDSurfaceDesc2->dwReserved = 0;

		// BackBufferCount must be at least 1
		if (lpDDSurfaceDesc2->dwFlags & DDSD_BACKBUFFERCOUNT)
		{
			if (!lpDDSurfaceDesc2->dwBackBufferCount)
			{
				lpDDSurfaceDesc2->dwBackBufferCount = 1;
			}
		}
		else
		{
			lpDDSurfaceDesc2->dwBackBufferCount = 0;
		}

		// Add flag for 3D device
		if ((DirectXVersion < 4) && (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE))
		{
			lpDDSurfaceDesc2->ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
		}

		// Remove unused flags
		if (!lpDDSurfaceDesc2->dwWidth || !lpDDSurfaceDesc2->dwHeight)
		{
			lpDDSurfaceDesc2->dwFlags &= ~(DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH);
		}
		if (!lpDDSurfaceDesc2->dwRefreshRate)
		{
			lpDDSurfaceDesc2->dwFlags &= ~DDSD_REFRESHRATE;
		}

		// Get present parameters
		if (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
		{
			// Anti-aliasing
			bool OldAntiAliasing = AntiAliasing;
			AntiAliasing = ((lpDDSurfaceDesc2->ddsCaps.dwCaps2 & DDSCAPS2_HINTANTIALIASING) && (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_3DDEVICE));

			// Check if there is a change in the present parameters
			if (d3d9Device && AntiAliasing != OldAntiAliasing)
			{
				// Release existing d3d9 device
				ReleaseD3d9Device();
			}
		}

		// Setup d3d9 device
		if (!d3d9Device)
		{
			CreateD3D9Device();
		}

		m_IDirectDrawSurfaceX *p_IDirectDrawSurfaceX = new m_IDirectDrawSurfaceX(&d3d9Device, this, DirectXVersion, lpDDSurfaceDesc2);

		*lplpDDSurface = (LPDIRECTDRAWSURFACE7)p_IDirectDrawSurfaceX->GetWrapperInterfaceX(DirectXVersion);

		return DD_OK;
	}

	if (ProxyDirectXVersion != DirectXVersion && lpDDSurfaceDesc2)
	{
		// BackBufferCount must be at least 1
		if (lpDDSurfaceDesc2->dwFlags & DDSD_BACKBUFFERCOUNT)
		{
			if (!lpDDSurfaceDesc2->dwBackBufferCount)
			{
				lpDDSurfaceDesc2->dwBackBufferCount = 1;
			}
		}
		else
		{
			lpDDSurfaceDesc2->dwBackBufferCount = 0;
		}

		// Add flag for 3D device
		if ((DirectXVersion < 4) && (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE))
		{
			lpDDSurfaceDesc2->ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
		}
	}

	// Adjust Height and Width
	DWORD Width = 0;
	DWORD Height = 0;
	if (lpDDSurfaceDesc2 && (lpDDSurfaceDesc2->dwFlags & DDSD_WIDTH) && (lpDDSurfaceDesc2->dwFlags & DDSD_HEIGHT))
	{
		Width = lpDDSurfaceDesc2->dwWidth;
		Height = lpDDSurfaceDesc2->dwHeight;
		lpDDSurfaceDesc2->dwWidth += lpDDSurfaceDesc2->dwWidth % 2;
		lpDDSurfaceDesc2->dwHeight += lpDDSurfaceDesc2->dwHeight % 2;
	}

	HRESULT hr = ProxyInterface->CreateSurface(lpDDSurfaceDesc2, lplpDDSurface, pUnkOuter);

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

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		lpDDSurfaceX->GetSurfaceDesc2(&Desc2);
		Desc2.ddsCaps.dwCaps &= ~DDSCAPS_PRIMARYSURFACE;		// Remove Primary surface flag

		m_IDirectDrawSurfaceX *p_IDirectDrawSurfaceX = new m_IDirectDrawSurfaceX(&d3d9Device, this, DirectXVersion, &Desc2);

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

				DDSURFACEDESC Desc;
				Desc.dwSize = sizeof(DDSURFACEDESC);
				ConvertSurfaceDesc(Desc, *lpDDSurfaceDesc2);

				return self->lpCallback(&Desc, self->lpContext);
			}
		} CallbackContext;
		CallbackContext.lpContext = lpContext;
		CallbackContext.lpCallback = lpEnumModesCallback;

		DDSURFACEDESC2 Desc2;
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
		DDSURFACEDESC2 Desc2 = { NULL };
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
					Desc2.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_REFRESHRATE | DDSD_PITCH;
					Desc2.dwWidth = d3ddispmode.Width;
					Desc2.dwHeight = d3ddispmode.Height;
					Desc2.dwRefreshRate = (SetRefreshRate) ? d3ddispmode.RefreshRate : 0;

					// Set adapter pixel format
					Desc2.dwFlags |= DDSD_PIXELFORMAT;
					Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
					SetDisplayFormat(DisplayBitCount, Desc2.ddpfPixelFormat);
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

		DDSURFACEDESC2 Desc2;
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
	} CallbackContext;
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

	if (Config.Dd7to9)
	{
		if ((lpDDSurfaceDesc2 && lpDDSurfaceDesc2->dwSize != sizeof(DDSURFACEDESC2)) || (!lpDDSurfaceDesc2 && !(dwFlags & DDENUMSURFACES_ALL)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDSurfaceDesc2) ? lpDDSurfaceDesc2->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

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
	CallbackContext.lpCallback = lpEnumSurfacesCallback7;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ConvertSurfaceDescTo2 = (ProxyDirectXVersion > 3 && DirectXVersion < 4);

	return ProxyInterface->EnumSurfaces(dwFlags, lpDDSurfaceDesc2, &CallbackContext, EnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawX::FlipToGDISurface()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// ToDo: Do proper implementation
		return DD_OK;
	}

	return ProxyInterface->FlipToGDISurface();
}

HRESULT m_IDirectDrawX::GetCaps(LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDHELCaps)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if ((!lpDDDriverCaps && !lpDDHELCaps) ||
		(lpDDDriverCaps && lpDDDriverCaps->dwSize != sizeof(DDCAPS_DX1) &&
		lpDDDriverCaps->dwSize != sizeof(DDCAPS_DX3) && lpDDDriverCaps->dwSize != sizeof(DDCAPS_DX5) &&
		lpDDDriverCaps->dwSize != sizeof(DDCAPS_DX6) && lpDDDriverCaps->dwSize != sizeof(DDCAPS_DX7)) ||
		(lpDDHELCaps && lpDDHELCaps->dwSize != sizeof(DDCAPS_DX1) &&
		lpDDHELCaps->dwSize != sizeof(DDCAPS_DX3) && lpDDHELCaps->dwSize != sizeof(DDCAPS_DX5) &&
		lpDDHELCaps->dwSize != sizeof(DDCAPS_DX6) && lpDDHELCaps->dwSize != sizeof(DDCAPS_DX7)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. dwSize: " << ((lpDDDriverCaps) ? lpDDDriverCaps->dwSize : -1) << " " << ((lpDDHELCaps) ? lpDDHELCaps->dwSize : -1));
		return DDERR_INVALIDPARAMS;
	}

	DDCAPS DriverCaps, HELCaps;
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
		DWORD dwVidTotal = MaxVidMemory;
		DWORD dwVidFree = MaxVidMemory - UsedVidMemory;
		GetAvailableVidMem2(nullptr, &dwVidTotal, &dwVidFree);

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
	}
	else
	{
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

		DDSURFACEDESC2 Desc2;
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
		if (FAILED(SetDisplayFormat(displayModeBits, lpDDSurfaceDesc2->ddpfPixelFormat)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Not implemented bit count " << displayModeBits);
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

		// Cpoy data to array
		if (lpCodes)
		{
			DWORD SizeToCopy = min(NumFourCCs, *lpNumCodes);
			memcpy(lpCodes, FourCCTypes, SizeToCopy * sizeof(DWORD));
		}

		// Return value
		*lpNumCodes = NumFourCCs;
		return DD_OK;
	}

	return ProxyInterface->GetFourCCCodes(lpNumCodes, lpCodes);
}

HRESULT m_IDirectDrawX::GetGDISurface(LPDIRECTDRAWSURFACE7 FAR * lplpGDIDDSSurface, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
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
		isWindowed = true;

		// Release existing d3d9device
		ReleaseD3d9Device();

		return DD_OK;
	}

	return ProxyInterface->RestoreDisplayMode();
}

HRESULT m_IDirectDrawX::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for valid parameters
		if (!(dwFlags & (DDSCL_EXCLUSIVE | DDSCL_NORMAL)) ||																			// An application must set either the DDSCL_EXCLUSIVE or the DDSCL_NORMAL flag
			((dwFlags & DDSCL_NORMAL) && (dwFlags & (DDSCL_ALLOWMODEX | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN))) ||						// Normal flag cannot be used with Modex, Exclusive or Fullscreen flags
			((dwFlags & DDSCL_EXCLUSIVE) && !(dwFlags & DDSCL_FULLSCREEN)) ||															// If Exclusive flag is set then Fullscreen flag must be set
			((dwFlags & DDSCL_FULLSCREEN) && !(dwFlags & DDSCL_EXCLUSIVE)) ||															// If Fullscreen flag is set then Exclusive flag must be set
			((dwFlags & DDSCL_ALLOWMODEX) && (!(dwFlags & DDSCL_EXCLUSIVE) || !(dwFlags & DDSCL_FULLSCREEN))) ||						// If AllowModeX is set then Exclusive and Fullscreen flags must be set
			((dwFlags & DDSCL_SETDEVICEWINDOW) && (dwFlags & DDSCL_SETFOCUSWINDOW)) ||													// SetDeviceWindow flag cannot be used with SetFocusWindow flag
			(!hWnd && !(dwFlags & DDSCL_NORMAL)) ||																						// hWnd can only be null if normal flag is set
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

		// ToDo: The DDSCL_EXCLUSIVE flag must be set to call functions that can adversely affect performance of other applications.

		// If SetCooperativeLevel is called once in a process, a binding is established between the process and the window.
		// If it is called again in the same process with a different non-null window handle, it returns the DDERR_HWNDALREADYSET error value.

		bool ChangeMode = false;

		// Set windowed mode
		if (dwFlags & DDSCL_NORMAL)
		{
			// Check for exclusive mode
			if (ExclusiveMode && hWnd && ExclusiveHwnd == hWnd)
			{
				ExclusiveMode = false;
				ExclusiveHwnd = nullptr;
				ExclusiveWidth = 0;
				ExclusiveHeight = 0;
				ExclusiveBPP = 0;
				ExclusiveRefreshRate = 0;
				ChangeMode = true;
			}
		}
		else if (dwFlags & DDSCL_FULLSCREEN)
		{
			if (ExclusiveMode && ExclusiveHwnd != hWnd && IsWindow(ExclusiveHwnd))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Exclusive mode already set.");
				return DDERR_EXCLUSIVEMODEALREADYSET;
			}
			if (!ExclusiveMode)
			{
				ChangeMode = true;
			}
			ExclusiveMode = true;
			ExclusiveHwnd = hWnd;
		}

		// Set device flags
		AllowModeX = ((dwFlags & DDSCL_ALLOWMODEX) != 0);
		MultiThreaded = ((dwFlags & DDSCL_MULTITHREADED) != 0);
		FUPPreserve = ((dwFlags & (DDSCL_FPUPRESERVE | DDSCL_FPUSETUP)) != 0);
		NoWindowChanges = ((dwFlags & DDSCL_NOWINDOWCHANGES) != 0);

		// Check if DC needs to be released
		if (MainhWnd && MainhDC && (MainhWnd != hWnd))
		{
			CloseVSync();
			ReleaseDC(MainhWnd, MainhDC);
			MainhDC = nullptr;
		}

		MainhWnd = hWnd;

		if (MainhWnd && !MainhDC)
		{
			MainhDC = ::GetDC(MainhWnd);
		}

		// Reset if mode was changed and primary surface does not exist
		if (ChangeMode && d3d9Device && !PrimarySurface)
		{
			// Release existing d3d9device
			ReleaseD3d9Device();
		}

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->SetCooperativeLevel(hWnd, dwFlags);

	// Release previouse Exclusive flag
	// Hook window message to get notified when the window is about to exit to remove the exclusive flag
	if (SUCCEEDED(hr) && (dwFlags & DDSCL_EXCLUSIVE) && IsWindow(hWnd) && hWnd != chWnd)
	{
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
							lpDDraw->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
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

			// Overload WndProc
			Utils::SetWndProcFilter(hWnd);

			// Removing WS_CAPTION
			SetWindowLong(hWnd, GWL_STYLE, lStyle & ~WS_CAPTION);
			SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_NOOWNERZORDER);

			// Resetting WndProc
			Utils::RestoreWndProcFilter(hWnd);
		}
	}

	return hr;
}

HRESULT m_IDirectDrawX::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!dwWidth || !dwHeight || (dwBPP != 8 && dwBPP != 16 && dwBPP != 24 && dwBPP != 32))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid parameters. " << dwWidth << "x" << dwHeight << " " << dwBPP);
			return DDERR_INVALIDPARAMS;
		}

		bool ChangeMode = false;
		DWORD NewBPP = (Config.DdrawOverrideBitMode) ? Config.DdrawOverrideBitMode : dwBPP;

		if (displayModeWidth != dwWidth || displayModeHeight != dwHeight || displayModeBPP != NewBPP || displayModeRefreshRate != dwRefreshRate)
		{
			DWORD FoundWidth = dwWidth;
			DWORD FoundHeight = dwHeight;

			// Check if it is a supported resolution
			if (ExclusiveMode && (!Config.EnableWindowMode || Config.FullscreenWindowMode))
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

			ChangeMode = true;
			displayModeWidth = dwWidth;
			displayModeHeight = dwHeight;
			displayModeBPP = NewBPP;
			displayModeRefreshRate = dwRefreshRate;
			isWindowed = !ExclusiveMode;

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

		// Update the d3d9 device to use new display mode
		if (ChangeMode)
		{
			SetResolution = ExclusiveMode;

			// Recreate d3d9 device
			CreateD3D9Device();
		}

		return DD_OK;
	}

	// Force color mode
	dwBPP = (Config.DdrawOverrideBitMode) ? Config.DdrawOverrideBitMode : dwBPP;

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
		D3DRASTER_STATUS RasterStatus;

		// Check flags
		switch (dwFlags)
		{
		case DDWAITVB_BLOCKBEGIN:
		case DDWAITVB_BLOCKEND:
			// Get vertical blank begin first, then get vertical blank end if requested
			// Use D3DKMT for vertical blank begin
			if (InitVSync() && m_pD3DKMTWaitForVerticalBlankEvent(&VBlankEvent) == STATUS_SUCCESS)
			{
				if (dwFlags == DDWAITVB_BLOCKBEGIN)
				{
					return DD_OK;
				}
			}
			// Use raster status for vertical blank begin (uses high CPU)
			else if (d3d9Device && SUCCEEDED(d3d9Device->GetRasterStatus(0, &RasterStatus)))
			{
				while (SUCCEEDED(d3d9Device->GetRasterStatus(0, &RasterStatus)) && !RasterStatus.InVBlank);
				if (dwFlags == DDWAITVB_BLOCKBEGIN)
				{
					return DD_OK;
				}
			}
			// Use raster status for vertical blank end (uses high CPU)
			while (SUCCEEDED(d3d9Device->GetRasterStatus(0, &RasterStatus)) && RasterStatus.InVBlank);
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

		// TODO: Get correct total video memory size
		DWORD TotalMemory = 0;
		DWORD AvailableMemory = 0;

		// Get available video memory
		if (d3d9Device)
		{
			AvailableMemory = d3d9Device->GetAvailableTextureMem();
		}
		else
		{
			TotalMemory = Utils::GetVideoRam(MaxVidMemory);
		}

		// If memory cannot be found just return default memory
		if (!TotalMemory)
		{
			TotalMemory = (AvailableMemory) ? AvailableMemory : MaxVidMemory;
		}

		// If memory cannot be found just return default memory
		if (!AvailableMemory)
		{
			AvailableMemory = TotalMemory - UsedVidMemory;
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
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
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
			return DDERR_GENERIC;
		}

		// Check device status
		if (d3d9Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			ReinitDevice();
		}
		else
		{
			SetCriticalSection();

			for (m_IDirectDrawSurfaceX *pSurface : SurfaceVector)
			{
				pSurface->ResetSurfaceDisplay();
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
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->StartModeTest(lpModesToTest, dwNumEntries, dwFlags);
}

HRESULT m_IDirectDrawX::EvaluateMode(DWORD dwFlags, DWORD * pSecondsUntilTimeout)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
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

		// Exclusive mode
		ExclusiveMode = false;
		ExclusiveHwnd = nullptr;
		ExclusiveWidth = 0;
		ExclusiveHeight = 0;
		ExclusiveBPP = 0;
		ExclusiveRefreshRate = 0;

		// Application display mode
		displayModeWidth = 0;
		displayModeHeight = 0;
		displayModeBPP = 0;
		displayModeRefreshRate = 0;

		// Last resolution
		LastWidth = 0;
		LastHeight = 0;
		LastBPP = 0;

		// Display mode settings
		isWindowed = true;
		AntiAliasing = false;
		AllowModeX = false;
		MultiThreaded = false;
		FUPPreserve = false;
		NoWindowChanges = false;

		// High resolution counter
		FrequencyFlag = (QueryPerformanceFrequency(&clockFrequency) != 0);
		clickTime.QuadPart = 0;
		lastPresentTime.QuadPart = 0;
		lastFrameTime = 0;
		FrameCounter = 0;
		monitorRefreshRate = 0;
		monitorHeight = 0;

		// Direct3D9 Objects
		d3d9Object = nullptr;
		d3d9Device = nullptr;

		ZeroMemory(&presParams, sizeof(D3DPRESENT_PARAMETERS));

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

		static bool EnableMouseHook = ((Config.DdrawUseNativeResolution || Config.DdrawOverrideWidth || Config.DdrawOverrideHeight) &&
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

	DWORD ref = InterlockedDecrement(&ddrawRefCount);

	SetCriticalSection();

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
		pSurface->ClearDdraw();
		pSurface->ReleaseD9Surface();
	}
	SurfaceVector.clear();

	// Release palettes
	for (m_IDirectDrawPalette *pPalette : PaletteVector)
	{
		pPalette->ClearDdraw();
	}
	PaletteVector.clear();

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

	if (ref == 0)
	{
		// Release d3d9device
		if (d3d9Device)
		{
			ReleaseD9Interface(&d3d9Device);
		}

		// Release d3d9object
		if (d3d9Object)
		{
			ReleaseD9Interface(&d3d9Object);
		}

		// Close vsync
		if (VSyncLoaded)
		{
			CloseVSync();
		}

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

bool m_IDirectDrawX::InitVSync()
{
	if (VSyncLoaded)
	{
		return true;
	}

	static bool RunOnce = true;
	if (RunOnce)
	{
		RunOnce = false;
		gdi_dll = LoadLibrary("gdi32.dll");
		m_pD3DKMTWaitForVerticalBlankEvent = (D3DKMTWaitForVerticalBlankEventProc)GetProcAddress(gdi_dll, "D3DKMTWaitForVerticalBlankEvent");
		m_pD3DKMTOpenAdapterFromHdc = (D3DKMTOpenAdapterFromHdcProc)GetProcAddress(gdi_dll, "D3DKMTOpenAdapterFromHdc");
		m_pD3DKMTCloseAdapter = (D3DKMTCloseAdapterProc)GetProcAddress(gdi_dll, "D3DKMTCloseAdapter");
	}

	HDC hDC = GetDC();

	if (m_pD3DKMTWaitForVerticalBlankEvent && m_pD3DKMTOpenAdapterFromHdc && m_pD3DKMTCloseAdapter && hDC)
	{
		openAdapter.hDc = hDC;
		if (m_pD3DKMTOpenAdapterFromHdc(&openAdapter) == STATUS_SUCCESS)
		{
			VSyncLoaded = true;
			VBlankEvent.hAdapter = openAdapter.hAdapter;
			closeAdapter.hAdapter = openAdapter.hAdapter;
		}
	}
	return VSyncLoaded;
}

void m_IDirectDrawX::CloseVSync()
{
	if (VSyncLoaded && m_pD3DKMTCloseAdapter)
	{
		if (m_pD3DKMTCloseAdapter(&closeAdapter) == STATUS_SUCCESS)
		{
			VSyncLoaded = false;
		}
	}
}

HWND m_IDirectDrawX::GetHwnd()
{
	return MainhWnd;
}

HDC m_IDirectDrawX::GetDC()
{
	return MainhDC;
}

bool m_IDirectDrawX::IsExclusiveMode()
{
	return ExclusiveMode;
}

void m_IDirectDrawX::GetFullDisplay(DWORD &Width, DWORD &Height, DWORD &RefreshRate, DWORD &BPP)
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
	else if (isWindowed && IsWindow(hWnd) && !Config.DdrawWriteToGDI)
	{
		RECT Rect = { NULL };
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
		SetCriticalSection();

		for (m_IDirectDrawSurfaceX *pSurface : SurfaceVector)
		{
			pSurface->ResetSurfaceDisplay();
		}

		ReleaseCriticalSection();
	}
	LastWidth = Width;
	LastHeight = Height;
	LastBPP = BPP;
}

void m_IDirectDrawX::GetDisplay(DWORD &Width, DWORD &Height)
{
	Width = presParams.BackBufferWidth;
	Height = presParams.BackBufferHeight;
}

void m_IDirectDrawX::SetD3DDevice(m_IDirect3DDeviceX *D3DDevice)
{
	if (!D3DDeviceInterface)
	{
		ReleaseD3d9Device();
	}
	D3DDeviceInterface = D3DDevice;
}

bool m_IDirectDrawX::IsDynamicTexturesSupported()
{
	return DynamicTexturesSupported;
}

HRESULT m_IDirectDrawX::CheckInterface(char *FunctionName, bool CheckD3DDevice)
{
	// Check for device
	if (!d3d9Object)
	{
		// Declare Direct3DCreate9
		static PFN_Direct3DCreate9 Direct3DCreate9 = reinterpret_cast<PFN_Direct3DCreate9>(Direct3DCreate9_out);

		if (!Direct3DCreate9)
		{
			LOG_LIMIT(100, FunctionName << " Error: failed to get 'Direct3DCreate9' ProcAddress of d3d9.dll!");
			return DDERR_GENERIC;
		}

		d3d9Object = Direct3DCreate9(D3D_SDK_VERSION);

		// Error creating Direct3D9
		if (!d3d9Object)
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

LPDIRECT3D9 m_IDirectDrawX::GetDirect3D9Object()
{
	return d3d9Object;
}

LPDIRECT3DDEVICE9 *m_IDirectDrawX::GetDirect3D9Device()
{
	return &d3d9Device;
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
		// Release existing d3d9 device
		if (d3d9Device)
		{
			ReleaseD3d9Device();
		}

		// Check device caps to make sure it supports dynamic textures
		DynamicTexturesSupported = false;
		D3DCAPS9 d3dcaps;
		ZeroMemory(&d3dcaps, sizeof(D3DCAPS9));
		if (SUCCEEDED(d3d9Object->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dcaps)))
		{
			DynamicTexturesSupported = (d3dcaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES);
		}

		// Get hwnd
		HWND hWnd = GetHwnd();

		// Get width and height
		DWORD BackBufferWidth = displayWidth;
		DWORD BackBufferHeight = displayHeight;
		if (!BackBufferWidth || !BackBufferHeight)
		{
			if (isWindowed && IsWindow(hWnd))
			{
				RECT Rect = { NULL };
				GetClientRect(hWnd, &Rect);
				BackBufferWidth = Rect.right - Rect.left;
				BackBufferHeight = Rect.bottom - Rect.top;
			}
			if (!BackBufferWidth || !BackBufferHeight)
			{
				Utils::GetScreenSize(hWnd, BackBufferWidth, BackBufferHeight);
			}
		}

		// Get current resolution
		DWORD CurrentWidth, CurrentHeight;
		Utils::GetScreenSize(hWnd, CurrentWidth, CurrentHeight);

		// Set display window
		ZeroMemory(&presParams, sizeof(presParams));

		// Width/height
		presParams.BackBufferWidth = BackBufferWidth;
		presParams.BackBufferHeight = BackBufferHeight;
		// Discard swap
		presParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
		// Backbuffer
		presParams.BackBufferCount = 1;
		// Auto stencel
		presParams.EnableAutoDepthStencil = TRUE;
		// Auto stencel format
		presParams.AutoDepthStencilFormat = D3DFMT_D24S8;
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

		// Set behavior flags
		DWORD BehaviorFlags = ((d3dcaps.VertexProcessingCaps) ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING) |
			((MultiThreaded || !Config.SingleProcAffinity) ? D3DCREATE_MULTITHREADED : 0) |
			((FUPPreserve) ? D3DCREATE_FPU_PRESERVE : 0) |
			((NoWindowChanges) ? D3DCREATE_NOWINDOWCHANGES : 0);

		Logging::LogDebug() << __FUNCTION__ << " wnd: " << hWnd << " D3d9 Device params: " << presParams << " flags: " << Logging::hex(BehaviorFlags);

		// Create d3d9 Device
		if (FAILED(d3d9Object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, BehaviorFlags, &presParams, &d3d9Device)))
		{
			DWORD dwProcessId = 0;
			GetWindowThreadProcessId(hWnd, &dwProcessId);
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create Direct3D9 device! " << ((dwProcessId && dwProcessId != GetCurrentThreadId()) ? "Calling from a non-owner thread! " : "")
				<< presParams.BackBufferWidth << "x" << presParams.BackBufferHeight << " refresh: " << presParams.FullScreen_RefreshRateInHz <<
				" format: " << presParams.BackBufferFormat << " wnd: " << hWnd);
			hr = DDERR_GENERIC;
			break;
		}

		// Reset flags after creating device
		IsInScene = false;
		EnableWaitVsync = false;

		// Set window pos
		if (IsWindow(hWnd))
		{
			// Get new resolution
			DWORD NewWidth, NewHeight;
			Utils::GetScreenSize(hWnd, NewWidth, NewHeight);

			// Set display change message
			if ((SetResolution || NewWidth != CurrentWidth || NewHeight != CurrentHeight) && NewWidth && NewHeight)
			{
				SetResolution = false;
				DWORD bpp = (displayModeBPP) ? displayModeBPP : 32;
				DWORD res = (WORD)NewWidth | ((WORD)NewHeight << 16);
				SendMessage(hWnd, WM_DISPLAYCHANGE, (WPARAM)bpp, (LPARAM)res);
			}

			// Get window size
			RECT tempRect = { NULL };
			GetClientRect(hWnd, &tempRect);

			// Send message about window changes
			WINDOWPOS winpos = { nullptr, hWnd, tempRect.left, tempRect.top, tempRect.right, tempRect.bottom, WM_NULL };
			SendMessage(hWnd, WM_WINDOWPOSCHANGED, (WPARAM)TRUE, (LPARAM)&winpos);
		}

		// Store display frequency
		monitorRefreshRate = (presParams.FullScreen_RefreshRateInHz) ? presParams.FullScreen_RefreshRateInHz : Utils::GetRefreshRate(hWnd);
		DWORD tmpWidth = 0;
		Utils::GetScreenSize(hWnd, tmpWidth, monitorHeight);

	} while (false);

	ReleaseCriticalSection();

	// Success
	return hr;
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
	else if (hr != D3DERR_DEVICENOTRESET)
	{
		return DDERR_GENERIC;
	}

	// Release surfaces to prepare for reset
	ReleaseAllDirectDrawD9Surfaces();

	// Attempt to reset the device
	if (FAILED(d3d9Device->Reset(&presParams)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to reset Direct3D9 device");
		return DDERR_GENERIC;
	}

	// Reset flags after resetting device
	IsInScene = false;
	EnableWaitVsync = false;

	// Success
	return DD_OK;
}

template <typename T>
void m_IDirectDrawX::ReleaseD9Interface(T **ppInterface)
{
	if (ppInterface && *ppInterface)
	{
		DWORD x = 0, z = 0;
		do
		{
			z = (*ppInterface)->Release();
		} while (z != 0 && ++x < 100);

		// Error checking
		if (z != 0)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: failed to release Direct3D9 interface");
		}
		else
		{
			*ppInterface = nullptr;
		}
	}
}

// Release all surfaces from all ddraw devices
void m_IDirectDrawX::ReleaseAllDirectDrawD9Surfaces()
{
	SetCriticalSection();

	for (m_IDirectDrawX *pDDraw : DDrawVector)
	{
		pDDraw->ReleaseAllD9Surfaces();
	}

	ReleaseCriticalSection();
}

// Release all d3d9 surfaces
void m_IDirectDrawX::ReleaseAllD9Surfaces()
{
	SetCriticalSection();

	for (m_IDirectDrawSurfaceX *pSurface : SurfaceVector)
	{
		pSurface->ReleaseD9Surface();
	}

	ReleaseCriticalSection();
}

// Release all d3d9 classes for Release()
void m_IDirectDrawX::ReleaseD3d9Device()
{
	// Release all existing surfaces
	ReleaseAllDirectDrawD9Surfaces();

	// Release device
	if (d3d9Device)
	{
		ReleaseD9Interface(&d3d9Device);
	}

	// Reset flags
	EnableWaitVsync = false;
}

// Add surface wrapper to vector
void m_IDirectDrawX::AddSurfaceToVector(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX || DoesSurfaceExist(lpSurfaceX))
	{
		return;
	}

	if (lpSurfaceX->IsPrimarySurface())
	{
		PrimarySurface = lpSurfaceX;
	}

	SetCriticalSection();

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
			pSurface->ReleaseD9Surface();
		}
	}

	ReleaseCriticalSection();
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

	if (it != std::end(PaletteVector))
	{
		PaletteVector.erase(it);
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
		[=](auto pSurface) -> bool { return pSurface == lpPalette; });

	if (it != std::end(PaletteVector))
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
	DWORD TotalVidMem = (lpdwTotal) ? *lpdwTotal : (lpdwFree) ? *lpdwFree : MaxVidMemory;
	TotalVidMem = (!TotalVidMem || TotalVidMem > MaxVidMemory) ? MaxVidMemory : TotalVidMem;
	if (lpdwTotal)
	{
		*lpdwTotal = TotalVidMem;
	}
	if (lpdwFree && (!*lpdwFree || *lpdwFree >= TotalVidMem))
	{
		*lpdwFree = TotalVidMem - UsedVidMemory;
	}
}

void m_IDirectDrawX::SetVsync()
{
	EnableWaitVsync = true;
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

	// Begine scene
	if (!IsInScene)
	{
		if (FAILED(d3d9Device->BeginScene()))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to end scene");
			return DDERR_GENERIC;
		}
	}
	IsInScene = true;

	// Draw primitive
	if (FAILED(d3d9Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to draw primitive");
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

	// End scene
	if (FAILED(d3d9Device->EndScene()))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to end scene");
		return DDERR_GENERIC;
	}
	IsInScene = false;

	// Use WaitForVerticalBlank for wait timer
	if (UseVSync)
	{
		WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, nullptr);
		EnableWaitVsync = false;
	}

	// Present everthing, skip Preset for SWAT 2
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

	// Clear device before BeginScene
	d3d9Device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	// BeginScene after present is done
	if (FAILED(d3d9Device->BeginScene()))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to begin scene");
		return DDERR_GENERIC;
	}
	IsInScene = true;

	return hr;
}

int WINAPI dd_GetDeviceCaps(HDC hdc, int index)
{
	static GetDeviceCapsProc m_pGetDeviceCaps = (Wrapper::ValidProcAddress(GetDeviceCaps_out)) ? (GetDeviceCapsProc)GetDeviceCaps_out : nullptr;

	if (ddrawRefCount && index == BITSPIXEL)
	{
		int BPP = (ExclusiveBPP) ? ExclusiveBPP : (displayModeBPP) ? displayModeBPP : 0;
		if (BPP)
		{
			return (BPP == 15) ? 16 : BPP;		// When nIndex is BITSPIXEL and the device has 15bpp or 16bpp, the return value is 16.
		}
	}

	if (!m_pGetDeviceCaps)
	{
		return 0;
	}

	return m_pGetDeviceCaps(hdc, index);
}
