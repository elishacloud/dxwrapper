/**
* Copyright (C) 2018 Elisha Riedlinger
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

std::unordered_map<HWND, IDirectDraw7*> g_hookmap;

HRESULT m_IDirectDrawX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	if (Config.Dd7to9)
	{
		if ((riid == IID_IDirectDraw || riid == IID_IDirectDraw2 || riid == IID_IDirectDraw4 || riid == IID_IDirectDraw7 || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return S_OK;
		}
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, WrapperInterface);
}

ULONG m_IDirectDrawX::AddRef()
{
	if (Config.Dd7to9)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawX::Release()
{
	if (Config.Dd7to9)
	{
		LONG ref = InterlockedDecrement(&RefCount);

		if (ref == 0)
		{
			ReleaseD3d9();

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

	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		WrapperInterface->DeleteMe();
	}

	return x;
}

void m_IDirectDrawX::ReleaseD3d9()
{
	// Release existing vertex buffer
	if (vertexBuffer)
	{
		vertexBuffer->Release();
		vertexBuffer = nullptr;
	}

	// Release existing surface texture
	if (surfaceTexture)
	{
		surfaceTexture->Release();
		surfaceTexture = nullptr;
	}

	// Release existing d3ddevice
	if (d3d9Device)
	{
		if (d3d9Device->Release() != 0)
		{
			Logging::Log() << __FUNCTION__ << " Unable to release Direct3D9 device";
		}
		d3d9Device = nullptr;
	}

	// Release existing d3dobject
	if (d3d9Object)
	{
		if (d3d9Object->Release() != 0)
		{
			Logging::Log() << __FUNCTION__ << " Unable to release Direct3D9 device";
		}
		d3d9Object = nullptr;
	}
}

HRESULT m_IDirectDrawX::Compact()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->Compact();
}

HRESULT m_IDirectDrawX::CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR * lplpDDClipper, IUnknown FAR * pUnkOuter)
{
	if (Config.Dd7to9)
	{
		if (!lplpDDClipper)
		{
			return DDERR_INVALIDPARAMS;
		}

		*lplpDDClipper = new m_IDirectDrawClipper(nullptr);

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
	if (Config.Dd7to9)
	{
		if (!lplpDDPalette || !lpDDColorArray)
		{
			return DDERR_INVALIDPARAMS;
		}

		*lplpDDPalette = new m_IDirectDrawPalette(dwFlags, lpDDColorArray);

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->CreatePalette(dwFlags, lpDDColorArray, lplpDDPalette, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDPalette)
	{
		*lplpDDPalette = ProxyAddressLookupTable.FindAddress<m_IDirectDrawPalette>(*lplpDDPalette);
	}

	return hr;
}

HRESULT m_IDirectDrawX::CreateSurface(LPDDSURFACEDESC2 lpDDSurfaceDesc, LPDIRECTDRAWSURFACE7 FAR * lplpDDSurface, IUnknown FAR * pUnkOuter)
{
	if (!lplpDDSurface)
	{
		return DDERR_INVALIDPARAMS;
	}

	DDSURFACEDESC2 Desc2;
	if (lpDDSurfaceDesc && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *(LPDDSURFACEDESC)lpDDSurfaceDesc);
		lpDDSurfaceDesc = &Desc2;

		// BackBufferCount must be at least 1
		lpDDSurfaceDesc->dwBackBufferCount = (lpDDSurfaceDesc->dwBackBufferCount) ? lpDDSurfaceDesc->dwBackBufferCount : 1;
	}

	if (Config.Dd7to9)
	{
		// Set parameters before call
		lpDDSurfaceDesc->dwWidth = (lpDDSurfaceDesc->dwWidth) ? lpDDSurfaceDesc->dwWidth : displayModeWidth;
		lpDDSurfaceDesc->dwHeight = (lpDDSurfaceDesc->dwHeight) ? lpDDSurfaceDesc->dwHeight : displayModeHeight;
		lpDDSurfaceDesc->dwRefreshRate = (lpDDSurfaceDesc->dwRefreshRate) ? lpDDSurfaceDesc->dwRefreshRate : displayModerefreshRate;

		if (!lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount && displayModeBPP)
		{
			// Set BitCount
			lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount = displayModeBPP;

			// Set BitMask
			switch (displayModeBPP)
			{
				case 8:
					break;
				case 16:
					lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask = 0xF800;
					lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask = 0x7E0;
					lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask = 0x1F;
					break;
				case 24:
				case 32:
					lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask = 0xFF0000;
					lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask = 0xFF00;
					lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask = 0xFF;
					break;
			}
		}

		*lplpDDSurface = new m_IDirectDrawSurfaceX(&d3d9Device, this, DirectXVersion, lpDDSurfaceDesc, displayModeWidth, displayModeHeight);

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->CreateSurface(lpDDSurfaceDesc, lplpDDSurface, pUnkOuter);

	if (SUCCEEDED(hr))
	{
		*lplpDDSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpDDSurface, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawX::DuplicateSurface(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDIRECTDRAWSURFACE7 FAR * lplpDupDDSurface)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
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

HRESULT m_IDirectDrawX::EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK2 lpEnumModesCallback)
{
	DDSURFACEDESC2 Desc2;
	ENUMDISPLAYMODES CallbackContext;
	if (lpDDSurfaceDesc && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *(LPDDSURFACEDESC)lpDDSurfaceDesc);

		CallbackContext.lpContext = lpContext;
		CallbackContext.lpCallback = (LPDDENUMMODESCALLBACK)lpEnumModesCallback;

		lpDDSurfaceDesc = &Desc2;
		lpContext = &CallbackContext;
		lpEnumModesCallback = m_IDirectDrawEnumDisplayModes::ConvertCallback;
	}

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->EnumDisplayModes(dwFlags, lpDDSurfaceDesc, lpContext, lpEnumModesCallback);
}

HRESULT m_IDirectDrawX::EnumSurfaces(DWORD dwFlags, LPDDSURFACEDESC2 lpDDSD, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback)
{
	DDSURFACEDESC2 Desc2;
	if (lpDDSD && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *(LPDDSURFACEDESC)lpDDSD);
		lpDDSD = &Desc2;
	}

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = (LPDDENUMSURFACESCALLBACK7)lpEnumSurfacesCallback;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ProxyDirectXVersion = ProxyDirectXVersion;

	return ProxyInterface->EnumSurfaces(dwFlags, lpDDSD, &CallbackContext, m_IDirectDrawEnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawX::FlipToGDISurface()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->FlipToGDISurface();
}

HRESULT m_IDirectDrawX::GetCaps(LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDHELCaps)
{
	DDCAPS DriverCaps, HELCaps;
	DriverCaps.dwSize = sizeof(DriverCaps);
	HELCaps.dwSize = sizeof(HELCaps);

	HRESULT hr;

	if (Config.Dd7to9)
	{
		ZeroMemory(&DriverCaps, sizeof(DDCAPS));
		DriverCaps.dwSize = sizeof(DriverCaps);
		DriverCaps.dwCaps = 0x85d277c1;
		DriverCaps.dwCaps2 = 0xa06ab230;
		DriverCaps.dwCKeyCaps = 0x2210;
		DriverCaps.dwFXCaps = 0x3aad54e0;
		DriverCaps.dwVidMemTotal = 0x8000000;		// Just hard code the memory size
		DriverCaps.dwVidMemFree = 0x7e00000;		// Just hard code the memory size
		DriverCaps.dwMaxVisibleOverlays = 0x1;
		DriverCaps.dwNumFourCCCodes = 0x12;
		DriverCaps.dwRops[6] = 4096;
		DriverCaps.dwMinOverlayStretch = 0x1;
		DriverCaps.dwMaxOverlayStretch = 0x4e20;
		DriverCaps.ddsCaps.dwCaps = 809923324;
		DriverCaps.ddsOldCaps.dwCaps = DriverCaps.ddsCaps.dwCaps;

		ConvertCaps(HELCaps, DriverCaps);

		hr = DD_OK;
	}
	else
	{
		hr = ProxyInterface->GetCaps(lpDDDriverCaps ? &DriverCaps : nullptr, lpDDHELCaps ? &HELCaps : nullptr);
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

	return hr;
}

HRESULT m_IDirectDrawX::GetDisplayMode(LPDDSURFACEDESC2 lpDDSurfaceDesc)
{
	if (!lpDDSurfaceDesc)
	{
		return DDERR_INVALIDPARAMS;
	}

	DDSURFACEDESC2 Desc2;
	if (ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *(LPDDSURFACEDESC)lpDDSurfaceDesc);
		lpDDSurfaceDesc = &Desc2;
	}

	if (Config.Dd7to9)
	{
		if (lpDDSurfaceDesc->dwSize != sizeof(*lpDDSurfaceDesc))
		{
			return DDERR_INVALIDPARAMS;
		}

		lpDDSurfaceDesc->dwWidth = displayModeWidth;
		lpDDSurfaceDesc->dwHeight = displayModeHeight;
		lpDDSurfaceDesc->dwDepth = displayModeBPP;
		lpDDSurfaceDesc->dwRefreshRate = displayModerefreshRate;
		lpDDSurfaceDesc->dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_DEPTH | DDSD_REFRESHRATE;

		return DD_OK;
	}

	return ProxyInterface->GetDisplayMode(lpDDSurfaceDesc);
}

HRESULT m_IDirectDrawX::GetFourCCCodes(LPDWORD lpNumCodes, LPDWORD lpCodes)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetFourCCCodes(lpNumCodes, lpCodes);
}

HRESULT m_IDirectDrawX::GetGDISurface(LPDIRECTDRAWSURFACE7 FAR * lplpGDIDDSSurface)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetMonitorFrequency(lpdwFrequency);
}

HRESULT m_IDirectDrawX::GetScanLine(LPDWORD lpdwScanLine)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetScanLine(lpdwScanLine);
}

HRESULT m_IDirectDrawX::GetVerticalBlankStatus(LPBOOL lpbIsInVB)
{
	if (Config.Dd7to9)
	{
		if (!lpbIsInVB)
		{
			return DDERR_INVALIDPARAMS;
		}

		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetVerticalBlankStatus(lpbIsInVB);
}

HRESULT m_IDirectDrawX::Initialize(GUID FAR * lpGUID)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->Initialize(lpGUID);
}

HRESULT m_IDirectDrawX::RestoreDisplayMode()
{
	if (Config.Dd7to9)
	{
		// Do nothing for windows mode
		if (isWindowed)
		{
			return DD_OK;
		}

		// No exclusive mode
		if (ExclusiveMode)
		{
			return DDERR_NOEXCLUSIVEMODE;
		}

		// Reset screen settings
		std::string lpRamp((3 * 256 * 2), '\0');
		HDC hDC = GetDC(nullptr);
		GetDeviceGammaRamp(hDC, &lpRamp[0]);
		Sleep(0);
		SetDeviceGammaRamp(hDC, &lpRamp[0]);
		ReleaseDC(nullptr, hDC);
		Sleep(0);
		ChangeDisplaySettings(nullptr, 0);

		// Set mode
		ExclusiveMode = false;
		displayModeWidth = 0;
		displayModeHeight = 0;
		displayModeBPP = 0;
		displayModerefreshRate = 0;

		return DD_OK;
	}

	return ProxyInterface->RestoreDisplayMode();
}

// Fixes a bug in ddraw in Windows 8 and 10 where the exclusive flag remains even after the window (hWnd) closes
LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	if (nCode == HCBT_DESTROYWND)
	{
		HWND hWnd = (HWND)wParam;
		IDirectDraw7 *Interface = (IDirectDraw7*)InterlockedExchangePointer((PVOID*)&CurrentDDInterface, nullptr);
		if (Interface && Interface == g_hookmap[hWnd])
		{
			Interface->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
		}
		g_hookmap.erase(hWnd);
	}

	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

HRESULT m_IDirectDrawX::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		if (!hWnd)
		{
			Logging::Log() << __FUNCTION__ << " Unimplemented for NULL window handle";
			return DDERR_GENERIC;
		}

		// Set windowed mode
		if (dwFlags & DDSCL_FULLSCREEN)
		{
			isWindowed = false;
		}
		else if (dwFlags & DDSCL_NORMAL)
		{
			isWindowed = true;
		}
		else
		{
			return DDERR_INVALIDPARAMS;
		}

		// Set ExclusiveMode
		ExclusiveMode = false;
		if (dwFlags & DDSCL_EXCLUSIVE)
		{
			ExclusiveMode = true;
		}

		// Set display window
		MainhWnd = hWnd;

		return DD_OK;
	}

	// Release previouse Exclusive flag
	// Hook window message to get notified when the window is about to exit to remove the exclusive flag
	if ((dwFlags & DDSCL_EXCLUSIVE) && hWnd && hWnd != chWnd)
	{
		if (IsWindow(chWnd))
		{
			ProxyInterface->SetCooperativeLevel(chWnd, DDSCL_NORMAL);
		}
		else
		{
			if (g_hook)
			{
				UnhookWindowsHookEx(g_hook);
				g_hook = nullptr;
			}

			g_hookmap[hWnd] = ProxyInterface;
			g_hook = SetWindowsHookEx(WH_CBT, CBTProc, nullptr, GetWindowThreadProcessId(hWnd, nullptr));
		}

		if (chWnd)
		{
			g_hookmap.erase(chWnd);
		}

		chWnd = hWnd;
	}

	return ProxyInterface->SetCooperativeLevel(hWnd, dwFlags);
}

HRESULT m_IDirectDrawX::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		// Set display mode to dwWidth x dwHeight with dwBPP color depth
		displayModeWidth = dwWidth;
		displayModeHeight = dwHeight;
		displayModeBPP = dwBPP;
		displayModerefreshRate = dwRefreshRate;

		if (SetDefaultDisplayMode)
		{
			displayWidth = dwWidth;
			displayHeight = dwHeight;
		}

		if ((displayWidth != dwWidth || displayHeight != dwHeight) || isWindowed)
		{
			// Adjust the display window to match the current display mode
			AdjustWindow();
		}

		// Ignore color depth

		// Create the requested d3d device for this display mode, report error on failure
		if (!CreateD3DDevice())
		{
			Logging::Log() << __FUNCTION__ << " Error creating Direct3D9 Device";
			return DDERR_GENERIC;
		}

		// Init textures to new display mode
		if (!CreateSurfaceTexture())
		{
			Logging::Log() << __FUNCTION__ << " Error creating Direct3D9 surface texture";
			return DDERR_GENERIC;
		}

		return DD_OK;
	}

	// Force color mode
	if (Config.Force16bitColor)
	{
		dwBPP = 16;
	}
	if (Config.Force32bitColor)
	{
		dwBPP = 32;
	}

	if (ProxyDirectXVersion == 1)
	{
		return ((IDirectDraw*)ProxyInterface)->SetDisplayMode(dwWidth, dwHeight, dwBPP);
	}

	return ProxyInterface->SetDisplayMode(dwWidth, dwHeight, dwBPP, dwRefreshRate, dwFlags);
}

HRESULT m_IDirectDrawX::WaitForVerticalBlank(DWORD dwFlags, HANDLE hEvent)
{
	if (Config.Dd7to9)
	{
		if (dwFlags & DDWAITVB_BLOCKBEGIN)
		{
			// Return when vertical blank begins
		}
		else if (dwFlags & DDWAITVB_BLOCKBEGINEVENT)
		{
			// Triggers an event when the vertical blank begins. This value is not currently supported.
			return DDERR_UNSUPPORTED;
		}
		else if (dwFlags & DDWAITVB_BLOCKEND)
		{
			// Return when the vertical-blank interval ends and the display begins.
		}

		// Vblank supported by vsync so just immediately return
		return DD_OK;
	}

	return ProxyInterface->WaitForVerticalBlank(dwFlags, hEvent);
}

HRESULT m_IDirectDrawX::GetAvailableVidMem(LPDDSCAPS2 lpDDSCaps, LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
	DDSCAPS2 Caps2;
	if (lpDDSCaps && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertCaps(Caps2, *(LPDDSCAPS)lpDDSCaps);
		lpDDSCaps = &Caps2;
	}

	HRESULT hr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		hr = E_NOTIMPL;
	}
	else
	{
		hr = ProxyInterface->GetAvailableVidMem(lpDDSCaps, lpdwTotal, lpdwFree);
	}

	// Set available memory, some games have issues if this is set to high
	if (Config.ConvertToDirectDraw7 && lpdwTotal && lpdwFree && *lpdwTotal > 0x8000000)
	{
		*lpdwFree = 0x8000000 - (*lpdwTotal - *lpdwFree);
		*lpdwTotal = 0x8000000;
	}

	return hr;
}

HRESULT m_IDirectDrawX::GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 * lpDDS)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->RestoreAllSurfaces();
}

HRESULT m_IDirectDrawX::TestCooperativeLevel()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->TestCooperativeLevel();
}

HRESULT m_IDirectDrawX::GetDeviceIdentifier(LPDDDEVICEIDENTIFIER2 lpdddi, DWORD dwFlags)
{
	LPDDDEVICEIDENTIFIER2 lpdddi_tmp = lpdddi;
	DDDEVICEIDENTIFIER2 Id2 = {};
	if (lpdddi && ProxyDirectXVersion == 7 && DirectXVersion != 7)
	{
		lpdddi = &Id2;
	}

	HRESULT hr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		hr = E_NOTIMPL;
	}
	else
	{
		hr = ProxyInterface->GetDeviceIdentifier(lpdddi, dwFlags);
	}

	if (SUCCEEDED(hr) && lpdddi_tmp && ProxyDirectXVersion == 7 && DirectXVersion != 7)
	{
		lpdddi = lpdddi_tmp;
		ConvertDeviceIdentifier(*(LPDDDEVICEIDENTIFIER)lpdddi, Id2);
	}

	return hr;
}

HRESULT m_IDirectDrawX::StartModeTest(LPSIZE lpModesToTest, DWORD dwNumEntries, DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->StartModeTest(lpModesToTest, dwNumEntries, dwFlags);
}

HRESULT m_IDirectDrawX::EvaluateMode(DWORD dwFlags, DWORD * pSecondsUntilTimeout)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ProxyInterface->EvaluateMode(dwFlags, pSecondsUntilTimeout);
}

// Helper function for adjusting the window position
void m_IDirectDrawX::AdjustWindow()
{
	// If we don't yet have the window quit without action
	if (!MainhWnd)
	{
		return;
	}

	// window mode
	if (isWindowed)
	{
		// Window with border/caption
		SetWindowLong(MainhWnd, GWL_STYLE, WS_VISIBLE | WS_CAPTION);
		// Set window size
		SetWindowPos(MainhWnd, nullptr, 0, 0, displayWidth, displayHeight, SWP_NOMOVE | SWP_NOZORDER);
		// Adjust for window decoration to ensure client area matches display size
		RECT tempRect;
		GetClientRect(MainhWnd, &tempRect);
		tempRect.right = (displayWidth - tempRect.right) + displayWidth;
		tempRect.bottom = (displayHeight - tempRect.bottom) + displayHeight;
		// Move window to last position and adjust size
		SetWindowPos(MainhWnd, nullptr, lastPosition.x, lastPosition.y, tempRect.right, tempRect.bottom, SWP_NOZORDER);
	}
	else
	{
		// Window borderless and fullscreen size
		SetWindowLong(MainhWnd, GWL_STYLE, WS_VISIBLE);
		// Set full size
		SetWindowPos(MainhWnd, nullptr, 0, 0, displayWidth, displayHeight, SWP_NOZORDER);
	}
}

// Helper function to create/recreate D3D device
bool m_IDirectDrawX::CreateD3DDevice()
{
	// Release existing vertex buffer
	if (vertexBuffer)
	{
		vertexBuffer->Release();
		vertexBuffer = nullptr;
	}

	// Release existing surface texture
	if (surfaceTexture)
	{
		surfaceTexture->Release();
		surfaceTexture = nullptr;
	}

	// Release existing d3d9device
	if (d3d9Device)
	{
		if (d3d9Device->Release() != 0)
		{
			Logging::Log() << __FUNCTION__ << " Unable to release Direct3D9 device";
			return false;
		}
		d3d9Device = nullptr;
	}

	// Check device caps to make sure it supports dynamic textures
	D3DCAPS9 d3dcaps;
	ZeroMemory(&d3dcaps, sizeof(D3DCAPS9));
	if (d3d9Object->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dcaps) != D3D_OK)
	{
		Logging::Log() << __FUNCTION__ << " Unable to retrieve device-specific information about the device";
		return false;
	}

	// Is dynamic textures flag set?
	if ((d3dcaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES) == 0)
	{
		// No dynamic textures
		Logging::Log() << __FUNCTION__ << " Device does not support dynamic textures";
		return false;
	}

	ZeroMemory(&presParams, sizeof(presParams));
	// Set display window
	presParams.hDeviceWindow = MainhWnd;

	// Enumerate modes for format XRGB
	UINT modeCount = d3d9Object->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);

	D3DDISPLAYMODE d3ddispmode;
	D3DDISPLAYMODE set_d3ddispmode = { NULL };
	bool modeFound = false;
	// Loop through all modes looking for our requested resolution
	for (UINT i = 0; i < modeCount; i++)
	{
		// Get display modes here
		ZeroMemory(&d3ddispmode, sizeof(D3DDISPLAYMODE));
		if (d3d9Object->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, i, &d3ddispmode) != D3D_OK)
		{
			Logging::Log() << __FUNCTION__ << " EnumAdapterModes failed";
			return false;
		}
		if (d3ddispmode.Width == displayWidth && d3ddispmode.Height == displayHeight &&				// Check height and width
			(d3ddispmode.RefreshRate == displayModerefreshRate || displayModerefreshRate == 0) &&	// Check refresh rate
			!(d3ddispmode.Width > 1920 && d3ddispmode.Height > 1440))								// No modes above maximum size
		{
			// Found a match
			modeFound = true;
			memcpy(&set_d3ddispmode, &d3ddispmode, sizeof(D3DDISPLAYMODE));
		}
	}

	// Set parameters for the current display mode
	if (isWindowed)
	{
		// Window mode
		presParams.Windowed = TRUE;
		// Swap discard
		presParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
		// Unknown format
		presParams.BackBufferFormat = D3DFMT_UNKNOWN;
		// Interval level
		presParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}
	else
	{
		// No mode found
		if (!modeFound)
		{
			Logging::Log() << __FUNCTION__ << " Failed to find compatible fullscreen display mode";
			return false;
		}
		// Fullscreen
		presParams.Windowed = FALSE;
		// width/height
		presParams.BackBufferWidth = set_d3ddispmode.Width;
		presParams.BackBufferHeight = set_d3ddispmode.Height;
		// Discard swap
		presParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
		// Display mode fullscreen format
		presParams.BackBufferFormat = set_d3ddispmode.Format;
		// Display mode refresh
		presParams.FullScreen_RefreshRateInHz = set_d3ddispmode.RefreshRate;
		// Interval level
		presParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	// D3DCREATE_NOWINDOWCHANGES possible for alt+tab and mouse leaving window
	// create d3d device with hardware vertex processing if it's available
	if (d3dcaps.VertexProcessingCaps != 0)
	{
		if (d3d9Object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, MainhWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &presParams, &d3d9Device) != D3D_OK)
		{
			Logging::Log() << __FUNCTION__ << " Failed to create Direct3D9 device";
			return false;
		}
	}
	else
	{
		if (d3d9Object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, MainhWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presParams, &d3d9Device) != D3D_OK)
		{
			Logging::Log() << __FUNCTION__ << " Failed to create Direct3D9 device";
			return false;
		}
	}

	// Success
	return true;
}

// Helper function to create surface texture
bool m_IDirectDrawX::CreateSurfaceTexture()
{
	// Release existing vertex buffer
	if (vertexBuffer)
	{
		vertexBuffer->Release();
		vertexBuffer = nullptr;
	}

	// Release existing surface texture
	if (surfaceTexture)
	{
		surfaceTexture->Release();
		surfaceTexture = nullptr;
	}

	// Create managed dynamic texture to allow locking(debug as video size but change to display size)
	if (d3d9Device->CreateTexture(displayModeWidth, displayModeHeight, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &surfaceTexture, nullptr) != D3D_OK)
	{
		Logging::Log() << __FUNCTION__ << " Unable to create surface texture";
		return false;
	}

	// Set vertex shader
	if (d3d9Device->SetVertexShader(nullptr) != D3D_OK)
	{
		Logging::Log() << __FUNCTION__ << " Unable to set vertex shader";
		return false;
	}

	// Set fv format
	if (d3d9Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1) != D3D_OK)
	{
		Logging::Log() << __FUNCTION__ << " Unable to set the current vertex stream format";
		return false;
	}

	// Create vertex buffer
	if (d3d9Device->CreateVertexBuffer(sizeof(TLVERTEX) * 4, 0, (D3DFVF_XYZRHW | D3DFVF_TEX1), D3DPOOL_MANAGED, &vertexBuffer, nullptr) != D3D_OK)
	{
		Logging::Log() << __FUNCTION__ << " Unable to create vertex buffer";
		return false;
	}

	// Set stream source
	if (d3d9Device->SetStreamSource(0, vertexBuffer, 0, sizeof(TLVERTEX)) != D3D_OK)
	{
		Logging::Log() << __FUNCTION__ << " Unable to set vertex buffer stream source";
		return false;
	}

	// Set render states(no lighting)
	if (d3d9Device->SetRenderState(D3DRS_LIGHTING, FALSE) != D3D_OK)
	{
		Logging::Log() << __FUNCTION__ << " Unable to set device render state(no lighting)";
		return false;
	}

	// Query the device to see if it supports anything other than point filtering
	if (d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_QUERY_FILTER, D3DRTYPE_TEXTURE, D3DFMT_X8R8G8B8) == D3D_OK)
	{
		// Set scale mode to linear
		if (d3d9Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR) != DD_OK)
		{
			Logging::Log() << __FUNCTION__ << " Failed to set D3D device to LINEAR sampling";
			return false;
		}
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Device doesn't support linear sampling";
	}

	// Setup verticies (0,0,currentWidth,currentHeight)
	TLVERTEX* vertices;
	// Lock
	if (vertexBuffer->Lock(0, 0, (void**)&vertices, 0) != D3D_OK)
	{
		Logging::Log() << __FUNCTION__ << " Unable to lock vertex buffer";
		return false;
	}

	// Set vertex points
	// 0, 0
	vertices[0].x = -0.5f;
	vertices[0].y = -0.5f;
	vertices[0].z = 0.0f;
	vertices[0].rhw = 1.0f;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;

	// currentWidth, 0
	vertices[1].x = (float)displayWidth - 0.5f;
	vertices[1].y = -0.5f;
	vertices[1].z = 0.0f;
	vertices[1].rhw = 1.0f;
	vertices[1].u = 1.0f;
	vertices[1].v = 0.0f;

	// currentWidth, scaledHeight
	vertices[2].x = (float)displayWidth - 0.5f;
	vertices[2].y = (float)displayHeight - 0.5f;
	vertices[2].z = 0.0f;
	vertices[2].rhw = 1.0f;
	vertices[2].u = 1.0f;
	vertices[2].v = 1.0f;

	// 0, currentHeight
	vertices[3].x = -0.5f;
	vertices[3].y = (float)displayHeight - 0.5f;
	vertices[3].z = 0.0f;
	vertices[3].rhw = 1.0f;
	vertices[3].u = 0.0f;
	vertices[3].v = 1.0f;

	// Unlcok vertex buffer
	if (vertexBuffer->Unlock() != D3D_OK)
	{
		Logging::Log() << __FUNCTION__ << " Unable to unlock vertex buffer";
		return false;
	}

	return true;
}

// Helper function to reinitialize device
bool m_IDirectDrawX::ReinitDevice()
{
	// Release existing vertex buffer
	if (vertexBuffer)
	{
		vertexBuffer->Release();
		vertexBuffer = nullptr;
	}

	// Release existing surface texture
	if (surfaceTexture)
	{
		surfaceTexture->Release();
		surfaceTexture = nullptr;
	}

	// Attempt to reset the device
	if (d3d9Device->Reset(&presParams) != D3D_OK)
	{
		Logging::Log() << __FUNCTION__ << " Failed to reset Direct3D9 device";
		return false;
	}

	// Recreate the surface texutre
	return CreateSurfaceTexture();
}

// Helper function to present the d3d surface
HRESULT m_IDirectDrawX::Lock(D3DLOCKED_RECT *d3dlrect)
{
	// Make sure the device exists
	if (!d3d9Device)
	{
		Logging::Log() << __FUNCTION__ << " called when d3d9device doesn't exist";
		return DDERR_GENERIC;
	}

	// Make sure surface texture exists
	if (!surfaceTexture)
	{
		Logging::Log() << __FUNCTION__ << " called when texture doesn't exist";
		return DDERR_SURFACELOST;
	}

	// Lock full dynamic texture
	HRESULT hr;
	hr = surfaceTexture->LockRect(0, d3dlrect, nullptr, D3DLOCK_DISCARD);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Failed to lock texture memory";
		return hr;
	}

	return DD_OK;
}

// Helper function to present the d3d surface
HRESULT m_IDirectDrawX::Unlock()
{
	// Make sure the device exists
	if (!d3d9Device)
	{
		Logging::Log() << __FUNCTION__ << " called when d3d9device doesn't exist";
		return DDERR_INVALIDOBJECT;
	}

	// Make sure surface texture exists
	if (!surfaceTexture)
	{
		Logging::Log() << __FUNCTION__ << " called when texture doesn't exist";
		return DDERR_SURFACELOST;
	}

	// Unlock dynamic texture
	HRESULT hr;
	hr = surfaceTexture->UnlockRect(0);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Failed to unlock texture memory";
		return hr;
	}

	hr = d3d9Device->Clear(0, nullptr, D3DCLEAR_TARGET, 0xFF000000, 1.0f, 0);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Failed to clear device";
		return hr;
	}

	hr = d3d9Device->BeginScene();
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Failed to begin scene";
		return hr;
	}

	// Set texture
	hr = d3d9Device->SetTexture(0, surfaceTexture);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Failed to set texture";
		return hr;
	}

	// Draw primitive
	hr = d3d9Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Failed to draw primitive";
		return hr;
	}

	// And... End scene
	hr = d3d9Device->EndScene();
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Failed to end scene";
		return hr;
	}

	// Present everthing!
	hr = d3d9Device->Present(nullptr, nullptr, nullptr, nullptr);

	// Device lost
	if (hr == D3DERR_DEVICELOST)
	{
		Logging::Log() << __FUNCTION__ << " Device lost";
		// Attempt to reinit device
		return ReinitDevice();
	}
	else if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Failed to present scene";
		return hr;
	}

	return DD_OK;
}
