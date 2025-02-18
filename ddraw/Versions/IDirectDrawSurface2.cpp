/**
* Copyright (C) 2024 Elisha Riedlinger
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
	if (!ProxyInterface)
	{
		if (ppvObj)
		{
			*ppvObj = nullptr;
		}
		return E_NOINTERFACE;
	}
	if (ppvObj && riid == IID_GetMipMapLevel)
	{
		*ppvObj = (void*)MipMapLevel;
		return DD_OK;
	}
	if (ppvObj && MipMapLevel && (riid == WrapperID || riid == IID_IUnknown))
	{
		*ppvObj = this;

		AddRef();

		return DD_OK;
	}
	return ProxyInterface->QueryInterface(ReplaceIIDUnknown(riid, WrapperID), ppvObj, DirectXVersion);
}

ULONG m_IDirectDrawSurface2::AddRef()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	if (MipMapLevel)
	{
		return InterlockedIncrement(&RefCount);
	}
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirectDrawSurface2::Release()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	if (MipMapLevel)
	{
		return (InterlockedCompareExchange(&RefCount, 0, 0)) ? InterlockedDecrement(&RefCount) : 0;
	}
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirectDrawSurface2::AddAttachedSurface(LPDIRECTDRAWSURFACE2 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->AddAttachedSurface((LPDIRECTDRAWSURFACE7)a, DirectXVersion);
}

HRESULT m_IDirectDrawSurface2::AddOverlayDirtyRect(LPRECT a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->AddOverlayDirtyRect(a);
}

HRESULT m_IDirectDrawSurface2::Blt(LPRECT a, LPDIRECTDRAWSURFACE2 b, LPRECT c, DWORD d, LPDDBLTFX e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Blt(a, (LPDIRECTDRAWSURFACE7)b, c, d, e, MipMapLevel);
}

HRESULT m_IDirectDrawSurface2::BltBatch(LPDDBLTBATCH a, DWORD b, DWORD c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->BltBatch(a, b, c, MipMapLevel);
}

HRESULT m_IDirectDrawSurface2::BltFast(DWORD a, DWORD b, LPDIRECTDRAWSURFACE2 c, LPRECT d, DWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->BltFast(a, b, (LPDIRECTDRAWSURFACE7)c, d, e, MipMapLevel);
}

HRESULT m_IDirectDrawSurface2::DeleteAttachedSurface(DWORD a, LPDIRECTDRAWSURFACE2 b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DeleteAttachedSurface(a, (LPDIRECTDRAWSURFACE7)b);
}

HRESULT m_IDirectDrawSurface2::EnumAttachedSurfaces(LPVOID a, LPDDENUMSURFACESCALLBACK b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EnumAttachedSurfaces(a, b, MipMapLevel, DirectXVersion);
}

HRESULT m_IDirectDrawSurface2::EnumOverlayZOrders(DWORD a, LPVOID b, LPDDENUMSURFACESCALLBACK c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EnumOverlayZOrders(a, b, c, DirectXVersion);
}

HRESULT m_IDirectDrawSurface2::Flip(LPDIRECTDRAWSURFACE2 a, DWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Flip((LPDIRECTDRAWSURFACE7)a, b, DirectXVersion);
}

HRESULT m_IDirectDrawSurface2::GetAttachedSurface(LPDDSCAPS a, LPDIRECTDRAWSURFACE2 FAR * b)
{
	if (!ProxyInterface)
	{
		if (b)
		{
			*b = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetAttachedSurface(a, (LPDIRECTDRAWSURFACE7*)b, MipMapLevel, DirectXVersion);
}

HRESULT m_IDirectDrawSurface2::GetBltStatus(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetBltStatus(a);
}

HRESULT m_IDirectDrawSurface2::GetCaps(LPDDSCAPS a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetCaps(a);
}

HRESULT m_IDirectDrawSurface2::GetClipper(LPDIRECTDRAWCLIPPER FAR * a)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetClipper(a);
}

HRESULT m_IDirectDrawSurface2::GetColorKey(DWORD a, LPDDCOLORKEY b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetColorKey(a, b);
}

HRESULT m_IDirectDrawSurface2::GetDC(HDC FAR * a)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetDC(a, MipMapLevel);
}

HRESULT m_IDirectDrawSurface2::GetFlipStatus(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetFlipStatus(a, false);
}

HRESULT m_IDirectDrawSurface2::GetOverlayPosition(LPLONG a, LPLONG b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetOverlayPosition(a, b);
}

HRESULT m_IDirectDrawSurface2::GetPalette(LPDIRECTDRAWPALETTE FAR * a)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetPalette(a);
}

HRESULT m_IDirectDrawSurface2::GetPixelFormat(LPDDPIXELFORMAT a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetPixelFormat(a);
}

HRESULT m_IDirectDrawSurface2::GetSurfaceDesc(LPDDSURFACEDESC a)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			if (a->dwSize == sizeof(DDSURFACEDESC))
			{
				ZeroMemory(a, sizeof(DDSURFACEDESC));
				a->dwSize = sizeof(DDSURFACEDESC);
			}
			else if (a->dwSize == sizeof(DDSURFACEDESC2))
			{
				ZeroMemory(a, sizeof(DDSURFACEDESC2));
				a->dwSize = sizeof(DDSURFACEDESC);
			}
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetSurfaceDesc(a, MipMapLevel, DirectXVersion);
}

HRESULT m_IDirectDrawSurface2::Initialize(LPDIRECTDRAW a, LPDDSURFACEDESC b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Initialize(a, b);
}

HRESULT m_IDirectDrawSurface2::IsLost()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->IsLost();
}

HRESULT m_IDirectDrawSurface2::Lock(LPRECT a, LPDDSURFACEDESC b, DWORD c, HANDLE d)
{
	if (!ProxyInterface)
	{
		if (b)
		{
			b->lpSurface = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Lock(a, b, c, d, MipMapLevel, DirectXVersion);
}

HRESULT m_IDirectDrawSurface2::ReleaseDC(HDC a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->ReleaseDC(a);
}

HRESULT m_IDirectDrawSurface2::Restore()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Restore();
}

HRESULT m_IDirectDrawSurface2::SetClipper(LPDIRECTDRAWCLIPPER a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetClipper(a);
}

HRESULT m_IDirectDrawSurface2::SetColorKey(DWORD a, LPDDCOLORKEY b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetColorKey(a, b);
}

HRESULT m_IDirectDrawSurface2::SetOverlayPosition(LONG a, LONG b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetOverlayPosition(a, b);
}

HRESULT m_IDirectDrawSurface2::SetPalette(LPDIRECTDRAWPALETTE a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetPalette(a);
}

HRESULT m_IDirectDrawSurface2::Unlock(LPVOID a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Unlock((LPRECT)a, MipMapLevel);
}

HRESULT m_IDirectDrawSurface2::UpdateOverlay(LPRECT a, LPDIRECTDRAWSURFACE2 b, LPRECT c, DWORD d, LPDDOVERLAYFX e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->UpdateOverlay(a, (LPDIRECTDRAWSURFACE7)b, c, d, e);
}

HRESULT m_IDirectDrawSurface2::UpdateOverlayDisplay(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->UpdateOverlayDisplay(a);
}

HRESULT m_IDirectDrawSurface2::UpdateOverlayZOrder(DWORD a, LPDIRECTDRAWSURFACE2 b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->UpdateOverlayZOrder(a, (LPDIRECTDRAWSURFACE7)b);
}

HRESULT m_IDirectDrawSurface2::GetDDInterface(LPVOID FAR * a)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetDDInterface(a, DirectXVersion);
}

HRESULT m_IDirectDrawSurface2::PageLock(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->PageLock(a);
}

HRESULT m_IDirectDrawSurface2::PageUnlock(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->PageUnlock(a);
}
