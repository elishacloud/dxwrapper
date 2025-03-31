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

HRESULT m_IDirectDraw3::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	if (!ProxyInterface)
	{
		if (ppvObj)
		{
			*ppvObj = nullptr;
		}
		return E_NOINTERFACE;
	}
	return ProxyInterface->QueryInterface(ReplaceIIDUnknown(riid, WrapperID), ppvObj, DirectXVersion);
}

ULONG m_IDirectDraw3::AddRef()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirectDraw3::Release()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirectDraw3::Compact()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Compact();
}

HRESULT m_IDirectDraw3::CreateClipper(DWORD a, LPDIRECTDRAWCLIPPER FAR * b, IUnknown FAR * c)
{
	if (!ProxyInterface)
	{
		if (b)
		{
			*b = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->CreateClipper(a, b, c, DirectXVersion);
}

HRESULT m_IDirectDraw3::CreatePalette(DWORD a, LPPALETTEENTRY b, LPDIRECTDRAWPALETTE FAR * c, IUnknown FAR * d)
{
	if (!ProxyInterface)
	{
		if (c)
		{
			*c = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->CreatePalette(a, b, c, d, DirectXVersion);
}

HRESULT m_IDirectDraw3::CreateSurface(LPDDSURFACEDESC a, LPDIRECTDRAWSURFACE FAR * b, IUnknown FAR * c)
{
	if (!ProxyInterface)
	{
		if (b)
		{
			*b = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->CreateSurface(a, (LPDIRECTDRAWSURFACE7*)b, c, DirectXVersion);
}

HRESULT m_IDirectDraw3::DuplicateSurface(LPDIRECTDRAWSURFACE a, LPDIRECTDRAWSURFACE FAR * b)
{
	if (!ProxyInterface)
	{
		if (b)
		{
			*b = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DuplicateSurface((LPDIRECTDRAWSURFACE7)a, (LPDIRECTDRAWSURFACE7*)b, DirectXVersion);
}

HRESULT m_IDirectDraw3::EnumDisplayModes(DWORD a, LPDDSURFACEDESC b, LPVOID c, LPDDENUMMODESCALLBACK d)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EnumDisplayModes(a, b, c, d, DirectXVersion);
}

HRESULT m_IDirectDraw3::EnumSurfaces(DWORD a, LPDDSURFACEDESC b, LPVOID c, LPDDENUMSURFACESCALLBACK d)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EnumSurfaces(a, b, c, d, DirectXVersion);
}

HRESULT m_IDirectDraw3::FlipToGDISurface()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->FlipToGDISurface();
}

HRESULT m_IDirectDraw3::GetCaps(LPDDCAPS a, LPDDCAPS b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetCaps(a, b);
}

HRESULT m_IDirectDraw3::GetDisplayMode(LPDDSURFACEDESC a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetDisplayMode(a);
}

HRESULT m_IDirectDraw3::GetFourCCCodes(LPDWORD a, LPDWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetFourCCCodes(a, b);
}

HRESULT m_IDirectDraw3::GetGDISurface(LPDIRECTDRAWSURFACE FAR * a)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetGDISurface((LPDIRECTDRAWSURFACE7*)a, DirectXVersion);
}

HRESULT m_IDirectDraw3::GetMonitorFrequency(LPDWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetMonitorFrequency(a);
}

HRESULT m_IDirectDraw3::GetScanLine(LPDWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetScanLine(a);
}

HRESULT m_IDirectDraw3::GetVerticalBlankStatus(LPBOOL a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetVerticalBlankStatus(a);
}

HRESULT m_IDirectDraw3::Initialize(GUID FAR * a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Initialize(a);
}

HRESULT m_IDirectDraw3::RestoreDisplayMode()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->RestoreDisplayMode();
}

HRESULT m_IDirectDraw3::SetCooperativeLevel(HWND a, DWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetCooperativeLevel(a, b, DirectXVersion);
}

HRESULT m_IDirectDraw3::SetDisplayMode(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetDisplayMode(a, b, c, d, e);
}

HRESULT m_IDirectDraw3::WaitForVerticalBlank(DWORD a, HANDLE b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->WaitForVerticalBlank(a, b);
}

HRESULT m_IDirectDraw3::GetAvailableVidMem(LPDDSCAPS a, LPDWORD b, LPDWORD c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetAvailableVidMem(a, b, c);
}

HRESULT m_IDirectDraw3::GetSurfaceFromDC(HDC a, IDirectDrawSurface * * b)
{
	if (!ProxyInterface)
	{
		if (b)
		{
			*b = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetSurfaceFromDC(a, (LPDIRECTDRAWSURFACE7*)b, DirectXVersion);
}
