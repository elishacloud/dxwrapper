#pragma once

class m_IDirectDrawX : public IUnknown, public AddressLookupTableDdrawObject
{
private:
	IDirectDraw7 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	DWORD ClientDirectXVersion;
	ULONG RefCount1 = 0;
	ULONG RefCount2 = 0;
	ULONG RefCount3 = 0;
	ULONG RefCount4 = 0;
	ULONG RefCount7 = 0;

	// Store version wrappers
	m_IDirectDraw* WrapperInterface = nullptr;
	m_IDirectDraw2* WrapperInterface2 = nullptr;
	m_IDirectDraw3* WrapperInterface3 = nullptr;
	m_IDirectDraw4* WrapperInterface4 = nullptr;
	m_IDirectDraw7* WrapperInterface7 = nullptr;

	// Cached DirectDraw flags
	const bool IsUsingEx = false;
	bool Using3D = false;
	const UINT AdapterIndex = D3DADAPTER_DEFAULT;

	// Fix exclusive mode issue
	HHOOK g_hook = nullptr;
	HWND chWnd = nullptr;

	// Store primary surface
	m_IDirectDrawSurfaceX *PrimarySurface = nullptr;
	m_IDirectDrawSurfaceX *RenderTargetSurface = nullptr;
	m_IDirectDrawSurfaceX *DepthStencilSurface = nullptr;

	// Store color control interface
	m_IDirectDrawColorControl* ColorControlInterface = nullptr;

	// Store gamma control interface
	m_IDirectDrawGammaControl* GammaControlInterface = nullptr;

	// Store a list of clippers
	struct CLIPPERLIST {
		m_IDirectDrawClipper* Interface = nullptr;
		DWORD DxVersion = 0;
		DWORD RefCount = 0;
	};
	std::vector<CLIPPERLIST> ClipperList;

	// Store a list of palettes
	struct PALETTELIST {
		m_IDirectDrawPalette* Interface = nullptr;
		DWORD DxVersion = 0;
		DWORD RefCount = 0;
	};
	std::vector<PALETTELIST> PaletteList;

	// Store a list of surfaces
	struct SURFACELIST {
		m_IDirectDrawSurfaceX* Interface = nullptr;
		DWORD DxVersion = 0;
		DWORD RefCount = 0;
	};
	std::vector<SURFACELIST> SurfaceList;
	std::vector<m_IDirectDrawSurfaceX*> ReleasedSurfaceList;

	// Store a list of vertex buffers
	std::vector<m_IDirect3DVertexBufferX*> VertexBufferList;

	// Store d3d interface
	m_IDirect3DX *D3DInterface = nullptr;

	// Helper functions
	HRESULT CheckInterface(char* FunctionName, bool CheckD3DDevice);
	void FindMonitorHandle() const;
	HRESULT CreateD9Object();
	void BackupAndResetState(DRAWSTATEBACKUP& DrawStates, DWORD Width, DWORD Height);
	void RestoreState(DRAWSTATEBACKUP& DrawStates);
	HRESULT CopyPrimarySurface(LPDIRECT3DSURFACE9 pDestBuffer);
	HRESULT DrawPrimarySurface(LPDIRECT3DTEXTURE9 pDisplayTexture);
	static DWORD WINAPI PresentThreadFunction(LPVOID);
	HRESULT Present(RECT* pSourceRect, RECT* pDestRect);
	void RestoreD3DDeviceState();
	void Clear3DFlagForAllSurfaces();
	void MarkAllSurfacesDirty();
	void ResetAllSurfaceDisplay();
	void ReleaseD3D9IndexBuffer(LPDIRECT3DINDEXBUFFER9& d3d9IndexBuffer, DWORD& IndexBufferSize);
	void ReleaseAllD9Resources(bool BackupData, bool ResetInterface);
	void ReleaseD9Device();
	void ReleaseD9Object();

