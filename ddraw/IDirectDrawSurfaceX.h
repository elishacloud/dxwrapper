#pragma once

#include <map>

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

	// Convert to Direct3D9
	m_IDirectDrawX *ddrawParent = nullptr;
	m_IDirectDrawPalette *attachedPalette = nullptr;	// Associated palette
	m_IDirectDrawClipper *attachedClipper = nullptr;	// Associated clipper
	DDSURFACEDESC2 surfaceDesc2 = { NULL };				// Surface description for this surface
	std::vector<byte> surfaceArray;						// Memory used for coping from one surface to the same surface
	CKEYS ColorKeys[4];									// Color keys (0 = DDCKEY_DESTBLT, 1 = DDCKEY_DESTOVERLAY, 2 = DDCKEY_SRCBLT, 3 = DDCKEY_SRCOVERLAY)
	LONG overlayX = 0;
	LONG overlayY = 0;
	DWORD UniquenessValue = 0;
	HDC surfacehDC = nullptr;
	bool dirtyFlag = false;
	bool EndSceneLock = false;
	bool IsLocked = false;
	bool IsInDC = false;
	bool NewPalette = false;
	bool PaletteFirstRun = true;
	bool ClipperFirstRun = true;

	// Display resolution
	DWORD displayWidth = 0;								// Width used to override the default application set width
	DWORD displayHeight = 0;							// Height used to override the default application set height

	// Direct3D9 vars
	LPDIRECT3DDEVICE9 *d3d9Device = nullptr;			// Direct3D9 Device
	LPDIRECT3DSURFACE9 surfaceInterface = nullptr;		// Main surface used for GetDC and ReleaseDC
	LPDIRECT3DTEXTURE9 surfaceTexture = nullptr;		// Main surface texture used for locks, Blts and Flips
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
	std::map<DWORD, m_IDirectDrawSurfaceX*> AttachedSurfaceMap;
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

	static constexpr DWORD rgbPalette[] =
	{	0x000000, 0x0000a8, 0x00a800, 0x00a8a8, 0xa80000, 0xa800a8, 0xa85400, 0xa8a8a8, 0x545454, 0x5454fc, 0x54fc54, 0x54fcfc, 0xfc5454, 0xfc54fc, 0xfcfc54, 0xfcfcfc,
		0x000000, 0x141414, 0x202020, 0x2c2c2c, 0x383838, 0x444444, 0x505050, 0x606060, 0x707070, 0x808080, 0x909090, 0xa0a0a0, 0xb4b4b4, 0xc8c8c8, 0xe0e0e0, 0xfcfcfc,
		0x0000fc, 0x4000fc, 0x7c00fc, 0xbc00fc, 0xfc00fc, 0xfc00bc, 0xfc007c, 0xfc0040, 0xfc0000, 0xfc4000, 0xfc7c00, 0xfcbc00, 0xfcfc00, 0xbcfc00, 0x7cfc00, 0x40fc00,
		0x00fc00, 0x00fc40, 0x00fc7c, 0x00fcbc, 0x00fcfc, 0x00bcfc, 0x007cfc, 0x0040fc, 0x7c7cfc, 0x9c7cfc, 0xbc7cfc, 0xdc7cfc, 0xfc7cfc, 0xfc7cdc, 0xfc7cbc, 0xfc7c9c,
		0xfc7c7c, 0xfc9c7c, 0xfcbc7c, 0xfcdc7c, 0xfcfc7c, 0xdcfc7c, 0xbcfc7c, 0x9cfc7c, 0x7cfc7c, 0x7cfc9c, 0x7cfcbc, 0x7cfcdc, 0x7cfcfc, 0x7cdcfc, 0x7cbcfc, 0x7c9cfc,
		0xb4b4fc, 0xc4b4fc, 0xd8b4fc, 0xe8b4fc, 0xfcb4fc, 0xfcb4e8, 0xfcb4d8, 0xfcb4c4, 0xfcb4b4, 0xfcc4b4, 0xfcd8b4, 0xfce8b4, 0xfcfcb4, 0xe8fcb4, 0xd8fcb4, 0xc4fcb4,
		0xb4fcb4, 0xb4fcc4, 0xb4fcd8, 0xb4fce8, 0xb4fcfc, 0xb4e8fc, 0xb4d8fc, 0xb4c4fc, 0x000070, 0x1c0070, 0x380070, 0x540070, 0x700070, 0x700054, 0x700038, 0x70001c,
		0x700000, 0x701c00, 0x703800, 0x705400, 0x707000, 0x547000, 0x387000, 0x1c7000, 0x007000, 0x00701c, 0x007038, 0x007054, 0x007070, 0x005470, 0x003870, 0x001c70,
		0x383870, 0x443870, 0x543870, 0x603870, 0x703870, 0x703860, 0x703854, 0x703844, 0x703838, 0x704438, 0x705438, 0x706038, 0x707038, 0x607038, 0x547038, 0x447038,
		0x387038, 0x387044, 0x387054, 0x387060, 0x387070, 0x386070, 0x385470, 0x384470, 0x505070, 0x585070, 0x605070, 0x685070, 0x705070, 0x705068, 0x705060, 0x705058,
		0x705050, 0x705850, 0x706050, 0x706850, 0x707050, 0x687050, 0x607050, 0x587050, 0x507050, 0x507058, 0x507060, 0x507068, 0x507070, 0x506870, 0x506070, 0x505870,
		0x000040, 0x100040, 0x200040, 0x300040, 0x400040, 0x400030, 0x400020, 0x400010, 0x400000, 0x401000, 0x402000, 0x403000, 0x404000, 0x304000, 0x204000, 0x104000,
		0x004000, 0x004010, 0x004020, 0x004030, 0x004040, 0x003040, 0x002040, 0x001040, 0x202040, 0x282040, 0x302040, 0x382040, 0x402040, 0x402038, 0x402030, 0x402028,
		0x402020, 0x402820, 0x403020, 0x403820, 0x404020, 0x384020, 0x304020, 0x284020, 0x204020, 0x204028, 0x204030, 0x204038, 0x204040, 0x203840, 0x203040, 0x202840,
		0x2c2c40, 0x302c40, 0x342c40, 0x3c2c40, 0x402c40, 0x402c3c, 0x402c34, 0x402c30, 0x402c2c, 0x40302c, 0x40342c, 0x403c2c, 0x40402c, 0x3c402c, 0x34402c, 0x30402c,
		0x2c402c, 0x2c4030, 0x2c4034, 0x2c403c, 0x2c4040, 0x2c3c40, 0x2c3440, 0x2c3040, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000 };

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

		// Store surface
		if (ddrawParent)
		{
			ddrawParent->AddSurfaceToVector(this);
		}

		// Update surface description and create backbuffers
		InitSurfaceDesc(lpDDSurfaceDesc2, DirectXVersion);

		LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")" << " converting device from v" << DirectXVersion << " to v" << ProxyDirectXVersion);
	}
	~m_IDirectDrawSurfaceX()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting device!");

		if (Config.Dd7to9 && !Config.Exiting)
		{
			ReleaseInterface();
			ReleaseD9Surface(false);
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
	void ClearDdraw() { ddrawParent = nullptr; }
	void SetDdrawParent(m_IDirectDrawX *ddraw) { ddrawParent = ddraw; }

	// Direct3D9 interfaces
	LPDIRECT3DTEXTURE9 *GetSurfaceTexture() { return &surfaceTexture; }
	LPDIRECT3DSURFACE9 *GetSurfaceInterface() { return &surfaceInterface; }

	// Surface information functions
	bool IsPrimarySurface() { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) != 0; }
	bool IsSurfaceLocked() { return IsLocked; }
	bool IsSurfaceInDC() { return IsInDC; }
	bool IsSurfaceManaged() { return (surfaceDesc2.ddsCaps.dwCaps2 & (DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE)); }
	DWORD GetWidth() { return surfaceDesc2.dwWidth; }
	DWORD GetHeight() { return surfaceDesc2.dwHeight; }
	DDSCAPS2 GetSurfaceCaps() { return surfaceDesc2.ddsCaps; }
	DWORD GetSurfaceBitCount() { return GetBitCount(surfaceDesc2.ddpfPixelFormat); }
	D3DFORMAT GetSurfaceFormat() { return GetDisplayFormat(surfaceDesc2.ddpfPixelFormat); }

	// Direct3D9 interface functions
	HRESULT CheckInterface(char *FunctionName, bool CheckD3DDevice, bool CheckD3DSurface);
	HRESULT CreateD3d9Surface();
	template <typename T>
	void ReleaseD9Interface(T **ppInterface);
	void ReleaseD9Surface(bool BackupData = true);
	HRESULT PresentSurface(BOOL isSkipScene = false);

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
	HRESULT SetLock(D3DLOCKED_RECT* pLockedRect, LPRECT lpDestRect, DWORD dwFlags, BOOL isSkipScene = false);
	HRESULT SetUnlock(BOOL isSkipScene = false);

	// Attached surfaces
	void InitSurfaceDesc(LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD DirectXVersion);
	void AddAttachedSurfaceToMap(m_IDirectDrawSurfaceX* lpSurfaceX);
	void RemoveAttachedSurfaceFromMap(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool DoesAttachedSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX);

	// Copying surface textures
	HRESULT ColorFill(RECT* pRect, D3DCOLOR dwFillColor);
	HRESULT UpdateSurface(m_IDirectDrawSurfaceX* pSourceSurface, RECT* pSourceRect, POINT* pDestPoint);
	HRESULT StretchRect(m_IDirectDrawSurfaceX* pSourceSurface, RECT* pSourceRect, RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter);
	HRESULT UpdateSurfaceColorKey(m_IDirectDrawSurfaceX* pSourceSurface, RECT* pSourceRect, POINT* pDestPoint, DDCOLORKEY ColorKey, bool DoColorKey = true);
	HRESULT StretchRectColorKey(m_IDirectDrawSurfaceX* pSourceSurface, RECT* pSourceRect, RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter, DDCOLORKEY ColorKey, bool DoColorKey = true);

	// Release interface
	void ReleaseInterface();
};
