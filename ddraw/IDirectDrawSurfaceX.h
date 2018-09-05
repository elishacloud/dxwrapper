#pragma once
#include <map>

class m_IDirectDrawSurfaceX : public IDirectDrawSurface7
{
private:
	IDirectDrawSurface7 *ProxyInterface;
	m_IDirectDrawSurface7 *WrapperInterface;
	DWORD DirectXVersion;
	DWORD ProxyDirectXVersion;
	IID WrapperID;
	ULONG RefCount = 1;
	bool ConvertSurfaceDescTo2 = false;

	// Color Key Structure
	struct CKEYS
	{
		DDCOLORKEY Key = { 0, 0 };
		bool IsSet = false;
		bool IdColorSpace = false;
	};

	// Convert to d3d9
	m_IDirectDrawX *ddrawParent = nullptr;
	m_IDirectDrawPalette *attachedPalette = nullptr;	// Associated palette
	DDSURFACEDESC2 surfaceDesc2;
	D3DLOCKED_RECT d3dlrect = { 0, nullptr };
	RECT lkDestRect;
	CKEYS ColorKeys[4];		// Color keys(0 = DDCKEY_DESTBLT, 1 = DDCKEY_DESTOVERLAY, 2 = DDCKEY_SRCBLT, 3 = DDCKEY_SRCOVERLAY)
	LONG overlayX = 0;
	LONG overlayY = 0;
	DWORD BufferSize = 0;
	BYTE *rawVideoBuf = nullptr;						// Virtual video buffer
	bool WriteDirectlyToSurface = false;
	bool IsLocked = false;

	// Display resolution
	DWORD displayWidth = 0;
	DWORD displayHeight = 0;

	// Direct3D9 vars
	LPDIRECT3DDEVICE9 *d3d9Device = nullptr;
	LPDIRECT3DTEXTURE9 surfaceTexture = nullptr;
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer = nullptr;

	// Store a list of attached surfaces
	std::map<DWORD, m_IDirectDrawSurfaceX*> AttachedSurfaceMap;

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
	m_IDirectDrawSurfaceX(IDirectDrawSurface7 *pOriginal, DWORD Version, m_IDirectDrawSurface7 *Interface) : ProxyInterface(pOriginal), DirectXVersion(Version), WrapperInterface(Interface)
	{
		InitWrapper();
	}
	m_IDirectDrawSurfaceX(LPDIRECT3DDEVICE9 *lplpDevice, m_IDirectDrawX *Interface, DWORD Version, LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD Width, DWORD Height) :
		d3d9Device(lplpDevice), ddrawParent(Interface), DirectXVersion(Version), displayWidth(Width), displayHeight(Height)
	{
		ProxyInterface = nullptr;
		WrapperInterface = nullptr;

		InitWrapper();

		// Copy surface description
		memcpy(&surfaceDesc2, lpDDSurfaceDesc, sizeof(DDSURFACEDESC2));

		// Create Surface for d3d9
		if (d3d9Device)
		{
			CreateD3d9Surface();
		}

		// Store surface
		if (ddrawParent)
		{
			ddrawParent->AddSurfaceToVector(this);
		}
	}
	void InitWrapper()
	{
		WrapperID = (DirectXVersion == 1) ? IID_IDirectDrawSurface :
			(DirectXVersion == 2) ? IID_IDirectDrawSurface2 :
			(DirectXVersion == 3) ? IID_IDirectDrawSurface3 :
			(DirectXVersion == 4) ? IID_IDirectDrawSurface4 :
			(DirectXVersion == 7) ? IID_IDirectDrawSurface7 : IID_IDirectDrawSurface7;

		if (Config.Dd7to9)
		{
			ProxyDirectXVersion = 9;
		}
		else
		{
			ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(WrapperID));
		}

