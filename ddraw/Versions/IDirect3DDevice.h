#pragma once

class m_IDirect3DDevice : public IDirect3DDevice, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DDeviceX *ProxyInterface;
	IDirect3DDevice *RealInterface;
	REFIID WrapperID = IID_IDirect3DDevice;
	const DWORD DirectXVersion = 1;

public:
	m_IDirect3DDevice(IDirect3DDevice *aOriginal, m_IDirect3DDeviceX *Interface) : RealInterface(aOriginal), ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	~m_IDirect3DDevice()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DDevice methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3D, LPGUID, LPD3DDEVICEDESC);
	STDMETHOD(GetCaps)(THIS_ LPD3DDEVICEDESC, LPD3DDEVICEDESC);
	STDMETHOD(SwapTextureHandles)(THIS_ LPDIRECT3DTEXTURE, LPDIRECT3DTEXTURE);
	STDMETHOD(CreateExecuteBuffer)(THIS_ LPD3DEXECUTEBUFFERDESC, LPDIRECT3DEXECUTEBUFFER*, IUnknown*);
	STDMETHOD(GetStats)(THIS_ LPD3DSTATS);
	STDMETHOD(Execute)(THIS_ LPDIRECT3DEXECUTEBUFFER, LPDIRECT3DVIEWPORT, DWORD);
	STDMETHOD(AddViewport)(THIS_ LPDIRECT3DVIEWPORT);
	STDMETHOD(DeleteViewport)(THIS_ LPDIRECT3DVIEWPORT);
	STDMETHOD(NextViewport)(THIS_ LPDIRECT3DVIEWPORT, LPDIRECT3DVIEWPORT*, DWORD);
	STDMETHOD(Pick)(THIS_ LPDIRECT3DEXECUTEBUFFER, LPDIRECT3DVIEWPORT, DWORD, LPD3DRECT);
	STDMETHOD(GetPickRecords)(THIS_ LPDWORD, LPD3DPICKRECORD);
	STDMETHOD(EnumTextureFormats)(THIS_ LPD3DENUMTEXTUREFORMATSCALLBACK, LPVOID);
	STDMETHOD(CreateMatrix)(THIS_ LPD3DMATRIXHANDLE);
	STDMETHOD(SetMatrix)(THIS_ D3DMATRIXHANDLE, const LPD3DMATRIX);
	STDMETHOD(GetMatrix)(THIS_ D3DMATRIXHANDLE, LPD3DMATRIX);
	STDMETHOD(DeleteMatrix)(THIS_ D3DMATRIXHANDLE);
	STDMETHOD(BeginScene)(THIS);
	STDMETHOD(EndScene)(THIS);
	STDMETHOD(GetDirect3D)(THIS_ LPDIRECT3D*);
};
