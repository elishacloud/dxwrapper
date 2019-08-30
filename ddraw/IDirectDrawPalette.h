#pragma once

class m_IDirectDrawPalette : public IDirectDrawPalette, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawPalette *ProxyInterface = nullptr;
	REFIID WrapperID = IID_IDirectDrawPalette;
	ULONG RefCount = 1;

	// Convert to Direct3D9
	m_IDirectDrawX *ddrawParent = nullptr;
	DWORD paletteCaps = 0;						// Palette flags
	DWORD entryCount = 0;						// Number of palette entries
	bool hasAlpha = false;						// Raw palette has alpha data

	struct RGBDWORD {
		union
		{
			D3DCOLOR PaletteColor;
			DDARGB pe;
		};
	};

public:
	m_IDirectDrawPalette(IDirectDrawPalette *aOriginal) : ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")");

		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	m_IDirectDrawPalette(m_IDirectDrawX *Interface, DWORD dwFlags, LPPALETTEENTRY lpDDColorArray) : ddrawParent(Interface), paletteCaps(dwFlags)
	{
		LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")");

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

		// Allocate rgb palette
		rgbPalette = new RGBDWORD[entryCount];

		// Set initial entries
		SetEntries(dwFlags, 0, entryCount, lpDDColorArray);
	}
	~m_IDirectDrawPalette()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting device!");

		ProxyAddressLookupTable.DeleteAddress(this);

		if (rawPalette)
		{
			delete rawPalette;
		}
		if (rgbPalette)
		{
			delete rgbPalette;
		}
		if (Config.Dd7to9 && !Config.Exiting)
		{
			ReleaseInterface();
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

	// Functions handling the ddraw parent interface
	void SetDdrawParent(m_IDirectDrawX *ddraw) { ddrawParent = ddraw; }
	void ClearDdraw() { ddrawParent = nullptr; }

	// Release interface
	void ReleaseInterface();

	// Other functions
	DWORD GetEntryCount() { return entryCount; };

	// Public varables
	DWORD PaletteUSN = (DWORD)this;				// The USN that's used to see if the palette data was updated
	RGBDWORD *rgbPalette = nullptr;				// Rgb translated palette
	LPPALETTEENTRY rawPalette = nullptr;		// Raw palette data
};
