#pragma once

class m_IDirectDrawX : public IDirectDraw7
{
private:
	IDirectDraw7 *ProxyInterface;
	m_IDirectDraw7 *WrapperInterface;
	DWORD DirectXVersion;
	DWORD ProxyDirectXVersion;
	IID WrapperID;
	ULONG RefCount = 1;
	bool ConvertSurfaceDescTo2 = false;

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

	// Direct3D9 Objects
	LPDIRECT3D9 d3d9Object = nullptr;
	LPDIRECT3DDEVICE9 d3d9Device = nullptr;
	D3DPRESENT_PARAMETERS presParams;

	// Store a list of surfaces
	std::vector<m_IDirectDrawSurfaceX*> SurfaceVector;

public:
	m_IDirectDrawX(IDirectDraw7 *aOriginal, DWORD Version, m_IDirectDraw7 *Interface) : ProxyInterface(aOriginal), DirectXVersion(Version), WrapperInterface(Interface)
	{
		WrapperID = (DirectXVersion == 1) ? IID_IDirectDraw :
			(DirectXVersion == 2) ? IID_IDirectDraw2 :
			(DirectXVersion == 3) ? IID_IDirectDraw3 :
			(DirectXVersion == 4) ? IID_IDirectDraw4 :
			(DirectXVersion == 7) ? IID_IDirectDraw7 : IID_IDirectDraw7;

		if (Config.Dd7to9)
		{
			d3d9Object = (LPDIRECT3D9)ProxyInterface;

			ProxyDirectXVersion = 9;

			if (!displayWidth || !displayHeight)
			{
				SetDefaultDisplayMode = true;
			}
		}
		else
		{
			ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(WrapperID));
		}

		InterlockedExchangePointer((PVOID*)&CurrentDDInterface, this);

