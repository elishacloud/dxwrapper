#pragma once

class m_IDirect3DDXVADevice9 : public IDirect3DDXVADevice9, public AddressLookupTableD3d9Object
{
private:
	m_IDirect3DDXVADevice9* ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	const IID WrapperID = IID_IDirect3DDXVADevice9;

public:
	m_IDirect3DDXVADevice9(m_IDirect3DDXVADevice9* pVADevice9, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pVADevice9), m_pDeviceEx(pDevice)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface(pDevice, WrapperID, nullptr);

		m_pDeviceEx->GetLookupTable()->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DDXVADevice9()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DDXVADevice9 methods ***/
	STDMETHOD(BeginFrame)(THIS_ IDirect3DSurface9* pDstSurface, DWORD SizeInputData, VOID* pInputData, DWORD* pSizeOutputData, VOID* pOutputData);
	STDMETHOD(EndFrame)(THIS_ DWORD SizeMiscData, VOID* pMiscData);
	STDMETHOD(Execute)(THIS_ DWORD FunctionNum, VOID* pInputData, DWORD InputSize, VOID* OuputData, DWORD OutputSize, DWORD NumBuffers, DXVABufferInfo* pBufferInfo);
	STDMETHOD(QueryStatus)(THIS_ IDirect3DSurface9* pSurface, DWORD Flags);

	// Helper functions
	m_IDirect3DDXVADevice9* GetProxyInterface() const { return ProxyInterface; }
	void InitInterface(m_IDirect3DDevice9Ex* Device, REFIID, void*) { m_pDeviceEx = Device; }
};
