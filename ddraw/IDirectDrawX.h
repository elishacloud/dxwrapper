#pragma once

class m_IDirectDrawX : public IDirectDraw7
{
private:
	IDirectDraw7 *ProxyInterface;
	m_IDirectDraw7 *WrapperInterface;
	DWORD DirectXVersion;
	DWORD ProxyDirectXVersion;
	IID WrapperID;
	bool ConvertSurfaceDescTo2 = false;
	
	// Fix exclusive mode issue
	HHOOK g_hook = nullptr;
	HWND chWnd = nullptr;

	// Convert to d3d9
	ULONG RefCount = 1;
	HWND MainhWnd = nullptr;
	bool ExclusiveMode = false;
	POINT lastPosition = {100, 100};		// Last window position
	bool SetDefaultDisplayMode = false;		// Set native resolution
	bool isWindowed = false;				// Window mode enabled

	// Application display mode
	DWORD displayModeWidth = 0;
	DWORD displayModeHeight = 0;
	DWORD displayModeBPP = 0;
	DWORD displayModerefreshRate = 0;		// Refresh rate for fullscreen

	// Display resolution
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

			if ((!displayWidth || !displayHeight) && !isWindowed)
			{
				SetDefaultDisplayMode = true;
			}
		}
		else
		{
			REFIID ProxyID = ConvertREFIID(WrapperID);
			ProxyDirectXVersion = (ProxyID == IID_IDirectDraw) ? 1 :
				(ProxyID == IID_IDirectDraw2) ? 2 :
				(ProxyID == IID_IDirectDraw3) ? 3 :
				(ProxyID == IID_IDirectDraw4) ? 4 :
				(ProxyID == IID_IDirectDraw7) ? 7 : 7;
		}

		InterlockedExchangePointer((PVOID*)&CurrentDDInterface, ProxyInterface);

		if (ProxyDirectXVersion > 3 && DirectXVersion < 4)
		{
			ConvertSurfaceDescTo2 = true;
		}

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert DirectDraw v" << DirectXVersion << " to v" << ProxyDirectXVersion;
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
	IDirectDraw7 *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDraw7 *GetWrapperInterface() { return WrapperInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);
	/*** IDirectDraw methods ***/
	STDMETHOD(Compact)(THIS);
	STDMETHOD(CreateClipper)(THIS_ DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR *);
	STDMETHOD(CreatePalette)(THIS_ DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR*, IUnknown FAR *);
	STDMETHOD(CreateSurface)(THIS_  LPDDSURFACEDESC2, LPDIRECTDRAWSURFACE7 FAR *, IUnknown FAR *);
	STDMETHOD(DuplicateSurface)(THIS_ LPDIRECTDRAWSURFACE7, LPDIRECTDRAWSURFACE7 FAR *);
	STDMETHOD(EnumDisplayModes)(THIS_ DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2);
	STDMETHOD(EnumSurfaces)(THIS_ DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMSURFACESCALLBACK7);
	STDMETHOD(FlipToGDISurface)(THIS);
	STDMETHOD(GetCaps)(THIS_ LPDDCAPS, LPDDCAPS);
	STDMETHOD(GetDisplayMode)(THIS_ LPDDSURFACEDESC2);
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
	STDMETHOD(GetAvailableVidMem)(THIS_ LPDDSCAPS2, LPDWORD, LPDWORD);
	/*** Added in the V4 Interface ***/
	STDMETHOD(GetSurfaceFromDC) (THIS_ HDC, LPDIRECTDRAWSURFACE7 *);
	STDMETHOD(RestoreAllSurfaces)(THIS);
	STDMETHOD(TestCooperativeLevel)(THIS);
	STDMETHOD(GetDeviceIdentifier)(THIS_ LPDDDEVICEIDENTIFIER2, DWORD);
	STDMETHOD(StartModeTest)(THIS_ LPSIZE, DWORD, DWORD);
	STDMETHOD(EvaluateMode)(THIS_ DWORD, DWORD *);
	// Helper functions
	void ReleaseD3d9();
	void AdjustWindow();
	bool CreateD3DDevice();
	bool ReinitDevice();
	void ReleaseAllD9Surfaces();
	void RemoveSurfaceFromVector(m_IDirectDrawSurfaceX* lpSurfaceX);
	HRESULT CheckBeginScene(m_IDirectDrawSurfaceX *pSurface);
	HRESULT CheckEndScene(m_IDirectDrawSurfaceX *pSurface);
 };
