#pragma once

class m_IDirect3DViewport3 final : public IDirect3DViewport3, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DViewportX *ProxyInterface;
	const IID WrapperID = IID_IDirect3DViewport3;
	const DWORD DirectXVersion = 3;

public:
	m_IDirect3DViewport3(IDirect3DViewport3 *, m_IDirect3DViewportX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DViewport3()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	void SetProxy(m_IDirect3DViewportX* NewProxyInterface)
	{
		ProxyInterface = NewProxyInterface;
		if (NewProxyInterface)
		{
			ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
		}
		else
		{
			ProxyAddressLookupTable.DeleteAddress(this);
		}
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DViewport methods ***/
	IFACEMETHOD(Initialize)(THIS_ LPDIRECT3D) override;
	IFACEMETHOD(GetViewport)(THIS_ LPD3DVIEWPORT) override;
	IFACEMETHOD(SetViewport)(THIS_ LPD3DVIEWPORT) override;
	IFACEMETHOD(TransformVertices)(THIS_ DWORD, LPD3DTRANSFORMDATA, DWORD, LPDWORD) override;
	IFACEMETHOD(LightElements)(THIS_ DWORD, LPD3DLIGHTDATA) override;
	IFACEMETHOD(SetBackground)(THIS_ D3DMATERIALHANDLE) override;
	IFACEMETHOD(GetBackground)(THIS_ LPD3DMATERIALHANDLE, LPBOOL) override;
	IFACEMETHOD(SetBackgroundDepth)(THIS_ LPDIRECTDRAWSURFACE) override;
	IFACEMETHOD(GetBackgroundDepth)(THIS_ LPDIRECTDRAWSURFACE*, LPBOOL) override;
	IFACEMETHOD(Clear)(THIS_ DWORD, LPD3DRECT, DWORD) override;
	IFACEMETHOD(AddLight)(THIS_ LPDIRECT3DLIGHT) override;
	IFACEMETHOD(DeleteLight)(THIS_ LPDIRECT3DLIGHT) override;
	IFACEMETHOD(NextLight)(THIS_ LPDIRECT3DLIGHT, LPDIRECT3DLIGHT*, DWORD) override;
	/*** IDirect3DViewport2 methods ***/
	IFACEMETHOD(GetViewport2)(THIS_ LPD3DVIEWPORT2) override;
	IFACEMETHOD(SetViewport2)(THIS_ LPD3DVIEWPORT2) override;
	/*** IDirect3DViewport3 methods ***/
	IFACEMETHOD(SetBackgroundDepth2)(THIS_ LPDIRECTDRAWSURFACE4) override;
	IFACEMETHOD(GetBackgroundDepth2)(THIS_ LPDIRECTDRAWSURFACE4*, LPBOOL) override;
	IFACEMETHOD(Clear2)(THIS_ DWORD, LPD3DRECT, DWORD, D3DCOLOR, D3DVALUE, DWORD) override;
};
