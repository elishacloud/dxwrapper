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

HRESULT m_IDirect3DDevice::QueryInterface(REFIID riid, LPVOID * ppvObj)
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

ULONG m_IDirect3DDevice::AddRef()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirect3DDevice::Release()
{
	if (!ProxyInterface)
	{
		return 0;
	}
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirect3DDevice::Initialize(LPDIRECT3D a, LPGUID b, LPD3DDEVICEDESC c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Initialize(a, b, c);
}

HRESULT m_IDirect3DDevice::GetCaps(LPD3DDEVICEDESC a, LPD3DDEVICEDESC b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetCaps(a, b);
}

HRESULT m_IDirect3DDevice::SwapTextureHandles(LPDIRECT3DTEXTURE a, LPDIRECT3DTEXTURE b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SwapTextureHandles((LPDIRECT3DTEXTURE2)a, (LPDIRECT3DTEXTURE2)b);
}

HRESULT m_IDirect3DDevice::CreateExecuteBuffer(LPD3DEXECUTEBUFFERDESC a, LPDIRECT3DEXECUTEBUFFER * b, IUnknown * c)
{
	if (!ProxyInterface)
	{
		if (b)
		{
			*b = nullptr;
		}
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->CreateExecuteBuffer(a, b, c);
}

HRESULT m_IDirect3DDevice::GetStats(LPD3DSTATS a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetStats(a);
}

HRESULT m_IDirect3DDevice::Execute(LPDIRECT3DEXECUTEBUFFER a, LPDIRECT3DVIEWPORT b, DWORD c)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Execute(a, b, c);
}

HRESULT m_IDirect3DDevice::AddViewport(LPDIRECT3DVIEWPORT a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->AddViewport((LPDIRECT3DVIEWPORT3)a);
}

HRESULT m_IDirect3DDevice::DeleteViewport(LPDIRECT3DVIEWPORT a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DeleteViewport((LPDIRECT3DVIEWPORT3)a);
}

HRESULT m_IDirect3DDevice::NextViewport(LPDIRECT3DVIEWPORT a, LPDIRECT3DVIEWPORT * b, DWORD c)
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

HRESULT m_IDirect3DDevice::Pick(LPDIRECT3DEXECUTEBUFFER a, LPDIRECT3DVIEWPORT b, DWORD c, LPD3DRECT d)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->Pick(a, b, c, d);
}

HRESULT m_IDirect3DDevice::GetPickRecords(LPDWORD a, LPD3DPICKRECORD b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetPickRecords(a, b);
}

HRESULT m_IDirect3DDevice::EnumTextureFormats(LPD3DENUMTEXTUREFORMATSCALLBACK a, LPVOID b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EnumTextureFormats(a, b);
}

HRESULT m_IDirect3DDevice::CreateMatrix(LPD3DMATRIXHANDLE a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->CreateMatrix(a);
}

HRESULT m_IDirect3DDevice::SetMatrix(D3DMATRIXHANDLE a, const LPD3DMATRIX b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->SetMatrix(a, b);
}

HRESULT m_IDirect3DDevice::GetMatrix(D3DMATRIXHANDLE a, LPD3DMATRIX b)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->GetMatrix(a, b);
}

HRESULT m_IDirect3DDevice::DeleteMatrix(D3DMATRIXHANDLE a)
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->DeleteMatrix(a);
}

HRESULT m_IDirect3DDevice::BeginScene()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->BeginScene();
}

HRESULT m_IDirect3DDevice::EndScene()
{
	if (!ProxyInterface)
	{
		return DDERR_INVALIDOBJECT;
	}
	return ProxyInterface->EndScene();
}

HRESULT m_IDirect3DDevice::GetDirect3D(LPDIRECT3D * a)
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
