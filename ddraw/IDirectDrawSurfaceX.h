#pragma once

#include <map>

// Emulated surface
struct EMUSURFACE
{
	HDC surfaceDC = nullptr;
	DWORD surfaceSize = 0;
	void *surfacepBits = nullptr;
	DWORD surfacePitch = 0;
	HBITMAP bitmap = nullptr;
	BYTE bmiMemory[(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256)];
	PBITMAPINFO bmi = (PBITMAPINFO)bmiMemory;
	HGDIOBJ OldDCObject = nullptr;
};

class m_IDirectDrawSurfaceX : public IUnknown
{
private:
	IDirectDrawSurface7 *ProxyInterface = nullptr;
	m_IDirectDrawSurface7 *WrapperInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount = 1;

	// Color Key Structure
	struct CKEYS
	{
		DDCOLORKEY Key = { 0, 0 };
		bool IsSet = false;
		bool IsColorSpace = false;
	};

	// Used for 24-bit surfaces
	struct TRIBYTE
	{
		BYTE first;
		BYTE second;
		BYTE third;
	};

	// Convert to Direct3D9
	m_IDirectDrawX *ddrawParent = nullptr;
	m_IDirectDrawPalette *attachedPalette = nullptr;	// Associated palette
	m_IDirectDrawClipper *attachedClipper = nullptr;	// Associated clipper
	DDSURFACEDESC2 surfaceDesc2 = { NULL };				// Surface description for this surface
	D3DFORMAT surfaceFormat = D3DFMT_UNKNOWN;			// Format for this surface
	DWORD surfaceBitCount = 0;							// Bit count for this surface
	CRITICAL_SECTION ddscs;								// Critical section for surfaceArray
	std::vector<byte> surfaceArray;						// Memory used for coping from one surface to the same surface
	std::vector<byte> surfaceBackup;					// Memory used for backing up the surfaceTexture
	EMUSURFACE *emu = nullptr;
	LONG overlayX = 0;
	LONG overlayY = 0;
	DWORD Priority = 0;
	DWORD MaxLOD = 0;
	DWORD UniquenessValue = 0;
	bool IsSurfaceEmulated = false;
	bool DCRequiresEmulation = false;
	bool ComplexRoot = false;
	bool PresentOnUnlock = false;
	bool IsLocked = false;
	DWORD LockThreadID = 0;
	bool IsInDC = false;
	bool IsInBlt = false;
	bool IsInFlip = false;
	DWORD PaletteUSN = (DWORD)this;		// The USN thats used to see if the palette data was updated
	DWORD LastPaletteUSN = 0;			// The USN that was used last time the palette was updated
	bool PaletteFirstRun = true;
	bool ClipperFirstRun = true;
	bool RestoreDisplayFlags = false;
	bool DoCopyRect = false;
	RECT LastRect = { 0, 0, 0, 0 };

	// Display resolution
	DWORD displayWidth = 0;								// Width used to override the default application set width
	DWORD displayHeight = 0;							// Height used to override the default application set height

	// Direct3D9 vars
	LPDIRECT3DDEVICE9 *d3d9Device = nullptr;			// Direct3D9 Device
	LPDIRECT3DTEXTURE9 surfaceTexture = nullptr;		// Main surface texture used for locks, Blts and Flips
	LPDIRECT3DSURFACE9 contextSurface = nullptr;		// Main surface texture used for locks, Blts and Flips
	LPDIRECT3DTEXTURE9 displayTexture = nullptr;		// Surface texture used for displaying image
	LPDIRECT3DTEXTURE9 paletteTexture = nullptr;		// Extra surface texture used for storing palette entries for the pixel shader
	LPDIRECT3DPIXELSHADER9 pixelShader = nullptr;		// Used with palette surfaces to display proper palette data on the surface texture
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer = nullptr;		// Vertex buffer used to stretch the texture accross the screen

	// Store ddraw surface version wrappers
	std::unique_ptr<m_IDirectDrawSurface> UniqueProxyInterface = nullptr;
	std::unique_ptr<m_IDirectDrawSurface2> UniqueProxyInterface2 = nullptr;
	std::unique_ptr<m_IDirectDrawSurface3> UniqueProxyInterface3 = nullptr;
	std::unique_ptr<m_IDirectDrawSurface4> UniqueProxyInterface4 = nullptr;
	std::unique_ptr<m_IDirectDrawSurface7> UniqueProxyInterface7 = nullptr;

	// Store a list of attached surfaces
	std::unique_ptr<m_IDirectDrawSurfaceX> BackBufferInterface;
	struct ATTACHEDMAP
	{
		m_IDirectDrawSurfaceX* pSurface = nullptr;
		bool isAttachedSurfaceAdded = false;
	};
	std::map<DWORD, ATTACHEDMAP> AttachedSurfaceMap;
	DWORD MapKey = 0;

