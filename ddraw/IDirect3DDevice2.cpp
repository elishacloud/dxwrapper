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

HRESULT m_IDirect3DDevice2::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirect3DDevice2::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DDevice2::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		delete this;
	}

	return x;
}

HRESULT m_IDirect3DDevice2::GetCaps(LPD3DDEVICEDESC a, LPD3DDEVICEDESC b)
{
	return ProxyInterface->GetCaps(a, b);
}

HRESULT m_IDirect3DDevice2::SwapTextureHandles(LPDIRECT3DTEXTURE2 a, LPDIRECT3DTEXTURE2 b)
{
	if (a)
	{
		a = static_cast<m_IDirect3DTexture2 *>(a)->GetProxyInterface();
	}
	if (b)
	{
		b = static_cast<m_IDirect3DTexture2 *>(b)->GetProxyInterface();
	}

	return ProxyInterface->SwapTextureHandles(a, b);
}

HRESULT m_IDirect3DDevice2::GetStats(LPD3DSTATS a)
{
	return ProxyInterface->GetStats(a);
}

HRESULT m_IDirect3DDevice2::AddViewport(LPDIRECT3DVIEWPORT2 a)
{
	if (a)
	{
		a = static_cast<m_IDirect3DViewport2 *>(a)->GetProxyInterface();
	}

	return ProxyInterface->AddViewport(a);
}

HRESULT m_IDirect3DDevice2::DeleteViewport(LPDIRECT3DVIEWPORT2 a)
{
	if (a)
	{
		a = static_cast<m_IDirect3DViewport2 *>(a)->GetProxyInterface();
	}

	return ProxyInterface->DeleteViewport(a);
}

HRESULT m_IDirect3DDevice2::NextViewport(LPDIRECT3DVIEWPORT2 a, LPDIRECT3DVIEWPORT2 * b, DWORD c)
{
	if (a)
	{
		a = static_cast<m_IDirect3DViewport2 *>(a)->GetProxyInterface();
	}

	HRESULT hr = ProxyInterface->NextViewport(a, b, c);

	if (SUCCEEDED(hr))
	{
		*b = ProxyAddressLookupTable.FindAddress<m_IDirect3DViewport2>(*b);
	}

	return hr;
}

HRESULT m_IDirect3DDevice2::EnumTextureFormats(LPD3DENUMTEXTUREFORMATSCALLBACK a, LPVOID b)
{
	return ProxyInterface->EnumTextureFormats(a, b);
}

HRESULT m_IDirect3DDevice2::BeginScene()
{
	return ProxyInterface->BeginScene();
}

HRESULT m_IDirect3DDevice2::EndScene()
{
	return ProxyInterface->EndScene();
}

HRESULT m_IDirect3DDevice2::GetDirect3D(LPDIRECT3D2 * a)
{
	HRESULT hr = ProxyInterface->GetDirect3D(a);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirect3D2>(*a);
	}

	return hr;
}

HRESULT m_IDirect3DDevice2::SetCurrentViewport(LPDIRECT3DVIEWPORT2 a)
{
	if (a)
	{
		a = static_cast<m_IDirect3DViewport2 *>(a)->GetProxyInterface();
	}

	return ProxyInterface->SetCurrentViewport(a);
}

HRESULT m_IDirect3DDevice2::GetCurrentViewport(LPDIRECT3DVIEWPORT2 * a)
{
	HRESULT hr = ProxyInterface->GetCurrentViewport(a);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirect3DViewport2>(*a);
	}

	return hr;
}

HRESULT m_IDirect3DDevice2::SetRenderTarget(LPDIRECTDRAWSURFACE a, DWORD b)
{
	if (a)
	{
		a = static_cast<m_IDirectDrawSurface *>(a)->GetProxyInterface();
	}

	return ProxyInterface->SetRenderTarget(a, b);
}

HRESULT m_IDirect3DDevice2::GetRenderTarget(LPDIRECTDRAWSURFACE * a)
{
	HRESULT hr = ProxyInterface->GetRenderTarget(a);

	if (SUCCEEDED(hr))
	{
		*a = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface>(*a);
	}

	return hr;
}

HRESULT m_IDirect3DDevice2::Begin(D3DPRIMITIVETYPE a, D3DVERTEXTYPE b, DWORD c)
{
	return ProxyInterface->Begin(a, b, c);
}

HRESULT m_IDirect3DDevice2::BeginIndexed(D3DPRIMITIVETYPE a, D3DVERTEXTYPE b, LPVOID c, DWORD d, DWORD e)
{
	return ProxyInterface->BeginIndexed(a, b, c, d, e);
}

HRESULT m_IDirect3DDevice2::Vertex(LPVOID a)
{
	return ProxyInterface->Vertex(a);
}

HRESULT m_IDirect3DDevice2::Index(WORD a)
{
	return ProxyInterface->Index(a);
}

HRESULT m_IDirect3DDevice2::End(DWORD a)
{
	return ProxyInterface->End(a);
}

HRESULT m_IDirect3DDevice2::GetRenderState(D3DRENDERSTATETYPE a, LPDWORD b)
{
	return ProxyInterface->GetRenderState(a, b);
}

HRESULT m_IDirect3DDevice2::SetRenderState(D3DRENDERSTATETYPE a, DWORD b)
{
	return ProxyInterface->SetRenderState(a, b);
}

HRESULT m_IDirect3DDevice2::GetLightState(D3DLIGHTSTATETYPE a, LPDWORD b)
{
	return ProxyInterface->GetLightState(a, b);
}

HRESULT m_IDirect3DDevice2::SetLightState(D3DLIGHTSTATETYPE a, DWORD b)
{
	return ProxyInterface->SetLightState(a, b);
}

HRESULT m_IDirect3DDevice2::SetTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	return ProxyInterface->SetTransform(a, b);
}

HRESULT m_IDirect3DDevice2::GetTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	return ProxyInterface->GetTransform(a, b);
}

HRESULT m_IDirect3DDevice2::MultiplyTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	return ProxyInterface->MultiplyTransform(a, b);
}

HRESULT m_IDirect3DDevice2::DrawPrimitive(D3DPRIMITIVETYPE a, D3DVERTEXTYPE b, LPVOID c, DWORD d, DWORD e)
{
	return ProxyInterface->DrawPrimitive(a, b, c, d, e);
}

HRESULT m_IDirect3DDevice2::DrawIndexedPrimitive(D3DPRIMITIVETYPE a, D3DVERTEXTYPE b, LPVOID c, DWORD d, LPWORD e, DWORD f, DWORD g)
{
	return ProxyInterface->DrawIndexedPrimitive(a, b, c, d, e, f, g);
}

HRESULT m_IDirect3DDevice2::SetClipStatus(LPD3DCLIPSTATUS a)
{
	return ProxyInterface->SetClipStatus(a);
}

HRESULT m_IDirect3DDevice2::GetClipStatus(LPD3DCLIPSTATUS a)
{
	return ProxyInterface->GetClipStatus(a);
}
