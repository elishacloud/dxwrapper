#pragma once

class m_IDirect3DStateBlock9 : public IDirect3DStateBlock9, public AddressLookupTableObject
{
private:
	LPDIRECT3DSTATEBLOCK9 ProxyInterface;
	m_IDirect3DDevice9* m_pDevice = nullptr;
	m_IDirect3DDevice9Ex* m_pDeviceEx = nullptr;

public:
	m_IDirect3DStateBlock9(LPDIRECT3DSTATEBLOCK9 pBlock9, m_IDirect3DDevice9* pDevice) : ProxyInterface(pBlock9), m_pDevice(pDevice)
	{
		pDevice->ProxyAddressLookupTable->SaveAddress(this, ProxyInterface);
	}
	m_IDirect3DStateBlock9(LPDIRECT3DSTATEBLOCK9 pBlock9, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pBlock9), m_pDeviceEx(pDevice)
	{
		pDevice->ProxyAddressLookupTable->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DStateBlock9() {}

	LPDIRECT3DSTATEBLOCK9 GetProxyInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DStateBlock9 methods ***/
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	STDMETHOD(Capture)(THIS);
	STDMETHOD(Apply)(THIS);
};
