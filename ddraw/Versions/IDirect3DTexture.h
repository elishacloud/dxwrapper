#pragma once

class m_IDirect3DTexture final : public IDirect3DTexture, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DTextureX *ProxyInterface;
	const IID WrapperID = IID_IDirect3DTexture;
	const DWORD DirectXVersion = 1;

public:
	m_IDirect3DTexture(IDirect3DTexture *, m_IDirect3DTextureX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DTexture()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	void SetProxy(m_IDirect3DTextureX* NewProxyInterface)
	{
		ProxyInterface = NewProxyInterface;
		if (NewProxyInterface)
		{
			ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
		}
		else
		{
			ProxyAddressLookupTable.DeleteAddress(this);
		}
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DTexture methods ***/
	IFACEMETHOD(Initialize)(THIS_ LPDIRECT3DDEVICE, LPDIRECTDRAWSURFACE) override;
	IFACEMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE, LPD3DTEXTUREHANDLE) override;
	IFACEMETHOD(PaletteChanged)(THIS_ DWORD, DWORD) override;
	IFACEMETHOD(Load)(THIS_ LPDIRECT3DTEXTURE) override;
	IFACEMETHOD(Unload)(THIS) override;
};
