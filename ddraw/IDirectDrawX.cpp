/**
* Copyright (C) 2019 Elisha Riedlinger
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
#include "Utils\Utils.h"

// ddraw interface counter
DWORD ddrawRefCount = 0;
bool ShareD3d9DeviceFlag = false;

// Convert to Direct3D9
HWND MainhWnd;
bool IsInScene;
bool ExclusiveMode;
bool AllowModeX;
bool MultiThreaded;
bool FUPPreserve;
bool NoWindowChanges;
bool isWindowed;					// Window mode enabled

// Application display mode
DWORD displayModeWidth;
DWORD displayModeHeight;
DWORD displayModeBPP;
DWORD displayModeRefreshRate;		// Refresh rate for fullscreen

// Display resolution
bool SetDefaultDisplayMode;			// Set native resolution
DWORD displayWidth;
DWORD displayHeight;
DWORD displayRefreshRate;			// Refresh rate for fullscreen

// High resolution counter
bool FrequencyFlag = false;
LARGE_INTEGER clockFrequency, clickTime, lastPresentTime = { 0, 0 };
LONGLONG lastFrameTime = 0;
DWORD FrameCounter = 0;
DWORD monitorRefreshRate = 0;
DWORD monitorHeight = 0;

// Direct3D9 Objects
LPDIRECT3D9 d3d9Object = nullptr;
LPDIRECT3DDEVICE9 d3d9Device = nullptr;
D3DPRESENT_PARAMETERS presParams;

struct handle_data
{
	DWORD process_id = 0;
	HWND best_handle = nullptr;
};

std::unordered_map<HWND, m_IDirectDraw7*> g_hookmap;

/************************/
/*** IUnknown methods ***/
/************************/

HRESULT m_IDirectDrawX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if ((riid == IID_IDirectDraw || riid == IID_IDirectDraw2 || riid == IID_IDirectDraw3 || riid == IID_IDirectDraw4 || riid == IID_IDirectDraw7 || riid == IID_IUnknown) && ppvObj)
		{
			DWORD DxVersion = (riid == IID_IUnknown) ? DirectXVersion : GetIIDVersion(riid);

			*ppvObj = GetWrapperInterfaceX(DxVersion);

			::AddRef(*ppvObj);

			return DD_OK;
		}
		if ((riid == IID_IDirect3D || riid == IID_IDirect3D2 || riid == IID_IDirect3D3 || riid == IID_IDirect3D7) && ppvObj)
		{
			DWORD DxVersion = GetIIDVersion(riid);

			SetCriticalSection();
			if (D3DInterface)
			{
				*ppvObj = D3DInterface->GetWrapperInterfaceX(DxVersion);

				::AddRef(*ppvObj);
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
	}

	HRESULT hr = ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DirectXVersion), WrapperInterface);

	if (SUCCEEDED(hr) && Config.ConvertToDirect3D7 && ppvObj)
	{
		if (riid == IID_IDirect3D || riid == IID_IDirect3D2 || riid == IID_IDirect3D3 || riid == IID_IDirect3D7)
		{
			m_IDirect3DX *lpD3DirectX = ((m_IDirect3D7*)*ppvObj)->GetWrapperInterface();

			if (lpD3DirectX)
			{
				lpD3DirectX->SetDdrawParent(this);
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
		if (!UniqueProxyInterface.get())
		{
			UniqueProxyInterface = std::make_unique<m_IDirectDraw>(this);
		}
		return UniqueProxyInterface.get();
	case 2:
		if (!UniqueProxyInterface2.get())
		{
			UniqueProxyInterface2 = std::make_unique<m_IDirectDraw2>(this);
		}
		return UniqueProxyInterface2.get();
	case 3:
		if (!UniqueProxyInterface3.get())
		{
			UniqueProxyInterface3 = std::make_unique<m_IDirectDraw3>(this);
		}
		return UniqueProxyInterface3.get();
	case 4:
		if (!UniqueProxyInterface4.get())
		{
			UniqueProxyInterface4 = std::make_unique<m_IDirectDraw4>(this);
		}
		return UniqueProxyInterface4.get();
	case 7:
		if (!UniqueProxyInterface7.get())
		{
			UniqueProxyInterface7 = std::make_unique<m_IDirectDraw7>(this);
		}
		return UniqueProxyInterface7.get();
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return nullptr;
	}
}

ULONG m_IDirectDrawX::AddRef()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawX::Release()
{
	Logging::LogDebug() << __FUNCTION__;

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

/***************************/
/*** IDirectDraw methods ***/
/***************************/

HRESULT m_IDirectDrawX::Compact()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// This method is not currently implemented even in ddraw.
		return DD_OK;
	}

	return ProxyInterface->Compact();
}

HRESULT m_IDirectDrawX::CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR * lplpDDClipper, IUnknown FAR * pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__;

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
		*lplpDDClipper = ProxyAddressLookupTable.FindAddress<m_IDirectDrawClipper>(*lplpDDClipper);
	}

	return hr;
}

HRESULT m_IDirectDrawX::CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, LPDIRECTDRAWPALETTE FAR * lplpDDPalette, IUnknown FAR * pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__;

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
		*lplpDDPalette = ProxyAddressLookupTable.FindAddress<m_IDirectDrawPalette>(*lplpDDPalette);
	}

	return hr;
}

