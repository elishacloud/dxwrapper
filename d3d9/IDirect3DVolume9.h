#pragma once

class m_IDirect3DVolume9 final : public IDirect3DVolume9, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DVOLUME9 ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	const IID WrapperID = IID_IDirect3DVolume9;

public:
	m_IDirect3DVolume9(LPDIRECT3DVOLUME9 pVolume8, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pVolume8), m_pDeviceEx(pDevice)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface(pDevice, WrapperID, nullptr);

		m_pDeviceEx->GetLookupTable()->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DVolume9()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DVolume9 methods ***/
	IFACEMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
	IFACEMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) override;
	IFACEMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) override;
	IFACEMETHOD(FreePrivateData)(THIS_ REFGUID refguid) override;
	IFACEMETHOD(GetContainer)(THIS_ REFIID riid, void** ppContainer) override;
	IFACEMETHOD(GetDesc)(THIS_ D3DVOLUME_DESC *pDesc) override;
	IFACEMETHOD(LockBox)(THIS_ D3DLOCKED_BOX * pLockedVolume, CONST D3DBOX* pBox, DWORD Flags) override;
	IFACEMETHOD(UnlockBox)(THIS) override;

	// Helper functions
	LPDIRECT3DVOLUME9 GetProxyInterface() const { return ProxyInterface; }
	void InitInterface(m_IDirect3DDevice9Ex* Device, REFIID, void*) { m_pDeviceEx = Device; }
};
