#pragma once

class m_IDirectDrawPalette final : public IDirectDrawPalette, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawPalette *ProxyInterface = nullptr;
	ULONG RefCount = 1;
	const IID WrapperID = IID_IDirectDrawPalette;

	struct DXPALETTEENTRY : public PALETTEENTRY
	{
		// Default constructor
		DXPALETTEENTRY() {}

		// Construct from PALETTEENTRY
		DXPALETTEENTRY(const PALETTEENTRY& pe)
		{
			*reinterpret_cast<DWORD*>(this) = *reinterpret_cast<const DWORD*>(&pe);
		}

		// Assignment from PALETTEENTRY
		DXPALETTEENTRY& operator=(const PALETTEENTRY& pe)
		{
			*reinterpret_cast<DWORD*>(this) = *reinterpret_cast<const DWORD*>(&pe);
			return *this;
		}

		// Conversion to RGBQUAD
		operator RGBQUAD() const
		{
			return RGBQUAD{ peBlue, peGreen, peRed, peFlags };
		}

		// Equality operators
		bool operator==(const DXPALETTEENTRY& other) const
		{
			return *reinterpret_cast<const DWORD*>(this) == *reinterpret_cast<const DWORD*>(&other);
		}

		bool operator!=(const DXPALETTEENTRY& other) const
		{
			return !(*this == other);
		}
	};

	// Convert to Direct3D9
	m_IDirectDrawX *ddrawParent = nullptr;
	DWORD paletteCaps = 0;							// Palette flags
	DXPALETTEENTRY rawPalette[MaxPaletteSize] = {};	// Raw palette data
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
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef) (THIS) override;
	IFACEMETHOD_(ULONG, Release) (THIS) override;

	/*** IDirectDrawPalette methods ***/
	IFACEMETHOD(GetCaps)(THIS_ LPDWORD) override;
	IFACEMETHOD(GetEntries)(THIS_ DWORD, DWORD, DWORD, LPPALETTEENTRY) override;
	IFACEMETHOD(Initialize)(THIS_ LPDIRECTDRAW, DWORD, LPPALETTEENTRY) override;
	IFACEMETHOD(SetEntries)(THIS_ DWORD, DWORD, DWORD, LPPALETTEENTRY) override;

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
