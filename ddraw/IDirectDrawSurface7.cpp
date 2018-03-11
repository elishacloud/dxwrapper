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

HRESULT m_IDirectDrawSurface7::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG m_IDirectDrawSurface7::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawSurface7::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		delete this;
	}

	return x;
}

HRESULT m_IDirectDrawSurface7::AddAttachedSurface(LPDIRECTDRAWSURFACE7 a)
{
	return ProxyInterface->AddAttachedSurface(a);
}

HRESULT m_IDirectDrawSurface7::AddOverlayDirtyRect(LPRECT a)
{
	return ProxyInterface->AddOverlayDirtyRect(a);
}

HRESULT m_IDirectDrawSurface7::Blt(LPRECT a, LPDIRECTDRAWSURFACE7 b, LPRECT c, DWORD d, LPDDBLTFX e)
{
	return ProxyInterface->Blt(a, b, c, d, e);
}

HRESULT m_IDirectDrawSurface7::BltBatch(LPDDBLTBATCH a, DWORD b, DWORD c)
{
	return ProxyInterface->BltBatch(a, b, c);
}

HRESULT m_IDirectDrawSurface7::BltFast(DWORD a, DWORD b, LPDIRECTDRAWSURFACE7 c, LPRECT d, DWORD e)
{
	return ProxyInterface->BltFast(a, b, c, d, e);
}

HRESULT m_IDirectDrawSurface7::DeleteAttachedSurface(DWORD a, LPDIRECTDRAWSURFACE7 b)
{
	return ProxyInterface->DeleteAttachedSurface(a, b);
}

HRESULT m_IDirectDrawSurface7::EnumAttachedSurfaces(LPVOID a, LPDDENUMSURFACESCALLBACK7 b)
{
	return ProxyInterface->EnumAttachedSurfaces(a, b);
}

HRESULT m_IDirectDrawSurface7::EnumOverlayZOrders(DWORD a, LPVOID b, LPDDENUMSURFACESCALLBACK7 c)
{
	return ProxyInterface->EnumOverlayZOrders(a, b, c);
}

HRESULT m_IDirectDrawSurface7::Flip(LPDIRECTDRAWSURFACE7 a, DWORD b)
{
	return ProxyInterface->Flip(a, b);
}

HRESULT m_IDirectDrawSurface7::GetAttachedSurface(LPDDSCAPS2 a, LPDIRECTDRAWSURFACE7 FAR * b)
{
	return ProxyInterface->GetAttachedSurface(a, b);
}

HRESULT m_IDirectDrawSurface7::GetBltStatus(DWORD a)
{
	return ProxyInterface->GetBltStatus(a);
}

HRESULT m_IDirectDrawSurface7::GetCaps(LPDDSCAPS2 a)
{
	return ProxyInterface->GetCaps(a);
}

HRESULT m_IDirectDrawSurface7::GetClipper(LPDIRECTDRAWCLIPPER FAR * a)
{
	return ProxyInterface->GetClipper(a);
}

HRESULT m_IDirectDrawSurface7::GetColorKey(DWORD a, LPDDCOLORKEY b)
{
	return ProxyInterface->GetColorKey(a, b);
}

HRESULT m_IDirectDrawSurface7::GetDC(HDC FAR * a)
{
	return ProxyInterface->GetDC(a);
}

HRESULT m_IDirectDrawSurface7::GetFlipStatus(DWORD a)
{
	return ProxyInterface->GetFlipStatus(a);
}

HRESULT m_IDirectDrawSurface7::GetOverlayPosition(LPLONG a, LPLONG b)
{
	return ProxyInterface->GetOverlayPosition(a, b);
}

HRESULT m_IDirectDrawSurface7::GetPalette(LPDIRECTDRAWPALETTE FAR * a)
{
	return ProxyInterface->GetPalette(a);
}

HRESULT m_IDirectDrawSurface7::GetPixelFormat(LPDDPIXELFORMAT a)
{
	return ProxyInterface->GetPixelFormat(a);
}

HRESULT m_IDirectDrawSurface7::GetSurfaceDesc(LPDDSURFACEDESC2 a)
{
	return ProxyInterface->GetSurfaceDesc(a);
}

HRESULT m_IDirectDrawSurface7::Initialize(LPDIRECTDRAW a, LPDDSURFACEDESC2 b)
{
	return ProxyInterface->Initialize(a, b);
}

