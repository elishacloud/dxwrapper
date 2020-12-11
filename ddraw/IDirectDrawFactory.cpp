/**
* Copyright (C) 2020 Elisha Riedlinger
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

/************************/
/*** IUnknown methods ***/
/************************/

HRESULT m_IDirectDrawFactory::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if ((riid == IID_IDirectDrawFactory || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return DD_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID);
}

ULONG m_IDirectDrawFactory::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawFactory::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG ref;

	if (!ProxyInterface)
	{
		ref = InterlockedDecrement(&RefCount);
	}
	else
	{
		ref = ProxyInterface->Release();
	}

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

/**********************************/
/*** IDirectDrawFactory methods ***/
/**********************************/

HRESULT m_IDirectDrawFactory::CreateDirectDraw(GUID * pGUID, HWND hWnd, DWORD dwCoopLevelFlags, DWORD dwReserved, IUnknown * pUnkOuter, IDirectDraw * * ppDirectDraw)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		HRESULT hr = dd_DirectDrawCreate(pGUID, ppDirectDraw, pUnkOuter);

		if (SUCCEEDED(hr) && ppDirectDraw)
		{
			hr = (*ppDirectDraw)->SetCooperativeLevel(hWnd, dwCoopLevelFlags);
			if (FAILED(hr))
			{
				(*ppDirectDraw)->Release();
				*ppDirectDraw = nullptr;
			}
		}

		return hr;
	}

	HRESULT hr = ProxyInterface->CreateDirectDraw(pGUID, hWnd, dwCoopLevelFlags, dwReserved, pUnkOuter, ppDirectDraw);

	if (SUCCEEDED(hr) && ppDirectDraw)
	{
		m_IDirectDrawX *Interface = new m_IDirectDrawX((IDirectDraw7*)*ppDirectDraw, 1);

		*ppDirectDraw = (LPDIRECTDRAW)Interface->GetWrapperInterfaceX(1);
	}

	return hr;
}

HRESULT m_IDirectDrawFactory::DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		return dd_DirectDrawEnumerateA(lpCallback, lpContext);
	}

	return ProxyInterface->DirectDrawEnumerateA(lpCallback, lpContext);
}

HRESULT m_IDirectDrawFactory::DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		return dd_DirectDrawEnumerateW(lpCallback, lpContext);
	}

	return ProxyInterface->DirectDrawEnumerateW(lpCallback, lpContext);
}
