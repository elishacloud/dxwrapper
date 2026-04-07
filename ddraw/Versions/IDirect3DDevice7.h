#pragma once

class m_IDirect3DDevice7 final : public IDirect3DDevice7, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DDeviceX *ProxyInterface;
	const IID WrapperID = IID_IDirect3DDevice7;
	const DWORD DirectXVersion = 7;

public:
	m_IDirect3DDevice7(IDirect3DDevice7 *, m_IDirect3DDeviceX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DDevice7()
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

	/*** IDirect3DDevice7 methods ***/
	IFACEMETHOD(GetCaps)(THIS_ LPD3DDEVICEDESC7) override;
	IFACEMETHOD(EnumTextureFormats)(THIS_ LPD3DENUMPIXELFORMATSCALLBACK, LPVOID) override;
	IFACEMETHOD(BeginScene)(THIS) override;
	IFACEMETHOD(EndScene)(THIS) override;
	IFACEMETHOD(GetDirect3D)(THIS_ LPDIRECT3D7*) override;
	IFACEMETHOD(SetRenderTarget)(THIS_ LPDIRECTDRAWSURFACE7, DWORD) override;
	IFACEMETHOD(GetRenderTarget)(THIS_ LPDIRECTDRAWSURFACE7 *) override;
	IFACEMETHOD(Clear)(THIS_ DWORD, LPD3DRECT, DWORD, D3DCOLOR, D3DVALUE, DWORD) override;
	IFACEMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX) override;
	IFACEMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX) override;
	IFACEMETHOD(SetViewport)(THIS_ LPD3DVIEWPORT7) override;
	IFACEMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX) override;
	IFACEMETHOD(GetViewport)(THIS_ LPD3DVIEWPORT7) override;
	IFACEMETHOD(SetMaterial)(THIS_ LPD3DMATERIAL7) override;
	IFACEMETHOD(GetMaterial)(THIS_ LPD3DMATERIAL7) override;
	IFACEMETHOD(SetLight)(THIS_ DWORD, LPD3DLIGHT7) override;
	IFACEMETHOD(GetLight)(THIS_ DWORD, LPD3DLIGHT7) override;
	IFACEMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE, DWORD) override;
	IFACEMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE, LPDWORD) override;
	IFACEMETHOD(BeginStateBlock)(THIS) override;
	IFACEMETHOD(EndStateBlock)(THIS_ LPDWORD) override;
	IFACEMETHOD(PreLoad)(THIS_ LPDIRECTDRAWSURFACE7) override;
	IFACEMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE, DWORD, LPVOID, DWORD, DWORD) override;
	IFACEMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE, DWORD, LPVOID, DWORD, LPWORD, DWORD, DWORD) override;
	IFACEMETHOD(SetClipStatus)(THIS_ LPD3DCLIPSTATUS) override;
	IFACEMETHOD(GetClipStatus)(THIS_ LPD3DCLIPSTATUS) override;
	IFACEMETHOD(DrawPrimitiveStrided)(THIS_ D3DPRIMITIVETYPE, DWORD, LPD3DDRAWPRIMITIVESTRIDEDDATA, DWORD, DWORD) override;
	IFACEMETHOD(DrawIndexedPrimitiveStrided)(THIS_ D3DPRIMITIVETYPE, DWORD, LPD3DDRAWPRIMITIVESTRIDEDDATA, DWORD, LPWORD, DWORD, DWORD) override;
	IFACEMETHOD(DrawPrimitiveVB)(THIS_ D3DPRIMITIVETYPE, LPDIRECT3DVERTEXBUFFER7, DWORD, DWORD, DWORD) override;
	IFACEMETHOD(DrawIndexedPrimitiveVB)(THIS_ D3DPRIMITIVETYPE, LPDIRECT3DVERTEXBUFFER7, DWORD, DWORD, LPWORD, DWORD, DWORD) override;
	IFACEMETHOD(ComputeSphereVisibility)(THIS_ LPD3DVECTOR, LPD3DVALUE, DWORD, DWORD, LPDWORD) override;
	IFACEMETHOD(GetTexture)(THIS_ DWORD, LPDIRECTDRAWSURFACE7 *) override;
	IFACEMETHOD(SetTexture)(THIS_ DWORD, LPDIRECTDRAWSURFACE7) override;
	IFACEMETHOD(GetTextureStageState)(THIS_ DWORD, D3DTEXTURESTAGESTATETYPE, LPDWORD) override;
	IFACEMETHOD(SetTextureStageState)(THIS_ DWORD, D3DTEXTURESTAGESTATETYPE, DWORD) override;
	IFACEMETHOD(ValidateDevice)(THIS_ LPDWORD) override;
	IFACEMETHOD(ApplyStateBlock)(THIS_ DWORD) override;
	IFACEMETHOD(CaptureStateBlock)(THIS_ DWORD) override;
	IFACEMETHOD(DeleteStateBlock)(THIS_ DWORD) override;
	IFACEMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE, LPDWORD) override;
	IFACEMETHOD(Load)(THIS_ LPDIRECTDRAWSURFACE7, LPPOINT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD) override;
	IFACEMETHOD(LightEnable)(THIS_ DWORD, BOOL) override;
	IFACEMETHOD(GetLightEnable)(THIS_ DWORD, BOOL*) override;
	IFACEMETHOD(SetClipPlane)(THIS_ DWORD, D3DVALUE*) override;
	IFACEMETHOD(GetClipPlane)(THIS_ DWORD, D3DVALUE*) override;
	IFACEMETHOD(GetInfo)(THIS_ DWORD, LPVOID, DWORD) override;
};
