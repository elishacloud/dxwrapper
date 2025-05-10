/**
* Copyright (C) 2025 Elisha Riedlinger
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

// ******************************
// IUnknown functions
// ******************************

HRESULT m_IDirectDrawClipper::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ppvObj)
	{
		return E_POINTER;
	}
	*ppvObj = nullptr;

	if (riid == IID_GetRealInterface)
	{
		*ppvObj = ProxyInterface;
		return DD_OK;
	}
	if (riid == IID_GetInterfaceX)
	{
		*ppvObj = this;
		return DD_OK;
	}

	if (riid == IID_IDirectDrawClipper || riid == IID_IUnknown)
	{
		AddRef();

		*ppvObj = this;

		return DD_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID);
}

ULONG m_IDirectDrawClipper::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawClipper::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		ULONG ref = (InterlockedCompareExchange(&RefCount, 0, 0)) ? InterlockedDecrement(&RefCount) : 0;

		if (ref == 0)
		{
			SaveInterfaceAddress(this);
		}

		return ref;
	}

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		SaveInterfaceAddress(this);
	}

	return ref;
}

// ******************************
// IDirectDrawClipper functions
// ******************************

HRESULT m_IDirectDrawClipper::GetClipList(LPRECT lpRect, LPRGNDATA lpClipList, LPDWORD lpdwSize)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwSize)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (IsClipListSet)
		{
			// ToDo: add support for lpRect
			// A pointer to a RECT structure that GetClipList uses to clip the clip list. Set this parameter to NULL to retrieve the entire clip list.

			// ToDo: fix sizeof(RGNDATA) to be the atual size of the data
			if (!lpClipList)
			{
				*lpdwSize = ClipList.size();
				return DD_OK;
			}
			else if (*lpdwSize == ClipList.size())
			{
				IsClipListChangedFlag = false;				// Just set this to false for now
				memcpy(lpClipList, &ClipList, *lpdwSize);
				return DD_OK;
			}
			else if (lpdwSize && *lpdwSize >= sizeof(RGNDATA))
			{
				return DDERR_REGIONTOOSMALL;
			}
		}
		// Get default clip area
		else
		{
			// ToDo: add support for lpRect
			// A pointer to a RECT structure that GetClipList uses to clip the clip list. Set this parameter to NULL to retrieve the entire clip list.

			HRESULT hr = DD_OK;

			// Get the HWND of the window
			HWND hWnd = (cliphWnd) ? cliphWnd : (ddrawParent) ? ddrawParent->GetHwnd() : nullptr;

			if (!hWnd)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Could not get window handle!");
				return DDERR_GENERIC;
			}

			// Get the client area rectangle
			RECT clientRect = {};
			GetClientRect(hWnd, &clientRect);

			// Map the client area coordinates to screen coordinates
			MapWindowPoints(hWnd, HWND_DESKTOP, (LPPOINT)&clientRect, 2);

			// Create a region from the client rectangle
			HRGN hRgn = CreateRectRgn(clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);

			if (!hRgn)
			{
				return DDERR_GENERIC;
			}

			// Get region data
			*lpdwSize = GetRegionData(hRgn, *lpdwSize, lpClipList);

			// Check region data
			if (!*lpdwSize)
			{
				if (lpClipList)
				{
					*lpdwSize = GetRegionData(hRgn, 0, nullptr);
					hr = DDERR_REGIONTOOSMALL;
				}
				else
				{
					hr = DDERR_GENERIC;
				}
			}

			// Get region data size
			if (*lpdwSize && *lpdwSize != ClipList.size())
			{
				ClipList.resize(*lpdwSize);
			}

			// Release the region
			DeleteObject(hRgn);

			return hr;
		}

		return DDERR_GENERIC;
	}

	return ProxyInterface->GetClipList(lpRect, lpClipList, lpdwSize);
}

HRESULT m_IDirectDrawClipper::GetHWnd(HWND FAR * lphWnd)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lphWnd)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!cliphWnd || !IsWindow(cliphWnd))
		{
			cliphWnd = nullptr;

			Logging::Log() << __FUNCTION__ << " Warning: Clip Window invalid or has not been set!";

			return DDERR_GENERIC;
		}

		*lphWnd = cliphWnd;

		return DD_OK;
	}

	return ProxyInterface->GetHWnd(lphWnd);
}

HRESULT m_IDirectDrawClipper::Initialize(LPDIRECTDRAW lpDD, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Returns D3D_OK if successful, otherwise it returns an error.
		return D3D_OK;
	}

	if (lpDD)
	{
		lpDD->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDD);
	}

	return ProxyInterface->Initialize(lpDD, dwFlags);
}

HRESULT m_IDirectDrawClipper::IsClipListChanged(BOOL FAR * lpbChanged)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpbChanged)
		{
			return DDERR_INVALIDPARAMS;
		}

		// ToDo: Fix this to get real status of ClipList
		// lpbChanged is TRUE if the clip list has changed, and FALSE otherwise.
		if (IsClipListChangedFlag)
		{
			*lpbChanged = TRUE;
		}
		else
		{
			*lpbChanged = FALSE;
		}

		return DD_OK;
	}

	return ProxyInterface->IsClipListChanged(lpbChanged);
}

HRESULT m_IDirectDrawClipper::SetClipList(LPRGNDATA lpClipList, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// You cannot set the clip list if a window handle is already associated with the DirectDrawClipper object.
		if (cliphWnd)
		{
			return DDERR_CLIPPERISUSINGHWND;
		}

		if (!lpClipList)
		{
			// Delete associated clip list if it exists
			IsClipListSet = false;
		}
		else
		{
			// Set clip list to lpClipList
			IsClipListSet = true;
			IsClipListChangedFlag = true;
			if (ClipList.empty())
			{
				ClipList.resize(sizeof(RGNDATA));
			}
			// ToDo: Fix this to get correct size of ClipList
			memcpy(ClipList.data(), lpClipList, ClipList.size());
		}

		return DD_OK;
	}

	return ProxyInterface->SetClipList(lpClipList, dwFlags);
}

HRESULT m_IDirectDrawClipper::SetHWnd(DWORD dwFlags, HWND hWnd)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (hWnd && !IsWindow(hWnd))
		{
			if (cliphWnd && !IsWindow(cliphWnd))
			{
				cliphWnd = nullptr;
			}

			return DDERR_INVALIDPARAMS;
		}

		cliphWnd = hWnd;

		// Load clip list from window

		return DD_OK;
	}

	return ProxyInterface->SetHWnd(dwFlags, hWnd);
}

// ******************************
// Helper functions
// ******************************

void m_IDirectDrawClipper::InitInterface(DWORD dwFlags)
{
	ScopedDDCriticalSection ThreadLockDD;

	if (ddrawParent)
	{
		ddrawParent->AddClipper(this);
	}

	clipperCaps = dwFlags;
	cliphWnd = nullptr;
	ClipList.clear();
	IsClipListSet = false;
	IsClipListChangedFlag = false;
}

void m_IDirectDrawClipper::ReleaseInterface()
{
	if (Config.Exiting)
	{
		return;
	}

	ScopedDDCriticalSection ThreadLockDD;

	m_IDirectDrawX::ClearBaseClipper(this);

	if (ddrawParent)
	{
		ddrawParent->ClearClipper(this);
	}
}

m_IDirectDrawClipper* m_IDirectDrawClipper::CreateDirectDrawClipper(IDirectDrawClipper* aOriginal, m_IDirectDrawX* NewParent, DWORD dwFlags)
{
	m_IDirectDrawClipper* Interface = InterfaceAddressCache<m_IDirectDrawClipper>(nullptr);
	if (Interface)
	{
		Interface->SetProxy(aOriginal, NewParent, dwFlags);
	}
	else
	{
		if (aOriginal)
		{
			Interface = new m_IDirectDrawClipper(aOriginal);
		}
		else
		{
			Interface = new m_IDirectDrawClipper(NewParent, dwFlags);
		}
	}
	return Interface;
}