		if (ProxyDirectXVersion > 3 && DirectXVersion < 4)
		{
			ConvertSurfaceDescTo2 = true;
		}

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert DirectDrawSurface v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
	}
	~m_IDirectDrawSurfaceX()
	{
		if (rawVideoBuf)
		{
			delete rawVideoBuf;
			rawVideoBuf = nullptr;
		}

		if (ddrawParent)
		{
			ddrawParent->RemoveSurfaceFromVector(this);
			ReleaseD9Surface();
		}
	}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirectDrawSurface7 *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDrawSurface7 *GetWrapperInterface() { return WrapperInterface; }
	LPDIRECT3DTEXTURE9 *GetSurfaceTexture() { return &surfaceTexture; }
	bool IsSurfaceLocked() { return IsLocked; }
	bool IsPrimarySurface() { return (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) != 0; }
	bool NeedsLock() { return !IsLocked && WriteDirectlyToSurface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IDirectDrawSurface methods ***/
	STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE7);
	STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT);
	STDMETHOD(Blt)(THIS_ LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX);
	STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD);
	STDMETHOD(BltFast)(THIS_ DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD);
	STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD, LPDIRECTDRAWSURFACE7);
	STDMETHOD(EnumAttachedSurfaces)(THIS_ LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback7)
	{
		if (DirectXVersion < 4)
		{
			return EnumAttachedSurfaces(lpContext, (LPDDENUMSURFACESCALLBACK)lpEnumSurfacesCallback7);
		}

		return EnumAttachedSurfaces2(lpContext, lpEnumSurfacesCallback7);
	}
	HRESULT EnumAttachedSurfaces(LPVOID, LPDDENUMSURFACESCALLBACK);
	HRESULT EnumAttachedSurfaces2(LPVOID, LPDDENUMSURFACESCALLBACK7);
	STDMETHOD(EnumOverlayZOrders)(THIS_ DWORD dwFlags, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpfnCallback7)
	{
		if (DirectXVersion < 4)
		{
			return EnumOverlayZOrders(dwFlags, lpContext, (LPDDENUMSURFACESCALLBACK)lpfnCallback7);
		}

		return EnumOverlayZOrders2(dwFlags, lpContext, lpfnCallback7);
	}
	HRESULT EnumOverlayZOrders(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK);
	HRESULT EnumOverlayZOrders2(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7);
	STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE7, DWORD);
	STDMETHOD(GetAttachedSurface)(THIS_ LPDDSCAPS2 lpDDSCaps2, LPDIRECTDRAWSURFACE7 FAR * lplpDDAttachedSurface)
	{
		if (DirectXVersion < 4)
		{
			return GetAttachedSurface((LPDDSCAPS)lpDDSCaps2, lplpDDAttachedSurface);
		}

		return GetAttachedSurface2(lpDDSCaps2, lplpDDAttachedSurface);
	}
	HRESULT GetAttachedSurface(LPDDSCAPS, LPDIRECTDRAWSURFACE7 FAR *);
	HRESULT GetAttachedSurface2(LPDDSCAPS2, LPDIRECTDRAWSURFACE7 FAR *);
	STDMETHOD(GetBltStatus)(THIS_ DWORD);
	STDMETHOD(GetCaps)(THIS_ LPDDSCAPS2 lpDDSCaps2)
	{
		if (DirectXVersion < 4)
		{
			return GetCaps((LPDDSCAPS)lpDDSCaps2);
		}

		return GetCaps2(lpDDSCaps2);
	}
	HRESULT m_IDirectDrawSurfaceX::GetCaps(LPDDSCAPS);
	HRESULT m_IDirectDrawSurfaceX::GetCaps2(LPDDSCAPS2);
	STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*);
	STDMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY);
	STDMETHOD(GetDC)(THIS_ HDC FAR *);
	STDMETHOD(GetFlipStatus)(THIS_ DWORD);
	STDMETHOD(GetOverlayPosition)(THIS_ LPLONG, LPLONG);
	STDMETHOD(GetPalette)(THIS_ LPDIRECTDRAWPALETTE FAR*);
	STDMETHOD(GetPixelFormat)(THIS_ LPDDPIXELFORMAT);
	STDMETHOD(GetSurfaceDesc)(THIS_ LPDDSURFACEDESC2 lpDDSurfaceDesc2)
	{
		if (DirectXVersion < 4)
		{
			return GetSurfaceDesc((LPDDSURFACEDESC)lpDDSurfaceDesc2);
		}

		return GetSurfaceDesc2(lpDDSurfaceDesc2);
	}
	HRESULT GetSurfaceDesc(LPDDSURFACEDESC);
	HRESULT GetSurfaceDesc2(LPDDSURFACEDESC2);
	STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW lpDD, LPDDSURFACEDESC2 lpDDSurfaceDesc2)
	{
		if (DirectXVersion < 4)
		{
			return Initialize(lpDD, (LPDDSURFACEDESC)lpDDSurfaceDesc2);
		}

		return Initialize2(lpDD, lpDDSurfaceDesc2);
	}
	HRESULT Initialize(LPDIRECTDRAW lpDD, LPDDSURFACEDESC lpDDSurfaceDesc);
	HRESULT Initialize2(LPDIRECTDRAW lpDD, LPDDSURFACEDESC2 lpDDSurfaceDesc2);
	STDMETHOD(IsLost)(THIS);
	STDMETHOD(Lock)(THIS_ LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD dwFlags, HANDLE hEvent)
	{
		if (DirectXVersion < 4)
		{
			return Lock(lpDestRect, (LPDDSURFACEDESC)lpDDSurfaceDesc2, dwFlags, hEvent);
		}

		return Lock2(lpDestRect, lpDDSurfaceDesc2, dwFlags, hEvent);
	}
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
	STDMETHOD(GetDDInterface)(THIS_ LPVOID FAR *);
	STDMETHOD(PageLock)(THIS_ DWORD);
	STDMETHOD(PageUnlock)(THIS_ DWORD);

	/*** Added in the v3 interface ***/
	STDMETHOD(SetSurfaceDesc)(THIS_ LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD dwFlags)
	{
		if (DirectXVersion < 4)
		{
			return SetSurfaceDesc((LPDDSURFACEDESC)lpDDSurfaceDesc2, dwFlags);
		}

		return SetSurfaceDesc2(lpDDSurfaceDesc2, dwFlags);
	}
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
	void AlocateVideoBuffer();
	HRESULT CreateD3d9Surface();
	void ReleaseD9Surface();
	bool FixRect(LPRECT lpOutRect, LPRECT lpInRect);
	HRESULT SetLock(LPRECT lpDestRect, DWORD dwFlags);
	HRESULT SetUnLock();
	HRESULT GetSurfaceInfo(D3DLOCKED_RECT *pLockRect, DWORD *lpBitCount, D3DFORMAT *lpFormat);
	void AddAttachedSurfaceToMap(m_IDirectDrawSurfaceX* lpSurfaceX);
	void RemoveAttachedSurfaceFromMap(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool DoesAttachedSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX);
	HRESULT ColorFill(RECT *pRect, DWORD dwFillColor);
	HRESULT WritePaletteToSurface(m_IDirectDrawPalette *lpDDPalette, RECT *pRect, BYTE *lpVideoBuf, DWORD BitCount);
	HRESULT CopyRect(D3DLOCKED_RECT *pDestLockRect, RECT *pDestRect, DWORD DestBitCount, D3DFORMAT DestFormat, D3DLOCKED_RECT *pSrcLockRect, RECT *pSrcRect, DWORD SrcBitCount, D3DFORMAT SrcFormat);
	HRESULT CopyRectColorKey(D3DLOCKED_RECT *pDestLockRect, RECT *pDestRect, DWORD DestBitCount, D3DFORMAT DestFormat, D3DLOCKED_RECT *pSrcLockRect, RECT *pSrcRect, DWORD SrcBitCount, D3DFORMAT SrcFormat, DDCOLORKEY ColorKey);
	HRESULT StretchRect(D3DLOCKED_RECT *pDestLockRect, RECT *pDestRect, DWORD DestBitCount, D3DFORMAT DestFormat, D3DLOCKED_RECT *pSrcLockRect, RECT *pSrcRect, DWORD SrcBitCount, D3DFORMAT SrcFormat);
};
