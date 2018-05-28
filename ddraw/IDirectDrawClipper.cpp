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
	if (!ProxyInterface)
	{
		if ((riid == IID_IDirectDrawClipper || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return S_OK;
		}
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, this);
}

ULONG m_IDirectDrawClipper::AddRef()
{
	if (!ProxyInterface)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawClipper::Release()
{
	if (!ProxyInterface)
	{
		LONG ref = InterlockedDecrement(&RefCount);

		if (ref == 0)
		{
			delete this;
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
	if (!ProxyInterface)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return DDERR_GENERIC;
	}

	return ProxyInterface->GetClipList(lpRect, lpClipList, lpdwSize);
}

HRESULT m_IDirectDrawClipper::GetHWnd(HWND FAR * lphWnd)
{
	if (!lphWnd)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (!ProxyInterface)
	{
		if (!cliphWnd)
		{
			return DDERR_INVALIDOBJECT;
		}

		*lphWnd = cliphWnd;

		return DD_OK;
	}

	return ProxyInterface->GetHWnd(lphWnd);
}

HRESULT m_IDirectDrawClipper::Initialize(LPDIRECTDRAW lpDD, DWORD dwFlags)
{
	if (!ProxyInterface)
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
	if (!lpbChanged)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (!ProxyInterface)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";

		// lpbChanged is TRUE if the clip list has changed, and FALSE otherwise.

		return DDERR_GENERIC;
	}

	return ProxyInterface->IsClipListChanged(lpbChanged);
}

HRESULT m_IDirectDrawClipper::SetClipList(LPRGNDATA lpClipList, DWORD dwFlags)
{
	if (!ProxyInterface)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";

		// You cannot set the clip list if a window handle is already associated with the DirectDrawClipper object.
		if (cliphWnd)
		{
			return DDERR_CLIPPERISUSINGHWND;
		}

		// **NOTE:  If you call IDirectDrawSurface7::BltFast on a surface with an attached clipper, it returns DDERR_UNSUPPORTED.
		if (!lpClipList)
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
	if (!ProxyInterface)
	{
		cliphWnd = hWnd;

		// Load clip list from window

		return DD_OK;
	}

	return ProxyInterface->SetHWnd(dwFlags, hWnd);
}
