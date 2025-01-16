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

HRESULT m_IDirect3DDevice2::QueryInterface(REFIID riid, LPVOID * ppvObj)
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

ULONG m_IDirect3DDevice2::AddRef()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirect3DDevice2::Release()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirect3DDevice2::GetCaps(LPD3DDEVICEDESC a, LPD3DDEVICEDESC b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetCaps(a, b);
}

HRESULT m_IDirect3DDevice2::SwapTextureHandles(LPDIRECT3DTEXTURE2 a, LPDIRECT3DTEXTURE2 b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SwapTextureHandles(a, b);
}

HRESULT m_IDirect3DDevice2::GetStats(LPD3DSTATS a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetStats(a, DirectXVersion);
}

HRESULT m_IDirect3DDevice2::AddViewport(LPDIRECT3DVIEWPORT2 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->AddViewport((LPDIRECT3DVIEWPORT3)a);
}

HRESULT m_IDirect3DDevice2::DeleteViewport(LPDIRECT3DVIEWPORT2 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DeleteViewport((LPDIRECT3DVIEWPORT3)a);
}

HRESULT m_IDirect3DDevice2::NextViewport(LPDIRECT3DVIEWPORT2 a, LPDIRECT3DVIEWPORT2 * b, DWORD c)
{
	if (!ProxyInterface)
	{
		if (b)
		{
			*b = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->NextViewport((LPDIRECT3DVIEWPORT3)a, (LPDIRECT3DVIEWPORT3*)b, c, DirectXVersion);
}

HRESULT m_IDirect3DDevice2::EnumTextureFormats(LPD3DENUMTEXTUREFORMATSCALLBACK a, LPVOID b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EnumTextureFormats(a, b);
}

HRESULT m_IDirect3DDevice2::BeginScene()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->BeginScene();
}

HRESULT m_IDirect3DDevice2::EndScene()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EndScene();
}

HRESULT m_IDirect3DDevice2::GetDirect3D(LPDIRECT3D2 * a)
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

HRESULT m_IDirect3DDevice2::SetCurrentViewport(LPDIRECT3DVIEWPORT2 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetCurrentViewport((LPDIRECT3DVIEWPORT3)a);
}

HRESULT m_IDirect3DDevice2::GetCurrentViewport(LPDIRECT3DVIEWPORT2 * a)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetCurrentViewport((LPDIRECT3DVIEWPORT3*)a, DirectXVersion);
}

HRESULT m_IDirect3DDevice2::SetRenderTarget(LPDIRECTDRAWSURFACE a, DWORD b)
{
	return ProxyInterface->SetRenderTarget((LPDIRECTDRAWSURFACE7)a, b);
}

HRESULT m_IDirect3DDevice2::GetRenderTarget(LPDIRECTDRAWSURFACE * a)
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

HRESULT m_IDirect3DDevice2::Begin(D3DPRIMITIVETYPE a, D3DVERTEXTYPE b, DWORD c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Begin(a, b, c);
}

HRESULT m_IDirect3DDevice2::BeginIndexed(D3DPRIMITIVETYPE a, D3DVERTEXTYPE b, LPVOID c, DWORD d, DWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->BeginIndexed(a, b, c, d, e);
}

HRESULT m_IDirect3DDevice2::Vertex(LPVOID a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Vertex(a);
}

HRESULT m_IDirect3DDevice2::Index(WORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Index(a);
}

HRESULT m_IDirect3DDevice2::End(DWORD a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->End(a);
}

HRESULT m_IDirect3DDevice2::GetRenderState(D3DRENDERSTATETYPE a, LPDWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetRenderState(a, b);
}

HRESULT m_IDirect3DDevice2::SetRenderState(D3DRENDERSTATETYPE a, DWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetRenderState(a, b);
}

HRESULT m_IDirect3DDevice2::GetLightState(D3DLIGHTSTATETYPE a, LPDWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetLightState(a, b);
}

HRESULT m_IDirect3DDevice2::SetLightState(D3DLIGHTSTATETYPE a, DWORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetLightState(a, b);
}

HRESULT m_IDirect3DDevice2::SetTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetTransform(a, b);
}

HRESULT m_IDirect3DDevice2::GetTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetTransform(a, b);
}

HRESULT m_IDirect3DDevice2::MultiplyTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->MultiplyTransform(a, b);
}

HRESULT m_IDirect3DDevice2::DrawPrimitive(D3DPRIMITIVETYPE a, D3DVERTEXTYPE b, LPVOID c, DWORD d, DWORD e)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DrawPrimitive(a, b, c, d, e, DirectXVersion);
}

HRESULT m_IDirect3DDevice2::DrawIndexedPrimitive(D3DPRIMITIVETYPE a, D3DVERTEXTYPE b, LPVOID c, DWORD d, LPWORD e, DWORD f, DWORD g)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DrawIndexedPrimitive(a, b, c, d, e, f, g, DirectXVersion);
}

HRESULT m_IDirect3DDevice2::SetClipStatus(LPD3DCLIPSTATUS a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetClipStatus(a);
}

HRESULT m_IDirect3DDevice2::GetClipStatus(LPD3DCLIPSTATUS a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetClipStatus(a);
}
