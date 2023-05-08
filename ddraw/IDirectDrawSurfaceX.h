#pragma once

#include <map>
#include "d3dx9.h"

#define BLT_MIRRORLEFTRIGHT		0x00000002l
#define BLT_MIRRORUPDOWN		0x00000004l
#define BLT_COLORKEY			0x00002000l

// Emulated surface
struct EMUSURFACE
{
	HDC surfaceDC = nullptr;
	DWORD surfaceSize = 0;
	void *surfacepBits = nullptr;
	DWORD surfacePitch = 0;
	HBITMAP bitmap = nullptr;
	BYTE bmiMemory[(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256)] = {};
	PBITMAPINFO bmi = (PBITMAPINFO)bmiMemory;
	HGDIOBJ OldDCObject = nullptr;
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

	struct DDRAWEMULATELOCK
	{
		bool Locked = false;
		std::vector<byte> surfaceMem;
		void *Addr = nullptr;
		DWORD Pitch = 0;
		DWORD BBP = 0;
		DWORD Height = 0;
		DWORD Width = 0;
	} EmuLock;

	struct DSWRAPPER
	{
		DWORD Height = 0;
		DWORD Width = 0;
	} DsWrapper;

	struct LASTLOCK
	{
		bool bEvenScanlines = false;
		bool bOddScanlines = false;
		bool ReadOnly = false;
		bool isSkipScene = false;
		DWORD ScanlineWidth = 0;
		std::vector<BYTE> EvenScanLine;
		std::vector<BYTE> OddScanLine;
		RECT Rect = {};
		D3DLOCKED_RECT LockedRect = {};
	} LastLock;

	// Convert to Direct3D9
	bool IsDirect3DSurface = false;
	m_IDirectDrawX *ddrawParent = nullptr;
	m_IDirectDrawPalette *attachedPalette = nullptr;	// Associated palette
	m_IDirectDrawClipper *attachedClipper = nullptr;	// Associated clipper
	m_IDirect3DTextureX *attachedTexture = nullptr;		// Associated texture
	DDSURFACEDESC2 surfaceDesc2 = {};					// Surface description for this surface
	D3DFORMAT surfaceFormat = D3DFMT_UNKNOWN;			// Format for this surface
	DWORD surfaceBitCount = 0;							// Bit count for this surface
	DWORD ResetDisplayFlags = 0;						// Flags that need to be reset when display mode changes
	std::vector<byte> surfaceArray;						// Memory used for coping from one surface to the same surface
	std::vector<byte> surfaceBackup;					// Memory used for backing up the surfaceTexture
	std::vector<RECT> surfaceLockRectList;				// Rects used to lock the surface
	EMUSURFACE *emu = nullptr;
	LONG overlayX = 0;
	LONG overlayY = 0;
	DWORD Priority = 0;
	DWORD MaxLOD = 0;
	DWORD UniquenessValue = 0;
	bool DCRequiresEmulation = false;
	bool ComplexRoot = false;
	bool PresentOnUnlock = false;
	bool IsLocked = false;
	DWORD LockedWithID = 0;
	bool IsInDC = false;
	HDC LastDC = nullptr;
	bool IsInBlt = false;
	bool IsInFlip = false;
	bool DirtyFlip = false;								// Dirty flip flag indicates that surface needs to be cleared before flipping
	DWORD PaletteUSN = (DWORD)this;						// The USN thats used to see if the palette data was updated
	DWORD LastPaletteUSN = 0;							// The USN that was used last time the palette was updated
	bool PaletteFirstRun = true;
	bool ClipperFirstRun = true;

	// Direct3D9 vars
	LPDIRECT3DDEVICE9 *d3d9Device = nullptr;			// Direct3D9 Device
	LPDIRECT3DSURFACE9 surface3D = nullptr;				// Surface used for Direct3D
	LPDIRECT3DTEXTURE9 surfaceTexture = nullptr;		// Main surface texture used for locks, Blts and Flips
	LPDIRECT3DSURFACE9 contextSurface = nullptr;		// Main surface texture used for locks, Blts and Flips
	LPDIRECT3DTEXTURE9 paletteTexture = nullptr;		// Extra surface texture used for storing palette entries for the pixel shader
	LPDIRECT3DPIXELSHADER9 pixelShader = nullptr;		// Used with palette surfaces to display proper palette data on the surface texture
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer = nullptr;		// Vertex buffer used to stretch the texture accross the screen

