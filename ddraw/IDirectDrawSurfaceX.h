#pragma once

class m_IDirectDrawSurfaceX : public IUnknown, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawSurface7 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount1 = 0;
	ULONG RefCount2 = 0;
	ULONG RefCount3 = 0;
	ULONG RefCount4 = 0;
	ULONG RefCount7 = 0;

	// Store version wrappers
	m_IDirectDrawSurface* WrapperInterface = nullptr;
	m_IDirectDrawSurface2* WrapperInterface2 = nullptr;
	m_IDirectDrawSurface3* WrapperInterface3 = nullptr;
	m_IDirectDrawSurface4* WrapperInterface4 = nullptr;
	m_IDirectDrawSurface7* WrapperInterface7 = nullptr;

	// Remember the last lock info
	struct LASTLOCK
	{
		bool IsLocked = false;
		DWORD LockedWithID = 0;								// Thread ID of the current lock
		std::vector<RECT> LockRectList;						// Rects used to lock the surface
		bool ReadOnly = false;
		bool IsSkipScene = false;
		RECT Rect = {};
		D3DLOCKED_RECT LockedRect = {};
		DWORD MipMapLevel = 0;

		LASTLOCK() = default;

		// Prevent copy and move
		LASTLOCK(const LASTLOCK&) = delete;
		LASTLOCK& operator=(const LASTLOCK&) = delete;
		LASTLOCK(LASTLOCK&&) = delete;
		LASTLOCK& operator=(LASTLOCK&&) = delete;
	};

	// Removal of scanlines
	struct REMOVESCANLINE
	{
		bool bEvenScanlines = false;
		bool bOddScanlines = false;
		DWORD ScanlineWidth = 0;
		std::vector<BYTE, aligned_allocator<BYTE, 4>> EvenScanLine;
		std::vector<BYTE, aligned_allocator<BYTE, 4>> OddScanLine;
	};

	// Mipmap struct
	struct MIPMAP
	{
		m_IDirectDrawSurface* Addr = nullptr;
		m_IDirectDrawSurface2* Addr2 = nullptr;
		m_IDirectDrawSurface3* Addr3 = nullptr;
		m_IDirectDrawSurface4* Addr4 = nullptr;
		m_IDirectDrawSurface7* Addr7 = nullptr;
		DWORD dwWidth = 0;
		DWORD dwHeight = 0;
		LONG lPitch = 0;
		DWORD UniquenessValue = 0;
		bool IsDummy = false;
	};

	// For ensuring the MipMap context is released after acquiring it
	struct ScopedGetMipMapContext
	{
	private:
		IDirect3DSurface9* pSurfaceD9 = nullptr;
		DWORD MipMapLevel = 0;
		m_IDirectDrawSurfaceX* parent = nullptr;

	public:
		ScopedGetMipMapContext(m_IDirectDrawSurfaceX* parentClass, DWORD Level)
			: MipMapLevel(Level), parent(parentClass)
		{
			pSurfaceD9 = parent->Get3DMipMapSurface(MipMapLevel);
		}

		~ScopedGetMipMapContext()
		{
			parent->Release3DMipMapSurface(pSurfaceD9, MipMapLevel);
		}

		IDirect3DSurface9* GetSurface() const { return pSurfaceD9; }
	};

	// For aligning bits after a lock for games that hard code the pitch
	struct DDRAWEMULATELOCK
	{
		bool Locked = false;
		std::vector<BYTE, aligned_allocator<BYTE, 4>> Mem;
		void* Addr = nullptr;
		DWORD Pitch = 0;
		DWORD NewPitch = 0;
		DWORD BBP = 0;
		DWORD Height = 0;
		DWORD Width = 0;
	};

	struct DDBACKUP
	{
		D3DFORMAT Format = D3DFMT_UNKNOWN;
		DWORD Width = 0;
		DWORD Height = 0;
		DWORD Pitch = 0;
		std::vector<BYTE, aligned_allocator<BYTE, 4>> Bits;
	};

	// Store a list of attached surfaces
	struct ATTACHEDMAP
	{
		m_IDirectDrawSurfaceX* pSurface = nullptr;
		bool isAttachedSurfaceAdded = false;
		DWORD DxVersion = 0;
		DWORD RefCount = 0;
	};

	struct COLORKEY
	{
		bool IsSet = false;
		float lowColorKey[4] = {};
		float highColorKey[4] = {};
	};

	struct SURFACEOVERLAY
	{
		bool OverlayEnabled = false;
		bool isSrcRectNull = true;
		RECT SrcRect = {};
		LPDIRECTDRAWSURFACE7 lpDDDestSurface = nullptr;
		m_IDirectDrawSurfaceX* lpDDDestSurfaceX = nullptr;
		bool isDestRectNull = true;
		RECT DestRect = {};
		DWORD DDOverlayFxFlags = 0;
		DDOVERLAYFX DDOverlayFx = {};
		DWORD DDBltFxFlags = 0;
		DDBLTFX DDBltFx = {};
	};

	// Extra Direct3D9 devices used in the primary surface
	struct D9PRIMARY
	{
		DWORD LastPaletteUSN = 0;								// The USN that was used last time the palette was updated
		COLORKEY ShaderColorKey;								// Used to store color key array for shader
		LPDIRECT3DTEXTURE9 PaletteTexture = nullptr;			// Extra surface texture used for storing palette entries for the pixel shader
	};

	// Real surface and surface data using Direct3D9 devices
	struct D9SURFACE
	{
		bool CanBeRenderTarget = false;
		bool IsUsingWindowedMode = false;
		bool RecreateAuxiliarySurfaces = false;
		bool HasData = false;
		bool UsingSurfaceMemory = false;
		bool UsingShadowSurface = false;
		bool IsDirtyFlag = false;
		bool IsDrawTextureDirty = false;
		bool IsPaletteDirty = false;						// Used to detect if the palette surface needs to be updated
		DWORD BitCount = 0;									// Bit count for this surface
		D3DFORMAT Format = D3DFMT_UNKNOWN;					// Format for this surface
		DWORD Width = 0;									// Width surface/texture was created with
		DWORD Height = 0;									// Height surface/texture was created with
		D3DSURFACETYPE Type = D3DTYPE_NONE;					// Type of resource texture vs surface
		DWORD Usage = 0;									// Usage surface was created with
		D3DPOOL Pool = D3DPOOL_DEFAULT;						// Memory pool texture was created with
		D3DMULTISAMPLE_TYPE MultiSampleType = D3DMULTISAMPLE_NONE;
		DWORD MultiSampleQuality = 0;
		DWORD LastPaletteUSN = 0;							// The USN that was used last time the palette was updated
		const PALETTEENTRY* PaletteEntryArray = nullptr;	// Used to store palette data address
		EMUSURFACE* emu = nullptr;							// Emulated surface using device context
		LPDIRECT3DSURFACE9 Surface = nullptr;				// Surface used for Direct3D
		LPDIRECT3DSURFACE9 Shadow = nullptr;				// Shadow surface for render target
		LPDIRECT3DTEXTURE9 Texture = nullptr;				// Main surface texture used for locks, Blts and Flips
		LPDIRECT3DTEXTURE9 DrawTexture = nullptr;			// Main surface texture with SetTexture calls
		LPDIRECT3DSURFACE9 Context = nullptr;				// Context of the main surface texture
		LPDIRECT3DTEXTURE9 DisplayTexture = nullptr;		// Used to convert palette texture into a texture that can be displayed
		LPDIRECT3DSURFACE9 DisplayContext = nullptr;		// Context for the palette display texture
	};

	typedef enum _SURFACECREATE {
		SC_NOT_CREATED = 0,
		SC_FORCE_EMULATED = 1,
		SC_DONT_FORCE = 2,
	} SURFACECREATE;

	// Convert to Direct3D9
	const DWORD CreatedVersion;
	CRITICAL_SECTION ddscs = {};
	m_IDirectDrawX *ddrawParent = nullptr;				// DirectDraw parent device
	SURFACEOVERLAY SurfaceOverlay;						// The overlays for this surface
	std::vector<MIPMAP> MipMaps;						// MipMaps structure with addresses
	std::unordered_map<DWORD, LASTLOCK> LockedLevel;	// LastLocked MipMap levels
	std::unordered_map<DWORD, HDC> GetDCLevel;			// DC in MipMap levels
	DWORD MaxMipMapLevel = 0;							// Total number of manually created MipMap levels
	bool IsMipMapReadyToUse = false;					// Used for MipMap filtering
	bool RecreateAuxiliarySurfaces = false;
	LPDIRECT3DTEXTURE9 PrimaryDisplayTexture = nullptr;	// Used for the texture surface for the primary surface
	m_IDirectDrawPalette *attachedPalette = nullptr;	// Associated palette
	m_IDirectDrawClipper *attachedClipper = nullptr;	// Associated clipper
	m_IDirect3DTextureX *attached3DTexture = nullptr;	// Associated texture
	m_IDirect3DDeviceX *attached3DDevice = nullptr;		// Associated device
	DDSURFACEDESC2 surfaceDesc2 = {};					// Surface description for this surface
	DWORD ResetDisplayFlags = 0;						// Flags that need to be reset when display mode changes
	DWORD UniquenessValue = 0;
	LONG overlayX = 0;
	LONG overlayY = 0;
	DWORD Priority = 0;

	bool Using3D = false;								// Direct3D is being used on top of DirectDraw
	bool DCRequiresEmulation = false;
	bool SurfaceRequiresEmulation = false;
	bool ComplexRoot = false;
	bool ComplexChild = false;
	bool IsSurfaceLost = false;
	bool IsInFlip = false;
	bool PresentOnUnlock = false;
	bool IsInBlt = false;
	bool IsInBltBatch = false;
	bool WasBitAlignLocked = false;
	DDRAWEMULATELOCK EmuLock;							// For aligning bits after a lock for games that hard code the pitch
	REMOVESCANLINE EmuScanLine;							// For removal an restoration of scanlines
	std::vector<BYTE, aligned_allocator<BYTE, 4>> ByteArray;						// Memory used for coping from one surface to the same surface
	std::vector<DDBACKUP> LostDeviceBackup;				// Memory used for backing up the surfaceTexture
	COLORKEY ShaderColorKey;							// Used to store color key array for shader
	SURFACECREATE ShouldEmulate = SC_NOT_CREATED;		// Used to help determine if surface should be emulated

	// Extra Direct3D9 devices used in the primary surface
	D9PRIMARY primary;

	// Real surface and surface data using Direct3D9 devices
	D9SURFACE surface;

	// Direct3D9 device address
	LPDIRECT3DDEVICE9* d3d9Device = nullptr;

	// Store a list of attached surfaces
	std::unique_ptr<m_IDirectDrawSurfaceX> BackBufferInterface;
	std::map<DWORD, ATTACHEDMAP> AttachedSurfaceMap;
	DWORD MapKey = 0;

	// Helper functions
	LPDIRECT3DSURFACE9 Get3DSurface();
	LPDIRECT3DSURFACE9 Get3DMipMapSurface(DWORD MipMapLevel);
	void Release3DMipMapSurface(LPDIRECT3DSURFACE9 pSurfaceD9, DWORD MipMapLevel);
	LPDIRECT3DTEXTURE9 Get3DTexture();
	void CheckMipMapLevelGen();
	HRESULT CheckInterface(char* FunctionName, bool CheckD3DDevice, bool CheckD3DSurface, bool CheckLostSurface);
	HRESULT CreateD9AuxiliarySurfaces();
	HRESULT CreateD9Surface();
	bool DoesDCMatch(EMUSURFACE* pEmuSurface) const;
	void SetEmulationGameDC();
	void UnsetEmulationGameDC();
	HRESULT CreateDCSurface();
	void ReleaseDCSurface();
	void UpdateAttachedDepthStencil(m_IDirectDrawSurfaceX* lpAttachedSurfaceX);
	void UpdateSurfaceDesc();

	// Direct3D9 interfaces
	HRESULT LockD3d9Surface(D3DLOCKED_RECT* pLockedRect, RECT* pRect, DWORD Flags, DWORD MipMapLevel);
	HRESULT UnLockD3d9Surface(DWORD MipMapLevel);
	void SetDirtyFlag(DWORD MipMapLevel);

	// Swap surface addresses for Flip
	template <typename T>
	inline void SwapAddresses(T* Address1, T* Address2)
	{
		T tmpAddr = *Address1;
		*Address1 = *Address2;
		*Address2 = tmpAddr;
	}
	HRESULT CheckBackBufferForFlip(m_IDirectDrawSurfaceX* lpTargetSurface);

	// Locking rect coordinates
	bool CheckCoordinates(LPRECT lpInRect)
	{
		RECT Rect = {};
		return CheckCoordinates(Rect, lpInRect, &surfaceDesc2);
	}
	bool CheckCoordinates(RECT& OutRect, LPRECT lpInRect, LPDDSURFACEDESC2 lpDDSurfaceDesc2);
	HRESULT LockEmulatedSurface(D3DLOCKED_RECT* pLockedRect, LPRECT lpDestRect) const;
	bool CheckRectforSkipScene(RECT& DestRect);
	HRESULT PresentOverlay(LPRECT lpSrcRect);
	void BeginWritePresent(bool IsSkipScene);
	void EndWritePresent(LPRECT lpDestRect, bool IsFlip, bool WriteToWindow, bool FullPresent, bool IsSkipScene);
	void EndWriteSyncSurfaces(LPRECT lpDestRect);

	// Surface information functions
	bool IsSurfaceLocked(DWORD MipMapLevel = DXW_ALL_SURFACE_LEVELS);
	bool IsSurfaceBlitting() const { return (IsInBlt || IsInBltBatch); }
	bool IsSurfaceInDC(DWORD MipMapLevel = DXW_ALL_SURFACE_LEVELS);
	bool IsD9UsingVideoMemory() const { return ((surface.Surface || surface.Texture) ? surface.Pool == D3DPOOL_DEFAULT : false); }
	bool IsUsingShadowSurface() const { return (surface.UsingShadowSurface && surface.Shadow); }
	bool IsLockedFromOtherThread(DWORD MipMapLevel);
	bool IsDummyMipMap(DWORD MipMapLevel) { return (MipMapLevel > MaxMipMapLevel || ((MipMapLevel & ~DXW_IS_MIPMAP_DUMMY) - 1 < MipMaps.size() && MipMaps[(MipMapLevel & ~DXW_IS_MIPMAP_DUMMY) - 1].IsDummy)); }
	DWORD GetD3d9MipMapLevel(DWORD MipMapLevel) const { return min(MipMapLevel, MaxMipMapLevel); }
	DWORD GetWidth() const { return surfaceDesc2.dwWidth; }
	DWORD GetHeight() const { return surfaceDesc2.dwHeight; }
	DDSCAPS2 GetSurfaceCaps() const { return surfaceDesc2.ddsCaps; }
	D3DFORMAT GetSurfaceFormat() const { return surface.Format; }

	// Attached surfaces
	void InitSurfaceDesc(DWORD DirectXVersion);
	void AddAttachedSurfaceToMap(m_IDirectDrawSurfaceX* lpSurfaceX, bool MarkAttached, DWORD DxVersion, DWORD RefCount);
	bool DoesAttachedSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool WasAttachedSurfaceAdded(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool DoesFlipBackBufferExist(m_IDirectDrawSurfaceX* lpSurfaceX);
	HRESULT GetFlipList(std::vector<m_IDirectDrawSurfaceX*>& FlipList, LPDIRECTDRAWSURFACE7 lpDDSurfaceTargetOverride);

	// Copying surface textures
	void SetRenderTargetShadow();
	HRESULT SaveDXTDataToDDS(const void* data, size_t dataSize, const char* filename, int dxtVersion) const;
	HRESULT SaveSurfaceToFile(const char* filename, D3DXIMAGE_FILEFORMAT format);
	HRESULT CopySurface(m_IDirectDrawSurfaceX* pSourceSurface, RECT* pSourceRect, RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter, D3DCOLOR ColorKey, DWORD dwFlags, DWORD SrcMipMapLevel, DWORD MipMapLevel);
	HRESULT CopyZBuffer(m_IDirectDrawSurfaceX* pSourceSurface, RECT* pSourceRect, RECT* pDestRect, bool DepthFill, DWORD DepthColor);
	HRESULT CopyToDrawTexture(LPRECT lpDestRect);
	HRESULT LoadSurfaceFromMemory(LPDIRECT3DSURFACE9 pDestSurface, const RECT& Rect, LPCVOID pSrcMemory, D3DFORMAT SrcFormat, UINT SrcPitch);
	HRESULT CopyFromEmulatedSurface(LPRECT lpDestRect);
	HRESULT CopyToEmulatedSurface(LPRECT lpDestRect);
	HRESULT CopyEmulatedPaletteSurface(LPRECT lpDestRect);
	HRESULT CopyEmulatedSurfaceFromGDI(LPRECT lpDestRect);
	HRESULT CopyEmulatedSurfaceToGDI(LPRECT lpDestRect);

	// Wrapper interface functions
	inline REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirectDrawSurface :
			(DirectXVersion == 2) ? IID_IDirectDrawSurface2 :
			(DirectXVersion == 3) ? IID_IDirectDrawSurface3 :
			(DirectXVersion == 4) ? IID_IDirectDrawSurface4 :
			(DirectXVersion == 7) ? IID_IDirectDrawSurface7 : IID_IUnknown;
	}
	inline bool CheckWrapperType(REFIID IID)
	{
		return (IID == IID_IDirectDrawSurface ||
			IID == IID_IDirectDrawSurface2 ||
			IID == IID_IDirectDrawSurface3 ||
			IID == IID_IDirectDrawSurface4 ||
			IID == IID_IDirectDrawSurface7) ? true : false;
	}
	inline IDirectDrawSurface *GetProxyInterfaceV1() { return (IDirectDrawSurface *)ProxyInterface; }
	inline IDirectDrawSurface2 *GetProxyInterfaceV2() { return (IDirectDrawSurface2 *)ProxyInterface; }
	inline IDirectDrawSurface3 *GetProxyInterfaceV3() { return (IDirectDrawSurface3 *)ProxyInterface; }
	inline IDirectDrawSurface4 *GetProxyInterfaceV4() { return (IDirectDrawSurface4 *)ProxyInterface; }
	inline IDirectDrawSurface7 *GetProxyInterfaceV7() { return ProxyInterface; }

	// Interface initialization functions
	void InitInterface(DWORD DirectXVersion);
	void ReleaseDirectDrawResources();
	void ReleaseInterface();

