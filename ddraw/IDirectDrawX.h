#pragma once

class m_IDirectDrawX : public IUnknown
{
private:
	IDirectDraw7 *ProxyInterface = nullptr;
	m_IDirectDraw7 *WrapperInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount = 1;

	// Fix exclusive mode issue
	HHOOK g_hook = nullptr;
	HWND chWnd = nullptr;

	// Convert to d3d9
	HWND MainhWnd = nullptr;
	bool IsInScene = false;
	bool ExclusiveMode = false;
	bool isWindowed = false;				// Window mode enabled

	// Application display mode
	DWORD displayModeWidth = 0;
	DWORD displayModeHeight = 0;
	DWORD displayModeBPP = 0;
	DWORD displayModeRefreshRate = 0;		// Refresh rate for fullscreen

	// Display resolution
	bool SetDefaultDisplayMode = false;		// Set native resolution
	DWORD displayWidth = 0;
	DWORD displayHeight = 0;

	// High resolution counter
	bool FrequencyFlag = false;
	LARGE_INTEGER clockFrequency, clickTime, lastTime = { 0, 0 };
	DWORD FrameCounter = 0;
	DWORD monitorRefreshRate = 0;

	// Direct3D9 Objects
	LPDIRECT3D9 d3d9Object = nullptr;
	LPDIRECT3DDEVICE9 d3d9Device = nullptr;
	D3DPRESENT_PARAMETERS presParams;

	// Store ddraw version wrappers
	std::unique_ptr<m_IDirectDraw> UniqueProxyInterface = nullptr;
	std::unique_ptr<m_IDirectDraw2> UniqueProxyInterface2 = nullptr;
	std::unique_ptr<m_IDirectDraw3> UniqueProxyInterface3 = nullptr;
	std::unique_ptr<m_IDirectDraw4> UniqueProxyInterface4 = nullptr;
	std::unique_ptr<m_IDirectDraw7> UniqueProxyInterface7 = nullptr;

	// Store a list of surfaces
	std::vector<m_IDirectDrawSurfaceX*> SurfaceVector;

