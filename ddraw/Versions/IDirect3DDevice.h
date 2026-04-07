#pragma once

class m_IDirect3DDevice final : public IDirect3DDevice, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DDeviceX *ProxyInterface;
	const IID WrapperID = IID_IDirect3DDevice;
	const DWORD DirectXVersion = 1;

public:
	m_IDirect3DDevice(IDirect3DDevice *, m_IDirect3DDeviceX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DDevice()
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

	/*** IDirect3DDevice methods ***/
	IFACEMETHOD(Initialize)(THIS_ LPDIRECT3D, LPGUID, LPD3DDEVICEDESC) override;
	IFACEMETHOD(GetCaps)(THIS_ LPD3DDEVICEDESC, LPD3DDEVICEDESC) override;
	IFACEMETHOD(SwapTextureHandles)(THIS_ LPDIRECT3DTEXTURE, LPDIRECT3DTEXTURE) override;
	IFACEMETHOD(CreateExecuteBuffer)(THIS_ LPD3DEXECUTEBUFFERDESC, LPDIRECT3DEXECUTEBUFFER*, IUnknown*) override;
	IFACEMETHOD(GetStats)(THIS_ LPD3DSTATS) override;
	IFACEMETHOD(Execute)(THIS_ LPDIRECT3DEXECUTEBUFFER, LPDIRECT3DVIEWPORT, DWORD) override;
	IFACEMETHOD(AddViewport)(THIS_ LPDIRECT3DVIEWPORT) override;
	IFACEMETHOD(DeleteViewport)(THIS_ LPDIRECT3DVIEWPORT) override;
	IFACEMETHOD(NextViewport)(THIS_ LPDIRECT3DVIEWPORT, LPDIRECT3DVIEWPORT*, DWORD) override;
	IFACEMETHOD(Pick)(THIS_ LPDIRECT3DEXECUTEBUFFER, LPDIRECT3DVIEWPORT, DWORD, LPD3DRECT) override;
	IFACEMETHOD(GetPickRecords)(THIS_ LPDWORD, LPD3DPICKRECORD) override;
	IFACEMETHOD(EnumTextureFormats)(THIS_ LPD3DENUMTEXTUREFORMATSCALLBACK, LPVOID) override;
	IFACEMETHOD(CreateMatrix)(THIS_ LPD3DMATRIXHANDLE) override;
	IFACEMETHOD(SetMatrix)(THIS_ D3DMATRIXHANDLE, const LPD3DMATRIX) override;
	IFACEMETHOD(GetMatrix)(THIS_ D3DMATRIXHANDLE, LPD3DMATRIX) override;
	IFACEMETHOD(DeleteMatrix)(THIS_ D3DMATRIXHANDLE) override;
	IFACEMETHOD(BeginScene)(THIS) override;
	IFACEMETHOD(EndScene)(THIS) override;
	IFACEMETHOD(GetDirect3D)(THIS_ LPDIRECT3D*) override;
};
