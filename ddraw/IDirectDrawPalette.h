#pragma once

class m_IDirectDrawPalette : public IDirectDrawPalette, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawPalette *ProxyInterface;
	REFIID WrapperID = IID_IDirectDrawPalette;

public:
	m_IDirectDrawPalette(IDirectDrawPalette *aOriginal) : ProxyInterface(aOriginal)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectDrawPalette()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
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
};