public:
	m_IDirectDrawSurfaceX(IDirectDrawSurface7 *pOriginal, DWORD DirectXVersion) : ProxyInterface(pOriginal), CreatedVersion(DirectXVersion)
	{
		ProxyDirectXVersion = GetGUIDVersion(GetWrapperType(DirectXVersion));

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
	m_IDirectDrawSurfaceX(m_IDirectDrawX *Interface, DWORD DirectXVersion, LPDDSURFACEDESC2 lpDDSurfaceDesc2) : ddrawParent(Interface), CreatedVersion(DirectXVersion)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		// Copy surface description, needs to run before InitInterface()
		if (lpDDSurfaceDesc2)
		{
			surfaceDesc2 = *lpDDSurfaceDesc2;
		}

		InitInterface(DirectXVersion);
	}
	~m_IDirectDrawSurfaceX()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseInterface();
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, 0); }
	STDMETHOD_(ULONG, AddRef) (THIS) { return AddRef(0); }
	STDMETHOD_(ULONG, Release) (THIS) { return Release(0); }

	/*** IDirectDrawSurface methods ***/
	STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE7, DWORD);
	STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT);
	HRESULT Blt(LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX, DWORD, bool PresentBlt = true);
	STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD, DWORD);
	STDMETHOD(BltFast)(THIS_ DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, DWORD);
	STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD, LPDIRECTDRAWSURFACE7);
	HRESULT GetMipMapLevelAddr(LPDIRECTDRAWSURFACE7 FAR* lplpDDAttachedSurface, MIPMAP& MipMapSurface, DWORD MipMapLevel, DWORD DirectXVersion);
	HRESULT GetMipMapSubLevel(LPDIRECTDRAWSURFACE7 FAR* lplpDDAttachedSurface, DWORD MipMapLevel, DWORD DirectXVersion);
	HRESULT EnumAttachedSurfaces(LPVOID, LPDDENUMSURFACESCALLBACK, DWORD, DWORD);
	HRESULT EnumAttachedSurfaces2(LPVOID, LPDDENUMSURFACESCALLBACK7, LPDDENUMSURFACESCALLBACK, DWORD, DWORD);
	HRESULT EnumOverlayZOrders(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK, DWORD);
	HRESULT EnumOverlayZOrders2(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7, LPDDENUMSURFACESCALLBACK, DWORD);
	STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE7, DWORD, DWORD);
	HRESULT GetAttachedSurface(LPDDSCAPS, LPDIRECTDRAWSURFACE7 FAR *, DWORD, DWORD);
	HRESULT GetAttachedSurface2(LPDDSCAPS2, LPDIRECTDRAWSURFACE7 FAR *, DWORD, DWORD);
	STDMETHOD(GetBltStatus)(THIS_ DWORD, DWORD MipMapLevel);
	HRESULT GetCaps(LPDDSCAPS);
	HRESULT GetCaps2(LPDDSCAPS2);
	STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*);
	STDMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY);
	STDMETHOD(GetDC)(THIS_ HDC FAR *, DWORD MipMapLevel);
	STDMETHOD(GetFlipStatus)(THIS_ DWORD, bool);
	STDMETHOD(GetOverlayPosition)(THIS_ LPLONG, LPLONG);
	STDMETHOD(GetPalette)(THIS_ LPDIRECTDRAWPALETTE FAR*);
	STDMETHOD(GetPixelFormat)(THIS_ LPDDPIXELFORMAT);
	HRESULT GetSurfaceDesc(LPDDSURFACEDESC, DWORD, DWORD);
	HRESULT GetSurfaceDesc2(LPDDSURFACEDESC2, DWORD, DWORD);
	HRESULT Initialize(LPDIRECTDRAW, LPDDSURFACEDESC);
	HRESULT Initialize2(LPDIRECTDRAW, LPDDSURFACEDESC2);
	STDMETHOD(IsLost)(THIS);
	HRESULT Lock(LPRECT, LPDDSURFACEDESC, DWORD, HANDLE, DWORD, DWORD);
	HRESULT Lock2(LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE, DWORD, DWORD);
	STDMETHOD(ReleaseDC)(THIS_ HDC, DWORD MipMapLevel);
	STDMETHOD(Restore)(THIS);
	STDMETHOD(SetClipper)(THIS_ LPDIRECTDRAWCLIPPER);
	STDMETHOD(SetColorKey)(THIS_ DWORD, LPDDCOLORKEY);
	STDMETHOD(SetOverlayPosition)(THIS_ LONG, LONG);
	STDMETHOD(SetPalette)(THIS_ LPDIRECTDRAWPALETTE);
	STDMETHOD(Unlock)(THIS_ LPRECT, DWORD);
	STDMETHOD(UpdateOverlay)(THIS_ LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDOVERLAYFX);
	STDMETHOD(UpdateOverlayDisplay)(THIS_ DWORD);
	STDMETHOD(UpdateOverlayZOrder)(THIS_ DWORD, LPDIRECTDRAWSURFACE7);

	/*** Added in the v2 interface ***/
	STDMETHOD(GetDDInterface)(THIS_ LPVOID FAR *, DWORD);
	STDMETHOD(PageLock)(THIS_ DWORD);
	STDMETHOD(PageUnlock)(THIS_ DWORD);

	/*** Added in the v3 interface ***/
	HRESULT SetSurfaceDesc(LPDDSURFACEDESC, DWORD);
	HRESULT SetSurfaceDesc2(LPDDSURFACEDESC2, DWORD);

	/*** Added in the v4 interface ***/
	STDMETHOD(SetPrivateData)(THIS_ REFGUID, LPVOID, DWORD, DWORD);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID, LPVOID, LPDWORD);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID);
	STDMETHOD(GetUniquenessValue)(THIS_ LPDWORD, DWORD MipMapLevel);
	STDMETHOD(ChangeUniquenessValue)(THIS_ DWORD MipMapLevel);

	/*** Moved Texture7 methods here ***/
	STDMETHOD(SetPriority)(THIS_ DWORD);
	STDMETHOD(GetPriority)(THIS_ LPDWORD);
	STDMETHOD(SetLOD)(THIS_ DWORD);
	STDMETHOD(GetLOD)(THIS_ LPDWORD);

	// Helper functions
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
	ULONG AddRef(DWORD DirectXVersion);
	ULONG Release(DWORD DirectXVersion);

	// Fix byte alignment issue
	void LockEmuLock(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc);
	void UnlockEmuLock();

	// For removing scanlines
	void RestoreScanlines(LASTLOCK &LLock) const;
	void RemoveScanlines(LASTLOCK &LLock);

	// Functions handling the ddraw parent interface
	void SetDdrawParent(m_IDirectDrawX* ddraw);
	m_IDirectDrawX* GetDDrawParent() { return ddrawParent; };
	void ClearDdraw();

	// Direct3D9 interface functions
	void SetAsRenderTarget();
	void ReleaseD9AuxiliarySurfaces();
	void ReleaseD9Surface(bool BackupData, bool ResetSurface);
	HRESULT PresentSurface(bool IsFlip, bool IsSkipScene);
	void ResetSurfaceDisplay();

	// Surface information functions
	bool IsPrimarySurface() const { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) != 0; }
	bool IsBackBuffer() const { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_BACKBUFFER) != 0; }
	bool IsPrimaryOrBackBuffer() const { return (IsPrimarySurface() || IsBackBuffer()); }
	bool IsRenderTarget() const { return surface.CanBeRenderTarget; }
	bool IsFlipSurface() const { return ((surfaceDesc2.ddsCaps.dwCaps & (DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER)) == (DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER)); }
	bool IsSurface3D() const { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_3DDEVICE) != 0; }
	bool IsSurfaceTexture() const { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_TEXTURE) != 0; }
	bool IsSurfaceCreated() const { return (surface.Texture || surface.Surface); }
	bool HasAlphaChannel() const;
	bool IsColorKeyTexture() const { return (IsSurfaceTexture() && (surfaceDesc2.dwFlags & DDSD_CKSRCBLT)); }
	bool IsPalette() const { return (surface.Format == D3DFMT_P8); }
	bool IsDepthStencil() const { return (surfaceDesc2.ddpfPixelFormat.dwFlags & (DDPF_ZBUFFER | DDPF_STENCILBUFFER)) != 0; }
	DWORD GetAttachedStencilSurfaceZBits();
	bool IsSurfaceManaged() const { return (surfaceDesc2.ddsCaps.dwCaps2 & (DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE)) != 0; }
	bool IsSurfaceBusy(DWORD MipMapLevel = DXW_ALL_SURFACE_LEVELS) { return (IsSurfaceBlitting() || IsSurfaceLocked(MipMapLevel) || IsSurfaceInDC(MipMapLevel)); }
	bool CanSurfaceBeDeleted() const { return !ComplexChild; }
	bool CanSurfaceUseEmulation() const
	{ return ((IsPixelFormatRGB(surfaceDesc2.ddpfPixelFormat) || IsPixelFormatPalette(surfaceDesc2.ddpfPixelFormat)) && (!IsSurface3D() || !Using3D) && !surface.UsingSurfaceMemory); }
	bool IsUsingEmulation() const { return (surface.emu && surface.emu->DC && surface.emu->GameDC && surface.emu->pBits); }
	bool IsEmulationDCReady() const { return (IsUsingEmulation() && !surface.emu->UsingGameDC); }
	bool IsSurfaceDirty() const { return surface.IsDirtyFlag; }
	bool IsMipMapAutogen() const { return surface.Texture && (surface.Usage & D3DUSAGE_AUTOGENMIPMAP); }
	bool IsMipMapGenerated() const { return IsMipMapReadyToUse || IsMipMapAutogen(); }
	void FixTextureFlags(LPDDSURFACEDESC2 lpDDSurfaceDesc2);
	void PrepareRenderTarget();
	void ClearDirtyFlags();
	bool CanSurfaceBeLost() const;
	void MarkSurfaceLost();
	bool GetColorKeyForShader(float(&lowColorKey)[4], float(&highColorKey)[4]);
	bool GetColorKeyForPrimaryShader(float(&lowColorKey)[4], float(&highColorKey)[4]);
	bool GetWasBitAlignLocked() const { return WasBitAlignLocked; }
	bool GetSurfaceSetSize(DWORD& Width, DWORD& Height) const
	{
		if ((surfaceDesc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT)) == (DDSD_WIDTH | DDSD_HEIGHT) &&
			(ResetDisplayFlags & (DDSD_WIDTH | DDSD_HEIGHT)) == 0 &&
			surfaceDesc2.dwWidth && surfaceDesc2.dwHeight)
		{
			Width = surfaceDesc2.dwWidth;
			Height = surfaceDesc2.dwHeight;
			return true;
		}
		return false;
	}
	m_IDirectDrawSurfaceX* GetAttachedDepthStencil();
	LPDIRECT3DSURFACE9 GetD3d9Surface();
	LPDIRECT3DTEXTURE9 GetD3d9DrawTexture();
	LPDIRECT3DTEXTURE9 GetD3d9Texture(bool InterfaceCheck = true);
	HRESULT GenerateMipMapLevels();
	DWORD GetD3d9Width() const { return surface.Width; }
	DWORD GetD3d9Height() const { return surface.Height; }
	D3DFORMAT GetD3d9Format() const { return surface.Format; }
	LPDIRECT3DTEXTURE9 GetD3d9PaletteTexture() const { return primary.PaletteTexture; }
	m_IDirect3DTextureX* GetAttachedTexture() { return attached3DTexture; }
	void ClearUsing3DFlag();
	HRESULT GetPresentWindowRect(LPRECT pRect, RECT& DestRect);
	CRITICAL_SECTION* GetCriticalSection() { return IsPrimaryOrBackBuffer() || IsRenderTarget() ? DdrawWrapper::GetDDCriticalSection() : &ddscs; }

	// For texture loading
	HRESULT Load(LPDIRECTDRAWSURFACE7 lpDestTex, LPPOINT lpDestPoint, LPDIRECTDRAWSURFACE7 lpSrcTex, LPRECT lprcSrcRect, DWORD dwFlags);

	// For Presenting
	void CopyGDIToPrimaryAndBackbuffer();

	// For Present checking
	bool ShouldReadFromGDI() const { return (Config.DdrawReadFromGDI && IsPrimarySurface() && IsUsingEmulation() && !Using3D); }
	bool ShouldWriteToGDI() const { return (Config.DdrawWriteToGDI && IsPrimarySurface() && IsUsingEmulation() && !Using3D); }
	bool ShouldPresentToWindow(bool IsPresenting) const
	{
		return (surface.IsUsingWindowedMode && (IsPresenting ? (IsPrimarySurface() && !IsRenderTarget()) : IsPrimaryOrBackBuffer()) && !Config.DdrawWriteToGDI);
	}

	// Draw 2D DirectDraw surface
	HRESULT ColorFill(RECT* pRect, D3DCOLOR dwFillColor, DWORD MipMapLevel);

	// Attached surfaces
	void RemoveAttachedSurfaceFromMap(m_IDirectDrawSurfaceX* lpSurfaceX);

	// For clipper
	void RemoveClipper(m_IDirectDrawClipper* ClipperToRemove);

	// For palettes
	m_IDirectDrawPalette *GetAttachedPalette() { return attachedPalette; }
	void RemovePalette(m_IDirectDrawPalette* PaletteToRemove);
	void UpdatePaletteData();

	// External static functions
	static void StartSharedEmulatedMemory();
	static void DeleteEmulatedMemory(EMUSURFACE **ppEmuSurface);
	static void CleanupSharedEmulatedMemory();
	static void SizeDummySurface(size_t size);
	static void CleanupDummySurface();
};
