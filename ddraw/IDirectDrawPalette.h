#pragma once

class m_IDirectDrawPalette : public IDirectDrawPalette, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawPalette *ProxyInterface = nullptr;
	ULONG RefCount = 1;
	const IID WrapperID = IID_IDirectDrawPalette;

	// Convert to Direct3D9
	m_IDirectDrawX *ddrawParent = nullptr;
	DWORD paletteCaps = 0;							// Palette flags
	PALETTEENTRY rawPalette[MaxPaletteSize] = {};	// Raw palette data
	RGBQUAD rgbPalette[MaxPaletteSize] = {};		// Rgb translated palette
	DWORD PaletteUSN;								// The USN that's used to see if the palette data was updated (don't initialize)
	DWORD entryCount = MaxPaletteSize;				// Number of palette entries (Default to 256 entries)

	// Interface initialization functions
	void InitInterface(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray);
	void ReleaseInterface();

public:
	m_IDirectDrawPalette(IDirectDrawPalette *aOriginal) : ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		if (Config.Dd7to9)
		{
			Logging::Log() << __FUNCTION__ << " (" << this << ") Warning: created from non-dd7to9 interface!";
		}

		InitInterface(0, nullptr);

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	m_IDirectDrawPalette(m_IDirectDrawX *Interface, DWORD dwFlags, LPPALETTEENTRY lpDDColorArray) : ddrawParent(Interface)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface(dwFlags, lpDDColorArray);

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	~m_IDirectDrawPalette()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseInterface();

		ProxyAddressLookupTable.DeleteAddress(this);
	}

	void SetProxy(IDirectDrawPalette* NewProxyInterface, m_IDirectDrawX* NewParent, DWORD dwFlags, LPPALETTEENTRY lpDDColorArray)
	{
		if (NewProxyInterface || NewParent)
		{
			RefCount = 1;
			ProxyInterface = NewProxyInterface;
			ddrawParent = NewParent;
			InitInterface(dwFlags, lpDDColorArray);
			ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
		}
		else
		{
			ReleaseInterface();
			ProxyAddressLookupTable.DeleteAddress(this);
			ProxyInterface = nullptr;
			ddrawParent = nullptr;
		}
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
	const PALETTEENTRY* GetPaletteEntries() const { return rawPalette; }
	const RGBQUAD* GetRGBPalette() const { return rgbPalette; }
	DWORD GetPaletteUSN() const { return PaletteUSN; }
	DWORD GetEntryCount() const { return entryCount; }
	void SetPrimary() { paletteCaps |= DDPCAPS_PRIMARYSURFACE; }
	void RemovePrimary() { paletteCaps &= ~DDPCAPS_PRIMARYSURFACE; }
	static m_IDirectDrawPalette* CreateDirectDrawPalette(IDirectDrawPalette* aOriginal, m_IDirectDrawX* NewParent, DWORD dwFlags, LPPALETTEENTRY lpDDColorArray);
};
