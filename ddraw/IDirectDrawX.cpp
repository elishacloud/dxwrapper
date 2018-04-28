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
*/

#include "ddraw.h"

HRESULT m_IDirectDrawX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, WrapperInterface);
}

ULONG m_IDirectDrawX::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawX::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		WrapperInterface->DeleteMe();
	}

	return x;
}

HRESULT m_IDirectDrawX::Compact()
{
	return ProxyInterface->Compact();
}

HRESULT m_IDirectDrawX::CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR * lplpDDClipper, IUnknown FAR * pUnkOuter)
{
	HRESULT hr = ProxyInterface->CreateClipper(dwFlags, lplpDDClipper, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDClipper)
	{
		*lplpDDClipper = ProxyAddressLookupTable.FindAddress<m_IDirectDrawClipper>(*lplpDDClipper);
	}

	return hr;
}

HRESULT m_IDirectDrawX::CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, LPDIRECTDRAWPALETTE FAR * lplpDDPalette, IUnknown FAR * pUnkOuter)
{
	HRESULT hr = ProxyInterface->CreatePalette(dwFlags, lpDDColorArray, lplpDDPalette, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDPalette)
	{
		*lplpDDPalette = ProxyAddressLookupTable.FindAddress<m_IDirectDrawPalette>(*lplpDDPalette);
	}

	return hr;
}

