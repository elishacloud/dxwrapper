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

HRESULT m_IDirect3DViewport3::QueryInterface(REFIID riid, LPVOID * ppvObj)
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

ULONG m_IDirect3DViewport3::AddRef()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirect3DViewport3::Release()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirect3DViewport3::Initialize(LPDIRECT3D a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Initialize(a);
}

HRESULT m_IDirect3DViewport3::GetViewport(LPD3DVIEWPORT a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetViewport(a);
}

HRESULT m_IDirect3DViewport3::SetViewport(LPD3DVIEWPORT a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetViewport(a);
}

HRESULT m_IDirect3DViewport3::TransformVertices(DWORD a, LPD3DTRANSFORMDATA b, DWORD c, LPDWORD d)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->TransformVertices(a, b, c, d);
}

HRESULT m_IDirect3DViewport3::LightElements(DWORD a, LPD3DLIGHTDATA b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->LightElements(a, b);
}

HRESULT m_IDirect3DViewport3::SetBackground(D3DMATERIALHANDLE a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetBackground(a);
}

HRESULT m_IDirect3DViewport3::GetBackground(LPD3DMATERIALHANDLE a, LPBOOL b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetBackground(a, b);
}

HRESULT m_IDirect3DViewport3::SetBackgroundDepth(LPDIRECTDRAWSURFACE a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetBackgroundDepth(a);
}

HRESULT m_IDirect3DViewport3::GetBackgroundDepth(LPDIRECTDRAWSURFACE * a, LPBOOL b)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetBackgroundDepth(a, b);
}

HRESULT m_IDirect3DViewport3::Clear(DWORD a, LPD3DRECT b, DWORD c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Clear(a, b, c);
}

HRESULT m_IDirect3DViewport3::AddLight(LPDIRECT3DLIGHT a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->AddLight(a);
}

HRESULT m_IDirect3DViewport3::DeleteLight(LPDIRECT3DLIGHT a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DeleteLight(a);
}

HRESULT m_IDirect3DViewport3::NextLight(LPDIRECT3DLIGHT a, LPDIRECT3DLIGHT * b, DWORD c)
{
	if (!ProxyInterface)
	{
		if (b)
		{
			*b = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->NextLight(a, b, c);
}

HRESULT m_IDirect3DViewport3::GetViewport2(LPD3DVIEWPORT2 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetViewport2(a);
}

HRESULT m_IDirect3DViewport3::SetViewport2(LPD3DVIEWPORT2 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetViewport2(a);
}

HRESULT m_IDirect3DViewport3::SetBackgroundDepth2(LPDIRECTDRAWSURFACE4 a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetBackgroundDepth2(a);
}

HRESULT m_IDirect3DViewport3::GetBackgroundDepth2(LPDIRECTDRAWSURFACE4 * a, LPBOOL b)
{
	if (!ProxyInterface)
	{
		if (a)
		{
			*a = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetBackgroundDepth2(a, b, 4);
}

HRESULT m_IDirect3DViewport3::Clear2(DWORD a, LPD3DRECT b, DWORD c, D3DCOLOR d, D3DVALUE e, DWORD f)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Clear2(a, b, c, d, e, f);
}
