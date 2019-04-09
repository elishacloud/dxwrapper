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

	// Convert to d3d9
	m_IDirectDrawX *ddrawParent = nullptr;
	m_IDirectDrawPalette *attachedPalette = nullptr;	// Associated palette
	m_IDirectDrawClipper *attachedClipper = nullptr;	// Associated clipper
	DDSURFACEDESC2 surfaceDesc2;
	D3DLOCKED_RECT d3dlrect = { 0, nullptr };
	RECT lkDestRect;
	CKEYS ColorKeys[4];		// Color keys(0 = DDCKEY_DESTBLT, 1 = DDCKEY_DESTOVERLAY, 2 = DDCKEY_SRCBLT, 3 = DDCKEY_SRCOVERLAY)
	LONG overlayX = 0;
	LONG overlayY = 0;
	DWORD BufferSize = 0;
	BYTE *rawVideoBuf = nullptr;						// Virtual video buffer
	DWORD UniquenessValue = 0;
	bool WriteDirectlyToSurface = false;
	bool IsLocked = false;
	bool PaletteFirstRun = true;
	bool ClipperFirstRun = true;

	// Display resolution
	DWORD displayWidth = 0;
	DWORD displayHeight = 0;

	// Direct3D9 vars
	LPDIRECT3DDEVICE9 *d3d9Device = nullptr;
	LPDIRECT3DTEXTURE9 surfaceTexture = nullptr;
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer = nullptr;

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

public:
	m_IDirectDrawSurfaceX(IDirectDrawSurface7 *pOriginal, DWORD DirectXVersion, m_IDirectDrawSurface7 *Interface) : ProxyInterface(pOriginal), WrapperInterface(Interface)
	{
		ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert DirectDrawSurface v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
		else
		{
			Logging::LogDebug() << "Create " << __FUNCTION__ << " v" << DirectXVersion;
		}
	}
	m_IDirectDrawSurfaceX(LPDIRECT3DDEVICE9 *lplpDevice, m_IDirectDrawX *Interface, DWORD DirectXVersion, LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD Width, DWORD Height) :
		d3d9Device(lplpDevice), ddrawParent(Interface), displayWidth(Width), displayHeight(Height)
	{
		ProxyDirectXVersion = 9;

		// Copy surface description
		surfaceDesc2.dwSize = sizeof(DDSURFACEDESC2);
		surfaceDesc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		ConvertSurfaceDesc(surfaceDesc2, *lpDDSurfaceDesc2);

		Logging::LogDebug() << "Convert DirectDrawSurface v" << DirectXVersion << " to v" << ProxyDirectXVersion;

		// Store surface
		if (ddrawParent)
		{
			ddrawParent->AddSurfaceToVector(this);
		}
	}
	~m_IDirectDrawSurfaceX()
	{
		if (rawVideoBuf)
		{
			delete rawVideoBuf;
			rawVideoBuf = nullptr;
		}

		if (Config.Dd7to9 && !Config.Exiting)
		{
		}
	}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return IID_IUnknown; }
	IDirectDrawSurface7 *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDrawSurface7 *GetWrapperInterface() { return WrapperInterface; }
	void SetDdrawParent(m_IDirectDrawX *ddraw) { ddrawParent = ddraw; }

	/*** IUnknown methods ***/
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, ProxyDirectXVersion); }
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IDirectDrawSurface methods ***/
	STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE7);
	STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT);
	STDMETHOD(Blt)(THIS_ LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX);
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
	HRESULT Initialize(LPDIRECTDRAW lpDD, LPDDSURFACEDESC lpDDSurfaceDesc);
	HRESULT Initialize2(LPDIRECTDRAW lpDD, LPDDSURFACEDESC2 lpDDSurfaceDesc2);
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

	/*** Helper functions ***/
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
	LPDIRECT3DTEXTURE9 *GetSurfaceTexture() { return &surfaceTexture; }
	m_IDirectDrawPalette **GetPallete() { return &attachedPalette; }
	BYTE **GetRawVideoMemory() { return &rawVideoBuf; }
	bool IsSurfaceLocked() { return IsLocked; }
	bool IsPrimarySurface() { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) != 0; }
	bool NeedsLock() { return !IsLocked && WriteDirectlyToSurface; }
	void ClearDdraw() { ddrawParent = nullptr; }
	template <typename T>
	void SwapAddresses(T *Address1, T *Address2)
	{
		T tmpAddr = *Address1;
		*Address1 = *Address2;
		*Address2 = tmpAddr;
	}
	void AlocateVideoBuffer();
	bool CheckD3d9Surface();
	HRESULT CreateD3d9Surface();
	void ReleaseD9Surface();
	bool FixRect(LPRECT lpOutRect, LPRECT lpInRect);
	HRESULT SetLock(LPRECT lpDestRect, DWORD dwFlags, bool SkipBeginSceneFlag = false);
	HRESULT SetUnLock(bool SkipEndSceneFlag = false);
	HRESULT GetSurfaceInfo(D3DLOCKED_RECT *pLockRect, DWORD *lpBitCount, D3DFORMAT *lpFormat);
	void AddAttachedSurfaceToMap(m_IDirectDrawSurfaceX* lpSurfaceX);
	void RemoveAttachedSurfaceFromMap(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool DoesAttachedSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX);
	HRESULT ColorFill(RECT *pRect, DWORD dwFillColor);
	HRESULT WritePaletteToSurface(m_IDirectDrawPalette *lpDDPalette, RECT *pRect, BYTE *lpVideoBuf, DWORD BitCount);
	HRESULT CopyRect(D3DLOCKED_RECT *pDestLockRect, RECT *pDestRect, DWORD DestBitCount, D3DFORMAT DestFormat, D3DLOCKED_RECT *pSrcLockRect, RECT *pSrcRect, DWORD SrcBitCount, D3DFORMAT SrcFormat);
	HRESULT CopyRectColorKey(D3DLOCKED_RECT *pDestLockRect, RECT *pDestRect, DWORD DestBitCount, D3DFORMAT DestFormat, D3DLOCKED_RECT *pSrcLockRect, RECT *pSrcRect, DWORD SrcBitCount, D3DFORMAT SrcFormat, DDCOLORKEY ColorKey);
	HRESULT StretchRect(D3DLOCKED_RECT *pDestLockRect, RECT *pDestRect, DWORD DestBitCount, D3DFORMAT DestFormat, D3DLOCKED_RECT *pSrcLockRect, RECT *pSrcRect, DWORD SrcBitCount, D3DFORMAT SrcFormat);
	HRESULT WriteVideoDataToSurface();
	void SwapSurface(m_IDirectDrawSurfaceX *lpTargetSurface1, m_IDirectDrawSurfaceX *lpTargetSurface2);
	void ReleaseSurface();
};
