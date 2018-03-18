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

HRESULT m_IDirectDrawFactory::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, IID_IDirectDrawFactory);
}

ULONG m_IDirectDrawFactory::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawFactory::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		delete this;
	}

	return x;
}

HRESULT m_IDirectDrawFactory::CreateDirectDraw(GUID * pGUID, HWND hWnd, DWORD dwCoopLevelFlags, DWORD dwReserved, IUnknown * pUnkOuter, IDirectDraw * * ppDirectDraw)
{
	HRESULT hr = ProxyInterface->CreateDirectDraw(pGUID, hWnd, dwCoopLevelFlags, dwReserved, pUnkOuter, ppDirectDraw);

	if (SUCCEEDED(hr))
	{
		*ppDirectDraw = ProxyAddressLookupTable.FindAddress<m_IDirectDraw>(*ppDirectDraw);
	}

	return hr;
}

HRESULT m_IDirectDrawFactory::DirectDrawEnumerate(LPDDENUMCALLBACK lpCallback, LPVOID lpContext)
{
	return ProxyInterface->DirectDrawEnumerate(lpCallback, lpContext);
}
