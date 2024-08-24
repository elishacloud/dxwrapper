#pragma once

class m_IDirect3DPixelShader9 : public IDirect3DPixelShader9, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DPIXELSHADER9 ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	REFIID WrapperID = IID_IDirect3DPixelShader9;

public:
	m_IDirect3DPixelShader9(LPDIRECT3DPIXELSHADER9 pShader9, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pShader9), m_pDeviceEx(pDevice)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		ProxyAddressLookupTable9.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DPixelShader9()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DPixelShader9 methods ***/
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	STDMETHOD(GetFunction)(THIS_ void* pData, UINT* pSizeOfData);

	// Helper functions
	LPDIRECT3DPIXELSHADER9 GetProxyInterface() { return ProxyInterface; }
};
