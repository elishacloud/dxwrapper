#pragma once

class m_IDirect3DVertexShader9 : public IDirect3DVertexShader9, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DVERTEXSHADER9 ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;

	AddressLookupTableD3d9<m_IDirect3DDevice9Ex>* ProxyAddressLookupTable;

public:
	m_IDirect3DVertexShader9(LPDIRECT3DVERTEXSHADER9 pShader9, m_IDirect3DDevice9Ex* pDevice) :
		ProxyInterface(pShader9), m_pDeviceEx(pDevice), ProxyAddressLookupTable(pDevice->ProxyAddressLookupTable)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		ProxyAddressLookupTable->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DVertexShader9()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DVertexShader9 methods ***/
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	STDMETHOD(GetFunction)(THIS_ void* pData, UINT* pSizeOfData);

	// Helper functions
	LPDIRECT3DVERTEXSHADER9 GetProxyInterface() { return ProxyInterface; }
};
