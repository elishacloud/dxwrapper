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

HRESULT m_IDirect3D7::QueryInterface(REFIID riid, LPVOID * ppvObj)
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

ULONG m_IDirect3D7::AddRef()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirect3D7::Release()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirect3D7::EnumDevices(LPD3DENUMDEVICESCALLBACK7 a, LPVOID b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EnumDevices7(a, nullptr, b, DirectXVersion);
}

HRESULT m_IDirect3D7::CreateDevice(REFCLSID a, LPDIRECTDRAWSURFACE7 b, LPDIRECT3DDEVICE7 * c)
{
	if (!ProxyInterface)
	{
		if (c)
		{
			*c = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->CreateDevice(a, b, c, nullptr, DirectXVersion);
}

HRESULT m_IDirect3D7::CreateVertexBuffer(LPD3DVERTEXBUFFERDESC a, LPDIRECT3DVERTEXBUFFER7 * b, DWORD c)
{
	if (!ProxyInterface)
	{
		if (b)
		{
			*b = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->CreateVertexBuffer(a, b, c, nullptr, DirectXVersion);
}

HRESULT m_IDirect3D7::EnumZBufferFormats(REFCLSID a, LPD3DENUMPIXELFORMATSCALLBACK b, LPVOID c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EnumZBufferFormats(a, b, c);
}

HRESULT m_IDirect3D7::EvictManagedTextures()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EvictManagedTextures();
}
