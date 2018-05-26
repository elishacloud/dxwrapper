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
*
* Code taken from: https://github.com/strangebytes/diablo-ddrawwrapper
*/

#include "ddraw.h"

HRESULT m_IDirectDrawClipper::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	if (ProxyInterface == nullptr)
	{
		if ((riid == IID_IDirectDrawClipper || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return S_OK;
		}

		return E_NOINTERFACE;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, this);
}

ULONG m_IDirectDrawClipper::AddRef()
{
	if (ProxyInterface == nullptr)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawClipper::Release()
{
	if (ProxyInterface == nullptr)
	{
		LONG ref = InterlockedDecrement(&RefCount);

		if (ref == 0)
		{
			delete this;
			return 0;
		}

		return ref;
	}

	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

HRESULT m_IDirectDrawClipper::GetClipList(LPRECT lpRect, LPRGNDATA lpClipList, LPDWORD lpdwSize)
{
	if (ProxyInterface == nullptr)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return DDERR_GENERIC;
	}

	return ProxyInterface->GetClipList(lpRect, lpClipList, lpdwSize);
}

HRESULT m_IDirectDrawClipper::GetHWnd(HWND FAR * lphWnd)
{
	if (ProxyInterface == nullptr)
	{
		// lphWnd cannot be null
		if (!lphWnd)
		{
			return DDERR_INVALIDPARAMS;
		}
		else if (!cliphWnd)
		{
			return DDERR_INVALIDOBJECT;
		}

		// Set lphWnd to associated window handle
		*lphWnd = cliphWnd;

		// Success
		return DD_OK;
	}

	return ProxyInterface->GetHWnd(lphWnd);
}

HRESULT m_IDirectDrawClipper::Initialize(LPDIRECTDRAW lpDD, DWORD dwFlags)
{
	if (ProxyInterface == nullptr)
	{
		return DD_OK;
	}

	if (lpDD)
	{
		lpDD = static_cast<m_IDirectDraw *>(lpDD)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(lpDD, dwFlags);
}

HRESULT m_IDirectDrawClipper::IsClipListChanged(BOOL FAR * lpbChanged)
{
	if (ProxyInterface == nullptr)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";

		// lpbChanged cannot be null
		if (lpbChanged == NULL)
		{
			return DDERR_INVALIDPARAMS;
		}

		// lpbChanged is TRUE if the clip list has changed, and FALSE otherwise.

		return DDERR_GENERIC;
	}

	return ProxyInterface->IsClipListChanged(lpbChanged);
}

HRESULT m_IDirectDrawClipper::SetClipList(LPRGNDATA lpClipList, DWORD dwFlags)
{
	if (ProxyInterface == nullptr)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";

		//You cannot set the clip list if a window handle is already associated
		// with the DirectDrawClipper objet.
		if (cliphWnd)
		{
			return DDERR_CLIPPERISUSINGHWND;
		}

		// ******NOTE:  If you call IDirectDrawSurface7::BltFast on a surface with an attached 
		// clipper, it returns DDERR_UNSUPPORTED.
		if (lpClipList == NULL)
		{
			// Delete associated clip list if it exists
		}
		else
		{
			// Set clip list to lpClipList
		}

		return DDERR_GENERIC;
	}

	return ProxyInterface->SetClipList(lpClipList, dwFlags);
}

HRESULT m_IDirectDrawClipper::SetHWnd(DWORD dwFlags, HWND hWnd)
{
	if (ProxyInterface == nullptr)
	{
		cliphWnd = hWnd;

		// Load clip list from window

		return DD_OK;
	}

	return ProxyInterface->SetHWnd(dwFlags, hWnd);
}

// Initialize wrapper function
HRESULT m_IDirectDrawClipper::WrapperInitialize(DWORD dwFlags)
{
	UNREFERENCED_PARAMETER(dwFlags);

	return DD_OK;
}
