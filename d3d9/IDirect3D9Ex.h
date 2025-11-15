#pragma once

class m_IDirect3D9Ex : public IDirect3D9Ex, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3D9 ProxyInterface;
	LPDIRECT3D9EX ProxyInterfaceEx = nullptr;
	DWORD ClientDirectXVersion = 9;
	const IID WrapperID;

	struct ENUM_ADAPTERS_CACHE {
		UINT Adapter = 0;
		bool IsEx = false;
		D3DDISPLAYMODEFILTER Filter = {};
		std::vector<D3DDISPLAYMODEEX_CONVERT> DisplayModeList;
	};

	// Adapter Modes cache
	std::vector<ENUM_ADAPTERS_CACHE> AdapterModesCache;
	UINT GetAdapterModeCache(THIS_ UINT Adapter, D3DFORMAT Format, bool IsEx, CONST D3DDISPLAYMODEFILTER* pFilter);

	// For Create & CreateEx
	template <typename T>
	HRESULT CreateDeviceT(DEVICEDETAILS& DeviceDetails, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, bool IsEx, D3DDISPLAYMODEEX* pFullscreenDisplayMode, T ppReturnedDeviceInterface);
	inline HRESULT CreateDeviceT(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX*, IDirect3DDevice9** ppReturnedDeviceInterface)
	{ return ProxyInterface->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface); }
	inline HRESULT CreateDeviceT(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface)
	{ return (ProxyInterfaceEx) ? ProxyInterfaceEx->CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, ppReturnedDeviceInterface) : D3DERR_INVALIDCALL; }

	// Other helper functions
	void LogAdapterNames();
	void InitInterface();
	void ReleaseInterface();

public:
	m_IDirect3D9Ex(LPDIRECT3D9EX pDirect3D, REFIID DeviceID) : ProxyInterface(pDirect3D), WrapperID(DeviceID)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ") " << WrapperID);

		if (WrapperID == IID_IDirect3D9Ex)
		{
			ProxyInterfaceEx = pDirect3D;
		}

		LogAdapterNames();

		InitInterface();
	}
	~m_IDirect3D9Ex()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseInterface();
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3D9 methods ***/
	STDMETHOD(RegisterSoftwareDevice)(THIS_ void* pInitializeFunction);
	STDMETHOD_(UINT, GetAdapterCount)(THIS);
	STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier);
	STDMETHOD_(UINT, GetAdapterModeCount)(THIS_ UINT Adapter, D3DFORMAT Format);
	STDMETHOD(EnumAdapterModes)(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode);
	STDMETHOD(GetAdapterDisplayMode)(THIS_ UINT Adapter, D3DDISPLAYMODE* pMode);
	STDMETHOD(CheckDeviceType)(THIS_ UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed);
	STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
	STDMETHOD(CheckDeviceMultiSampleType)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels);
	STDMETHOD(CheckDepthStencilMatch)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
	STDMETHOD(CheckDeviceFormatConversion)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat);
	STDMETHOD(GetDeviceCaps)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps);
	STDMETHOD_(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter);
	STDMETHOD(CreateDevice)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface);

	/*** IDirect3D9Ex methods ***/
	STDMETHOD_(UINT, GetAdapterModeCountEx)(THIS_ UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter);
	STDMETHOD(EnumAdapterModesEx)(THIS_ UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode);
	STDMETHOD(GetAdapterDisplayModeEx)(THIS_ UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation);
	STDMETHOD(CreateDeviceEx)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface);
	STDMETHOD(GetAdapterLUID)(THIS_ UINT Adapter, LUID * pLUID);

	// Helper functions
	void InitInterface(void*, REFIID, void*) {}	// Stub only
	void SetDirectXVersion(DWORD DxVersion);
	bool TestResolution(UINT Adapter, DWORD BackBufferWidth, DWORD BackBufferHeight);

	// Static functions
	static DWORD AdjustPOW2Caps(DWORD OriginalCaps);
	static DWORD UpdateBehaviorFlags(DWORD BehaviorFlags);
	static void UpdatePresentParameter(D3DPRESENT_PARAMETERS* pPresentationParameters, HWND hFocusWindow, DEVICEDETAILS& DeviceDetails, bool IsEx, bool ForceExclusiveFullscreen, bool SetWindow);
	static void GetFinalPresentParameter(D3DPRESENT_PARAMETERS* pPresentationParameters, DEVICEDETAILS& DeviceDetails);
	static void GetFullscreenDisplayMode(D3DPRESENT_PARAMETERS& d3dpp, D3DDISPLAYMODEEX& Mode);
	static void UpdatePresentParameterForMultisample(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD MultiSampleQuality);
	static void AdjustWindow(HMONITOR hMonitor, HWND MainhWnd, LONG displayWidth, LONG displayHeight, bool EnableWindowMode, bool FullscreenWindowMode);
};
