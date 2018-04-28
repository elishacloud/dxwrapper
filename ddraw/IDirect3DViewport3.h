#pragma once

class m_IDirect3DViewport3 : public IDirect3DViewport3, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DViewportX> ProxyInterface;
	IDirect3DViewport3 *RealInterface;
	REFIID WrapperID = IID_IDirect3DViewport3;

public:
	m_IDirect3DViewport3(IDirect3DViewport3 *aOriginal) : RealInterface(aOriginal)
	{
		ProxyInterface = std::make_unique<m_IDirect3DViewportX>(RealInterface, 3, this);
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	~m_IDirect3DViewport3()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return 3; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirect3DViewport3 *GetProxyInterface() { return RealInterface; }
	m_IDirect3DViewportX *GetWrapperInterface() { return ProxyInterface.get(); }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DViewport2 methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3D);
	STDMETHOD(GetViewport)(THIS_ LPD3DVIEWPORT);
	STDMETHOD(SetViewport)(THIS_ LPD3DVIEWPORT);
	STDMETHOD(TransformVertices)(THIS_ DWORD, LPD3DTRANSFORMDATA, DWORD, LPDWORD);
	STDMETHOD(LightElements)(THIS_ DWORD, LPD3DLIGHTDATA);
	STDMETHOD(SetBackground)(THIS_ D3DMATERIALHANDLE);
	STDMETHOD(GetBackground)(THIS_ LPD3DMATERIALHANDLE, LPBOOL);
	STDMETHOD(SetBackgroundDepth)(THIS_ LPDIRECTDRAWSURFACE);
	STDMETHOD(GetBackgroundDepth)(THIS_ LPDIRECTDRAWSURFACE*, LPBOOL);
	STDMETHOD(Clear)(THIS_ DWORD, LPD3DRECT, DWORD);
	STDMETHOD(AddLight)(THIS_ LPDIRECT3DLIGHT);
	STDMETHOD(DeleteLight)(THIS_ LPDIRECT3DLIGHT);
	STDMETHOD(NextLight)(THIS_ LPDIRECT3DLIGHT, LPDIRECT3DLIGHT*, DWORD);
	STDMETHOD(GetViewport2)(THIS_ LPD3DVIEWPORT2);
	STDMETHOD(SetViewport2)(THIS_ LPD3DVIEWPORT2);
	STDMETHOD(SetBackgroundDepth2)(THIS_ LPDIRECTDRAWSURFACE4);
	STDMETHOD(GetBackgroundDepth2)(THIS_ LPDIRECTDRAWSURFACE4*, LPBOOL);
	STDMETHOD(Clear2)(THIS_ DWORD, LPD3DRECT, DWORD, D3DCOLOR, D3DVALUE, DWORD);
};
