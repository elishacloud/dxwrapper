#pragma once

class m_IDirect3DDevice3 final : public IDirect3DDevice3, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DDeviceX *ProxyInterface;
	const IID WrapperID = IID_IDirect3DDevice3;
	const DWORD DirectXVersion = 3;

public:
	m_IDirect3DDevice3(IDirect3DDevice3 *, m_IDirect3DDeviceX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DDevice3()
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

	/*** IDirect3DDevice3 methods ***/
	IFACEMETHOD(GetCaps)(THIS_ LPD3DDEVICEDESC, LPD3DDEVICEDESC) override;
	IFACEMETHOD(GetStats)(THIS_ LPD3DSTATS) override;
	IFACEMETHOD(AddViewport)(THIS_ LPDIRECT3DVIEWPORT3) override;
	IFACEMETHOD(DeleteViewport)(THIS_ LPDIRECT3DVIEWPORT3) override;
	IFACEMETHOD(NextViewport)(THIS_ LPDIRECT3DVIEWPORT3, LPDIRECT3DVIEWPORT3*, DWORD) override;
	IFACEMETHOD(EnumTextureFormats)(THIS_ LPD3DENUMPIXELFORMATSCALLBACK, LPVOID) override;
	IFACEMETHOD(BeginScene)(THIS) override;
	IFACEMETHOD(EndScene)(THIS) override;
	IFACEMETHOD(GetDirect3D)(THIS_ LPDIRECT3D3*) override;
	IFACEMETHOD(SetCurrentViewport)(THIS_ LPDIRECT3DVIEWPORT3) override;
	IFACEMETHOD(GetCurrentViewport)(THIS_ LPDIRECT3DVIEWPORT3 *) override;
	IFACEMETHOD(SetRenderTarget)(THIS_ LPDIRECTDRAWSURFACE4, DWORD) override;
	IFACEMETHOD(GetRenderTarget)(THIS_ LPDIRECTDRAWSURFACE4 *) override;
	IFACEMETHOD(Begin)(THIS_ D3DPRIMITIVETYPE, DWORD, DWORD) override;
	IFACEMETHOD(BeginIndexed)(THIS_ D3DPRIMITIVETYPE, DWORD, LPVOID, DWORD, DWORD) override;
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
	IFACEMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE, DWORD, LPVOID, DWORD, DWORD) override;
	IFACEMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE, DWORD, LPVOID, DWORD, LPWORD, DWORD, DWORD) override;
	IFACEMETHOD(SetClipStatus)(THIS_ LPD3DCLIPSTATUS) override;
	IFACEMETHOD(GetClipStatus)(THIS_ LPD3DCLIPSTATUS) override;
	IFACEMETHOD(DrawPrimitiveStrided)(THIS_ D3DPRIMITIVETYPE, DWORD, LPD3DDRAWPRIMITIVESTRIDEDDATA, DWORD, DWORD) override;
	IFACEMETHOD(DrawIndexedPrimitiveStrided)(THIS_ D3DPRIMITIVETYPE, DWORD, LPD3DDRAWPRIMITIVESTRIDEDDATA, DWORD, LPWORD, DWORD, DWORD) override;
	IFACEMETHOD(DrawPrimitiveVB)(THIS_ D3DPRIMITIVETYPE, LPDIRECT3DVERTEXBUFFER, DWORD, DWORD, DWORD) override;
	IFACEMETHOD(DrawIndexedPrimitiveVB)(THIS_ D3DPRIMITIVETYPE, LPDIRECT3DVERTEXBUFFER, LPWORD, DWORD, DWORD) override;
	IFACEMETHOD(ComputeSphereVisibility)(THIS_ LPD3DVECTOR, LPD3DVALUE, DWORD, DWORD, LPDWORD) override;
	IFACEMETHOD(GetTexture)(THIS_ DWORD, LPDIRECT3DTEXTURE2 *) override;
	IFACEMETHOD(SetTexture)(THIS_ DWORD, LPDIRECT3DTEXTURE2) override;
	IFACEMETHOD(GetTextureStageState)(THIS_ DWORD, D3DTEXTURESTAGESTATETYPE, LPDWORD) override;
	IFACEMETHOD(SetTextureStageState)(THIS_ DWORD, D3DTEXTURESTAGESTATETYPE, DWORD) override;
	IFACEMETHOD(ValidateDevice)(THIS_ LPDWORD) override;
};
