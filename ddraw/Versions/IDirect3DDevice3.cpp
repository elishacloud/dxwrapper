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

HRESULT m_IDirect3DDevice3::QueryInterface(REFIID riid, LPVOID * ppvObj)
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

ULONG m_IDirect3DDevice3::AddRef()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirect3DDevice3::Release()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirect3DDevice3::GetCaps(LPD3DDEVICEDESC a, LPD3DDEVICEDESC b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetCaps(a, b);
}

HRESULT m_IDirect3DDevice3::GetStats(LPD3DSTATS a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetStats(a);
}

HRESULT m_IDirect3DDevice3::AddViewport(LPDIRECT3DVIEWPORT3 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->AddViewport(a);
}

HRESULT m_IDirect3DDevice3::DeleteViewport(LPDIRECT3DVIEWPORT3 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DeleteViewport(a);
}

HRESULT m_IDirect3DDevice3::NextViewport(LPDIRECT3DVIEWPORT3 a, LPDIRECT3DVIEWPORT3 * b, DWORD c)
{
	if (!ProxyInterface)
	{
		if (b)
		{
			*b = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->NextViewport(a, b, c, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::EnumTextureFormats(LPD3DENUMPIXELFORMATSCALLBACK a, LPVOID b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EnumTextureFormats(a, b);
}

HRESULT m_IDirect3DDevice3::BeginScene()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->BeginScene();
}

HRESULT m_IDirect3DDevice3::EndScene()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EndScene();
}

HRESULT m_IDirect3DDevice3::GetDirect3D(LPDIRECT3D3 * a)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetDirect3D((LPDIRECT3D7*)a, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::SetCurrentViewport(LPDIRECT3DVIEWPORT3 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetCurrentViewport(a);
}

HRESULT m_IDirect3DDevice3::GetCurrentViewport(LPDIRECT3DVIEWPORT3 * a)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetCurrentViewport(a, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::SetRenderTarget(LPDIRECTDRAWSURFACE4 a, DWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetRenderTarget((LPDIRECTDRAWSURFACE7)a, b);
}

HRESULT m_IDirect3DDevice3::GetRenderTarget(LPDIRECTDRAWSURFACE4 * a)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetRenderTarget((LPDIRECTDRAWSURFACE7*)a, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::Begin(D3DPRIMITIVETYPE a, DWORD b, DWORD c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Begin(a, b, c);
}

HRESULT m_IDirect3DDevice3::BeginIndexed(D3DPRIMITIVETYPE a, DWORD b, LPVOID c, DWORD d, DWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->BeginIndexed(a, b, c, d, e);
}

HRESULT m_IDirect3DDevice3::Vertex(LPVOID a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Vertex(a);
}

HRESULT m_IDirect3DDevice3::Index(WORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Index(a);
}

HRESULT m_IDirect3DDevice3::End(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->End(a);
}

HRESULT m_IDirect3DDevice3::GetRenderState(D3DRENDERSTATETYPE a, LPDWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetRenderState(a, b);
}

HRESULT m_IDirect3DDevice3::SetRenderState(D3DRENDERSTATETYPE a, DWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetRenderState(a, b);
}

HRESULT m_IDirect3DDevice3::GetLightState(D3DLIGHTSTATETYPE a, LPDWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetLightState(a, b);
}

HRESULT m_IDirect3DDevice3::SetLightState(D3DLIGHTSTATETYPE a, DWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetLightState(a, b);
}

HRESULT m_IDirect3DDevice3::SetTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetTransform(a, b);
}

HRESULT m_IDirect3DDevice3::GetTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetTransform(a, b);
}

HRESULT m_IDirect3DDevice3::MultiplyTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->MultiplyTransform(a, b);
}

HRESULT m_IDirect3DDevice3::DrawPrimitive(D3DPRIMITIVETYPE a, DWORD b, LPVOID c, DWORD d, DWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DrawPrimitive(a, b, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::DrawIndexedPrimitive(D3DPRIMITIVETYPE a, DWORD b, LPVOID c, DWORD d, LPWORD e, DWORD f, DWORD g)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DrawIndexedPrimitive(a, b, c, d, e, f, g, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::SetClipStatus(LPD3DCLIPSTATUS a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetClipStatus(a);
}

HRESULT m_IDirect3DDevice3::GetClipStatus(LPD3DCLIPSTATUS a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetClipStatus(a);
}

HRESULT m_IDirect3DDevice3::DrawPrimitiveStrided(D3DPRIMITIVETYPE a, DWORD b, LPD3DDRAWPRIMITIVESTRIDEDDATA c, DWORD d, DWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DrawPrimitiveStrided(a, b, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::DrawIndexedPrimitiveStrided(D3DPRIMITIVETYPE a, DWORD b, LPD3DDRAWPRIMITIVESTRIDEDDATA c, DWORD d, LPWORD e, DWORD f, DWORD g)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DrawIndexedPrimitiveStrided(a, b, c, d, e, f, g, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::DrawPrimitiveVB(D3DPRIMITIVETYPE a, LPDIRECT3DVERTEXBUFFER b, DWORD c, DWORD d, DWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DrawPrimitiveVB(a, (LPDIRECT3DVERTEXBUFFER7)b, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE a, LPDIRECT3DVERTEXBUFFER b, LPWORD c, DWORD d, DWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DrawIndexedPrimitiveVB(a, (LPDIRECT3DVERTEXBUFFER7)b, 0, 0, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice3::ComputeSphereVisibility(LPD3DVECTOR a, LPD3DVALUE b, DWORD c, DWORD d, LPDWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->ComputeSphereVisibility(a, b, c, d, e);
}

HRESULT m_IDirect3DDevice3::GetTexture(DWORD a, LPDIRECT3DTEXTURE2 * b)
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

HRESULT m_IDirect3DDevice3::SetTexture(DWORD a, LPDIRECT3DTEXTURE2 b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetTexture(a, b);
}

HRESULT m_IDirect3DDevice3::GetTextureStageState(DWORD a, D3DTEXTURESTAGESTATETYPE b, LPDWORD c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetTextureStageState(a, b, c);
}

HRESULT m_IDirect3DDevice3::SetTextureStageState(DWORD a, D3DTEXTURESTAGESTATETYPE b, DWORD c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetTextureStageState(a, b, c);
}

HRESULT m_IDirect3DDevice3::ValidateDevice(LPDWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->ValidateDevice(a);
}
