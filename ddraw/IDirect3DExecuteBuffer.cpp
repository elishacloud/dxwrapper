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

HRESULT m_IDirect3DExecuteBuffer::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, this);
}

ULONG m_IDirect3DExecuteBuffer::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DExecuteBuffer::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		delete this;
	}

	return x;
}

HRESULT m_IDirect3DExecuteBuffer::Initialize(LPDIRECT3DDEVICE a, LPD3DEXECUTEBUFFERDESC b)
{
	if (a)
	{
		a = static_cast<m_IDirect3DDevice *>(a)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(a, b);
}

HRESULT m_IDirect3DExecuteBuffer::Lock(LPD3DEXECUTEBUFFERDESC a)
{
	return ProxyInterface->Lock(a);
}

HRESULT m_IDirect3DExecuteBuffer::Unlock()
{
	return ProxyInterface->Unlock();
}

HRESULT m_IDirect3DExecuteBuffer::SetExecuteData(LPD3DEXECUTEDATA a)
{
	return ProxyInterface->SetExecuteData(a);
}

HRESULT m_IDirect3DExecuteBuffer::GetExecuteData(LPD3DEXECUTEDATA a)
{
	return ProxyInterface->GetExecuteData(a);
}

HRESULT m_IDirect3DExecuteBuffer::Validate(LPDWORD a, LPD3DVALIDATECALLBACK b, LPVOID c, DWORD d)
{
	return ProxyInterface->Validate(a, b, c, d);
}

HRESULT m_IDirect3DExecuteBuffer::Optimize(DWORD a)
{
	return ProxyInterface->Optimize(a);
}