	// Custom vertex
	struct TLVERTEX
	{
		float x;
		float y;
		float z;
		float rhw;
		float u;
		float v;
	};

public:
	m_IDirectDrawSurfaceX(IDirectDrawSurface7 *pOriginal, DWORD DirectXVersion, m_IDirectDrawSurface7 *Interface) : ProxyInterface(pOriginal), WrapperInterface(Interface)
	{
		ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		if (ProxyDirectXVersion != DirectXVersion)
		{
			LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")" << " converting device from v" << DirectXVersion << " to v" << ProxyDirectXVersion);
		}
		else
		{
			LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ") v" << DirectXVersion);
		}
	}
	m_IDirectDrawSurfaceX(LPDIRECT3DDEVICE9 *lplpDevice, m_IDirectDrawX *Interface, DWORD DirectXVersion, LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD Width, DWORD Height) :
		d3d9Device(lplpDevice), ddrawParent(Interface), displayWidth(Width), displayHeight(Height)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")" << " converting device from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		// Store surface
		if (ddrawParent)
		{
			ddrawParent->AddSurfaceToVector(this);
		}

		// Initialize Critical Section for surface array
		InitializeCriticalSection(&ddscs);

		// Update surface description and create backbuffers
		InitSurfaceDesc(lpDDSurfaceDesc2, DirectXVersion);
	}
	~m_IDirectDrawSurfaceX()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting device!");

		if (Config.Dd7to9 && !Config.Exiting)
		{
			ReleaseInterface();
			ReleaseD9Surface(false);

			// Delete Critical Section for surface array
			DeleteCriticalSection(&ddscs);
		}
	}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return IID_IUnknown; }
	IDirectDrawSurface7 *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDrawSurface7 *GetWrapperInterface() { return WrapperInterface; }

	/*** IUnknown methods ***/
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, ProxyDirectXVersion); }
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IDirectDrawSurface methods ***/
	STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE7);
	STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT);
	HRESULT Blt(LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX, BOOL isSkipScene = false);
	STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD);
	STDMETHOD(BltFast)(THIS_ DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD);
	STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD, LPDIRECTDRAWSURFACE7);
	HRESULT EnumAttachedSurfaces(LPVOID, LPDDENUMSURFACESCALLBACK, DWORD);
	HRESULT EnumAttachedSurfaces2(LPVOID, LPDDENUMSURFACESCALLBACK7, DWORD);
	HRESULT EnumOverlayZOrders(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK, DWORD);
	HRESULT EnumOverlayZOrders2(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7, DWORD);
	STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE7, DWORD);
	HRESULT GetAttachedSurface(LPDDSCAPS, LPDIRECTDRAWSURFACE7 FAR *, DWORD);
	HRESULT GetAttachedSurface2(LPDDSCAPS2, LPDIRECTDRAWSURFACE7 FAR *, DWORD);
	STDMETHOD(GetBltStatus)(THIS_ DWORD);
	HRESULT m_IDirectDrawSurfaceX::GetCaps(LPDDSCAPS);
	HRESULT m_IDirectDrawSurfaceX::GetCaps2(LPDDSCAPS2);
	STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*);
	STDMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY);
	STDMETHOD(GetDC)(THIS_ HDC FAR *);
	STDMETHOD(GetFlipStatus)(THIS_ DWORD);
	STDMETHOD(GetOverlayPosition)(THIS_ LPLONG, LPLONG);
	STDMETHOD(GetPalette)(THIS_ LPDIRECTDRAWPALETTE FAR*);
	STDMETHOD(GetPixelFormat)(THIS_ LPDDPIXELFORMAT);
	HRESULT GetSurfaceDesc(LPDDSURFACEDESC);
	HRESULT GetSurfaceDesc2(LPDDSURFACEDESC2);
	HRESULT Initialize(LPDIRECTDRAW, LPDDSURFACEDESC);
	HRESULT Initialize2(LPDIRECTDRAW, LPDDSURFACEDESC2);
	STDMETHOD(IsLost)(THIS);
	HRESULT Lock(LPRECT, LPDDSURFACEDESC, DWORD, HANDLE);
	HRESULT Lock2(LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE);
	STDMETHOD(ReleaseDC)(THIS_ HDC);
	STDMETHOD(Restore)(THIS);
	STDMETHOD(SetClipper)(THIS_ LPDIRECTDRAWCLIPPER);
	STDMETHOD(SetColorKey)(THIS_ DWORD, LPDDCOLORKEY);
	STDMETHOD(SetOverlayPosition)(THIS_ LONG, LONG);
	STDMETHOD(SetPalette)(THIS_ LPDIRECTDRAWPALETTE);
	STDMETHOD(Unlock)(THIS_ LPRECT);
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

	// Wrapper interface functions
	REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirectDrawSurface :
			(DirectXVersion == 2) ? IID_IDirectDrawSurface2 :
			(DirectXVersion == 3) ? IID_IDirectDrawSurface3 :
			(DirectXVersion == 4) ? IID_IDirectDrawSurface4 :
			(DirectXVersion == 7) ? IID_IDirectDrawSurface7 : IID_IUnknown;
	}
	IDirectDrawSurface *GetProxyInterfaceV1() { return (IDirectDrawSurface *)ProxyInterface; }
	IDirectDrawSurface2 *GetProxyInterfaceV2() { return (IDirectDrawSurface2 *)ProxyInterface; }
	IDirectDrawSurface3 *GetProxyInterfaceV3() { return (IDirectDrawSurface3 *)ProxyInterface; }
	IDirectDrawSurface4 *GetProxyInterfaceV4() { return (IDirectDrawSurface4 *)ProxyInterface; }
	IDirectDrawSurface7 *GetProxyInterfaceV7() { return ProxyInterface; }
	void *GetWrapperInterfaceX(DWORD DirectXVersion);

	// Functions handling the ddraw parent interface
	void SetDdrawParent(m_IDirectDrawX *ddraw) { ddrawParent = ddraw; }
	void ClearDdraw() { ddrawParent = nullptr; }

	// Direct3D9 interfaces
	EMUSURFACE **GetEmulatedSurface() { return &emu; }
	LPDIRECT3DTEXTURE9 *GetSurfaceTexture() { return &surfaceTexture; }
	LPDIRECT3DSURFACE9 *GetContextSurface() { return &contextSurface; }

	// Surface information functions
	bool IsPrimarySurface() { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) != 0; }
	bool IsSurfaceLocked() { return IsLocked; }
	bool IsSurfaceInDC() { return IsInDC; }
	bool IsSurfaceManaged() { return (surfaceDesc2.ddsCaps.dwCaps2 & (DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE)) != 0; }
	bool CanSurfaceBeDeleted() { return (ComplexRoot || (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_COMPLEX) == 0); }
	DWORD GetWidth() { return surfaceDesc2.dwWidth; }
	DWORD GetHeight() { return surfaceDesc2.dwHeight; }
	DDSCAPS2 GetSurfaceCaps() { return surfaceDesc2.ddsCaps; }
	D3DFORMAT GetSurfaceFormat() { return surfaceFormat; }

	// Direct3D9 interface functions
	HRESULT CheckInterface(char *FunctionName, bool CheckD3DDevice, bool CheckD3DSurface);
	HRESULT CreateD3d9Surface();
	HRESULT CreateDCSurface();
	void UpdateSurfaceDesc();
	template <typename T>
	void ReleaseD9Interface(T **ppInterface);
	void ReleaseD9Surface(bool BackupData = true);
	HRESULT PresentSurface(BOOL isSkipScene = false);
	void RestoreSurfaceDisplay();

	// Swap surface addresses for Flip
	template <typename T>
	void SwapAddresses(T *Address1, T *Address2)
	{
		T tmpAddr = *Address1;
		*Address1 = *Address2;
		*Address2 = tmpAddr;
	}
	void SwapSurface(m_IDirectDrawSurfaceX* lpTargetSurface1, m_IDirectDrawSurfaceX* lpTargetSurface2);
	HRESULT FlipBackBuffer();

	// Locking rect coordinates
	bool CheckCoordinates(LPRECT lpOutRect, LPRECT lpInRect);
	bool WaitForLockState();
	HRESULT SetLock(D3DLOCKED_RECT* pLockedRect, LPRECT lpDestRect, DWORD dwFlags, BOOL isSkipScene = false);
	HRESULT SetUnlock(BOOL isSkipScene = false);
	HRESULT LockEmulatedSurface(D3DLOCKED_RECT* pLockedRect, LPRECT lpDestRect);
	void SetDirtyFlag();
	void BeginWritePresent(bool isSkipScene = false);
	void EndWritePresent(bool isSkipScene = false);

	// Attached surfaces
	void InitSurfaceDesc(LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD DirectXVersion);
	void AddAttachedSurfaceToMap(m_IDirectDrawSurfaceX* lpSurfaceX, bool MarkAttached = false);
	void RemoveAttachedSurfaceFromMap(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool DoesAttachedSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool WasAttachedSurfaceAdded(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool DoesFlipBackBufferExist(m_IDirectDrawSurfaceX* lpSurfaceX);

	// Copying surface textures
	HRESULT ColorFill(RECT* pRect, D3DCOLOR dwFillColor);
	HRESULT CopySurface(m_IDirectDrawSurfaceX* pSourceSurface, RECT* pSourceRect, RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter, DDCOLORKEY ColorKey, DWORD dwFlags);
	HRESULT CopyEmulatedSurface(LPRECT lpDestRect, bool CopyToRealSurfaceTexture);

	// For palettes
	m_IDirectDrawPalette *GetAttachedPalette() { return attachedPalette; }
	void UpdatePaletteData();
	DWORD GetPaletteUSN() { return PaletteUSN; }

	// For emulated surfaces
	static void StartSharedEmulatedMemory();
	static void DeleteSharedEmulatedMemory(EMUSURFACE **ppEmuSurface);
	static void CleanupSharedEmulatedMemory();

	// Release interface
	void ReleaseInterface();
};
