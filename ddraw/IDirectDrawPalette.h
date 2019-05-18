#pragma once

class m_IDirectDrawPalette : public IDirectDrawPalette, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawPalette *ProxyInterface = nullptr;
	REFIID WrapperID = IID_IDirectDrawPalette;
	ULONG RefCount = 1;
	DWORD paletteCaps = 0;						// Palette flags
	UINT entryCount = 0;						// Number of palette entries
	bool hasAlpha = false;						// Raw palette has alpha data

public:
	m_IDirectDrawPalette(IDirectDrawPalette *aOriginal) : ProxyInterface(aOriginal)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);

		LOG_LIMIT(3, "Create " << __FUNCTION__);
	}
	m_IDirectDrawPalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray) : paletteCaps(dwFlags)
	{
		// Default to 256 entries
		entryCount = 256;

		// Create palette of requested bit size
		if (dwFlags & DDPCAPS_1BIT)
		{
			entryCount = 2;
		}
		else if (dwFlags & DDPCAPS_2BIT)
		{
			entryCount = 4;
		}
		else if (dwFlags & DDPCAPS_4BIT)
		{
			entryCount = 16;
		}
		else if (dwFlags & DDPCAPS_8BIT || dwFlags & DDPCAPS_ALLOW256)
		{
			entryCount = 256;
		}

		// Allocate raw ddraw palette
		rawPalette = new PALETTEENTRY[entryCount];

		// Copy inital palette into raw palette
		if (lpDDColorArray)
		{
			memcpy(rawPalette, lpDDColorArray, sizeof(PALETTEENTRY) * entryCount);
		}

		// Check flags for alpha
		if (dwFlags & DDPCAPS_ALPHA)
		{
			hasAlpha = true;
		}
		else
		{
			hasAlpha = false;
		}

		// Allocate rgb palette
		rgbPalette = new DWORD[entryCount];

		// For all entries
		for (UINT i = 0; i < entryCount; i++)
		{
			// Translate the raw palette to ARGB
			if (hasAlpha)
			{
				// Include peFlags as 8bit alpha
				rgbPalette[i] = D3DCOLOR_ARGB(rawPalette[i].peFlags, rawPalette[i].peRed, rawPalette[i].peGreen, rawPalette[i].peBlue);
			}
			else
			{
				// Alpha is always 255
				rgbPalette[i] = D3DCOLOR_XRGB(rawPalette[i].peRed, rawPalette[i].peGreen, rawPalette[i].peBlue);
			}
		}

		Logging::LogDebug() << "Create " << __FUNCTION__;
	}
	~m_IDirectDrawPalette()
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		if (rawPalette)
		{
			delete rawPalette;
		}
		if (rgbPalette)
		{
			delete rgbPalette;
		}
	}

	DWORD GetDirectXVersion() { return 1; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirectDrawPalette *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDrawPalette *GetWrapperInterface() { return this; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IDirectDrawPalette methods ***/
	STDMETHOD(GetCaps)(THIS_ LPDWORD);
	STDMETHOD(GetEntries)(THIS_ DWORD, DWORD, DWORD, LPPALETTEENTRY);
	STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, DWORD, LPPALETTEENTRY);
	STDMETHOD(SetEntries)(THIS_ DWORD, DWORD, DWORD, LPPALETTEENTRY);

	// Public varables
	bool NewPaletteData = false;				// Flag to see if there is new palette data
	DWORD *rgbPalette = nullptr;				// Rgb translated palette
	LPPALETTEENTRY rawPalette = nullptr;		// Raw palette data
};
