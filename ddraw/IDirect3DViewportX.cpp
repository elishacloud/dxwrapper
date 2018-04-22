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

HRESULT m_IDirect3DViewportX::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, WrapperInterface);
}

ULONG m_IDirect3DViewportX::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DViewportX::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		WrapperInterface->DeleteMe();
	}

	return x;
}

HRESULT m_IDirect3DViewportX::Initialize(LPDIRECT3D a)
{
	if (a)
	{
		a = static_cast<m_IDirect3D *>(a)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(a);
}

HRESULT m_IDirect3DViewportX::GetViewport(LPD3DVIEWPORT a)
{
	return ProxyInterface->GetViewport(a);
}

HRESULT m_IDirect3DViewportX::SetViewport(LPD3DVIEWPORT a)
{
	return ProxyInterface->SetViewport(a);
}

HRESULT m_IDirect3DViewportX::TransformVertices(DWORD a, LPD3DTRANSFORMDATA b, DWORD c, LPDWORD d)
{
	return ProxyInterface->TransformVertices(a, b, c, d);
}

HRESULT m_IDirect3DViewportX::LightElements(DWORD a, LPD3DLIGHTDATA b)
{
	return ProxyInterface->LightElements(a, b);
}

HRESULT m_IDirect3DViewportX::SetBackground(D3DMATERIALHANDLE a)
{
	return ProxyInterface->SetBackground(a);
}

HRESULT m_IDirect3DViewportX::GetBackground(LPD3DMATERIALHANDLE a, LPBOOL b)
{
	return ProxyInterface->GetBackground(a, b);
}

HRESULT m_IDirect3DViewportX::SetBackgroundDepth(LPDIRECTDRAWSURFACE a)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawSurface *>(a)->GetProxyInterface();
	}

	return ProxyInterface->SetBackgroundDepth(a);
}

HRESULT m_IDirect3DViewportX::GetBackgroundDepth(LPDIRECTDRAWSURFACE * a, LPBOOL b)
{
	HRESULT hr = ProxyInterface->GetBackgroundDepth(a, b);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface>(*a);
	}

	return hr;
}

HRESULT m_IDirect3DViewportX::Clear(DWORD a, LPD3DRECT b, DWORD c)
{
	return ProxyInterface->Clear(a, b, c);
}

HRESULT m_IDirect3DViewportX::AddLight(LPDIRECT3DLIGHT a)
{
	if (a)
	{
		a = static_cast<m_IDirect3DLight *>(a)->GetProxyInterface();
	}

	return ProxyInterface->AddLight(a);
}

HRESULT m_IDirect3DViewportX::DeleteLight(LPDIRECT3DLIGHT a)
{
	if (a)
	{
		a = static_cast<m_IDirect3DLight *>(a)->GetProxyInterface();
	}

	return ProxyInterface->DeleteLight(a);
}

HRESULT m_IDirect3DViewportX::NextLight(LPDIRECT3DLIGHT a, LPDIRECT3DLIGHT * b, DWORD c)
{
	if (a)
	{
		a = static_cast<m_IDirect3DLight *>(a)->GetProxyInterface();
	}

	HRESULT hr = ProxyInterface->NextLight(a, b, c);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirect3DLight>(*b);
	}

	return hr;
}

HRESULT m_IDirect3DViewportX::GetViewport2(LPD3DVIEWPORT2 a)
{
	return ProxyInterface->GetViewport2(a);
}

HRESULT m_IDirect3DViewportX::SetViewport2(LPD3DVIEWPORT2 a)
{
	return ProxyInterface->SetViewport2(a);
}

HRESULT m_IDirect3DViewportX::SetBackgroundDepth2(LPDIRECTDRAWSURFACE4 a)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawSurface4 *>(a)->GetProxyInterface();
	}

	return ProxyInterface->SetBackgroundDepth2(a);
}

HRESULT m_IDirect3DViewportX::GetBackgroundDepth2(LPDIRECTDRAWSURFACE4 * a, LPBOOL b)
{
	HRESULT hr = ProxyInterface->GetBackgroundDepth2(a, b);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface4>(*a);
	}

	return hr;
}

HRESULT m_IDirect3DViewportX::Clear2(DWORD a, LPD3DRECT b, DWORD c, D3DCOLOR d, D3DVALUE e, DWORD f)
{
	return ProxyInterface->Clear2(a, b, c, d, e, f);
}
