#pragma once

class m_IDirectDrawPalette : public IDirectDrawPalette, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawPalette *ProxyInterface;
	REFIID WrapperID = IID_IDirectDrawPalette;
	ULONG RefCount = 1;
	DWORD paletteCaps = 0;						// Palette flags
	UINT entryCount = 0;						// Number of palette entries
	bool hasAlpha = false;						// Raw palette has alpha data

public:
	m_IDirectDrawPalette(IDirectDrawPalette *aOriginal) : ProxyInterface(aOriginal)
	{
		if (ProxyInterface)
		{
			ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
		}
	}
	m_IDirectDrawPalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray)
	{
		ProxyInterface = nullptr;

		// Save palette caps
		paletteCaps = dwFlags;

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
		memcpy(rawPalette, lpDDColorArray, sizeof(PALETTEENTRY) * entryCount);

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
		rgbPalette = new UINT32[entryCount];

		// For all entries
		for (UINT i = 0; i < entryCount; i++)
		{
			// Translate the raw palette to ARGB
			if (hasAlpha)
			{
				// Include peFlags as 8bit alpha
				rgbPalette[i] = rawPalette[i].peFlags << 24;
				rgbPalette[i] |= rawPalette[i].peRed << 16;
				rgbPalette[i] |= rawPalette[i].peGreen << 8;
				rgbPalette[i] |= rawPalette[i].peBlue;
			}
			else
			{
				// Alpha is always 255
				rgbPalette[i] = 0xFF000000;
				rgbPalette[i] |= rawPalette[i].peRed << 16;
				rgbPalette[i] |= rawPalette[i].peGreen << 8;
				rgbPalette[i] |= rawPalette[i].peBlue;
			}
		}
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

	UINT32 *rgbPalette = nullptr;				// Rgb translated palette
	LPPALETTEENTRY rawPalette = nullptr;		// Raw palette data

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
};
