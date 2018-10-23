#pragma once

class m_IDirect3D : public IDirect3D, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DX> UniqueProxyInterface;
	m_IDirect3DX *ProxyInterface;
	IDirect3D *RealInterface;
	REFIID WrapperID = IID_IDirect3D;
	const DWORD DirectXVersion = 1;

public:
	m_IDirect3D(IDirect3D *aOriginal) : RealInterface(aOriginal)
	{
		UniqueProxyInterface = std::make_unique<m_IDirect3DX>((IDirect3D7*)RealInterface, DirectXVersion, (m_IDirect3D7*)this);
		ProxyInterface = UniqueProxyInterface.get();
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	m_IDirect3D(m_IDirect3DX *aOriginal) : ProxyInterface(aOriginal)
	{
		RealInterface = nullptr;
	}
	~m_IDirect3D()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return DirectXVersion; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirect3D *GetProxyInterface() { return RealInterface; }
	m_IDirect3DX *GetWrapperInterface() { return ProxyInterface; }

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