HRESULT m_IDirectDrawX::CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE7 FAR * lplpDDSurface, IUnknown FAR * pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (ProxyDirectXVersion > 3)
	{
		if (!lplpDDSurface || !lpDDSurfaceDesc)
		{
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);

		return CreateSurface2(&Desc2, lplpDDSurface, pUnkOuter, DirectXVersion);
	}

	HRESULT hr = GetProxyInterfaceV3()->CreateSurface(lpDDSurfaceDesc, (LPDIRECTDRAWSURFACE*)lplpDDSurface, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDSurface)
	{
		*lplpDDSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpDDSurface, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawX::CreateSurface2(LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPDIRECTDRAWSURFACE7 FAR * lplpDDSurface, IUnknown FAR * pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lplpDDSurface || !lpDDSurfaceDesc2)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for existing primary surface
		if ((lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) && GetPrimarySurface())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: primary surface already exists!");
			return DDERR_PRIMARYSURFACEALREADYEXISTS;
		}

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc2);
		Desc2.ddsCaps.dwCaps4 = 0x01;	// Indicates surface was created using CreateSurface()
		Desc2.dwReserved = 0;

		if ((lpDDSurfaceDesc2->dwFlags & DDSD_BACKBUFFERCOUNT) || (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_FLIP))
		{
			Desc2.ddsCaps.dwCaps |= DDSCAPS_FRONTBUFFER;
			if (!Desc2.dwBackBufferCount)
			{
				Desc2.dwBackBufferCount = 1;
			}
		}
		else
		{
			Desc2.dwBackBufferCount = 0;
		}

		if (displayModeBPP && (Desc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) && (!Desc2.dwWidth || !Desc2.dwHeight))
		{
			// Set width and height
			Desc2.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT;
			Desc2.dwWidth = displayModeWidth;
			Desc2.dwHeight = displayModeHeight;

			// Set refresh
			if (displayModeRefreshRate)
			{
				Desc2.dwFlags |= DDSD_REFRESHRATE;
				Desc2.dwRefreshRate = displayModeRefreshRate;
			}

			// Set Pixel Format
			Desc2.dwFlags |= DDSD_PIXELFORMAT;
			switch (displayModeBPP)
			{
			case 8:
				SetPixelDisplayFormat(D3DFMT_P8, Desc2.ddpfPixelFormat);
				break;
			case 16:
				SetPixelDisplayFormat(D3DFMT_R5G6B5, Desc2.ddpfPixelFormat);
				break;
			case 24:
				SetPixelDisplayFormat(D3DFMT_R8G8B8, Desc2.ddpfPixelFormat);
				break;
			case 32:
				SetPixelDisplayFormat(D3DFMT_X8R8G8B8, Desc2.ddpfPixelFormat);
				break;
			default:
				LOG_LIMIT(100, __FUNCTION__ << " Not implemented bit count " << displayModeBPP);
			}
		}

		m_IDirectDrawSurfaceX *p_IDirectDrawSurfaceX = new m_IDirectDrawSurfaceX(&d3d9Device, this, DirectXVersion, &Desc2, displayWidth, displayHeight);

		*lplpDDSurface = (LPDIRECTDRAWSURFACE7)p_IDirectDrawSurfaceX->GetWrapperInterfaceX(DirectXVersion);

		return DD_OK;
	}

	// BackBufferCount must be at least 1
	if (ProxyDirectXVersion != DirectXVersion && lpDDSurfaceDesc2)
	{
		if (((lpDDSurfaceDesc2->dwFlags & DDSD_BACKBUFFERCOUNT) || (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_FLIP)) && lpDDSurfaceDesc2->dwBackBufferCount == 0)
		{
			lpDDSurfaceDesc2->dwBackBufferCount = 1;
		}
	}

	HRESULT hr = ProxyInterface->CreateSurface(lpDDSurfaceDesc2, lplpDDSurface, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDSurface)
	{
		*lplpDDSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpDDSurface, DirectXVersion);

		if (Config.ConvertToDirectDraw7 && *lplpDDSurface)
		{
			m_IDirectDrawSurfaceX *lpDDSurfaceX = ((m_IDirectDrawSurface7*)*lplpDDSurface)->GetWrapperInterface();

			if (lpDDSurfaceX)
			{
				lpDDSurfaceX->SetDdrawParent(this);
			}
		}
	}

	return hr;
}

HRESULT m_IDirectDrawX::DuplicateSurface(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDIRECTDRAWSURFACE7 FAR * lplpDupDDSurface, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		m_IDirectDrawSurfaceX *lpDDSurfaceX = (m_IDirectDrawSurfaceX*)lpDDSurface;
		if (!DoesSurfaceExist(lpDDSurfaceX))
		{
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		lpDDSurfaceX->GetSurfaceDesc2(&Desc2);
		Desc2.ddsCaps.dwCaps &= ~DDSCAPS_PRIMARYSURFACE;		// Remove Primary surface flag

		m_IDirectDrawSurfaceX *p_IDirectDrawSurfaceX = new m_IDirectDrawSurfaceX(&d3d9Device, this, DirectXVersion, &Desc2, displayWidth, displayHeight);

		*lplpDupDDSurface = (LPDIRECTDRAWSURFACE7)p_IDirectDrawSurfaceX->GetWrapperInterfaceX(DirectXVersion);

		return DD_OK;
	}

	if (lpDDSurface)
	{
		lpDDSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDSurface)->GetProxyInterface();
	}

	HRESULT hr = ProxyInterface->DuplicateSurface(lpDDSurface, lplpDupDDSurface);

	if (SUCCEEDED(hr) && lplpDupDDSurface && lpDDSurface)
	{
		*lplpDupDDSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpDupDDSurface, ((m_IDirectDrawSurface7 *)lpDDSurface)->GetDirectXVersion());
	}

	return hr;
}

HRESULT m_IDirectDrawX::EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback)
{
	Logging::LogDebug() << __FUNCTION__;

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (ProxyDirectXVersion > 3)
	{
		if (!lpEnumModesCallback)
		{
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		if (lpDDSurfaceDesc)
		{
			ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);
		}

		ENUMDISPLAYMODES CallbackContext;
		CallbackContext.lpContext = lpContext;
		CallbackContext.lpCallback = lpEnumModesCallback;

		return EnumDisplayModes2(dwFlags, (lpDDSurfaceDesc) ? &Desc2 : nullptr, &CallbackContext, m_IDirectDrawEnumDisplayModes::ConvertCallback);
	}

	return GetProxyInterfaceV3()->EnumDisplayModes(dwFlags, lpDDSurfaceDesc, lpContext, lpEnumModesCallback);
}

