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

HRESULT m_IDirectDrawSurface3::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectDrawSurface3::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawSurface3::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		delete this;
	}

	return x;
}

HRESULT m_IDirectDrawSurface3::AddAttachedSurface(LPDIRECTDRAWSURFACE3 a)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawSurface3 *>(a)->GetProxyInterface();
	}

	return ProxyInterface->AddAttachedSurface(a);
}

HRESULT m_IDirectDrawSurface3::AddOverlayDirtyRect(LPRECT a)
{
	return ProxyInterface->AddOverlayDirtyRect(a);
}

HRESULT m_IDirectDrawSurface3::Blt(LPRECT a, LPDIRECTDRAWSURFACE3 b, LPRECT c, DWORD d, LPDDBLTFX e)
{
	if (b)
	{
		b = static_cast<m_IDirectDrawSurface3 *>(b)->GetProxyInterface();
	}

	return ProxyInterface->Blt(a, b, c, d, e);
}

HRESULT m_IDirectDrawSurface3::BltBatch(LPDDBLTBATCH a, DWORD b, DWORD c)
{
	return ProxyInterface->BltBatch(a, b, c);
}

HRESULT m_IDirectDrawSurface3::BltFast(DWORD a, DWORD b, LPDIRECTDRAWSURFACE3 c, LPRECT d, DWORD e)
{
	if (c)
	{
		c = static_cast<m_IDirectDrawSurface3 *>(c)->GetProxyInterface();
	}

	return ProxyInterface->BltFast(a, b, c, d, e);
}

HRESULT m_IDirectDrawSurface3::DeleteAttachedSurface(DWORD a, LPDIRECTDRAWSURFACE3 b)
{
	if (b)
	{
		b = static_cast<m_IDirectDrawSurface3 *>(b)->GetProxyInterface();
	}

	return ProxyInterface->DeleteAttachedSurface(a, b);
}

HRESULT m_IDirectDrawSurface3::EnumAttachedSurfaces(LPVOID a, LPDDENUMSURFACESCALLBACK b)
{
	m_IDirectDrawEnumSurface::SetCallback(b);

	HRESULT hr = ProxyInterface->EnumAttachedSurfaces(a, reinterpret_cast<LPDDENUMSURFACESCALLBACK>(m_IDirectDrawEnumSurface::EnumSurfaceCallback));

	m_IDirectDrawEnumSurface::ReleaseCallback();

	return hr;
}

HRESULT m_IDirectDrawSurface3::EnumOverlayZOrders(DWORD a, LPVOID b, LPDDENUMSURFACESCALLBACK c)
{
	m_IDirectDrawEnumSurface::SetCallback(c);

	HRESULT hr = ProxyInterface->EnumOverlayZOrders(a, b, reinterpret_cast<LPDDENUMSURFACESCALLBACK>(m_IDirectDrawEnumSurface::EnumSurfaceCallback));

	m_IDirectDrawEnumSurface::ReleaseCallback();

	return hr;
}

HRESULT m_IDirectDrawSurface3::Flip(LPDIRECTDRAWSURFACE3 a, DWORD b)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawSurface3 *>(a)->GetProxyInterface();
	}

	return ProxyInterface->Flip(a, b);
}

HRESULT m_IDirectDrawSurface3::GetAttachedSurface(LPDDSCAPS a, LPDIRECTDRAWSURFACE3 FAR * b)
{
	HRESULT hr = ProxyInterface->GetAttachedSurface(a, b);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface3>(*b);
	}

	return hr;
}

HRESULT m_IDirectDrawSurface3::GetBltStatus(DWORD a)
{
	return ProxyInterface->GetBltStatus(a);
}

HRESULT m_IDirectDrawSurface3::GetCaps(LPDDSCAPS a)
{
	return ProxyInterface->GetCaps(a);
}

HRESULT m_IDirectDrawSurface3::GetClipper(LPDIRECTDRAWCLIPPER FAR * a)
{
	HRESULT hr = ProxyInterface->GetClipper(a);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirectDrawClipper>(*a);
	}

	return hr;
}

HRESULT m_IDirectDrawSurface3::GetColorKey(DWORD a, LPDDCOLORKEY b)
{
	return ProxyInterface->GetColorKey(a, b);
}

HRESULT m_IDirectDrawSurface3::GetDC(HDC FAR * a)
{
	return ProxyInterface->GetDC(a);
}

