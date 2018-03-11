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
	HRESULT hr = ProxyInterface->QueryInterface(ConvertREFIID(riid), ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectDrawX::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawX::Release()
{
	return ProxyInterface->Release();
}

HRESULT m_IDirectDrawX::Compact()
{
	return ProxyInterface->Compact();
}

HRESULT m_IDirectDrawX::CreateClipper(DWORD a, LPDIRECTDRAWCLIPPER FAR * b, IUnknown FAR * c)
{
	HRESULT hr = ProxyInterface->CreateClipper(a, b, c);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawClipper>(*b);
	}

	return hr;
}

HRESULT m_IDirectDrawX::CreatePalette(DWORD a, LPPALETTEENTRY b, LPDIRECTDRAWPALETTE FAR * c, IUnknown FAR * d)
{
	HRESULT hr = ProxyInterface->CreatePalette(a, b, c, d);

	if (SUCCEEDED(hr))
	{
		*c = ProxyAddressLookupTable.FindAddress<m_IDirectDrawPalette>(*c);
	}

	return hr;
}

template HRESULT m_IDirectDrawX::CreateSurface<LPDDSURFACEDESC>(LPDDSURFACEDESC a, LPDIRECTDRAWSURFACE7 FAR * b, IUnknown FAR * c);
template HRESULT m_IDirectDrawX::CreateSurface<LPDDSURFACEDESC2>(LPDDSURFACEDESC2 a, LPDIRECTDRAWSURFACE7 FAR * b, IUnknown FAR * c);
template <typename T>
HRESULT m_IDirectDrawX::CreateSurface(T a, LPDIRECTDRAWSURFACE7 FAR * b, IUnknown FAR * c)
{
	LPDDSURFACEDESC2 pDesc2 = (LPDDSURFACEDESC2)a;
	DDSURFACEDESC2 Desc;
	if (a != nullptr && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc, *a);
		pDesc2 = &Desc;
	}

	HRESULT hr = ProxyInterface->CreateSurface(pDesc2, b, c);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*b, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawX::DuplicateSurface(LPDIRECTDRAWSURFACE7 a, LPDIRECTDRAWSURFACE7 FAR * b)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawSurface7 *>(a)->GetProxyInterface();
	}

	HRESULT hr = ProxyInterface->DuplicateSurface(a, b);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*b, ((m_IDirectDrawSurface7 *)a)->GetDirectXVersion());
	}

	return hr;
}

template HRESULT m_IDirectDrawX::EnumDisplayModes<LPDDSURFACEDESC>(DWORD a, LPDDSURFACEDESC b, LPVOID c, LPDDENUMMODESCALLBACK d);
template HRESULT m_IDirectDrawX::EnumDisplayModes<LPDDSURFACEDESC2>(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMMODESCALLBACK2 d);
template <typename T, typename D>
HRESULT m_IDirectDrawX::EnumDisplayModes(DWORD a, T b, LPVOID c, D d)
{
	DDSURFACEDESC2 Desc2;
	if (b != nullptr && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *b);
		b = (T)&Desc2;
	}

	m_IDirectDrawEnumDisplayModes::SetCallback((LPDDENUMMODESCALLBACK2)d, DirectXVersion, ProxyDirectXVersion);

	HRESULT hr = ProxyInterface->EnumDisplayModes(a, (LPDDSURFACEDESC2)b, c, m_IDirectDrawEnumDisplayModes::ConvertCallback);

	m_IDirectDrawEnumDisplayModes::ReleaseCallback();

	return hr;
}

template HRESULT m_IDirectDrawX::EnumSurfaces<LPDDSURFACEDESC>(DWORD a, LPDDSURFACEDESC b, LPVOID c, LPDDENUMSURFACESCALLBACK d);
template HRESULT m_IDirectDrawX::EnumSurfaces<LPDDSURFACEDESC2>(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMSURFACESCALLBACK2 d);
template HRESULT m_IDirectDrawX::EnumSurfaces<LPDDSURFACEDESC2>(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMSURFACESCALLBACK7 d);
template <typename T, typename D>
HRESULT m_IDirectDrawX::EnumSurfaces(DWORD a, T b, LPVOID c, D d)
{
	DDSURFACEDESC2 Desc2;
	if (b != nullptr && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *b);
		b = (T)&Desc2;
	}

	m_IDirectDrawEnumSurface::SetCallback((LPDDENUMSURFACESCALLBACK7)d, DirectXVersion, ProxyDirectXVersion);

	HRESULT hr = ProxyInterface->EnumSurfaces(a, (LPDDSURFACEDESC2)b, c, m_IDirectDrawEnumSurface::ConvertCallback);

	m_IDirectDrawEnumSurface::ReleaseCallback();

	return hr;
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

	if (lpDDDriverCaps && !ConvertCaps(*lpDDDriverCaps, DriverCaps))
	{
		Logging::Log() << __FUNCTION__ << " Failed to convert Caps!";
		hr = E_INVALIDARG;
	}

	if (lpDDHELCaps && !ConvertCaps(*lpDDHELCaps, HELCaps))
	{
		Logging::Log() << __FUNCTION__ << " Failed to convert Caps!";
		hr = E_INVALIDARG;
	}

	return hr;
}

