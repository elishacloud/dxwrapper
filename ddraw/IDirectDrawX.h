#pragma once

class m_IDirectDrawX : public IUnknown, public AddressLookupTableDdrawObject
{
private:
	IDirectDraw7 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount = 1;

	// Fix exclusive mode issue
	HHOOK g_hook = nullptr;
	HWND chWnd = nullptr;

	// Store ddraw version wrappers
	m_IDirectDraw *WrapperInterface;
	m_IDirectDraw2 *WrapperInterface2;
	m_IDirectDraw3 *WrapperInterface3;
	m_IDirectDraw4 *WrapperInterface4;
	m_IDirectDraw7 *WrapperInterface7;

	// Store primary surface
	m_IDirectDrawSurfaceX *PrimarySurface = nullptr;

	// Store a list of surfaces
	std::vector<m_IDirectDrawSurfaceX*> SurfaceVector;

	// Store a list of palettes
	std::vector<m_IDirectDrawPalette*> PaletteVector;

	// Store color control interface
	m_IDirectDrawColorControl *ColorControlInterface = nullptr;

	// Store gamma control interface
	m_IDirectDrawGammaControl *GammaControlInterface = nullptr;

	// Store d3d interface
	m_IDirect3DX *D3DInterface = nullptr;
	m_IDirect3DDeviceX *D3DDeviceInterface = nullptr;

	// Wrapper interface functions
	REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirectDraw :
			(DirectXVersion == 2) ? IID_IDirectDraw2 :
			(DirectXVersion == 3) ? IID_IDirectDraw3 :
			(DirectXVersion == 4) ? IID_IDirectDraw4 :
			(DirectXVersion == 7) ? IID_IDirectDraw7 : IID_IUnknown;
	}
	bool CheckWrapperType(REFIID IID)
	{
		return (IID == IID_IDirectDraw ||
			IID == IID_IDirectDraw2 ||
			IID == IID_IDirectDraw3 ||
			IID == IID_IDirectDraw4 ||
			IID == IID_IDirectDraw7) ? true : false;
	}
	IDirectDraw *GetProxyInterfaceV1() { return (IDirectDraw *)ProxyInterface; }
	IDirectDraw2 *GetProxyInterfaceV2() { return (IDirectDraw2 *)ProxyInterface; }
	IDirectDraw3 *GetProxyInterfaceV3() { return (IDirectDraw3 *)ProxyInterface; }
	IDirectDraw4 *GetProxyInterfaceV4() { return (IDirectDraw4 *)ProxyInterface; }
	IDirectDraw7 *GetProxyInterfaceV7() { return ProxyInterface; }

	// Device information functions
	void InitDdrawSettings();
	void ReleaseDdraw();

	// Direct3D9 interface functions
	HRESULT CheckInterface(char *FunctionName, bool CheckD3DDevice);
	void ReleaseAllDirectDrawD9Surfaces();
	void ReleaseAllD9Surfaces();
	void ReleaseD3d9Device();

public:
	m_IDirectDrawX(IDirectDraw7 *aOriginal, DWORD DirectXVersion) : ProxyInterface(aOriginal)
	{
		ProxyDirectXVersion = GetGUIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		if (ProxyDirectXVersion != DirectXVersion)
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);
		}
		else
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ") v" << DirectXVersion);
		}

		WrapperInterface = new m_IDirectDraw((LPDIRECTDRAW)ProxyInterface, this);
		WrapperInterface2 = new m_IDirectDraw2((LPDIRECTDRAW2)ProxyInterface, this);
		WrapperInterface3 = new m_IDirectDraw3((LPDIRECTDRAW3)ProxyInterface, this);
		WrapperInterface4 = new m_IDirectDraw4((LPDIRECTDRAW4)ProxyInterface, this);
		WrapperInterface7 = new m_IDirectDraw7((LPDIRECTDRAW7)ProxyInterface, this);

		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	m_IDirectDrawX(DWORD DirectXVersion)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		WrapperInterface = new m_IDirectDraw((LPDIRECTDRAW)ProxyInterface, this);
		WrapperInterface2 = new m_IDirectDraw2((LPDIRECTDRAW2)ProxyInterface, this);
		WrapperInterface3 = new m_IDirectDraw3((LPDIRECTDRAW3)ProxyInterface, this);
		WrapperInterface4 = new m_IDirectDraw4((LPDIRECTDRAW4)ProxyInterface, this);
		WrapperInterface7 = new m_IDirectDraw7((LPDIRECTDRAW7)ProxyInterface, this);

		InitDdrawSettings();

		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectDrawX()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting interface!");

		WrapperInterface->DeleteMe();
		WrapperInterface2->DeleteMe();
		WrapperInterface3->DeleteMe();
		WrapperInterface4->DeleteMe();
		WrapperInterface7->DeleteMe();

		if (g_hook)
		{
			UnhookWindowsHookEx(g_hook);
		}

		if (Config.Dd7to9 && !Config.Exiting)
		{
			ReleaseDdraw();
		}

		ProxyAddressLookupTable.DeleteAddress(this);
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, GetGUIDVersion(riid)); }
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

	/*** Added in the V7 Interface ***/
	STDMETHOD(StartModeTest)(THIS_ LPSIZE, DWORD, DWORD);
	STDMETHOD(EvaluateMode)(THIS_ DWORD, DWORD *);

	// Helper functions
	void *GetWrapperInterfaceX(DWORD DirectXVersion);

	// Direct3D interfaces
	m_IDirect3DX **GetCurrentD3D() { return &D3DInterface; }
	void ClearD3D() { D3DInterface = nullptr; }
	void SetD3DDevice(m_IDirect3DDeviceX *D3DDevice) { D3DDeviceInterface = D3DDevice; }
	m_IDirect3DDeviceX **GetCurrentD3DDevice() { return &D3DDeviceInterface; }
	void ClearD3DDevice() { D3DDeviceInterface = nullptr; }

	// Direct3D9 interfaces
	LPDIRECT3D9 GetDirect3D9Object();
	LPDIRECT3DDEVICE9 *GetDirect3D9Device();
	HRESULT CreateD3D9Device();
	HRESULT ReinitDevice();

	// Device information functions
	HWND GetHwnd();
	HDC GetDC();
	bool IsExclusiveMode();
	void GetResolution(DWORD &Width, DWORD &Height, DWORD &RefreshRate, DWORD &BPP);
	void GetDisplay(DWORD &Width, DWORD &Height);

	// Surface vector functions
	void AddSurfaceToVector(m_IDirectDrawSurfaceX* lpSurfaceX);
	void RemoveSurfaceFromVector(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool DoesSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX);
	m_IDirectDrawSurfaceX *GetPrimarySurface() { return PrimarySurface; }
	void EvictManagedTextures();

	// Palette vector functions
	void AddPaletteToVector(m_IDirectDrawPalette* lpPalette);
	void RemovePaletteFromVector(m_IDirectDrawPalette* lpPalette);
	bool DoesPaletteExist(m_IDirectDrawPalette* lpPalette);

	// Color and gamma control
	HRESULT CreateColorInterface(LPVOID *ppvObj);
	HRESULT CreateGammaInterface(LPVOID *ppvObj);
	void ClearColorInterface() { ColorControlInterface = nullptr;  };
	void ClearGammaInterface() { GammaControlInterface = nullptr; };

	// Video memory size
	static void AdjustVidMemory(LPDWORD lpdwTotal, LPDWORD lpdwFree);

	// Begin & end scene
	void SetVsync();
	HRESULT BeginScene();
	HRESULT EndScene();
};