template HRESULT m_IDirectDrawX::CreateSurface<LPDDSURFACEDESC>(LPDDSURFACEDESC, LPDIRECTDRAWSURFACE7 FAR *, IUnknown FAR *);
template HRESULT m_IDirectDrawX::CreateSurface<LPDDSURFACEDESC2>(LPDDSURFACEDESC2, LPDIRECTDRAWSURFACE7 FAR *, IUnknown FAR *);
template <typename T>
HRESULT m_IDirectDrawX::CreateSurface(T lpDDSurfaceDesc, LPDIRECTDRAWSURFACE7 FAR * lplpDDSurface, IUnknown FAR * pUnkOuter)
{
	DDSURFACEDESC2 Desc2;
	if (lpDDSurfaceDesc && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);
		lpDDSurfaceDesc = (T)&Desc2;

		// BackBufferCount must be at least 1
		lpDDSurfaceDesc->dwBackBufferCount = (lpDDSurfaceDesc->dwBackBufferCount) ? lpDDSurfaceDesc->dwBackBufferCount : 1;
	}

	HRESULT hr = ProxyInterface->CreateSurface((LPDDSURFACEDESC2)lpDDSurfaceDesc, lplpDDSurface, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDSurface)
	{
		*lplpDDSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpDDSurface, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawX::DuplicateSurface(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDIRECTDRAWSURFACE7 FAR * lplpDupDDSurface)
{
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

template HRESULT m_IDirectDrawX::EnumDisplayModes<LPDDSURFACEDESC>(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK);
template HRESULT m_IDirectDrawX::EnumDisplayModes<LPDDSURFACEDESC2>(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2);
template <typename T, typename D>
HRESULT m_IDirectDrawX::EnumDisplayModes(DWORD dwFlags, T lpDDSurfaceDesc, LPVOID lpContext, D lpEnumModesCallback)
{
	DDSURFACEDESC2 Desc2;
	ENUMDISPLAYMODES CallbackContext;
	if (lpDDSurfaceDesc && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);

		CallbackContext.lpContext = lpContext;
		CallbackContext.lpCallback = (LPDDENUMMODESCALLBACK)lpEnumModesCallback;

		lpDDSurfaceDesc = (T)&Desc2;
		lpContext = &CallbackContext;
		lpEnumModesCallback = (D)m_IDirectDrawEnumDisplayModes::ConvertCallback;
	}

	return ProxyInterface->EnumDisplayModes(dwFlags, (LPDDSURFACEDESC2)lpDDSurfaceDesc, lpContext, (LPDDENUMMODESCALLBACK2)lpEnumModesCallback);
}

template HRESULT m_IDirectDrawX::EnumSurfaces<LPDDSURFACEDESC>(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMSURFACESCALLBACK);
template HRESULT m_IDirectDrawX::EnumSurfaces<LPDDSURFACEDESC2>(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMSURFACESCALLBACK2);
template HRESULT m_IDirectDrawX::EnumSurfaces<LPDDSURFACEDESC2>(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMSURFACESCALLBACK7);
template <typename T, typename D>
HRESULT m_IDirectDrawX::EnumSurfaces(DWORD dwFlags, T lpDDSD, LPVOID lpContext, D lpEnumSurfacesCallback)
{
	DDSURFACEDESC2 Desc2;
	if (lpDDSD && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *lpDDSD);
		lpDDSD = (T)&Desc2;
	}

	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = (LPDDENUMSURFACESCALLBACK7)lpEnumSurfacesCallback;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ProxyDirectXVersion = ProxyDirectXVersion;

	return ProxyInterface->EnumSurfaces(dwFlags, (LPDDSURFACEDESC2)lpDDSD, &CallbackContext, m_IDirectDrawEnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawX::FlipToGDISurface()
{
	return ProxyInterface->FlipToGDISurface();
}

HRESULT m_IDirectDrawX::GetCaps(LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDHELCaps)
{
	DDCAPS DriverCaps, HELCaps;
	DriverCaps.dwSize = sizeof(DriverCaps);
	HELCaps.dwSize = sizeof(HELCaps);

	HRESULT hr = ProxyInterface->GetCaps(lpDDDriverCaps ? &DriverCaps : nullptr, lpDDHELCaps ? &HELCaps : nullptr);

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

template HRESULT m_IDirectDrawX::GetDisplayMode<LPDDSURFACEDESC>(LPDDSURFACEDESC);
template HRESULT m_IDirectDrawX::GetDisplayMode<LPDDSURFACEDESC2>(LPDDSURFACEDESC2);
template <typename T>
HRESULT m_IDirectDrawX::GetDisplayMode(T lpDDSurfaceDesc)
{
	DDSURFACEDESC2 Desc2;
	if (lpDDSurfaceDesc && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);
		lpDDSurfaceDesc = (T)&Desc2;
	}

	return ProxyInterface->GetDisplayMode((LPDDSURFACEDESC2)lpDDSurfaceDesc);
}

HRESULT m_IDirectDrawX::GetFourCCCodes(LPDWORD lpNumCodes, LPDWORD lpCodes)
{
	return ProxyInterface->GetFourCCCodes(lpNumCodes, lpCodes);
}

HRESULT m_IDirectDrawX::GetGDISurface(LPDIRECTDRAWSURFACE7 FAR * lplpGDIDDSSurface)
{
	HRESULT hr = ProxyInterface->GetGDISurface(lplpGDIDDSSurface);

	if (SUCCEEDED(hr) && lplpGDIDDSSurface)
	{
		*lplpGDIDDSSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpGDIDDSSurface, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawX::GetMonitorFrequency(LPDWORD lpdwFrequency)
{
	return ProxyInterface->GetMonitorFrequency(lpdwFrequency);
}

HRESULT m_IDirectDrawX::GetScanLine(LPDWORD lpdwScanLine)
{
	return ProxyInterface->GetScanLine(lpdwScanLine);
}

HRESULT m_IDirectDrawX::GetVerticalBlankStatus(LPBOOL lpbIsInVB)
{
	return ProxyInterface->GetVerticalBlankStatus(lpbIsInVB);
}

HRESULT m_IDirectDrawX::Initialize(GUID FAR * lpGUID)
{
	return ProxyInterface->Initialize(lpGUID);
}

HRESULT m_IDirectDrawX::RestoreDisplayMode()
{
	return ProxyInterface->RestoreDisplayMode();
}

HRESULT m_IDirectDrawX::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
	// Set ModeEx to fix issues with some games like Star Trek Armada 1 & 2
	if (IsDDrawCreateEx && (dwFlags & DDSCL_EXCLUSIVE) && !(dwFlags & DDSCL_ALLOWMODEX))
	{
		dwFlags |= DDSCL_ALLOWMODEX | DDSCL_FULLSCREEN;
	}

	return ProxyInterface->SetCooperativeLevel(hWnd, dwFlags);
}

HRESULT m_IDirectDrawX::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags)
{
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
	return ProxyInterface->WaitForVerticalBlank(dwFlags, hEvent);
}

template HRESULT m_IDirectDrawX::GetAvailableVidMem<LPDDSCAPS>(LPDDSCAPS, LPDWORD, LPDWORD);
template HRESULT m_IDirectDrawX::GetAvailableVidMem<LPDDSCAPS2>(LPDDSCAPS2, LPDWORD, LPDWORD);
template <typename T>
HRESULT m_IDirectDrawX::GetAvailableVidMem(T lpDDSCaps, LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
	DDSCAPS2 Caps2;
	if (lpDDSCaps && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertCaps(Caps2, *lpDDSCaps);
		lpDDSCaps = (T)&Caps2;
	}

	// Set available memory, some games have issues if this is set to high
	if (Config.ConvertToDirectDraw7 && lpdwTotal && lpdwFree && *lpdwTotal > 0x8000000)
	{
		*lpdwFree = 0x8000000 - (*lpdwTotal - *lpdwFree);
		*lpdwTotal = 0x8000000;
	}

	return ProxyInterface->GetAvailableVidMem((LPDDSCAPS2)lpDDSCaps, lpdwTotal, lpdwFree);
}

HRESULT m_IDirectDrawX::GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 * lpDDS)
{
	HRESULT hr = ProxyInterface->GetSurfaceFromDC(hdc, lpDDS);

	if (SUCCEEDED(hr) && lpDDS)
	{
		*lpDDS = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lpDDS, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawX::RestoreAllSurfaces()
{
	return ProxyInterface->RestoreAllSurfaces();
}

HRESULT m_IDirectDrawX::TestCooperativeLevel()
{
	return ProxyInterface->TestCooperativeLevel();
}

template HRESULT m_IDirectDrawX::GetDeviceIdentifier<LPDDDEVICEIDENTIFIER>(LPDDDEVICEIDENTIFIER, DWORD);
template HRESULT m_IDirectDrawX::GetDeviceIdentifier<LPDDDEVICEIDENTIFIER2>(LPDDDEVICEIDENTIFIER2, DWORD);
template <typename T>
HRESULT m_IDirectDrawX::GetDeviceIdentifier(T lpdddi, DWORD dwFlags)
{
	T lpdddi_tmp = lpdddi;
	DDDEVICEIDENTIFIER2 Id2 = {};
	if (lpdddi && ProxyDirectXVersion == 7 && DirectXVersion != 7)
	{
		lpdddi = (T)&Id2;
	}

	HRESULT hr = ProxyInterface->GetDeviceIdentifier((LPDDDEVICEIDENTIFIER2)lpdddi, dwFlags);

	if (SUCCEEDED(hr) && lpdddi_tmp && ProxyDirectXVersion == 7 && DirectXVersion != 7)
	{
		lpdddi = lpdddi_tmp;
		ConvertDeviceIdentifier(*lpdddi, Id2);
	}

	return hr;
}

HRESULT m_IDirectDrawX::StartModeTest(LPSIZE lpModesToTest, DWORD dwNumEntries, DWORD dwFlags)
{
	return ProxyInterface->StartModeTest(lpModesToTest, dwNumEntries, dwFlags);
}

HRESULT m_IDirectDrawX::EvaluateMode(DWORD dwFlags, DWORD * pSecondsUntilTimeout)
{
	return ProxyInterface->EvaluateMode(dwFlags, pSecondsUntilTimeout);
}
