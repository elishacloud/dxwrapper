#pragma once

class m_IDirect3DVideoDevice9 final : public IDirect3DVideoDevice9, public AddressLookupTableD3d9Object
{
private:
	IDirect3DVideoDevice9* ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	const IID WrapperID = IID_IDirect3DVideoDevice9;

public:
	m_IDirect3DVideoDevice9(m_IDirect3DVideoDevice9* pVADevice9, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pVADevice9), m_pDeviceEx(pDevice)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface(pDevice, WrapperID, nullptr);

		m_pDeviceEx->GetLookupTable()->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DVideoDevice9()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DVideoDevice9 methods ***/
	IFACEMETHOD(CreateSurface)(THIS_ UINT Width, UINT Height, UINT BackBuffers, D3DFORMAT Format, D3DPOOL Pool, DWORD Usage, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) override;
	IFACEMETHOD(GetDXVACompressedBufferInfo)(THIS_ GUID* pGuid, DXVAUncompDataInfo* pUncompData, DWORD* pNumBuffers, DXVACompBufferInfo* pBufferInfo) override;
	IFACEMETHOD(GetDXVAGuids)(THIS_ DWORD* pNumGuids, GUID* pGuids) override;
	IFACEMETHOD(GetDXVAInternalInfo)(THIS_ GUID* pGuid, DXVAUncompDataInfo* pUncompData, DWORD* pMemoryUsed) override;
	IFACEMETHOD(GetUncompressedDXVAFormats)(THIS_ GUID* pGuid, DWORD* pNumFormats, D3DFORMAT* pFormats) override;
	IFACEMETHOD(CreateDXVADevice)(THIS_ GUID* pGuid, DXVAUncompDataInfo* pUncompData, LPVOID pData, DWORD DataSize, IDirect3DDXVADevice9** ppDXVADevice) override;

	// Helper functions
	IDirect3DVideoDevice9* GetProxyInterface() const { return ProxyInterface; }
	void InitInterface(m_IDirect3DDevice9Ex* Device, REFIID, void*) { m_pDeviceEx = Device; }
};
