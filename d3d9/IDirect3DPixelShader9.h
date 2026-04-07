#pragma once

class m_IDirect3DPixelShader9 final : public IDirect3DPixelShader9, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DPIXELSHADER9 ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	const IID WrapperID = IID_IDirect3DPixelShader9;

public:
	m_IDirect3DPixelShader9(LPDIRECT3DPIXELSHADER9 pShader9, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pShader9), m_pDeviceEx(pDevice)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface(pDevice, WrapperID, nullptr);

		m_pDeviceEx->GetLookupTable()->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DPixelShader9()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DPixelShader9 methods ***/
	IFACEMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
	IFACEMETHOD(GetFunction)(THIS_ void* pData, UINT* pSizeOfData) override;

	// Helper functions
	LPDIRECT3DPIXELSHADER9 GetProxyInterface() const { return ProxyInterface; }
	void InitInterface(m_IDirect3DDevice9Ex* Device, REFIID, void*) { m_pDeviceEx = Device; }
};
