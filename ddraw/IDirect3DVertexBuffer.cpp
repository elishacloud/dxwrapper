/**
* Copyright (C) 2018 Elisha Riedlinger
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

#include "ddraw.h"

HRESULT m_IDirect3DVertexBuffer::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, IID_IDirect3DVertexBuffer, this);
}

ULONG m_IDirect3DVertexBuffer::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DVertexBuffer::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		delete this;
	}

	return x;
}

HRESULT m_IDirect3DVertexBuffer::Lock(DWORD a, LPVOID * b, LPDWORD c)
{
	return ProxyInterface->Lock(a, b, c);
}

HRESULT m_IDirect3DVertexBuffer::Unlock()
{
	return ProxyInterface->Unlock();
}

HRESULT m_IDirect3DVertexBuffer::ProcessVertices(DWORD a, DWORD b, DWORD c, LPDIRECT3DVERTEXBUFFER d, DWORD e, LPDIRECT3DDEVICE3 f, DWORD g)
{
	if (d)
	{
		d = static_cast<m_IDirect3DVertexBuffer *>(d)->GetProxyInterface();
	}
	if (f)
	{
		f = static_cast<m_IDirect3DDevice3 *>(f)->GetProxyInterface();
	}

	return ProxyInterface->ProcessVertices(a, b, c, d, e, f, g);
}

HRESULT m_IDirect3DVertexBuffer::GetVertexBufferDesc(LPD3DVERTEXBUFFERDESC a)
{
	return ProxyInterface->GetVertexBufferDesc(a);
}

HRESULT m_IDirect3DVertexBuffer::Optimize(LPDIRECT3DDEVICE3 a, DWORD b)
{
	if (a)
	{
		a = static_cast<m_IDirect3DDevice3 *>(a)->GetProxyInterface();
	}

	return ProxyInterface->Optimize(a, b);
}
