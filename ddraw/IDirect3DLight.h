#pragma once

class m_IDirect3DLight : public IDirect3DLight, public AddressLookupTableDdrawObject
{
private:
	IDirect3DLight *ProxyInterface;
	REFIID WrapperID = IID_IDirect3DLight;

public:
	m_IDirect3DLight(IDirect3DLight *aOriginal) : ProxyInterface(aOriginal)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DLight()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return 1; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirect3DLight *GetProxyInterface() { return ProxyInterface; }
	m_IDirect3DLight *GetWrapperInterface() { return this; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DLight methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3D);
	STDMETHOD(SetLight)(THIS_ LPD3DLIGHT);
	STDMETHOD(GetLight)(THIS_ LPD3DLIGHT);
};
