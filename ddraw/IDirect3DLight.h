#pragma once

class m_IDirect3DLight : public IDirect3DLight, public AddressLookupTableObject
{
private:
	IDirect3DLight *ProxyInterface;

public:
	m_IDirect3DLight(IDirect3DLight *aOriginal, void *) : ProxyInterface(aOriginal)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DLight() {}

	IDirect3DLight *GetProxyInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DLight methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3D);
	STDMETHOD(SetLight)(THIS_ LPD3DLIGHT);
	STDMETHOD(GetLight)(THIS_ LPD3DLIGHT);
};
