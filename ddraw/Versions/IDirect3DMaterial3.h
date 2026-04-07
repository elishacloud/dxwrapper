#pragma once

class m_IDirect3DMaterial3 final : public IDirect3DMaterial3, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DMaterialX *ProxyInterface;
	const IID WrapperID = IID_IDirect3DMaterial3;
	const DWORD DirectXVersion = 3;

public:
	m_IDirect3DMaterial3(IDirect3DMaterial3 *, m_IDirect3DMaterialX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DMaterial3()
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

	/*** IDirect3DMaterial3 methods ***/
	IFACEMETHOD(SetMaterial)(THIS_ LPD3DMATERIAL) override;
	IFACEMETHOD(GetMaterial)(THIS_ LPD3DMATERIAL) override;
	IFACEMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE3, LPD3DMATERIALHANDLE) override;
};
