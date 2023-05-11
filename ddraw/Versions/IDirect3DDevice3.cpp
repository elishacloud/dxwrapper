/**
* Copyright (C) 2022 Elisha Riedlinger
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

HRESULT m_IDirect3DDevice3::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return ProxyInterface->QueryInterface(ReplaceIIDUnknown(riid, WrapperID), ppvObj, DirectXVersion);
}

ULONG m_IDirect3DDevice3::AddRef()
{
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirect3DDevice3::Release()
{
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirect3DDevice3::GetCaps(LPD3DDEVICEDESC a, LPD3DDEVICEDESC b)
{
	return ProxyInterface->GetCaps(a, b);
}

HRESULT m_IDirect3DDevice3::GetStats(LPD3DSTATS a)
{
	return ProxyInterface->GetStats(a);
}

HRESULT m_IDirect3DDevice3::AddViewport(LPDIRECT3DVIEWPORT3 a)
{
	return ProxyInterface->AddViewport(a);
}

HRESULT m_IDirect3DDevice3::DeleteViewport(LPDIRECT3DVIEWPORT3 a)
{
	return ProxyInterface->DeleteViewport(a);
}

HRESULT m_IDirect3DDevice3::NextViewport(LPDIRECT3DVIEWPORT3 a, LPDIRECT3DVIEWPORT3 * b, DWORD c)
{
	return ProxyInterface->NextViewport(a, b, c, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::EnumTextureFormats(LPD3DENUMPIXELFORMATSCALLBACK a, LPVOID b)
{
	return ProxyInterface->EnumTextureFormats(a, b);
}

HRESULT m_IDirect3DDevice3::BeginScene()
{
	return ProxyInterface->BeginScene();
}

HRESULT m_IDirect3DDevice3::EndScene()
{
	return ProxyInterface->EndScene();
}

HRESULT m_IDirect3DDevice3::GetDirect3D(LPDIRECT3D3 * a)
{
	return ProxyInterface->GetDirect3D((LPDIRECT3D7*)a, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::SetCurrentViewport(LPDIRECT3DVIEWPORT3 a)
{
	return ProxyInterface->SetCurrentViewport(a);
}

HRESULT m_IDirect3DDevice3::GetCurrentViewport(LPDIRECT3DVIEWPORT3 * a)
{
	return ProxyInterface->GetCurrentViewport(a, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::SetRenderTarget(LPDIRECTDRAWSURFACE4 a, DWORD b)
{
	return ProxyInterface->SetRenderTarget((LPDIRECTDRAWSURFACE7)a, b);
}

HRESULT m_IDirect3DDevice3::GetRenderTarget(LPDIRECTDRAWSURFACE4 * a)
{
	return ProxyInterface->GetRenderTarget((LPDIRECTDRAWSURFACE7*)a, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::Begin(D3DPRIMITIVETYPE a, DWORD b, DWORD c)
{
	return ProxyInterface->Begin(a, b, c);
}

HRESULT m_IDirect3DDevice3::BeginIndexed(D3DPRIMITIVETYPE a, DWORD b, LPVOID c, DWORD d, DWORD e)
{
	return ProxyInterface->BeginIndexed(a, b, c, d, e);
}

HRESULT m_IDirect3DDevice3::Vertex(LPVOID a)
{
	return ProxyInterface->Vertex(a);
}

HRESULT m_IDirect3DDevice3::Index(WORD a)
{
	return ProxyInterface->Index(a);
}

HRESULT m_IDirect3DDevice3::End(DWORD a)
{
	return ProxyInterface->End(a);
}

HRESULT m_IDirect3DDevice3::GetRenderState(D3DRENDERSTATETYPE a, LPDWORD b)
{
	return ProxyInterface->GetRenderState(a, b);
}

HRESULT m_IDirect3DDevice3::SetRenderState(D3DRENDERSTATETYPE a, DWORD b)
{
	return ProxyInterface->SetRenderState(a, b);
}

HRESULT m_IDirect3DDevice3::GetLightState(D3DLIGHTSTATETYPE a, LPDWORD b)
{
	return ProxyInterface->GetLightState(a, b);
}

HRESULT m_IDirect3DDevice3::SetLightState(D3DLIGHTSTATETYPE a, DWORD b)
{
	return ProxyInterface->SetLightState(a, b);
}

HRESULT m_IDirect3DDevice3::SetTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	return ProxyInterface->SetTransform(a, b);
}

HRESULT m_IDirect3DDevice3::GetTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	return ProxyInterface->GetTransform(a, b);
}

HRESULT m_IDirect3DDevice3::MultiplyTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	return ProxyInterface->MultiplyTransform(a, b);
}

HRESULT m_IDirect3DDevice3::DrawPrimitive(D3DPRIMITIVETYPE a, DWORD b, LPVOID c, DWORD d, DWORD e)
{
	return ProxyInterface->DrawPrimitive(a, b, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::DrawIndexedPrimitive(D3DPRIMITIVETYPE a, DWORD b, LPVOID c, DWORD d, LPWORD e, DWORD f, DWORD g)
{
	return ProxyInterface->DrawIndexedPrimitive(a, b, c, d, e, f, g, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::SetClipStatus(LPD3DCLIPSTATUS a)
{
	return ProxyInterface->SetClipStatus(a);
}

HRESULT m_IDirect3DDevice3::GetClipStatus(LPD3DCLIPSTATUS a)
{
	return ProxyInterface->GetClipStatus(a);
}

HRESULT m_IDirect3DDevice3::DrawPrimitiveStrided(D3DPRIMITIVETYPE a, DWORD b, LPD3DDRAWPRIMITIVESTRIDEDDATA c, DWORD d, DWORD e)
{
	return ProxyInterface->DrawPrimitiveStrided(a, b, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::DrawIndexedPrimitiveStrided(D3DPRIMITIVETYPE a, DWORD b, LPD3DDRAWPRIMITIVESTRIDEDDATA c, DWORD d, LPWORD e, DWORD f, DWORD g)
{
	return ProxyInterface->DrawIndexedPrimitiveStrided(a, b, c, d, e, f, g, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::DrawPrimitiveVB(D3DPRIMITIVETYPE a, LPDIRECT3DVERTEXBUFFER b, DWORD c, DWORD d, DWORD e)
{
	return ProxyInterface->DrawPrimitiveVB(a, (LPDIRECT3DVERTEXBUFFER7)b, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE a, LPDIRECT3DVERTEXBUFFER b, LPWORD c, DWORD d, DWORD e)
{
	return ProxyInterface->DrawIndexedPrimitiveVB(a, (LPDIRECT3DVERTEXBUFFER7)b, 0, 0, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::ComputeSphereVisibility(LPD3DVECTOR a, LPD3DVALUE b, DWORD c, DWORD d, LPDWORD e)
{
	return ProxyInterface->ComputeSphereVisibility(a, b, c, d, e);
}

HRESULT m_IDirect3DDevice3::GetTexture(DWORD a, LPDIRECT3DTEXTURE2 * b)
{
	return ProxyInterface->GetTexture(a, b);
}

HRESULT m_IDirect3DDevice3::SetTexture(DWORD a, LPDIRECT3DTEXTURE2 b)
{
	return ProxyInterface->SetTexture(a, b);
}

HRESULT m_IDirect3DDevice3::GetTextureStageState(DWORD a, D3DTEXTURESTAGESTATETYPE b, LPDWORD c)
{
	return ProxyInterface->GetTextureStageState(a, b, c);
}

HRESULT m_IDirect3DDevice3::SetTextureStageState(DWORD a, D3DTEXTURESTAGESTATETYPE b, DWORD c)
{
	return ProxyInterface->SetTextureStageState(a, b, c);
}

HRESULT m_IDirect3DDevice3::ValidateDevice(LPDWORD a)
{
	return ProxyInterface->ValidateDevice(a);
}
