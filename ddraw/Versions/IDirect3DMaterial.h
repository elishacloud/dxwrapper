#pragma once

class m_IDirect3DMaterial : public IDirect3DMaterial, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DMaterialX> ProxyInterface;
	IDirect3DMaterial *RealInterface;
	REFIID WrapperID = IID_IDirect3DMaterial;

public:
	m_IDirect3DMaterial(IDirect3DMaterial *aOriginal) : RealInterface(aOriginal)
	{
		ProxyInterface = std::make_unique<m_IDirect3DMaterialX>((IDirect3DMaterial3*)RealInterface, 1, (m_IDirect3DMaterial3*)this);
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	~m_IDirect3DMaterial()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return 1; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirect3DMaterial *GetProxyInterface() { return RealInterface; }
	m_IDirect3DMaterialX *GetWrapperInterface() { return ProxyInterface.get(); }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DMaterial methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3D);
	STDMETHOD(SetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(GetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE, LPD3DMATERIALHANDLE);
	STDMETHOD(Reserve)(THIS);
	STDMETHOD(Unreserve)(THIS);
};
