#pragma once

class m_IDirect3DViewport final : public IDirect3DViewport, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DViewportX *ProxyInterface;
	const IID WrapperID = IID_IDirect3DViewport;
	const DWORD DirectXVersion = 1;

public:
	m_IDirect3DViewport(IDirect3DViewport *, m_IDirect3DViewportX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DViewport()
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
};