		if (ProxyDirectXVersion > 3 && DirectXVersion < 4)
		{
			ConvertSurfaceDescTo2 = true;
		}

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert DirectDraw v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
		else
		{
			Logging::LogDebug() << "Create " << __FUNCTION__ << " v" << DirectXVersion;
		}
	}
	~m_IDirectDrawX()
	{
		PVOID MyNull = nullptr;
		InterlockedExchangePointer((PVOID*)&CurrentDDInterface, MyNull);

		if (g_hook)
		{
			UnhookWindowsHookEx(g_hook);
		}
	}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirectDraw *GetProxyInterfaceV1() { return (IDirectDraw *)ProxyInterface; }
	IDirectDraw3 *GetProxyInterfaceV3() { return (IDirectDraw3 *)ProxyInterface; }
	IDirectDraw4 *GetProxyInterfaceV4() { return (IDirectDraw4 *)ProxyInterface; }
	IDirectDraw7 *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDraw7 *GetWrapperInterface() { return WrapperInterface; }
	LPDIRECT3D9 GetDirect3DObject() { return d3d9Object; }
	LPDIRECT3DDEVICE9 *GetDirect3DDevice() { return &d3d9Device; }
	HWND GetHwnd() { return MainhWnd; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IDirectDraw methods ***/
	STDMETHOD(Compact)(THIS);
	STDMETHOD(CreateClipper)(THIS_ DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR *);
	STDMETHOD(CreatePalette)(THIS_ DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR*, IUnknown FAR *);
	STDMETHOD(CreateSurface)(THIS_ LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPDIRECTDRAWSURFACE7 FAR * lplpDDSurface, IUnknown FAR * pUnkOuter)
	{
		if (DirectXVersion < 4)
		{
			return CreateSurface((LPDDSURFACEDESC)lpDDSurfaceDesc2, lplpDDSurface, pUnkOuter);
		}

		return CreateSurface2(lpDDSurfaceDesc2, lplpDDSurface, pUnkOuter);
	}
	HRESULT CreateSurface(LPDDSURFACEDESC, LPDIRECTDRAWSURFACE7 FAR *, IUnknown FAR *);
	HRESULT CreateSurface2(LPDDSURFACEDESC2, LPDIRECTDRAWSURFACE7 FAR *, IUnknown FAR *);
	STDMETHOD(DuplicateSurface)(THIS_ LPDIRECTDRAWSURFACE7, LPDIRECTDRAWSURFACE7 FAR *);
	STDMETHOD(EnumDisplayModes)(THIS_ DWORD dwFlags, LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext, LPDDENUMMODESCALLBACK2 lpEnumModesCallback2)
	{
		if (DirectXVersion < 4)
		{
			return EnumDisplayModes(dwFlags, (LPDDSURFACEDESC)lpDDSurfaceDesc2, lpContext, (LPDDENUMMODESCALLBACK)lpEnumModesCallback2);
		}

		return EnumDisplayModes2(dwFlags, lpDDSurfaceDesc2, lpContext, lpEnumModesCallback2);
	}
	HRESULT EnumDisplayModes(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK);
	HRESULT EnumDisplayModes2(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2);
	STDMETHOD(EnumSurfaces)(THIS_ DWORD dwFlags, LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback7)
	{
		if (DirectXVersion < 4)
		{
			return EnumSurfaces(dwFlags, (LPDDSURFACEDESC)lpDDSurfaceDesc2, lpContext, (LPDDENUMSURFACESCALLBACK)lpEnumSurfacesCallback7);
		}

		return EnumSurfaces2(dwFlags, lpDDSurfaceDesc2, lpContext, lpEnumSurfacesCallback7);
	}
	HRESULT EnumSurfaces(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMSURFACESCALLBACK);
	HRESULT EnumSurfaces2(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMSURFACESCALLBACK7);
	STDMETHOD(FlipToGDISurface)(THIS);
	STDMETHOD(GetCaps)(THIS_ LPDDCAPS, LPDDCAPS);
	STDMETHOD(GetDisplayMode)(THIS_ LPDDSURFACEDESC2 lpDDSurfaceDesc2)
	{
		if (DirectXVersion < 4)
		{
			return GetDisplayMode((LPDDSURFACEDESC)lpDDSurfaceDesc2);
		}

		return GetDisplayMode2(lpDDSurfaceDesc2);
	}
	HRESULT GetDisplayMode(LPDDSURFACEDESC);
	HRESULT GetDisplayMode2(LPDDSURFACEDESC2);
	STDMETHOD(GetFourCCCodes)(THIS_ LPDWORD, LPDWORD);
	STDMETHOD(GetGDISurface)(THIS_ LPDIRECTDRAWSURFACE7 FAR *);
	STDMETHOD(GetMonitorFrequency)(THIS_ LPDWORD);
	STDMETHOD(GetScanLine)(THIS_ LPDWORD);
	STDMETHOD(GetVerticalBlankStatus)(THIS_ LPBOOL);
	STDMETHOD(Initialize)(THIS_ GUID FAR *);
	STDMETHOD(RestoreDisplayMode)(THIS);
	STDMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD);
	STDMETHOD(SetDisplayMode)(THIS_ DWORD, DWORD, DWORD, DWORD, DWORD);
	STDMETHOD(WaitForVerticalBlank)(THIS_ DWORD, HANDLE);

	/*** Added in the v2 interface ***/
	STDMETHOD(GetAvailableVidMem)(THIS_ LPDDSCAPS2 lpDDSCaps2, LPDWORD lpdwTotal, LPDWORD lpdwFree)
	{
		if (DirectXVersion < 4)
		{
			return GetAvailableVidMem((LPDDSCAPS)lpDDSCaps2, lpdwTotal, lpdwFree);
		}

		return GetAvailableVidMem2(lpDDSCaps2, lpdwTotal, lpdwFree);
	}
	HRESULT GetAvailableVidMem(LPDDSCAPS, LPDWORD, LPDWORD);
	HRESULT GetAvailableVidMem2(LPDDSCAPS2, LPDWORD, LPDWORD);

	/*** Added in the V4 Interface ***/
	STDMETHOD(GetSurfaceFromDC) (THIS_ HDC, LPDIRECTDRAWSURFACE7 *);
	STDMETHOD(RestoreAllSurfaces)(THIS);
	STDMETHOD(TestCooperativeLevel)(THIS);
	STDMETHOD(GetDeviceIdentifier)(THIS_ LPDDDEVICEIDENTIFIER2 lpdddi2, DWORD dwFlags)
	{
		if (DirectXVersion < 7)
		{
			return GetDeviceIdentifier((LPDDDEVICEIDENTIFIER)lpdddi2, dwFlags);
		}

		return GetDeviceIdentifier2(lpdddi2, dwFlags);
	}
	HRESULT GetDeviceIdentifier(LPDDDEVICEIDENTIFIER, DWORD);
	HRESULT GetDeviceIdentifier2(LPDDDEVICEIDENTIFIER2, DWORD);
	STDMETHOD(StartModeTest)(THIS_ LPSIZE, DWORD, DWORD);
	STDMETHOD(EvaluateMode)(THIS_ DWORD, DWORD *);

	/*** Helper functions ***/
	HRESULT CreateD3DDevice();
	HRESULT ReinitDevice();
	void ReleaseAllD9Surfaces();
	void ReleaseD3d9Device();
	void ReleaseD3d9();
	void AddSurfaceToVector(m_IDirectDrawSurfaceX* lpSurfaceX);
	void RemoveSurfaceFromVector(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool DoesSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX);
	HRESULT BeginScene();
	HRESULT EndScene();
 };
