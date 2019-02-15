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
*/

#include "..\ddraw.h"

HRESULT m_IDirectDrawSurface2::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	return ProxyInterface->QueryInterface(riid, ppvObj, DirectXVersion);
}

ULONG m_IDirectDrawSurface2::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawSurface2::Release()
{
	return ProxyInterface->Release();
}

HRESULT m_IDirectDrawSurface2::AddAttachedSurface(LPDIRECTDRAWSURFACE2 a)
{
	return ProxyInterface->AddAttachedSurface((LPDIRECTDRAWSURFACE7)a);
}

HRESULT m_IDirectDrawSurface2::AddOverlayDirtyRect(LPRECT a)
{
	return ProxyInterface->AddOverlayDirtyRect(a);
}

HRESULT m_IDirectDrawSurface2::Blt(LPRECT a, LPDIRECTDRAWSURFACE2 b, LPRECT c, DWORD d, LPDDBLTFX e)
{
	return ProxyInterface->Blt(a, (LPDIRECTDRAWSURFACE7)b, c, d, e);
}

HRESULT m_IDirectDrawSurface2::BltBatch(LPDDBLTBATCH a, DWORD b, DWORD c)
{
	return ProxyInterface->BltBatch(a, b, c);
}

HRESULT m_IDirectDrawSurface2::BltFast(DWORD a, DWORD b, LPDIRECTDRAWSURFACE2 c, LPRECT d, DWORD e)
{
	return ProxyInterface->BltFast(a, b, (LPDIRECTDRAWSURFACE7)c, d, e);
}

HRESULT m_IDirectDrawSurface2::DeleteAttachedSurface(DWORD a, LPDIRECTDRAWSURFACE2 b)
{
	return ProxyInterface->DeleteAttachedSurface(a, (LPDIRECTDRAWSURFACE7)b);
}

HRESULT m_IDirectDrawSurface2::EnumAttachedSurfaces(LPVOID a, LPDDENUMSURFACESCALLBACK b)
{
	return ProxyInterface->EnumAttachedSurfaces(a, b, DirectXVersion);
}

HRESULT m_IDirectDrawSurface2::EnumOverlayZOrders(DWORD a, LPVOID b, LPDDENUMSURFACESCALLBACK c)
{
	return ProxyInterface->EnumOverlayZOrders(a, b, c, DirectXVersion);
}

HRESULT m_IDirectDrawSurface2::Flip(LPDIRECTDRAWSURFACE2 a, DWORD b)
{
	return ProxyInterface->Flip((LPDIRECTDRAWSURFACE7)a, b);
}

HRESULT m_IDirectDrawSurface2::GetAttachedSurface(LPDDSCAPS a, LPDIRECTDRAWSURFACE2 FAR * b)
{
	return ProxyInterface->GetAttachedSurface(a, (LPDIRECTDRAWSURFACE7*)b, DirectXVersion);
}

HRESULT m_IDirectDrawSurface2::GetBltStatus(DWORD a)
{
	return ProxyInterface->GetBltStatus(a);
}

HRESULT m_IDirectDrawSurface2::GetCaps(LPDDSCAPS a)
{
	return ProxyInterface->GetCaps(a);
}

HRESULT m_IDirectDrawSurface2::GetClipper(LPDIRECTDRAWCLIPPER FAR * a)
{
	return ProxyInterface->GetClipper(a);
}

HRESULT m_IDirectDrawSurface2::GetColorKey(DWORD a, LPDDCOLORKEY b)
{
	return ProxyInterface->GetColorKey(a, b);
}

HRESULT m_IDirectDrawSurface2::GetDC(HDC FAR * a)
{
	return ProxyInterface->GetDC(a);
}

HRESULT m_IDirectDrawSurface2::GetFlipStatus(DWORD a)
{
	return ProxyInterface->GetFlipStatus(a);
}

HRESULT m_IDirectDrawSurface2::GetOverlayPosition(LPLONG a, LPLONG b)
{
	return ProxyInterface->GetOverlayPosition(a, b);
}

HRESULT m_IDirectDrawSurface2::GetPalette(LPDIRECTDRAWPALETTE FAR * a)
{
	return ProxyInterface->GetPalette(a);
}

HRESULT m_IDirectDrawSurface2::GetPixelFormat(LPDDPIXELFORMAT a)
{
	return ProxyInterface->GetPixelFormat(a);
}

HRESULT m_IDirectDrawSurface2::GetSurfaceDesc(LPDDSURFACEDESC a)
{
	return ProxyInterface->GetSurfaceDesc(a);
}

HRESULT m_IDirectDrawSurface2::Initialize(LPDIRECTDRAW a, LPDDSURFACEDESC b)
{
	return ProxyInterface->Initialize(a, b);
}

HRESULT m_IDirectDrawSurface2::IsLost()
{
	return ProxyInterface->IsLost();
}

HRESULT m_IDirectDrawSurface2::Lock(LPRECT a, LPDDSURFACEDESC b, DWORD c, HANDLE d)
{
	return ProxyInterface->Lock(a, b, c, d);
}

HRESULT m_IDirectDrawSurface2::ReleaseDC(HDC a)
{
	return ProxyInterface->ReleaseDC(a);
}

HRESULT m_IDirectDrawSurface2::Restore()
{
	return ProxyInterface->Restore();
}

HRESULT m_IDirectDrawSurface2::SetClipper(LPDIRECTDRAWCLIPPER a)
{
	return ProxyInterface->SetClipper(a);
}

HRESULT m_IDirectDrawSurface2::SetColorKey(DWORD a, LPDDCOLORKEY b)
{
	return ProxyInterface->SetColorKey(a, b);
}

HRESULT m_IDirectDrawSurface2::SetOverlayPosition(LONG a, LONG b)
{
	return ProxyInterface->SetOverlayPosition(a, b);
}

HRESULT m_IDirectDrawSurface2::SetPalette(LPDIRECTDRAWPALETTE a)
{
	return ProxyInterface->SetPalette(a);
}

HRESULT m_IDirectDrawSurface2::Unlock(LPVOID a)
{
	return ProxyInterface->Unlock((LPRECT)a);
}

HRESULT m_IDirectDrawSurface2::UpdateOverlay(LPRECT a, LPDIRECTDRAWSURFACE2 b, LPRECT c, DWORD d, LPDDOVERLAYFX e)
{
	return ProxyInterface->UpdateOverlay(a, (LPDIRECTDRAWSURFACE7)b, c, d, e);
}

HRESULT m_IDirectDrawSurface2::UpdateOverlayDisplay(DWORD a)
{
	return ProxyInterface->UpdateOverlayDisplay(a);
}

HRESULT m_IDirectDrawSurface2::UpdateOverlayZOrder(DWORD a, LPDIRECTDRAWSURFACE2 b)
{
	return ProxyInterface->UpdateOverlayZOrder(a, (LPDIRECTDRAWSURFACE7)b);
}

HRESULT m_IDirectDrawSurface2::GetDDInterface(LPVOID FAR * a)
{
	return ProxyInterface->GetDDInterface(a, DirectXVersion);
}

HRESULT m_IDirectDrawSurface2::PageLock(DWORD a)
{
	return ProxyInterface->PageLock(a);
}

HRESULT m_IDirectDrawSurface2::PageUnlock(DWORD a)
{
	return ProxyInterface->PageUnlock(a);
}
