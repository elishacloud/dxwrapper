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

		if (!IsClipListSet)
		{
			*lpdwSize = 0;
			return DDERR_NOCLIPLIST;
		}

		const RGNDATA* input = reinterpret_cast<const RGNDATA*>(ClipList.data());
		DWORD regionSize = sizeof(RGNDATAHEADER) + input->rdh.nRgnSize;

		if (!lpClipList)
		{
			*lpdwSize = regionSize;
			return DD_OK;
		}
		else if (*lpdwSize < regionSize)
		{
			*lpdwSize = regionSize;
			return DDERR_REGIONTOOSMALL;
		}

		// Intersect clip list with lpRect if needed
		if (lpRect)
		{
			RGNDATA* output = reinterpret_cast<RGNDATA*>(lpClipList);
			output->rdh.dwSize = sizeof(RGNDATAHEADER);
			output->rdh.iType = RDH_RECTANGLES;
			output->rdh.nCount = 0;
			output->rdh.nRgnSize = 0;
			SetRectEmpty(&output->rdh.rcBound);

			const RECT* inputRects = reinterpret_cast<const RECT*>(input->Buffer);
			RECT* outputRects = reinterpret_cast<RECT*>(output->Buffer);

			for (DWORD i = 0; i < input->rdh.nCount; ++i)
			{
				RECT result;
				if (IntersectRect(&result, &inputRects[i], lpRect))
				{
					outputRects[output->rdh.nCount++] = result;
					UnionRect(&output->rdh.rcBound, &output->rdh.rcBound, &result);
				}
			}

			*lpdwSize = sizeof(RGNDATAHEADER) + output->rdh.nRgnSize;
			output->rdh.nRgnSize = output->rdh.nCount * sizeof(RECT);
		}
		else
		{
			*lpdwSize = regionSize;
			memcpy(lpClipList, ClipList.data(), regionSize);
		}

		IsClipListChangedFlag = false;
		return DD_OK;
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
			if (lpClipList->rdh.nRgnSize < lpClipList->rdh.nCount * sizeof(RECT))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: nRgnSize is not Correct: " << lpClipList->rdh.nRgnSize << " -> " << (lpClipList->rdh.nCount * sizeof(RECT)));
				return DDERR_INVALIDPARAMS;
			}
			if (lpClipList->rdh.dwSize != sizeof(RGNDATAHEADER))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: dwSize does not match: " << lpClipList->rdh.dwSize);
				return DDERR_INVALIDPARAMS;
			}
			if (lpClipList->rdh.rcBound.left >= lpClipList->rdh.rcBound.right || lpClipList->rdh.rcBound.top >= lpClipList->rdh.rcBound.bottom)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid clip bounds: " << lpClipList->rdh.rcBound);
				return DDERR_INVALIDPARAMS;
			}
			if (lpClipList->rdh.nCount == 0)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: no clip list added!");
				return DDERR_NOCLIPLIST;
			}
			for (DWORD i = 0; i < lpClipList->rdh.nCount; ++i)
			{
				const RECT* rects = reinterpret_cast<const RECT*>(lpClipList->Buffer);

				if (rects[i].left >= rects[i].right || rects[i].top >= rects[i].bottom)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: invalid rect: " << i << " - " << rects[i]);
					return DDERR_INVALIDPARAMS;
				}
			}

			// Set clip list to lpClipList
			IsClipListSet = true;
			IsClipListChangedFlag = true;

			DWORD Size = sizeof(RGNDATAHEADER) + lpClipList->rdh.nRgnSize;
			ClipList.resize(Size);
			memcpy(ClipList.data(), lpClipList, Size);
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

bool m_IDirectDrawClipper::GetClipBoundsFromData(RECT& bounds)
{
	if (IsClipListSet)
	{
		const RGNDATA* data = reinterpret_cast<const RGNDATA*>(ClipList.data());
		bounds = data->rdh.rcBound;

		return true;
	}
	return false;
}

HRESULT m_IDirectDrawClipper::GetClipRegion(HRGN hOutRgn)
{
	if (!hOutRgn)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (IsClipListSet)
	{
		const RGNDATA* pRgnData = reinterpret_cast<const RGNDATA*>(ClipList.data());
		if (pRgnData->rdh.nCount == 0)
		{
			return DDERR_NOCLIPLIST;
		}

		const RECT* rects = reinterpret_cast<const RECT*>(pRgnData->Buffer);
		HRGN hCombinedRgn = CreateRectRgnIndirect(&rects[0]);

		for (DWORD i = 1; i < pRgnData->rdh.nCount; ++i)
		{
			HRGN hTmp = CreateRectRgnIndirect(&rects[i]);
			CombineRgn(hCombinedRgn, hCombinedRgn, hTmp, RGN_OR);
			DeleteObject(hTmp);
		}

		CombineRgn(hOutRgn, hCombinedRgn, nullptr, RGN_COPY);
		DeleteObject(hCombinedRgn);

		return DD_OK;
	}

	return DDERR_NOCLIPLIST;
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
