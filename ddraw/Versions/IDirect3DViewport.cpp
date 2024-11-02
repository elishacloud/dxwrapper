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

HRESULT m_IDirect3DViewport::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return ProxyInterface->QueryInterface(ReplaceIIDUnknown(riid, WrapperID), ppvObj, DirectXVersion);
}

ULONG m_IDirect3DViewport::AddRef()
{
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirect3DViewport::Release()
{
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirect3DViewport::Initialize(LPDIRECT3D a)
{
	return ProxyInterface->Initialize(a);
}

HRESULT m_IDirect3DViewport::GetViewport(LPD3DVIEWPORT a)
{
	return ProxyInterface->GetViewport(a);
}

HRESULT m_IDirect3DViewport::SetViewport(LPD3DVIEWPORT a)
{
	return ProxyInterface->SetViewport(a);
}

HRESULT m_IDirect3DViewport::TransformVertices(DWORD a, LPD3DTRANSFORMDATA b, DWORD c, LPDWORD d)
{
	return ProxyInterface->TransformVertices(a, b, c, d);
}

HRESULT m_IDirect3DViewport::LightElements(DWORD a, LPD3DLIGHTDATA b)
{
	return ProxyInterface->LightElements(a, b);
}

HRESULT m_IDirect3DViewport::SetBackground(D3DMATERIALHANDLE a)
{
	return ProxyInterface->SetBackground(a);
}

HRESULT m_IDirect3DViewport::GetBackground(LPD3DMATERIALHANDLE a, LPBOOL b)
{
	return ProxyInterface->GetBackground(a, b);
}

HRESULT m_IDirect3DViewport::SetBackgroundDepth(LPDIRECTDRAWSURFACE a)
{
	return ProxyInterface->SetBackgroundDepth(a);
}

HRESULT m_IDirect3DViewport::GetBackgroundDepth(LPDIRECTDRAWSURFACE * a, LPBOOL b)
{
	return ProxyInterface->GetBackgroundDepth(a, b);
}

HRESULT m_IDirect3DViewport::Clear(DWORD a, LPD3DRECT b, DWORD c)
{
	return ProxyInterface->Clear(a, b, c);
}

HRESULT m_IDirect3DViewport::AddLight(LPDIRECT3DLIGHT a)
{
	return ProxyInterface->AddLight(a);
}

HRESULT m_IDirect3DViewport::DeleteLight(LPDIRECT3DLIGHT a)
{
	return ProxyInterface->DeleteLight(a);
}

HRESULT m_IDirect3DViewport::NextLight(LPDIRECT3DLIGHT a, LPDIRECT3DLIGHT * b, DWORD c)
{
	return ProxyInterface->NextLight(a, b, c);
}