HRESULT m_IDirectDrawX::EnumDisplayModes2(DWORD dwFlags, LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext, LPDDENUMMODESCALLBACK2 lpEnumModesCallback2)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lpEnumModesCallback2)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false)))
		{
			return DDERR_GENERIC;
		}

		// Save width, height and refresh rate
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
			EnumRefreshRate = Utils::GetRefreshRate(MainhWnd);
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
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

		// Setup display mode
		D3DDISPLAYMODE d3ddispmode;

		// Enumerate modes for format XRGB
		UINT modeCount = d3d9Object->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);

		// Loop through all modes
		DWORD Loop = 0;
		for (UINT i = 0; i < modeCount; i++)
		{
			// Get display modes
			ZeroMemory(&d3ddispmode, sizeof(D3DDISPLAYMODE));
			if (FAILED(d3d9Object->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, i, &d3ddispmode)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: EnumAdapterModes failed");
				break;
			}

			// Loop through each bit count
			for (DWORD bpMode : {8, 16, 24, 32})
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
					Desc2.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_REFRESHRATE | DDSD_PITCH | DDSD_PIXELFORMAT;
					Desc2.dwWidth = d3ddispmode.Width;
					Desc2.dwHeight = d3ddispmode.Height;
					Desc2.dwRefreshRate = d3ddispmode.RefreshRate;

					// Set adapter pixel format
					Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
					switch (DisplayBitCount)
					{
					case 8:
						SetPixelDisplayFormat(D3DFMT_P8, Desc2.ddpfPixelFormat);
						break;
					case 16:
						SetPixelDisplayFormat(D3DFMT_R5G6B5, Desc2.ddpfPixelFormat);
						break;
					case 24:
						SetPixelDisplayFormat(D3DFMT_R8G8B8, Desc2.ddpfPixelFormat);
						break;
					case 32:
						SetPixelDisplayFormat(D3DFMT_X8R8G8B8, Desc2.ddpfPixelFormat);
						break;
					}
					Desc2.lPitch = (Desc2.ddpfPixelFormat.dwRGBBitCount / 8) * Desc2.dwHeight;

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
	Logging::LogDebug() << __FUNCTION__;

	if (!lpEnumSurfacesCallback)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (ProxyDirectXVersion > 3)
	{
		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		if (lpDDSurfaceDesc)
		{
			ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);
		}

		return EnumSurfaces2(dwFlags, (lpDDSurfaceDesc) ? &Desc2 : nullptr, lpContext, (LPDDENUMSURFACESCALLBACK7)lpEnumSurfacesCallback, DirectXVersion);
	}

	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = lpEnumSurfacesCallback;
	CallbackContext.DirectXVersion = DirectXVersion;

	return GetProxyInterfaceV3()->EnumSurfaces(dwFlags, lpDDSurfaceDesc, &CallbackContext, m_IDirectDrawEnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawX::EnumSurfaces2(DWORD dwFlags, LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback7, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!lpEnumSurfacesCallback7)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback7 = lpEnumSurfacesCallback7;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ConvertSurfaceDescTo2 = (ProxyDirectXVersion > 3 && DirectXVersion < 4);

	return ProxyInterface->EnumSurfaces(dwFlags, lpDDSurfaceDesc2, &CallbackContext, m_IDirectDrawEnumSurface::ConvertCallback2);
}

HRESULT m_IDirectDrawX::FlipToGDISurface()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DD_OK;
	}

	return ProxyInterface->FlipToGDISurface();
}

HRESULT m_IDirectDrawX::GetCaps(LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDHELCaps)
{
	Logging::LogDebug() << __FUNCTION__;

	DDCAPS DriverCaps, HELCaps;
	DriverCaps.dwSize = sizeof(DDCAPS);
	HELCaps.dwSize = sizeof(DDCAPS);

	HRESULT hr = DDERR_GENERIC;

	if (Config.Dd7to9)
	{
		if (!lpDDDriverCaps && !lpDDHELCaps)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false)))
		{
			return DDERR_GENERIC;
		}

		// Get video memory
		DWORD dwVidTotal = MaxVidMemory;
		DWORD dwVidFree = MaxVidMemory - 0x100000;
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
	Logging::LogDebug() << __FUNCTION__;

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (ProxyDirectXVersion > 3)
	{
		if (!lpDDSurfaceDesc)
		{
			return DDERR_INVALIDPARAMS;
		}

		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof(DDSURFACEDESC2);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);

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
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lpDDSurfaceDesc2)
		{
			// Just return OK
			return DD_OK;
		}

		// Set Surface Desc
		lpDDSurfaceDesc2->dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_REFRESHRATE | DDSD_PIXELFORMAT;
		DWORD displayModeBits = displayModeBPP;
		if (displayModeBits)
		{
			lpDDSurfaceDesc2->dwWidth = displayModeWidth;
			lpDDSurfaceDesc2->dwHeight = displayModeHeight;
			lpDDSurfaceDesc2->dwRefreshRate = displayModeRefreshRate;
		}
		else
		{
			HDC hdc = GetDC(nullptr);
			lpDDSurfaceDesc2->dwWidth = GetSystemMetrics(SM_CXSCREEN);
			lpDDSurfaceDesc2->dwHeight = GetSystemMetrics(SM_CYSCREEN);
			lpDDSurfaceDesc2->dwRefreshRate = Utils::GetRefreshRate(MainhWnd);
			displayModeBits = GetDeviceCaps(hdc, BITSPIXEL);
			ReleaseDC(nullptr, hdc);
		}

		// Force color mode
		displayModeBits = (Config.DdrawOverrideBitMode) ? Config.DdrawOverrideBitMode : displayModeBits;

		// Set Pixel Format
		switch (displayModeBits)
		{
		case 8:
			SetPixelDisplayFormat(D3DFMT_P8, lpDDSurfaceDesc2->ddpfPixelFormat);
			break;
		case 16:
			SetPixelDisplayFormat(D3DFMT_R5G6B5, lpDDSurfaceDesc2->ddpfPixelFormat);
			break;
		case 24:
			SetPixelDisplayFormat(D3DFMT_R8G8B8, lpDDSurfaceDesc2->ddpfPixelFormat);
			break;
		case 32:
			SetPixelDisplayFormat(D3DFMT_X8R8G8B8, lpDDSurfaceDesc2->ddpfPixelFormat);
			break;
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Not implemented bit count " << displayModeBits);
			return DDERR_UNSUPPORTED;
		}

		return DD_OK;
	}

	return ProxyInterface->GetDisplayMode(lpDDSurfaceDesc2);
}

