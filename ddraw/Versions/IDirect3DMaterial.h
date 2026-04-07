#pragma once

class m_IDirect3DMaterial final : public IDirect3DMaterial, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DMaterialX *ProxyInterface;
	const IID WrapperID = IID_IDirect3DMaterial;
	const DWORD DirectXVersion = 1;

public:
	m_IDirect3DMaterial(IDirect3DMaterial *, m_IDirect3DMaterialX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DMaterial()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	void SetProxy(m_IDirect3DMaterialX* NewProxyInterface)
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

	/*** IDirect3DMaterial methods ***/
	IFACEMETHOD(Initialize)(THIS_ LPDIRECT3D) override;
	IFACEMETHOD(SetMaterial)(THIS_ LPD3DMATERIAL) override;
	IFACEMETHOD(GetMaterial)(THIS_ LPD3DMATERIAL) override;
	IFACEMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE, LPD3DMATERIALHANDLE) override;
	IFACEMETHOD(Reserve)(THIS) override;
	IFACEMETHOD(Unreserve)(THIS) override;
};
