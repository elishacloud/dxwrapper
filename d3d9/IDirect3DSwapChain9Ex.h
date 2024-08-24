#pragma once

class m_IDirect3DSwapChain9Ex : public IDirect3DSwapChain9Ex, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DSWAPCHAIN9 ProxyInterface;
	LPDIRECT3DSWAPCHAIN9EX ProxyInterfaceEx = nullptr;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	REFIID WrapperID;

public:
	m_IDirect3DSwapChain9Ex(LPDIRECT3DSWAPCHAIN9EX pSwapChain9, m_IDirect3DDevice9Ex* pDevice, REFIID DeviceID) : ProxyInterface(pSwapChain9), m_pDeviceEx(pDevice), WrapperID(DeviceID)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ") " << WrapperID);

		if (WrapperID == IID_IDirect3DSwapChain9Ex)
		{
			ProxyInterfaceEx = pSwapChain9;
		}

		ProxyAddressLookupTable9.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DSwapChain9Ex()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DSwapChain9 methods ***/
	STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
	STDMETHOD(GetFrontBufferData)(THIS_ IDirect3DSurface9* pDestSurface);
	STDMETHOD(GetBackBuffer)(THIS_ UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer);
	STDMETHOD(GetRasterStatus)(THIS_ D3DRASTER_STATUS* pRasterStatus);
	STDMETHOD(GetDisplayMode)(THIS_ D3DDISPLAYMODE* pMode);
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	STDMETHOD(GetPresentParameters)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);

	/*** IDirect3DSwapChain9Ex methods ***/
	STDMETHOD(GetLastPresentCount)(THIS_ UINT* pLastPresentCount);
	STDMETHOD(GetPresentStats)(THIS_ D3DPRESENTSTATS* pPresentationStatistics);
	STDMETHOD(GetDisplayModeEx)(THIS_ D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation);

	// Helper functions
	LPDIRECT3DSWAPCHAIN9 GetProxyInterface() { return ProxyInterface; }
};
