#pragma once

class m_IDirect3DTexture : public IDirect3DTexture, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DTextureX *ProxyInterface;
	IDirect3DTexture *RealInterface;
	REFIID WrapperID = IID_IDirect3DTexture;
	const DWORD DirectXVersion = 1;

public:
	m_IDirect3DTexture(IDirect3DTexture *aOriginal, m_IDirect3DTextureX *Interface) : RealInterface(aOriginal), ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	~m_IDirect3DTexture()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

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
