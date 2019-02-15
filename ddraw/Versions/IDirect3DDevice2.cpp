/**
* Copyright (C) 2019 Elisha Riedlinger
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

HRESULT m_IDirect3DDevice2::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return ProxyInterface->QueryInterface(riid, ppvObj, DirectXVersion);
}

ULONG m_IDirect3DDevice2::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DDevice2::Release()
{
	return ProxyInterface->Release();
}

HRESULT m_IDirect3DDevice2::GetCaps(LPD3DDEVICEDESC a, LPD3DDEVICEDESC b)
{
	return ProxyInterface->GetCaps(a, b);
}

HRESULT m_IDirect3DDevice2::SwapTextureHandles(LPDIRECT3DTEXTURE2 a, LPDIRECT3DTEXTURE2 b)
{
	return ProxyInterface->SwapTextureHandles(a, b);
}

HRESULT m_IDirect3DDevice2::GetStats(LPD3DSTATS a)
{
	return ProxyInterface->GetStats(a);
}

HRESULT m_IDirect3DDevice2::AddViewport(LPDIRECT3DVIEWPORT2 a)
{
	return ProxyInterface->AddViewport((LPDIRECT3DVIEWPORT3)a);
}

HRESULT m_IDirect3DDevice2::DeleteViewport(LPDIRECT3DVIEWPORT2 a)
{
	return ProxyInterface->DeleteViewport((LPDIRECT3DVIEWPORT3)a);
}

HRESULT m_IDirect3DDevice2::NextViewport(LPDIRECT3DVIEWPORT2 a, LPDIRECT3DVIEWPORT2 * b, DWORD c)
{
	return ProxyInterface->NextViewport((LPDIRECT3DVIEWPORT3)a, (LPDIRECT3DVIEWPORT3*)b, c, DirectXVersion);
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
	return ProxyInterface->GetDirect3D((LPDIRECT3D7*)a, DirectXVersion);
}

HRESULT m_IDirect3DDevice2::SetCurrentViewport(LPDIRECT3DVIEWPORT2 a)
{
	return ProxyInterface->SetCurrentViewport((LPDIRECT3DVIEWPORT3)a);
}

HRESULT m_IDirect3DDevice2::GetCurrentViewport(LPDIRECT3DVIEWPORT2 * a)
{
	return ProxyInterface->GetCurrentViewport((LPDIRECT3DVIEWPORT3*)a, DirectXVersion);
}

HRESULT m_IDirect3DDevice2::SetRenderTarget(LPDIRECTDRAWSURFACE a, DWORD b)
{
	return ProxyInterface->SetRenderTarget((LPDIRECTDRAWSURFACE7)a, b);
}

HRESULT m_IDirect3DDevice2::GetRenderTarget(LPDIRECTDRAWSURFACE * a)
{
	return ProxyInterface->GetRenderTarget((LPDIRECTDRAWSURFACE7*)a, DirectXVersion);
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
