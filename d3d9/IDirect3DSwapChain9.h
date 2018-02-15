#pragma once

class m_IDirect3DSwapChain9 : public IDirect3DSwapChain9, public AddressLookupTableObject
{
private:
	LPDIRECT3DSWAPCHAIN9 ProxyInterface;
	m_IDirect3DDevice9* m_pDevice = nullptr;
	m_IDirect3DDevice9Ex* m_pDeviceEx = nullptr;

public:
	m_IDirect3DSwapChain9(LPDIRECT3DSWAPCHAIN9 pSwapChain9, m_IDirect3DDevice9* pDevice) : ProxyInterface(pSwapChain9), m_pDevice(pDevice)
	{
		pDevice->ProxyAddressLookupTable->SaveAddress(this, ProxyInterface);
	}
	m_IDirect3DSwapChain9(LPDIRECT3DSWAPCHAIN9 pSwapChain9, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pSwapChain9), m_pDeviceEx(pDevice)
	{
		pDevice->ProxyAddressLookupTable->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DSwapChain9() {}

	LPDIRECT3DSWAPCHAIN9 GetProxyInterface() { return ProxyInterface; }

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
};
