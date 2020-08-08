#pragma once

class m_IDirect3DMaterial3 : public IDirect3DMaterial3, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DMaterialX *ProxyInterface;
	IDirect3DMaterial3 *RealInterface;
	REFIID WrapperID = IID_IDirect3DMaterial3;
	const DWORD DirectXVersion = 3;

public:
	m_IDirect3DMaterial3(IDirect3DMaterial3 *aOriginal, m_IDirect3DMaterialX *Interface) : RealInterface(aOriginal), ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, (RealInterface) ? RealInterface : (void*)ProxyInterface);
	}
	~m_IDirect3DMaterial3()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DMaterial3 methods ***/
	STDMETHOD(SetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(GetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE3, LPD3DMATERIALHANDLE);
};
