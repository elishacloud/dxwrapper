#pragma once

class m_IDirect3D2 final : public IDirect3D2, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DX *ProxyInterface;
	const IID WrapperID = IID_IDirect3D2;
	const DWORD DirectXVersion = 2;

public:
	m_IDirect3D2(IDirect3D2 *, m_IDirect3DX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3D2()
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

	/*** IDirect3D2 methods ***/
	IFACEMETHOD(EnumDevices)(THIS_ LPD3DENUMDEVICESCALLBACK, LPVOID) override;
	IFACEMETHOD(CreateLight)(THIS_ LPDIRECT3DLIGHT*, IUnknown*) override;
	IFACEMETHOD(CreateMaterial)(THIS_ LPDIRECT3DMATERIAL2*, IUnknown*) override;
	IFACEMETHOD(CreateViewport)(THIS_ LPDIRECT3DVIEWPORT2*, IUnknown*) override;
	IFACEMETHOD(FindDevice)(THIS_ LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT) override;
	IFACEMETHOD(CreateDevice)(THIS_ REFCLSID, LPDIRECTDRAWSURFACE, LPDIRECT3DDEVICE2*) override;
};
