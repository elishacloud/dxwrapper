/**
* Copyright (C) 2024 Elisha Riedlinger
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

ULONG m_IDirect3DDevice7::AddRef()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirect3DDevice7::Release()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirect3DDevice7::GetCaps(LPD3DDEVICEDESC7 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetCaps(a);
}

HRESULT m_IDirect3DDevice7::EnumTextureFormats(LPD3DENUMPIXELFORMATSCALLBACK a, LPVOID b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EnumTextureFormats(a, b);
}

HRESULT m_IDirect3DDevice7::BeginScene()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->BeginScene();
}

HRESULT m_IDirect3DDevice7::EndScene()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EndScene();
}

HRESULT m_IDirect3DDevice7::GetDirect3D(LPDIRECT3D7 * a)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetDirect3D(a, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::SetRenderTarget(LPDIRECTDRAWSURFACE7 a, DWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetRenderTarget(a, b);
}

HRESULT m_IDirect3DDevice7::GetRenderTarget(LPDIRECTDRAWSURFACE7 * a)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetRenderTarget(a, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::Clear(DWORD a, LPD3DRECT b, DWORD c, D3DCOLOR d, D3DVALUE e, DWORD f)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Clear(a, b, c, d, e, f);
}

HRESULT m_IDirect3DDevice7::SetTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetTransform(a, b);
}

HRESULT m_IDirect3DDevice7::GetTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetTransform(a, b);
}

HRESULT m_IDirect3DDevice7::SetViewport(LPD3DVIEWPORT7 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetViewport(a);
}

HRESULT m_IDirect3DDevice7::MultiplyTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->MultiplyTransform(a, b);
}

HRESULT m_IDirect3DDevice7::GetViewport(LPD3DVIEWPORT7 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetViewport(a);
}

HRESULT m_IDirect3DDevice7::SetMaterial(LPD3DMATERIAL7 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetMaterial(a);
}

HRESULT m_IDirect3DDevice7::GetMaterial(LPD3DMATERIAL7 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetMaterial(a);
}

HRESULT m_IDirect3DDevice7::SetLight(DWORD a, LPD3DLIGHT7 b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetLight(a, b);
}

HRESULT m_IDirect3DDevice7::GetLight(DWORD a, LPD3DLIGHT7 b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetLight(a, b);
}

HRESULT m_IDirect3DDevice7::SetRenderState(D3DRENDERSTATETYPE a, DWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetRenderState(a, b);
}

HRESULT m_IDirect3DDevice7::GetRenderState(D3DRENDERSTATETYPE a, LPDWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetRenderState(a, b);
}

HRESULT m_IDirect3DDevice7::BeginStateBlock()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->BeginStateBlock();
}

HRESULT m_IDirect3DDevice7::EndStateBlock(LPDWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EndStateBlock(a);
}

HRESULT m_IDirect3DDevice7::PreLoad(LPDIRECTDRAWSURFACE7 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->PreLoad(a);
}

HRESULT m_IDirect3DDevice7::DrawPrimitive(D3DPRIMITIVETYPE a, DWORD b, LPVOID c, DWORD d, DWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DrawPrimitive(a, b, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::DrawIndexedPrimitive(D3DPRIMITIVETYPE a, DWORD b, LPVOID c, DWORD d, LPWORD e, DWORD f, DWORD g)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DrawIndexedPrimitive(a, b, c, d, e, f, g, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::SetClipStatus(LPD3DCLIPSTATUS a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetClipStatus(a);
}

HRESULT m_IDirect3DDevice7::GetClipStatus(LPD3DCLIPSTATUS a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetClipStatus(a);
}

HRESULT m_IDirect3DDevice7::DrawPrimitiveStrided(D3DPRIMITIVETYPE a, DWORD b, LPD3DDRAWPRIMITIVESTRIDEDDATA c, DWORD d, DWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DrawPrimitiveStrided(a, b, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::DrawIndexedPrimitiveStrided(D3DPRIMITIVETYPE a, DWORD b, LPD3DDRAWPRIMITIVESTRIDEDDATA c, DWORD d, LPWORD e, DWORD f, DWORD g)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DrawIndexedPrimitiveStrided(a, b, c, d, e, f, g, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::DrawPrimitiveVB(D3DPRIMITIVETYPE a, LPDIRECT3DVERTEXBUFFER7 b, DWORD c, DWORD d, DWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DrawPrimitiveVB(a, b, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE a, LPDIRECT3DVERTEXBUFFER7 b, DWORD c, DWORD d, LPWORD e, DWORD f, DWORD g)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DrawIndexedPrimitiveVB(a, b, c, d, e, f, g, DirectXVersion);
}

HRESULT m_IDirect3DDevice7::ComputeSphereVisibility(LPD3DVECTOR a, LPD3DVALUE b, DWORD c, DWORD d, LPDWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->ComputeSphereVisibility(a, b, c, d, e);
}

HRESULT m_IDirect3DDevice7::GetTexture(DWORD a, LPDIRECTDRAWSURFACE7 * b)
{
	if (!ProxyInterface)
	{
		if (b)
		{
			*b = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetTexture(a, b);
}

HRESULT m_IDirect3DDevice7::SetTexture(DWORD a, LPDIRECTDRAWSURFACE7 b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetTexture(a, b);
}

HRESULT m_IDirect3DDevice7::GetTextureStageState(DWORD a, D3DTEXTURESTAGESTATETYPE b, LPDWORD c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetTextureStageState(a, b, c);
}

HRESULT m_IDirect3DDevice7::SetTextureStageState(DWORD a, D3DTEXTURESTAGESTATETYPE b, DWORD c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetTextureStageState(a, b, c);
}

HRESULT m_IDirect3DDevice7::ValidateDevice(LPDWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->ValidateDevice(a);
}

HRESULT m_IDirect3DDevice7::ApplyStateBlock(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->ApplyStateBlock(a);
}

HRESULT m_IDirect3DDevice7::CaptureStateBlock(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->CaptureStateBlock(a);
}

HRESULT m_IDirect3DDevice7::DeleteStateBlock(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DeleteStateBlock(a);
}

HRESULT m_IDirect3DDevice7::CreateStateBlock(D3DSTATEBLOCKTYPE a, LPDWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->CreateStateBlock(a, b);
}

HRESULT m_IDirect3DDevice7::Load(LPDIRECTDRAWSURFACE7 a, LPPOINT b, LPDIRECTDRAWSURFACE7 c, LPRECT d, DWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Load(a, b, c, d, e);
}

HRESULT m_IDirect3DDevice7::LightEnable(DWORD a, BOOL b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->LightEnable(a, b);
}

HRESULT m_IDirect3DDevice7::GetLightEnable(DWORD a, BOOL * b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetLightEnable(a, b);
}

HRESULT m_IDirect3DDevice7::SetClipPlane(DWORD a, D3DVALUE * b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetClipPlane(a, b);
}

HRESULT m_IDirect3DDevice7::GetClipPlane(DWORD a, D3DVALUE * b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetClipPlane(a, b);
}

HRESULT m_IDirect3DDevice7::GetInfo(DWORD a, LPVOID b, DWORD c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetInfo(a, b, c);
}
