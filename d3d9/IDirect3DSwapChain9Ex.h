#pragma once

class m_IDirect3DSwapChain9Ex final : public IDirect3DSwapChain9Ex, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DSWAPCHAIN9 ProxyInterface;
	LPDIRECT3DSWAPCHAIN9EX ProxyInterfaceEx;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	IID WrapperID;

	// Information
	inline bool IsForcingD3d9to9Ex() const { return (Config.D3d9to9Ex && ProxyInterface == ProxyInterfaceEx); }

public:
	m_IDirect3DSwapChain9Ex(LPDIRECT3DSWAPCHAIN9EX pSwapChain9, m_IDirect3DDevice9Ex* pDevice, REFIID DeviceID) : ProxyInterface(pSwapChain9), m_pDeviceEx(pDevice), WrapperID(DeviceID)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ") " << WrapperID);

		InitInterface(pDevice, WrapperID, nullptr);

		m_pDeviceEx->GetLookupTable()->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DSwapChain9Ex()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DSwapChain9 methods ***/
	IFACEMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags) override;
	IFACEMETHOD(GetFrontBufferData)(THIS_ IDirect3DSurface9* pDestSurface) override;
	IFACEMETHOD(GetBackBuffer)(THIS_ UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) override;
	IFACEMETHOD(GetRasterStatus)(THIS_ D3DRASTER_STATUS* pRasterStatus) override;
	IFACEMETHOD(GetDisplayMode)(THIS_ D3DDISPLAYMODE* pMode) override;
	IFACEMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
	IFACEMETHOD(GetPresentParameters)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters) override;

	/*** IDirect3DSwapChain9Ex methods ***/
	IFACEMETHOD(GetLastPresentCount)(THIS_ UINT* pLastPresentCount) override;
	IFACEMETHOD(GetPresentStats)(THIS_ D3DPRESENTSTATS* pPresentationStatistics) override;
	IFACEMETHOD(GetDisplayModeEx)(THIS_ D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) override;

	// Helper functions
	LPDIRECT3DSWAPCHAIN9 GetProxyInterface() const { return ProxyInterface; }
	void InitInterface(m_IDirect3DDevice9Ex* Device, REFIID riid, void*) {
		m_pDeviceEx = Device;
		WrapperID == riid;
		if (riid == IID_IDirect3DSwapChain9Ex || ProxyInterface == ProxyInterfaceEx)
		{
			ProxyInterfaceEx = reinterpret_cast<LPDIRECT3DSWAPCHAIN9EX>(ProxyInterface);
		}
		else
		{
			ProxyInterfaceEx = nullptr;
		}
		if (Config.D3d9to9Ex && !IsForcingD3d9to9Ex())
		{
			LOG_LIMIT(3, __FUNCTION__ << " Warning: Creating non-Ex interface when using D3d9to9Ex!");
		}
	}
};
