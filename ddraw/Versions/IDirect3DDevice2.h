#pragma once

class m_IDirect3DDevice2 final : public IDirect3DDevice2, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DDeviceX *ProxyInterface;
	const IID WrapperID = IID_IDirect3DDevice2;
	const DWORD DirectXVersion = 2;

public:
	m_IDirect3DDevice2(IDirect3DDevice2 *, m_IDirect3DDeviceX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DDevice2()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	void SetProxy(m_IDirect3DDeviceX* NewProxyInterface)
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

	/*** IDirect3DDevice2 methods ***/
	IFACEMETHOD(GetCaps)(THIS_ LPD3DDEVICEDESC, LPD3DDEVICEDESC) override;
	IFACEMETHOD(SwapTextureHandles)(THIS_ LPDIRECT3DTEXTURE2, LPDIRECT3DTEXTURE2) override;
	IFACEMETHOD(GetStats)(THIS_ LPD3DSTATS) override;
	IFACEMETHOD(AddViewport)(THIS_ LPDIRECT3DVIEWPORT2) override;
	IFACEMETHOD(DeleteViewport)(THIS_ LPDIRECT3DVIEWPORT2) override;
	IFACEMETHOD(NextViewport)(THIS_ LPDIRECT3DVIEWPORT2, LPDIRECT3DVIEWPORT2*, DWORD) override;
	IFACEMETHOD(EnumTextureFormats)(THIS_ LPD3DENUMTEXTUREFORMATSCALLBACK, LPVOID) override;
	IFACEMETHOD(BeginScene)(THIS) override;
	IFACEMETHOD(EndScene)(THIS) override;
	IFACEMETHOD(GetDirect3D)(THIS_ LPDIRECT3D2*) override;
	IFACEMETHOD(SetCurrentViewport)(THIS_ LPDIRECT3DVIEWPORT2) override;
	IFACEMETHOD(GetCurrentViewport)(THIS_ LPDIRECT3DVIEWPORT2 *) override;
	IFACEMETHOD(SetRenderTarget)(THIS_ LPDIRECTDRAWSURFACE, DWORD) override;
	IFACEMETHOD(GetRenderTarget)(THIS_ LPDIRECTDRAWSURFACE *) override;
	IFACEMETHOD(Begin)(THIS_ D3DPRIMITIVETYPE, D3DVERTEXTYPE, DWORD) override;
	IFACEMETHOD(BeginIndexed)(THIS_ D3DPRIMITIVETYPE, D3DVERTEXTYPE, LPVOID, DWORD, DWORD) override;
	IFACEMETHOD(Vertex)(THIS_ LPVOID) override;
	IFACEMETHOD(Index)(THIS_ WORD) override;
	IFACEMETHOD(End)(THIS_ DWORD) override;
	IFACEMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE, LPDWORD) override;
	IFACEMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE, DWORD) override;
	IFACEMETHOD(GetLightState)(THIS_ D3DLIGHTSTATETYPE, LPDWORD) override;
	IFACEMETHOD(SetLightState)(THIS_ D3DLIGHTSTATETYPE, DWORD) override;
	IFACEMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX) override;
	IFACEMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX) override;
	IFACEMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX) override;
	IFACEMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE, D3DVERTEXTYPE, LPVOID, DWORD, DWORD) override;
	IFACEMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE, D3DVERTEXTYPE, LPVOID, DWORD, LPWORD, DWORD, DWORD) override;
	IFACEMETHOD(SetClipStatus)(THIS_ LPD3DCLIPSTATUS) override;
	IFACEMETHOD(GetClipStatus)(THIS_ LPD3DCLIPSTATUS) override;
};
