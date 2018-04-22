#pragma once

class m_IDirect3DTexture : public IDirect3DTexture, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DTextureX> ProxyInterface;
	IDirect3DTexture *RealInterface;

public:
	m_IDirect3DTexture(IDirect3DTexture *aOriginal) : RealInterface(aOriginal)
	{
		ProxyInterface = std::make_unique<m_IDirect3DTextureX>((IDirect3DTexture2*)RealInterface, 1, (m_IDirect3DTexture2*)this);
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	~m_IDirect3DTexture()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	IDirect3DTexture *GetProxyInterface() { return RealInterface; }
	DWORD GetDirectXVersion() { return 1; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DTexture methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3DDEVICE, LPDIRECTDRAWSURFACE);
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE, LPD3DTEXTUREHANDLE);
	STDMETHOD(PaletteChanged)(THIS_ DWORD, DWORD);
	STDMETHOD(Load)(THIS_ LPDIRECT3DTEXTURE);
	STDMETHOD(Unload)(THIS);
};
