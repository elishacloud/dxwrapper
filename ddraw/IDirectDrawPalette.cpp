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

HRESULT m_IDirectDrawPalette::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if ((riid == IID_IDirectDrawPalette || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return DD_OK;
		}
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, this);
}

ULONG m_IDirectDrawPalette::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawPalette::Release()
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

HRESULT m_IDirectDrawPalette::GetCaps(LPDWORD lpdwCaps)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if (!lpdwCaps)
		{
			return DDERR_INVALIDPARAMS;
		}

		// set return data to current palette caps
		*lpdwCaps = paletteCaps;

		return DD_OK;
	}

	return ProxyInterface->GetCaps(lpdwCaps);
}

HRESULT m_IDirectDrawPalette::GetEntries(DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpEntries)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		// lpEntries cannot be null and dwFlags must be 0
		if (!lpEntries || !rawPalette || dwBase > entryCount || dwFlags != 0)
		{
			return DDERR_INVALIDPARAMS;
		}
		dwNumEntries = min(dwNumEntries, entryCount - dwBase);

		// Copy raw palette entries to lpEntries(size dwNumEntries) starting at dwBase
		memcpy(lpEntries, &(rawPalette[dwBase]), sizeof(PALETTEENTRY) * dwNumEntries);

		// dwNumEntries is the number of palette entries that can fit in the array that lpEntries 
		// specifies. The colors of the palette entries are returned in sequence, from the value
		// of the dwStartingEntry parameter through the value of the dwCount parameter minus 1. 
		// (These parameters are set by IDirectDrawPalette::SetEntries.) 

		return DD_OK;
	}

	return ProxyInterface->GetEntries(dwFlags, dwBase, dwNumEntries, lpEntries);
}

HRESULT m_IDirectDrawPalette::Initialize(LPDIRECTDRAW lpDD, DWORD dwFlags, LPPALETTEENTRY lpDDColorTable)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		return DD_OK;
	}
	
	if (lpDD)
	{
		lpDD = static_cast<m_IDirectDraw *>(lpDD)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(lpDD, dwFlags, lpDDColorTable);
}

HRESULT m_IDirectDrawPalette::SetEntries(DWORD dwFlags, DWORD dwStartingEntry, DWORD dwCount, LPPALETTEENTRY lpEntries)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		// lpEntries cannot be null and dwFlags must be 0
		if (!lpEntries || !rawPalette || !rgbPalette || dwStartingEntry > entryCount || dwFlags != 0)
		{
			return DDERR_INVALIDPARAMS;
		}
		dwCount = min(dwCount, entryCount - dwStartingEntry);

		// Copy raw palette entries from dwStartingEntry and of count dwCount
		memcpy(&(rawPalette[dwStartingEntry]), lpEntries, sizeof(PALETTEENTRY) * dwCount);

		// Translate new raw pallete entries to RGB(make sure not to go off the end of the memory)
		for (UINT i = dwStartingEntry; i < dwStartingEntry + dwCount; i++)
		{
			// Translate the raw palette to ARGB
			if (hasAlpha)
			{
				// Include peFlags as 8bit alpha
				rgbPalette[i].pe.blue = rawPalette[i].peBlue;
				rgbPalette[i].pe.green = rawPalette[i].peGreen;
				rgbPalette[i].pe.red = rawPalette[i].peRed;
				rgbPalette[i].pe.alpha = rawPalette[i].peFlags;
			}
			else
			{
				// Alpha is always 255
				rgbPalette[i].pe.blue = rawPalette[i].peBlue;
				rgbPalette[i].pe.green = rawPalette[i].peGreen;
				rgbPalette[i].pe.red = rawPalette[i].peRed;
				rgbPalette[i].pe.alpha = 0xFF;
			}
		}

		// Note that there is new palette data
		PaletteUSN++;

		// Present new palette
		if (ddrawParent)
		{
			m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = ddrawParent->GetPrimarySurface();
			if (lpDDSrcSurfaceX)
			{
				lpDDSrcSurfaceX->PresentSurface();
			}
		}

		return DD_OK;
	}

	return ProxyInterface->SetEntries(dwFlags, dwStartingEntry, dwCount, lpEntries);
}

void m_IDirectDrawPalette::ReleaseInterface()
{
	SetCriticalSection();
	if (ddrawParent)
	{
		ddrawParent->RemovePaletteFromVector(this);
	}
	ReleaseCriticalSection();
}
