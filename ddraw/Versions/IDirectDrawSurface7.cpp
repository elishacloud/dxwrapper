/**
* Copyright (C) 2025 Elisha Riedlinger
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

HRESULT m_IDirectDrawSurface7::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
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

ULONG m_IDirectDrawSurface7::AddRef()
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

ULONG m_IDirectDrawSurface7::Release()
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

HRESULT m_IDirectDrawSurface7::AddAttachedSurface(LPDIRECTDRAWSURFACE7 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->AddAttachedSurface(a, DirectXVersion);
}

HRESULT m_IDirectDrawSurface7::AddOverlayDirtyRect(LPRECT a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->AddOverlayDirtyRect(a);
}

HRESULT m_IDirectDrawSurface7::Blt(LPRECT a, LPDIRECTDRAWSURFACE7 b, LPRECT c, DWORD d, LPDDBLTFX e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Blt(a, b, c, d, e, MipMapLevel);
}

HRESULT m_IDirectDrawSurface7::BltBatch(LPDDBLTBATCH a, DWORD b, DWORD c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->BltBatch(a, b, c, MipMapLevel);
}

HRESULT m_IDirectDrawSurface7::BltFast(DWORD a, DWORD b, LPDIRECTDRAWSURFACE7 c, LPRECT d, DWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->BltFast(a, b, c, d, e, MipMapLevel);
}

HRESULT m_IDirectDrawSurface7::DeleteAttachedSurface(DWORD a, LPDIRECTDRAWSURFACE7 b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DeleteAttachedSurface(a, b);
}

HRESULT m_IDirectDrawSurface7::EnumAttachedSurfaces(LPVOID a, LPDDENUMSURFACESCALLBACK7 b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EnumAttachedSurfaces2(a, b, MipMapLevel, DirectXVersion);
}

HRESULT m_IDirectDrawSurface7::EnumOverlayZOrders(DWORD a, LPVOID b, LPDDENUMSURFACESCALLBACK7 c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EnumOverlayZOrders2(a, b, c, DirectXVersion);
}

HRESULT m_IDirectDrawSurface7::Flip(LPDIRECTDRAWSURFACE7 a, DWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Flip(a, b, DirectXVersion);
}

HRESULT m_IDirectDrawSurface7::GetAttachedSurface(LPDDSCAPS2 a, LPDIRECTDRAWSURFACE7 FAR * b)
{
	if (!ProxyInterface)
	{
		if (b)
		{
			*b = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetAttachedSurface2(a, b, MipMapLevel, DirectXVersion);
}

HRESULT m_IDirectDrawSurface7::GetBltStatus(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetBltStatus(a);
}

HRESULT m_IDirectDrawSurface7::GetCaps(LPDDSCAPS2 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetCaps2(a);
}

HRESULT m_IDirectDrawSurface7::GetClipper(LPDIRECTDRAWCLIPPER FAR * a)
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

HRESULT m_IDirectDrawSurface7::GetColorKey(DWORD a, LPDDCOLORKEY b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetColorKey(a, b);
}

HRESULT m_IDirectDrawSurface7::GetDC(HDC FAR * a)
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

HRESULT m_IDirectDrawSurface7::GetFlipStatus(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetFlipStatus(a, false);
}

HRESULT m_IDirectDrawSurface7::GetOverlayPosition(LPLONG a, LPLONG b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetOverlayPosition(a, b);
}

HRESULT m_IDirectDrawSurface7::GetPalette(LPDIRECTDRAWPALETTE FAR * a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetPalette(a);
}

HRESULT m_IDirectDrawSurface7::GetPixelFormat(LPDDPIXELFORMAT a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetPixelFormat(a);
}

HRESULT m_IDirectDrawSurface7::GetSurfaceDesc(LPDDSURFACEDESC2 a)
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
	return ProxyInterface->GetSurfaceDesc2(a, MipMapLevel, DirectXVersion);
}

HRESULT m_IDirectDrawSurface7::Initialize(LPDIRECTDRAW a, LPDDSURFACEDESC2 b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Initialize2(a, b);
}

HRESULT m_IDirectDrawSurface7::IsLost()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->IsLost();
}

HRESULT m_IDirectDrawSurface7::Lock(LPRECT a, LPDDSURFACEDESC2 b, DWORD c, HANDLE d)
{
	if (!ProxyInterface)
	{
		if (b)
		{
			b->lpSurface = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Lock2(a, b, c, d, MipMapLevel, DirectXVersion);
}

HRESULT m_IDirectDrawSurface7::ReleaseDC(HDC a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->ReleaseDC(a);
}

HRESULT m_IDirectDrawSurface7::Restore()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Restore();
}

HRESULT m_IDirectDrawSurface7::SetClipper(LPDIRECTDRAWCLIPPER a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetClipper(a);
}

HRESULT m_IDirectDrawSurface7::SetColorKey(DWORD a, LPDDCOLORKEY b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetColorKey(a, b);
}

HRESULT m_IDirectDrawSurface7::SetOverlayPosition(LONG a, LONG b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetOverlayPosition(a, b);
}

HRESULT m_IDirectDrawSurface7::SetPalette(LPDIRECTDRAWPALETTE a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetPalette(a);
}

HRESULT m_IDirectDrawSurface7::Unlock(LPRECT a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Unlock(a, MipMapLevel);
}

HRESULT m_IDirectDrawSurface7::UpdateOverlay(LPRECT a, LPDIRECTDRAWSURFACE7 b, LPRECT c, DWORD d, LPDDOVERLAYFX e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->UpdateOverlay(a, b, c, d, e);
}

HRESULT m_IDirectDrawSurface7::UpdateOverlayDisplay(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->UpdateOverlayDisplay(a);
}

HRESULT m_IDirectDrawSurface7::UpdateOverlayZOrder(DWORD a, LPDIRECTDRAWSURFACE7 b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->UpdateOverlayZOrder(a, b);
}

HRESULT m_IDirectDrawSurface7::GetDDInterface(LPVOID FAR * a)
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

HRESULT m_IDirectDrawSurface7::PageLock(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->PageLock(a);
}

HRESULT m_IDirectDrawSurface7::PageUnlock(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->PageUnlock(a);
}

HRESULT m_IDirectDrawSurface7::SetSurfaceDesc(LPDDSURFACEDESC2 a, DWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetSurfaceDesc2(a, b);
}

HRESULT m_IDirectDrawSurface7::SetPrivateData(REFGUID a, LPVOID b, DWORD c, DWORD d)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetPrivateData(a, b, c, d);
}

HRESULT m_IDirectDrawSurface7::GetPrivateData(REFGUID a, LPVOID b, LPDWORD c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetPrivateData(a, b, c);
}

HRESULT m_IDirectDrawSurface7::FreePrivateData(REFGUID a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->FreePrivateData(a);
}

HRESULT m_IDirectDrawSurface7::GetUniquenessValue(LPDWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetUniquenessValue(a);
}

HRESULT m_IDirectDrawSurface7::ChangeUniquenessValue()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->ChangeUniquenessValue();
}

HRESULT m_IDirectDrawSurface7::SetPriority(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetPriority(a);
}

HRESULT m_IDirectDrawSurface7::GetPriority(LPDWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetPriority(a);
}

HRESULT m_IDirectDrawSurface7::SetLOD(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetLOD(a);
}

HRESULT m_IDirectDrawSurface7::GetLOD(LPDWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetLOD(a);
}
