/**
* Copyright (C) 2019 Elisha Riedlinger
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
	Logging::LogDebug() << __FUNCTION__;

	if (!ProxyInterface)
	{
		if ((riid == IID_IDirectDrawClipper || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return DD_OK;
		}
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, this);
}

ULONG m_IDirectDrawClipper::AddRef()
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ProxyInterface)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawClipper::Release()
{
	Logging::LogDebug() << __FUNCTION__;

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

HRESULT m_IDirectDrawClipper::GetClipList(LPRECT lpRect, LPRGNDATA lpClipList, LPDWORD lpdwSize)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->GetClipList(lpRect, lpClipList, lpdwSize);
}

HRESULT m_IDirectDrawClipper::GetHWnd(HWND FAR * lphWnd)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ProxyInterface)
	{
		if (!lphWnd)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!cliphWnd)
		{
			return DDERR_GENERIC;
		}

		*lphWnd = cliphWnd;

		return DD_OK;
	}

	return ProxyInterface->GetHWnd(lphWnd);
}

HRESULT m_IDirectDrawClipper::Initialize(LPDIRECTDRAW lpDD, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

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
	Logging::LogDebug() << __FUNCTION__;

	if (!ProxyInterface)
	{
		if (!lpbChanged)
		{
			return DDERR_INVALIDPARAMS;
		}

		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");

		// lpbChanged is TRUE if the clip list has changed, and FALSE otherwise.

		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->IsClipListChanged(lpbChanged);
}

HRESULT m_IDirectDrawClipper::SetClipList(LPRGNDATA lpClipList, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");

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

		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->SetClipList(lpClipList, dwFlags);
}

HRESULT m_IDirectDrawClipper::SetHWnd(DWORD dwFlags, HWND hWnd)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ProxyInterface)
	{
		cliphWnd = hWnd;

		// Load clip list from window

		return DD_OK;
	}

	return ProxyInterface->SetHWnd(dwFlags, hWnd);
}
