#pragma once

class m_IDirect3DStateBlock9 final : public IDirect3DStateBlock9, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DSTATEBLOCK9 ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	const IID WrapperID = IID_IDirect3DStateBlock9;
	UINT DDKey = NO_MAP_VALUE;

public:
	m_IDirect3DStateBlock9(LPDIRECT3DSTATEBLOCK9 pBlock9, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pBlock9), m_pDeviceEx(pDevice)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface(pDevice, WrapperID, nullptr);

		m_pDeviceEx->GetLookupTable()->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DStateBlock9()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DStateBlock9 methods ***/
	IFACEMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
	IFACEMETHOD(Capture)(THIS) override;
	IFACEMETHOD(Apply)(THIS) override;

	// Helper functions
	LPDIRECT3DSTATEBLOCK9 GetProxyInterface() const { return ProxyInterface; }
	void InitInterface(m_IDirect3DDevice9Ex* Device, REFIID, void*) { m_pDeviceEx = Device; DDKey = NO_MAP_VALUE; }
	void SetProxyAddress(LPDIRECT3DSTATEBLOCK9 Interface) {
		ProxyInterface = Interface;
		m_pDeviceEx->GetLookupTable()->SaveAddress(this, ProxyInterface);
	}
};
