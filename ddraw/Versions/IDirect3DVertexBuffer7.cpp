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

HRESULT m_IDirect3DVertexBuffer7::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return ProxyInterface->QueryInterface(ReplaceIIDUnknown(riid, WrapperID), ppvObj, DirectXVersion);
}

ULONG m_IDirect3DVertexBuffer7::AddRef()
{
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirect3DVertexBuffer7::Release()
{
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirect3DVertexBuffer7::Lock(DWORD a, LPVOID * b, LPDWORD c)
{
	return ProxyInterface->Lock(a, b, c);
}

HRESULT m_IDirect3DVertexBuffer7::Unlock()
{
	return ProxyInterface->Unlock();
}

HRESULT m_IDirect3DVertexBuffer7::ProcessVertices(DWORD a, DWORD b, DWORD c, LPDIRECT3DVERTEXBUFFER7 d, DWORD e, LPDIRECT3DDEVICE7 f, DWORD g)
{
	return ProxyInterface->ProcessVertices(a, b, c, d, e, f, g);
}

HRESULT m_IDirect3DVertexBuffer7::GetVertexBufferDesc(LPD3DVERTEXBUFFERDESC a)
{
	return ProxyInterface->GetVertexBufferDesc(a);
}

HRESULT m_IDirect3DVertexBuffer7::Optimize(LPDIRECT3DDEVICE7 a, DWORD b)
{
	return ProxyInterface->Optimize(a, b);
}

HRESULT m_IDirect3DVertexBuffer7::ProcessVerticesStrided(DWORD a, DWORD b, DWORD c, LPD3DDRAWPRIMITIVESTRIDEDDATA d, DWORD e, LPDIRECT3DDEVICE7 f, DWORD g)
{
	return ProxyInterface->ProcessVerticesStrided(a, b, c, d, e, f, g);
}
