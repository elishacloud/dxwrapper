#pragma once

class m_IDirect3DIndexBuffer9 final : public IDirect3DIndexBuffer9, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DINDEXBUFFER9 ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	const IID WrapperID = IID_IDirect3DIndexBuffer9;

public:
	m_IDirect3DIndexBuffer9(LPDIRECT3DINDEXBUFFER9 pBuffer9, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pBuffer9), m_pDeviceEx(pDevice)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface(pDevice, WrapperID, nullptr);

		m_pDeviceEx->GetLookupTable()->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DIndexBuffer9()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DIndexBuffer9 methods ***/
	IFACEMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
	IFACEMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) override;
	IFACEMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) override;
	IFACEMETHOD(FreePrivateData)(THIS_ REFGUID refguid) override;
	IFACEMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) override;
	IFACEMETHOD_(DWORD, GetPriority)(THIS) override;
	IFACEMETHOD_(void, PreLoad)(THIS) override;
	IFACEMETHOD_(D3DRESOURCETYPE, GetType)(THIS) override;
	IFACEMETHOD(Lock)(THIS_ UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags) override;
	IFACEMETHOD(Unlock)(THIS) override;
	IFACEMETHOD(GetDesc)(THIS_ D3DINDEXBUFFER_DESC *pDesc) override;

	// Helper functions
	LPDIRECT3DINDEXBUFFER9 GetProxyInterface() const { return ProxyInterface; }
	void InitInterface(m_IDirect3DDevice9Ex* Device, REFIID, void*) { m_pDeviceEx = Device; }
};
