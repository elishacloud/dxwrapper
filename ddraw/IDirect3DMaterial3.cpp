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

HRESULT m_IDirect3DMaterial3::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	HRESULT hr = ProxyInterface->QueryInterface(ConvertREFIID(riid), ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirect3DMaterial3::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DMaterial3::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		delete this;
	}

	return x;
}

HRESULT m_IDirect3DMaterial3::SetMaterial(LPD3DMATERIAL a)
{
	return ProxyInterface->SetMaterial(a);
}

HRESULT m_IDirect3DMaterial3::GetMaterial(LPD3DMATERIAL a)
{
	return ProxyInterface->GetMaterial(a);
}

HRESULT m_IDirect3DMaterial3::GetHandle(LPDIRECT3DDEVICE3 a, LPD3DMATERIALHANDLE b)
{
	if (a)
	{
		a = static_cast<m_IDirect3DDevice3 *>(a)->GetProxyInterface();
	}

	return ProxyInterface->GetHandle(a, b);
}