HRESULT m_IDirectDrawSurface3::GetFlipStatus(DWORD a)
{
	return ProxyInterface->GetFlipStatus(a);
}

HRESULT m_IDirectDrawSurface3::GetOverlayPosition(LPLONG a, LPLONG b)
{
	return ProxyInterface->GetOverlayPosition(a, b);
}

HRESULT m_IDirectDrawSurface3::GetPalette(LPDIRECTDRAWPALETTE FAR * a)
{
	HRESULT hr = ProxyInterface->GetPalette(a);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirectDrawPalette>(*a);
	}

	return hr;
}

HRESULT m_IDirectDrawSurface3::GetPixelFormat(LPDDPIXELFORMAT a)
{
	return ProxyInterface->GetPixelFormat(a);
}

HRESULT m_IDirectDrawSurface3::GetSurfaceDesc(LPDDSURFACEDESC a)
{
	return ProxyInterface->GetSurfaceDesc(a);
}

HRESULT m_IDirectDrawSurface3::Initialize(LPDIRECTDRAW a, LPDDSURFACEDESC b)
{
	if (a)
	{
		a = static_cast<m_IDirectDraw *>(a)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(a, b);
}

HRESULT m_IDirectDrawSurface3::IsLost()
{
	return ProxyInterface->IsLost();
}

HRESULT m_IDirectDrawSurface3::Lock(LPRECT a, LPDDSURFACEDESC b, DWORD c, HANDLE d)
{
	return ProxyInterface->Lock(a, b, c, d);
}

HRESULT m_IDirectDrawSurface3::ReleaseDC(HDC a)
{
	return ProxyInterface->ReleaseDC(a);
}

HRESULT m_IDirectDrawSurface3::Restore()
{
	return ProxyInterface->Restore();
}

HRESULT m_IDirectDrawSurface3::SetClipper(LPDIRECTDRAWCLIPPER a)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawClipper *>(a)->GetProxyInterface();
	}

	return ProxyInterface->SetClipper(a);
}

HRESULT m_IDirectDrawSurface3::SetColorKey(DWORD a, LPDDCOLORKEY b)
{
	return ProxyInterface->SetColorKey(a, b);
}

HRESULT m_IDirectDrawSurface3::SetOverlayPosition(LONG a, LONG b)
{
	return ProxyInterface->SetOverlayPosition(a, b);
}

HRESULT m_IDirectDrawSurface3::SetPalette(LPDIRECTDRAWPALETTE a)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawPalette *>(a)->GetProxyInterface();
	}

	return ProxyInterface->SetPalette(a);
}

HRESULT m_IDirectDrawSurface3::Unlock(LPVOID a)
{
	return ProxyInterface->Unlock(a);
}

HRESULT m_IDirectDrawSurface3::UpdateOverlay(LPRECT a, LPDIRECTDRAWSURFACE3 b, LPRECT c, DWORD d, LPDDOVERLAYFX e)
{
	if (b)
	{
		b = static_cast<m_IDirectDrawSurface3 *>(b)->GetProxyInterface();
	}

	return ProxyInterface->UpdateOverlay(a, b, c, d, e);
}

HRESULT m_IDirectDrawSurface3::UpdateOverlayDisplay(DWORD a)
{
	return ProxyInterface->UpdateOverlayDisplay(a);
}

HRESULT m_IDirectDrawSurface3::UpdateOverlayZOrder(DWORD a, LPDIRECTDRAWSURFACE3 b)
{
	if (b)
	{
		b = static_cast<m_IDirectDrawSurface3 *>(b)->GetProxyInterface();
	}

	return ProxyInterface->UpdateOverlayZOrder(a, b);
}

HRESULT m_IDirectDrawSurface3::GetDDInterface(LPVOID FAR * a)
{
	HRESULT hr = ProxyInterface->GetDDInterface(a);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirectDraw3>(*a);
	}

	return hr;
}

HRESULT m_IDirectDrawSurface3::PageLock(DWORD a)
{
	return ProxyInterface->PageLock(a);
}

HRESULT m_IDirectDrawSurface3::PageUnlock(DWORD a)
{
	return ProxyInterface->PageUnlock(a);
}

HRESULT m_IDirectDrawSurface3::SetSurfaceDesc(LPDDSURFACEDESC a, DWORD b)
{
	return ProxyInterface->SetSurfaceDesc(a, b);
}