template HRESULT m_IDirectDrawX::GetDisplayMode<LPDDSURFACEDESC>(LPDDSURFACEDESC a);
template HRESULT m_IDirectDrawX::GetDisplayMode<LPDDSURFACEDESC2>(LPDDSURFACEDESC2 a);
template <typename T>
HRESULT m_IDirectDrawX::GetDisplayMode(T a)
{
	DDSURFACEDESC2 Desc2;
	if (a != nullptr && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *a);
		a = (T)&Desc2;
	}

	return ProxyInterface->GetDisplayMode((LPDDSURFACEDESC2)a);
}

HRESULT m_IDirectDrawX::GetFourCCCodes(LPDWORD a, LPDWORD b)
{
	return ProxyInterface->GetFourCCCodes(a, b);
}

HRESULT m_IDirectDrawX::GetGDISurface(LPDIRECTDRAWSURFACE7 FAR * a)
{
	HRESULT hr = ProxyInterface->GetGDISurface(a);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*a, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawX::GetMonitorFrequency(LPDWORD a)
{
	return ProxyInterface->GetMonitorFrequency(a);
}

HRESULT m_IDirectDrawX::GetScanLine(LPDWORD a)
{
	return ProxyInterface->GetScanLine(a);
}

HRESULT m_IDirectDrawX::GetVerticalBlankStatus(LPBOOL a)
{
	return ProxyInterface->GetVerticalBlankStatus(a);
}

HRESULT m_IDirectDrawX::Initialize(GUID FAR * a)
{
	return ProxyInterface->Initialize(a);
}

HRESULT m_IDirectDrawX::RestoreDisplayMode()
{
	return ProxyInterface->RestoreDisplayMode();
}

HRESULT m_IDirectDrawX::SetCooperativeLevel(HWND a, DWORD b)
{
	// Star Trek Armada 1 fix
	if (Config.ArmadaFix && (b & DDSCL_EXCLUSIVE) && !(b & DDSCL_ALLOWMODEX))
	{
		Logging::Log() << "SetCooperativeLevel: Enabling ModeEx";
		b |= DDSCL_ALLOWMODEX | DDSCL_FULLSCREEN;
	}

	HRESULT hr = ProxyInterface->SetCooperativeLevel(a, b);

	return hr;
}

HRESULT m_IDirectDrawX::SetDisplayMode(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e)
{
	// Star Trek Armada 1 fix
	if (Config.ArmadaFix && c != 32)
	{
		Logging::Log() << "SetDisplayMode: Setting 32bit color";
		c = 32;
	}

	if (ProxyDirectXVersion == 1)
	{
		return ((IDirectDraw*)ProxyInterface)->SetDisplayMode(a, b, c);
	}

	return ProxyInterface->SetDisplayMode(a, b, c, d, e);
}

HRESULT m_IDirectDrawX::WaitForVerticalBlank(DWORD a, HANDLE b)
{
	return ProxyInterface->WaitForVerticalBlank(a, b);
}

template HRESULT m_IDirectDrawX::GetAvailableVidMem<LPDDSCAPS>(LPDDSCAPS a, LPDWORD b, LPDWORD c);
template HRESULT m_IDirectDrawX::GetAvailableVidMem<LPDDSCAPS2>(LPDDSCAPS2 a, LPDWORD b, LPDWORD c);
template <typename T>
HRESULT m_IDirectDrawX::GetAvailableVidMem(T a, LPDWORD b, LPDWORD c)
{
	DDSCAPS2 Caps2;
	if (a != nullptr && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertCaps(Caps2, *a);
		a = (T)&Caps2;
	}

	return ProxyInterface->GetAvailableVidMem((LPDDSCAPS2)a, b, c);
}

HRESULT m_IDirectDrawX::GetSurfaceFromDC(HDC a, LPDIRECTDRAWSURFACE7 * b)
{
	HRESULT hr = ProxyInterface->GetSurfaceFromDC(a, b);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*b, DirectXVersion);
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

template HRESULT m_IDirectDrawX::GetDeviceIdentifier<LPDDDEVICEIDENTIFIER>(LPDDDEVICEIDENTIFIER lpdddi, DWORD dwFlags);
template HRESULT m_IDirectDrawX::GetDeviceIdentifier<LPDDDEVICEIDENTIFIER2>(LPDDDEVICEIDENTIFIER2 lpdddi, DWORD dwFlags);
template <typename T>
HRESULT m_IDirectDrawX::GetDeviceIdentifier(T lpdddi, DWORD dwFlags)
{
	T lpdddi_tmp = lpdddi;
	DDDEVICEIDENTIFIER2 Id2 = {};
	if (lpdddi != nullptr && ProxyDirectXVersion == 7 && DirectXVersion != 7)
	{
		lpdddi = (T)&Id2;
	}

	HRESULT hr = ProxyInterface->GetDeviceIdentifier((LPDDDEVICEIDENTIFIER2)lpdddi, dwFlags);

	if (SUCCEEDED(hr) && ProxyDirectXVersion == 7 && DirectXVersion != 7)
	{
		ConvertDeviceIdentifier(*lpdddi_tmp, Id2);
	}

	return hr;
}

HRESULT m_IDirectDrawX::StartModeTest(LPSIZE a, DWORD b, DWORD c)
{
	return ProxyInterface->StartModeTest(a, b, c);
}

HRESULT m_IDirectDrawX::EvaluateMode(DWORD a, DWORD * b)
{
	return ProxyInterface->EvaluateMode(a, b);
}
