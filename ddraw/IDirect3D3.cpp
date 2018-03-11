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

HRESULT m_IDirect3D3::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	HRESULT hr = ProxyInterface->QueryInterface(ConvertREFIID(riid), ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirect3D3::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3D3::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		delete this;
	}

	return x;
}

HRESULT m_IDirect3D3::EnumDevices(LPD3DENUMDEVICESCALLBACK a, LPVOID b)
{
	return ProxyInterface->EnumDevices(a, b);
}

HRESULT m_IDirect3D3::CreateLight(LPDIRECT3DLIGHT * a, LPUNKNOWN b)
{
	HRESULT hr = ProxyInterface->CreateLight(a, b);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirect3DLight>(*a);
	}

	return hr;
}

HRESULT m_IDirect3D3::CreateMaterial(LPDIRECT3DMATERIAL3 * a, LPUNKNOWN b)
{
	HRESULT hr = ProxyInterface->CreateMaterial(a, b);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirect3DMaterial3>(*a);
	}

	return hr;
}

HRESULT m_IDirect3D3::CreateViewport(LPDIRECT3DVIEWPORT3 * a, LPUNKNOWN b)
{
	HRESULT hr = ProxyInterface->CreateViewport(a, b);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirect3DViewport3>(*a);
	}

	return hr;
}

HRESULT m_IDirect3D3::FindDevice(LPD3DFINDDEVICESEARCH a, LPD3DFINDDEVICERESULT b)
{
	return ProxyInterface->FindDevice(a, b);
}

HRESULT m_IDirect3D3::CreateDevice(REFCLSID a, LPDIRECTDRAWSURFACE4 b, LPDIRECT3DDEVICE3 * c, LPUNKNOWN d)
{
	if (b)
	{
		b = static_cast<m_IDirectDrawSurface4 *>(b)->GetProxyInterface();
	}

	HRESULT hr = ProxyInterface->CreateDevice(a, b, c, d);

	if (SUCCEEDED(hr))
	{
		*c = ProxyAddressLookupTable.FindAddress<m_IDirect3DDevice3>(*c);
	}

	return hr;
}

HRESULT m_IDirect3D3::CreateVertexBuffer(LPD3DVERTEXBUFFERDESC a, LPDIRECT3DVERTEXBUFFER * b, DWORD c, LPUNKNOWN d)
{
	HRESULT hr = ProxyInterface->CreateVertexBuffer(a, b, c, d);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirect3DVertexBuffer>(*b);
	}

	return hr;
}

HRESULT m_IDirect3D3::EnumZBufferFormats(REFCLSID a, LPD3DENUMPIXELFORMATSCALLBACK b, LPVOID c)
{
	return ProxyInterface->EnumZBufferFormats(a, b, c);
}

HRESULT m_IDirect3D3::EvictManagedTextures()
{
	return ProxyInterface->EvictManagedTextures();
}
