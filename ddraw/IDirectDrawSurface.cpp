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

HRESULT m_IDirectDrawSurface::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectDrawSurface::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawSurface::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		delete this;
	}

	return x;
}

HRESULT m_IDirectDrawSurface::AddAttachedSurface(LPDIRECTDRAWSURFACE a)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawSurface *>(a)->GetProxyInterface();
	}

	return ProxyInterface->AddAttachedSurface(a);
}

HRESULT m_IDirectDrawSurface::AddOverlayDirtyRect(LPRECT a)
{
	return ProxyInterface->AddOverlayDirtyRect(a);
}

HRESULT m_IDirectDrawSurface::Blt(LPRECT a, LPDIRECTDRAWSURFACE b, LPRECT c, DWORD d, LPDDBLTFX e)
{
	if (b)
	{
		b = static_cast<m_IDirectDrawSurface *>(b)->GetProxyInterface();
	}

	return ProxyInterface->Blt(a, b, c, d, e);
}

HRESULT m_IDirectDrawSurface::BltBatch(LPDDBLTBATCH a, DWORD b, DWORD c)
{
	return ProxyInterface->BltBatch(a, b, c);
}

HRESULT m_IDirectDrawSurface::BltFast(DWORD a, DWORD b, LPDIRECTDRAWSURFACE c, LPRECT d, DWORD e)
{
	if (c)
	{
		c = static_cast<m_IDirectDrawSurface *>(c)->GetProxyInterface();
	}

	return ProxyInterface->BltFast(a, b, c, d, e);
}

HRESULT m_IDirectDrawSurface::DeleteAttachedSurface(DWORD a, LPDIRECTDRAWSURFACE b)
{
	if (b)
	{
		b = static_cast<m_IDirectDrawSurface *>(b)->GetProxyInterface();
	}

	return ProxyInterface->DeleteAttachedSurface(a, b);
}

HRESULT m_IDirectDrawSurface::EnumAttachedSurfaces(LPVOID a, LPDDENUMSURFACESCALLBACK b)
{
	m_IDirectDrawEnumSurface::SetCallback(b);

	HRESULT hr = ProxyInterface->EnumAttachedSurfaces(a, reinterpret_cast<LPDDENUMSURFACESCALLBACK>(m_IDirectDrawEnumSurface::EnumSurfaceCallback));

	m_IDirectDrawEnumSurface::ReleaseCallback();

	return hr;
}

HRESULT m_IDirectDrawSurface::EnumOverlayZOrders(DWORD a, LPVOID b, LPDDENUMSURFACESCALLBACK c)
{
	m_IDirectDrawEnumSurface::SetCallback(c);

	HRESULT hr = ProxyInterface->EnumOverlayZOrders(a, b, reinterpret_cast<LPDDENUMSURFACESCALLBACK>(m_IDirectDrawEnumSurface::EnumSurfaceCallback));

	m_IDirectDrawEnumSurface::ReleaseCallback();

	return hr;
}

HRESULT m_IDirectDrawSurface::Flip(LPDIRECTDRAWSURFACE a, DWORD b)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawSurface *>(a)->GetProxyInterface();
	}

	return ProxyInterface->Flip(a, b);
}

HRESULT m_IDirectDrawSurface::GetAttachedSurface(LPDDSCAPS a, LPDIRECTDRAWSURFACE FAR * b)
{
	HRESULT hr = ProxyInterface->GetAttachedSurface(a, b);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface>(*b);
	}

	return hr;
}

HRESULT m_IDirectDrawSurface::GetBltStatus(DWORD a)
{
	return ProxyInterface->GetBltStatus(a);
}

HRESULT m_IDirectDrawSurface::GetCaps(LPDDSCAPS a)
{
	return ProxyInterface->GetCaps(a);
}

HRESULT m_IDirectDrawSurface::GetClipper(LPDIRECTDRAWCLIPPER FAR * a)
{
	HRESULT hr = ProxyInterface->GetClipper(a);

	if (SUCCEEDED(hr))
	{
		*a= ProxyAddressLookupTable.FindAddress<m_IDirectDrawClipper>(*a);
	}

	return hr;
}

