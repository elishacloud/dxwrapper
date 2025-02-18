/**
* Copyright (C) 2024 Elisha Riedlinger
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
#include "GDI\WndProc.h"
#include "Dllmain\DllMain.h"
#include "d3d9\d3d9External.h"
#include "d3dddi\d3dddiExternal.h"
#include "Shaders\PaletteShader.h"
#include "Shaders\ColorKeyShader.h"
#include "Shaders\GammaPixelShader.h"

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

// Store a list of ddraw devices
std::vector<m_IDirectDrawX*> DDrawVector;

// Cached wrapper interface
namespace {
	m_IDirectDraw* WrapperInterfaceBackup = nullptr;
	m_IDirectDraw2* WrapperInterfaceBackup2 = nullptr;
	m_IDirectDraw3* WrapperInterfaceBackup3 = nullptr;
	m_IDirectDraw4* WrapperInterfaceBackup4 = nullptr;
	m_IDirectDraw7* WrapperInterfaceBackup7 = nullptr;
}

// Default resolution
DWORD DefaultWidth = 0;
DWORD DefaultHeight = 0;
RECT LastWindowRect = {};

// Exclusive mode settings
bool ExclusiveMode = false;
bool FullScreenWindowed = false;
DISPLAYSETTINGS Exclusive = {};
HWND LastUsedHWnd = nullptr;	// Only initialize this here

// Clipper
HWND ClipperHWnd = nullptr;

// Display mode settings
DISPLAYSETTINGS DisplayMode = {};

// Device settings
DEVICESETTINGS Device = {};

// Display pixel format
DDPIXELFORMAT DisplayPixelFormat = {};

// Gamma data
bool IsGammaSet = false;
D3DGAMMARAMP RampData;
D3DGAMMARAMP DefaultRampData;

// Last used surface resolution
DWORD LastSetWidth = 0;
DWORD LastSetHeight = 0;
DWORD LastSetBPP = 0;

// Initial screen resolution
DWORD InitWidth = 0;
DWORD InitHeight = 0;

// Cached FourCC list
std::vector<D3DFORMAT> FourCCsList;

// Used for dummy mipmaps
extern std::vector<BYTE> dummySurface;

// Mouse hook
MOUSEHOOK MouseHook = {};

// High resolution counter used for auto frame skipping
HIGHRESCOUNTER Counter = {};

#ifdef ENABLE_PROFILING
std::chrono::steady_clock::time_point presentTime;
#endif

// Preset from another thread
PRESENTTHREAD PresentThread = {};

inline static void SetPTCriticalSection()
{
	if (PresentThread.IsInitialized)
	{
		EnterCriticalSection(&PresentThread.ddpt);
	}
}

inline static void ReleasePTCriticalSection()
{
	if (PresentThread.IsInitialized)
	{
		LeaveCriticalSection(&PresentThread.ddpt);
	}
}

// Direct3D9 flags
bool EnableWaitVsync = false;

// Direct3D9 Objects
LPDIRECT3D9 d3d9Object = nullptr;
LPDIRECT3DDEVICE9 d3d9Device = nullptr;
D3DPRESENT_PARAMETERS presParams = {};
LPDIRECT3DTEXTURE9 GammaLUTTexture = nullptr;
LPDIRECT3DTEXTURE9 ScreenCopyTexture = nullptr;
LPDIRECT3DPIXELSHADER9 palettePixelShader = nullptr;
LPDIRECT3DPIXELSHADER9 colorkeyPixelShader = nullptr;
LPDIRECT3DPIXELSHADER9 gammaPixelShader = nullptr;
LPDIRECT3DVERTEXBUFFER9 validateDeviceVertexBuffer = nullptr;
LPDIRECT3DINDEXBUFFER9 d3d9IndexBuffer = nullptr;

bool UsingCustomRenderTarget = false;
TLVERTEX DeviceVertices[4];
bool IsDeviceVerticesSet = false;
bool UsingShader32f = false;
DWORD IndexBufferSize = 0;
DWORD BehaviorFlags = 0;
HWND hFocusWindow = nullptr;
DWORD FocusWindowThreadID = 0;

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

	if ((riid == GetWrapperType(DxVersion) && riid != IID_IDirectDraw3) || riid == IID_IUnknown)
	{
		*ppvObj = GetWrapperInterfaceX(DxVersion);

		AddRef(DxVersion);

		return DD_OK;
	}

	if (Config.Dd7to9)
	{
		if (((riid == IID_IDirect3D || riid == IID_IDirect3D2 || riid == IID_IDirect3D3) && !IsCreatedEx()) || (riid == IID_IDirect3D7 && IsCreatedEx()))
		{
			if (Config.DdrawDisableDirect3DCaps)
			{
				return E_NOINTERFACE;
			}

			DxVersion = GetGUIDVersion(riid);

			if (!D3DInterface)
			{
				D3DInterface = new m_IDirect3DX(this, DxVersion, DirectXVersion);
			}

			AddRef(DirectXVersion);

			*ppvObj = D3DInterface->GetWrapperInterfaceX(DxVersion);

			return DD_OK;
		}
	}

	HRESULT hr = ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DirectXVersion));

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
	case 0:
		if (WrapperInterface7) return WrapperInterface7;
		if (WrapperInterface4) return WrapperInterface4;
		if (WrapperInterface3) return WrapperInterface3;
		if (WrapperInterface2) return WrapperInterface2;
		if (WrapperInterface) return WrapperInterface;
		break;
	case 1:
		return GetInterfaceAddress(WrapperInterface, WrapperInterfaceBackup, (LPDIRECTDRAW)ProxyInterface, this);
	case 2:
		return GetInterfaceAddress(WrapperInterface2, WrapperInterfaceBackup2, (LPDIRECTDRAW2)ProxyInterface, this);
	case 3:
		return GetInterfaceAddress(WrapperInterface3, WrapperInterfaceBackup3, (LPDIRECTDRAW3)ProxyInterface, this);
	case 4:
		return GetInterfaceAddress(WrapperInterface4, WrapperInterfaceBackup4, (LPDIRECTDRAW4)ProxyInterface, this);
	case 7:
		return GetInterfaceAddress(WrapperInterface7, WrapperInterfaceBackup7, (LPDIRECTDRAW7)ProxyInterface, this);
	}
	LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
	return nullptr;
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

HRESULT m_IDirectDrawX::CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR * lplpDDClipper, IUnknown FAR * pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpDDClipper || pUnkOuter)
		{
			return DDERR_INVALIDPARAMS;
		}

		m_IDirectDrawClipper* Interface = CreateDirectDrawClipper(nullptr, this, dwFlags);

		if (DirectXVersion > 3)
		{
			for (auto& entry : ClipperList)
			{
				if (entry.Interface == Interface)
				{
					entry.DxVersion = DirectXVersion;
					entry.RefCount = 1;

					AddRef(entry.DxVersion);

					break;
				}
			}
		}

		*lplpDDClipper = Interface;

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->CreateClipper(dwFlags, lplpDDClipper, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDClipper)
	{
		*lplpDDClipper = CreateDirectDrawClipper(*lplpDDClipper, nullptr, dwFlags);
	}

	return hr;
}

void m_IDirectDrawX::AddClipper(m_IDirectDrawClipper* lpClipper)
{
	if (!lpClipper)
	{
		return;
	}

	ClipperList.push_back({ lpClipper, 0, 0 });
}

void m_IDirectDrawX::ClearClipper(m_IDirectDrawClipper* lpClipper)
{
	// Find and remove the clipper from the list
	auto it = std::find_if(ClipperList.begin(), ClipperList.end(),
		[lpClipper](auto entry) {
			return entry.Interface == lpClipper;
		});
	if (it != ClipperList.end())
	{
		if (it->RefCount == 1)
		{
			Release(it->DxVersion);
		}
		ClipperList.erase(it);
	}
}

HRESULT m_IDirectDrawX::CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, LPDIRECTDRAWPALETTE FAR * lplpDDPalette, IUnknown FAR * pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpDDPalette || !lpDDColorArray || pUnkOuter)
		{
			return DDERR_INVALIDPARAMS;
		}

		m_IDirectDrawPalette* Interface = CreateDirectDrawPalette(nullptr, this, dwFlags, lpDDColorArray);

		if (DirectXVersion > 3)
		{
			for (auto& entry : PaletteList)
			{
				if (entry.Interface == Interface)
				{
					entry.DxVersion = DirectXVersion;
					entry.RefCount = 1;

					AddRef(entry.DxVersion);

					break;
				}
			}
		}

		*lplpDDPalette = Interface;

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->CreatePalette(dwFlags, lpDDColorArray, lplpDDPalette, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDPalette)
	{
		*lplpDDPalette = CreateDirectDrawPalette(*lplpDDPalette, nullptr, 0, nullptr);
	}

	return hr;
}

void m_IDirectDrawX::AddPalette(m_IDirectDrawPalette* lpPalette)
{
	if (!lpPalette)
	{
		return;
	}

	PaletteList.push_back({ lpPalette, 0, 0 });
}

void m_IDirectDrawX::ClearPalette(m_IDirectDrawPalette* lpPalette)
{
	// Find and remove the palette from the list
	auto it = std::find_if(PaletteList.begin(), PaletteList.end(),
		[lpPalette](auto entry) {
			return entry.Interface == lpPalette;
		});
	if (it != PaletteList.end())
	{
		if (it->RefCount == 1)
		{
			Release(it->DxVersion);
		}
		PaletteList.erase(it);
	}
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
	*lplpDDSurface = nullptr;

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

		// Check for flags only valid with textures
		if ((lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_TEXTURE) == NULL &&
			((lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_MIPMAP) || (lpDDSurfaceDesc2->ddsCaps.dwCaps2 & (DDSCAPS2_HINTDYNAMIC | DDSCAPS2_HINTSTATIC | DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: missing texture flag: " << lpDDSurfaceDesc2->ddsCaps);
			return DDERR_INVALIDPARAMS;
		}

		// Check MipMap count
		if ((lpDDSurfaceDesc2->ddsCaps.dwCaps & (DDSCAPS_MIPMAP | DDSCAPS_COMPLEX | DDSCAPS_TEXTURE)) == (DDSCAPS_MIPMAP | DDSCAPS_COMPLEX | DDSCAPS_TEXTURE) &&
			(((lpDDSurfaceDesc2->dwFlags & DDSD_MIPMAPCOUNT) && (lpDDSurfaceDesc2->dwMipMapCount != 1) &&
				((lpDDSurfaceDesc2->dwFlags & (DDSD_WIDTH | DDSD_HEIGHT)) == (DDSD_WIDTH | DDSD_HEIGHT) && lpDDSurfaceDesc2->dwWidth && lpDDSurfaceDesc2->dwHeight) &&
				GetMaxMipMapLevel(lpDDSurfaceDesc2->dwWidth, lpDDSurfaceDesc2->dwHeight) < lpDDSurfaceDesc2->dwMipMapCount) ||
				((!(lpDDSurfaceDesc2->dwFlags & DDSD_WIDTH) || !(lpDDSurfaceDesc2->dwFlags & DDSD_HEIGHT) || !lpDDSurfaceDesc2->dwWidth || !lpDDSurfaceDesc2->dwHeight) &&
					(!(lpDDSurfaceDesc2->dwFlags & DDSD_MIPMAPCOUNT) || ((lpDDSurfaceDesc2->dwFlags & DDSD_MIPMAPCOUNT) && (lpDDSurfaceDesc2->dwMipMapCount == 0))))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid MipMap count. Count: " << lpDDSurfaceDesc2->dwMipMapCount <<
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
				Logging::hex(lpDDSurfaceDesc2->ddsCaps.dwCaps2 & UnsupportedDDSCaps2) << " " << lpDDSurfaceDesc2->ddsCaps);
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false)))
		{
			return DDERR_GENERIC;
		}

		DDSURFACEDESC2 Desc2 = *lpDDSurfaceDesc2;

		// Add 3D device flag
		if (DirectXVersion <= 2 && (Desc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE))
		{
			Desc2.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
		}

		// Check pixel format flag
		if ((Desc2.dwFlags & DDSD_PIXELFORMAT) && !Desc2.ddpfPixelFormat.dwFlags)
		{
			Desc2.dwFlags &= ~DDSD_PIXELFORMAT;
		}

		// Check pixel format
		if (Desc2.dwFlags & DDSD_PIXELFORMAT)
		{
			Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
			const DWORD Usage = (Desc2.ddpfPixelFormat.dwFlags & (DDPF_ZBUFFER | DDPF_STENCILBUFFER)) ? D3DUSAGE_DEPTHSTENCIL :
				(Desc2.ddsCaps.dwCaps & (DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE)) ? D3DUSAGE_RENDERTARGET : 0;
			const D3DRESOURCETYPE Resource = ((lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_TEXTURE) && Usage != D3DUSAGE_DEPTHSTENCIL) ? D3DRTYPE_TEXTURE : D3DRTYPE_SURFACE;
			const D3DFORMAT Format = GetDisplayFormat(Desc2.ddpfPixelFormat);
			const D3DFORMAT TestFormat = ConvertSurfaceFormat(Format);

			if (IsUnsupportedFormat(TestFormat) ||
				(FAILED(d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D9DisplayFormat, Usage, Resource, TestFormat)) &&
				FAILED(d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D9DisplayFormat, Usage, Resource, GetFailoverFormat(TestFormat)))))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: non-supported pixel format! " << Usage << " " << Resource << " " << Format << "->" << TestFormat << " " << Desc2.ddpfPixelFormat);
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

		// Remove unused or conflicting flags
		if (!Desc2.dwWidth || !Desc2.dwHeight || !(Desc2.dwFlags & DDSD_WIDTH) || !(Desc2.dwFlags & DDSD_HEIGHT))
		{
			Desc2.dwFlags &= ~(DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH);
			Desc2.dwWidth = 0;
			Desc2.dwHeight = 0;
		}
		if ((Desc2.ddsCaps.dwCaps & DDSCAPS_TEXTURE) && (Desc2.dwFlags & DDSD_REFRESHRATE))
		{
			Desc2.dwFlags &= ~DDSD_REFRESHRATE;
			LOG_LIMIT(100, __FUNCTION__ << " Warning: removing refresh flag from surface!");
		}
		if (Desc2.dwFlags & DDSD_REFRESHRATE)
		{
			Desc2.dwRefreshRate = 0;
		}
		// Removing mipmap flags from primary and Direct3D surface
		if ((Desc2.ddsCaps.dwCaps & (DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE)) &&
			((Desc2.dwFlags & DDSD_MIPMAPCOUNT) || (Desc2.ddsCaps.dwCaps & DDSCAPS_MIPMAP)) && Desc2.dwMipMapCount != 1)
		{
			Desc2.dwFlags &= ~DDSD_MIPMAPCOUNT;
			Desc2.ddsCaps.dwCaps &= ~DDSCAPS_MIPMAP;
			LOG_LIMIT(100, __FUNCTION__ << " Warning: removing MipMap flag from primary or 3D surface!");
		}
		// Removing texture and mipmap flags from stencil buffer surfaces
		if (Desc2.ddpfPixelFormat.dwFlags & (DDPF_ZBUFFER | DDPF_STENCILBUFFER))
		{
			Desc2.dwFlags &= ~DDSD_MIPMAPCOUNT;
			Desc2.ddsCaps.dwCaps &= ~(DDSCAPS_TEXTURE | DDSCAPS_MIPMAP);
			Desc2.ddsCaps.dwCaps2 &= ~(DDSCAPS2_HINTDYNAMIC | DDSCAPS2_HINTSTATIC | DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE);
		}

		// Check for depth stencil surface
		if (!Config.DdrawOverrideStencilFormat && (Desc2.dwFlags & DDSD_PIXELFORMAT) && (Desc2.ddpfPixelFormat.dwFlags & (DDPF_ZBUFFER | DDPF_STENCILBUFFER)))
		{
			if (Config.DdrawOverrideStencilFormat)
			{
				SetPixelDisplayFormat((D3DFORMAT)Config.DdrawOverrideStencilFormat, Desc2.ddpfPixelFormat);
			}

			Logging::Log() << __FUNCTION__ << " Found depth stencil surface: " << GetDisplayFormat(Desc2.ddpfPixelFormat);
		}

		// Add flag for 3D device
		if (DirectXVersion == 1 && (Desc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE))
		{
			Desc2.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
		}

		// Log primary surface
		if (Desc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
		{
			Logging::Log() << __FUNCTION__ << " Primary surface " << (Desc2.dwFlags & DDSD_PIXELFORMAT ? GetDisplayFormat(Desc2.ddpfPixelFormat) : D3DFMT_UNKNOWN) <<
				" " << Desc2.dwWidth << "x" << Desc2.dwHeight <<
				" dwFlags: " << Logging::hex(Desc2.dwFlags) <<
				" ddsCaps: " << Logging::hex(Desc2.ddsCaps.dwCaps) << ", " << Logging::hex(Desc2.ddsCaps.dwCaps2) << ", " << LOWORD(Desc2.ddsCaps.dwVolumeDepth);
		}

		DWORD DxVersion = DirectXVersion > 3 ? DirectXVersion : 1;	// The first 3 versions create interface version 1

		m_IDirectDrawSurfaceX *Interface = new m_IDirectDrawSurfaceX(this, DxVersion, &Desc2);

		if (Desc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
		{
			PrimarySurface = Interface;
		}

		if (DirectXVersion > 3)
		{
			for (auto& entry : SurfaceList)
			{
				if (entry.Interface == Interface)
				{
					entry.DxVersion = DirectXVersion;
					entry.RefCount = IsCreatedEx() && (Desc2.ddsCaps.dwCaps & DDSCAPS_TEXTURE) ? 2 : 1;

					for (UINT x = 0; x < entry.RefCount; x++)
					{
						AddRef(DirectXVersion);
					}
					break;
				}
			}
		}

		*lplpDDSurface = (LPDIRECTDRAWSURFACE7)Interface->GetWrapperInterfaceX(DxVersion);

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

		// Add 3D device flag
		if (DirectXVersion <= 2 && (Desc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE))
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
		if (!lpDDSurface || !lplpDupDDSurface)
		{
			return DDERR_INVALIDPARAMS;
		}

		m_IDirectDrawSurfaceX *lpDDSurfaceX = nullptr;
		if (FAILED(lpDDSurface->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpDDSurfaceX)) || !DoesSurfaceExist(lpDDSurfaceX))
		{
			return DDERR_INVALIDPARAMS;
		}

		// A primary surface, 3-D surface, or implicitly created surface cannot be duplicated.
		if (lpDDSurfaceX->IsPrimarySurface() || lpDDSurfaceX->IsSurface3D() || !lpDDSurfaceX->CanSurfaceBeDeleted())
		{
			return DDERR_CANTDUPLICATE;
		}

		DDSURFACEDESC2 Desc2 = {};
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		lpDDSurfaceX->GetSurfaceDesc2(&Desc2, 0, DirectXVersion);

		m_IDirectDrawSurfaceX* Interface = new m_IDirectDrawSurfaceX(this, DirectXVersion, &Desc2);

		*lplpDupDDSurface = (LPDIRECTDRAWSURFACE7)Interface->GetWrapperInterfaceX(DirectXVersion);

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

void m_IDirectDrawX::AddSurface(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return;
	}

	SurfaceList.push_back({ lpSurfaceX, 0, 0 });
}

void m_IDirectDrawX::ClearSurface(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	// Remove attached surface from map
	for (const auto& pDDraw : DDrawVector)
	{
		m_IDirect3DDeviceX* D3DDevice = pDDraw->D3DDeviceInterface;
		if (D3DDevice)
		{
			D3DDevice->ClearSurface(lpSurfaceX);
		}
		if (lpSurfaceX == pDDraw->PrimarySurface)
		{
			pDDraw->PrimarySurface = nullptr;
			ClipperHWnd = nullptr;
			DisplayPixelFormat = {};
		}
		if (lpSurfaceX == pDDraw->RenderTargetSurface)
		{
			pDDraw->SetRenderTargetSurface(nullptr);
		}
		if (lpSurfaceX == pDDraw->DepthStencilSurface)
		{
			pDDraw->SetDepthStencilSurface(nullptr);
		}

		auto it = std::find_if(pDDraw->SurfaceList.begin(), pDDraw->SurfaceList.end(),
			[lpSurfaceX](auto entry) {
				return entry.Interface == lpSurfaceX;
			});
		if (it != std::end(pDDraw->SurfaceList))
		{
			for (UINT x = 0; x < it->RefCount; x++)
			{
				if (!Release(it->DxVersion)) break;
			}

			pDDraw->SurfaceList.erase(it);
		}

		for (const auto& pSurface : pDDraw->SurfaceList)
		{
			pSurface.Interface->RemoveAttachedSurfaceFromMap(lpSurfaceX);
		}
	}
}

HRESULT m_IDirectDrawX::EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback, DWORD DirectXVersion)
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

		return EnumDisplayModes2(dwFlags, (lpDDSurfaceDesc) ? &Desc2 : nullptr, &CallbackContext, EnumDisplay::ConvertCallback, DirectXVersion);
	}

	return GetProxyInterfaceV3()->EnumDisplayModes(dwFlags, lpDDSurfaceDesc, lpContext, lpEnumModesCallback);
}

HRESULT m_IDirectDrawX::EnumDisplayModes2(DWORD dwFlags, LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext, LPDDENUMMODESCALLBACK2 lpEnumModesCallback2, DWORD DirectXVersion)
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

			// Add resolution to global list
			AddDisplayResolution(d3ddispmode.Width, d3ddispmode.Height);

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
					return (Entry.Width == d3ddispmode.Width && Entry.Height == d3ddispmode.Height && Entry.RefreshRate == RefreshRate);
				});

			// Check mode
			if (ResolutionOkToUse && !ResolutionAlreadySent &&
				(!EnumWidth || d3ddispmode.Width == EnumWidth) &&
				(!EnumHeight || d3ddispmode.Height == EnumHeight))
			{
				// Store resolution
				ResolutionList.push_back({ d3ddispmode.Width, d3ddispmode.Height, RefreshRate });
			}
		}

		struct EnumDisplay
		{
			static void GetSurfaceDesc2(DDSURFACEDESC2& Desc2, DWORD Width, DWORD Height, DWORD RefreshRate, DWORD bpMode)
			{
				Desc2.dwSize = sizeof(DDSURFACEDESC2);
				Desc2.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_REFRESHRATE;
				Desc2.dwWidth = Width;
				Desc2.dwHeight = Height;
				Desc2.dwRefreshRate = RefreshRate;

				// Set adapter pixel format
				Desc2.dwFlags |= DDSD_PIXELFORMAT;
				Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
				D3DFORMAT Format = SetDisplayFormat(Desc2.ddpfPixelFormat, bpMode);

				// Set pitch
				Desc2.dwFlags |= DDSD_PITCH;
				Desc2.lPitch = ComputePitch(Format, GetByteAlignedWidth(Desc2.dwWidth, bpMode), bpMode);
			}
		};

		// Set display bit count modes
		std::vector<DWORD> BitCountList;
		if (DisplayAllModes)
		{
			BitCountList.push_back(32);
			BitCountList.push_back(16);
			BitCountList.push_back(8);
		}
		else
		{
			BitCountList.push_back(DisplayBitCount);
		}

		// Loop through each bit count
		if (DirectXVersion == 1)
		{
			for (const auto& entry : ResolutionList)
			{
				for (DWORD bpMode : BitCountList)
				{
					// Get surface desc options
					DDSURFACEDESC2 Desc2 = {};
					EnumDisplay::GetSurfaceDesc2(Desc2, entry.Width, entry.Height, entry.RefreshRate, bpMode);

					if (lpEnumModesCallback2(&Desc2, lpContext) == DDENUMRET_CANCEL)
					{
						return DD_OK;
					}
				}
			}
		}
		else
		{
			for (DWORD bpMode : BitCountList)
			{
				for (const auto& entry : ResolutionList)
				{
					// Get surface desc options
					DDSURFACEDESC2 Desc2 = {};
					EnumDisplay::GetSurfaceDesc2(Desc2, entry.Width, entry.Height, entry.RefreshRate, bpMode);

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
			for (const auto& pSurfaceX : SurfaceList)
			{
				LPDIRECTDRAWSURFACE7 pSurface7 = (LPDIRECTDRAWSURFACE7)pSurfaceX.Interface->GetWrapperInterfaceX(DirectXVersion);

				if (pSurface7)
				{
					DDSURFACEDESC2 Desc2 = {};
					Desc2.dwSize = sizeof(DDSURFACEDESC2);
					pSurfaceX.Interface->GetSurfaceDesc2(&Desc2, 0, DirectXVersion);

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
		else if (!SetDisplayFormat(lpDDSurfaceDesc2->ddpfPixelFormat, displayModeBits))
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
				if (!IsUnsupportedFormat(format) && SUCCEEDED(d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, format)))
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
		// Returns D3D_OK if successful, otherwise it returns an error.
		return D3D_OK;
	}

	HRESULT hr = ProxyInterface->Initialize(lpGUID);

	return (hr == DDERR_ALREADYINITIALIZED) ? DD_OK : hr;
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
		Exclusive.Width = DefaultWidth;
		Exclusive.Height = DefaultHeight;
		Exclusive.BPP = Utils::GetBitCount(GetHwnd());
		DisplayMode.Width = DefaultWidth;
		DisplayMode.Height = DefaultHeight;
		DisplayMode.BPP = Exclusive.BPP;
		Device.Width = (Config.DdrawUseNativeResolution || Config.DdrawOverrideWidth) ? Device.Width : DefaultWidth;
		Device.Height = (Config.DdrawUseNativeResolution || Config.DdrawOverrideHeight) ? Device.Height : DefaultHeight;

		// Release d3d9 device
		if (d3d9Device)
		{
			ReleaseAllD9Resources(true, false);
			ReleaseD9Device();
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

		HWND LasthWnd = DisplayMode.hWnd;
		bool LastFPUPreserve = Device.FPUPreserve;
		bool LastWindowed = Device.IsWindowed;

		// Set windowed mode
		if (dwFlags & DDSCL_NORMAL)
		{
			// Clear exclusive mode
			if ((ExclusiveMode && hWnd && Exclusive.hWnd == hWnd && Exclusive.SetBy == this) || !IsWindow(Exclusive.hWnd))
			{
				ExclusiveMode = false;
				Exclusive = {};
				FullScreenWindowed = false;
			}
			// Set fullscreen windowed
			else if (!hWnd && ExclusiveMode && Config.DdrawIntroVideoFix)
			{
				FullScreenWindowed = true;
			}
		}
		// Set fullscreen mode
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
			FullScreenWindowed = false;
		}

		// Check window handle
		if (IsWindow(hWnd) && DisplayMode.hWnd != hWnd &&
			(((!ExclusiveMode || Exclusive.hWnd == hWnd) && (!DisplayMode.hWnd || !DisplayMode.SetBy || DisplayMode.SetBy == this)) || !IsWindow(DisplayMode.hWnd)))
		{
			// Check if DC needs to be released
			if (DisplayMode.hWnd && DisplayMode.DC)
			{
				CloseD3DDDI();
				ReleaseDC(DisplayMode.hWnd, DisplayMode.DC);
				DisplayMode.DC = nullptr;
			}

			DisplayMode.hWnd = hWnd;

			if (DisplayMode.hWnd && !DisplayMode.DC)
			{
				DisplayMode.DC = ::GetDC(DisplayMode.hWnd);
			}
		}

		// Check window handle
		if (IsWindow(DisplayMode.hWnd) && ((!hWnd && Config.DdrawIntroVideoFix) || DisplayMode.hWnd == hWnd))
		{
			// Set exclusive mode resolution
			if (ExclusiveMode && DisplayMode.Width && DisplayMode.Height && DisplayMode.BPP)
			{
				Exclusive.Width = DisplayMode.Width;
				Exclusive.Height = DisplayMode.Height;
				Exclusive.BPP = DisplayMode.BPP;
				Exclusive.RefreshRate = DisplayMode.RefreshRate;
			}

			// Set windowed mode
			DisplayMode.SetBy = this;
			Device.IsWindowed = (!ExclusiveMode || FullScreenWindowed || Config.EnableWindowMode);

			// Check if just marking as non-exclusive
			bool MarkingUnexclusive = (hWnd && Exclusive.hWnd == hWnd && dwFlags == DDSCL_NORMAL);

			// Don't change flags or device if just marking as non-exclusive
			if (!MarkingUnexclusive)
			{
				// Set device flags
				Device.AllowModeX = !hWnd ? Device.AllowModeX :
					(((dwFlags & DDSCL_ALLOWMODEX) != 0) || (hWnd == LasthWnd ? Device.AllowModeX : 0));
				Device.MultiThreaded = !hWnd ? Device.MultiThreaded :
					(((dwFlags & DDSCL_MULTITHREADED) != 0) || (hWnd == LasthWnd ? Device.MultiThreaded : 0));
				// The flag (DDSCL_FPUPRESERVE) is assumed by default in DirectX 6 and earlier.
				Device.FPUPreserve = !hWnd ? Device.FPUPreserve :
					((((dwFlags & DDSCL_FPUPRESERVE) || DirectXVersion <= 6) && (dwFlags & DDSCL_FPUSETUP) == 0) || (hWnd == LasthWnd ? Device.FPUPreserve : 0));
				// The flag (DDSCL_NOWINDOWCHANGES) means DirectDraw is not allowed to minimize or restore the application window on activation.
				Device.NoWindowChanges = !hWnd ? Device.NoWindowChanges :
					(((dwFlags & DDSCL_NOWINDOWCHANGES) != 0) || (hWnd == LasthWnd ? Device.NoWindowChanges : 0));

				// Reset if mode was changed
				if ((dwFlags & (DDSCL_NORMAL | DDSCL_EXCLUSIVE)) &&
					(d3d9Device || !ExclusiveMode || (DisplayMode.Width && DisplayMode.Height)) &&	// Delay device creation when exclusive and no DisplayMode
					(LastWindowed != Device.IsWindowed || LasthWnd != DisplayMode.hWnd || LastFPUPreserve != Device.FPUPreserve))
				{
					CreateD9Device(__FUNCTION__);
				}
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
			SetWindowPos(hWnd, ((GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) ? HWND_TOPMOST : HWND_TOP), 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
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
			// Reset d3d9 device
			CreateD9Device(__FUNCTION__);
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

		D3DRASTER_STATUS RasterStatus = {};

		// Check flags
		switch (dwFlags)
		{
		case DDWAITVB_BLOCKBEGIN:
			// Use D3DKMTWaitForVerticalBlankEvent for vertical blank begin
			if (OpenD3DDDI(GetDC()) && D3DDDIWaitForVsync())
			{
				// Success using D3DKMTWaitForVerticalBlankEvent
				return DD_OK;
			}

			// Fallback: Wait for vertical blank begin using raster status
			while (SUCCEEDED(d3d9Device->GetRasterStatus(0, &RasterStatus)) && !RasterStatus.InVBlank)
			{
				Utils::BusyWaitYield(0);
			}
			return DD_OK;

		case DDWAITVB_BLOCKEND:
			// First, wait for the vertical blank to begin
			if (OpenD3DDDI(GetDC()) && D3DDDIWaitForVsync())
			{
				// Success using D3DKMTWaitForVerticalBlankEvent
			}
			// Fallback: Wait for vertical blank to end using raster status
			else
			{
				while (SUCCEEDED(d3d9Device->GetRasterStatus(0, &RasterStatus)) && !RasterStatus.InVBlank)
				{
					Utils::BusyWaitYield(0);
				}
			}

			// Then, wait for the vertical blank to end
			while (SUCCEEDED(d3d9Device->GetRasterStatus(0, &RasterStatus)) && RasterStatus.InVBlank)
			{
				Utils::BusyWaitYield(0);
			}
			return DD_OK;

		case DDWAITVB_BLOCKBEGINEVENT:
			// This value is unsupported
			Logging::Log() << __FUNCTION__ << " Error: DDWAITVB_BLOCKBEGINEVENT is not supported!";
			return DDERR_UNSUPPORTED;

		default:
			// Invalid parameter
			return DDERR_INVALIDPARAMS;
		}
	}

	// Call the original interface if not using D3D9
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

		// Get memory
		if (lpDDSCaps2 && ((lpDDSCaps2->dwCaps & (DDSCAPS_TEXTURE | DDSCAPS_OFFSCREENPLAIN | DDSCAPS_ZBUFFER)) ||	// Surface and Texture memory
			(lpDDSCaps2->dwCaps & (DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM | DDSCAPS_3DDEVICE))))					// Video memory
		{
			if (!OpenD3DDDI(GetDC()) || !D3DDDIGetVideoMemory(TotalMemory, AvailableMemory))
			{
				if (d3d9Device)
				{
					AvailableMemory = d3d9Device->GetAvailableTextureMem();
				}
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

		// Set memory values
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
			hr = ResetD9Device();
		}

		// Check device status
		if (hr == DD_OK || hr == DDERR_NOEXCLUSIVEMODE)
		{
			for (const auto& pDDraw : DDrawVector)
			{
				for (const auto& pSurface : pDDraw->SurfaceList)
				{
					pSurface.Interface->Restore();
				}
			}
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
			WndProc::SwitchingResolution = false;
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

void m_IDirectDrawX::InitInterface(DWORD DirectXVersion)
{
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
		LastWindowRect = {};

		// Release DC
		if (DisplayMode.hWnd && DisplayMode.DC)
		{
			ReleaseDC(DisplayMode.hWnd, DisplayMode.DC);
			DisplayMode.DC = nullptr;
		}
		ClipperHWnd = nullptr;

		// Display mode
		DisplayMode = {};

		// Exclusive mode
		ExclusiveMode = false;
		FullScreenWindowed = false;
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

		// Default gamma
		IsGammaSet = false;
		for (int i = 0; i < 256; ++i)
		{
			WORD value = static_cast<WORD>(i * 65535 / 255); // Linear interpolation from 0 to 65535
			RampData.red[i] = value;
			RampData.green[i] = value;
			RampData.blue[i] = value;
			DefaultRampData.red[i] = value;
			DefaultRampData.green[i] = value;
			DefaultRampData.blue[i] = value;
		}

		// High resolution counter
		Counter = {};
		QueryPerformanceFrequency(&Counter.Frequency);

#ifdef ENABLE_PROFILING
		presentTime = std::chrono::high_resolution_clock::now();
#endif
		// Direct3D9 flags
		EnableWaitVsync = false;

		// Direct3D9 Objects
		d3d9Object = nullptr;
		d3d9Device = nullptr;
		GammaLUTTexture = nullptr;
		ScreenCopyTexture = nullptr;
		palettePixelShader = nullptr;
		colorkeyPixelShader = nullptr;
		gammaPixelShader = nullptr;
		validateDeviceVertexBuffer = nullptr;

		presParams = {};
		UsingCustomRenderTarget = false;
		IsDeviceVerticesSet = false;
		BehaviorFlags = 0;
		hFocusWindow = nullptr;
		FocusWindowThreadID = 0;

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

void m_IDirectDrawX::ReleaseInterface()
{
	if (Config.Exiting)
	{
		return;
	}

	SetCriticalSection();
	SetPTCriticalSection();

	// Don't delete wrapper interface
	SaveInterfaceAddress(WrapperInterface, WrapperInterfaceBackup);
	SaveInterfaceAddress(WrapperInterface2, WrapperInterfaceBackup2);
	SaveInterfaceAddress(WrapperInterface3, WrapperInterfaceBackup3);
	SaveInterfaceAddress(WrapperInterface4, WrapperInterfaceBackup4);
	SaveInterfaceAddress(WrapperInterface7, WrapperInterfaceBackup7);

	if (g_hook)
	{
		UnhookWindowsHookEx(g_hook);
	}

	// Remove ddraw device
	DDrawVector.erase(std::remove(DDrawVector.begin(), DDrawVector.end(), this), DDrawVector.end());

	// Re-enable exclusive mode once non-exclusive device is released
	if (ExclusiveMode &&
		Device.IsWindowed && FullScreenWindowed &&
		DisplayMode.SetBy == this && DisplayMode.SetBy != Exclusive.SetBy &&
		std::find(DDrawVector.begin(), DDrawVector.end(), Exclusive.SetBy) != DDrawVector.end())
	{
		DisplayMode.SetBy = Exclusive.SetBy;
		Device.IsWindowed = false;
		FullScreenWindowed = false;

		CreateD9Device(__FUNCTION__);
	}

	// Clear SetBy handles
	if (DisplayMode.SetBy == this)
	{
		DisplayMode.SetBy = nullptr;
	}
	if (Exclusive.SetBy == this)
	{
		Exclusive.SetBy = nullptr;
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
		D3DInterface->DeleteMe();
		D3DInterface = nullptr;
	}

	// Release color control
	if (ColorControlInterface)
	{
		ColorControlInterface->ClearDdraw();
		ColorControlInterface = nullptr;
	}

	// Release gamma control
	if (GammaControlInterface)
	{
		GammaControlInterface->ClearDdraw();
		ColorControlInterface = nullptr;
	}

	// Release clippers
	for (const auto& pClipper : ClipperList)
	{
		pClipper.Interface->ClearDdraw();
	}
	ClipperList.clear();

	// Release palettes
	for (const auto& pPalette : PaletteList)
	{
		pPalette.Interface->ClearDdraw();
	}
	PaletteList.clear();

	// Release surfaces
	for (const auto& pSurface : SurfaceList)
	{
		pSurface.Interface->ReleaseD9Surface(false, false);
		pSurface.Interface->ClearDdraw();
	}
	SurfaceList.clear();

	// Delete released surfaces
	for (const auto& pSurface : ReleasedSurfaceList)
	{
		pSurface->DeleteMe();
	}
	ReleasedSurfaceList.clear();

	// Release vertex buffers
	for (const auto& pVertexBuffer : VertexBufferList)
	{
		pVertexBuffer->ReleaseD9Buffer(false, false);
		pVertexBuffer->ClearDdraw();
	}
	VertexBufferList.clear();

	ReleasePTCriticalSection();

	if (DDrawVector.empty())
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
			ReleaseD9Device();
		}

		// Release d3d9object
		if (d3d9Object)
		{
			ReleaseD9Object();
		}

		// Close DDI
		CloseD3DDDI();

		// Release DC
		if (IsWindow(DisplayMode.hWnd) && DisplayMode.DC)
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

		// Clean up dummy memory
		dummySurface.clear();
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
		if (!Config.DdrawWriteToGDI && !IsPrimaryRenderTarget())
		{
			if (Device.Width && Device.Height)
			{
				Utils::GetScreenSize(hWnd, (LONG&)Width, (LONG&)Height);
			}
			else
			{
				Width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
				Height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
			}
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

HRESULT m_IDirectDrawX::CheckInterface(char *FunctionName, bool CheckD3DDevice)
{
	// Check for object, if not then create it
	if (!d3d9Object)
	{
		// Create d3d9 object
		if (FAILED(CreateD9Object()))
		{
			LOG_LIMIT(100, FunctionName << " Error: d3d9 object not setup!");
			return DDERR_GENERIC;
		}
	}

	// Check for device, if not then create it
	if (CheckD3DDevice && !CheckD9Device(__FUNCTION__))
	{
		LOG_LIMIT(100, FunctionName << " Error: d3d9 device not setup!");
		return DDERR_GENERIC;
	}

	return DD_OK;
}

void m_IDirectDrawX::SetD3DDevice(m_IDirect3DDeviceX* lpD3DDevice)
{
	if (!lpD3DDevice)
	{
		return;
	}

	if (D3DDeviceInterface && D3DDeviceInterface != lpD3DDevice)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: Direct3D Device has already been created!");
	}

	D3DDeviceInterface = lpD3DDevice;
}

void m_IDirectDrawX::ClearD3DDevice(m_IDirect3DDeviceX* lpD3DDevice)
{
	if (D3DDeviceInterface != lpD3DDevice)
	{
		Logging::Log() << __FUNCTION__ << " Warning: released Direct3DDevice interface does not match cached one!";
	}

	Using3D = false;

	D3DDeviceInterface = nullptr;

	SetRenderTargetSurface(nullptr);

	Clear3DFlagForAllSurfaces();
}

bool m_IDirectDrawX::IsInScene()
{
	return (D3DDeviceInterface && D3DDeviceInterface->IsDeviceInScene());
}

bool m_IDirectDrawX::CheckD9Device(char* FunctionName)
{
	// Check for device, if not then create it
	if (!d3d9Device && FAILED(CreateD9Device(FunctionName)))
	{
		return false;
	}

	// Check for delay while resolution switching
	if (WndProc::SwitchingResolution)
	{
		for (int attempts = 0; attempts < 20; ++attempts)
		{
			if (d3d9Device->TestCooperativeLevel() != D3DERR_DEVICELOST)
			{
				break;
			}
			Sleep(500);
		}
	}
	WndProc::SwitchingResolution = false;

	return true;
}

LPDIRECT3D9 m_IDirectDrawX::GetDirectD9Object()
{
	return d3d9Object;
}

LPDIRECT3DDEVICE9* m_IDirectDrawX::GetDirectD9Device()
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

LPDIRECT3DPIXELSHADER9 m_IDirectDrawX::GetGammaPixelShader()
{
	// Create pixel shaders
	if (d3d9Device && !gammaPixelShader)
	{
		d3d9Device->CreatePixelShader((DWORD*)GammaPixelShaderSrc, &gammaPixelShader);
	}
	return gammaPixelShader;
}

LPDIRECT3DVERTEXBUFFER9 m_IDirectDrawX::GetValidateDeviceVertexBuffer(DWORD& FVF, DWORD& Size)
{
	// Create a simple vertex buffer
	struct SimpleVertex {
		float x, y, z;
		DWORD color;
	};

	SimpleVertex vertices[] = {
		{ -1.0f,  1.0f, 0.0f, 0xFFFFFFFF },  // Top-left
		{  1.0f,  1.0f, 0.0f, 0xFFFFFFFF },  // Top-right
		{  0.0f, -1.0f, 0.0f, 0xFFFFFFFF }   // Bottom-center
	};

	if (!validateDeviceVertexBuffer)
	{
		HRESULT hr = d3d9Device->CreateVertexBuffer(sizeof(vertices), 0, D3DFVF_XYZ | D3DFVF_DIFFUSE, D3DPOOL_MANAGED, &validateDeviceVertexBuffer, NULL);
		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to create vertex buffer: " << (DDERR)hr);
			return nullptr;
		}

		// Fill the vertex buffer with data
		void* pVertices = nullptr;
		hr = validateDeviceVertexBuffer->Lock(0, sizeof(vertices), &pVertices, 0);
		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to Lock vertex buffer: " << (DDERR)hr);
			validateDeviceVertexBuffer->Release();
			return nullptr;
		}
		memcpy(pVertices, vertices, sizeof(vertices));
		validateDeviceVertexBuffer->Unlock();
	}

	Size = sizeof(SimpleVertex);
	FVF = (D3DFVF_XYZ | D3DFVF_DIFFUSE);
	return validateDeviceVertexBuffer;
}

LPDIRECT3DINDEXBUFFER9 m_IDirectDrawX::GetIndexBuffer(LPWORD lpwIndices, DWORD dwIndexCount)
{
	if (!lpwIndices)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: nullptr Indices!");
		return nullptr;
	}

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true)))
	{
		return nullptr;
	}

	DWORD NewIndexSize = dwIndexCount * sizeof(WORD);

	HRESULT hr = D3D_OK;
	if (!d3d9IndexBuffer || NewIndexSize > IndexBufferSize)
	{
		ReleaseD3D9IndexBuffer();
		hr = d3d9Device->CreateIndexBuffer(NewIndexSize, D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_SYSTEMMEM, &d3d9IndexBuffer, nullptr);
	}

	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create index buffer: " << (D3DERR)hr << " Size: " << NewIndexSize);
		return nullptr;
	}

	if (NewIndexSize > IndexBufferSize)
	{
		IndexBufferSize = NewIndexSize;
	}

	void* pData = nullptr;
	hr = d3d9IndexBuffer->Lock(0, NewIndexSize, &pData, D3DLOCK_NOSYSLOCK);
	if (FAILED(hr))
	{
		hr = d3d9IndexBuffer->Lock(0, NewIndexSize, &pData, 0);
	}

	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock index buffer: " << (D3DERR)hr);
		return nullptr;
	}

	memcpy(pData, lpwIndices, NewIndexSize);

	d3d9IndexBuffer->Unlock();

	return d3d9IndexBuffer;
}

DWORD m_IDirectDrawX::GetHwndThreadID()
{
	return FocusWindowThreadID;
}

// Get AntiAliasing type and quality
D3DMULTISAMPLE_TYPE m_IDirectDrawX::GetMultiSampleTypeQuality(D3DFORMAT Format, DWORD MaxSampleType, DWORD& QualityLevels)
{
	if (d3d9Object)
	{
		for (int x = min(D3DMULTISAMPLE_16_SAMPLES, MaxSampleType); x > 0; x--)
		{
			D3DMULTISAMPLE_TYPE Samples = (D3DMULTISAMPLE_TYPE)x;

			if (SUCCEEDED(d3d9Object->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Format, presParams.Windowed, Samples, &QualityLevels)))
			{
				QualityLevels = (QualityLevels > 0) ? QualityLevels - 1 : 0;
				return Samples;
			}
		}
	}

	QualityLevels = 0;
	return D3DMULTISAMPLE_NONE;
}

// Resets the d3d9 device
HRESULT m_IDirectDrawX::ResetD9Device()
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
		WndProc::SwitchingResolution = false;
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

	// Reset device if current thread matches creation thread
	if (IsWindow(hFocusWindow) && FocusWindowThreadID == GetCurrentThreadId())
	{
		// Prepare for reset
		ReleaseAllD9Resources(true, true);

		// Reset device. When this method returns: BackBufferCount, BackBufferWidth, and BackBufferHeight are set to zero.
		D3DPRESENT_PARAMETERS newParams = presParams;
		hr = d3d9Device->Reset(&newParams);

		// If Reset fails then release the device and all resources
		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Reset failed: " << (D3DERR)hr);
			ReleaseAllD9Resources(false, false);	// Cannot backup surface after a failed Reset
			ReleaseD9Device();
			hr = CreateD9Device(__FUNCTION__);
		}
		else
		{
			// Clear render target flag
			UsingCustomRenderTarget = false;

			// Reset render target
			ReSetRenderTarget();

			// Reset D3D device settings
			RestoreD3DDeviceState();
		}
	}
	// Release and recreate device
	else
	{
		ReleaseAllD9Resources(true, false);
		ReleaseD9Device();
		hr = CreateD9Device(__FUNCTION__);
	}

	if (SUCCEEDED(hr))
	{
		WndProc::SwitchingResolution = false;
	}

	ReleasePTCriticalSection();
	ReleaseCriticalSection();

	// Return
	return hr;
}

// Creates or resets the d3d9 device
HRESULT m_IDirectDrawX::CreateD9Device(char* FunctionName)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, false)))
	{
		return DDERR_GENERIC;
	}

	// Get hwnd
	HWND hWnd = GetHwnd();

	// Hook WndProc before creating device
	WndProc::DATASTRUCT* WndDataStruct = WndProc::AddWndProc(hWnd);
	if (WndDataStruct) WndDataStruct->IsDirectDraw = true;

	// Check if creating from another thread
	FocusWindowThreadID = GetWindowThreadProcessId(hWnd, nullptr);
	if (WndDataStruct && FocusWindowThreadID != GetCurrentThreadId())
	{
		LOG_LIMIT(100, __FUNCTION__ << " " << FunctionName << " Warning: trying to create Direct3D9 device from a different thread than the hwnd was created from!");

		SendMessage(hWnd, WM_APP_CREATE_D3D9_DEVICE, (WPARAM)this, WM_MAKE_KEY(hWnd, this));

		return d3d9Device ? DD_OK : DDERR_GENERIC;
	}

	SetCriticalSection();
	SetPTCriticalSection();

	HRESULT hr = DD_OK;
	do {
		// Last call variables
		HWND LasthWnd = hFocusWindow;
		DWORD LastBehaviorFlags = BehaviorFlags;

		// Backup last present parameters
		D3DPRESENT_PARAMETERS presParamsBackup = presParams;

		// Store new focus window
		hFocusWindow = hWnd;

		// Get current resolution and rect
		DWORD CurrentWidth = 0, CurrentHeight = 0;
		Utils::GetScreenSize(hWnd, (LONG&)CurrentWidth, (LONG&)CurrentHeight);

		// Get current window size
		RECT LastClientRect = {};
		if (hWnd)
		{
			GetWindowRect(hWnd, &LastWindowRect);
			GetClientRect(hWnd, &LastClientRect);
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
			// Use default desktop resolution
			if (ExclusiveMode || Config.FullscreenWindowMode || Config.ForceExclusiveFullscreen)
			{
				BackBufferWidth = DefaultWidth;
				BackBufferHeight = DefaultHeight;
			}
			else
			{
				// Reset display to get proper screen size
				if (d3d9Device && !presParamsBackup.Windowed && !Config.EnableWindowMode)
				{
					ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, CDS_RESET, nullptr);
				}
				BackBufferWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
				BackBufferHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
			}
		}

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
			(Device.FPUPreserve ? D3DCREATE_FPU_PRESERVE : 0) |
			D3DCREATE_MULTITHREADED;

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

			Logging::Log() << __FUNCTION__ << " Recreate device! Last create: " << LasthWnd << "->" << hWnd << " " <<
				" Windowed: " << presParamsBackup.Windowed << "->" << presParams.Windowed << " " <<
				presParamsBackup.BackBufferWidth << "x" << presParamsBackup.BackBufferHeight << "->" <<
				presParams.BackBufferWidth << "x" << presParams.BackBufferHeight << " " <<
				Logging::hex(LastBehaviorFlags) << "->" << Logging::hex(BehaviorFlags);

			ReleaseAllD9Resources(true, false);
			ReleaseD9Device();

			// Reset display mode after release when display mode is already setup and there is a primary surface
			if (presParams.Windowed && (FullScreenWindowed || (PrimarySurface && DisplayMode.Width == CurrentWidth && DisplayMode.Height == CurrentHeight)))
			{
				Utils::SetDisplaySettings(hWnd, DisplayMode.Width, DisplayMode.Height);
			}
		}

		// Create d3d9 Device
		if (!d3d9Device)
		{
			// Prepare window and display size
			if ((!presParams.Windowed || FullScreenWindowed) && !Config.EnableWindowMode)
			{
				Utils::SetDisplaySettings(hWnd, presParams.BackBufferWidth, presParams.BackBufferHeight);

				SetWindowPos(hWnd, ((GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) ? HWND_TOPMOST : HWND_TOP),
					0, 0, presParams.BackBufferWidth, presParams.BackBufferHeight, SWP_NOZORDER | SWP_NOMOVE);
			}
			// Attempt to create a device
			hr = d3d9Object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, BehaviorFlags, &presParams, &d3d9Device);
			if (hr == D3DERR_INVALIDCALL && presParams.FullScreen_RefreshRateInHz)
			{
				presParams.FullScreen_RefreshRateInHz = 0;
				hr = d3d9Object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, BehaviorFlags, &presParams, &d3d9Device);
				if (SUCCEEDED(hr))
				{
					Device.RefreshRate = 0;
					DisplayMode.RefreshRate = 0;
					Exclusive.RefreshRate = 0;
				}
			}
		}

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create Direct3D9 device! " << (DDERR)hr << " " <<
				presParams.BackBufferWidth << "x" << presParams.BackBufferHeight << " refresh: " << presParams.FullScreen_RefreshRateInHz <<
				" format: " << presParams.BackBufferFormat << " wnd: " << hWnd << " params: " << presParams << " flags: " << Logging::hex(BehaviorFlags));
			break;
		}

		// Reset flags after creating device
		UsingCustomRenderTarget = false;
		WndProc::SwitchingResolution = false;
		LastUsedHWnd = hWnd;
		IsDeviceVerticesSet = false;
		EnableWaitVsync = false;
		FourCCsList.clear();

		// Create dummy memory (2x larger)
		dummySurface.resize(presParams.BackBufferWidth * presParams.BackBufferHeight * 4 * 2);

		// Set render target
		ReSetRenderTarget();

		// Reset D3D device settings
		RestoreD3DDeviceState();

		// Send window change messages for exclusive windows
		if (!Device.NoWindowChanges && !presParams.Windowed && !Config.EnableWindowMode)
		{
			// Get window size
			RECT NewWindowRect = {};
			GetWindowRect(hWnd, &NewWindowRect);
			RECT NewClientRect = {};
			GetClientRect(hWnd, &NewClientRect);

			// Check for window position change
			bool bWindowMove =
				LastWindowRect.left != NewWindowRect.left ||
				LastWindowRect.top != NewWindowRect.top;
			bool bWindowSize =
				(LONG)presParams.BackBufferWidth != NewClientRect.right ||
				(LONG)presParams.BackBufferHeight != NewClientRect.bottom ||
				LastClientRect.right != NewClientRect.right ||
				LastClientRect.bottom != NewClientRect.bottom;
			bool bIsNewWindow = LasthWnd != hFocusWindow;

			// Post window messages
			if (bWindowMove || bWindowSize)
			{
				// Window position variables
				HWND WindowInsert = GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST ? HWND_TOPMOST : HWND_TOP;
				static WINDOWPOS winpos;
				winpos = { hWnd, WindowInsert, NewWindowRect.left, NewWindowRect.top, NewWindowRect.right - NewWindowRect.left, NewWindowRect.bottom - NewWindowRect.top, WM_NULL };
				static NCCALCSIZE_PARAMS NCCalc;
				NCCalc = { { NewWindowRect, LastWindowRect, LastWindowRect }, &winpos };
				
				// Window placement
				WINDOWPLACEMENT wp = {};
				wp.length = sizeof(WINDOWPLACEMENT);
				GetWindowPlacement(hWnd, &wp);
				UINT SizeFlag = wp.showCmd == SW_SHOWMAXIMIZED ? SIZE_MAXIMIZED : wp.showCmd == SW_SHOWMINIMIZED ? SIZE_MINIMIZED : SIZE_RESTORED;

				// Notify window position
				PostMessage(hWnd, WM_WINDOWPOSCHANGING, 0, (LPARAM)&winpos);
				PostMessage(hWnd, WM_NCCALCSIZE, TRUE, (LPARAM)&NCCalc);
				PostMessage(hWnd, WM_NCPAINT, TRUE, NULL);
				PostMessage(hWnd, WM_ERASEBKGND, TRUE, NULL);
				PostMessage(hWnd, WM_WINDOWPOSCHANGED, 0, (LPARAM)&winpos);

				// Notify window move
				if (bWindowMove)
				{
					POINT ClientPoint = { NewClientRect.left, NewClientRect.top };
					MapWindowPoints(hWnd, GetParent(hWnd), (LPPOINT)&ClientPoint, 1);
					PostMessage(hWnd, WM_MOVE, 0, MAKELPARAM(ClientPoint.x, ClientPoint.y));
				}

				// Notify window size
				if (bWindowSize)
				{
					PostMessage(hWnd, WM_SIZE, SizeFlag, MAKELPARAM(NewClientRect.right, NewClientRect.bottom));
				}
			}

			// Window focus
			if (bIsNewWindow)
			{
				PostMessage(hWnd, WM_IME_SETCONTEXT, TRUE, ISC_SHOWUIALL);
				PostMessage(hWnd, WM_SETFOCUS, NULL, NULL);
				PostMessage(hWnd, WM_SYNCPAINT, (WPARAM)32, NULL);
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

void m_IDirectDrawX::UpdateVertices(DWORD Width, DWORD Height)
{
	// Get width and height
	DWORD displayWidth = 0, displayHeight = 0;
	if (Device.Width && Device.Height)
	{
		displayWidth = Device.Width;
		displayHeight = Device.Height;
	}
	else
	{
		displayWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		displayHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	}
	bool displayflag = Width && Height && (Width < displayWidth) && (Height < displayHeight);
	DWORD BackBufferWidth = (displayflag) ? displayWidth : Width;
	DWORD BackBufferHeight = (displayflag) ? displayHeight : Height;

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
	DeviceVertices[0].x = -0.5f + xpad;
	DeviceVertices[0].y = -0.5f + ypad;
	DeviceVertices[0].z = 0.0f;
	DeviceVertices[0].rhw = 1.0f;
	DeviceVertices[0].u = u0tex;
	DeviceVertices[0].v = v0tex;

	// scaledWidth, 0
	DeviceVertices[1].x = -0.5f + xpad + BackBufferWidth;
	DeviceVertices[1].y = DeviceVertices[0].y;
	DeviceVertices[1].z = 0.0f;
	DeviceVertices[1].rhw = 1.0f;
	DeviceVertices[1].u = u1tex;
	DeviceVertices[1].v = v0tex;

	// scaledWidth, scaledHeight
	DeviceVertices[2].x = DeviceVertices[1].x;
	DeviceVertices[2].y = -0.5f + ypad + BackBufferHeight;
	DeviceVertices[2].z = 0.0f;
	DeviceVertices[2].rhw = 1.0f;
	DeviceVertices[2].u = u1tex;
	DeviceVertices[2].v = v1tex;

	// 0, scaledHeight
	DeviceVertices[3].x = DeviceVertices[0].x;
	DeviceVertices[3].y = DeviceVertices[2].y;
	DeviceVertices[3].z = 0.0f;
	DeviceVertices[3].rhw = 1.0f;
	DeviceVertices[3].u = u0tex;
	DeviceVertices[3].v = v1tex;

	IsDeviceVerticesSet = true;
}

// Creates d3d9 object
HRESULT m_IDirectDrawX::CreateD9Object()
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

HRESULT m_IDirectDrawX::TestD3D9CooperativeLevel()
{
	if (d3d9Device)
	{
		return d3d9Device->TestCooperativeLevel();
	}

	return DD_OK;
}

// Set backbuffer to render target
void m_IDirectDrawX::ClearRenderTarget()
{
	if (d3d9Device && UsingCustomRenderTarget)
	{
		IDirect3DSurface9* pBackBuffer = nullptr;
		if (SUCCEEDED(d3d9Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
		{
			UsingCustomRenderTarget = false;

			d3d9Device->SetRenderState(D3DRS_ZENABLE, FALSE);
			d3d9Device->SetDepthStencilSurface(nullptr);
			d3d9Device->SetRenderTarget(0, pBackBuffer);

			pBackBuffer->Release();
		}
	}
}

void m_IDirectDrawX::ReSetRenderTarget()
{
	if (RenderTargetSurface && !UsingCustomRenderTarget)
	{
		SetRenderTargetSurface(RenderTargetSurface);
	}
}

void m_IDirectDrawX::SetCurrentRenderTarget()
{
	if (RenderTargetSurface)
	{
		SetRenderTargetSurface(RenderTargetSurface);
	}
}

HRESULT m_IDirectDrawX::SetRenderTargetSurface(m_IDirectDrawSurfaceX* lpSurface)
{
	SetCriticalSection();
	SetPTCriticalSection();

	HRESULT hr = D3D_OK;

	do {
		// Remove render target
		if (!lpSurface)
		{
			RenderTargetSurface = lpSurface;
			DepthStencilSurface = nullptr;

			ClearRenderTarget();

			hr = D3D_OK;
			break;
		}

		// Check for Direct3D surface
		if (!lpSurface->IsSurface3D())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not a Direct3D surface!");
			hr = DDERR_INVALIDPARAMS;
			break;
		}

		// Set surface as render target
		RenderTargetSurface = lpSurface;
		RenderTargetSurface->SetAsRenderTarget();

		Logging::LogDebug() << __FUNCTION__ << " Setting 3D Device Surface: " << RenderTargetSurface;

		// Set new render target
		if (d3d9Device)
		{
			LPDIRECT3DSURFACE9 pSurfaceD9 = RenderTargetSurface->GetD3d9Surface();
			if (pSurfaceD9)
			{
				UsingCustomRenderTarget = true;

				hr = d3d9Device->SetRenderTarget(0, pSurfaceD9);
			}
		}

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set render target: " << (D3DERR)hr);
			break;
		}

		m_IDirectDrawSurfaceX* pSurfaceZBuffer = RenderTargetSurface->GetAttachedDepthStencil();
		hr = SetDepthStencilSurface(pSurfaceZBuffer);

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set depth stencil surface: " << (D3DERR)hr);
			break;
		}
	} while (false);

	ReleasePTCriticalSection();
	ReleaseCriticalSection();

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
			d3d9Device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
			hr = d3d9Device->SetDepthStencilSurface(nullptr);
		}
	}
	else if (lpSurface->IsDepthStencil())
	{
		DepthStencilSurface = lpSurface;

		Logging::LogDebug() << __FUNCTION__ << " Setting Depth Stencil Surface: " << DepthStencilSurface;

		if (d3d9Device)
		{
			LPDIRECT3DSURFACE9 pSurfaceD9 = DepthStencilSurface->GetD3d9Surface();
			if (pSurfaceD9)
			{
				d3d9Device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
				hr = d3d9Device->SetDepthStencilSurface(pSurfaceD9);
			}
		}
	}

	return hr;
}

void m_IDirectDrawX::RestoreD3DDeviceState()
{
	for (const auto& pDDraw : DDrawVector)
	{
		m_IDirect3DDeviceX* Interface = pDDraw->D3DDeviceInterface;
		if (Interface)
		{
			Interface->AfterResetDevice();
		}
	}
}

void m_IDirectDrawX::Clear3DFlagForAllSurfaces()
{
	for (const auto& pDDraw : DDrawVector)
	{
		for (const auto& pSurface : pDDraw->SurfaceList)
		{
			pSurface.Interface->ClearUsing3DFlag();
		}
	}
}

inline void m_IDirectDrawX::ResetAllSurfaceDisplay()
{
	for (const auto& pDDraw : DDrawVector)
	{
		for (const auto& pSurface : pDDraw->SurfaceList)
		{
			pSurface.Interface->ResetSurfaceDisplay();
		}
	}
}

inline void m_IDirectDrawX::ReleaseD3D9IndexBuffer()
{
	// Release index buffer
	if (d3d9IndexBuffer)
	{
		ULONG ref = d3d9IndexBuffer->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " (" << this << ")" << " Error: there is still a reference to 'd3d9IndexBuffer' " << ref;
		}
		d3d9IndexBuffer = nullptr;
		IndexBufferSize = 0;
	}
}

// Release all dd9 resources
inline void m_IDirectDrawX::ReleaseAllD9Resources(bool BackupData, bool ResetInterface)
{
	SetCriticalSection();
	SetPTCriticalSection();

	// Remove render target and depth stencil surfaces
	if (d3d9Device && ResetInterface && (RenderTargetSurface || DepthStencilSurface))
	{
		SetRenderTargetSurface(nullptr);
	}

	// Release all surfaces from all ddraw devices
	for (const auto& pDDraw : DDrawVector)
	{
		for (const auto& pSurface : pDDraw->SurfaceList)
		{
			pSurface.Interface->ReleaseD9Surface(BackupData, ResetInterface);
		}
		for (const auto& pSurface : pDDraw->ReleasedSurfaceList)
		{
			pSurface->DeleteMe();
		}
		pDDraw->ReleasedSurfaceList.clear();
	}

	// Release all state blocks from all ddraw devices
	for (const auto& pDDraw : DDrawVector)
	{
		m_IDirect3DDeviceX* Interface = pDDraw->D3DDeviceInterface;
		if (Interface)
		{
			Interface->BeforeResetDevice();
			if (!ResetInterface)
			{
				Interface->ReleaseAllStateBlocks();
			}
		}
	}

	// Release all vertex buffers from all ddraw devices
	for (const auto& pDDraw : DDrawVector)
	{
		for (const auto& pBuffer : pDDraw->VertexBufferList)
		{
			pBuffer->ReleaseD9Buffer(BackupData, ResetInterface);
		}
	}

	// Release gamma texture
	if (GammaLUTTexture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 gamma texture";
		ULONG ref = GammaLUTTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'GammaLUTTexture' " << ref;
		}
		GammaLUTTexture = nullptr;
	}

	// Release gamma screen copy texture
	if (ScreenCopyTexture)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 gamma screen copy texture";
		ULONG ref = ScreenCopyTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'ScreenCopyTexture' " << ref;
		}
		ScreenCopyTexture = nullptr;
	}

	// Release validate device d3d9 vertex buffer
	if (validateDeviceVertexBuffer)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 validate device vertext buffer";
		ULONG ref = validateDeviceVertexBuffer->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'validateDeviceVertexBuffer' " << ref;
		}
		validateDeviceVertexBuffer = nullptr;
	}

	// Release index buffer
	if (!ResetInterface)
	{
		ReleaseD3D9IndexBuffer();
	}

	// Release palette pixel shader
	if (palettePixelShader)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 palette pixel shader";
		if (d3d9Device && ResetInterface)
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
		if (d3d9Device && ResetInterface)
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

	// Release gamma pixel shader
	if (gammaPixelShader)
	{
		Logging::LogDebug() << __FUNCTION__ << " Releasing Direct3D9 gamma pixel shader";
		ULONG ref = gammaPixelShader->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'gammaPixelShader' " << ref;
		}
		gammaPixelShader = nullptr;
	}

	ReleasePTCriticalSection();
	ReleaseCriticalSection();
}

// Release d3d9 device
void m_IDirectDrawX::ReleaseD9Device()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetCriticalSection();
	SetPTCriticalSection();

	if (d3d9Device)
	{
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
void m_IDirectDrawX::ReleaseD9Object()
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

HRESULT m_IDirectDrawX::CreateColorControl(m_IDirectDrawColorControl** lplpColorControl)
{
	if (lplpColorControl)
	{
		*lplpColorControl = CreateDirectDrawColorControl(nullptr, this);

		return DD_OK;
	}
	return DDERR_GENERIC;
}

void m_IDirectDrawX::SetColorControl(m_IDirectDrawColorControl* lpColorControl)
{
	if (!lpColorControl)
	{
		return;
	}

	if (ColorControlInterface && ColorControlInterface != lpColorControl)
	{
		Logging::Log() << __FUNCTION__ << " Warning: ColorControl interface has already been created!";
	}

	ColorControlInterface = lpColorControl;
}

void m_IDirectDrawX::ClearColorControl(m_IDirectDrawColorControl* lpColorControl)
{
	if (lpColorControl != ColorControlInterface)
	{
		Logging::Log() << __FUNCTION__ << " Warning: released ColorControl interface does not match cached one!";
	}

	ColorControlInterface = nullptr;
}

HRESULT m_IDirectDrawX::CreateGammaControl(m_IDirectDrawGammaControl** lplpGammaControl)
{
	if (lplpGammaControl)
	{
		*lplpGammaControl = CreateDirectDrawGammaControl(nullptr, this);

		return DD_OK;
	}
	return DDERR_GENERIC;
}

void m_IDirectDrawX::SetGammaControl(m_IDirectDrawGammaControl* lpGammaControl)
{
	if (!lpGammaControl)
	{
		return;
	}

	if (GammaControlInterface && GammaControlInterface != lpGammaControl)
	{
		Logging::Log() << __FUNCTION__ << " Warning: GammaControl interface has already been created!";
	}

	GammaControlInterface = lpGammaControl;
}

void m_IDirectDrawX::ClearGammaControl(m_IDirectDrawGammaControl* lpGammaControl)
{
	if (lpGammaControl != GammaControlInterface)
	{
		Logging::Log() << __FUNCTION__ << " Warning: released GammaControl interface does not match cached one!";
	}

	GammaControlInterface = nullptr;
}

// Add released surface wrapper
void m_IDirectDrawX::AddReleasedSurface(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return;
	}

	ReleasedSurfaceList.push_back(lpSurfaceX);
}

// Check if surface wrapper exists
bool m_IDirectDrawX::DoesSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return false;
	}

	const bool found = std::find_if(SurfaceList.begin(), SurfaceList.end(),
		[lpSurfaceX](auto entry) {
			return entry.Interface == lpSurfaceX;
		}) != std::end(SurfaceList);

	return found;
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

	const bool found = std::find_if(ClipperList.begin(), ClipperList.end(),
		[lpClipper](auto entry) {
			return entry.Interface == lpClipper;
		}) != std::end(ClipperList);

	return found;
}

// Check if palette wrapper exists
bool m_IDirectDrawX::DoesPaletteExist(m_IDirectDrawPalette* lpPalette)
{
	if (!lpPalette)
	{
		return false;
	}

	const bool found = std::find_if(PaletteList.begin(), PaletteList.end(),
		[lpPalette](auto entry) {
			return entry.Interface == lpPalette;
		}) != std::end(PaletteList);

	return found;
}

void m_IDirectDrawX::AddVertexBuffer(m_IDirect3DVertexBufferX* lpVertexBuffer)
{
	if (!lpVertexBuffer)
	{
		return;
	}

	VertexBufferList.push_back(lpVertexBuffer);
}

void m_IDirectDrawX::ClearVertexBuffer(m_IDirect3DVertexBufferX* lpVertexBuffer)
{
	// Remove palette from attached surface
	for (const auto& pDDraw : DDrawVector)
	{
		auto it = std::find(pDDraw->VertexBufferList.begin(), pDDraw->VertexBufferList.end(), lpVertexBuffer);

		// Remove vertex buffer
		if (it != std::end(pDDraw->VertexBufferList))
		{
			lpVertexBuffer->ClearDdraw();
			pDDraw->VertexBufferList.erase(it);
		}
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
			return CreateD9Device(__FUNCTION__);
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

	if (!lpRampData)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (dwFlags)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: gamma flags not supported! " << Logging::hex(dwFlags));
	}

	memcpy(lpRampData, &RampData, sizeof(D3DGAMMARAMP));

	return DD_OK;
}

HRESULT m_IDirectDrawX::SetBrightnessLevel(D3DGAMMARAMP& Ramp)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!d3d9Device)
	{
		return DD_OK;
	}

	// Create or update the gamma LUT texture
	if (!GammaLUTTexture)
	{
		if (SUCCEEDED(d3d9Device->CreateTexture(256, 1, 1, 0, D3DFMT_A32B32G32R32F, D3DPOOL_MANAGED, &GammaLUTTexture, nullptr)))
		{
			UsingShader32f = true;
		}
		else
		{
			d3d9Device->CreateTexture(256, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &GammaLUTTexture, nullptr);
			UsingShader32f = false;
		}
	}

	D3DLOCKED_RECT lockedRect;
	if (SUCCEEDED(GammaLUTTexture->LockRect(0, &lockedRect, nullptr, D3DLOCK_DISCARD)))
	{
		if (UsingShader32f)
		{
			float* texData = static_cast<float*>(lockedRect.pBits);
			for (int i = 0; i < 256; ++i)
			{
				texData[i * 4 + 0] = Ramp.red[i] / 65535.0f;
				texData[i * 4 + 1] = Ramp.green[i] / 65535.0f;
				texData[i * 4 + 2] = Ramp.blue[i] / 65535.0f;
				texData[i * 4 + 3] = 1.0f;
			}
		}
		else
		{
			DWORD* texData = static_cast<DWORD*>(lockedRect.pBits);
			for (int i = 0; i < 256; ++i)
			{
				BYTE r = static_cast<BYTE>(Ramp.red[i] / 256);
				BYTE g = static_cast<BYTE>(Ramp.green[i] / 256);
				BYTE b = static_cast<BYTE>(Ramp.blue[i] / 256);

				texData[i] = D3DCOLOR_ARGB(255, r, g, b);
			}
		}
		GammaLUTTexture->UnlockRect(0);
	}

	return DD_OK;
}

HRESULT m_IDirectDrawX::SetD9Gamma(DWORD dwFlags, LPDDGAMMARAMP lpRampData)
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true)))
	{
		return DDERR_GENERIC;
	}

	if (!lpRampData)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (dwFlags)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: gamma flags not supported! " << Logging::hex(dwFlags));
	}

	IsGammaSet = false;
	memcpy(&RampData, lpRampData, sizeof(D3DGAMMARAMP));

	if (memcmp(&DefaultRampData, &RampData, sizeof(D3DGAMMARAMP)) != S_OK)
	{
		IsGammaSet = true;
		SetBrightnessLevel(RampData);
	}

	return DD_OK;
}

static void BackupAndResetState(LPDIRECT3DDEVICE9 ProxyInterface, DRAWSTATEBACKUP& DrawStates, DWORD Width, DWORD Height)
{
	// Sampler states
	ProxyInterface->GetSamplerState(0, D3DSAMP_MAGFILTER, &DrawStates.ssMagFilter);
	ProxyInterface->GetSamplerState(1, D3DSAMP_ADDRESSU, &DrawStates.ss1addressU);
	ProxyInterface->GetSamplerState(1, D3DSAMP_ADDRESSV, &DrawStates.ss1addressV);
	ProxyInterface->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	ProxyInterface->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	ProxyInterface->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// Texture states
	ProxyInterface->GetTextureStageState(0, D3DTSS_COLOROP, &DrawStates.tsColorOP);
	ProxyInterface->GetTextureStageState(0, D3DTSS_COLORARG1, &DrawStates.tsColorArg1);
	ProxyInterface->GetTextureStageState(0, D3DTSS_COLORARG2, &DrawStates.tsColorArg2);
	ProxyInterface->GetTextureStageState(0, D3DTSS_ALPHAOP, &DrawStates.tsAlphaOP);
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	ProxyInterface->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// Render states
	ProxyInterface->GetRenderState(D3DRS_LIGHTING, &DrawStates.rsLighting);
	ProxyInterface->GetRenderState(D3DRS_ALPHATESTENABLE, &DrawStates.rsAlphaTestEnable);
	ProxyInterface->GetRenderState(D3DRS_ALPHABLENDENABLE, &DrawStates.rsAlphaBlendEnable);
	ProxyInterface->GetRenderState(D3DRS_FOGENABLE, &DrawStates.rsFogEnable);
	ProxyInterface->GetRenderState(D3DRS_ZENABLE, &DrawStates.rsZEnable);
	ProxyInterface->GetRenderState(D3DRS_ZWRITEENABLE, &DrawStates.rsZWriteEnable);
	ProxyInterface->GetRenderState(D3DRS_STENCILENABLE, &DrawStates.rsStencilEnable);
	ProxyInterface->GetRenderState(D3DRS_CULLMODE, &DrawStates.rsCullMode);
	ProxyInterface->GetRenderState(D3DRS_CLIPPING, &DrawStates.rsClipping);
	ProxyInterface->SetRenderState(D3DRS_LIGHTING, FALSE);
	ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_FOGENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	ProxyInterface->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	ProxyInterface->SetRenderState(D3DRS_CLIPPING, FALSE);

	// Viewport
	ProxyInterface->GetViewport(&DrawStates.ViewPort);
	D3DVIEWPORT9 ViewPort = { 0, 0, presParams.BackBufferWidth, presParams.BackBufferHeight, 0.0f, 1.0f };

	// Calculate width and height with original aspect ratio
	DWORD DisplayBufferWidth = presParams.BackBufferWidth;
	DWORD DisplayBufferHeight = presParams.BackBufferHeight;
	DWORD TexWidth = Width;
	DWORD TexHeight = Height;
	if (Config.DdrawIntegerScalingClamp)
	{
		DWORD xScaleRatio = DisplayBufferWidth / TexWidth;
		DWORD yScaleRatio = DisplayBufferHeight / TexHeight;

		if (Config.DdrawMaintainAspectRatio)
		{
			xScaleRatio = min(xScaleRatio, yScaleRatio);
			yScaleRatio = min(xScaleRatio, yScaleRatio);
		}

		ViewPort.Width = xScaleRatio * TexWidth;
		ViewPort.Height = yScaleRatio * TexHeight;

		ViewPort.X = (DisplayBufferWidth - ViewPort.Width) / 2;
		ViewPort.Y = (DisplayBufferHeight - ViewPort.Height) / 2;
	}
	else if (Config.DdrawMaintainAspectRatio)
	{
		if (TexWidth * DisplayBufferHeight < TexHeight * DisplayBufferWidth)
		{
			// 4:3 displayed on 16:9
			ViewPort.Width = DisplayBufferHeight * TexWidth / TexHeight;
		}
		else
		{
			// 16:9 displayed on 4:3
			ViewPort.Height = DisplayBufferWidth * TexHeight / TexWidth;
		}
		ViewPort.X = (DisplayBufferWidth - ViewPort.Width) / 2;
		ViewPort.Y = (DisplayBufferHeight - ViewPort.Height) / 2;
	}

	// Set the viewport with the calculated values
	ProxyInterface->SetViewport(&ViewPort);

	// Trasform
	ProxyInterface->GetTransform(D3DTS_WORLD, &DrawStates.WorldMatrix);
	ProxyInterface->GetTransform(D3DTS_VIEW, &DrawStates.ViewMatrix);
	ProxyInterface->GetTransform(D3DTS_PROJECTION, &DrawStates.ProjectionMatrix);
	D3DMATRIX identityMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	ProxyInterface->SetTransform(D3DTS_WORLD, &identityMatrix);
	ProxyInterface->SetTransform(D3DTS_VIEW, &identityMatrix);
	ProxyInterface->SetTransform(D3DTS_PROJECTION, &identityMatrix);
}

static void RestoreState(LPDIRECT3DDEVICE9 ProxyInterface, DRAWSTATEBACKUP& DrawStates)
{
	// Restore sampler states
	ProxyInterface->SetSamplerState(0, D3DSAMP_MAGFILTER, DrawStates.ssMagFilter);
	ProxyInterface->SetSamplerState(1, D3DSAMP_ADDRESSU, DrawStates.ss1addressU);
	ProxyInterface->SetSamplerState(1, D3DSAMP_ADDRESSV, DrawStates.ss1addressV);

	// Restore texture states
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLOROP, DrawStates.tsColorOP);
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG1, DrawStates.tsColorArg1);
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG2, DrawStates.tsColorArg2);
	ProxyInterface->SetTextureStageState(0, D3DTSS_ALPHAOP, DrawStates.tsAlphaOP);

	// Restore render states
	ProxyInterface->SetRenderState(D3DRS_LIGHTING, DrawStates.rsLighting);
	ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, DrawStates.rsAlphaTestEnable);
	ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, DrawStates.rsAlphaBlendEnable);
	ProxyInterface->SetRenderState(D3DRS_FOGENABLE, DrawStates.rsFogEnable);
	ProxyInterface->SetRenderState(D3DRS_ZENABLE, DrawStates.rsZEnable);
	ProxyInterface->SetRenderState(D3DRS_ZWRITEENABLE, DrawStates.rsZWriteEnable);
	ProxyInterface->SetRenderState(D3DRS_STENCILENABLE, DrawStates.rsStencilEnable);
	ProxyInterface->SetRenderState(D3DRS_CULLMODE, DrawStates.rsCullMode);
	ProxyInterface->SetRenderState(D3DRS_CLIPPING, DrawStates.rsClipping);

	// Reset viewport
	ProxyInterface->SetViewport(&DrawStates.ViewPort);

	// Reset trasform
	ProxyInterface->SetTransform(D3DTS_WORLD, &DrawStates.WorldMatrix);
	ProxyInterface->SetTransform(D3DTS_VIEW, &DrawStates.ViewMatrix);
	ProxyInterface->SetTransform(D3DTS_PROJECTION, &DrawStates.ProjectionMatrix);
}

HRESULT m_IDirectDrawX::DrawPrimarySurface(LPDIRECT3DTEXTURE9 pDisplayTexture)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	bool IsUsingPalette = false;
	if (!pDisplayTexture)
	{
		if (!PrimarySurface)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no primary surface!");
			return DDERR_GENERIC;
		}

		// Get surface texture
		pDisplayTexture = PrimarySurface->GetD3d9Texture();
		if (!pDisplayTexture)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get surface texture!");
			return DDERR_GENERIC;
		}

		// Check palette
		IsUsingPalette = PrimarySurface->IsPalette();
	}

	// Get texture desc
	D3DSURFACE_DESC Desc = {};
	pDisplayTexture->GetLevelDesc(0, &Desc);

	// Backup current states
	DRAWSTATEBACKUP DrawStates;
	BackupAndResetState(d3d9Device, DrawStates, Desc.Width, Desc.Height);

	// Set texture
	d3d9Device->SetTexture(0, pDisplayTexture);

	// For palette surfaces
	if (IsUsingPalette)
	{
		// Get palette texture
		LPDIRECT3DTEXTURE9 PaletteTexture = PrimarySurface->GetD3d9PaletteTexture();

		// Set palette texture
		if (PaletteTexture && CreatePaletteShader())
		{
			// Set palette texture
			PrimarySurface->UpdatePaletteData();
			d3d9Device->SetTexture(1, PaletteTexture);

			// Set pixel shader
			d3d9Device->SetPixelShader(palettePixelShader);
		}
	}
	// For gamma
	else if (IsGammaSet && GammaControlInterface)
	{
		// Create gamma texture
		if (!GammaLUTTexture)
		{
			SetBrightnessLevel(RampData);
		}

		// Set gamma texture
		d3d9Device->SetTexture(1, GammaLUTTexture);

		// Set pixel shader
		d3d9Device->SetPixelShader(GetGammaPixelShader());
	}

	// Update vertices
	if (!IsDeviceVerticesSet)
	{
		UpdateVertices(Desc.Width, Desc.Height);
	}

	// Set vertex format
	d3d9Device->SetFVF(TLVERTEXFVF);

	// Set backbuffer to render target
	ClearRenderTarget();

	// Draw primitive
	HRESULT hr = d3d9Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, DeviceVertices, sizeof(TLVERTEX));

	// Reset redner target if needed
	ReSetRenderTarget();

	// Reset dirty flags
	if (SUCCEEDED(hr))
	{
		PrimarySurface->ClearDirtyFlags();
	}
	else
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to draw primitive");
	}

	// Reset textures
	d3d9Device->SetTexture(0, nullptr);
	d3d9Device->SetTexture(1, nullptr);

	// Reset pixel shader
	d3d9Device->SetPixelShader(nullptr);

	// Restore states
	RestoreState(d3d9Device, DrawStates);

	return hr;
}

HRESULT m_IDirectDrawX::CopyPrimarySurface(LPDIRECT3DSURFACE9 pDestBuffer)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!PrimarySurface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: no primary surface!");
		return DDERR_GENERIC;
	}

	// Get backbuffer render target
	IDirect3DSurface9* pRenderTarget = PrimarySurface->GetD3d9Surface();
	if (!pRenderTarget)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: no render target on primary surface!");
		return DDERR_GENERIC;
	}

	// Copy render target to backbuffer
	IDirect3DSurface9* pBackBuffer = nullptr;
	if (!pDestBuffer)
	{
		if (SUCCEEDED(d3d9Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
		{
			pDestBuffer = pBackBuffer;
		}
	}

	// Get window handle
	HWND hWnd = GetHwnd();

	// Get source rect
	RECT* pSrcRect = nullptr;
	RECT SrcRect = { 0, 0, (LONG)PrimarySurface->GetD3d9Width(), (LONG)PrimarySurface->GetD3d9Height() };
	if (!ExclusiveMode && IsWindow(hWnd) && !IsIconic(hWnd))
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
				pSrcRect = &SrcRect;
			}
		}
	}

	// Get destination rect
	RECT* pDestRect = nullptr;
	RECT DestRect;
	if (pSrcRect)
	{
		DestRect = { 0, 0, SrcRect.right - SrcRect.left, SrcRect.bottom - SrcRect.top };
		pDestRect = &DestRect;
	}

	// Copy render target
	HRESULT hr = d3d9Device->StretchRect(pRenderTarget, pSrcRect, pDestBuffer, pDestRect, D3DTEXF_NONE);
	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to copy primary render target!");
	}

	// If copying to back buffer
	if (pBackBuffer)
	{
		pBackBuffer->Release();

		if (SUCCEEDED(hr))
		{
			PrimarySurface->ClearDirtyFlags();
		}
	}

	return hr;
}

HRESULT m_IDirectDrawX::PresentScene(RECT* pRect)
{
	HRESULT hr = DDERR_GENERIC;

	if (IsUsingThreadPresent())
	{
		return DD_OK;
	}

	if (!PrimarySurface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: no primary surface!");
		return DDERR_GENERIC;
	}

	if (IsInScene())
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: is in Direct3D scene already! PresentToWindow: " << PrimarySurface->ShouldPresentToWindow(true));
		return DDERR_GENERIC;
	}

	// Prepare primary surface render target before presenting
	PrimarySurface->PrepareRenderTarget();

	LPRECT pDestRect = nullptr;
	RECT DestRect = {};
	if (PrimarySurface->ShouldPresentToWindow(true))
	{
		if (FAILED(PrimarySurface->GetPresentWindowRect(pRect, DestRect)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get present rect!");
			return DDERR_GENERIC;
		}
		pDestRect = &DestRect;
	}

	// Begin scene
	d3d9Device->BeginScene();

	// Copy or draw primary surface before presenting
	if (IsPrimaryRenderTarget() && !PrimarySurface->GetD3d9Texture())
	{
		if (IsGammaSet && GammaControlInterface)
		{
			IDirect3DSurface9* pBackBuffer = nullptr;
			if (SUCCEEDED(d3d9Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
			{
				// Create intermediate texture for shader input
				if (!ScreenCopyTexture)
				{
					D3DSURFACE_DESC Desc;
					pBackBuffer->GetDesc(&Desc);
					if (FAILED(d3d9Device->CreateTexture(Desc.Width, Desc.Height, 1, D3DUSAGE_RENDERTARGET, Desc.Format, D3DPOOL_DEFAULT, &ScreenCopyTexture, nullptr)))
					{
						Logging::Log() << __FUNCTION__ << " Error: Failed to create screen copy texture!";
						pBackBuffer->Release();
					}
				}

				// Copy back buffer to texture surface and draw surface to screen
				if (ScreenCopyTexture)
				{
					IDirect3DSurface9* pCopySurface = nullptr;
					if (SUCCEEDED(ScreenCopyTexture->GetSurfaceLevel(0, &pCopySurface)))
					{
						hr = CopyPrimarySurface(pCopySurface);
						pCopySurface->Release();
					}

					// Release back buffer
					pBackBuffer->Release();

					// Draw surface
					hr = DrawPrimarySurface(ScreenCopyTexture);
				}
			}
		}
		else
		{
			hr = CopyPrimarySurface(nullptr);
		}
	}
	else
	{
		hr = DrawPrimarySurface(nullptr);
	}
	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to draw primary surface!");
	}

	// End scene
	d3d9Device->EndScene();

	// Present to d3d9
	if (SUCCEEDED(hr))
	{
		hr = Present(pDestRect, pDestRect);
	}

	return hr;
}

bool m_IDirectDrawX::IsUsingThreadPresent()
{
	return (PresentThread.IsInitialized && ExclusiveMode && !RenderTargetSurface && !IsPrimaryRenderTarget());
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
			for (const auto& instance : DDrawVector)
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

				// Draw surface before presenting
				pDDraw->DrawPrimarySurface(nullptr);

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
HRESULT m_IDirectDrawX::Present(RECT* pSourceRect, RECT* pDestRect)
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
	if (EnableWaitVsync && !Config.EnableVSync && !Config.ForceVsyncMode)
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
	}
	EnableWaitVsync = false;

	// Present everthing, skip Preset when using DdrawWriteToGDI
	HRESULT hr = D3DERR_DEVICELOST;
	if (!IsUsingThreadPresent())
	{
		hr = d3d9Device->Present(pSourceRect, pDestRect, nullptr, nullptr);
	}
	
#ifdef ENABLE_PROFILING
	Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(presentTime);
	presentTime = std::chrono::high_resolution_clock::now();
#endif

	// Test cooperative level
	if (hr == D3DERR_DEVICELOST)
	{
		hr = TestD3D9CooperativeLevel();
		hr = (hr == DDERR_NOEXCLUSIVEMODE) ? DD_OK : hr;
	}

	// Reset device
	if (hr == D3DERR_DEVICENOTRESET)
	{
		hr = ResetD9Device();
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
	if (presParams.Windowed && !ExclusiveMode && !IsIconic(hWnd) && GetWindowRect(hWnd, &ClientRect) && LastWindowRect.right > 0 && LastWindowRect.bottom > 0)
	{
		if (ClientRect.left != LastWindowRect.left || ClientRect.top != LastWindowRect.top ||
			ClientRect.right != LastWindowRect.right || ClientRect.bottom != LastWindowRect.bottom)
		{
			RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
	}
	LastWindowRect = ClientRect;

	// Store new click time after frame draw is complete
	QueryPerformanceCounter(&Counter.LastPresentTime);

	return DD_OK;
}

bool CheckDirectDrawXInterface(void* pInterface)
{
	for (auto& entry : DDrawVector)
	{
		if (entry == pInterface)
		{
			return true;
		}
	}
	return false;
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
