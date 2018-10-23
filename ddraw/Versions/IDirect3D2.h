#pragma once

class m_IDirect3D2 : public IDirect3D2, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DX> UniqueProxyInterface;
	m_IDirect3DX *ProxyInterface;
	IDirect3D2 *RealInterface;
	REFIID WrapperID = IID_IDirect3D2;
	const DWORD DirectXVersion = 2;

public:
	m_IDirect3D2(IDirect3D2 *aOriginal) : RealInterface(aOriginal)
	{
		UniqueProxyInterface = std::make_unique<m_IDirect3DX>((IDirect3D7*)RealInterface, DirectXVersion, (m_IDirect3D7*)this);
		ProxyInterface = UniqueProxyInterface.get();
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	m_IDirect3D2(m_IDirect3DX *aOriginal) : ProxyInterface(aOriginal)
	{
		RealInterface = nullptr;
	}
	~m_IDirect3D2()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return DirectXVersion; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirect3D2 *GetProxyInterface() { return RealInterface; }
	m_IDirect3DX *GetWrapperInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3D2 methods ***/
	STDMETHOD(EnumDevices)(THIS_ LPD3DENUMDEVICESCALLBACK, LPVOID);
	STDMETHOD(CreateLight)(THIS_ LPDIRECT3DLIGHT*, IUnknown*);
	STDMETHOD(CreateMaterial)(THIS_ LPDIRECT3DMATERIAL2*, IUnknown*);
	STDMETHOD(CreateViewport)(THIS_ LPDIRECT3DVIEWPORT2*, IUnknown*);
	STDMETHOD(FindDevice)(THIS_ LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT);
	STDMETHOD(CreateDevice)(THIS_ REFCLSID, LPDIRECTDRAWSURFACE, LPDIRECT3DDEVICE2*);
};