	// Store ddraw surface version wrappers
	m_IDirectDrawSurface *WrapperInterface;
	m_IDirectDrawSurface2 *WrapperInterface2;
	m_IDirectDrawSurface3 *WrapperInterface3;
	m_IDirectDrawSurface4 *WrapperInterface4;
	m_IDirectDrawSurface7 *WrapperInterface7;

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

	const DWORD TLVERTEXFVF = (D3DFVF_XYZRHW | D3DFVF_TEX1);

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
	void ReleaseSurface();

	// Swap surface addresses for Flip
	template <typename T>
	inline void SwapAddresses(T *Address1, T *Address2)
	{
		T tmpAddr = *Address1;
		*Address1 = *Address2;
		*Address2 = tmpAddr;
	}
	void SwapSurface(m_IDirectDrawSurfaceX* lpTargetSurface1, m_IDirectDrawSurfaceX* lpTargetSurface2);
	HRESULT FlipBackBuffer();

	// Direct3D9 interface functions
	HRESULT CheckInterface(char *FunctionName, bool CheckD3DDevice, bool CheckD3DSurface);
	HRESULT CreateD3d9Surface();
	bool DoesDCMatch(EMUSURFACE* emu);
	HRESULT CreateDCSurface();
	void UpdateSurfaceDesc();

	// Direct3D9 interfaces
	inline EMUSURFACE **GetEmulatedSurface() { return &emu; }
	inline LPDIRECT3DSURFACE9 *GetSurface3D() { return &surface3D; }
	inline LPDIRECT3DTEXTURE9 *GetSurfaceTexture() { return &surfaceTexture; }
	inline LPDIRECT3DSURFACE9 *GetContextSurface() { return &contextSurface; }
	inline HRESULT LockD39Surface(D3DLOCKED_RECT* pLockedRect, RECT* pRect, DWORD Flags);
	inline HRESULT UnlockD39Surface();

	// Locking rect coordinates
	bool CheckCoordinates(LPRECT lpOutRect, LPRECT lpInRect);
	HRESULT LockEmulatedSurface(D3DLOCKED_RECT* pLockedRect, LPRECT lpDestRect);
	void SetDirtyFlag();
	bool CheckRectforSkipScene(RECT& DestRect);
	void BeginWritePresent(bool isSkipScene);
	void EndWritePresent(bool isSkipScene);

