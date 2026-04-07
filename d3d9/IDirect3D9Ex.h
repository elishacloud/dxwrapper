#pragma once

class m_IDirect3D9Ex final : public IDirect3D9Ex, public AddressLookupTableD3d9Object
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

	// Information
	inline bool IsForcingD3d9to9Ex() const { return (Config.D3d9to9Ex && ProxyInterface == ProxyInterfaceEx); }

	// Other helper functions
	void InitInterface();
	void ReleaseInterface();
	void LogAdapterNames();

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
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3D9 methods ***/
	IFACEMETHOD(RegisterSoftwareDevice)(THIS_ void* pInitializeFunction) override;
	IFACEMETHOD_(UINT, GetAdapterCount)(THIS) override;
	IFACEMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) override;
	IFACEMETHOD_(UINT, GetAdapterModeCount)(THIS_ UINT Adapter, D3DFORMAT Format) override;
	IFACEMETHOD(EnumAdapterModes)(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) override;
	IFACEMETHOD(GetAdapterDisplayMode)(THIS_ UINT Adapter, D3DDISPLAYMODE* pMode) override;
	IFACEMETHOD(CheckDeviceType)(THIS_ UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed) override;
	IFACEMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) override;
	IFACEMETHOD(CheckDeviceMultiSampleType)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) override;
	IFACEMETHOD(CheckDepthStencilMatch)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) override;
	IFACEMETHOD(CheckDeviceFormatConversion)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) override;
	IFACEMETHOD(GetDeviceCaps)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) override;
	IFACEMETHOD_(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter) override;
	IFACEMETHOD(CreateDevice)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) override;

	/*** IDirect3D9Ex methods ***/
	IFACEMETHOD_(UINT, GetAdapterModeCountEx)(THIS_ UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter) override;
	IFACEMETHOD(EnumAdapterModesEx)(THIS_ UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode) override;
	IFACEMETHOD(GetAdapterDisplayModeEx)(THIS_ UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) override;
	IFACEMETHOD(CreateDeviceEx)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface) override;
	IFACEMETHOD(GetAdapterLUID)(THIS_ UINT Adapter, LUID * pLUID) override;

	// Helper functions
	void InitInterface(void*, REFIID, void*) {}	// Stub only
	void SetDirectXVersion(DWORD DxVersion);
	bool TestResolution(UINT Adapter, DWORD BackBufferWidth, DWORD BackBufferHeight);

	// Static functions
	static DWORD AdjustPOW2Caps(DWORD OriginalCaps);
	static DWORD UpdateBehaviorFlags(DWORD BehaviorFlags);
	static void UpdatePresentParameter(D3DPRESENT_PARAMETERS* pPresentationParameters, HWND hFocusWindow, DEVICEDETAILS& DeviceDetails, bool IsEx, bool ForceExclusiveFullscreen, bool SetWindow);
	static void GetFinalPresentParameter(D3DPRESENT_PARAMETERS* pPresentationParameters, DEVICEDETAILS& DeviceDetails);
	static void UpdatePresentParameterForMultisample(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD MultiSampleQuality);
	static void GetFullscreenDisplayMode(D3DPRESENT_PARAMETERS& d3dpp, D3DDISPLAYMODEEX& Mode);
	static void AdjustWindowStyle(HWND hWnd);
	static void AdjustWindowSize(HMONITOR hMonitor, HWND hWnd, LONG displayWidth, LONG displayHeight, bool EnableWindowMode, bool FullscreenWindowMode);
};
