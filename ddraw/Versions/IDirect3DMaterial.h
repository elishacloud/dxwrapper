#pragma once

class m_IDirect3DMaterial : public IDirect3DMaterial, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DMaterialX *ProxyInterface;
	IDirect3DMaterial *RealInterface;
	REFIID WrapperID = IID_IDirect3DMaterial;
	const DWORD DirectXVersion = 1;

public:
	m_IDirect3DMaterial(IDirect3DMaterial *aOriginal, m_IDirect3DMaterialX *Interface) : RealInterface(aOriginal), ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, (RealInterface) ? RealInterface : (void*)ProxyInterface);
	}
	~m_IDirect3DMaterial()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

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