HRESULT m_IDirectDrawSurface7::IsLost()
{
	return ProxyInterface->IsLost();
}

HRESULT m_IDirectDrawSurface7::Lock(LPRECT a, LPDDSURFACEDESC2 b, DWORD c, HANDLE d)
{
	return ProxyInterface->Lock(a, b, c, d);
}

HRESULT m_IDirectDrawSurface7::ReleaseDC(HDC a)
{
	return ProxyInterface->ReleaseDC(a);
}

HRESULT m_IDirectDrawSurface7::Restore()
{
	return ProxyInterface->Restore();
}

HRESULT m_IDirectDrawSurface7::SetClipper(LPDIRECTDRAWCLIPPER a)
{
	return ProxyInterface->SetClipper(a);
}

HRESULT m_IDirectDrawSurface7::SetColorKey(DWORD a, LPDDCOLORKEY b)
{
	return ProxyInterface->SetColorKey(a, b);
}

HRESULT m_IDirectDrawSurface7::SetOverlayPosition(LONG a, LONG b)
{
	return ProxyInterface->SetOverlayPosition(a, b);
}

HRESULT m_IDirectDrawSurface7::SetPalette(LPDIRECTDRAWPALETTE a)
{
	return ProxyInterface->SetPalette(a);
}

HRESULT m_IDirectDrawSurface7::Unlock(LPRECT a)
{
	return ProxyInterface->Unlock(a);
}

HRESULT m_IDirectDrawSurface7::UpdateOverlay(LPRECT a, LPDIRECTDRAWSURFACE7 b, LPRECT c, DWORD d, LPDDOVERLAYFX e)
{
	return ProxyInterface->UpdateOverlay(a, b, c, d, e);
}

HRESULT m_IDirectDrawSurface7::UpdateOverlayDisplay(DWORD a)
{
	return ProxyInterface->UpdateOverlayDisplay(a);
}

HRESULT m_IDirectDrawSurface7::UpdateOverlayZOrder(DWORD a, LPDIRECTDRAWSURFACE7 b)
{
	return ProxyInterface->UpdateOverlayZOrder(a, b);
}

HRESULT m_IDirectDrawSurface7::GetDDInterface(LPVOID FAR * a)
{
	return ProxyInterface->GetDDInterface(a);
}

HRESULT m_IDirectDrawSurface7::PageLock(DWORD a)
{
	return ProxyInterface->PageLock(a);
}

HRESULT m_IDirectDrawSurface7::PageUnlock(DWORD a)
{
	return ProxyInterface->PageUnlock(a);
}

HRESULT m_IDirectDrawSurface7::SetSurfaceDesc(LPDDSURFACEDESC2 a, DWORD b)
{
	return ProxyInterface->SetSurfaceDesc(a, b);
}

HRESULT m_IDirectDrawSurface7::SetPrivateData(REFGUID a, LPVOID b, DWORD c, DWORD d)
{
	return ProxyInterface->SetPrivateData(a, b, c, d);
}

HRESULT m_IDirectDrawSurface7::GetPrivateData(REFGUID a, LPVOID b, LPDWORD c)
{
	return ProxyInterface->GetPrivateData(a, b, c);
}

HRESULT m_IDirectDrawSurface7::FreePrivateData(REFGUID a)
{
	return ProxyInterface->FreePrivateData(a);
}

HRESULT m_IDirectDrawSurface7::GetUniquenessValue(LPDWORD a)
{
	return ProxyInterface->GetUniquenessValue(a);
}

HRESULT m_IDirectDrawSurface7::ChangeUniquenessValue()
{
	return ProxyInterface->ChangeUniquenessValue();
}

HRESULT m_IDirectDrawSurface7::SetPriority(DWORD a)
{
	return ProxyInterface->SetPriority(a);
}

HRESULT m_IDirectDrawSurface7::GetPriority(LPDWORD a)
{
	return ProxyInterface->GetPriority(a);
}

HRESULT m_IDirectDrawSurface7::SetLOD(DWORD a)
{
	return ProxyInterface->SetLOD(a);
}

HRESULT m_IDirectDrawSurface7::GetLOD(LPDWORD a)
{
	return ProxyInterface->GetLOD(a);
}
