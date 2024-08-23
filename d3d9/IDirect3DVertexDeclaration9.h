#pragma once

class m_IDirect3DVertexDeclaration9 : public IDirect3DVertexDeclaration9, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DVERTEXDECLARATION9 ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;

	AddressLookupTableD3d9<m_IDirect3DDevice9Ex>* ProxyAddressLookupTable;

public:
	m_IDirect3DVertexDeclaration9(LPDIRECT3DVERTEXDECLARATION9 pDeclaration9, m_IDirect3DDevice9Ex* pDevice) :
		ProxyInterface(pDeclaration9), m_pDeviceEx(pDevice), ProxyAddressLookupTable(pDevice->ProxyAddressLookupTable)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		ProxyAddressLookupTable->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DVertexDeclaration9()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DVertexDeclaration9 methods ***/
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	STDMETHOD(GetDeclaration)(THIS_ D3DVERTEXELEMENT9* pElement, UINT* pNumElements);

	// Helper functions
	LPDIRECT3DVERTEXDECLARATION9 GetProxyInterface() { return ProxyInterface; }
};
