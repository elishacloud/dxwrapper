#pragma once

class m_IDirect3D3 final : public IDirect3D3, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DX *ProxyInterface;
	const IID WrapperID = IID_IDirect3D3;
	const DWORD DirectXVersion = 3;

public:
	m_IDirect3D3(IDirect3D3 *, m_IDirect3DX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3D3()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	void SetProxy(m_IDirect3DX* NewProxyInterface)
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

	/*** IDirect3D3 methods ***/
	IFACEMETHOD(EnumDevices)(THIS_ LPD3DENUMDEVICESCALLBACK, LPVOID) override;
	IFACEMETHOD(CreateLight)(THIS_ LPDIRECT3DLIGHT*, LPUNKNOWN) override;
	IFACEMETHOD(CreateMaterial)(THIS_ LPDIRECT3DMATERIAL3*, LPUNKNOWN) override;
	IFACEMETHOD(CreateViewport)(THIS_ LPDIRECT3DVIEWPORT3*, LPUNKNOWN) override;
	IFACEMETHOD(FindDevice)(THIS_ LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT) override;
	IFACEMETHOD(CreateDevice)(THIS_ REFCLSID, LPDIRECTDRAWSURFACE4, LPDIRECT3DDEVICE3*, LPUNKNOWN) override;
	IFACEMETHOD(CreateVertexBuffer)(THIS_ LPD3DVERTEXBUFFERDESC, LPDIRECT3DVERTEXBUFFER*, DWORD, LPUNKNOWN) override;
	IFACEMETHOD(EnumZBufferFormats)(THIS_ REFCLSID, LPD3DENUMPIXELFORMATSCALLBACK, LPVOID) override;
	IFACEMETHOD(EvictManagedTextures)(THIS) override;
};
