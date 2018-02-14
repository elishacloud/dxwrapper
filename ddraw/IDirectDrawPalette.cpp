/**
* Copyright (C) 2017 Elisha Riedlinger
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

HRESULT m_IDirectDrawPalette::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectDrawPalette::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawPalette::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		delete this;
	}

	return x;
}

HRESULT m_IDirectDrawPalette::GetCaps(LPDWORD a)
{
	return ProxyInterface->GetCaps(a);
}

HRESULT m_IDirectDrawPalette::GetEntries(DWORD a, DWORD b, DWORD c, LPPALETTEENTRY d)
{
	return ProxyInterface->GetEntries(a, b, c, d);
}

HRESULT m_IDirectDrawPalette::Initialize(LPDIRECTDRAW a, DWORD b, LPPALETTEENTRY c)
{
	if (a)
	{
		a = static_cast<m_IDirectDraw *>(a)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(a, b, c);
}

HRESULT m_IDirectDrawPalette::SetEntries(DWORD a, DWORD b, DWORD c, LPPALETTEENTRY d)
{
	return ProxyInterface->SetEntries(a, b, c, d);
}
