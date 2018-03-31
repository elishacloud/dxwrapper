#pragma once

class m_IDirect3DMaterial2 : public IDirect3DMaterial2, public AddressLookupTableDdrawObject
{
private:
	IDirect3DMaterial2 *ProxyInterface;

public:
	m_IDirect3DMaterial2(IDirect3DMaterial2 *aOriginal) : ProxyInterface(aOriginal)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DMaterial2() {}

	IDirect3DMaterial2 *GetProxyInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DMaterial2 methods ***/
	STDMETHOD(SetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(GetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE2, LPD3DMATERIALHANDLE);
};
