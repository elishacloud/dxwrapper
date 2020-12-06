#pragma once

class m_IDirectDrawPalette : public IDirectDrawPalette, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawPalette *ProxyInterface = nullptr;
	REFIID WrapperID = IID_IDirectDrawPalette;
	ULONG RefCount = 1;

	// Used for RGB palette
	struct RGBDWORD {
		union
		{
			D3DCOLOR PaletteColor;
			DDARGB pe;
		};
	};

	// Convert to Direct3D9
	m_IDirectDrawX *ddrawParent = nullptr;
	DWORD paletteCaps = 0;						// Palette flags
	LPPALETTEENTRY rawPalette = nullptr;		// Raw palette data
	RGBDWORD *rgbPalette = nullptr;				// Rgb translated palette
	DWORD PaletteUSN = (DWORD)this;				// The USN that's used to see if the palette data was updated
	DWORD entryCount = 256;						// Number of palette entries (Default to 256 entries)

	// Interface initialization functions
	void InitPalette();
	void ReleasePalette();

public:
	m_IDirectDrawPalette(IDirectDrawPalette *aOriginal) : ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitPalette();

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	m_IDirectDrawPalette(m_IDirectDrawX *Interface, DWORD dwFlags, LPPALETTEENTRY lpDDColorArray) : ddrawParent(Interface), paletteCaps(dwFlags & ~DDPCAPS_INITIALIZE)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitPalette();

		// Set initial entries after initializing the palette
		if (lpDDColorArray)
		{
			SetEntries(dwFlags, 0, entryCount, lpDDColorArray);
		}

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	~m_IDirectDrawPalette()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleasePalette();

		ProxyAddressLookupTable.DeleteAddress(this);
	}

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

	// Helper functions
	RGBDWORD *GetRgbPalette() { return rgbPalette; }
	DWORD GetPaletteUSN() { return PaletteUSN; }
	DWORD GetEntryCount() { return entryCount; }
};
