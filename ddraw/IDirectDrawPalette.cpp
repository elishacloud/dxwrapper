/**
* Copyright (C) 2022 Elisha Riedlinger
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
#include "Utils\Utils.h"

HRESULT m_IDirectDrawPalette::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ppvObj)
	{
		return E_POINTER;
	}

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

	if (riid == IID_IDirectDrawPalette || riid == IID_IUnknown)
	{
		AddRef();

		*ppvObj = this;

		return DD_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID);
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
		// Do some error checking
		if (!lpEntries || dwBase > entryCount)
		{
			return DDERR_INVALIDPARAMS;
		}
		dwNumEntries = min(dwNumEntries, entryCount - dwBase);

		// Copy raw palette entries to lpEntries(size dwNumEntries) starting at dwBase
		memcpy(lpEntries, &(rawPalette[dwBase]), dwNumEntries * sizeof(PALETTEENTRY));

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
		// Because the DirectDrawPalette object is initialized when it is created, this method always returns DDERR_ALREADYINITIALIZED.
		return DDERR_ALREADYINITIALIZED;
	}

	if (lpDD)
	{
		lpDD->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDD);
	}

	return ProxyInterface->Initialize(lpDD, dwFlags, lpDDColorTable);
}

HRESULT m_IDirectDrawPalette::SetEntries(DWORD dwFlags, DWORD dwStartingEntry, DWORD dwCount, LPPALETTEENTRY lpEntries)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		// Do some error checking
		if (!lpEntries || dwStartingEntry > entryCount)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Get new entry count
		dwCount = min(dwCount, entryCount - dwStartingEntry);

		// Set start and end entries
		DWORD Start = max(0, dwStartingEntry);
		DWORD End = min(entryCount, dwStartingEntry + dwCount);

		// Handle DDPCAPS_ALLOW256
		if (!(paletteCaps & DDPCAPS_ALLOW256))
		{
			Start = max(1, Start);
			End = min(255, End);
		}

		// lpEntries array location
		DWORD x = (Start - dwStartingEntry);

		// Check if new palette data found
		if (memcmp(&(rawPalette[Start]), &lpEntries[x], (End - Start) * sizeof(PALETTEENTRY)) == S_OK)
		{
			return DD_OK;	// No new data found
		}

		SetCriticalSection();

		// Translate new raw pallete entries to RGB
		for (UINT i = Start; i < End; i++, x++)
		{
			BYTE alpha = (paletteCaps & DDPCAPS_ALPHA) ? lpEntries[x].peFlags : 0x00;
			// Palette entry
			rawPalette[i].peFlags = alpha;
			rawPalette[i].peRed = lpEntries[x].peRed;
			rawPalette[i].peGreen = lpEntries[x].peGreen;
			rawPalette[i].peBlue = lpEntries[x].peBlue;
			// RGB palette
			rgbPalette[i].rgbBlue = lpEntries[x].peBlue;
			rgbPalette[i].rgbGreen = lpEntries[x].peGreen;
			rgbPalette[i].rgbRed = lpEntries[x].peRed;
			rgbPalette[i].rgbReserved = alpha;
		}

		// Note that there is new palette data
		PaletteUSN++;

		// Present new palette
		if (ddrawParent)
		{
			if (paletteCaps & DDPCAPS_PRIMARYSURFACE)
			{
				if (paletteCaps & DDPCAPS_VSYNC)
				{
					ddrawParent->SetVsync();
				}

				m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = ddrawParent->GetPrimarySurface();
				if (lpDDSrcSurfaceX)
				{
					lpDDSrcSurfaceX->PresentSurface(false);
				}
			}
		}

		ReleaseCriticalSection();

		return DD_OK;
	}

	return ProxyInterface->SetEntries(dwFlags, dwStartingEntry, dwCount, lpEntries);
}

void m_IDirectDrawPalette::InitPalette()
{
	if (ProxyInterface)
	{
		return;
	}

	// Compute new USN number
	LARGE_INTEGER PerformanceCount = {};
	QueryPerformanceCounter(&PerformanceCount);
	DWORD Seed = PerformanceCount.HighPart ^ PerformanceCount.LowPart;
	PaletteUSN = (PaletteUSN ^ Seed) + ((DWORD)this ^ ((Seed << 16) + (Seed >> 16))) + Utils::ReverseBits(Seed);

	// Create palette of requested bit size
	if ((paletteCaps & DDPCAPS_8BIT) || (paletteCaps & DDPCAPS_ALLOW256))
	{
		entryCount = 256;
	}
	else if (paletteCaps & DDPCAPS_1BIT)
	{
		entryCount = 2;
	}
	else if (paletteCaps & DDPCAPS_2BIT)
	{
		entryCount = 4;
	}
	else if (paletteCaps & DDPCAPS_4BIT)
	{
		entryCount = 16;
	}

	// Check for unsupported flags
	if (paletteCaps & DDPCAPS_PRIMARYSURFACELEFT)
	{
		Logging::Log() << __FUNCTION__ << " Warning: Primary surface left is not implemented.";
	}

	// The palette entries are 1 byte each if the DDPCAPS_8BITENTRIES flag is set, and 4 bytes otherwise.
	if (paletteCaps & DDPCAPS_ALPHA)
	{
		Logging::Log() << __FUNCTION__ << " Warning: alpha palette entries are not implemented.";
	}

	// The palette entries are 1 byte each if the DDPCAPS_8BITENTRIES flag is set, and 4 bytes otherwise.
	// This flag is valid only when used with the DDPCAPS_1BIT, DDPCAPS_2BIT, or DDPCAPS_4BIT flag, and when the target surface is 8 bpp.
	if ((paletteCaps & DDPCAPS_8BITENTRIES) && (paletteCaps & (DDPCAPS_1BIT | DDPCAPS_2BIT | DDPCAPS_4BIT)))
	{
		Logging::Log() << __FUNCTION__ << " Warning: DDPCAPS_8BITENTRIES is not implemented.";
	}
	else
	{
		paletteCaps &= ~DDPCAPS_8BITENTRIES;
	}

	// Init palette entry 255 to white to simulate ddraw functionality
	if (entryCount == 256)
	{
		// Palette entry
		rawPalette[255].peRed = 0xFF;
		rawPalette[255].peGreen = 0xFF;
		rawPalette[255].peBlue = 0xFF;
		// RGB palette
		rgbPalette[255].rgbBlue = 0xFF;
		rgbPalette[255].rgbGreen = 0xFF;
		rgbPalette[255].rgbRed = 0xFF;
	}

	if (ddrawParent)
	{
		ddrawParent->AddPaletteToVector(this);
	}
}

void m_IDirectDrawPalette::ReleasePalette()
{
	if (ddrawParent && !Config.Exiting)
	{
		ddrawParent->RemovePaletteFromVector(this);
	}
}
