/**
* Copyright (C) 2020 Elisha Riedlinger
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

#include "..\ddraw.h"

HRESULT m_IDirectDraw4::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	return ProxyInterface->QueryInterface(ReplaceIIDUnknown(riid, WrapperID), ppvObj);
}

ULONG m_IDirectDraw4::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDraw4::Release()
{
	return ProxyInterface->Release();
}

HRESULT m_IDirectDraw4::Compact()
{
	return ProxyInterface->Compact();
}

HRESULT m_IDirectDraw4::CreateClipper(DWORD a, LPDIRECTDRAWCLIPPER FAR * b, IUnknown FAR * c)
{
	return ProxyInterface->CreateClipper(a, b, c);
}

HRESULT m_IDirectDraw4::CreatePalette(DWORD a, LPPALETTEENTRY b, LPDIRECTDRAWPALETTE FAR * c, IUnknown FAR * d)
{
	return ProxyInterface->CreatePalette(a, b, c, d);
}

HRESULT m_IDirectDraw4::CreateSurface(LPDDSURFACEDESC2 a, LPDIRECTDRAWSURFACE4 FAR * b, IUnknown FAR * c)
{
	return ProxyInterface->CreateSurface2(a, (LPDIRECTDRAWSURFACE7*)b, c, DirectXVersion);
}

HRESULT m_IDirectDraw4::DuplicateSurface(LPDIRECTDRAWSURFACE4 a, LPDIRECTDRAWSURFACE4 FAR * b)
{
	return ProxyInterface->DuplicateSurface((LPDIRECTDRAWSURFACE7)a, (LPDIRECTDRAWSURFACE7*)b, DirectXVersion);
}

HRESULT m_IDirectDraw4::EnumDisplayModes(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMMODESCALLBACK2 d)
{
	return ProxyInterface->EnumDisplayModes2(a, b, c, d);
}

HRESULT m_IDirectDraw4::EnumSurfaces(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMSURFACESCALLBACK2 d)
{
	return ProxyInterface->EnumSurfaces2(a, b, c, (LPDDENUMSURFACESCALLBACK7)d, DirectXVersion);
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
	return ProxyInterface->GetDisplayMode2(a);
}

HRESULT m_IDirectDraw4::GetFourCCCodes(LPDWORD a, LPDWORD b)
{
	return ProxyInterface->GetFourCCCodes(a, b);
}

HRESULT m_IDirectDraw4::GetGDISurface(LPDIRECTDRAWSURFACE4 FAR * a)
{
	return ProxyInterface->GetGDISurface((LPDIRECTDRAWSURFACE7*)a, DirectXVersion);
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
	return ProxyInterface->SetCooperativeLevel(a, b);
}

HRESULT m_IDirectDraw4::SetDisplayMode(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e)
{
	return ProxyInterface->SetDisplayMode(a, b, c, d, e);
}

HRESULT m_IDirectDraw4::WaitForVerticalBlank(DWORD a, HANDLE b)
{
	return ProxyInterface->WaitForVerticalBlank(a, b);
}

HRESULT m_IDirectDraw4::GetAvailableVidMem(LPDDSCAPS2 a, LPDWORD b, LPDWORD c)
{
	return ProxyInterface->GetAvailableVidMem2(a, b, c);
}

HRESULT m_IDirectDraw4::GetSurfaceFromDC(HDC a, LPDIRECTDRAWSURFACE4 * b)
{
	return ProxyInterface->GetSurfaceFromDC(a, (LPDIRECTDRAWSURFACE7*)b, DirectXVersion);
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
