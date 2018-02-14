/**
* Copyright (C) 2017 Elisha Riedlinger
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

HRESULT m_IDirectDraw4::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectDraw4::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDraw4::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		delete this;
	}

	return x;
}

HRESULT m_IDirectDraw4::Compact()
{
	return ProxyInterface->Compact();
}

HRESULT m_IDirectDraw4::CreateClipper(DWORD a, LPDIRECTDRAWCLIPPER FAR * b, IUnknown FAR * c)
{
	HRESULT hr = ProxyInterface->CreateClipper(a, b, c);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawClipper>(*b);
	}

	return hr;
}

HRESULT m_IDirectDraw4::CreatePalette(DWORD a, LPPALETTEENTRY b, LPDIRECTDRAWPALETTE FAR * c, IUnknown FAR * d)
{
	HRESULT hr = ProxyInterface->CreatePalette(a, b, c, d);

	if (SUCCEEDED(hr))
	{
		*c = ProxyAddressLookupTable.FindAddress<m_IDirectDrawPalette>(*c);
	}

	return hr;
}

HRESULT m_IDirectDraw4::CreateSurface(LPDDSURFACEDESC2 a, LPDIRECTDRAWSURFACE4 FAR * b, IUnknown FAR * c)
{
	HRESULT hr = ProxyInterface->CreateSurface(a, b, c);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface4>(*b);
	}

	return hr;
}

HRESULT m_IDirectDraw4::DuplicateSurface(LPDIRECTDRAWSURFACE4 a, LPDIRECTDRAWSURFACE4 FAR * b)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawSurface4 *>(a)->GetProxyInterface();
	}

	HRESULT hr = ProxyInterface->DuplicateSurface(a, b);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface4>(*b);
	}

	return hr;
}

HRESULT m_IDirectDraw4::EnumDisplayModes(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMMODESCALLBACK2 d)
{
	return ProxyInterface->EnumDisplayModes(a, b, c, d);
}

HRESULT m_IDirectDraw4::EnumSurfaces(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMSURFACESCALLBACK2 d)
{
	m_IDirectDrawEnumSurface2::SetCallback(d);

	HRESULT hr = ProxyInterface->EnumSurfaces(a, b, c, reinterpret_cast<LPDDENUMSURFACESCALLBACK2>(m_IDirectDrawEnumSurface2::EnumSurface2Callback));

	m_IDirectDrawEnumSurface2::ReleaseCallback();

	return hr;
}

HRESULT m_IDirectDraw4::FlipToGDISurface()
{
	return ProxyInterface->FlipToGDISurface();
}

HRESULT m_IDirectDraw4::GetCaps(LPDDCAPS a, LPDDCAPS b)
{
	return ProxyInterface->GetCaps(a, b);
}

HRESULT m_IDirectDraw4::GetDisplayMode(LPDDSURFACEDESC2 a)
{
	return ProxyInterface->GetDisplayMode(a);
}

HRESULT m_IDirectDraw4::GetFourCCCodes(LPDWORD a, LPDWORD b)
{
	return ProxyInterface->GetFourCCCodes(a, b);
}

HRESULT m_IDirectDraw4::GetGDISurface(LPDIRECTDRAWSURFACE4 FAR * a)
{
	HRESULT hr = ProxyInterface->GetGDISurface(a);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface4>(*a);
	}

	return hr;
}

HRESULT m_IDirectDraw4::GetMonitorFrequency(LPDWORD a)
{
	return ProxyInterface->GetMonitorFrequency(a);
}

HRESULT m_IDirectDraw4::GetScanLine(LPDWORD a)
{
	return ProxyInterface->GetScanLine(a);
}

HRESULT m_IDirectDraw4::GetVerticalBlankStatus(LPBOOL a)
{
	return ProxyInterface->GetVerticalBlankStatus(a);
}

HRESULT m_IDirectDraw4::Initialize(GUID FAR * a)
{
	return ProxyInterface->Initialize(a);
}

HRESULT m_IDirectDraw4::RestoreDisplayMode()
{
	return ProxyInterface->RestoreDisplayMode();
}

HRESULT m_IDirectDraw4::SetCooperativeLevel(HWND a, DWORD b)
{
	// Star Trek Armada 1 fix
	if (Config.ArmadaFix && b & DDSCL_EXCLUSIVE)
	{
		b |= DDSCL_ALLOWMODEX;
	}
	return ProxyInterface->SetCooperativeLevel(a, b);
}

HRESULT m_IDirectDraw4::SetDisplayMode(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e)
{
	// Star Trek Armada 1 fix
	if (Config.ArmadaFix && c == 16)
	{
		c = 32;
	}
	return ProxyInterface->SetDisplayMode(a, b, c, d, e);
}

HRESULT m_IDirectDraw4::WaitForVerticalBlank(DWORD a, HANDLE b)
{
	return ProxyInterface->WaitForVerticalBlank(a, b);
}

HRESULT m_IDirectDraw4::GetAvailableVidMem(LPDDSCAPS2 a, LPDWORD b, LPDWORD c)
{
	return ProxyInterface->GetAvailableVidMem(a, b, c);
}

HRESULT m_IDirectDraw4::GetSurfaceFromDC(HDC a, LPDIRECTDRAWSURFACE4 * b)
{
	HRESULT hr = ProxyInterface->GetSurfaceFromDC(a, b);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface4>(*b);
	}

	return hr;
}

HRESULT m_IDirectDraw4::RestoreAllSurfaces()
{
	return ProxyInterface->RestoreAllSurfaces();
}

HRESULT m_IDirectDraw4::TestCooperativeLevel()
{
	return ProxyInterface->TestCooperativeLevel();
}

HRESULT m_IDirectDraw4::GetDeviceIdentifier(LPDDDEVICEIDENTIFIER a, DWORD b)
{
	return ProxyInterface->GetDeviceIdentifier(a, b);
}
