#pragma once

class m_IDirect3DViewport2 : public IDirect3DViewport2, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DViewportX> ProxyInterface;
	IDirect3DViewport2 *RealInterface;

public:
	m_IDirect3DViewport2(IDirect3DViewport2 *aOriginal) : RealInterface(aOriginal)
	{
		ProxyInterface = std::make_unique<m_IDirect3DViewportX>((IDirect3DViewport3*)RealInterface, 2, (m_IDirect3DViewport3*)this);
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	~m_IDirect3DViewport2()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	IDirect3DViewport2 *GetProxyInterface() { return RealInterface; }
	DWORD GetDirectXVersion() { return 2; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DViewport methods ***/
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
};
