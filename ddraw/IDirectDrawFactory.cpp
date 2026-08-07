/**
* Copyright (C) 2026 Elisha Riedlinger
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

using namespace DdrawWrapper;

// ******************************
// IUnknown functions
// ******************************

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
		return _InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawFactory::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		ULONG ref = InterlockedDecrementIfPositive(&RefCount);

		if (ref == 0)
		{
			delete this;
		}

		return ref;
	}

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

// ******************************
// IDirectDrawFactory functions
// ******************************

HRESULT m_IDirectDrawFactory::CreateDirectDraw(GUID* pGUID, HWND hWnd, DWORD dwCoopLevelFlags, DWORD dwReserved, IUnknown* pUnkOuter, IDirectDraw** ppDirectDraw)
{
	UNREFERENCED_PARAMETER(dwReserved);

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = dd_DirectDrawCreate(pGUID, ppDirectDraw, pUnkOuter);

	if (SUCCEEDED(hr) && ppDirectDraw)
	{
		if (Config.Dd7to9)
		{
			m_IDirectDrawX* pDirectDrawX = nullptr;
			if (SUCCEEDED((*ppDirectDraw)->QueryInterface(IID_GetInterfaceX, reinterpret_cast<LPVOID*>(&pDirectDrawX))))
			{
				pDirectDrawX->SetAsCreatedByDDFactory();
			}
		}

		hr = (*ppDirectDraw)->SetCooperativeLevel(hWnd, dwCoopLevelFlags);

		if (FAILED(hr))
		{
			ULONG ref = (*ppDirectDraw)->Release();
			if (ref)
			{
				Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'DirectDraw' " << ref;
			}
			*ppDirectDraw = nullptr;

			return hr;
		}
	}

	return hr;
}

HRESULT m_IDirectDrawFactory::DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return dd_DirectDrawEnumerateA(lpCallback, lpContext);
}

HRESULT m_IDirectDrawFactory::DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return dd_DirectDrawEnumerateW(lpCallback, lpContext);
}