HRESULT m_IDirectDrawX::GetFourCCCodes(LPDWORD lpNumCodes, LPDWORD lpCodes)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->GetFourCCCodes(lpNumCodes, lpCodes);
}

HRESULT m_IDirectDrawX::GetGDISurface(LPDIRECTDRAWSURFACE7 FAR * lplpGDIDDSSurface, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

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
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lpdwFrequency)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		D3DDISPLAYMODE Mode;
		if (FAILED(d3d9Device->GetDisplayMode(0, &Mode)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get display mode!");
			return DDERR_GENERIC;
		}

		*lpdwFrequency = Mode.RefreshRate;

		return DD_OK;
	}

	return ProxyInterface->GetMonitorFrequency(lpdwFrequency);
}

HRESULT m_IDirectDrawX::GetScanLine(LPDWORD lpdwScanLine)
{
	Logging::LogDebug() << __FUNCTION__;

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
	Logging::LogDebug() << __FUNCTION__;

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
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Not needed with d3d9
		return DD_OK;
	}

	if (Config.ConvertToDirectDraw7)
	{
		ProxyInterface->Initialize(lpGUID);

		// Just return OK
		return DD_OK;
	}

	return ProxyInterface->Initialize(lpGUID);
}

HRESULT m_IDirectDrawX::RestoreDisplayMode()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// No exclusive mode
		if (!ExclusiveMode)
		{
			return DDERR_NOEXCLUSIVEMODE;
		}

		// Set mode
		IsInScene = false;
		ExclusiveMode = false;
		AllowModeX = false;
		MultiThreaded = false;
		FUPPreserve = false;
		NoWindowChanges = false;
		isWindowed = false;
		displayModeWidth = 0;
		displayModeHeight = 0;
		displayModeBPP = 0;
		displayModeRefreshRate = 0;

		return DD_OK;
	}

	return ProxyInterface->RestoreDisplayMode();
}

// Fixes a bug in ddraw in Windows 8 and 10 where the exclusive flag remains even after the window (hWnd) closes
LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	if (nCode == HCBT_DESTROYWND && !Config.Exiting)
	{
		Logging::LogDebug() << __FUNCTION__;

		HWND hWnd = (HWND)wParam;
		auto it = g_hookmap.find(hWnd);
		if (it != std::end(g_hookmap))
		{
			m_IDirectDraw7 *lpDDraw = it->second;
			if (lpDDraw && ProxyAddressLookupTable.IsValidAddress(lpDDraw))
			{
				lpDDraw->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
			}
			g_hookmap.clear();
		}
	}

	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

// Enums all windows and returns the handle to the active window
BOOL CALLBACK EnumProcWindowCallback(HWND hwnd, LPARAM lParam)
{
	// Get variables from call back
	handle_data& data = *(handle_data*)lParam;

	// Skip windows that are from a different process ID
	DWORD process_id;
	GetWindowThreadProcessId(hwnd, &process_id);
	if (data.process_id != process_id)
	{
		return TRUE;
	}

	// Skip compatibility class windows
	char class_name[80] = { 0 };
	GetClassName(hwnd, class_name, sizeof(class_name));
	if (strcmp(class_name, "CompatWindowDesktopReplacement") == 0)			// Compatibility class windows
	{
		return TRUE;
	}

	// Match found returning value
	data.best_handle = hwnd;
	return FALSE;
}

HRESULT m_IDirectDrawX::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if ((dwFlags & DDSCL_NORMAL) && (dwFlags & (DDSCL_ALLOWMODEX | DDSCL_FULLSCREEN)))
		{
			return DDERR_INVALIDPARAMS;
		}

		bool ChangeMode = false;

		// Set windowed mode
		if ((dwFlags & DDSCL_NORMAL) || Config.EnableWindowMode)
		{
			if (!isWindowed)
			{
				ChangeMode = true;
				isWindowed = true;
			}
		}
		else if (dwFlags & DDSCL_FULLSCREEN)
		{
			if (isWindowed)
			{
				ChangeMode = true;
				isWindowed = false;
			}
		}

		// Set device flags
		ExclusiveMode = (dwFlags & DDSCL_EXCLUSIVE);
		AllowModeX = (dwFlags & DDSCL_ALLOWMODEX);
		MultiThreaded = (dwFlags & DDSCL_MULTITHREADED);
		FUPPreserve = (dwFlags & (DDSCL_FPUPRESERVE | DDSCL_FPUSETUP));
		NoWindowChanges = (dwFlags & DDSCL_NOWINDOWCHANGES);

		// Set display window
		HWND t_hWnd = hWnd;
		if (!t_hWnd)
		{
			// Set variables
			handle_data data;
			data.process_id = GetCurrentProcessId();
			data.best_handle = nullptr;

			// Gets all window layers and looks for a main window that is fullscreen
			EnumWindows(EnumProcWindowCallback, (LPARAM)&data);

			// Get top window
			HWND m_hWnd = GetTopWindow(data.best_handle);
			if (m_hWnd)
			{
				data.best_handle = m_hWnd;
			}

			// Cannot find window handle
			if (!data.best_handle)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get window handle");
			}

			// Return the best handle
			t_hWnd = data.best_handle;
		}

		MainhWnd = t_hWnd;

		// Update the d3d9 device to use new windowed mode
		if (ChangeMode && hWnd && d3d9Device && FAILED(CreateD3D9Device()))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: creating Direct3D9 Device");
			return DDERR_GENERIC;
		}

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->SetCooperativeLevel(hWnd, dwFlags);

	// Release previouse Exclusive flag
	// Hook window message to get notified when the window is about to exit to remove the exclusive flag
	if (SUCCEEDED(hr) && (dwFlags & DDSCL_EXCLUSIVE) && IsWindow(hWnd) && hWnd != chWnd)
	{
		g_hookmap.clear();

		if (g_hook)
		{
			UnhookWindowsHookEx(g_hook);
			g_hook = nullptr;
		}

		g_hookmap[hWnd] = WrapperInterface;
		g_hook = SetWindowsHookEx(WH_CBT, CBTProc, nullptr, GetWindowThreadProcessId(hWnd, nullptr));

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
			Logging::LogDebug() << __FUNCTION__ << " Removing window WS_CAPTION!";

			SetWindowLong(hWnd, GWL_STYLE, lStyle & ~WS_CAPTION);
			SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_FRAMECHANGED);
		}
	}

	return hr;
}

