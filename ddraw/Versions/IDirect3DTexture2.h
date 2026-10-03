#pragma once

class m_IDirect3DTexture2 final : public IDirect3DTexture2, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DTextureX *ProxyInterface;
	const IID WrapperID = IID_IDirect3DTexture2;
	const DWORD DirectXVersion = 2;

public:
	m_IDirect3DTexture2(IDirect3DTexture2 *, m_IDirect3DTextureX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTableDdraw.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DTexture2()
	{
		ProxyAddressLookupTableDdraw.DeleteAddress(this);
	}

	void SetProxy(m_IDirect3DTextureX* NewProxyInterface)
	{
		ProxyInterface = NewProxyInterface;
		if (NewProxyInterface)
		{
			ProxyAddressLookupTableDdraw.SaveAddress(this, ProxyInterface);
		}
		else
		{
			ProxyAddressLookupTableDdraw.DeleteAddress(this);
		}
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DTexture2 methods ***/
	IFACEMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE2, LPD3DTEXTUREHANDLE) override;
	IFACEMETHOD(PaletteChanged)(THIS_ DWORD, DWORD) override;
	IFACEMETHOD(Load)(THIS_ LPDIRECT3DTEXTURE2) override;
};
