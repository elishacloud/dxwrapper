#pragma once

class m_IDirect3D final : public IDirect3D, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DX *ProxyInterface;
	const IID WrapperID = IID_IDirect3D;
	const DWORD DirectXVersion = 1;

public:
	m_IDirect3D(IDirect3D *, m_IDirect3DX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3D()
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

	/*** IDirect3D methods ***/
	IFACEMETHOD(Initialize)(THIS_ REFCLSID) override;
	IFACEMETHOD(EnumDevices)(THIS_ LPD3DENUMDEVICESCALLBACK, LPVOID) override;
	IFACEMETHOD(CreateLight)(THIS_ LPDIRECT3DLIGHT*, IUnknown*) override;
	IFACEMETHOD(CreateMaterial)(THIS_ LPDIRECT3DMATERIAL*, IUnknown*) override;
	IFACEMETHOD(CreateViewport)(THIS_ LPDIRECT3DVIEWPORT*, IUnknown*) override;
	IFACEMETHOD(FindDevice)(THIS_ LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT) override;
};
