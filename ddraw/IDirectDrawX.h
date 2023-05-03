#pragma once

class m_IDirectDrawX : public IUnknown, public AddressLookupTableDdrawObject
{
private:
	IDirectDraw7 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount1 = 0;
	ULONG RefCount2 = 0;
	ULONG RefCount3 = 0;
	ULONG RefCount4 = 0;
	ULONG RefCount7 = 0;

	bool IsInitialize = false;
	bool Using3D = false;

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

	// Store a list of clippers
	std::vector<m_IDirectDrawClipper*> ClipperVector;

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
	inline REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirectDraw :
			(DirectXVersion == 2) ? IID_IDirectDraw2 :
			(DirectXVersion == 3) ? IID_IDirectDraw3 :
			(DirectXVersion == 4) ? IID_IDirectDraw4 :
			(DirectXVersion == 7) ? IID_IDirectDraw7 : IID_IUnknown;
	}
	inline bool CheckWrapperType(REFIID IID)
	{
		return (IID == IID_IDirectDraw ||
			IID == IID_IDirectDraw2 ||
			IID == IID_IDirectDraw3 ||
			IID == IID_IDirectDraw4 ||
			IID == IID_IDirectDraw7) ? true : false;
	}
	inline IDirectDraw *GetProxyInterfaceV1() { return (IDirectDraw *)ProxyInterface; }
	inline IDirectDraw2 *GetProxyInterfaceV2() { return (IDirectDraw2 *)ProxyInterface; }
	inline IDirectDraw3 *GetProxyInterfaceV3() { return (IDirectDraw3 *)ProxyInterface; }
	inline IDirectDraw4 *GetProxyInterfaceV4() { return (IDirectDraw4 *)ProxyInterface; }
	inline IDirectDraw7 *GetProxyInterfaceV7() { return ProxyInterface; }

	// Interface initialization functions
	void InitDdraw(DWORD DirectXVersion);
	void ReleaseDdraw();

	// Direct3D9 interface functions
	HRESULT CheckInterface(char *FunctionName, bool CheckD3DDevice);
	HRESULT CreateD3D9Object();
	void ReleaseAllD9Surfaces(bool BackupData);
	void ReleaseD3D9Device();
	void ReleaseD3D9Object();

public:
	m_IDirectDrawX(IDirectDraw7 *aOriginal, DWORD DirectXVersion) : ProxyInterface(aOriginal)
	{
		ProxyDirectXVersion = GetGUIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		if (ProxyDirectXVersion != DirectXVersion)
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);
		}
		else
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ") v" << DirectXVersion);
		}

		InitDdraw(DirectXVersion);
	}
	m_IDirectDrawX(DWORD DirectXVersion)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		InitDdraw(DirectXVersion);
	}
	~m_IDirectDrawX()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseDdraw();
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, 0); }
	STDMETHOD_(ULONG, AddRef) (THIS) { return AddRef(0); }
	STDMETHOD_(ULONG, Release) (THIS) { return Release(0); }

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
	STDMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD, DWORD);
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
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
	ULONG AddRef(DWORD DirectXVersion);
	ULONG Release(DWORD DirectXVersion);

	// Direct3D interfaces
	m_IDirect3DX **GetCurrentD3D() { return &D3DInterface; }
	void ClearD3D() { D3DInterface = nullptr; }
	void SetD3DDevice(m_IDirect3DDeviceX *D3DDevice);
	m_IDirect3DDeviceX **GetCurrentD3DDevice() { return &D3DDeviceInterface; }
	void ClearD3DDevice() { D3DDeviceInterface = nullptr; Using3D = false; }
	bool IsUsing3D() { return Using3D; }
	void Enable3D() { Using3D = true; }

	// Direct3D9 interfaces
	LPDIRECT3D9 GetDirect3D9Object();
	LPDIRECT3DDEVICE9 *GetDirect3D9Device();
	HRESULT CreateD3D9Device();
	HRESULT ReinitDevice();

	// Device information functions
	HWND GetHwnd();
	HDC GetDC();
	void ClearSencilSurface();
	D3DMULTISAMPLE_TYPE GetMultiSampleType();
	DWORD GetMultiSampleQuality();
	bool IsExclusiveMode();
	void GetFullDisplay(DWORD &Width, DWORD &Height, DWORD& BPP, DWORD &RefreshRate);
	void GetDisplayPixelFormat(DDPIXELFORMAT& ddpfPixelFormat, DWORD BPP);
	void GetDisplay(DWORD &Width, DWORD &Height);

	// Surface vector functions
	void AddSurfaceToVector(m_IDirectDrawSurfaceX* lpSurfaceX);
	void RemoveSurfaceFromVector(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool DoesSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX);
	m_IDirectDrawSurfaceX *GetPrimarySurface() { return PrimarySurface; }
	void EvictManagedTextures();

	// Clipper vector functions
	void AddClipperToVector(m_IDirectDrawClipper* lpClipper);
	void RemoveClipperFromVector(m_IDirectDrawClipper* lpClipper);
	bool DoesClipperExist(m_IDirectDrawClipper* lpClipper);

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
	HRESULT Present();
};
