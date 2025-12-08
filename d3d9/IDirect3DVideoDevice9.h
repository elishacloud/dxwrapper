#pragma once

class m_IDirect3DVideoDevice9 : public IDirect3DVideoDevice9, public AddressLookupTableD3d9Object
{
private:
	m_IDirect3DVideoDevice9* ProxyInterface;
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
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DVideoDevice9 methods ***/
	STDMETHOD(CreateSurface)(THIS_ UINT Width, UINT Height, UINT BackBuffers, D3DFORMAT Format, D3DPOOL Pool, DWORD Usage, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
	STDMETHOD(GetDXVACompressedBufferInfo)(THIS_ GUID* pGuid, DXVAUncompDataInfo* pUncompData, DWORD* pNumBuffers, DXVACompBufferInfo* pBufferInfo);
	STDMETHOD(GetDXVAGuids)(THIS_ DWORD* pNumGuids, GUID* pGuids);
	STDMETHOD(GetDXVAInternalInfo)(THIS_ GUID* pGuid, DXVAUncompDataInfo* pUncompData, DWORD* pMemoryUsed);
	STDMETHOD(GetUncompressedDXVAFormats)(THIS_ GUID* pGuid, DWORD* pNumFormats, D3DFORMAT* pFormats);
	STDMETHOD(CreateDXVADevice)(THIS_ GUID* pGuid, DXVAUncompDataInfo* pUncompData, LPVOID pData, DWORD DataSize, IDirect3DDXVADevice9** ppDXVADevice);

	// Helper functions
	m_IDirect3DVideoDevice9* GetProxyInterface() const { return ProxyInterface; }
	void InitInterface(m_IDirect3DDevice9Ex* Device, REFIID, void*) { m_pDeviceEx = Device; }
};