	// Gamma functions
	LPDIRECT3DPIXELSHADER9 GetGammaPixelShader();
	HRESULT SetBrightnessLevel(D3DGAMMARAMP& RampData);

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
	void InitInterface(DWORD DirectXVersion);
	void ReleaseInterface();

public:
	m_IDirectDrawX(IDirectDraw7 *aOriginal, DWORD DirectXVersion) : ProxyInterface(aOriginal)
	{
		ProxyDirectXVersion = GetGUIDVersion(GetWrapperType(DirectXVersion));

		ClientDirectXVersion = DirectXVersion;

		if (ProxyDirectXVersion != DirectXVersion)
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);
		}
		else
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ") v" << DirectXVersion);
		}
		if (Config.Dd7to9)
		{
			Logging::Log() << __FUNCTION__ << " (" << this << ") Warning: created from non-dd7to9 interface!";
		}

		InitInterface(DirectXVersion);
	}
	m_IDirectDrawX(DWORD DirectXVersion, UINT Adapter, bool IsEx) : AdapterIndex(Adapter), IsUsingEx(IsEx)
	{
		ProxyDirectXVersion = 9;

		ClientDirectXVersion = DirectXVersion;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		InitInterface(DirectXVersion);
	}
	~m_IDirectDrawX()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseInterface();
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, 0); }
	STDMETHOD_(ULONG, AddRef) (THIS) { return AddRef(0); }
	STDMETHOD_(ULONG, Release) (THIS) { return Release(0); }

	/*** IDirectDraw methods ***/
	STDMETHOD(Compact)(THIS);
	STDMETHOD(CreateClipper)(THIS_ DWORD, LPDIRECTDRAWCLIPPER FAR *, IUnknown FAR *, DWORD);
	STDMETHOD(CreatePalette)(THIS_ DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR *, IUnknown FAR *, DWORD);
	HRESULT CreateSurface(LPDDSURFACEDESC, LPDIRECTDRAWSURFACE7 FAR *, IUnknown FAR *, DWORD);
	HRESULT CreateSurface2(LPDDSURFACEDESC2, LPDIRECTDRAWSURFACE7 FAR *, IUnknown FAR *, DWORD);
	STDMETHOD(DuplicateSurface)(THIS_ LPDIRECTDRAWSURFACE7, LPDIRECTDRAWSURFACE7 FAR *, DWORD);
	HRESULT EnumDisplayModes(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK, DWORD);
	HRESULT EnumDisplayModes2(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2, DWORD);
	HRESULT EnumSurfaces(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMSURFACESCALLBACK, DWORD);
	HRESULT EnumSurfaces2(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMSURFACESCALLBACK7, LPDDENUMSURFACESCALLBACK, DWORD);
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
	m_IDirect3DX** GetCurrentD3D() { return &D3DInterface; }
	void ClearD3DDevice();
	bool IsCreatedEx() const { return IsUsingEx; }
	void Enable3D() { Using3D = true; }
	bool IsUsing3D() const { return Using3D; }
	bool IsPrimaryRenderTarget() { return PrimarySurface ? PrimarySurface->IsRenderTarget() : false; }
	bool IsInScene();

	// Direct3D9 interfaces
	UINT GetAdapterIndex() const { return AdapterIndex; }
	bool CheckD9Device(char* FunctionName);
	LPDIRECT3D9 GetDirectD9Object();
	LPDIRECT3DDEVICE9 *GetDirectD9Device();
	bool CreatePaletteShader();
	LPDIRECT3DPIXELSHADER9* GetColorKeyShader();
	LPDIRECT3DVERTEXSHADER9* GetVertexFixupShader();
	LPDIRECT3DVERTEXBUFFER9 GetValidateDeviceVertexBuffer(DWORD& FVF, DWORD& Size);
	LPDIRECT3DINDEXBUFFER9 GetIndexBuffer(LPWORD lpwIndices, DWORD dwIndexCount);
	LPDIRECT3DINDEXBUFFER9 GetIndexBufferX(LPWORD lpwIndices, DWORD dwIndexCount, DWORD& IndexBufferSize, LPDIRECT3DINDEXBUFFER9& d3d9IndexBuffer);
	D3DMULTISAMPLE_TYPE GetMultiSampleTypeQuality(D3DFORMAT Format, DWORD MaxSampleType, DWORD& QualityLevels) const;
	HRESULT ResetD9Device();
	HRESULT CreateD9Device(char* FunctionName);
	void UpdateVertices(DWORD Width, DWORD Height);
	HRESULT TestD3D9CooperativeLevel();

	// Device information functions
	static m_IDirectDrawX* GetDirectDrawInterface();
	HMONITOR GetHMonitor();
	HWND GetHwnd();
	DWORD GetHwndThreadID();
	HDC GetDC();
	DWORD GetDisplayBPP();
	bool IsExclusiveMode();
	DWORD GetLastDrawDevice();
	void SetLastDrawDevice(DWORD DrawDevice);
	void GetSurfaceDisplay(DWORD& Width, DWORD& Height, DWORD& BPP, DWORD& RefreshRate);
	void GetViewportResolution(DWORD& Width, DWORD& Height);
	void GetDisplayPixelFormat(DDPIXELFORMAT& ddpfPixelFormat, DWORD BPP);

	// State block functions
	void CreateStateBlock();
	void ApplyStateBlock();

	// Surface vector functions
	void AddReleasedSurface(m_IDirectDrawSurfaceX* lpSurfaceX);
	void AddSurface(m_IDirectDrawSurfaceX* lpSurfaceX);
	void ClearSurface(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool DoesSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX);
	m_IDirectDrawSurfaceX *GetPrimarySurface() { return PrimarySurface; }
	m_IDirectDrawSurfaceX *GetRenderTargetSurface() { return RenderTargetSurface; }
	void ClearRenderTarget();
	void SetCurrentRenderTarget();
	HRESULT SetRenderTargetSurface(m_IDirectDrawSurfaceX* lpSurface);
	m_IDirectDrawSurfaceX *GetDepthStencilSurface() { return DepthStencilSurface; }
	HRESULT SetDepthStencilSurface(m_IDirectDrawSurfaceX* lpSurface);

	// Texture functions
	void ClearTextureHandle(D3DTEXTUREHANDLE tHandle);

	// Clipper functions
	static void AddBaseClipper(m_IDirectDrawClipper* lpClipper);
	static void ClearBaseClipper(m_IDirectDrawClipper* lpClipper);
	static bool DoesBaseClipperExist(m_IDirectDrawClipper* lpClipper);
	void AddClipper(m_IDirectDrawClipper* lpClipper);
	void ClearClipper(m_IDirectDrawClipper* lpClipper);
	bool DoesClipperExist(m_IDirectDrawClipper* lpClipper);
	HWND GetClipperHWnd();
	HRESULT SetClipperHWnd(HWND hWnd);

	// Palette vector functions
	void AddPalette(m_IDirectDrawPalette* lpPalette);
	void ClearPalette(m_IDirectDrawPalette* lpPalette);
	bool DoesPaletteExist(m_IDirectDrawPalette* lpPalette);

	// Vertex buffer vector functions
	void AddVertexBuffer(m_IDirect3DVertexBufferX* lpVertexBuffer);
	void ClearVertexBuffer(m_IDirect3DVertexBufferX* lpVertexBuffer);

	// Color and gamma control
	m_IDirectDrawColorControl* GetColorControlInterface() { return ColorControlInterface; }
	HRESULT CreateColorControl(m_IDirectDrawColorControl** lplpColorControl);
	void SetColorControl(m_IDirectDrawColorControl* lpColorControl);
	void ClearColorControl(m_IDirectDrawColorControl* lpColorControl);
	m_IDirectDrawGammaControl* GetGammaControlInterface() { return GammaControlInterface; }
	HRESULT CreateGammaControl(m_IDirectDrawGammaControl** lplpGammaControl);
	void SetGammaControl(m_IDirectDrawGammaControl* lpGammaControl);
	void ClearGammaControl(m_IDirectDrawGammaControl* lpGammaControl);

	// Begin & end scene
	void SetVsync();
	HRESULT GetD9Gamma(DWORD dwFlags, LPDDGAMMARAMP lpRampData);
	HRESULT SetD9Gamma(DWORD dwFlags, LPDDGAMMARAMP lpRampData);
	bool IsUsingThreadPresent();
	HRESULT PresentScene(RECT* pRect);

	// External static functions
	static bool CheckDirectDrawXInterface(void* pInterface);
	static DWORD GetDDrawBitsPixel(HWND hWnd);
	static DWORD GetDDrawWidth();
	static DWORD GetDDrawHeight();
};