HRESULT m_IDirectDrawX::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		bool ChangeMode = false;

		// Set display mode to dwWidth x dwHeight with dwBPP color depth
		DWORD NewWidth = dwWidth;
		DWORD NewHeight = dwHeight;
		DWORD NewBPP = (Config.DdrawOverrideBitMode) ? Config.DdrawOverrideBitMode : dwBPP;
		DWORD NewRefreshRate = dwRefreshRate;

		if (displayModeWidth != NewWidth || displayModeHeight != NewHeight || displayModeBPP != NewBPP || displayModeRefreshRate != NewRefreshRate)
		{
			ChangeMode = true;
			displayModeWidth = NewWidth;
			displayModeHeight = NewHeight;
			displayModeBPP = NewBPP;
			displayModeRefreshRate = NewRefreshRate;
		}

		if (SetDefaultDisplayMode || !displayWidth || !displayHeight)
		{
			displayWidth = dwWidth;
			displayHeight = dwHeight;
		}

		// Update the d3d9 device to use new display mode
		if (ChangeMode && d3d9Device && FAILED(CreateD3D9Device()))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: creating Direct3D9 Device");
			return DDERR_GENERIC;
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
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Check flags
		switch (dwFlags)
		{
		case DDWAITVB_BLOCKBEGIN:
			// Return when vertical blank begins
		case DDWAITVB_BLOCKEND:
			// Return when the vertical blank interval ends and the display begins
			break;
		case DDWAITVB_BLOCKBEGINEVENT:
			// Triggers an event when the vertical blank begins. This value is not supported.
			return DDERR_UNSUPPORTED;
		default:
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		// Do some simple wait
		D3DRASTER_STATUS RasterStatus;
		if (SUCCEEDED(d3d9Device->GetRasterStatus(0, &RasterStatus)) && 
			(!RasterStatus.InVBlank || dwFlags != DDWAITVB_BLOCKBEGIN) &&
			monitorHeight && monitorRefreshRate)
		{
			float percentageLeft = 1.0f - (float)((RasterStatus.InVBlank) ? monitorHeight : RasterStatus.ScanLine) / (float)monitorHeight;
			float blinkTime = trunc(1000.0f / monitorRefreshRate);
			DWORD WaitTime = min(100, (DWORD)trunc(blinkTime * percentageLeft) + ((dwFlags == DDWAITVB_BLOCKBEGIN) ? 0 : 2));
			Sleep(WaitTime);
		}

		// Vertical blank supported by vsync so just return
		return DD_OK;
	}

	return ProxyInterface->WaitForVerticalBlank(dwFlags, hEvent);
}

/*********************************/
/*** Added in the v2 interface ***/
/*********************************/

HRESULT m_IDirectDrawX::GetAvailableVidMem(LPDDSCAPS lpDDSCaps, LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
	Logging::LogDebug() << __FUNCTION__;

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
	SetVidMemory(lpdwTotal, lpdwFree);

	return hr;
}

HRESULT m_IDirectDrawX::GetAvailableVidMem2(LPDDSCAPS2 lpDDSCaps2, LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = DD_OK;

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		DWORD TotalMemory = d3d9Device->GetAvailableTextureMem();

		if (lpdwTotal)
		{
			*lpdwTotal = TotalMemory;
		}
		if (lpdwFree)
		{
			*lpdwFree = (TotalMemory > 0x100000) ? TotalMemory - 0x100000 : TotalMemory;
		}
	}
	else
	{
		hr = ProxyInterface->GetAvailableVidMem(lpDDSCaps2, lpdwTotal, lpdwFree);
	}

	// Set available memory
	SetVidMemory(lpdwTotal, lpdwFree);

	return hr;
}

/*********************************/
/*** Added in the V4 Interface ***/
/*********************************/

HRESULT m_IDirectDrawX::GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 * lpDDS, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

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
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		for (m_IDirectDrawSurfaceX* it : SurfaceVector)
		{
			HRESULT hr = it->Restore();
			if (FAILED(hr))
			{
				return hr;
			}
		}

		return DD_OK;
	}

	return ProxyInterface->RestoreAllSurfaces();
}

HRESULT m_IDirectDrawX::TestCooperativeLevel()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!d3d9Device)
		{
			if (ExclusiveMode)
			{
				return DDERR_EXCLUSIVEMODEALREADYSET;
			}

			// ToDo: impement the following return codes
			//
			// DDERR_NOEXCLUSIVEMODE - Operation requires the application to have exclusive mode but the application does not have exclusive mode.
			// DDERR_WRONGMODE - This surface can not be restored because it was created in a different mode.
			return DD_OK;
		}

		return d3d9Device->TestCooperativeLevel();
	}

	return ProxyInterface->TestCooperativeLevel();
}

