#pragma once

class m_IDirect3DQuery9 final : public IDirect3DQuery9, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DQUERY9 ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	const IID WrapperID = IID_IDirect3DQuery9;

public:
	m_IDirect3DQuery9(LPDIRECT3DQUERY9 pQuery9, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pQuery9), m_pDeviceEx(pDevice)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface(pDevice, WrapperID, nullptr);

		m_pDeviceEx->GetLookupTable()->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DQuery9()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DQuery9 methods ***/
	IFACEMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
	IFACEMETHOD_(D3DQUERYTYPE, GetType)(THIS) override;
	IFACEMETHOD_(DWORD, GetDataSize)(THIS) override;
	IFACEMETHOD(Issue)(THIS_ DWORD dwIssueFlags) override;
	IFACEMETHOD(GetData)(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags) override;

	// Helper functions
	LPDIRECT3DQUERY9 GetProxyInterface() const { return ProxyInterface; }
	void InitInterface(m_IDirect3DDevice9Ex* Device, REFIID, void*) { m_pDeviceEx = Device; }
};
