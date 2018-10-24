#pragma once

class m_IDirect3DTexture : public IDirect3DTexture, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DTextureX> UniqueProxyInterface;
	m_IDirect3DTextureX *ProxyInterface;
	IDirect3DTexture *RealInterface;
	REFIID WrapperID = IID_IDirect3DTexture;
	const DWORD DirectXVersion = 1;

public:
	m_IDirect3DTexture(IDirect3DTexture *aOriginal) : RealInterface(aOriginal)
	{
		UniqueProxyInterface = std::make_unique<m_IDirect3DTextureX>((IDirect3DTexture2*)RealInterface, DirectXVersion, (m_IDirect3DTexture2*)this);
		ProxyInterface = UniqueProxyInterface.get();
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	m_IDirect3DTexture(m_IDirect3DTextureX *aOriginal) : ProxyInterface(aOriginal)
	{
		RealInterface = nullptr;
	}
	~m_IDirect3DTexture()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return DirectXVersion; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirect3DTexture *GetProxyInterface() { return RealInterface; }
	m_IDirect3DTextureX *GetWrapperInterface() { return ProxyInterface; }

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
