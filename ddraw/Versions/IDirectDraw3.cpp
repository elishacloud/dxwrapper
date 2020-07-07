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

HRESULT m_IDirectDraw3::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	return ProxyInterface->QueryInterface(ReplaceIIDUnknown(riid, WrapperID), ppvObj);
}

ULONG m_IDirectDraw3::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDraw3::Release()
{
	return ProxyInterface->Release();
}

HRESULT m_IDirectDraw3::Compact()
{
	return ProxyInterface->Compact();
}

HRESULT m_IDirectDraw3::CreateClipper(DWORD a, LPDIRECTDRAWCLIPPER FAR * b, IUnknown FAR * c)
{
	return ProxyInterface->CreateClipper(a, b, c);
}

HRESULT m_IDirectDraw3::CreatePalette(DWORD a, LPPALETTEENTRY b, LPDIRECTDRAWPALETTE FAR * c, IUnknown FAR * d)
{
	return ProxyInterface->CreatePalette(a, b, c, d);
}

HRESULT m_IDirectDraw3::CreateSurface(LPDDSURFACEDESC a, LPDIRECTDRAWSURFACE FAR * b, IUnknown FAR * c)
{
	return ProxyInterface->CreateSurface(a, (LPDIRECTDRAWSURFACE7*)b, c, DirectXVersion);
}

HRESULT m_IDirectDraw3::DuplicateSurface(LPDIRECTDRAWSURFACE a, LPDIRECTDRAWSURFACE FAR * b)
{
	return ProxyInterface->DuplicateSurface((LPDIRECTDRAWSURFACE7)a, (LPDIRECTDRAWSURFACE7*)b, DirectXVersion);
}

HRESULT m_IDirectDraw3::EnumDisplayModes(DWORD a, LPDDSURFACEDESC b, LPVOID c, LPDDENUMMODESCALLBACK d)
{
	return ProxyInterface->EnumDisplayModes(a, b, c, d);
}

HRESULT m_IDirectDraw3::EnumSurfaces(DWORD a, LPDDSURFACEDESC b, LPVOID c, LPDDENUMSURFACESCALLBACK d)
{
	return ProxyInterface->EnumSurfaces(a, b, c, d, DirectXVersion);
}

HRESULT m_IDirectDraw3::FlipToGDISurface()
{
	return ProxyInterface->FlipToGDISurface();
}

HRESULT m_IDirectDraw3::GetCaps(LPDDCAPS a, LPDDCAPS b)
{
	return ProxyInterface->GetCaps(a, b);
}

HRESULT m_IDirectDraw3::GetDisplayMode(LPDDSURFACEDESC a)
{
	return ProxyInterface->GetDisplayMode(a);
}

HRESULT m_IDirectDraw3::GetFourCCCodes(LPDWORD a, LPDWORD b)
{
	return ProxyInterface->GetFourCCCodes(a, b);
}

HRESULT m_IDirectDraw3::GetGDISurface(LPDIRECTDRAWSURFACE FAR * a)
{
	return ProxyInterface->GetGDISurface((LPDIRECTDRAWSURFACE7*)a, DirectXVersion);
}

HRESULT m_IDirectDraw3::GetMonitorFrequency(LPDWORD a)
{
	return ProxyInterface->GetMonitorFrequency(a);
}

HRESULT m_IDirectDraw3::GetScanLine(LPDWORD a)
{
	return ProxyInterface->GetScanLine(a);
}

HRESULT m_IDirectDraw3::GetVerticalBlankStatus(LPBOOL a)
{
	return ProxyInterface->GetVerticalBlankStatus(a);
}

HRESULT m_IDirectDraw3::Initialize(GUID FAR * a)
{
	return ProxyInterface->Initialize(a);
}

HRESULT m_IDirectDraw3::RestoreDisplayMode()
{
	return ProxyInterface->RestoreDisplayMode();
}

HRESULT m_IDirectDraw3::SetCooperativeLevel(HWND a, DWORD b)
{
	return ProxyInterface->SetCooperativeLevel(a, b);
}

HRESULT m_IDirectDraw3::SetDisplayMode(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e)
{
	return ProxyInterface->SetDisplayMode(a, b, c, d, e);
}

HRESULT m_IDirectDraw3::WaitForVerticalBlank(DWORD a, HANDLE b)
{
	return ProxyInterface->WaitForVerticalBlank(a, b);
}

HRESULT m_IDirectDraw3::GetAvailableVidMem(LPDDSCAPS a, LPDWORD b, LPDWORD c)
{
	return ProxyInterface->GetAvailableVidMem(a, b, c);
}

HRESULT m_IDirectDraw3::GetSurfaceFromDC(HDC a, IDirectDrawSurface * * b)
{
	return ProxyInterface->GetSurfaceFromDC(a, (LPDIRECTDRAWSURFACE7*)b, DirectXVersion);
}
