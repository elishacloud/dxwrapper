#pragma once

class m_IDirect3DTexture2 : public IDirect3DTexture2, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DTextureX> ProxyInterface;
	IDirect3DTexture2 *RealInterface;

public:
	m_IDirect3DTexture2(IDirect3DTexture2 *aOriginal) : RealInterface(aOriginal)
	{
		ProxyInterface = std::make_unique<m_IDirect3DTextureX>(RealInterface, 2, this);
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	~m_IDirect3DTexture2()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	IDirect3DTexture2 *GetProxyInterface() { return RealInterface; }
	DWORD GetDirectXVersion() { return 2; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DTexture2 methods ***/
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE2, LPD3DTEXTUREHANDLE);
	STDMETHOD(PaletteChanged)(THIS_ DWORD, DWORD);
	STDMETHOD(Load)(THIS_ LPDIRECT3DTEXTURE2);
};
