#pragma once

#include <map>
#include "d3dx9.h"

// Emulated surface
struct EMUSURFACE
{
	HDC DC = nullptr;
	HDC GameDC = nullptr;
	bool UsingGameDC = false;
	DWORD Size = 0;
	D3DFORMAT Format = D3DFMT_UNKNOWN;
	void *pBits = nullptr;
	DWORD Pitch = 0;
	HBITMAP bitmap = nullptr;
	BYTE bmiMemory[(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256)] = {};
	PBITMAPINFO bmi = (PBITMAPINFO)bmiMemory;
	HGDIOBJ OldDCObject = nullptr;
	HGDIOBJ OldGameDCObject = nullptr;
	DWORD LastPaletteUSN = 0;
};

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

	// Remember the last lock info
	struct LASTLOCK
	{
		bool bEvenScanlines = false;
		bool bOddScanlines = false;
		bool ReadOnly = false;
		bool IsSkipScene = false;
		DWORD ScanlineWidth = 0;
		std::vector<BYTE> EvenScanLine;
		std::vector<BYTE> OddScanLine;
		RECT Rect = {};
		D3DLOCKED_RECT LockedRect = {};
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
	};

	// For aligning bits after a lock for games that hard code the pitch
	struct DDRAWEMULATELOCK
	{
		bool Locked = false;
		std::vector<byte> Mem;
		void* Addr = nullptr;
		DWORD Pitch = 0;
		DWORD NewPitch = 0;
		DWORD BBP = 0;
		DWORD Height = 0;
		DWORD Width = 0;
	};

	struct DDBACKUP
	{
		std::vector<byte> Bits;
	};

	// Store a list of attached surfaces
	struct ATTACHEDMAP
	{
		m_IDirectDrawSurfaceX* pSurface = nullptr;
		bool isAttachedSurfaceAdded = false;
	};

	struct COLORKEY
	{
		bool IsSet = false;
		float lowColorKey[4] = {};
		float highColorKey[4] = {};
	};

	struct TEXTURECREATE
	{
		DWORD Width = 0;
		DWORD Height = 0;
		D3DPOOL Pool = D3DPOOL_DEFAULT;
		DWORD Usage = 0;
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
		bool IsUsingWindowedMode = false;
		bool SurfaceHasData = false;
		bool UsingSurfaceMemory = false;
		bool IsDirtyFlag = false;
		bool IsDrawTextureDirty = false;
		bool IsPaletteDirty = false;						// Used to detect if the palette surface needs to be updated
		DWORD LastPaletteUSN = 0;							// The USN that was used last time the palette was updated
		TEXTURECREATE Tex;									// Values used for creating textures
		D3DPOOL SurfacePool = D3DPOOL_DEFAULT;				// Memory pool surface was created with
		EMUSURFACE* emu = nullptr;							// Emulated surface using device context
		LPPALETTEENTRY PaletteEntryArray = nullptr;			// Used to store palette data address
		LPDIRECT3DSURFACE9 Surface = nullptr;				// Surface used for Direct3D
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
	CRITICAL_SECTION ddlcs = {};
	m_IDirectDrawX *ddrawParent = nullptr;				// DirectDraw parent device
	std::vector<MIPMAP> MipMaps;						// For MipMaps
	DWORD MaxMipMapLevel = 1;							// For MipMaps
	LPDIRECT3DTEXTURE9 PrimaryDisplayTexture = nullptr;	// Used for the texture surface for the primary surface
	m_IDirectDrawPalette *attachedPalette = nullptr;	// Associated palette
	m_IDirectDrawClipper *attachedClipper = nullptr;	// Associated clipper
	HWND ClipperWindow = nullptr;						// Current clipper
	m_IDirect3DTextureX *attached3DTexture = nullptr;	// Associated texture
	DDSURFACEDESC2 surfaceDesc2 = {};					// Surface description for this surface
	D3DFORMAT surfaceFormat = D3DFMT_UNKNOWN;			// Format for this surface
	DWORD surfaceBitCount = 0;							// Bit count for this surface
	DWORD ResetDisplayFlags = 0;						// Flags that need to be reset when display mode changes
	DWORD UniquenessValue = 0;
	LONG overlayX = 0;
	LONG overlayY = 0;
	DWORD Priority = 0;

	bool Is3DRenderingTarget = false;					// Surface used for Direct3D rendering target, called from m_IDirect3DX::CreateDevice()
	bool Using3D = false;								// Direct3D is being used on top of DirectDraw
	bool DCRequiresEmulation = false;
	bool SurfaceRequiresEmulation = false;
	bool ComplexRoot = false;
	bool IsSurfaceLost = false;
	bool IsInFlip = false;
	bool PresentOnUnlock = false;
	bool IsInDC = false;
	HDC LastDC = nullptr;
	bool IsInBlt = false;
	bool IsInBltBatch = false;
	bool IsLocked = false;
	bool WasBitAlignLocked = false;
	DWORD LockedWithID = 0;								// Thread ID of the current lock
	LASTLOCK LastLock;									// Remember the last lock info
	std::vector<RECT> LockRectList;						// Rects used to lock the surface
	DDRAWEMULATELOCK EmuLock;							// For aligning bits after a lock for games that hard code the pitch
	std::vector<byte> ByteArray;						// Memory used for coping from one surface to the same surface
	std::vector<DDBACKUP> LostDeviceBackup;				// Memory used for backing up the surfaceTexture
	COLORKEY ShaderColorKey;							// Used to store color key array for shader
	SURFACECREATE ShouldEmulate = SC_NOT_CREATED;		// Used to help determine if surface should be emulated

	// Extra Direct3D9 devices used in the primary surface
	D9PRIMARY primary;

	// Real surface and surface data using Direct3D9 devices
	D9SURFACE surface;

	// Direct3D9 device address
	LPDIRECT3DDEVICE9* d3d9Device = nullptr;

	// Store ddraw surface version wrappers
	m_IDirectDrawSurface *WrapperInterface;
	m_IDirectDrawSurface2 *WrapperInterface2;
	m_IDirectDrawSurface3 *WrapperInterface3;
	m_IDirectDrawSurface4 *WrapperInterface4;
	m_IDirectDrawSurface7 *WrapperInterface7;

	// Store a list of attached surfaces
	std::unique_ptr<m_IDirectDrawSurfaceX> BackBufferInterface;
	std::map<DWORD, ATTACHEDMAP> AttachedSurfaceMap;
	DWORD MapKey = 0;

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
	void InitSurface(DWORD DirectXVersion);
	void ReleaseDirectDrawResources();
	void ReleaseSurface();

	// Swap surface addresses for Flip
	template <typename T>
	inline void SwapAddresses(T *Address1, T *Address2)
	{
		T tmpAddr = *Address1;
		*Address1 = *Address2;
		*Address2 = tmpAddr;
	}
	HRESULT CheckBackBufferForFlip(m_IDirectDrawSurfaceX* lpTargetSurface);

	// Direct3D9 interface functions
	HRESULT CheckInterface(char* FunctionName, bool CheckD3DDevice, bool CheckD3DSurface, bool CheckLostSurface);
	HRESULT CreateD3d9Surface();
	bool DoesDCMatch(EMUSURFACE* pEmuSurface);
	void SetEmulationGameDC();
	void UnsetEmulationGameDC();
	HRESULT CreateDCSurface();
	void ReleaseDCSurface();
	void UpdateSurfaceDesc();

	// Direct3D9 interfaces
	inline HRESULT LockD39Surface(D3DLOCKED_RECT* pLockedRect, RECT* pRect, DWORD Flags, DWORD MipMapLevel);
	inline HRESULT UnlockD39Surface(DWORD MipMapLevel);

	// Locking rect coordinates
	bool CheckCoordinates(RECT& OutRect, LPRECT lpInRect, LPDDSURFACEDESC2 lpDDSurfaceDesc2);
	HRESULT LockEmulatedSurface(D3DLOCKED_RECT* pLockedRect, LPRECT lpDestRect);
	void SetDirtyFlag();
	bool CheckRectforSkipScene(RECT& DestRect);
	void BeginWritePresent(bool isSkipScene);
	void EndWritePresent(bool isSkipScene);

	// Surface information functions
	inline bool IsSurfaceLocked() { return IsLocked; }
	inline bool IsSurfaceBlitting() { return (IsInBlt || IsInBltBatch); }
	inline bool IsSurfaceInDC() { return IsInDC; }
	inline bool IsSurfaceBusy() { return (IsSurfaceBlitting() || IsSurfaceLocked() || IsSurfaceInDC()); }
	inline bool IsD9UsingVideoMemory() { return ((surface.Texture && surface.Tex.Pool == D3DPOOL_DEFAULT) ||
		(surface.Surface && surface.SurfacePool == D3DPOOL_DEFAULT)); }
	inline bool IsLockedFromOtherThread() { return (IsSurfaceBlitting() || IsSurfaceLocked()) && LockedWithID && LockedWithID != GetCurrentThreadId(); }
	inline DWORD GetWidth() { return surfaceDesc2.dwWidth; }
	inline DWORD GetHeight() { return surfaceDesc2.dwHeight; }
	inline RECT GetSurfaceRect() { return { 0, 0, (LONG)surfaceDesc2.dwWidth, (LONG)surfaceDesc2.dwHeight }; }
	inline bool CanSurfaceBeDeleted() { return (ComplexRoot || (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_COMPLEX) == 0); }
	inline DDSCAPS2 GetSurfaceCaps() { return surfaceDesc2.ddsCaps; }
	inline D3DFORMAT GetSurfaceFormat() { return surfaceFormat; }
	inline bool CheckSurfaceExists(LPDIRECTDRAWSURFACE7 lpDDSrcSurface) { return
		(ProxyAddressLookupTable.IsValidWrapperAddress((m_IDirectDrawSurface*)lpDDSrcSurface) ||
		ProxyAddressLookupTable.IsValidWrapperAddress((m_IDirectDrawSurface2*)lpDDSrcSurface) ||
		ProxyAddressLookupTable.IsValidWrapperAddress((m_IDirectDrawSurface3*)lpDDSrcSurface) ||
		ProxyAddressLookupTable.IsValidWrapperAddress((m_IDirectDrawSurface4*)lpDDSrcSurface) ||
		ProxyAddressLookupTable.IsValidWrapperAddress((m_IDirectDrawSurface7*)lpDDSrcSurface));
	}

	// Attached surfaces
	void InitSurfaceDesc(DWORD DirectXVersion);
	void AddAttachedSurfaceToMap(m_IDirectDrawSurfaceX* lpSurfaceX, bool MarkAttached = false);
	bool DoesAttachedSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool WasAttachedSurfaceAdded(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool DoesFlipBackBufferExist(m_IDirectDrawSurfaceX* lpSurfaceX);

	// Copying surface textures
	HRESULT SaveDXTDataToDDS(const void* data, size_t dataSize, const char* filename, int dxtVersion) const;
	HRESULT SaveSurfaceToFile(const char* filename, D3DXIMAGE_FILEFORMAT format);
	HRESULT CopySurface(m_IDirectDrawSurfaceX* pSourceSurface, RECT* pSourceRect, RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter, D3DCOLOR ColorKey, DWORD dwFlags);
	HRESULT CopyToDrawTexture(LPRECT lpDestRect);
	HRESULT CopyFromEmulatedSurface(LPRECT lpDestRect);
	HRESULT CopyToEmulatedSurface(LPRECT lpDestRect);
	HRESULT CopyEmulatedPaletteSurface(LPRECT lpDestRect);
	HRESULT CopyEmulatedSurfaceFromGDI(RECT Rect);
	HRESULT CopyEmulatedSurfaceToGDI(RECT Rect);
	HRESULT PresentSurfaceToWindow(RECT Rect);


public:
	m_IDirectDrawSurfaceX(IDirectDrawSurface7 *pOriginal, DWORD DirectXVersion) : ProxyInterface(pOriginal), CreatedVersion(DirectXVersion)
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

		InitSurface(DirectXVersion);
	}
	m_IDirectDrawSurfaceX(m_IDirectDrawX *Interface, DWORD DirectXVersion, LPDDSURFACEDESC2 lpDDSurfaceDesc2) : ddrawParent(Interface), CreatedVersion(DirectXVersion)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		// Copy surface description, needs to run before InitSurface()
		if (lpDDSurfaceDesc2)
		{
			surfaceDesc2 = *lpDDSurfaceDesc2;
		}

		InitSurface(DirectXVersion);
	}
	~m_IDirectDrawSurfaceX()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseSurface();
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, 0); }
	STDMETHOD_(ULONG, AddRef) (THIS) { return AddRef(0); }
	STDMETHOD_(ULONG, Release) (THIS) { return Release(0); }

	/*** IDirectDrawSurface methods ***/
	STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE7);
	STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT);
	HRESULT Blt(LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX, DWORD, bool DontPresentBlt = false);
	STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD, DWORD);
	STDMETHOD(BltFast)(THIS_ DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, DWORD);
	STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD, LPDIRECTDRAWSURFACE7);
	HRESULT GetMipMapSubLevel(LPDIRECTDRAWSURFACE7 FAR* lplpDDAttachedSurface, DWORD MipMapLevel, DWORD DirectXVersion);
	HRESULT EnumAttachedSurfaces(LPVOID, LPDDENUMSURFACESCALLBACK, DWORD, DWORD);
	HRESULT EnumAttachedSurfaces2(LPVOID, LPDDENUMSURFACESCALLBACK7, DWORD, DWORD);
	HRESULT EnumOverlayZOrders(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK, DWORD);
	HRESULT EnumOverlayZOrders2(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7, DWORD);
	STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE7, DWORD, DWORD);
	HRESULT GetAttachedSurface(LPDDSCAPS, LPDIRECTDRAWSURFACE7 FAR *, DWORD, DWORD);
	HRESULT GetAttachedSurface2(LPDDSCAPS2, LPDIRECTDRAWSURFACE7 FAR *, DWORD, DWORD);
	STDMETHOD(GetBltStatus)(THIS_ DWORD);
	HRESULT GetCaps(LPDDSCAPS);
	HRESULT GetCaps2(LPDDSCAPS2);
	STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*);
	STDMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY);
	STDMETHOD(GetDC)(THIS_ HDC FAR *);
	STDMETHOD(GetFlipStatus)(THIS_ DWORD);
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
	STDMETHOD(ReleaseDC)(THIS_ HDC);
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
	STDMETHOD(GetUniquenessValue)(THIS_ LPDWORD);
	STDMETHOD(ChangeUniquenessValue)(THIS);

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
	void SetSurfaceCriticalSection() { EnterCriticalSection(&ddscs); }
	void ReleaseSurfaceCriticalSection() { LeaveCriticalSection(&ddscs); }
	void SetLockCriticalSection() { EnterCriticalSection(&ddlcs); }
	void ReleaseLockCriticalSection() { LeaveCriticalSection(&ddlcs); }

	// Fix byte alignment issue
	void LockEmuLock(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc);
	void UnlockEmuLock();

	// For removing scanlines
	void RestoreScanlines(LASTLOCK &LLock);
	void RemoveScanlines(LASTLOCK &LLock);

	// Functions handling the ddraw parent interface
	inline void SetDdrawParent(m_IDirectDrawX *ddraw) { ddrawParent = ddraw; }
	inline void ClearDdraw() { ddrawParent = nullptr; }

	// Direct3D9 interface functions
	void ReleaseD9ContextSurface();
	void ReleaseD9Surface(bool BackupData, bool ResetSurface);
	bool SetDepthSencil();
	HRESULT PresentSurface(bool isSkipScene);
	void ResetSurfaceDisplay();

	// Surface information functions
	inline bool IsPrimarySurface() { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) != 0; }
	inline bool IsBackBuffer() { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_BACKBUFFER) != 0; }
	inline bool IsPrimaryOrBackBuffer() { return (IsPrimarySurface() || IsBackBuffer()); }
	inline bool isFlipSurface() { return ((surfaceDesc2.ddsCaps.dwCaps & (DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER)) == (DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER)); }
	inline bool IsSurface3D() { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_3DDEVICE) != 0; }
	inline bool IsTexture() { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_TEXTURE) != 0; }
	inline bool IsColorKeyTexture() { return (IsTexture() && (surfaceDesc2.dwFlags & DDSD_CKSRCBLT)); }
	inline bool IsPalette() { return (surfaceFormat == D3DFMT_P8); }
	inline bool IsDepthBuffer() { return (surfaceDesc2.ddpfPixelFormat.dwFlags & (DDPF_ZBUFFER | DDPF_STENCILBUFFER)) != 0; }
	inline bool IsSurfaceManaged() { return (surfaceDesc2.ddsCaps.dwCaps2 & (DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE)) != 0; }
	inline bool IsUsingEmulation() { return (surface.emu && surface.emu->DC && surface.emu->GameDC && surface.emu->pBits); }
	inline bool IsEmulationDCReady() { return (IsUsingEmulation() && !surface.emu->UsingGameDC); }
	inline bool IsRenderingTarget() { return Is3DRenderingTarget; }
	inline bool IsSurfaceDirty() { return surface.IsDirtyFlag; }
	inline DWORD GetD39MipMapLevel(DWORD MipMapLevel) { return min(MipMapLevel, MaxMipMapLevel - 1); }
	bool GetColorKeyForShader(float(&lowColorKey)[4], float(&highColorKey)[4]);
	bool GetColorKeyForPrimaryShader(float(&lowColorKey)[4], float(&highColorKey)[4]);
	bool GetWasBitAlignLocked() { return WasBitAlignLocked; }
	inline bool GetSurfaceSetSize(DWORD& Width, DWORD& Height)
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
	inline void AttachD9BackBuffer() { Is3DRenderingTarget = true; }
	inline void DetachD9BackBuffer() { Is3DRenderingTarget = false; }
	LPDIRECT3DTEXTURE9 Get3DDrawTexture();
	LPDIRECT3DSURFACE9 Get3DSurface();
	LPDIRECT3DTEXTURE9 Get3DTexture();
	LPDIRECT3DTEXTURE9 Get3DPaletteTexture() { return primary.PaletteTexture; }
	LPDIRECT3DSURFACE9 GetD3D9Surface();
	LPDIRECT3DTEXTURE9 GetD3D9Texture();
	inline m_IDirect3DTextureX* GetAttachedTexture() { return attached3DTexture; }
	inline void ClearAttachedTexture() { attached3DTexture = nullptr; }
	void ClearDirtyFlags();

	// Draw 2D DirectDraw surface
	HRESULT ColorFill(RECT* pRect, D3DCOLOR dwFillColor);

	// Attached surfaces
	void RemoveAttachedSurfaceFromMap(m_IDirectDrawSurfaceX* lpSurfaceX);

	// For clipper
	void RemoveClipper(m_IDirectDrawClipper* ClipperToRemove);

	// For palettes
	inline m_IDirectDrawPalette *GetAttachedPalette() { return attachedPalette; }
	void RemovePalette(m_IDirectDrawPalette* PaletteToRemove);
	void UpdatePaletteData();

	// For emulated surfaces
	static void StartSharedEmulatedMemory();
	static void DeleteEmulatedMemory(EMUSURFACE **ppEmuSurface);
	static void CleanupSharedEmulatedMemory();
};
