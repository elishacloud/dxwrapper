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

HRESULT m_IDirect3D2::QueryInterface(REFIID riid, LPVOID * ppvObj)
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

ULONG m_IDirect3D2::AddRef()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirect3D2::Release()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirect3D2::EnumDevices(LPD3DENUMDEVICESCALLBACK a, LPVOID b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EnumDevices(a, b, DirectXVersion);
}

HRESULT m_IDirect3D2::CreateLight(LPDIRECT3DLIGHT * a, IUnknown * b)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->CreateLight(a, b);
}

HRESULT m_IDirect3D2::CreateMaterial(LPDIRECT3DMATERIAL2 * a, IUnknown * b)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->CreateMaterial((LPDIRECT3DMATERIAL3*)a, b, DirectXVersion);
}

HRESULT m_IDirect3D2::CreateViewport(LPDIRECT3DVIEWPORT2 * a, IUnknown * b)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->CreateViewport((LPDIRECT3DVIEWPORT3*)a, b, DirectXVersion);
}

HRESULT m_IDirect3D2::FindDevice(LPD3DFINDDEVICESEARCH a, LPD3DFINDDEVICERESULT b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->FindDevice(a, b, DirectXVersion);
}

HRESULT m_IDirect3D2::CreateDevice(REFCLSID a, LPDIRECTDRAWSURFACE b, LPDIRECT3DDEVICE2 * c)
{
	if (!ProxyInterface)
	{
		if (c)
		{
			*c = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->CreateDevice(a, (LPDIRECTDRAWSURFACE7)b, (LPDIRECT3DDEVICE7*)c, nullptr, DirectXVersion);
}