	// Surface information functions
	inline bool IsSurfaceLocked() { return IsLocked; }
	inline bool IsSurfaceInDC() { return IsInDC; }
	inline bool CanSurfaceBeDeleted() { return (ComplexRoot || (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_COMPLEX) == 0); }
	inline DWORD GetWidth() { return surfaceDesc2.dwWidth; }
	inline DWORD GetHeight() { return surfaceDesc2.dwHeight; }
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
	HRESULT ColorFill(RECT* pRect, D3DCOLOR dwFillColor);
	HRESULT SaveDXTDataToDDS(const void* data, size_t dataSize, const char* filename, int dxtVersion) const;
	HRESULT SaveSurfaceToFile(const char* filename, D3DXIMAGE_FILEFORMAT format);
	HRESULT CopySurface(m_IDirectDrawSurfaceX* pSourceSurface, RECT* pSourceRect, RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter, DDCOLORKEY ColorKey, DWORD dwFlags);
	HRESULT CopyFromEmulatedSurface(LPRECT lpDestRect);
	HRESULT CopyToEmulatedSurface(LPRECT lpDestRect);
	HRESULT CopyEmulatedSurfaceFromGDI(RECT Rect);
	HRESULT CopyEmulatedSurfaceToGDI(RECT Rect);

public:
	m_IDirectDrawSurfaceX(IDirectDrawSurface7 *pOriginal, DWORD DirectXVersion) : ProxyInterface(pOriginal)
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
	m_IDirectDrawSurfaceX(m_IDirectDrawX *Interface, DWORD DirectXVersion, LPDDSURFACEDESC2 lpDDSurfaceDesc2) : ddrawParent(Interface)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		// Copy surface description, needs to run before InitSurface()
		if (lpDDSurfaceDesc2)
		{
			surfaceDesc2.dwSize = sizeof(DDSURFACEDESC2);
			ConvertSurfaceDesc(surfaceDesc2, *lpDDSurfaceDesc2);
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
	HRESULT Blt(LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX, bool isSkipScene = false);
	STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD);
	STDMETHOD(BltFast)(THIS_ DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD);
	STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD, LPDIRECTDRAWSURFACE7);
	HRESULT EnumAttachedSurfaces(LPVOID, LPDDENUMSURFACESCALLBACK, DWORD);
	HRESULT EnumAttachedSurfaces2(LPVOID, LPDDENUMSURFACESCALLBACK7, DWORD);
	HRESULT EnumOverlayZOrders(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK, DWORD);
	HRESULT EnumOverlayZOrders2(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7, DWORD);
	STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE7, DWORD, DWORD);
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
	HRESULT Lock(LPRECT, LPDDSURFACEDESC, DWORD, HANDLE, DWORD);
	HRESULT Lock2(LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE, DWORD);
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

	// Helper functions
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
	ULONG AddRef(DWORD DirectXVersion);
	ULONG Release(DWORD DirectXVersion);

	// Fix byte alignment issue
	template <class T>
	void LockBitAlign(LPRECT lpDestRect, T lpDDSurfaceDesc);

	// For removing scanlines
	void RestoreScanlines(LASTLOCK &LLock);
	void RemoveScanlines(LASTLOCK &LLock);

	// Functions handling the ddraw parent interface
	inline void SetDdrawParent(m_IDirectDrawX *ddraw) { ddrawParent = ddraw; }
	inline void ClearDdraw() { ddrawParent = nullptr; }

	// Direct3D9 interface functions
	void ReleaseD9Surface(bool BackupData);
	HRESULT PresentSurface(bool isSkipScene);
	void ResetSurfaceDisplay();

	// Surface information functions
	inline bool IsPrimarySurface() { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) != 0; }
	inline bool IsBackBuffer() { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_BACKBUFFER) != 0; }
	inline bool IsSurface3D() { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_3DDEVICE) != 0; }
	inline bool IsTexture() { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_TEXTURE) != 0; }
	inline bool IsDepthBuffer() { return (surfaceDesc2.ddpfPixelFormat.dwFlags & (DDPF_ZBUFFER | DDPF_STENCILBUFFER)) != 0; }
	inline bool IsSurfaceManaged() { return (surfaceDesc2.ddsCaps.dwCaps2 & (DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE)) != 0; }
	inline bool IsUsingEmulation() { return (emu && emu->surfaceDC && emu->surfacepBits); }
	LPDIRECT3DSURFACE9 Get3DSurface();
	LPDIRECT3DTEXTURE9 Get3DTexture();
	LPDIRECT3DSURFACE9 GetD3D9Surface();
	inline void ClearTexture() { attachedTexture = nullptr; }
	inline void SetWrapperSurfaceSize(DWORD Width, DWORD Height) { DsWrapper.Width = Width; DsWrapper.Height = Height; }

	// Attached surfaces
	void SetDirtyFlipFlag();
	void RemoveAttachedSurfaceFromMap(m_IDirectDrawSurfaceX* lpSurfaceX);

	// For clipper
	void RemoveClipper(m_IDirectDrawClipper* ClipperToRemove);

	// For palettes
	inline m_IDirectDrawPalette *GetAttachedPalette() { return attachedPalette; }
	inline DWORD GetPaletteUSN() { return PaletteUSN; }
	void RemovePalette(m_IDirectDrawPalette* PaletteToRemove);
	void UpdatePaletteData();

	// For emulated surfaces
	static void StartSharedEmulatedMemory();
	static void DeleteEmulatedMemory(EMUSURFACE **ppEmuSurface);
	static void CleanupSharedEmulatedMemory();
};
