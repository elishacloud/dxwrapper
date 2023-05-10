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

HRESULT m_IDirect3DDevice7::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return ProxyInterface->QueryInterface(ReplaceIIDUnknown(riid, WrapperID), ppvObj, DirectXVersion);
}

ULONG m_IDirect3DDevice7::AddRef()
{
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirect3DDevice7::Release()
{
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirect3DDevice7::GetCaps(LPD3DDEVICEDESC7 a)
{
	return ProxyInterface->GetCaps(a);
}

HRESULT m_IDirect3DDevice7::EnumTextureFormats(LPD3DENUMPIXELFORMATSCALLBACK a, LPVOID b)
{
	return ProxyInterface->EnumTextureFormats(a, b);
}

HRESULT m_IDirect3DDevice7::BeginScene()
{
	return ProxyInterface->BeginScene();
}

HRESULT m_IDirect3DDevice7::EndScene()
{
	return ProxyInterface->EndScene();
}

HRESULT m_IDirect3DDevice7::GetDirect3D(LPDIRECT3D7 * a)
{
	return ProxyInterface->GetDirect3D(a, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::SetRenderTarget(LPDIRECTDRAWSURFACE7 a, DWORD b)
{
	return ProxyInterface->SetRenderTarget(a, b);
}

HRESULT m_IDirect3DDevice7::GetRenderTarget(LPDIRECTDRAWSURFACE7 * a)
{
	return ProxyInterface->GetRenderTarget(a, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::Clear(DWORD a, LPD3DRECT b, DWORD c, D3DCOLOR d, D3DVALUE e, DWORD f)
{
	return ProxyInterface->Clear(a, b, c, d, e, f);
}

HRESULT m_IDirect3DDevice7::SetTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	return ProxyInterface->SetTransform(a, b);
}

HRESULT m_IDirect3DDevice7::GetTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	return ProxyInterface->GetTransform(a, b);
}

HRESULT m_IDirect3DDevice7::SetViewport(LPD3DVIEWPORT7 a)
{
	return ProxyInterface->SetViewport(a);
}

HRESULT m_IDirect3DDevice7::MultiplyTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	return ProxyInterface->MultiplyTransform(a, b);
}

HRESULT m_IDirect3DDevice7::GetViewport(LPD3DVIEWPORT7 a)
{
	return ProxyInterface->GetViewport(a);
}

HRESULT m_IDirect3DDevice7::SetMaterial(LPD3DMATERIAL7 a)
{
	return ProxyInterface->SetMaterial(a);
}

HRESULT m_IDirect3DDevice7::GetMaterial(LPD3DMATERIAL7 a)
{
	return ProxyInterface->GetMaterial(a);
}

HRESULT m_IDirect3DDevice7::SetLight(DWORD a, LPD3DLIGHT7 b)
{
	return ProxyInterface->SetLight(a, b);
}

HRESULT m_IDirect3DDevice7::GetLight(DWORD a, LPD3DLIGHT7 b)
{
	return ProxyInterface->GetLight(a, b);
}

HRESULT m_IDirect3DDevice7::SetRenderState(D3DRENDERSTATETYPE a, DWORD b)
{
	return ProxyInterface->SetRenderState(a, b);
}

HRESULT m_IDirect3DDevice7::GetRenderState(D3DRENDERSTATETYPE a, LPDWORD b)
{
	return ProxyInterface->GetRenderState(a, b);
}

HRESULT m_IDirect3DDevice7::BeginStateBlock()
{
	return ProxyInterface->BeginStateBlock();
}

HRESULT m_IDirect3DDevice7::EndStateBlock(LPDWORD a)
{
	return ProxyInterface->EndStateBlock(a);
}

HRESULT m_IDirect3DDevice7::PreLoad(LPDIRECTDRAWSURFACE7 a)
{
	return ProxyInterface->PreLoad(a);
}

HRESULT m_IDirect3DDevice7::DrawPrimitive(D3DPRIMITIVETYPE a, DWORD b, LPVOID c, DWORD d, DWORD e)
{
	return ProxyInterface->DrawPrimitive(a, b, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::DrawIndexedPrimitive(D3DPRIMITIVETYPE a, DWORD b, LPVOID c, DWORD d, LPWORD e, DWORD f, DWORD g)
{
	return ProxyInterface->DrawIndexedPrimitive(a, b, c, d, e, f, g, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::SetClipStatus(LPD3DCLIPSTATUS a)
{
	return ProxyInterface->SetClipStatus(a);
}

HRESULT m_IDirect3DDevice7::GetClipStatus(LPD3DCLIPSTATUS a)
{
	return ProxyInterface->GetClipStatus(a);
}

HRESULT m_IDirect3DDevice7::DrawPrimitiveStrided(D3DPRIMITIVETYPE a, DWORD b, LPD3DDRAWPRIMITIVESTRIDEDDATA c, DWORD d, DWORD e)
{
	return ProxyInterface->DrawPrimitiveStrided(a, b, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::DrawIndexedPrimitiveStrided(D3DPRIMITIVETYPE a, DWORD b, LPD3DDRAWPRIMITIVESTRIDEDDATA c, DWORD d, LPWORD e, DWORD f, DWORD g)
{
	return ProxyInterface->DrawIndexedPrimitiveStrided(a, b, c, d, e, f, g, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::DrawPrimitiveVB(D3DPRIMITIVETYPE a, LPDIRECT3DVERTEXBUFFER7 b, DWORD c, DWORD d, DWORD e)
{
	return ProxyInterface->DrawPrimitiveVB(a, b, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE a, LPDIRECT3DVERTEXBUFFER7 b, DWORD c, DWORD d, LPWORD e, DWORD f, DWORD g)
{
	return ProxyInterface->DrawIndexedPrimitiveVB(a, b, c, d, e, f, g, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::ComputeSphereVisibility(LPD3DVECTOR a, LPD3DVALUE b, DWORD c, DWORD d, LPDWORD e)
{
	return ProxyInterface->ComputeSphereVisibility(a, b, c, d, e);
}

HRESULT m_IDirect3DDevice7::GetTexture(DWORD a, LPDIRECTDRAWSURFACE7 * b)
{
	return ProxyInterface->GetTexture(a, b);
}

HRESULT m_IDirect3DDevice7::SetTexture(DWORD a, LPDIRECTDRAWSURFACE7 b)
{
	return ProxyInterface->SetTexture(a, b);
}

HRESULT m_IDirect3DDevice7::GetTextureStageState(DWORD a, D3DTEXTURESTAGESTATETYPE b, LPDWORD c)
{
	return ProxyInterface->GetTextureStageState(a, b, c);
}

HRESULT m_IDirect3DDevice7::SetTextureStageState(DWORD a, D3DTEXTURESTAGESTATETYPE b, DWORD c)
{
	return ProxyInterface->SetTextureStageState(a, b, c);
}

HRESULT m_IDirect3DDevice7::ValidateDevice(LPDWORD a)
{
	return ProxyInterface->ValidateDevice(a);
}

HRESULT m_IDirect3DDevice7::ApplyStateBlock(DWORD a)
{
	return ProxyInterface->ApplyStateBlock(a);
}

HRESULT m_IDirect3DDevice7::CaptureStateBlock(DWORD a)
{
	return ProxyInterface->CaptureStateBlock(a);
}

HRESULT m_IDirect3DDevice7::DeleteStateBlock(DWORD a)
{
	return ProxyInterface->DeleteStateBlock(a);
}

HRESULT m_IDirect3DDevice7::CreateStateBlock(D3DSTATEBLOCKTYPE a, LPDWORD b)
{
	return ProxyInterface->CreateStateBlock(a, b);
}

HRESULT m_IDirect3DDevice7::Load(LPDIRECTDRAWSURFACE7 a, LPPOINT b, LPDIRECTDRAWSURFACE7 c, LPRECT d, DWORD e)
{
	return ProxyInterface->Load(a, b, c, d, e);
}

HRESULT m_IDirect3DDevice7::LightEnable(DWORD a, BOOL b)
{
	return ProxyInterface->LightEnable(a, b);
}

HRESULT m_IDirect3DDevice7::GetLightEnable(DWORD a, BOOL * b)
{
	return ProxyInterface->GetLightEnable(a, b);
}

HRESULT m_IDirect3DDevice7::SetClipPlane(DWORD a, D3DVALUE * b)
{
	return ProxyInterface->SetClipPlane(a, b);
}

HRESULT m_IDirect3DDevice7::GetClipPlane(DWORD a, D3DVALUE * b)
{
	return ProxyInterface->GetClipPlane(a, b);
}

HRESULT m_IDirect3DDevice7::GetInfo(DWORD a, LPVOID b, DWORD c)
{
	return ProxyInterface->GetInfo(a, b, c);
}