	// Store d3d interface
	m_IDirect3DX *D3DInterface = nullptr;
	m_IDirect3DDeviceX *D3DDeviceInterface = nullptr;

public:
	m_IDirectDrawX(IDirectDraw7 *aOriginal, DWORD DirectXVersion, m_IDirectDraw7 *Interface) : ProxyInterface(aOriginal), WrapperInterface(Interface)
	{
		ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert DirectDraw v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
		else
		{
			Logging::LogDebug() << "Create " << __FUNCTION__ << " v" << DirectXVersion;
		}
	}
	m_IDirectDrawX(IDirect3D9 *aOriginal, DWORD DirectXVersion) : d3d9Object(aOriginal)
	{
		ProxyDirectXVersion = 9;

		if (!displayWidth || !displayHeight)
		{
			SetDefaultDisplayMode = true;
		}

		Logging::LogDebug() << "Convert DirectDraw v" << DirectXVersion << " to v" << ProxyDirectXVersion;
	}
	~m_IDirectDrawX()
	{
		if (g_hook)
		{
			UnhookWindowsHookEx(g_hook);
		}

		if (Config.Dd7to9 && !Config.Exiting)
		{
			if (SurfaceVector.size())
			{
				ReleaseAllD9Surfaces(true);
			}
			ReleaseD3DInterfaces();
			D3DInterface = nullptr;
			D3DDeviceInterface = nullptr;
		}
	}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return IID_IUnknown; }
	IDirectDraw7 *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDraw7 *GetWrapperInterface() { return WrapperInterface; }

	/*** IUnknown methods ***/
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, ProxyDirectXVersion); }
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IDirectDraw methods ***/
	STDMETHOD(Compact)(THIS);
	STDMETHOD(CreateClipper)(THIS_ DWORD, LPDIRECTDRAWCLIPPER FAR *, IUnknown FAR *);
	STDMETHOD(CreatePalette)(THIS_ DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR *, IUnknown FAR *);
	HRESULT CreateSurface(LPDDSURFACEDESC, LPDIRECTDRAWSURFACE7 FAR *, IUnknown FAR *, DWORD);
	HRESULT CreateSurface2(LPDDSURFACEDESC2, LPDIRECTDRAWSURFACE7 FAR *, IUnknown FAR *, DWORD);
	STDMETHOD(DuplicateSurface)(THIS_ LPDIRECTDRAWSURFACE7, LPDIRECTDRAWSURFACE7 FAR *, DWORD);
	HRESULT EnumDisplayModes(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK);
	HRESULT EnumDisplayModes2(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2);
	HRESULT EnumSurfaces(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMSURFACESCALLBACK, DWORD);
	HRESULT EnumSurfaces2(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMSURFACESCALLBACK7, DWORD);
	STDMETHOD(FlipToGDISurface)(THIS);
	STDMETHOD(GetCaps)(THIS_ LPDDCAPS, LPDDCAPS);
	HRESULT GetDisplayMode(LPDDSURFACEDESC);
	HRESULT GetDisplayMode2(LPDDSURFACEDESC2);
	STDMETHOD(GetFourCCCodes)(THIS_ LPDWORD, LPDWORD);
	STDMETHOD(GetGDISurface)(THIS_ LPDIRECTDRAWSURFACE7 FAR *, DWORD);
	STDMETHOD(GetMonitorFrequency)(THIS_ LPDWORD);
	STDMETHOD(GetScanLine)(THIS_ LPDWORD);
	STDMETHOD(GetVerticalBlankStatus)(THIS_ LPBOOL);
	STDMETHOD(Initialize)(THIS_ GUID FAR *);
	STDMETHOD(RestoreDisplayMode)(THIS);
	STDMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD);
	STDMETHOD(SetDisplayMode)(THIS_ DWORD, DWORD, DWORD, DWORD, DWORD);
	STDMETHOD(WaitForVerticalBlank)(THIS_ DWORD, HANDLE);

	/*** Added in the v2 interface ***/
	HRESULT GetAvailableVidMem(LPDDSCAPS, LPDWORD, LPDWORD);
	HRESULT GetAvailableVidMem2(LPDDSCAPS2, LPDWORD, LPDWORD);

	/*** Added in the V4 Interface ***/
	STDMETHOD(GetSurfaceFromDC) (THIS_ HDC, LPDIRECTDRAWSURFACE7 *, DWORD DirectXVersion);
	STDMETHOD(RestoreAllSurfaces)(THIS);
	STDMETHOD(TestCooperativeLevel)(THIS);
	HRESULT GetDeviceIdentifier(LPDDDEVICEIDENTIFIER, DWORD);
	HRESULT GetDeviceIdentifier2(LPDDDEVICEIDENTIFIER2, DWORD);
	STDMETHOD(StartModeTest)(THIS_ LPSIZE, DWORD, DWORD);
	STDMETHOD(EvaluateMode)(THIS_ DWORD, DWORD *);

	/*** Helper functions ***/
	REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirectDraw :
			(DirectXVersion == 2) ? IID_IDirectDraw2 :
			(DirectXVersion == 3) ? IID_IDirectDraw3 :
			(DirectXVersion == 4) ? IID_IDirectDraw4 :
			(DirectXVersion == 7) ? IID_IDirectDraw7 : IID_IUnknown;
	}
	IDirectDraw *GetProxyInterfaceV1() { return (IDirectDraw *)ProxyInterface; }
	IDirectDraw2 *GetProxyInterfaceV2() { return (IDirectDraw2 *)ProxyInterface; }
	IDirectDraw3 *GetProxyInterfaceV3() { return (IDirectDraw3 *)ProxyInterface; }
	IDirectDraw4 *GetProxyInterfaceV4() { return (IDirectDraw4 *)ProxyInterface; }
	IDirectDraw7 *GetProxyInterfaceV7() { return ProxyInterface; }
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
	LPDIRECT3D9 GetDirect3D9Object() { return d3d9Object; }
	LPDIRECT3DDEVICE9 *GetDirect3D9Device() { return &d3d9Device; }
	HWND GetHwnd() { return MainhWnd; }
	bool IsExclusiveMode() { return ExclusiveMode; }
	m_IDirect3DDeviceX **GetCurrentD3DDevice() { return &D3DDeviceInterface; }
	void SetD3DDevice(m_IDirect3DDeviceX *D3DDevice) { D3DDeviceInterface = D3DDevice; }
	void ClearD3DDevice() { D3DDeviceInterface = nullptr; }
	void ClearD3D() { D3DInterface = nullptr; }
	DWORD GetRefreshRate();
	HRESULT CreateD3D9Device();
	HRESULT ReinitDevice();
	void ReleaseD3DInterfaces();
	void ReleaseAllD9Surfaces(bool ClearDDraw = false);
	void ReleaseD3d9Device();
	void ReleaseD3d9();
	void AddSurfaceToVector(m_IDirectDrawSurfaceX* lpSurfaceX);
	void RemoveSurfaceFromVector(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool DoesSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX);
	HRESULT BeginScene();
	HRESULT EndScene();
 };
