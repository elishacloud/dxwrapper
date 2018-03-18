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

HRESULT m_IDirectDrawSurfaceX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, RealIID);
}

ULONG m_IDirectDrawSurfaceX::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawSurfaceX::Release()
{
	return ProxyInterface->Release();
}

HRESULT m_IDirectDrawSurfaceX::AddAttachedSurface(LPDIRECTDRAWSURFACE7 a)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawSurface7 *>(a)->GetProxyInterface();
	}

	return ProxyInterface->AddAttachedSurface(a);
}

HRESULT m_IDirectDrawSurfaceX::AddOverlayDirtyRect(LPRECT a)
{
	return ProxyInterface->AddOverlayDirtyRect(a);
}

HRESULT m_IDirectDrawSurfaceX::Blt(LPRECT a, LPDIRECTDRAWSURFACE7 b, LPRECT c, DWORD d, LPDDBLTFX e)
{
	if (b)
	{
		b = static_cast<m_IDirectDrawSurface7 *>(b)->GetProxyInterface();
	}

	return ProxyInterface->Blt(a, b, c, d, e);
}

HRESULT m_IDirectDrawSurfaceX::BltBatch(LPDDBLTBATCH a, DWORD b, DWORD c)
{
	return ProxyInterface->BltBatch(a, b, c);
}

HRESULT m_IDirectDrawSurfaceX::BltFast(DWORD a, DWORD b, LPDIRECTDRAWSURFACE7 c, LPRECT d, DWORD e)
{
	if (c)
	{
		c = static_cast<m_IDirectDrawSurface7 *>(c)->GetProxyInterface();
	}
	
	return ProxyInterface->BltFast(a, b, c, d, e);
}

HRESULT m_IDirectDrawSurfaceX::DeleteAttachedSurface(DWORD a, LPDIRECTDRAWSURFACE7 b)
{
	if (b)
	{
		b = static_cast<m_IDirectDrawSurface7 *>(b)->GetProxyInterface();
	}

	return ProxyInterface->DeleteAttachedSurface(a, b);
}

template HRESULT m_IDirectDrawSurfaceX::EnumAttachedSurfaces<LPDDENUMSURFACESCALLBACK>(LPVOID a, LPDDENUMSURFACESCALLBACK b);
template HRESULT m_IDirectDrawSurfaceX::EnumAttachedSurfaces<LPDDENUMSURFACESCALLBACK7>(LPVOID a, LPDDENUMSURFACESCALLBACK7 b);
template <typename T>
HRESULT m_IDirectDrawSurfaceX::EnumAttachedSurfaces(LPVOID a, T b)
{
	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = a;
	CallbackContext.lpCallback = (LPDDENUMSURFACESCALLBACK7)b;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ProxyDirectXVersion = ProxyDirectXVersion;

	HRESULT hr = ProxyInterface->EnumAttachedSurfaces(a, m_IDirectDrawEnumSurface::ConvertCallback);

	return hr;
}

template HRESULT m_IDirectDrawSurfaceX::EnumOverlayZOrders<LPDDENUMSURFACESCALLBACK>(DWORD a, LPVOID b, LPDDENUMSURFACESCALLBACK c);
template HRESULT m_IDirectDrawSurfaceX::EnumOverlayZOrders<LPDDENUMSURFACESCALLBACK7>(DWORD a, LPVOID b, LPDDENUMSURFACESCALLBACK7 c);
template <typename T>
HRESULT m_IDirectDrawSurfaceX::EnumOverlayZOrders(DWORD a, LPVOID b, T c)
{
	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = b;
	CallbackContext.lpCallback = (LPDDENUMSURFACESCALLBACK7)c;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ProxyDirectXVersion = ProxyDirectXVersion;

	HRESULT hr = ProxyInterface->EnumOverlayZOrders(a, b, m_IDirectDrawEnumSurface::ConvertCallback);

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::Flip(LPDIRECTDRAWSURFACE7 a, DWORD b)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawSurface7 *>(a)->GetProxyInterface();
	}

	return ProxyInterface->Flip(a, b);
}

