/**
* Copyright (C) 2023 Elisha Riedlinger
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

HRESULT m_IDirectDraw::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	return ProxyInterface->QueryInterface(ReplaceIIDUnknown(riid, WrapperID), ppvObj, DirectXVersion);
}

ULONG m_IDirectDraw::AddRef()
{
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirectDraw::Release()
{
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirectDraw::Compact()
{
	return ProxyInterface->Compact();
}

HRESULT m_IDirectDraw::CreateClipper(DWORD a, LPDIRECTDRAWCLIPPER FAR * b, IUnknown FAR * c)
{
	return ProxyInterface->CreateClipper(a, b, c);
}

HRESULT m_IDirectDraw::CreatePalette(DWORD a, LPPALETTEENTRY b, LPDIRECTDRAWPALETTE FAR * c, IUnknown FAR * d)
{
	return ProxyInterface->CreatePalette(a, b, c, d);
}

HRESULT m_IDirectDraw::CreateSurface(LPDDSURFACEDESC a, LPDIRECTDRAWSURFACE FAR * b, IUnknown FAR * c)
{
	return ProxyInterface->CreateSurface(a, (LPDIRECTDRAWSURFACE7*)b, c, DirectXVersion);
}

HRESULT m_IDirectDraw::DuplicateSurface(LPDIRECTDRAWSURFACE a, LPDIRECTDRAWSURFACE FAR * b)
{
	return ProxyInterface->DuplicateSurface((LPDIRECTDRAWSURFACE7)a, (LPDIRECTDRAWSURFACE7*)b, DirectXVersion);
}

HRESULT m_IDirectDraw::EnumDisplayModes(DWORD a, LPDDSURFACEDESC b, LPVOID c, LPDDENUMMODESCALLBACK d)
{
	return ProxyInterface->EnumDisplayModes(a, b, c, d);
}

HRESULT m_IDirectDraw::EnumSurfaces(DWORD a, LPDDSURFACEDESC b, LPVOID c, LPDDENUMSURFACESCALLBACK d)
{
	return ProxyInterface->EnumSurfaces(a, b, c, d, DirectXVersion);
}

HRESULT m_IDirectDraw::FlipToGDISurface()
{
	return ProxyInterface->FlipToGDISurface();
}

HRESULT m_IDirectDraw::GetCaps(LPDDCAPS a, LPDDCAPS b)
{
	return ProxyInterface->GetCaps(a, b);
}

HRESULT m_IDirectDraw::GetDisplayMode(LPDDSURFACEDESC a)
{
	return ProxyInterface->GetDisplayMode(a);
}

HRESULT m_IDirectDraw::GetFourCCCodes(LPDWORD a, LPDWORD b)
{
	return ProxyInterface->GetFourCCCodes(a, b);
}

HRESULT m_IDirectDraw::GetGDISurface(LPDIRECTDRAWSURFACE FAR * a)
{
	return ProxyInterface->GetGDISurface((LPDIRECTDRAWSURFACE7*)a, DirectXVersion);
}

HRESULT m_IDirectDraw::GetMonitorFrequency(LPDWORD a)
{
	return ProxyInterface->GetMonitorFrequency(a);
}

HRESULT m_IDirectDraw::GetScanLine(LPDWORD a)
{
	return ProxyInterface->GetScanLine(a);
}

HRESULT m_IDirectDraw::GetVerticalBlankStatus(LPBOOL a)
{
	return ProxyInterface->GetVerticalBlankStatus(a);
}

HRESULT m_IDirectDraw::Initialize(GUID FAR * a)
{
	return ProxyInterface->Initialize(a);
}

HRESULT m_IDirectDraw::RestoreDisplayMode()
{
	return ProxyInterface->RestoreDisplayMode();
}

HRESULT m_IDirectDraw::SetCooperativeLevel(HWND a, DWORD b)
{
	return ProxyInterface->SetCooperativeLevel(a, b, DirectXVersion);
}

HRESULT m_IDirectDraw::SetDisplayMode(DWORD a, DWORD b, DWORD c)
{
	return ProxyInterface->SetDisplayMode(a, b, c, 0, 0);
}

HRESULT m_IDirectDraw::WaitForVerticalBlank(DWORD a, HANDLE b)
{
	return ProxyInterface->WaitForVerticalBlank(a, b);
}