HRESULT m_IDirectDrawX::GetDeviceIdentifier(LPDDDEVICEIDENTIFIER lpdddi, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

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
	Logging::LogDebug() << __FUNCTION__;

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
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->StartModeTest(lpModesToTest, dwNumEntries, dwFlags);
}

HRESULT m_IDirectDrawX::EvaluateMode(DWORD dwFlags, DWORD * pSecondsUntilTimeout)
{
	Logging::LogDebug() << __FUNCTION__;

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

LPDIRECT3D9 m_IDirectDrawX::GetDirect3D9Object()
{
	return d3d9Object;
}

LPDIRECT3DDEVICE9 *m_IDirectDrawX::GetDirect3D9Device()
{
	return &d3d9Device;
}

void m_IDirectDrawX::InitDdraw(LPDIRECT3D9 pObject)
{
	DWORD ref = InterlockedIncrement(&ddrawRefCount);

	ShareD3d9DeviceFlag = true;

	if (ref == 1)
	{
		d3d9Object = pObject;

		SetDdrawDefaults();
	}
}

void m_IDirectDrawX::SetDdrawDefaults()
{
	// Convert to Direct3D9
	MainhWnd = nullptr;
	IsInScene = false;
	ExclusiveMode = false;
	AllowModeX = false;
	MultiThreaded = false;
	FUPPreserve = false;
	NoWindowChanges = false;
	isWindowed = true;

	// Application display mode
	displayModeWidth = 0;
	displayModeHeight = 0;
	displayModeBPP = 0;
	displayModeRefreshRate = 0;

	// Display resolution
	displayWidth = (Config.DdrawUseNativeResolution) ? GetSystemMetrics(SM_CXSCREEN) : (Config.DdrawOverrideWidth) ? Config.DdrawOverrideWidth : 0;
	displayHeight = (Config.DdrawUseNativeResolution) ? GetSystemMetrics(SM_CYSCREEN) : (Config.DdrawOverrideHeight) ? Config.DdrawOverrideHeight : 0;
	displayRefreshRate = (Config.DdrawOverrideRefreshRate) ? Config.DdrawOverrideRefreshRate : 0;

	SetDefaultDisplayMode = (!displayWidth || !displayHeight);

	// Other settings
	monitorRefreshRate = 0;
	monitorHeight = 0;
	FrequencyFlag = QueryPerformanceFrequency(&clockFrequency);
}

void m_IDirectDrawX::ReleaseDdraw()
{
	DWORD ref = InterlockedDecrement(&ddrawRefCount);

	if (ref == 0)
	{
		ShareD3d9DeviceFlag = false;
	}

	SetCriticalSection();

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
		pSurface->ReleaseD9Surface();
		pSurface->ClearDdraw();
	}
	SurfaceVector.clear();

	// Release palettes
	for (m_IDirectDrawPalette *pPalette : PaletteVector)
	{
		pPalette->ClearDdraw();
	}
	PaletteVector.clear();

	// Release shared d3d9device
	ReleaseD3d9Device();

	// Release shared d3d9object
	if (d3d9Object)
	{
		if (d3d9Object->Release() == 0)
		{
			d3d9Object = nullptr;
		}
	}

	ReleaseCriticalSection();
}

HWND m_IDirectDrawX::GetHwnd()
{
	return MainhWnd;
}

bool m_IDirectDrawX::IsExclusiveMode()
{
	return ExclusiveMode;
}

HRESULT m_IDirectDrawX::CheckInterface(char *FunctionName, bool CheckD3DDevice)
{
	// Check for device
	if (!d3d9Object)
	{
		LOG_LIMIT(100, FunctionName << " Error: no d3d9 object!");
		return DDERR_GENERIC;
	}

	// Check for device, if not then create it
	if (CheckD3DDevice)
	{
		if (!d3d9Device)
		{
			if (FAILED(CreateD3D9Device()))
			{
				LOG_LIMIT(100, FunctionName << " Error: d3d9 device not setup!");
				return DDERR_GENERIC;
			}
		}
	}

	return DD_OK;
}