template HRESULT m_IDirectDrawSurfaceX::GetAttachedSurface<LPDDSCAPS>(LPDDSCAPS a, LPDIRECTDRAWSURFACE7 FAR * b);
template HRESULT m_IDirectDrawSurfaceX::GetAttachedSurface<LPDDSCAPS2>(LPDDSCAPS2 a, LPDIRECTDRAWSURFACE7 FAR * b);
template <typename T>
HRESULT m_IDirectDrawSurfaceX::GetAttachedSurface(T a, LPDIRECTDRAWSURFACE7 FAR * b)
{
	DDSCAPS2 Caps2;
	if (a != nullptr && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertCaps(Caps2, *a);
		a = (T)&Caps2;
	}

	HRESULT hr = ProxyInterface->GetAttachedSurface((LPDDSCAPS2)a, b);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*b, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetBltStatus(DWORD a)
{
	return ProxyInterface->GetBltStatus(a);
}

template HRESULT m_IDirectDrawSurfaceX::GetCaps<LPDDSCAPS>(LPDDSCAPS lpDDSCaps);
template HRESULT m_IDirectDrawSurfaceX::GetCaps<LPDDSCAPS2>(LPDDSCAPS2 lpDDSCaps);
template <typename T>
HRESULT m_IDirectDrawSurfaceX::GetCaps(T lpDDSCaps)
{
	T lpDDSCaps_tmp = lpDDSCaps;
	DDSCAPS2 Caps2;
	if (lpDDSCaps != nullptr && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertCaps(Caps2, *lpDDSCaps);
		lpDDSCaps = (T)&Caps2;
	}

	HRESULT hr = ProxyInterface->GetCaps((LPDDSCAPS2)lpDDSCaps);

	if (SUCCEEDED(hr) && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertCaps(*lpDDSCaps_tmp, Caps2);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetClipper(LPDIRECTDRAWCLIPPER FAR * a)
{
	HRESULT hr = ProxyInterface->GetClipper(a);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirectDrawClipper>(*a);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetColorKey(DWORD a, LPDDCOLORKEY b)
{
	return ProxyInterface->GetColorKey(a, b);
}

HRESULT m_IDirectDrawSurfaceX::GetDC(HDC FAR * a)
{
	return ProxyInterface->GetDC(a);
}

HRESULT m_IDirectDrawSurfaceX::GetFlipStatus(DWORD a)
{
	return ProxyInterface->GetFlipStatus(a);
}

HRESULT m_IDirectDrawSurfaceX::GetOverlayPosition(LPLONG a, LPLONG b)
{
	return ProxyInterface->GetOverlayPosition(a, b);
}

HRESULT m_IDirectDrawSurfaceX::GetPalette(LPDIRECTDRAWPALETTE FAR * a)
{
	HRESULT hr = ProxyInterface->GetPalette(a);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirectDrawPalette>(*a);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetPixelFormat(LPDDPIXELFORMAT a)
{
	return ProxyInterface->GetPixelFormat(a);
}

template HRESULT m_IDirectDrawSurfaceX::GetSurfaceDesc<LPDDSURFACEDESC>(LPDDSURFACEDESC lpDDSCaps);
template HRESULT m_IDirectDrawSurfaceX::GetSurfaceDesc<LPDDSURFACEDESC2>(LPDDSURFACEDESC2 lpDDSCaps);
template <typename T>
HRESULT m_IDirectDrawSurfaceX::GetSurfaceDesc(T lpDDSurfaceDesc)
{
	T lpDDSurfaceDesc_tmp = lpDDSurfaceDesc;
	DDSURFACEDESC2 Desc2;
	if (lpDDSurfaceDesc != nullptr && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);
		lpDDSurfaceDesc = (T)&Desc2;
	}

	HRESULT hr = ProxyInterface->GetSurfaceDesc((LPDDSURFACEDESC2)lpDDSurfaceDesc);

	if (SUCCEEDED(hr) && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(*lpDDSurfaceDesc_tmp, Desc2);
	}

	return hr;
}

template HRESULT m_IDirectDrawSurfaceX::Initialize<LPDDSURFACEDESC>(LPDIRECTDRAW a, LPDDSURFACEDESC b);
template HRESULT m_IDirectDrawSurfaceX::Initialize<LPDDSURFACEDESC2>(LPDIRECTDRAW a, LPDDSURFACEDESC2 b);
template <typename T>
HRESULT m_IDirectDrawSurfaceX::Initialize(LPDIRECTDRAW a, T b)
{
	DDSURFACEDESC2 Desc2;
	if (b != nullptr && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *b);
		b = (T)&Desc2;
	}

	if (a)
	{
		a = static_cast<m_IDirectDraw *>(a)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(a, (LPDDSURFACEDESC2)b);
}

HRESULT m_IDirectDrawSurfaceX::IsLost()
{
	return ProxyInterface->IsLost();
}

template HRESULT m_IDirectDrawSurfaceX::Lock<LPDDSURFACEDESC>(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent);
template HRESULT m_IDirectDrawSurfaceX::Lock<LPDDSURFACEDESC2>(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent);
template <typename T>
HRESULT m_IDirectDrawSurfaceX::Lock(LPRECT lpDestRect, T lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent)
{
	T lpDDSurfaceDesc_tmp = lpDDSurfaceDesc;
	DDSURFACEDESC2 Desc2;
	if (lpDDSurfaceDesc != nullptr && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *lpDDSurfaceDesc);
		lpDDSurfaceDesc = (T)&Desc2;
	}

	HRESULT hr = ProxyInterface->Lock(lpDestRect, (LPDDSURFACEDESC2)lpDDSurfaceDesc, dwFlags, hEvent);

	if (SUCCEEDED(hr) && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(*lpDDSurfaceDesc_tmp, Desc2);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::ReleaseDC(HDC a)
{
	return ProxyInterface->ReleaseDC(a);
}

HRESULT m_IDirectDrawSurfaceX::Restore()
{
	return ProxyInterface->Restore();
}

HRESULT m_IDirectDrawSurfaceX::SetClipper(LPDIRECTDRAWCLIPPER a)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawClipper *>(a)->GetProxyInterface();
	}

	return ProxyInterface->SetClipper(a);
}

HRESULT m_IDirectDrawSurfaceX::SetColorKey(DWORD a, LPDDCOLORKEY b)
{
	return ProxyInterface->SetColorKey(a, b);
}

HRESULT m_IDirectDrawSurfaceX::SetOverlayPosition(LONG a, LONG b)
{
	return ProxyInterface->SetOverlayPosition(a, b);
}

HRESULT m_IDirectDrawSurfaceX::SetPalette(LPDIRECTDRAWPALETTE a)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawPalette *>(a)->GetProxyInterface();
	}

	return ProxyInterface->SetPalette(a);
}

HRESULT m_IDirectDrawSurfaceX::Unlock(LPRECT a)
{
	return ProxyInterface->Unlock(a);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlay(LPRECT a, LPDIRECTDRAWSURFACE7 b, LPRECT c, DWORD d, LPDDOVERLAYFX e)
{
	if (b)
	{
		b = static_cast<m_IDirectDrawSurface7 *>(b)->GetProxyInterface();
	}

	if (e)
	{
		if (e->lpDDSAlphaSrc)
		{
			e->lpDDSAlphaSrc = static_cast<m_IDirectDrawSurface *>(e->lpDDSAlphaSrc)->GetProxyInterface();
		}
		if (e->lpDDSAlphaDest)
		{
			e->lpDDSAlphaDest = static_cast<m_IDirectDrawSurface *>(e->lpDDSAlphaDest)->GetProxyInterface();
		}
	}

	return ProxyInterface->UpdateOverlay(a, b, c, d, e);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlayDisplay(DWORD a)
{
	return ProxyInterface->UpdateOverlayDisplay(a);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlayZOrder(DWORD a, LPDIRECTDRAWSURFACE7 b)
{
	if (b)
	{
		b = static_cast<m_IDirectDrawSurface7 *>(b)->GetProxyInterface();
	}

	return ProxyInterface->UpdateOverlayZOrder(a, b);
}

HRESULT m_IDirectDrawSurfaceX::GetDDInterface(LPVOID FAR * a)
{
	HRESULT hr = ProxyInterface->GetDDInterface(a);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>((LPDIRECTDRAWSURFACE7)*a, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::PageLock(DWORD a)
{
	return ProxyInterface->PageLock(a);
}

HRESULT m_IDirectDrawSurfaceX::PageUnlock(DWORD a)
{
	return ProxyInterface->PageUnlock(a);
}

template HRESULT m_IDirectDrawSurfaceX::SetSurfaceDesc<LPDDSURFACEDESC>(LPDDSURFACEDESC a, DWORD b);
template HRESULT m_IDirectDrawSurfaceX::SetSurfaceDesc<LPDDSURFACEDESC2>(LPDDSURFACEDESC2 a, DWORD b);
template <typename T>
HRESULT m_IDirectDrawSurfaceX::SetSurfaceDesc(T a, DWORD b)
{
	DDSURFACEDESC2 Desc2;
	if (a != nullptr && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc2, *a);
		a = (T)&Desc2;
	}

	return ProxyInterface->SetSurfaceDesc((LPDDSURFACEDESC2)a, b);
}

HRESULT m_IDirectDrawSurfaceX::SetPrivateData(REFGUID a, LPVOID b, DWORD c, DWORD d)
{
	return ProxyInterface->SetPrivateData(a, b, c, d);
}

HRESULT m_IDirectDrawSurfaceX::GetPrivateData(REFGUID a, LPVOID b, LPDWORD c)
{
	return ProxyInterface->GetPrivateData(a, b, c);
}

HRESULT m_IDirectDrawSurfaceX::FreePrivateData(REFGUID a)
{
	return ProxyInterface->FreePrivateData(a);
}

HRESULT m_IDirectDrawSurfaceX::GetUniquenessValue(LPDWORD a)
{
	return ProxyInterface->GetUniquenessValue(a);
}

HRESULT m_IDirectDrawSurfaceX::ChangeUniquenessValue()
{
	return ProxyInterface->ChangeUniquenessValue();
}

HRESULT m_IDirectDrawSurfaceX::SetPriority(DWORD a)
{
	return ProxyInterface->SetPriority(a);
}

HRESULT m_IDirectDrawSurfaceX::GetPriority(LPDWORD a)
{
	return ProxyInterface->GetPriority(a);
}

HRESULT m_IDirectDrawSurfaceX::SetLOD(DWORD a)
{
	return ProxyInterface->SetLOD(a);
}

HRESULT m_IDirectDrawSurfaceX::GetLOD(LPDWORD a)
{
	return ProxyInterface->GetLOD(a);
}
