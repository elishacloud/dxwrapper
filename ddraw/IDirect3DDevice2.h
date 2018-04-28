#pragma once

class m_IDirect3DDevice2 : public IDirect3DDevice2, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DDeviceX> ProxyInterface;
	IDirect3DDevice2 *RealInterface;
	REFIID WrapperID = IID_IDirect3DDevice2;

public:
	m_IDirect3DDevice2(IDirect3DDevice2 *aOriginal) : RealInterface(aOriginal)
	{
		ProxyInterface = std::make_unique<m_IDirect3DDeviceX>((IDirect3DDevice7*)RealInterface, 2, (m_IDirect3DDevice7*)this);
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	~m_IDirect3DDevice2()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return 2; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirect3DDevice2 *GetProxyInterface() { return RealInterface; }
	m_IDirect3DDeviceX *GetWrapperInterface() { return ProxyInterface.get(); }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DDevice2 methods ***/
	STDMETHOD(GetCaps)(THIS_ LPD3DDEVICEDESC, LPD3DDEVICEDESC);
	STDMETHOD(SwapTextureHandles)(THIS_ LPDIRECT3DTEXTURE2, LPDIRECT3DTEXTURE2);
	STDMETHOD(GetStats)(THIS_ LPD3DSTATS);
	STDMETHOD(AddViewport)(THIS_ LPDIRECT3DVIEWPORT2);
	STDMETHOD(DeleteViewport)(THIS_ LPDIRECT3DVIEWPORT2);
	STDMETHOD(NextViewport)(THIS_ LPDIRECT3DVIEWPORT2, LPDIRECT3DVIEWPORT2*, DWORD);
	STDMETHOD(EnumTextureFormats)(THIS_ LPD3DENUMTEXTUREFORMATSCALLBACK, LPVOID);
	STDMETHOD(BeginScene)(THIS);
	STDMETHOD(EndScene)(THIS);
	STDMETHOD(GetDirect3D)(THIS_ LPDIRECT3D2*);
	STDMETHOD(SetCurrentViewport)(THIS_ LPDIRECT3DVIEWPORT2);
	STDMETHOD(GetCurrentViewport)(THIS_ LPDIRECT3DVIEWPORT2 *);
	STDMETHOD(SetRenderTarget)(THIS_ LPDIRECTDRAWSURFACE, DWORD);
	STDMETHOD(GetRenderTarget)(THIS_ LPDIRECTDRAWSURFACE *);
	STDMETHOD(Begin)(THIS_ D3DPRIMITIVETYPE, D3DVERTEXTYPE, DWORD);
	STDMETHOD(BeginIndexed)(THIS_ D3DPRIMITIVETYPE, D3DVERTEXTYPE, LPVOID, DWORD, DWORD);
	STDMETHOD(Vertex)(THIS_ LPVOID);
	STDMETHOD(Index)(THIS_ WORD);
	STDMETHOD(End)(THIS_ DWORD);
	STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE, LPDWORD);
	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE, DWORD);
	STDMETHOD(GetLightState)(THIS_ D3DLIGHTSTATETYPE, LPDWORD);
	STDMETHOD(SetLightState)(THIS_ D3DLIGHTSTATETYPE, DWORD);
	STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX);
	STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX);
	STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX);
	STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE, D3DVERTEXTYPE, LPVOID, DWORD, DWORD);
	STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE, D3DVERTEXTYPE, LPVOID, DWORD, LPWORD, DWORD, DWORD);
	STDMETHOD(SetClipStatus)(THIS_ LPD3DCLIPSTATUS);
	STDMETHOD(GetClipStatus)(THIS_ LPD3DCLIPSTATUS);
};
