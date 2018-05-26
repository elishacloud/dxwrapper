#pragma once

class m_IDirectDrawPalette : public IDirectDrawPalette, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawPalette *ProxyInterface;
	REFIID WrapperID = IID_IDirectDrawPalette;
	ULONG RefCount = 1;
	UINT32 *rgbPalette = nullptr;				// Rgb translated palette
	LPPALETTEENTRY rawPalette = nullptr;		// Raw palette data
	DWORD paletteCaps = 0;						// Palette flags
	UINT entryCount = 0;							// Number of palette entries
	bool hasAlpha = false;						// Raw palette has alpha data

public:
	m_IDirectDrawPalette(IDirectDrawPalette *aOriginal) : ProxyInterface(aOriginal)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectDrawPalette()
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		// Free objects
		if (rawPalette != NULL)
		{
			delete rawPalette;
		}
		if (rgbPalette != NULL)
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
	HRESULT WrapperInitialize(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray);
};
