#pragma once

class m_IDirect3D : public IDirect3D, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DX> ProxyInterface;
	IDirect3D *RealInterface;
	REFIID WrapperID = IID_IDirect3D;

public:
	m_IDirect3D(IDirect3D *aOriginal) : RealInterface(aOriginal)
	{
		ProxyInterface = std::make_unique<m_IDirect3DX>((IDirect3D7*)RealInterface, 1, (m_IDirect3D7*)this);
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	~m_IDirect3D()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return 1; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirect3D *GetProxyInterface() { return RealInterface; }
	m_IDirect3DX *GetWrapperInterface() { return ProxyInterface.get(); }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3D methods ***/
	STDMETHOD(Initialize)(THIS_ REFCLSID);
	STDMETHOD(EnumDevices)(THIS_ LPD3DENUMDEVICESCALLBACK, LPVOID);
	STDMETHOD(CreateLight)(THIS_ LPDIRECT3DLIGHT*, IUnknown*);
	STDMETHOD(CreateMaterial)(THIS_ LPDIRECT3DMATERIAL*, IUnknown*);
	STDMETHOD(CreateViewport)(THIS_ LPDIRECT3DVIEWPORT*, IUnknown*);
	STDMETHOD(FindDevice)(THIS_ LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT);
};