HRESULT m_IDirectDrawSurface::GetColorKey(DWORD a, LPDDCOLORKEY b)
{
	return ProxyInterface->GetColorKey(a, b);
}

HRESULT m_IDirectDrawSurface::GetDC(HDC FAR * a)
{
	return ProxyInterface->GetDC(a);
}

HRESULT m_IDirectDrawSurface::GetFlipStatus(DWORD a)
{
	return ProxyInterface->GetFlipStatus(a);
}

HRESULT m_IDirectDrawSurface::GetOverlayPosition(LPLONG a, LPLONG b)
{
	return ProxyInterface->GetOverlayPosition(a, b);
}

HRESULT m_IDirectDrawSurface::GetPalette(LPDIRECTDRAWPALETTE FAR * a)
{
	HRESULT hr = ProxyInterface->GetPalette(a);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirectDrawPalette>(*a);
	}

	return hr;
}

HRESULT m_IDirectDrawSurface::GetPixelFormat(LPDDPIXELFORMAT a)
{
	return ProxyInterface->GetPixelFormat(a);
}

HRESULT m_IDirectDrawSurface::GetSurfaceDesc(LPDDSURFACEDESC a)
{
	return ProxyInterface->GetSurfaceDesc(a);
}

HRESULT m_IDirectDrawSurface::Initialize(LPDIRECTDRAW a, LPDDSURFACEDESC b)
{
	if (a)
	{
		a = static_cast<m_IDirectDraw *>(a)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(a, b);
}

HRESULT m_IDirectDrawSurface::IsLost()
{
	return ProxyInterface->IsLost();
}

HRESULT m_IDirectDrawSurface::Lock(LPRECT a, LPDDSURFACEDESC b, DWORD c, HANDLE d)
{
	return ProxyInterface->Lock(a, b, c, d);
}

HRESULT m_IDirectDrawSurface::ReleaseDC(HDC a)
{
	return ProxyInterface->ReleaseDC(a);
}

HRESULT m_IDirectDrawSurface::Restore()
{
	return ProxyInterface->Restore();
}

HRESULT m_IDirectDrawSurface::SetClipper(LPDIRECTDRAWCLIPPER a)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawClipper *>(a)->GetProxyInterface();
	}

	return ProxyInterface->SetClipper(a);
}

HRESULT m_IDirectDrawSurface::SetColorKey(DWORD a, LPDDCOLORKEY b)
{
	return ProxyInterface->SetColorKey(a, b);
}

HRESULT m_IDirectDrawSurface::SetOverlayPosition(LONG a, LONG b)
{
	return ProxyInterface->SetOverlayPosition(a, b);
}

HRESULT m_IDirectDrawSurface::SetPalette(LPDIRECTDRAWPALETTE a)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawPalette *>(a)->GetProxyInterface();
	}

	return ProxyInterface->SetPalette(a);
}

HRESULT m_IDirectDrawSurface::Unlock(LPVOID a)
{
	return ProxyInterface->Unlock(a);
}

HRESULT m_IDirectDrawSurface::UpdateOverlay(LPRECT a, LPDIRECTDRAWSURFACE b, LPRECT c, DWORD d, LPDDOVERLAYFX e)
{
	if (b)
	{
		b = static_cast<m_IDirectDrawSurface *>(b)->GetProxyInterface();
	}

	return ProxyInterface->UpdateOverlay(a, b, c, d, e);
}

HRESULT m_IDirectDrawSurface::UpdateOverlayDisplay(DWORD a)
{
	return ProxyInterface->UpdateOverlayDisplay(a);
}

HRESULT m_IDirectDrawSurface::UpdateOverlayZOrder(DWORD a, LPDIRECTDRAWSURFACE b)
{
	if (b)
	{
		b = static_cast<m_IDirectDrawSurface *>(b)->GetProxyInterface();
	}

	return ProxyInterface->UpdateOverlayZOrder(a, b);
}
