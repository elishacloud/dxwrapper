#pragma once

class m_IDirect3DMaterial3 : public IDirect3DMaterial3, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DMaterialX> ProxyInterface;
	IDirect3DMaterial3 *RealInterface;

public:
	m_IDirect3DMaterial3(IDirect3DMaterial3 *aOriginal) : RealInterface(aOriginal)
	{
		ProxyInterface = std::make_unique<m_IDirect3DMaterialX>(RealInterface, 3, this);
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	~m_IDirect3DMaterial3()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	IDirect3DMaterial3 *GetProxyInterface() { return RealInterface; }
	DWORD GetDirectXVersion() { return 3; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DMaterial3 methods ***/
	STDMETHOD(SetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(GetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE3, LPD3DMATERIALHANDLE);
};
