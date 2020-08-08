#pragma once

class m_IDirect3D2 : public IDirect3D2, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DX *ProxyInterface;
	IDirect3D2 *RealInterface;
	REFIID WrapperID = IID_IDirect3D2;
	const DWORD DirectXVersion = 2;

public:
	m_IDirect3D2(IDirect3D2 *aOriginal, m_IDirect3DX *Interface) : RealInterface(aOriginal), ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, (RealInterface) ? RealInterface : (void*)ProxyInterface);
	}
	~m_IDirect3D2()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

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
