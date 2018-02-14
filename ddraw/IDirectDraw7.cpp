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

HRESULT m_IDirectDraw7::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectDraw7::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDraw7::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		delete this;
	}

	return x;
}

HRESULT m_IDirectDraw7::Compact()
{
	return ProxyInterface->Compact();
}

HRESULT m_IDirectDraw7::CreateClipper(DWORD a, LPDIRECTDRAWCLIPPER FAR * b, IUnknown FAR * c)
{
	HRESULT hr = ProxyInterface->CreateClipper(a, b, c);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawClipper>(*b);
	}

	return hr;
}

HRESULT m_IDirectDraw7::CreatePalette(DWORD a, LPPALETTEENTRY b, LPDIRECTDRAWPALETTE FAR * c, IUnknown FAR * d)
{
	HRESULT hr = ProxyInterface->CreatePalette(a, b, c, d);

	if (SUCCEEDED(hr))
	{
		*c = ProxyAddressLookupTable.FindAddress<m_IDirectDrawPalette>(*c);
	}

	return hr;
}

HRESULT m_IDirectDraw7::CreateSurface(LPDDSURFACEDESC2 a, LPDIRECTDRAWSURFACE7 FAR * b, IUnknown FAR * c)
{
	HRESULT hr = ProxyInterface->CreateSurface(a, b, c);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*b);
	}

	return hr;
}

HRESULT m_IDirectDraw7::DuplicateSurface(LPDIRECTDRAWSURFACE7 a, LPDIRECTDRAWSURFACE7 FAR * b)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawSurface7 *>(a)->GetProxyInterface();
	}

	HRESULT hr = ProxyInterface->DuplicateSurface(a, b);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*b);
	}

	return hr;
}

HRESULT m_IDirectDraw7::EnumDisplayModes(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMMODESCALLBACK2 d)
{
	return ProxyInterface->EnumDisplayModes(a, b, c, d);
}

HRESULT m_IDirectDraw7::EnumSurfaces(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMSURFACESCALLBACK7 d)
{
	m_IDirectDrawEnumSurface7::SetCallback(d);

	HRESULT hr = ProxyInterface->EnumSurfaces(a, b, c, reinterpret_cast<LPDDENUMSURFACESCALLBACK7>(m_IDirectDrawEnumSurface7::EnumSurface7Callback));

	m_IDirectDrawEnumSurface7::ReleaseCallback();

	return hr;
}

HRESULT m_IDirectDraw7::FlipToGDISurface()
{
	return ProxyInterface->FlipToGDISurface();
}

HRESULT m_IDirectDraw7::GetCaps(LPDDCAPS a, LPDDCAPS b)
{
	return ProxyInterface->GetCaps(a, b);
}

HRESULT m_IDirectDraw7::GetDisplayMode(LPDDSURFACEDESC2 a)
{
	return ProxyInterface->GetDisplayMode(a);
}

HRESULT m_IDirectDraw7::GetFourCCCodes(LPDWORD a, LPDWORD b)
{
	return ProxyInterface->GetFourCCCodes(a, b);
}

HRESULT m_IDirectDraw7::GetGDISurface(LPDIRECTDRAWSURFACE7 FAR * a)
{
	HRESULT hr = ProxyInterface->GetGDISurface(a);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*a);
	}

	return hr;
}

HRESULT m_IDirectDraw7::GetMonitorFrequency(LPDWORD a)
{
	return ProxyInterface->GetMonitorFrequency(a);
}

HRESULT m_IDirectDraw7::GetScanLine(LPDWORD a)
{
	return ProxyInterface->GetScanLine(a);
}

HRESULT m_IDirectDraw7::GetVerticalBlankStatus(LPBOOL a)
{
	return ProxyInterface->GetVerticalBlankStatus(a);
}

HRESULT m_IDirectDraw7::Initialize(GUID FAR * a)
{
	return ProxyInterface->Initialize(a);
}

HRESULT m_IDirectDraw7::RestoreDisplayMode()
{
	return ProxyInterface->RestoreDisplayMode();
}

HRESULT m_IDirectDraw7::SetCooperativeLevel(HWND a, DWORD b)
{
	// Star Trek Armada 1 fix
	if (Config.ArmadaFix && b & DDSCL_EXCLUSIVE)
	{
		b |= DDSCL_ALLOWMODEX;
	}
	return ProxyInterface->SetCooperativeLevel(a, b);
}

HRESULT m_IDirectDraw7::SetDisplayMode(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e)
{
	// Star Trek Armada 1 fix
	if (Config.ArmadaFix && c == 16)
	{
		c = 32;
	}
	return ProxyInterface->SetDisplayMode(a, b, c, d, e);
}

HRESULT m_IDirectDraw7::WaitForVerticalBlank(DWORD a, HANDLE b)
{
	return ProxyInterface->WaitForVerticalBlank(a, b);
}

HRESULT m_IDirectDraw7::GetAvailableVidMem(LPDDSCAPS2 a, LPDWORD b, LPDWORD c)
{
	return ProxyInterface->GetAvailableVidMem(a, b, c);
}

HRESULT m_IDirectDraw7::GetSurfaceFromDC(HDC a, LPDIRECTDRAWSURFACE7 * b)
{
	HRESULT hr = ProxyInterface->GetSurfaceFromDC(a, b);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*b);
	}

	return hr;
}

HRESULT m_IDirectDraw7::RestoreAllSurfaces()
{
	return ProxyInterface->RestoreAllSurfaces();
}

HRESULT m_IDirectDraw7::TestCooperativeLevel()
{
	return ProxyInterface->TestCooperativeLevel();
}

HRESULT m_IDirectDraw7::GetDeviceIdentifier(LPDDDEVICEIDENTIFIER2 a, DWORD b)
{
	return ProxyInterface->GetDeviceIdentifier(a, b);
}

HRESULT m_IDirectDraw7::StartModeTest(LPSIZE a, DWORD b, DWORD c)
{
	return ProxyInterface->StartModeTest(a, b, c);
}

HRESULT m_IDirectDraw7::EvaluateMode(DWORD a, DWORD * b)
{
	return ProxyInterface->EvaluateMode(a, b);
}