// Creates d3d9 device, destroying the old one if exists
HRESULT m_IDirectDrawX::CreateD3D9Device()
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, false)))
	{
		return DDERR_GENERIC;
	}

	// Release all existing surfaces
	ReleaseAllD9Surfaces();

	// Release existing d3d9device
	ReleaseD3d9Device();

	// Check device caps to make sure it supports dynamic textures
	D3DCAPS9 d3dcaps;
	ZeroMemory(&d3dcaps, sizeof(D3DCAPS9));
	if (FAILED(d3d9Object->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dcaps)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to retrieve device-specific information about the device");
		return DDERR_GENERIC;
	}

	// Is dynamic textures flag set?
	if (!(d3dcaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES))
	{
		// No dynamic textures
		LOG_LIMIT(100, __FUNCTION__ << " Error: device does not support dynamic textures");
		return DDERR_GENERIC;
	}

	// Get width and height
	DWORD BackBufferWidth = displayWidth;
	DWORD BackBufferHeight = displayHeight;
	if (!BackBufferWidth || !BackBufferHeight)
	{
		BackBufferWidth = GetSystemMetrics(SM_CXSCREEN);
		BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
	}

	// Set display window
	ZeroMemory(&presParams, sizeof(presParams));

	// Discard swap
	presParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	// Backbuffer
	presParams.BackBufferCount = 1;
	// Auto stencel
	presParams.EnableAutoDepthStencil = true;
	// Auto stencel format
	presParams.AutoDepthStencilFormat = D3DFMT_D24S8;
	// Interval level
	presParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	// Set parameters for the current display mode
	if (isWindowed || !MainhWnd)
	{
		// Window mode
		presParams.Windowed = TRUE;
		// Width/height
		presParams.BackBufferWidth = BackBufferWidth;
		presParams.BackBufferHeight = BackBufferHeight;
	}
	else
	{
		// Enumerate modes for format XRGB
		UINT modeCount = d3d9Object->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);

		// Check for ModeX resolutions
		if ((BackBufferWidth == 320 && BackBufferHeight == 200) ||
			(BackBufferWidth == 640 && BackBufferHeight == 400))
		{
			BackBufferHeight += BackBufferHeight / 5;
		}

		// Get refresh rate
		if (Config.DdrawUseNativeResolution && !displayRefreshRate)
		{
			displayRefreshRate = Utils::GetRefreshRate(MainhWnd);
		}
		DWORD BackBufferRefreshRate = (displayRefreshRate) ? displayRefreshRate : Utils::GetRefreshRate(MainhWnd);

		// Loop through all modes looking for our requested resolution
		D3DDISPLAYMODE d3ddispmode;
		D3DDISPLAYMODE set_d3ddispmode = { NULL };
		bool modeFound = false;
		for (UINT i = 0; i < modeCount; i++)
		{
			// Get display modes here
			ZeroMemory(&d3ddispmode, sizeof(D3DDISPLAYMODE));
			if (FAILED(d3d9Object->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, i, &d3ddispmode)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: EnumAdapterModes failed");
				return DDERR_GENERIC;
			}
			if (d3ddispmode.Width == BackBufferWidth && d3ddispmode.Height == BackBufferHeight &&		// Check height and width
				(d3ddispmode.RefreshRate == BackBufferRefreshRate || !BackBufferRefreshRate))			// Check refresh rate
			{
				// Found a match
				modeFound = true;
				memcpy(&set_d3ddispmode, &d3ddispmode, sizeof(D3DDISPLAYMODE));
			}
		}

		// No mode found
		if (!modeFound)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to find compatible fullscreen display mode " << BackBufferWidth << "x" << BackBufferHeight);
			return DDERR_GENERIC;
		}

		// Fullscreen
		presParams.Windowed = FALSE;
		// Width/height
		presParams.BackBufferWidth = set_d3ddispmode.Width;
		presParams.BackBufferHeight = set_d3ddispmode.Height;
		// Backbuffer
		presParams.BackBufferFormat = set_d3ddispmode.Format;
		// Display mode refresh
		presParams.FullScreen_RefreshRateInHz = set_d3ddispmode.RefreshRate;
	}

	// Set behavior flags
	DWORD BehaviorFlags = ((d3dcaps.VertexProcessingCaps) ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING) | D3DCREATE_MULTITHREADED |
		((MultiThreaded) ? D3DCREATE_MULTITHREADED : 0) |
		((FUPPreserve) ? D3DCREATE_FPU_PRESERVE : 0) |
		((NoWindowChanges) ? D3DCREATE_NOWINDOWCHANGES : 0);

	Logging::LogDebug() << __FUNCTION__ << " wnd: " << MainhWnd << " D3d9 Device size: " << presParams.BackBufferWidth << "x" << presParams.BackBufferHeight << " refresh: " << presParams.FullScreen_RefreshRateInHz <<
		" format: " << presParams.BackBufferFormat << " flags: " << BehaviorFlags;

	// Create d3d9 Device
	if (FAILED(d3d9Object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, MainhWnd, BehaviorFlags, &presParams, &d3d9Device)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create Direct3D9 device");
		return DDERR_GENERIC;
	}

	// Store display frequency
	monitorRefreshRate = (presParams.FullScreen_RefreshRateInHz) ? presParams.FullScreen_RefreshRateInHz : Utils::GetRefreshRate(MainhWnd);
	monitorHeight = GetSystemMetrics(SM_CYSCREEN);

	// Reset BeginScene
	IsInScene = false;

	// Success
	return DD_OK;
}

// Reinitialize d3d9 device
HRESULT m_IDirectDrawX::ReinitDevice()
{
	Logging::LogDebug() << __FUNCTION__;

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true)))
	{
		return DDERR_GENERIC;
	}

	// Check if device is ready to be restored
	if (d3d9Device->TestCooperativeLevel() == D3DERR_DEVICELOST)
	{
		return DDERR_GENERIC;
	}

	// Release existing surfaces
	ReleaseAllD9Surfaces();

	// Attempt to reset the device
	if (FAILED(d3d9Device->Reset(&presParams)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to reset Direct3D9 device");
		return DDERR_GENERIC;
	}

	// Reset BeginScene
	IsInScene = false;

	// Success
	return DD_OK;
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
	// Release device
	if (d3d9Device)
	{
		// EndEcene
		if (IsInScene)
		{
			d3d9Device->EndScene();
		}

		if (d3d9Device->Release() == 0)
		{
			d3d9Device = nullptr;
		}
	}

	// Set is not in scene
	IsInScene = false;
}

// Add surface wrapper to vector
void m_IDirectDrawX::AddSurfaceToVector(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX || DoesSurfaceExist(lpSurfaceX))
	{
		return;
	}

	// Store surface
	SurfaceVector.push_back(lpSurfaceX);
}

// Remove surface wrapper from vector
void m_IDirectDrawX::RemoveSurfaceFromVector(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return;
	}

	auto it = std::find_if(SurfaceVector.begin(), SurfaceVector.end(),
		[=](auto pSurface) -> bool { return pSurface == lpSurfaceX; });

	if (it != std::end(SurfaceVector))
	{
		SurfaceVector.erase(it);
	}

	// Remove attached surface from map
	for (m_IDirectDrawSurfaceX *pSurface : SurfaceVector)
	{
		pSurface->RemoveAttachedSurfaceFromMap(lpSurfaceX);
	}
}

// Check if surface wrapper exists
bool m_IDirectDrawX::DoesSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return false;
	}

	auto it = std::find_if(SurfaceVector.begin(), SurfaceVector.end(),
		[=](auto pSurface) -> bool { return pSurface == lpSurfaceX; });

	if (it == std::end(SurfaceVector))
	{
		return false;
	}

	return true;
}

