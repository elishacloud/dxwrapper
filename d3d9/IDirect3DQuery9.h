#pragma once

class m_IDirect3DQuery9 : public IDirect3DQuery9, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DQUERY9 ProxyInterface;
	m_IDirect3DDevice9* m_pDevice = nullptr;
	m_IDirect3DDevice9Ex* m_pDeviceEx = nullptr;

public:
	m_IDirect3DQuery9(LPDIRECT3DQUERY9 pQuery9, m_IDirect3DDevice9* pDevice) : ProxyInterface(pQuery9), m_pDevice(pDevice)
	{
		pDevice->ProxyAddressLookupTable->SaveAddress(this, ProxyInterface);
	}
	m_IDirect3DQuery9(LPDIRECT3DQUERY9 pQuery9, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pQuery9), m_pDeviceEx(pDevice)
	{
		pDevice->ProxyAddressLookupTable->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DQuery9() {}

	LPDIRECT3DQUERY9 GetProxyInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DQuery9 methods ***/
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	STDMETHOD_(D3DQUERYTYPE, GetType)(THIS);
	STDMETHOD_(DWORD, GetDataSize)(THIS);
	STDMETHOD(Issue)(THIS_ DWORD dwIssueFlags);
	STDMETHOD(GetData)(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags);
};