// Get the primary surface
m_IDirectDrawSurfaceX *m_IDirectDrawX::GetPrimarySurface()
{
	// Check for primary surface
	for (m_IDirectDrawSurfaceX *pSurface : SurfaceVector)
	{
		if (pSurface->IsPrimarySurface())
		{
			return pSurface;
		}
	}
	return nullptr;
}

// This method removes any texture surfaces created with the DDSCAPS2_TEXTUREMANAGE or DDSCAPS2_D3DTEXTUREMANAGE flags
void m_IDirectDrawX::EvictManagedTextures()
{
	// Check if any surfaces are locked
	for (m_IDirectDrawSurfaceX *pSurface : SurfaceVector)
	{
		if (pSurface->IsSurfaceManaged())
		{
			pSurface->ReleaseD9Surface();
		}
	}
}

// Add palette wrapper to vector
void m_IDirectDrawX::AddPaletteToVector(m_IDirectDrawPalette* lpPalette)
{
	if (!lpPalette || DoesPaletteExist(lpPalette))
	{
		return;
	}

	// Store palette
	PaletteVector.push_back(lpPalette);
}

// Remove palette wrapper from vector
void m_IDirectDrawX::RemovePaletteFromVector(m_IDirectDrawPalette* lpPalette)
{
	if (!lpPalette)
	{
		return;
	}

	auto it = std::find_if(PaletteVector.begin(), PaletteVector.end(),
		[=](auto pPalette) -> bool { return pPalette == lpPalette; });

	if (it != std::end(PaletteVector))
	{
		PaletteVector.erase(it);
	}
}

// Check if palette wrapper exists
bool m_IDirectDrawX::DoesPaletteExist(m_IDirectDrawPalette* lpPalette)
{
	if (!lpPalette)
	{
		return false;
	}

	auto it = std::find_if(PaletteVector.begin(), PaletteVector.end(),
		[=](auto pSurface) -> bool { return pSurface == lpPalette; });

	if (it == std::end(PaletteVector))
	{
		return false;
	}

	return true;
}

void m_IDirectDrawX::SetVidMemory(LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
	// Set available memory, some games have issues if this is set to high
	if (lpdwTotal && *lpdwTotal > MaxVidMemory)
	{
		*lpdwTotal = MaxVidMemory;
	}
	DWORD TotalVidMem = (lpdwTotal) ? *lpdwTotal : (lpdwFree) ? *lpdwFree : MaxVidMemory;
	if (lpdwFree && *lpdwFree > TotalVidMem)
	{
		*lpdwFree = (TotalVidMem > 0x100000) ? TotalVidMem - 0x100000 : TotalVidMem;
	}
}

// Do d3d9 BeginScene if all surfaces are unlocked
HRESULT m_IDirectDrawX::BeginScene()
{
	Logging::LogDebug() << __FUNCTION__;

	// Check if we can run BeginScene
	if (IsInScene)
	{
		return DDERR_GENERIC;
	}

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true)))
	{
		return DDERR_GENERIC;
	}

	// Check if any surfaces are locked
	for (m_IDirectDrawSurfaceX* it : SurfaceVector)
	{
		if (it->IsSurfaceLocked())
		{
			return DDERR_LOCKEDSURFACES;
		}
	}

	// Begin scene
	if (FAILED(d3d9Device->BeginScene()))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to begin scene");
		return DDERR_GENERIC;
	}
	IsInScene = true;

	return DD_OK;
}

// Do d3d9 EndScene and Present if all surfaces are unlocked
HRESULT m_IDirectDrawX::EndScene()
{
	Logging::LogDebug() << __FUNCTION__;

	// Run BeginScene (ignore results)
	BeginScene();

	// Check if BeginScene has finished
	if (!IsInScene)
	{
		return DDERR_GENERIC;
	}

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true)))
	{
		return DDERR_GENERIC;
	}

	// Draw primitive
	if (FAILED(d3d9Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to draw primitive");
		return DDERR_GENERIC;
	}

	// Skip frame if time lapse is too small
	if (Config.AutoFrameSkip)
	{
		if (FrequencyFlag)
		{
			FrameCounter++;

			// Get screen frequency timer
			float MaxScreenTimer = (1000.0f / monitorRefreshRate);

			// Get time since last successful endscene
			bool CounterFlag = QueryPerformanceCounter(&clickTime);
			float deltaPresentMS = ((clickTime.QuadPart - lastPresentTime.QuadPart) * 1000.0f) / clockFrequency.QuadPart;

			// Get time since last skipped frame
			float deltaFrameMS = (lastFrameTime) ? ((clickTime.QuadPart - lastFrameTime) * 1000.0f) / clockFrequency.QuadPart : deltaPresentMS;
			lastFrameTime = clickTime.QuadPart;

			// Use last frame time and average frame time to decide if next frame will be less than the screen frequency timer
			if (CounterFlag && (deltaPresentMS + (deltaFrameMS * 1.1f) < MaxScreenTimer) && (deltaPresentMS + ((deltaPresentMS / FrameCounter) * 1.1f) < MaxScreenTimer))
			{
				Logging::LogDebug() << __func__ << " Skipping frame " << deltaPresentMS << "ms screen frequancy " << MaxScreenTimer;
				return D3D_OK;
			}
			Logging::LogDebug() << __func__ << " Drawing frame " << deltaPresentMS << "ms screen frequancy " << MaxScreenTimer;
		}
	}

	// End scene
	if (FAILED(d3d9Device->EndScene()))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to end scene");
		return DDERR_GENERIC;
	}
	IsInScene = false;

	// Present everthing
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
		return DDERR_GENERIC;
	}

	// Store new click time after frame draw is complete
	if (Config.AutoFrameSkip)
	{
		if (QueryPerformanceCounter(&clickTime))
		{
			lastPresentTime.QuadPart = clickTime.QuadPart;
			lastFrameTime = 0;
			FrameCounter = 0;
		}
	}

	// BeginScene after EndScene is done
	BeginScene();

	return DD_OK;
}
